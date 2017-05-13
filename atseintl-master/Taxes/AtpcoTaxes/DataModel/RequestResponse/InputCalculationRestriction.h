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

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Common/Types.h"
#include "DataModel/RequestResponse/InputCalculationRestrictionTax.h"

namespace tax
{
class InputCalculationRestriction
{
public:
  InputCalculationRestriction() : _restrictionType(type::CalcRestriction::Blank) {};

  type::CalcRestriction& restrictionType() { return _restrictionType; }

  const type::CalcRestriction& restrictionType() const { return _restrictionType; }

  boost::ptr_vector<InputCalculationRestrictionTax>& calculationRestrictionTax()
  {
    return _calculationRestrictionTax;
  };

  const boost::ptr_vector<InputCalculationRestrictionTax>& calculationRestrictionTax() const
  {
    return _calculationRestrictionTax;
  };

private:
  boost::ptr_vector<InputCalculationRestrictionTax> _calculationRestrictionTax;
  type::CalcRestriction _restrictionType;
};

} // namespace tax
