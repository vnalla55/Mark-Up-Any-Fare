// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "DataModel/DiversityModelType.h"

namespace tse
{

DiversityModelType::Enum
DiversityModelType::parse(const std::string& str)
{
  if (str == "BASIC")
    return BASIC;
  if (str == "ALTDATES")
    return ALTDATES;
  if (str == "PRICE")
    return PRICE;
  if (str == "V2")
    return V2;
  if (str == "EFA")
      return EFA;
  return DEFAULT;
}

std::string
DiversityModelType::getName(Enum dmt)
{
  switch (dmt)
  {
  case BASIC:
    return "BASIC";
  case ALTDATES:
    return "ALTDATES";
  case PRICE:
    return "PRICE";
  case V2:
    return "V2";
  case EFA:
    return "EFA";
  case DEFAULT:
    return "DEFAULT";
  default:
    return "DEFAULT";
  }
}
}
