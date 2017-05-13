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

#include "Common/Timestamp.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

struct ReportingRecordEntry
{
  type::TaxLabel taxLabel;
};

struct ReportingRecord
{
  typedef ReportingRecordEntry entry_type;

  ReportingRecord() :
    isVatTax(false),
    isCommissionable(false),
    isInterlineable(false),
    taxOrChargeTag(type::TaxOrChargeTag::Blank),
    accountableDocTag(type::AccountableDocTag::Blank),
    refundableTag(type::RefundableTag::Blank),
    reportingTextItemNo(0),
    taxTextItemNo(0),
    taxApplicableToItemNo(0),
    taxRateItemNo(0),
    taxExemptionsItemNo(0),
    taxCollectRemitItemNo(0),
    taxingAuthorityItemNo(0),
    taxCommentsItemNo(0),
    taxSpecialInstructionsItemNo(0) {}

  type::Vendor vendor;
  type::Nation nation;
  type::CarrierCode taxCarrier;
  type::TaxCode taxCode;
  type::TaxType taxType;
  type::Timestamp effDate;
  type::Timestamp discDate;
  bool isVatTax;
  bool isCommissionable;
  bool isInterlineable;
  type::TaxOrChargeTag taxOrChargeTag;
  type::AccountableDocTag accountableDocTag;
  type::RefundableTag refundableTag;
  type::Index reportingTextItemNo;
  type::Index taxTextItemNo;
  type::Index taxApplicableToItemNo;
  type::Index taxRateItemNo;
  type::Index taxExemptionsItemNo;
  type::Index taxCollectRemitItemNo;
  type::Index taxingAuthorityItemNo;
  type::Index taxCommentsItemNo;
  type::Index taxSpecialInstructionsItemNo;

  boost::ptr_vector<entry_type> entries;
};

} // namespace tax
