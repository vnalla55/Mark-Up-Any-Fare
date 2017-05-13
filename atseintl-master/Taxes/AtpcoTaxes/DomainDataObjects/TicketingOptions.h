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

#include <iosfwd>
#include "DataModel/Common/Types.h"
#include "Common/Timestamp.h"

namespace tax
{

class TicketingOptions
{
public:
  TicketingOptions();

  type::CurrencyCode& paymentCurrency() { return _paymentCurrency; }
  const type::CurrencyCode& paymentCurrency() const { return _paymentCurrency; }

  type::FormOfPayment& formOfPayment() { return _formOfPayment; }
  const type::FormOfPayment& formOfPayment() const { return _formOfPayment; }

  type::AirportCode& ticketingPoint() { return _ticketingPoint; }
  const type::AirportCode& ticketingPoint() const { return _ticketingPoint; }

  type::Date& ticketingDate() { return _ticketingDate; }
  const type::Date& ticketingDate() const { return _ticketingDate; }

  type::Time& ticketingTime() { return _ticketingTime; }
  const type::Time& ticketingTime() const { return _ticketingTime; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::CurrencyCode _paymentCurrency;
  type::FormOfPayment _formOfPayment;
  type::AirportCode _ticketingPoint;
  type::Date _ticketingDate;
  type::Time _ticketingTime;
};

} // namespace tax

