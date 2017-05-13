//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#pragma once

#include <vector>

namespace tse
{
class RoutingMap;

namespace RoutingDbValidator
{
// return false if a routing map is incorrect:
// - contains incorrectly defined edges
// - contains a cycle
bool validate(const std::vector<RoutingMap*>& map);
}
}
