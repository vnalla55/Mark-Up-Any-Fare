#pragma once

#include "Common/Singleton.h"
#include "FareDisplay/MPChooser.h"

namespace tse
{

class ShortMPChooser : public MPChooser
{
public:
  MileageAdapterImpl& getMileageAdapterImpl() const override;
  MPDataBuilderImpl& getMPDataBuilderImpl() const override;
  ~ShortMPChooser();

protected:
  ShortMPChooser() {}
  ShortMPChooser(const ShortMPChooser&);
  ShortMPChooser& operator=(const ShortMPChooser&);
  friend class tse::Singleton<ShortMPChooser>;

private:
  static MPChooser& proxy();
  static const bool _registered;
};

} // namespace tse

