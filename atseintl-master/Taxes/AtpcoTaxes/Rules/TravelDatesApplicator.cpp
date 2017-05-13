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
#include "Rules/TravelDatesApplicator.h"

#include <algorithm>
#include <map>

namespace tax
{

bool
TravelDatesTaxPointApplicator::apply(PaymentDetail& paymentDetail) const
{
  type::Index taxPointId(paymentDetail.getTaxPointBegin().id());
  type::Date dateAtPoint(_itin.dateAtTaxPoint(taxPointId));
  bool result = true;
  if (dateAtPoint.is_blank_date()) // the exact date could not be computed, as in an
  // open segment or ARUNK.
  {
    dateAtPoint = _itin.firstDateAtTaxPoint(taxPointId);
    type::Date lastDateAtPoint(_itin.lastDateAtTaxPoint(taxPointId));
    if (dateAtPoint.is_blank_date()) // no earlier non-open segment - use ticketing date
      dateAtPoint = _saleDate;
    if (lastDateAtPoint.is_blank_date()) // no later non-open segment - use ticketing date + 2 years
      lastDateAtPoint = dateAtPoint.advance(730);
    result = dateAtPoint.is_between(_firstDate, _lastDate) && lastDateAtPoint.is_between(_firstDate, _lastDate);
  }
  else
  {
    result = dateAtPoint.is_between(_firstDate, _lastDate);
  }
  return result;
}
}
