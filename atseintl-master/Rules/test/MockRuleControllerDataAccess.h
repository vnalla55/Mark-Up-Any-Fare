// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifndef MOCK_RULE_CONTROLLER_DATA_ACCESS_H
#define MOCK_RULE_CONTROLLER_DATA_ACCESS_H

#include "Rules/RuleControllerDataAccess.h"

#include "gmock/gmock.h"

namespace tse
{

class MockRuleControllerDataAccess : public RuleControllerDataAccess
{
public:
  MOCK_METHOD0(itin, Itin*());
  MOCK_CONST_METHOD0(paxTypeFare, PaxTypeFare&());
  MOCK_METHOD0(trx, PricingTrx&());
  MOCK_METHOD0(getFareUsage, FareUsage*());
};

} // namespace tse

#endif
