#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "FareDisplay/MileageAdapterImpl.h"

#include <memory>

namespace tse
{

class ShortMPMileageAdapterImpl : public MileageAdapterImpl
{
public:
  bool getMPM(MPData&) const override;

private:
  ShortMPMileageAdapterImpl() {}
  ShortMPMileageAdapterImpl(const ShortMPMileageAdapterImpl&);
  ShortMPMileageAdapterImpl& operator=(const ShortMPMileageAdapterImpl&);
  friend class tse::Singleton<ShortMPMileageAdapterImpl>;
};

} // namespace tse

