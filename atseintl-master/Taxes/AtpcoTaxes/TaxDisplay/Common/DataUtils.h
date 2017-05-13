// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Common/Types.h"

namespace tax
{

class LocService;
class TaxDisplayRequest;

namespace display
{

bool getNationCode(const TaxDisplayRequest& request,
                   const LocService& locService,
                   type::Nation& nationCode);

bool getNationName(const type::Nation& requestNationCode,
                   const type::NationName& requestNationName,
                   const LocService& locService,
                   type::NationName& nationName);

} // namespace display
} // namespace tax
