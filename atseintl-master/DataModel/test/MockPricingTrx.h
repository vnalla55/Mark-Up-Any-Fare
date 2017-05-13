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
#ifndef MOCKPRICINGTRX_H
#define MOCKPRICINGTRX_H

#include "gmock/gmock.h"
#include "DataModel/PricingTrx.h"

namespace tse
{

class MockPricingTrx : public PricingTrx
{
public:
  MOCK_METHOD0(ticketingDate, DateTime&());
  MOCK_CONST_METHOD0(ticketingDate, const DateTime&());
};

} // namespace tse

#endif // MOCKPRICINGTRX_H
