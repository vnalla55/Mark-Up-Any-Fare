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

#ifndef MOCK_FARE_PATH_H
#define MOCK_FARE_PATH_H

#include "DataModel/FarePath.h"

namespace tse
{
class MockFarePath : public FarePath
{
public:
  // Make public for testing purposes
  MockFarePath();
  virtual ~MockFarePath();

  bool addPU(PricingUnit* pu);
};
}
#endif
