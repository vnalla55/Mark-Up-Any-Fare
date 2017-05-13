#include "FareDisplay/MPChooser.h"

namespace tse
{
MPChooser&
MPChooser::getChooser(MPType mpType)
{
  ChooserMapConstIter i(_chooserMap.find(mpType));
  if (i == _chooserMap.end())
    throw ChooserNotRegisteredException();
  return (i->second)();
}

bool
MPChooser::registerChooser(MPType mpType, MPChooserProxy proxy)
{
  return _chooserMap.insert(ChooserMap::value_type(mpType, proxy)).second;
}

bool
MPChooser::unregisterChooser(MPType mpType)
{
  return _chooserMap.erase(mpType) == 1;
}
}
