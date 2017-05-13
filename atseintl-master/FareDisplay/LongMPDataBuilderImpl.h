#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "FareDisplay/MPDataBuilderImpl.h"


#include <memory>

namespace tse
{

class LongMPDataBuilderImpl : public MPDataBuilderImpl
{
public:
  MPData* buildMPData(FareDisplayTrx&) const override;

private:
  LongMPDataBuilderImpl() {}
  LongMPDataBuilderImpl(const LongMPDataBuilderImpl&);
  LongMPDataBuilderImpl& operator=(const LongMPDataBuilderImpl&);
  friend class tse::Singleton<LongMPDataBuilderImpl>;

};

} // namespace tse

