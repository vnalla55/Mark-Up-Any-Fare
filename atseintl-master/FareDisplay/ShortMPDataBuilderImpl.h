#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "FareDisplay/MPDataBuilderImpl.h"


#include <memory>

namespace tse
{

class PaxTypeFare;
class FareDisplayOptions;

class ShortMPDataBuilderImpl : public MPDataBuilderImpl
{
public:
  MPData* buildMPData(FareDisplayTrx&) const override;

protected:
  ShortMPDataBuilderImpl() {}
  ShortMPDataBuilderImpl(const ShortMPDataBuilderImpl&);
  ShortMPDataBuilderImpl& operator=(const ShortMPDataBuilderImpl&);
  friend class tse::Singleton<ShortMPDataBuilderImpl>;

private:
  static MoneyAmount getAmount(const PaxTypeFare&, const FareDisplayOptions*);
  static CurrencyCode getCurrency(const PaxTypeFare&, const FareDisplayTrx&);
  static void prepareInput(FareDisplayTrx&);
};

} // namespace tse

