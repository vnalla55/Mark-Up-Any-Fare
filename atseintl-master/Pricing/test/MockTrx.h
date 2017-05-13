//-------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#ifndef MOCK_TRX_H
#define MOCK_TRX_H

#include "DataModel/PricingTrx.h"

namespace tse
{
class MockTrx : public PricingTrx
{
public:
  MockTrx();
  ~MockTrx();

  void addFareMarket(FareMarket* fareMarket);
};
}
#endif
