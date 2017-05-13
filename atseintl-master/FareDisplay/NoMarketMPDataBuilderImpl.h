#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "FareDisplay/MPDataBuilderImpl.h"

#include <memory>

namespace tse
{

class NoMarketMPDataBuilderImpl : public MPDataBuilderImpl
{
public:
  MPData* buildMPData(FareDisplayTrx&) const override;

private:
  NoMarketMPDataBuilderImpl() {}
  NoMarketMPDataBuilderImpl(const NoMarketMPDataBuilderImpl&);
  NoMarketMPDataBuilderImpl& operator=(const NoMarketMPDataBuilderImpl&);
  friend class tse::Singleton<NoMarketMPDataBuilderImpl>;
};

} // namespace tse

