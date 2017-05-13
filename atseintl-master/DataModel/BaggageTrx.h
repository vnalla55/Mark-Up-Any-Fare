#pragma once

#include "DataModel/PricingTrx.h"
#include "Service/Service.h"

namespace tse
{
class BaggageTrx : public PricingTrx
{
public:
  BaggageTrx();
  virtual bool process(Service& srv) override { return srv.process(*this); }
};
} // tse namespace
