// ----------------------------------------------------------------------------
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

#include <vector>

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"

namespace tax
{
struct PaymentWithRules;
class YqYrPath;

class TaxLimiter
{
  friend class TaxLimiterTest;
public:
  static void
  limitYqYrs(const YqYrPath& yqYrPath, std::vector<PaymentWithRules>& paymentsToCalculate);

  static void
  overlapItinerary(std::vector<PaymentWithRules>& paymentsToCalculate);

  static void
  overlapYqYrs(const type::Index yqYrCount, std::vector<PaymentWithRules>& paymentsToCalculate);

private:
  static std::vector<bool>
  limit(const std::vector<type::TaxApplicationLimit>& limits,
        const std::vector<type::MoneyAmount>& amounts);
};
}

