// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

type::SabreTaxCode
makeItinSabreCode(type::TaxCode taxCode,
                  const type::TaxType& taxType,
                  const type::PercentFlatTag& percentFlatTag);
type::SabreTaxCode
makeServiceSabreCode(type::TaxCode taxCode);
}
