//----------------------------------------------------------------------------
//  File:           LocationDescriptionUtil.h
//  Description:    LocationDescriptionUtil header file for ATSE International Project
//  Created:        6/25/2007
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef LOCATION_DESCRIPTION_UTIL_H
#define LOCATION_DESCRIPTION_UTIL_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class TaxTrx;

class LocationDescriptionUtil
{

public:
  static std::string description(TaxTrx& taxTrx, LocType locType, LocCode locCode);

protected:
private:
  static std::string nationDescription(TaxTrx& taxTrx, LocCode nationCode);

  static std::string zoneDescription(TaxTrx& taxTrx,
                                     LocCode zoneCode,
                                     const VendorCode& vendorCode = Vendor::SABRE,
                                     const ZoneType& zoneType = MANUAL);

  static std::string zoneDecode(TaxTrx& taxTrx, const ZoneInfo* zoneInfo, bool isExcl);

  static std::string areaDescription(TaxTrx& taxTrx, LocCode areaCode);

  static std::string subAreaDescription(TaxTrx& taxTrx, LocCode subAreaCode);
};
} // namespace tse
#endif // LOCATION_DESCRIPTION_UTIL_H
