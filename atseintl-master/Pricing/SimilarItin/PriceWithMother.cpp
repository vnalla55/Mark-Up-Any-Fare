/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/SimilarItin/PriceWithMother.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/IntlJourneyUtil.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TseEnums.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/MIPFamilyLogicUtils.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/FarePathBuilder.h"
#include "Pricing/SimilarItin/SurchargesValidator.h"

namespace tse
{

FALLBACK_DECL(fallbackGSAChildItinFix2);
FALLBACK_DECL(revalidateVcxForSimilarItins_CAT12)
FALLBACK_DECL(validateSurchargesMotherSolution)
FALLBACK_DECL(revalidateCat12MotherModule)
FALLBACK_DECL(gsaSurchargesFixMotherModule)
FALLBACK_DECL(validateCarrierModuleMotherSolution)
FALLBACK_DECL(reworkTrxAborter);

namespace similaritin
{
namespace
{
ConfigurableValue<uint32_t>
maxNextGFPAttempts("SHOPPING_OPT", "NEXT_GFP_FOR_SIMILAR_ITIN_ATTEMPTS_NUMBER", 1);
}

template <typename D>
PriceWithMother<D>::PriceWithMother(Context& context, D& diagnostic)
  : _validatingCarrierModule(context, diagnostic.get990Diag()),
    _revalidator(context, diagnostic),
    _farePathBuilder(
        context, diagnostic, !fallback::validateCarrierModuleMotherSolution(&context.trx)),
    _context(context),
    _diagnostic(diagnostic),
    _nextGFPAttemptsNumber(maxNextGFPAttempts.getValue())
{
  //TODO(andrzej.fediuk) FL: introduce config value for BFA max numbers
  //hardcoded for now
  if (_context.trx.isBRAll())
    _nextGFPAttemptsNumber = 32; //magic number!
}

template <typename D>
bool
PriceWithMother<D>::findSolution(GroupFarePathFactory& groupFarePathFactory)
{
  PricingTrx& trx = _context.trx;
  DiagCollector& diag = _context.diagnostic;

  uint16_t attemptNo = 0;
  bool generateNextGFP = true;

  // thrufare not required for BRALL
  const bool mustBeThroughFare = trx.isBRAll() ? false :
      !(trx.getTrxType() == PricingTrx::MIP_TRX && trx.billing()->actionCode() == "WFR");

  if (mustBeThroughFare)
    groupFarePathFactory.setThroughFarePricing();

  while (1)
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    GroupFarePath* gfp = groupFarePathFactory.getGroupFarePath(diag);

    if (!gfp)
      return false;

    const bool isPricedThroughFare = MIPFamilyLogicUtils::pricedThroughFare(trx, *gfp);

    if (!mustBeThroughFare || isPricedThroughFare)
    {
      processFinalGroupFarePath(gfp->groupFPPQItem());

      _diagnostic.printAttempt(attemptNo + 1, *gfp);

      generateNextGFP = !verifySimilarItin(gfp->groupFPPQItem());

      _diagnostic.printNotApplicable();

      if (!generateNextGFP || (attemptNo >= _nextGFPAttemptsNumber))
        break;
    }

    attemptNo++;
  }

