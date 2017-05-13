#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "FareDisplay/MileageAdapterImpl.h"

#include <memory>

namespace tse
{

class LongMPMileageAdapterImpl : public MileageAdapterImpl
{
public:
  bool getMPM(MPData&) const override;

private:
  LongMPMileageAdapterImpl() {}
  LongMPMileageAdapterImpl(const LongMPMileageAdapterImpl&);
  LongMPMileageAdapterImpl& operator=(const LongMPMileageAdapterImpl&);
  friend class tse::Singleton<LongMPMileageAdapterImpl>;
};

} // namespace tse

