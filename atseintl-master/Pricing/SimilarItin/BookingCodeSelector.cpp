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
#include "Pricing/SimilarItin/BookingCodeSelector.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/JourneyUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SimilarItinData.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diag990Collector.h"
#include "Pricing/JourneyValidator.h"
#include "Pricing/SimilarItin/DataSwappers.h"
#include "Pricing/SimilarItin/FarePathValidator.h"

namespace tse
{
FALLBACK_DECL(fallbackEnableFamilyLogicAvailiablityValidationInBfa)
FALLBACK_DECL(similarItinSelectThroughAvailability)

static Logger
logger("atseintl.Pricing.SimilarItin.BookingCodeSelector");

namespace similaritin
{
namespace
{
class SwapFareMarkets
{
public:
  SwapFareMarkets(Itin& itin, Itin& motherItin) : _itin(itin), _motherItin(motherItin)
  {
    itin.fareMarket().swap(motherItin.fareMarket());
  }

  ~SwapFareMarkets() { _itin.fareMarket().swap(_motherItin.fareMarket()); }

private:
  Itin& _itin;
  Itin& _motherItin;
};

std::string
itinName(Itin& itin)
{
  if (itin.itinNum() == INVALID_INT_INDEX)
    return "Itin";
  return "Itin #" + std::to_string(itin.itinNum());
}
}

template <typename D>
BookingCodeSelector<D>::BookingCodeSelector(Context& context, D& diagnostic)
  : _diagnostic(diagnostic), _revalidator(context, _diagnostic), _context(context)
{
}

template <typename D>
bool
BookingCodeSelector<D>::getBookingCode(FarePath* farePath, Itin& est)
{
  PricingTrx& trx = _context.trx;
  TSELatencyData metrics(trx, "SIMILAR ITIN BOOKING CODES");

  Diag990Collector* diag990 = _diagnostic.get990Diag();

  std::string localRes;

  FareMarketDataResetter<FMContext> fareMarketDataResetter(*farePath);

  if (!fillWithSimilarFareMarkets(*farePath, est))
  {
    if (UNLIKELY(diag990))
      diag990->setBookingCodeForChildFailedReason(&est,
                                                  "Mother and similar fare markets do not match");
    return false;
  }

  if (!revalidateFBR(*farePath))
  {
    if (UNLIKELY(diag990))
      diag990->setBookingCodeForChildFailedReason(&est, "FBR prime RBD validation failed");
    return false;
  }

  if (!revalidateBookingCodes(*farePath, est))
  {
    if (UNLIKELY(diag990))
      diag990->setBookingCodeForChildFailedReason(&est, "FareBookingCode validation failed");
    return false;
  }

  const bool journeyLogic = JourneyUtil::journeyActivated(trx);
  JourneyValidator journeyValidator(trx, _context.diagnostic);
  if (journeyLogic)
  {
    if (!journeyValidator.validateJourney(*farePath, localRes))
    {
      if (UNLIKELY(diag990))
        diag990->setBookingCodeForChildFailedReason(&est, "Journey validation failed");
      return false;
    }
  }

  if (!validateMixedClass(*farePath))
  {
    if (UNLIKELY(diag990))
      diag990->setBookingCodeForChildFailedReason(&est, "Mixed class validation failed");
    return false;
  }

  if (journeyLogic)
  {
    if (!journeyValidator.processJourneyAfterDifferential(*farePath, localRes))
    {
      if (UNLIKELY(diag990))
        diag990->setBookingCodeForChildFailedReason(&est,
                                                    "Journey after differential validation failed");
      return false;
    }
  }

  {
    // The FMs have to be swapped since FP uses mothers FMs.
    SwapFareMarkets swap(est, _context.motherItin);
    ShoppingUtil::updateFinalBookingBasedOnAvailBreaks(trx, *farePath, est);
  }

  return true;
}

template <typename D>
bool
BookingCodeSelector<D>::fillWithSimilarFareMarkets(FarePath& farePath, Itin& est)
{
  const auto itinData = _context.motherItin.getSimilarItinData(est);

  if (!itinData)
    return false;

  for (auto* const pu : farePath.pricingUnit())
  {
    for (auto* const fu : pu->fareUsage())
    {
      PaxTypeFare* ptf = fu->paxTypeFare();
      FareMarket* fm = ptf->fareMarket();
      FareMarket* similarFM = itinData->getSimilarFareMarket(fm);

      if (!similarFM)
      {
        LOG4CXX_ERROR(logger,
                      itinName(est) << ": Could not find prebuilt similar fare market for "
                                    << FareMarketUtil::getDisplayString(*fm));
        return false;
      }

      if (fm->governingCarrier() != similarFM->governingCarrier())
        return false;

      TSE_ASSERT(fm->travelSeg().size() == similarFM->travelSeg().size());

      fm->availBreaks() = similarFM->availBreaks();
      fm->travelBoundary() = similarFM->travelBoundary();
      fm->primarySector() = similarFM->primarySector();

      if (fallback::similarItinSelectThroughAvailability(&_context.trx))
        ShoppingUtil::getFMCOSBasedOnAvailBreak(_context.trx, &est, fm);
      else
        ShoppingUtil::getFMCOSBasedOnAvailBreak(_context.trx, &est, fm, false);
      _diagnostic.updateChildBookingClass(&est, fm->classOfServiceVec());
    }
  }

  return true;
}

template <typename D>
bool
BookingCodeSelector<D>::revalidateFBR(FarePath& farePath)
{
  for (const auto pu : farePath.pricingUnit())
  {
    for (const auto fu : pu->fareUsage())
    {
      const PaxTypeFare* ptf = fu->paxTypeFare();
      if (!ptf->isFareByRule())
        continue;
      if (!_revalidator.validateFbr(*ptf))
        return false;
    }
  }

  return true;
}

template <typename D>
bool
BookingCodeSelector<D>::revalidateBookingCodes(FarePath& farePath, Itin& est)
{
  for (const auto pu : farePath.pricingUnit())
  {
    for (const auto fu : pu->fareUsage())
    {
      PaxTypeFare* ptf = fu->paxTypeFare();
      FareMarket* fm = ptf->fareMarket();

      FareBookingCodeValidator fbcv(_context.trx, *fm, &est);

      // Use PTF, since FU validation is broken.
      if (!fbcv.validate(*fu->paxTypeFare(), &farePath))
        return false;

      fu->copyBkgStatusFromPaxTypeFare();
    }
  }

  bool result = true;
  if (!fallback::fallbackEnableFamilyLogicAvailiablityValidationInBfa(&_context.trx) &&
      _context.trx.isBRAll())
  {
    FarePathValidator fpValidator(_context.trx, _context.motherItin);
    result = fpValidator.isFarePathValidMotherSolution(&farePath);
  }
  return result;
}

template <typename D>
bool
BookingCodeSelector<D>::validateMixedClass(FarePath& farePath)
{
  MixedClassController mxc(_context.trx);

  for (const auto pu : farePath.pricingUnit())
  {
    for (const auto fu : pu->fareUsage())
    {
      const auto plusUp = fu->differentialAmt();
      farePath.plusUpAmount() -= plusUp;
      farePath.decreaseTotalNUCAmount(plusUp);
    }

    if (!mxc.validate(farePath, *pu))
      return false;
  }
  return true;
}

template class BookingCodeSelector<DiagnosticWrapper>;
template class BookingCodeSelector<NoDiagnostic>;
}
}
