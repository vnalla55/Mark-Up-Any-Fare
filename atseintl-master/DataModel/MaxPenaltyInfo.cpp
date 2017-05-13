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

#include "Common/CurrencyRoundingUtil.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "Diagnostic/DiagManager.h"
#include <boost/optional/optional_io.hpp>
#include <ostream>

namespace tse
{
MaxPenaltyResponse::Fee&
MaxPenaltyResponse::Fee::operator+=(const Fee& fee)
{
  if (isFullyNon() || fee.isFullyNon())
  {
    setFullyNon();
  }
  else
  {
    if (fee._fee)
    {
      _fee = _fee.get() + fee._fee.get();
    }

    _non = _non || fee._non;
  }

  return *this;
}

smp::RecordApplication
MaxPenaltyResponse::Fees::calculateCat16Application(const smp::RecordApplication departure)
{
  smp::RecordApplication result = smp::INVALID;

  if ((departure & smp::BEFORE) && !_before._fee && !_before._non)
  {
    result |= smp::BEFORE;
  }
  if ((departure & smp::AFTER) && !_after._fee && !_after._non)
  {
    result |= smp::AFTER;
  }
  _cat16 = result;

  return result;
}

void
MaxPenaltyResponse::Fee::setDefaultFee(const CurrencyCode& currencyCode)
{
  _fee = Money(0., currencyCode);
}

void
MaxPenaltyResponse::Fee::setFullyNon()
{
  _fee = boost::none;
  _non = true;
}

void
MaxPenaltyResponse::Fee::finalRounding(PricingTrx& trx)
{
  if (_fee && _fee.get().value())
  {
    CurrencyRoundingUtil().round(_fee.get().value(), _fee.get().code(), trx);
  }
}

void
MaxPenaltyResponse::Fees::finalRounding(PricingTrx& trx)
{
  _before.finalRounding(trx);
  _after.finalRounding(trx);
}

void
MaxPenaltyResponse::Fees::setFullyNon()
{
  _before.setFullyNon();
  _after.setFullyNon();
}

MaxPenaltyResponse::Fees&
MaxPenaltyResponse::Fees::operator+=(const Fees& fees)
{
  _before += fees._before;
  _after += fees._after;
  return *this;
}

std::ostream& operator<<(std::ostream& s, const tse::MaxPenaltyResponse::Fee& fee)
{
  return s << '[' << fee._fee << ',' << fee._non << ']';
}

std::ostream& operator<<(std::ostream& s, const tse::MaxPenaltyResponse::Fees& fees)
{
  return s << '[' << fees._before << ',' << fees._after << ','
           << smp::printRecordApplication(fees._cat16) << ']';
}

namespace smp
{
void
resultDiagnostic(DiagManager& diag,
                 const MaxPenaltyResponse::Fee& fee,
                 const char* chgRef,
                 const char* cat,
                 const char* afterBefore)
{
  diag << "  " << chgRef << " PENALTY " << cat << ' ' << afterBefore << " DEPARTURE: " << fee._fee;
  if (fee._non)
  {
    diag << "/NON";
  }
  diag << '\n';
}

void
resultDiagnostic(DiagManager& diag,
                 const MaxPenaltyResponse::Fees& fees,
                 smp::RecordApplication departure,
                 const char* chgRef,
                 const char* cat)
{
  if (departure & smp::BEFORE)
  {
    resultDiagnostic(diag, fees._before, chgRef, cat, "BEFORE");
  }

  if (departure & smp::AFTER)
  {
    resultDiagnostic(diag, fees._after, chgRef, cat, "AFTER");
  }
}
}

} // tse


