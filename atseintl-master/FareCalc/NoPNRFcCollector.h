#pragma once

#include "FareCalc/FcCollector.h"

namespace tse
{
class WarningMap;

namespace FareCalc
{

class NoPNRFcCollector : public FcCollector
{
public:
  using FcCollector::FcCollector;

private:
  void collectMessage() override;
  virtual void collectAirportSpecificMessages() {}
  virtual void collectTaxWarningMessages() {}

  virtual void collectRuleMessages();
};

} // namespace FareCalc
} // namespace tse

