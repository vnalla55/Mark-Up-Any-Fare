//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "Common/TsePrimitiveTypes.h"

#include <boost/logic/tribool.hpp>

namespace tse
{
class SavedBaggageCharge
{
public:
  bool processed() const { return !boost::logic::indeterminate(_isValid); }
  bool valid() const { return  _isValid; }
  MoneyAmount amount() const { return _amount; }

  void setInvalid()
  {
    _isValid = false;
    _amount = 0;
  }
  void setAmount(MoneyAmount amt)
  {
    _isValid = true;
    _amount = amt;
  }

private:
  boost::logic::tribool _isValid = boost::logic::indeterminate;
  MoneyAmount _amount = 0;
};
}
