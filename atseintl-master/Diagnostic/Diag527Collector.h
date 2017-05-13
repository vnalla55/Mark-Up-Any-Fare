#pragma once

#include "Diagnostic/DiagCollector.h"

#include <string>

using std::string;

namespace tse
{
class Diag527Collector : public DiagCollector
{
public:
  explicit Diag527Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag527Collector() {}

  void displayFarePath(const FarePath& farePath);
  void displayPricingUnit(const PricingUnit& pricingUnit);
  void displayPaxTypeFare(const PaxTypeFare& paxTypeFare, PricingUnit::Type puType);

  void displayHeader();
  void displayPUHeader();
  void displayFPHeader();
  void displayStatus(Record3ReturnTypes status);

private:
  virtual void getTourCode(const PaxTypeFare& paxTypeFare, string& tourCode);
  virtual void getMoney(const PaxTypeFare& paxTypeFare, string& money);
};
}

