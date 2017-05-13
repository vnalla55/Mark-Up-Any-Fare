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

#include "Pricing/SimilarItin/FarePathBuilder.h"

#include "Common/FallbackUtil.h"
#include "Common/SimilarItinSegmentsBuilder.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NetRemitFarePath.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/VectorSwapper.h"

#include <list>

namespace tse
{
FALLBACK_DECL(fallbackGSAChildItinFix2)
namespace similaritin
{
namespace
{
template <typename Data>
bool
adjustOtherData(const SegmentsBuilder& segmentsBuilder,
                std::vector<Data*>& data,
                DataHandle& dataHandle)
{
  for (Data*& dataItem : data)
  {
    Data* newItem = dataItem->clone(dataHandle);

    auto newTSVec = segmentsBuilder.constructByOriginAndDestination(newItem->travelSeg());
    if (newTSVec.empty())
      return false;

    newItem->travelSeg().swap(newTSVec);
    dataItem = newItem;
  }
  return true;
}
}

template <typename D>
FarePathBuilder<D>::FarePathBuilder(Context& context, D& diagnostic, bool validateCarrierModule)
  : FarePathCopier(context.trx.dataHandle()),
    _validatingCarrierModule(context, diagnostic.get990Diag()),
    _bookingCodeSelector(context, diagnostic),
    _diagnostic(diagnostic),
    _context(context),
    _validateCarrierModule(validateCarrierModule)
{
}

template <typename D>
FarePath*
FarePathBuilder<D>::cloneFarePath(FPPQItem& sourceItem, Itin& est)
{
  FarePath* result = duplicateAndAdjustFarePath(*sourceItem.farePath(), est);

  if (result)
  {
    if (_context.trx.isBRAll() && _context.motherItin.isItinBrandingSet())
      result->brandIndex() =
          _context.motherItin.getItinBranding().getBrandingSpaceIndex(_context.motherItin.fmpMatrix());
    else
      result->brandIndex() = sourceItem.farePath()->brandIndex();
  }

  bool cloningPassed = (nullptr != result);

  if (fallback::fallbackGSAChildItinFix2(&_context.trx) || _validateCarrierModule)
    cloningPassed = cloningPassed && _validatingCarrierModule.buildCarrierLists(est, result);

  if (!cloningPassed)
  {
    _diagnostic.itinNotApplicable(est, Diag990Collector::CloningFailed);
      return nullptr;
  }

  result->gsaClonedFarePaths() = duplicateFarePathsForGSA(*sourceItem.farePath(), est);

  if (!selectBookingCode(*result, est))
  {
    _diagnostic.itinNotApplicable(est, Diag990Collector::GetBookingCodeForChildItin);
    return nullptr;
  }

  if (est.validatingCarrier().empty())
    updateValidatingCarrier(_context.trx, *result);

  return result;
}

template <typename D>
FPPQItem*
FarePathBuilder<D>::cloneFPPQItem(const FPPQItem& source, Itin& est)
{
  FPPQItem* result = source.createDuplicate(_context.trx);
  if ((result->farePath() = cloneFarePath(*result, est)))
    return result;
  return nullptr;
}

template <typename D>
std::vector<FarePath*>
FarePathBuilder<D>::cloneForEachFPPQItem(const std::vector<FPPQItem*>& groupFPath, Itin& est)
{
  std::vector<FarePath*> result;
  for (FPPQItem* fppqItem : groupFPath)
  {
    FarePath* clone = cloneFarePath(*fppqItem, est);
    if (clone)
      result.push_back(clone);
    else
      return std::vector<FarePath*>();
  }
  return result;
}

template <typename D>
std::vector<FarePath*>
FarePathBuilder<D>::cloneFarePaths(const std::vector<FPPQItem*>& groupFPath, Itin& est)
{
  std::vector<FarePath*> result(cloneForEachFPPQItem(groupFPath, est));
  if (!result.empty())
  {
    FarePath* fp = result.front();
    if (est.validatingCarrier().empty() && fp)
      updateValidatingCarrier(_context.trx, *fp);
  }

  return result;
}

template <typename D>
FarePath*
FarePathBuilder<D>::duplicateAndAdjustFarePath(const FarePath& farePath, Itin& similar)
{
  Itin& itin = _context.motherItin;
  FarePath* result = duplicateBase(farePath);
  result->setYqyrCalculator(nullptr);

  if (farePath.netRemitFarePath())
  {
    result->netRemitFarePath() = dynamic_cast<NetRemitFarePath*>(
        duplicateAndAdjustFarePath(*farePath.netRemitFarePath(), similar));

    if (!result->netRemitFarePath())
      return nullptr;
  }

  result->itin() = &similar;
  SegmentsBuilder segmentsBuilder(_context.trx, itin, similar);

  for (size_t indexPU = 0; indexPU != result->pricingUnit().size(); ++indexPU)
  {
    PricingUnit& resultPu = *result->pricingUnit()[indexPU];

    std::vector<TravelSeg*> puTSVec;

    for (size_t indexFU = 0; indexFU != resultPu.fareUsage().size(); ++indexFU)
    {
      FareUsage& fareUsage = *resultPu.fareUsage()[indexFU];
      if (!adjustFareUsage(
              segmentsBuilder, *farePath.pricingUnit()[indexPU]->fareUsage()[indexFU], fareUsage))
        return nullptr;

      // Copy FU travel segments into PU
      puTSVec.insert(puTSVec.end(), fareUsage.travelSeg().begin(), fareUsage.travelSeg().end());
    }

    // Update PU travel segment vector
    if (segmentsBuilder.similarItinGeoConsistent())
    {
      for (TravelSeg*& travelSeg : resultPu.travelSeg())
      {
        const int index = itin.segmentOrder(travelSeg) - 1;
        if (LIKELY(index >= 0 && index < int(similar.travelSeg().size())))
          travelSeg = similar.travelSeg()[index];
        else
          return nullptr;
      }
    }
    else
      resultPu.travelSeg().swap(puTSVec);
  }

  return result;
}

template <typename D>
bool
FarePathBuilder<D>::adjustFareUsage(const SegmentsBuilder& segmentsBuilder,
                                    const FareUsage& source,
                                    FareUsage& output)
{
  if (!output.differentialPlusUp().empty())
    if (!adjustOtherData(segmentsBuilder, output.differentialPlusUp(), _context.trx.dataHandle()))
      return false;

  output.tktEndorsement() = source.tktEndorsement();
  output.minFarePlusUp() = source.minFarePlusUp();

  // Rebuild FU level travel segment vector
  auto fuTSVec = segmentsBuilder.constructByOriginAndDestination(output.travelSeg());
  if (fuTSVec.empty())
    return false;

  // Replace the FU travel segment vector with the newly constructed vector
  output.travelSeg().swap(fuTSVec);
  return true;
}

template <typename D>
std::vector<FarePath*>
FarePathBuilder<D>::duplicateFarePathsForGSA(const FarePath& farePath, Itin& similar)
{
  std::vector<FarePath*> result;

  if (farePath.gsaClonedFarePaths().empty())
    return result;

  for (const FarePath* gsaFarePath : farePath.gsaClonedFarePaths())
  {
    FarePath* fp = nullptr;
    fp = duplicateAndAdjustFarePath(*gsaFarePath, similar);
    fp->itin() = &similar;
    result.push_back(fp);
  }
  return result;
}

template <typename D>
bool
FarePathBuilder<D>::selectBookingCode(FarePath& farePath, Itin& est)
{
  std::list<similaritin::VectorSwapper<TravelSeg*>> segments;
  for (PricingUnit* pu : farePath.pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
      segments.emplace_back(fu->travelSeg(), fu->paxTypeFare()->fareMarket()->travelSeg());

  return _bookingCodeSelector.getBookingCode(&farePath, est);
}

template class FarePathBuilder<NoDiagnostic>;
template class FarePathBuilder<DiagnosticWrapper>;
}
}
