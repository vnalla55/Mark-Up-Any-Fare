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
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/TicketingOptions.h"

namespace tax
{

TicketingOptions::TicketingOptions()
  : _formOfPayment(type::FormOfPayment::Blank),
    _ticketingDate(type::Date::blank_date()),
    _ticketingTime(type::Time::blank_time())
{
}

std::ostream&
TicketingOptions::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /*= ' ' */)
    const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "PAYMENTCURRENCY: " << _paymentCurrency << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "FORMOFPAYMENT: " << _formOfPayment << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "TICKETINGPOINT: " << _ticketingPoint << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "TICKETINGDATE: " << _ticketingDate << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "TICKETINGTIME: " << _ticketingTime << "\n";
  return out;
}

} // namespace tax
