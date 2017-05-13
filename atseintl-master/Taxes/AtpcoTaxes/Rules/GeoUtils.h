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
#pragma once

#include <algorithm>
#include "DataModel/Common/Types.h"

namespace tax
{
namespace GeoUtils
{

const type::StateProvinceCode ALASKA = "USAK";
const type::StateProvinceCode HAWAII = "USHI";

bool
isUSTerritory(const type::Nation& nation);
bool
isHawaii(const type::StateProvinceCode& stateProvinceCode);
bool
isAlaska(const type::StateProvinceCode& stateProvinceCode);

} // namespace GeoUtils
} // namespace tax

