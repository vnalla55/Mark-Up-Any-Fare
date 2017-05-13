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
#ifndef MOCKMILAGESURCHARGEEXCEPTION_H
#define MOCKMILAGESURCHARGEEXCEPTION_H

#include <gmock/gmock.h>
#include "Routing/MileageSurchargeException.h"

namespace tse
{

class MockMileageSurchargeException : public MileageSurchargeException
{
public:
  MOCK_CONST_METHOD4(
      isExceptionApplicable,
      bool(const MileageSurchExcept&, const PaxTypeFare&, const TravelRoute&, PricingTrx&));
  MOCK_CONST_METHOD2(getData, std::vector<MileageSurchExcept*>(const PaxTypeFare&, PricingTrx&));
};

} // namespace tse

#endif // MOCKMILAGESURCHARGEEXCEPTION_H
