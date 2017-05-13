// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Rules/GeoUtils.h"

namespace tax
{
namespace GeoUtils
{

const type::Nation USA ("US");
const type::Nation GUAM ("GU");
const type::Nation AMERICAN_SAMOA ("AS");
const type::Nation CANTON_AND_ENDERBURY ("CT");
const type::Nation MIDWAY ("MI");
const type::Nation WAKE ("WK");
const type::Nation MICRONESIA ("FM");
const type::Nation SAIPAN ("MP");
const type::Nation NORTHERN_MARIANA ("MH");
const type::Nation PUERTO_RICO ("PR");
const type::Nation VIRGIN_ISLANDS("VI");

bool
isUSTerritory(const type::Nation& nation)
{
  static type::Nation usTeritories[] = { USA,              AMERICAN_SAMOA, CANTON_AND_ENDERBURY,
                                         GUAM,             MICRONESIA,     MIDWAY,
                                         NORTHERN_MARIANA, SAIPAN,         WAKE,
                                         VIRGIN_ISLANDS,   PUERTO_RICO};
  return (std::find(std::begin(usTeritories), std::end(usTeritories),
                    nation) != std::end(usTeritories));
}

bool
isHawaii(const type::StateProvinceCode& stateProvinceCode)
{
  return (stateProvinceCode == HAWAII);
}

bool
isAlaska(const type::StateProvinceCode& stateProvinceCode)
{
  return (stateProvinceCode == ALASKA);
}

} // namespace GeoUtils
} // namespace tax
