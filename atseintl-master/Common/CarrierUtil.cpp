//-------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/CarrierUtil.h"

#include "DataModel/PricingTrx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"

namespace tse
{

namespace CarrierUtil
{
static bool
carrierAllianceMatchImpl(const CarrierCode& cxr,
                         const CarrierCode& restriction,
                         const PricingTrx& trx)
{
  typedef std::vector<AirlineAllianceCarrierInfo*> AllianceCarrierInfoVector;

  const AllianceCarrierInfoVector& allianceOptCxrInfoVec =
      trx.dataHandle().getAirlineAllianceCarrier(cxr);

  if (allianceOptCxrInfoVec.empty())
    return false;

  const AirlineAllianceCarrierInfo* segOptCxrAllianceInfo = allianceOptCxrInfoVec[0];
  return segOptCxrAllianceInfo->genericAllianceCode() == restriction;
}

bool
carrierAllianceMatch(const CarrierCode& cxr, const CarrierCode& restriction, const PricingTrx& trx)
{
  if (UNLIKELY(restriction.empty()))
    return true;

  if (!isAllianceCode(restriction))
    return false;

  return carrierAllianceMatchImpl(cxr, restriction, trx);
}

bool
carrierExactOrAllianceMatch(const CarrierCode& cxr,
                            const CarrierCode& restriction,
                            const PricingTrx& trx)
{
  if (restriction.empty())
    return true;

  return isAllianceCode(restriction) ? carrierAllianceMatchImpl(cxr, restriction, trx)
                                     : (cxr == restriction);
}

} // ns CarrierUtil
} // ns tse
