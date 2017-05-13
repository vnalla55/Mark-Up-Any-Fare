//-------------------------------------------------------------------
//
//  File:        RefundPenalty.h
//  Created:     September 16, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Money.h"


#include <map>
#include <vector>

namespace tse
{

class FareUsage;

class RefundPenalty
{
public:
  enum Scope
  {
    FC,
    PU
  };

  struct Fee
  {
    Fee() : _amount("NUC"), _discount(false), _nonRefundable(false), _highest(false) {}

    Fee(const Money& m, bool discount = false, bool nonRefundable = false)
      : _amount(m), _discount(discount), _nonRefundable(nonRefundable), _highest(false)
    {
    }

    const Money& amount() const { return _amount; }
    bool nonRefundable() const { return _nonRefundable; }
    bool discount() const { return _discount; }
    bool highest() const { return _highest; }
    bool& highest() { return _highest; }

  protected:
    Money _amount;
    bool _discount;
    bool _nonRefundable;
    bool _highest;
  };

  RefundPenalty() : _scope(FC) {}

  void assign(const std::vector<Fee>& fee, Scope scope)
  {
    _fee = fee;
    _scope = scope;
  }

  bool isRefundable(const std::vector<FareUsage*>& fareUsage) const
  {
    if (_fee.size() == 1)
    {
      if (!_fee.at(0).nonRefundable())
      {
        return true;
      }
      if (_scope == FC)
      {
        return (fareUsage.size() > 1);
      }
      return false;
    }
    return !allNonRefundable();
  }

  const std::vector<Fee>& fee() const { return _fee; }
  std::vector<Fee>& fee() { return _fee; }
  Scope scope() const { return _scope; }
  bool isPuScope() const { return _scope == PU; }
  bool isFcScope() const { return _scope == FC; }

protected:
  std::vector<Fee> _fee;
  Scope _scope;

private:
  bool allNonRefundable() const
  {
    return std::find_if(_fee.begin(),
                        _fee.end(),
                        std::not1(std::mem_fun_ref<bool>(&Fee::nonRefundable))) == _fee.end();
  }
};
}

