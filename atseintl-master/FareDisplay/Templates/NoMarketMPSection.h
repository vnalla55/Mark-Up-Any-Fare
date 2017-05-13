#pragma once

#include "FareDisplay/Templates/MPSection.h"

namespace tse
{

class NoMarketMPSection : public MPSection
{
public:
  NoMarketMPSection(FareDisplayTrx& trx) : MPSection(trx) {}

private:
  void doBuildDisplay() override;
};
} // namespace tse
