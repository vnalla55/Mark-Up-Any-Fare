#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "FareDisplay/MileageAdapterImpl.h"

#include <memory>

namespace tse
{

class NoMarketMPMileageAdapterImpl : public MileageAdapterImpl
{
public:
  bool getMPM(MPData&) const override { return true; }

private:
  NoMarketMPMileageAdapterImpl() {}
  NoMarketMPMileageAdapterImpl(const NoMarketMPMileageAdapterImpl&);
  NoMarketMPMileageAdapterImpl& operator=(const NoMarketMPMileageAdapterImpl&);
  friend class Singleton<NoMarketMPMileageAdapterImpl>;
};

} // namespace tse

