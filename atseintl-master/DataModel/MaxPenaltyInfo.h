//----------------------------------------------------------------------------
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

#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include <boost/optional.hpp>
#include <iosfwd>

namespace tse
{

struct MaxPenaltyInfo
{
  struct Filter
  {
    smp::RecordApplication _departure;

    boost::optional<smp::ChangeQuery> _query;

    boost::optional<Money> _maxFee;
  };

  smp::Mode _mode;

  Filter _changeFilter;

  Filter _refundFilter;
};

struct MaxPenaltyResponse
{
  struct Fee
  {
    Fee() : _non(true) {}
    Fee(const Money& fee) : _fee(fee), _non(false) {}
    Fee(boost::optional<Money> fee, bool non) : _fee(fee), _non(non) {}

    // actual penalty fee; if not set ticket is nonchangeable/nonrefundable
    boost::optional<Money> _fee;

    // true if ticket is nonchangeable/nonrefundable
    bool _non;

    // vector holding every fare that was specified as "missing data"
    std::vector<const FareInfo*> _missingDataVec;

    Fee& operator+=(const Fee& fee);

    void setDefaultFee(const CurrencyCode& currencyCode);
    bool isFullyNon() const { return !_fee && _non; }
    void setFullyNon();
    bool isMissingData() const { return !_missingDataVec.empty(); }

    void finalRounding(PricingTrx& trx);
  };

  struct Fees
  {
    Fee _before;
    Fee _after;

    // true when penalty comes from category 16 or has missing data
    // if any _fee value is present and c16 == true then it means that the penalty is an otherwise
    // if no _fee is present and c16 == true then it means that there are missing records
    // and there will be fares posted inside of current penalty (chg bef/chg aft/etc)
    //
    // when the fees is added the fee isinitialized as (0, false) to correctly be validated
    smp::RecordApplication _cat16 = smp::INVALID;

    // When the fees struct leaves the Cat16Calculator the missingData is seen as (--, false)
    // It is only later in MaximumPenaltyCalc that fares are being populated into the mdt vector
    // Afterwards missingData can be deducted only upon the insides of the missingDataVec, because
    // missingData is changed from (--,false) to (0,false)
    smp::RecordApplication missingDataInsideCalc() const
    {
      return (_cat16 & ((!_before._fee && !_before._non ? smp::BEFORE : smp::INVALID) |
                        (!_after._fee && !_after._non ? smp::AFTER : smp::INVALID)));
    }

    Fees& operator+=(const Fees& fees);

    smp::RecordApplication calculateCat16Application(const smp::RecordApplication departure);

    void finalRounding(PricingTrx& trx);
    void setFullyNon();
  };

  Fees _changeFees;
  Fees _refundFees;
};

inline MaxPenaltyResponse::Fee
operator+(MaxPenaltyResponse::Fee lhs, const MaxPenaltyResponse::Fee& rhs)
{
  lhs += rhs;
  return lhs;
}

inline MaxPenaltyResponse::Fees
operator+(MaxPenaltyResponse::Fees lhs, const MaxPenaltyResponse::Fees& rhs)
{
  lhs += rhs;
  return lhs;
}

std::ostream&
operator<<(std::ostream& s, const tse::MaxPenaltyResponse::Fee& fee);

std::ostream&
operator<<(std::ostream& s, const tse::MaxPenaltyResponse::Fees& fees);

namespace smp
{
void
resultDiagnostic(DiagManager& diag,
                 const MaxPenaltyResponse::Fee& fee,
                 const char* chgRef,
                 const char* cat,
                 const char* afterBefore);

void
resultDiagnostic(DiagManager& diag,
                 const MaxPenaltyResponse::Fees& fees,
                 smp::RecordApplication departure,
                 const char* chgRef,
                 const char* cat);
}
} // tse

