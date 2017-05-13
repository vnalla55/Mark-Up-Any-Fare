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
#include <boost/date_time/posix_time/posix_time.hpp>
#include "DataModel/Common/Types.h"

namespace tax
{

class InputTicketingOptions
{
public:
  InputTicketingOptions()
    : _formOfPayment(type::FormOfPayment::Blank),
      _ticketingDate(boost::gregorian::not_a_date_time),
      _ticketingTime(boost::posix_time::not_a_date_time) {};

  type::CurrencyCode& paymentCurrency() { return _paymentCurrency; }
  const type::CurrencyCode& paymentCurrency() const { return _paymentCurrency; }

  type::FormOfPayment& formOfPayment() { return _formOfPayment; }
  const type::FormOfPayment& formOfPayment() const { return _formOfPayment; }

  type::AirportCode const& ticketingPoint() const
  {
    return _ticketingPoint;
  };
  type::AirportCode& ticketingPoint()
  {
    return _ticketingPoint;
  };

  boost::gregorian::date& ticketingDate() { return _ticketingDate; }
  const boost::gregorian::date& ticketingDate() const { return _ticketingDate; }

  boost::posix_time::time_duration& ticketingTime() { return _ticketingTime; }
  const boost::posix_time::time_duration& ticketingTime() const { return _ticketingTime; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::CurrencyCode _paymentCurrency;
  type::FormOfPayment _formOfPayment;
  type::AirportCode _ticketingPoint;
  boost::gregorian::date _ticketingDate;
  boost::posix_time::time_duration _ticketingTime;
};

} // namespace tax

