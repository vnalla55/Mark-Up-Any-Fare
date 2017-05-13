#include "Pricing/test/MockTrx.h"

using namespace tse;

MockTrx::MockTrx() {}

MockTrx::~MockTrx() {}

void
MockTrx::addFareMarket(FareMarket* fareMarket)
{
  if (fareMarket)
  {
    _fareMarket.push_back(fareMarket);
  }
}
