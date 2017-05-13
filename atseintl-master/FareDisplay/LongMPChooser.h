#pragma once

#include "Common/Singleton.h"
#include "FareDisplay/MPChooser.h"

namespace tse
{

class LongMPChooser : public MPChooser
{
public:
  MileageAdapterImpl& getMileageAdapterImpl() const override;
  MPDataBuilderImpl& getMPDataBuilderImpl() const override;
  ~LongMPChooser();

protected:
  LongMPChooser() {}
  LongMPChooser(const LongMPChooser&);
  LongMPChooser& operator=(const LongMPChooser&);
  friend class tse::Singleton<LongMPChooser>;

private:
  static MPChooser& proxy();
  static const bool _registered;
};

} // namespace tse

