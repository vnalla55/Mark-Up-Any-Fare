// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class TaxResponse;
class TaxItem;

class TaxOnTaxFilter
{
public:
  TaxOnTaxFilter()
  {}

  TaxOnTaxFilter(const DateTime& date)
  : _ticketingDT(date)
  {}

  TaxOnTaxFilter(const TaxOnTaxFilter&) = delete;

  bool
  isFilteredItem(const TaxResponse& taxResponse, const TaxItem& item) const;

  bool
  isFilteredSegment(const TravelSeg& seg, const TaxCode& code) const;

  TaxOnTaxFilter& operator =(const TaxOnTaxFilter& src);

  bool enable() const {return _enable; };

protected:
  bool _enable = false;
  LocTypeCode _loc1Type;
  LocCode _loc1Code;
  TaxCode _taxCode;
  DateTime _ticketingDT;
};

} // end of tse namespace
