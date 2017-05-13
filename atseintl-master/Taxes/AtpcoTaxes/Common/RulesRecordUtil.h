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

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"

namespace tax
{

struct RulesRecord;

class RulesRecordUtil
{
public:
  RulesRecordUtil() = delete;

  static TaxableUnitTagSet
  narrowDownTuts(const TaxableUnitTagSet& taxableUnits,
                 type::ProcessingGroup processignGroup);

  static type::MoneyAmount
  getTaxAmt(const RulesRecord& rulesRecord);

  static std::string
  getVendorFullStr(const type::Vendor& vendor);
};

} /* namespace tax */
