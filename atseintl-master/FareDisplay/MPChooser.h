#pragma once

#include "FareDisplay/FDConsts.h"

#include <map>

namespace tse
{

class MileageAdapterImpl;
class MPDataBuilderImpl;

class MPChooser
{
public:
  class ChooserNotRegisteredException
  {
  };

  static MPChooser& getChooser(MPType); // throw (ChooserNotRegisteredException)

  virtual MileageAdapterImpl& getMileageAdapterImpl() const = 0;
  virtual MPDataBuilderImpl& getMPDataBuilderImpl() const = 0;

  virtual ~MPChooser() {}

protected:
  /**
   * All derived classes should statically register
   * their proxy functions in StaticInitialization.cpp.
   */
  typedef MPChooser& (*MPChooserProxy)();
  static bool registerChooser(MPType, MPChooserProxy);
  static bool unregisterChooser(MPType);

private:
  typedef std::map<MPType, MPChooserProxy> ChooserMap;
  typedef ChooserMap::const_iterator ChooserMapConstIter;
  static ChooserMap _chooserMap;
};

} // namespace tse

