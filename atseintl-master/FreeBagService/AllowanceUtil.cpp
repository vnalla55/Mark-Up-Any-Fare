// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "FreeBagService/AllowanceUtil.h"

#include "Common/Assert.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Itin.h"
#include "DBAccess/SubCodeInfo.h"

#include <algorithm>

namespace tse
{
class BaggageTravel;

namespace AllowanceUtil
{
bool
DotTableChecker::operator()(const TravelSeg* travelSeg) const
{
  const AirSeg* as = travelSeg->toAirSeg();

  if (!as)
    return false;

  if (_btt.usDotCarrierApplicable() &&
      !_trx.dataHandle().isCarrierUSDot(as->carrier()))
    return false;

  if (_btt.ctaCarrierApplicable() &&
      !_trx.dataHandle().isCarrierCTA(as->carrier()))
    return false;

  return true;
}

inline CarrierCode
getDeferTargetCxrUsDot(const BaggageTravel& bt)
{
  const TravelSeg* const ts = *bt._MSSJourney;
  TSE_ASSERT(ts);
  const AirSeg* const as = ts->toAirSeg();
  return as ? as->marketingCarrierCode() : CarrierCode();
}

CarrierCode
getAllowanceCxrNonUsDot(const PricingTrx& trx, const AirSeg& as)
{
  return (as.segmentType() == Open || TrxUtil::isIataReso302MandateActivated(trx))
             ? as.carrier()
             : as.operatingCarrierCode();
}

CarrierCode
getDeferTargetCxrNonUsDot(const PricingTrx& trx, const AirSeg& as)
{
  if (as.segmentType() == Open)
    return CarrierCode();
  return TrxUtil::isIataReso302MandateActivated(trx) ? as.operatingCarrierCode() : as.carrier();
}

CarrierCode
getDeferTargetCxr(const BaggageTravel& bt)
{
  TSE_ASSERT(bt.itin());
  const BaggageTripType btt = bt.itin()->getBaggageTripType();

  if (btt.isWhollyWithinUsOrCa())
    return CarrierCode();

  if (btt.isUsDot())
    return getDeferTargetCxrUsDot(bt);

  TSE_ASSERT(bt._carrierTravelSeg);
  const AirSeg* const as = bt._carrierTravelSeg->toAirSeg();
  return as ? getDeferTargetCxrNonUsDot(*bt._trx, *as) : CarrierCode();
}

static bool
isAllowanceS5(const SubCodeInfo* const s5)
{
  const bool isPermittedRfi = (s5->rfiCode() == 'C' || s5->rfiCode() == ' ');
  return s5->serviceSubTypeCode().equalToConst("0DF") && s5->fltTktMerchInd() == BAGGAGE_ALLOWANCE &&
         s5->concur() == 'X' && isPermittedRfi && s5->ssimCode() == ' ' &&
         s5->ssrCode().empty() && s5->emdType() == '4' && s5->bookingInd().empty();
}

static const SubCodeInfo*
retrieveS5(const PricingTrx& trx, const VendorCode vendor, const CarrierCode cxr)
{
  const std::vector<SubCodeInfo*>& s5items = FreeBaggageUtil::retrieveS5Records(vendor, cxr, trx);
  const auto it = std::find_if(s5items.begin(), s5items.end(), isAllowanceS5);
  return (it != s5items.end()) ? *it : nullptr;
}

const SubCodeInfo*
retrieveS5(const PricingTrx& trx, const CarrierCode cxr)
{
  const SubCodeInfo* const s5 = retrieveS5(trx, ATPCO_VENDOR_CODE, cxr);
  return s5 ? s5 : retrieveS5(trx, MERCH_MANAGER_VENDOR_CODE, cxr);
}

}
}
