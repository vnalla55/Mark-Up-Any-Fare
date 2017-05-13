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

#include "Common/Timestamp.h"

namespace tax
{

class BusinessRulesContainer;
class RulesRecord;

bool
validBusinessRuleDatePredicate(const std::shared_ptr<tax::BusinessRulesContainer>& rule,
                               type::Timestamp ticketingDate);
bool
validRulesRecordDatePredicate(const RulesRecord& rule,
                              type::Timestamp ticketingDate);
bool
validTravelDatePredicate(const RulesRecord& rule,
                         const type::Date travelDate);
bool
validDatePredicate(const type::Date& effectiveDate,
                   const type::Date& discDate,
                   const type::Timestamp& expireDate,
                   const type::Timestamp& ticketingDate);

} // namespace tax