  return !generateNextGFP;
}

template <typename D>
bool
PriceWithMother<D>::verifySimilarItin(const std::vector<FPPQItem*>& groupFPath)
{
  Itin& motherItin = _context.motherItin;

  bool allPriced = true;
  for (const SimilarItinData& similarItinData : motherItin.getSimilarItins())
  {
    Itin& similarItin = *similarItinData.itin;
    if (isPriced(similarItin, groupFPath))
    {
      // It Already Priced, BkgCode might be same as Head-Itin
      continue;
    }
    // Don't replace with lazy-evaluated "&&"
    allPriced &= populateSimilarItin(groupFPath, similarItin);
  }

  return allPriced;
}

template <typename D>
bool
PriceWithMother<D>::isPriced(const Itin& est, const std::vector<FPPQItem*>& gfp) const
{
  if (_context.trx.isBRAll() && _context.motherItin.isItinBrandingSet())
  {
    //TODO(andrzej.fediuk) FL: needed?
    const size_t spaceIndex = _context.motherItin.getItinBranding().getBrandingSpaceIndex(
        _context.motherItin.fmpMatrix());
    return std::any_of(est.farePath().crbegin(),
                       est.farePath().crend(),
                       [&spaceIndex](const FarePath* fp)
                       { return fp->brandIndex() == spaceIndex; });
  }

  const PricingTrx& trx = _context.trx;
  if (LIKELY(!trx.isFlexFare()))
    return !est.farePath().empty();

  // safety checks
  if (UNLIKELY(gfp.empty() || !gfp.front() || !gfp.front()->farePath()))
  {
    return true; // we cannot use this gfp to price similar itin,
    // so returning true should stop processing it
  }

  const flexFares::GroupId groupId = gfp.front()->farePath()->getFlexFaresGroupId();

  return std::any_of(est.farePath().rbegin(),
                     est.farePath().rend(),
                     [&groupId](const FarePath* fp)
                     { return fp->getFlexFaresGroupId() == groupId; });
}

template <typename D>
bool
PriceWithMother<D>::populateSimilarItin(const std::vector<FPPQItem*>& groupFPath, Itin& est)
{
  Itin& motherItin = _context.motherItin;

  if (isPriced(est, groupFPath))
    return true;

  MIPFamilyLogicUtils::populateSimilarItinData(_context.trx, est, _context.motherItin);
  std::vector<FarePath*> farePathVec(_farePathBuilder.cloneFarePaths(groupFPath, est));
  if (farePathVec.empty())
    return false;

  if (fallback::fallbackGSAChildItinFix2(&_context.trx))
  {
    if ((motherItin.farePath().size() > 0) &&
        (motherItin.farePath()[0]->validatingCarriers().size() > 0))
    {
      // Use first validating carrier from mother fare path
      est.validatingCarrier() = motherItin.farePath()[0]->validatingCarriers()[0];
      motherItin.validatingCarrier() = motherItin.farePath()[0]->validatingCarriers()[0];
    }
  }

  // Update the child fare paths with child itin
  for (FarePath* farePath : est.farePath())
    farePath->itin() = &est;

  if (!fallback::revalidateCat12MotherModule(&_context.trx))
  {
    if (!fallback::revalidateVcxForSimilarItins_CAT12(&_context.trx))
    {
      SurchargesValidator<D> validator(_context.trx, _diagnostic);

      for (FarePath* farePath : farePathVec)
      {
        if (!fallback::gsaSurchargesFixMotherModule(&_context.trx))
          if (_context.trx.isValidatingCxrGsaApplicable() &&
              !farePath->validatingCarriers().empty())
            farePath->itin()->validatingCarrier() = farePath->validatingCarriers().front();

        validator.applySurcharges(*farePath);
      }
    }
  }

  if (!_revalidator.validate(farePathVec, groupFPath, est))
    return false;

  if (!fallback::revalidateCat12MotherModule(&_context.trx))
  {
    if (fallback::validateSurchargesMotherSolution(&_context.trx))
      for (auto* farePath : farePathVec)
        PricingUtil::finalPricingProcess(_context.trx, *farePath);
  }

  est.farePath().insert(est.farePath().end(), farePathVec.begin(), farePathVec.end());
  _diagnostic.itinPopulated(est, farePathVec);

  return true;
}

template <typename D>
bool
PriceWithMother<D>::processFinalGroupFarePath(const std::vector<FPPQItem*>& groupFPath)
{
  for (FPPQItem* fppqItem : groupFPath)
  {
    if (!PricingUtil::finalPricingProcess(_context.trx, *fppqItem->farePath()))
      return false;
    MaximumPenaltyValidator(_context.trx).completeResponse(*fppqItem->farePath());
  }
  return true;
}

template class PriceWithMother<DiagnosticWrapper>;
template class PriceWithMother<NoDiagnostic>;
}
}
