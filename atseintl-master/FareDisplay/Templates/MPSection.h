#pragma once

#include "FareDisplay/Templates/Section.h"

namespace tse
{

class FareDisplayTrx;

class MPSection : public Section
{
public:
  MPSection(FareDisplayTrx& trx) : Section(trx) {}

  void buildDisplay() override;

  static const uint16_t SurchargeRanges = 6;

protected:
  virtual void doBuildDisplay();

  typedef std::vector<uint32_t> MPMs;
  typedef std::map<GlobalDirection, MPMs> MPMsByGlobal;
  typedef std::vector<MoneyAmount> Amounts;

  void prepareMPMs(MPMsByGlobal&);
  void prepareAmounts(Amounts&);
  void displayHeader();
  void displayColumnHeaders();
  void displaySpace();
  void displayData(const MPMsByGlobal&, const Amounts&);
  void displayMPMs(const MPMs&);
  void displayAmounts(const Amounts&);
  void displayLine(const std::string&);
  void displayColumn(std::ostringstream&, uint16_t, const std::string&);
};
} // namespace tse
