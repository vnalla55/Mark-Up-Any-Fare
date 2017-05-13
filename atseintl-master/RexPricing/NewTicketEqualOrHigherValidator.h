//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/Money.h"

#include <memory>

namespace tse
{

class RexBaseTrx;
class ProcessTagPermutation;
class Diag689Collector;
class FarePath;

class NewTicketEqualOrHigherValidator
{
  friend class NewTicketEqualOrHigherValidatorTest;

public:
  NewTicketEqualOrHigherValidator(RexBaseTrx& trx,
                                  FarePath& farePath,
                                  Diag689Collector* dc)
    : _trx(trx),
      _farePath(getFarePath(farePath)),
      _diag(dc),
      _epsilon(getEpsilon()),
      _newBaseAmount(getNewTicketAmount()),
      _newNonrefAmount(getNewNonrefTicketAmount()),
      _excNonrefAmount(getExcNonrefTicketAmount()),
      _stateCache(new ValidationStateCache),
      _isNetFarePath(&_farePath != &farePath)
  { }

  void setDiagnostic(Diag689Collector* dc) { _diag = dc; }

  bool match(const ProcessTagPermutation& perm) const;

private:
  struct ValidationStateCache
  {
    struct Status
    {
      Status() : isDetermined(false), value(false) {}
      bool determine(bool state)
      {
        isDetermined = true;
        return value = state;
      }
      bool isDetermined, value;
    } baseFareStatus, nonrefFareStatus;
  };

  bool isDiagnostic() const;
  Money getNewTicketAmount() const;
  Money getNewNonrefTicketAmount() const;
  Money getExcNonrefTicketAmount() const;
  bool matchImpl(const ProcessTagPermutation& perm) const;
  bool isNewTicketAmountEqualOrHigher() const;
  bool isNewNonrefTicketAmountEqualOrHigher(bool isSubtractForNonref) const;
  const FarePath& getFarePath(FarePath& farePath) const;
  MoneyAmount getEpsilon() const;
  Money convert(const Money& source, const CurrencyCode& targetCurr) const;
  Money convert(const MoneyAmount source, const CurrencyCode& targetCurr) const
  {
      return convert(Money(source, NUC), targetCurr);
  }


  RexBaseTrx& _trx;
  const FarePath& _farePath;
  Diag689Collector* _diag;
  const MoneyAmount _epsilon;
  Money _newBaseAmount;
  Money _newNonrefAmount;
  Money _excNonrefAmount;
  std::unique_ptr<ValidationStateCache> _stateCache;
  const bool _isNetFarePath;
};
} // tse
