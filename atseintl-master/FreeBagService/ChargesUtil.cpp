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

#include "FreeBagService/ChargesUtil.h"

#include "Common/FreeBaggageUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/Itin.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"

#include <boost/lexical_cast.hpp>

namespace tse
{
namespace ChargesUtil
{
bool
isOnlineNonCodeShare(const Itin& itin)
{
  CarrierCode cxr;

  for (const TravelSeg* ts : itin.travelSeg())
  {
    const AirSeg* as = ts->toAirSeg();

    if (!as)
      continue;
    if (cxr.empty())
      cxr = as->carrier();
    if (cxr != as->carrier() || cxr != as->operatingCarrierCode())
      return false;
  }
  return true;
}

static bool
isNumeric(const ServiceGroupDescription& description)
{
  try { boost::lexical_cast<short>(description); }
  catch (boost::bad_lexical_cast&) { return false; }
  return true;
}

std::vector<const SubCodeInfo*>
retrieveS5(const PricingTrx& trx, const Itin& itin, const CarrierCode cxr)
{
  const bool isOnline = isOnlineNonCodeShare(itin);
  const auto filter = [&](const SubCodeInfo* s5)
  {
    return (isOnline || s5->industryCarrierInd() == 'I') &&
           s5->fltTktMerchInd() == BAGGAGE_CHARGE && s5->serviceSubGroup().empty() &&
           isNumeric(s5->description1());
  };

  std::vector<const SubCodeInfo*> subCodes;
  FreeBaggageUtil::S5RecordsRetriever(filter, cxr, trx).get(subCodes);
  return subCodes;
}

void
selectCheaper(BaggageCharge*& target, BaggageCharge& newCharge)
{
  if (!target || newCharge.feeAmount() < target->feeAmount())
    target = &newCharge;
}

void
selectForPricing(BaggageCharge*& target, BaggageCharge& newCharge)
{
  if (!target)
    target = &newCharge;
  else if (target->optFee()->notAvailNoChargeInd() == 'X')
    return;
  else if (newCharge.optFee()->notAvailNoChargeInd() == 'X' ||
           newCharge.feeAmount() < target->feeAmount())
    target = &newCharge;
}
}
}
