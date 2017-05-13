//---------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#ifndef TAXES_COMMON_TAXMILEAGEUTIL_H
#define TAXES_COMMON_TAXMILEAGEUTIL_H

#include <boost/optional.hpp>
#include <stdint.h>
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class DataHandle;
class DateTime;
class Loc;

class TaxMileageUtil
{
public:
  static uint32_t getDistance(const Loc& origin,
                              const Loc& destination,
                              const GlobalDirection globalDirection,
                              const DateTime& dateTime,
                              DataHandle& dataHandle);

  static uint32_t getGCMDistance(const Loc& origin,
                                 const Loc& destination);
private:
  static boost::optional<uint32_t> getMileageWithSubstitution(const LocCode& city1,
                                                              const LocCode& city2,
                                                              const GlobalDirection& globalDir,
                                                              const DateTime& dateTime,
                                                              const Indicator mileageType,
                                                              DataHandle& dataHndle);
  static boost::optional<uint32_t> getMileage(const LocCode& city1,
                                              const LocCode& city2,
                                              const GlobalDirection& globalDir,
                                              const DateTime& dateTime,
                                              const Indicator mileageType,
                                              DataHandle& dataHndle);
};
}

#endif
