//-------------------------------------------------------------------
//
//  File:        BrandInfo.cpp
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "BrandedFares/BrandInfo.h"

#include "DataModel/PricingTrx.h"

namespace tse
{
BrandInfo::BrandInfo() : _tier(0), _primaryFareIdTable(0), _secondaryFareIdTable(0), _carrierFlightItemNum(0) {}

bool
BrandInfo::collectPrimaryFareIDdata(PricingTrx& trx, long long fareIDdataPrimaryT189, VendorCode vc)
{
  _fareIDdataPrimaryT189 = trx.dataHandle().getSvcFeesFareIds(vc, fareIDdataPrimaryT189);
  return true;
}

bool
BrandInfo::collectSecondaryFareIDdata(PricingTrx& trx,
                                      long long fareIDdataSecondaryT189,
                                      VendorCode vc)
{
  _fareIDdataSecondaryT189 = trx.dataHandle().getSvcFeesFareIds(vc, fareIDdataSecondaryT189);
  return true;
}
}
