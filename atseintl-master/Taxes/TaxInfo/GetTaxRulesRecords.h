//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <vector>
#include <utility>

namespace tse
{

class DataHandle;
class TaxCodeReg;
class TaxRulesRecord;

const std::vector<TaxCodeReg*>&
getTaxRulesRecords(std::vector<TaxCodeReg*>& out, DataHandle& dataHandle, TaxCode taxCode,
                   DateTime ticketingDate);

// for testing purposes 

struct TaxCodeAndType
{
  TaxCode code;
  TaxType type;
  explicit TaxCodeAndType(TaxCode cd, TaxCode tp) : code(cd), type(tp) {}
};

std::vector<TaxCodeAndType> translateToAtpcoCodes(TaxCode taxCode);

std::pair<DateTime, DateTime>
makeTravelRange(const TaxRulesRecord& rulesRec, const DateTime& tktDate);

} // namespace tse

