// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/DateTime.h"

namespace tse
{
class PricingTrx;

class GetTicketingDate
{
public:
  explicit GetTicketingDate(PricingTrx& trx);
  const DateTime& get() const;

private:
  DateTime _date;
};

} // end of tse namespace
