#pragma once

#include "Common/Singleton.h"
#include "FareDisplay/MPChooser.h"

namespace tse
{

class NoMarketMPChooser : public MPChooser
{
public:
  MileageAdapterImpl& getMileageAdapterImpl() const override;
  MPDataBuilderImpl& getMPDataBuilderImpl() const override;
  ~NoMarketMPChooser();

protected:
  NoMarketMPChooser() {}
  NoMarketMPChooser(const NoMarketMPChooser&);
  NoMarketMPChooser& operator=(const NoMarketMPChooser&);
  friend class tse::Singleton<NoMarketMPChooser>;

private:
  static MPChooser& proxy();
  static const bool _registered;
};

} // namespace tse

