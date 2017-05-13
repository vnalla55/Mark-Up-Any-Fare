//----------------------------------------------------------------------------
//  Description: Global TSE Resources
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigBundle.h"
#endif
#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Server/LoadableModule.h"
#include "Service/Service.h"

#include <atomic>
#include <map>
#include <string>

class Adapter;
class Manager;

namespace tse
{
class TseServer;
class Xform;
class ConfigMan;
class MetricsMan;

class Global
{
public:
  typedef std::shared_ptr<tse::ConfigMan> configPtr;

  static tse::ConfigMan& config() { return *_configMan; }
  static tse::MetricsMan& metrics() { return *_metricsMan; }

  static configPtr& dynamicCfg() { return _dynamicCfg; }
  static configPtr& newDynamicCfg() { return _newDynamicCfg; }
  static std::atomic<bool>& configUpdateInProgress() { return _configUpdateInProgress; }
#ifdef CONFIG_HIERARCHY_REFACTOR
  static std::atomic<ConfigBundle*>& configBundle() { return _configBundle; }
#endif

  static bool hasConfig() { return _configMan != nullptr; }

  static Service* service(const std::string& name) { return fromMap<Service>(_services, name); }
  static std::map<std::string, LoadableModule<Service> >* services() { return _services; }

  static Xform* xform(const std::string& name) { return fromMap<Xform>(_xforms, name); }
  static std::map<std::string, LoadableModule<Xform> >* xforms() { return _xforms; }

  static Adapter* adapter(const std::string& name) { return fromMap<Adapter>(_adapters, name); }
  static std::map<std::string, LoadableModule<Adapter> >* adapters() { return _adapters; }

  static Manager* manager(const std::string& name) { return fromMap<Manager>(_managers, name); }
  static std::map<std::string, LoadableModule<Manager> >* managers() { return _managers; }

  // TseServer should be the only one setting the object pooling attribute.
  // Changing this value at runtime WILL be catastrophic!
  static bool allowHistorical() { return _allowHistorical; }
  static size_t getUnlimitedCacheSize()
  {
    return _unlimitedCacheSize;
  }
  static void setUnlimitedCacheSize(size_t a_UnlimitedCacheSizeIndicator)
  {
    _unlimitedCacheSize = a_UnlimitedCacheSizeIndicator;
  }
  static const DateTime startTime() { return _startTime; }

  static HasherMethod hasherMethod() { return _hasherMethod; }
  static void setHasherMethod(uint16_t hasherMethodNumber);

protected:
  static tse::ConfigMan* _configMan;
  static tse::MetricsMan* _metricsMan;
  static bool _allowHistorical;
  static size_t _unlimitedCacheSize;
  static DateTime _startTime;
  static HasherMethod _hasherMethod;

  static configPtr _dynamicCfg;
  static configPtr _newDynamicCfg;
  static std::atomic<bool> _configUpdateInProgress;
#ifdef CONFIG_HIERARCHY_REFACTOR
  static std::atomic<ConfigBundle*> _configBundle;
#endif

  static std::map<std::string, LoadableModule<Adapter> >* _adapters;
  static std::map<std::string, LoadableModule<Manager> >* _managers;
  static std::map<std::string, LoadableModule<Service> >* _services;
  static std::map<std::string, LoadableModule<Xform> >* _xforms;

  template <class T>
  static T* fromMap(std::map<std::string, LoadableModule<T> >* map, const std::string& name)
  {
    if (map == nullptr)
      return nullptr;

    typename std::map<std::string, LoadableModule<T> >::iterator i = map->find(name);
    if (i == map->end())
      return nullptr;

    return i->second.instance();
  }

private:
  friend class TseServer;
  friend class TseServerMock;
  friend class ConstructionJobMock;

  //
  // Placed here so they cant be called
  //
  Global(const Global& rhs);
  Global& operator=(const Global& rhs);
};

} // end tse namespace

