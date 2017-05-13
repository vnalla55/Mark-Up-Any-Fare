//-------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "ServiceFees/AncillaryWpDisplayValidator.h"

#include "Common/Logger.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/PricingTrx.h"
#include "ServiceFees/OCFees.h"

using namespace std;

namespace tse
{

static Logger
logger("atseintl.ServiceFees.AncillaryWpDisplayValidator");

bool
AncillaryWpDisplayValidator::setPBDate(const OptionalServicesInfo& optSrvInfo,
                                       OCFees& ocFees,
                                       const DateTime& pbDate) const
{
  LOG4CXX_DEBUG(logger, "Entered AncillaryWpDisplayValidator::setPBDate()");
  if (isAdvPurUnitNHDM(optSrvInfo))
  {
    if (optSrvInfo.advPurchUnit() == ServicePurchaseUnit('D'))
    {
      uint32_t days = 0;
      istringstream stream(optSrvInfo.advPurchPeriod());
      stream >> days;
      if (days <= 0)
      {
        ocFees.purchaseByDate() = pbDate;
        return true;
      }
      ocFees.purchaseByDate() = (*_segI)->departureDT().subtractDays(days);
      return true;
    }
    else if ((optSrvInfo.advPurchUnit() == ServicePurchaseUnit('M')))
    {
      int32_t numberOfMonths = 0;
      istringstream stream(optSrvInfo.advPurchPeriod());
      stream >> numberOfMonths;
      if (numberOfMonths < 0)
        numberOfMonths = 0;
      ocFees.purchaseByDate() = (*_segI)->departureDT().subtractMonths(numberOfMonths);
      return true;
    }
    boost::posix_time::time_duration count = _trx.ticketingDate() - pbDate;
    if (count.is_negative())
      count = count.invert_sign();

    ocFees.purchaseByDate() = (*_segI)->departureDT() - count;
  }
  else
  {
    uint32_t calcDays = 0;
    int advPinDays = 0;
    getAdvPurPeriod(optSrvInfo, (*_segI)->departureDT(), calcDays, advPinDays);

    if (calcDays == 0 && advPinDays == 0)
      calcDays = 7;
    calcDays += advPinDays;
    ocFees.purchaseByDate() = (*_segI)->departureDT().subtractDays(calcDays);
  }
  return true;
}

bool
AncillaryWpDisplayValidator::checkInterlineIndicator(const OptionalServicesInfo& optSrvInfo) const
{
  //[EMD] add code for Interline Indicator or remove this method.
  return true;
}

}
