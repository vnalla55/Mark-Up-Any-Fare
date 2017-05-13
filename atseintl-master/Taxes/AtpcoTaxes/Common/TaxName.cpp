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

#include "Common/TaxName.h"

#include <tuple>

namespace tax
{

// Tax remittance id not included in this comparison as it should be constant for a given
// (taxCode, taxType) pair.
bool
TaxName::
operator<(TaxName const& other) const
{
  return std::tie(_nation, _taxCode, _taxType) <
         std::tie(other._nation, other._taxCode, other._taxType);
}

bool
TaxName::
operator==(TaxName const& other) const
{
  return (_taxCode == other._taxCode && _taxType == other._taxType && _nation == other._nation);
}

} // tax
