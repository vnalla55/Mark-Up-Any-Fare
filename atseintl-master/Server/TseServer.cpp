//----------------------------------------------------------------------------
//
//  File:        TseServer.C
//  Description: See .h file
//  Created:     Dec 11, 2003
//  Authors:     Mark Kasprowicz
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2003
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
#include "Server/TseServer.h"

#include "Adapter/Adapter.h"
#include "Allocator/TrxMalloc.h"
#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigBundle.h"
#endif
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/Config/FallbackValue.h"
#include "Common/DiskspaceWatcher.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/ManagerObserver.h"
#include "Common/Memory/Monitor.h"
#include "Common/Message.h"
#include "Common/MetricsUtil.h"
#include "Common/NotTimingOutDetector.h"
#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/Thread/WheelTimerTaskExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/DistCachePool.h"
#include "Manager/Manager.h"
#include "Server/LoadableModuleMethods.h"
#include "Service/Service.h"
#include "Xform/Xform.h"
#include "Xray/AsyncJsonContainer.h"
#include "Xray/XrayClient.h"
#include "Xray/XraySender.h"

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>

#include <string>
#include <memory>

#include <unistd.h>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackXrayJsonContainerInTseServer);

// Static initializers of constants
//
const std::string TseServer::DEFAULT_CFG_FILE = "tseserver.acms.cfg";

const std::string TseServer::LOG_CFG_KEY = "LOG_CFG";
const std::string TseServer::DATAMGR_KEY = "DATA_MANAGER";
const std::string TseServer::METRICS_RESPONSES = "METRICS_RESPONSES";
const std::string TseServer::METRICS_FACTORS = "METRICS_FACTORS";
const std::string TseServer::DEBUG_EXCEPTIONS = "DEBUG_EXCEPTIONS";
const std::string TseServer::ALLOW_HISTORICAL = "ALLOW_HISTORICAL";
const std::string TseServer::ABACUS_ENABLED_KEY = "ABACUS_ENABLED";
const std::string TseServer::THREAD_METRICS_KEY = "THREAD_METRICS";

const std::string TseServer::DEBUG_FLAGS_SECTION = "DEBUG_INFO";
const std::string TseServer::DELETELIST_METRICS_KEY = "DELETELIST_METRICS_FLAGS";
const std::string TseServer::INVOKE_FOREACH_USES_DATAHANDLE_KEY = "INVOKE_FOREACH_USES_DATAHANDLE";

const std::string TseServer::DISK_CHECK_INTERVAL = "DISK_CHECK_INTERVAL";
const std::string TseServer::DISK_FREE_THRESHOLD = "DISK_FREE_THRESHOLD";
const std::string TseServer::DISK_TO_WATCH = "DISK_TO_WATCH";
const std::string TseServer::DISK_ALERT_EMAIL = "DISK_ALERT_EMAIL";
const std::string TseServer::DISK_WATCHER_DISABLED = "DISK_WATCHER_DISABLED";

static Logger
logger("atseintl.Server.TseServer");

TseServer* TseServer::_instance = nullptr;

FIXEDFALLBACK_DECL(wheelTimerTaskExecutor)

namespace
{
ConfigurableValue<std::string>
diskToWatch("TSE_SERVER", TseServer::DISK_TO_WATCH);
ConfigurableValue<int>
diskCheckInterval("TSE_SERVER", TseServer::DISK_CHECK_INTERVAL);
ConfigurableValue<int>
diskFreeThreshold("TSE_SERVER", TseServer::DISK_FREE_THRESHOLD);
ConfigurableValue<std::string>
diskAlertEmail("TSE_SERVER", TseServer::DISK_ALERT_EMAIL);
ConfigurableValue<bool>
diskWatcherDisabled("TSE_SERVER", TseServer::DISK_WATCHER_DISABLED, false);
ConfigurableValue<bool>
abacusEnabledCfg("TSE_SERVER", TseServer::ABACUS_ENABLED_KEY, false);
ConfigurableValue<bool>
tmEnabled("TSE_SERVER", TseServer::THREAD_METRICS_KEY, true);
ConfigurableValue<std::string>
logCfg("TSE_SERVER", TseServer::LOG_CFG_KEY);
ConfigurableValue<uint32_t>
deleteMetrics(TseServer::DEBUG_FLAGS_SECTION,
              TseServer::DELETELIST_METRICS_KEY,
              DeleteList::DEBUG_NONE);
ConfigurableValue<std::string>
dataMgrKey("TSE_SERVER", TseServer::DATAMGR_KEY);
ConfigurableValue<uint32_t>
trxTimeoutCfg("TSE_SERVER", "TRX_TIMEOUT", 0);
ConfigurableValue<bool>
trxAllocatorVal("TSE_SERVER", "TRX_ALLOCATOR", false);
ConfigurableValue<size_t>
trxAllocatorMBLimit("TSE_SERVER", "TRX_ALLOCATOR_MEMORY_LIMIT", 0);
ConfigurableValue<uint32_t>
gcSleepTime("TSE_SERVER", "TRX_ALLOCATOR_GC_SLEEP_TIME");
ConfigurableValue<std::string>
logLevelCfg("TSE_SERVER", "TRX_ALLOCATOR_LOG_LEVEL");
ConfigurableValue<bool>
allowHistoricalValue("TSE_SERVER", TseServer::ALLOW_HISTORICAL, false);
ConfigurableValue<uint16_t>
hasherMethodCfg("TSE_SERVER", "HASHER_METHOD", 0);
ConfigurableValue<bool>
useasap("TSE_SERVER", "USE_ASAP", false);
ConfigurableValue<std::string>
asapini("TSE_SERVER", "ASAP_INI", "asap.ini");
}

TseServer::TseServer(const std::string& name) : _name(name)
{
}

TseServer::~TseServer()
{
  LOG4CXX_DEBUG(logger, "TseServer destructing");

  LOG4CXX_DEBUG(logger, "TseServer unloading all modules");

  unload();

  LOG4CXX_DEBUG(logger, "TseServer destroying Logger");

  LOG4CXX_DEBUG(logger, "TseServer destroying Data Manager");

  if (_dataManager != nullptr)
  {
    delete _dataManager;
    _dataManager = nullptr;
  }

  if (TimerTaskExecutor::hasInstance())
  {
    if (!Memory::changesFallback)
    {
      Memory::MemoryMonitor::destroyInstance();
    }

    NotTimingOutDetector::stop();

    DiskspaceWatcher::stopWatching();
    _diskThread = nullptr;

    DynamicConfigLoader::stopThread();
    _dynamicLoadThread = nullptr;

    TimerTaskExecutor::destroyInstance();
  }

  if (!fallback::fixed::fallbackXrayJsonContainerInTseServer())
  {
    xray::asyncjsoncontainer::closeWaitForSendThread();
    LOG4CXX_DEBUG(logger, "XrayWaitForSendThread destroyed");
  }

  TseSrvStats::shutdown();

  LOG4CXX_DEBUG(logger, "TseServer destructed");
}

bool
TseServer::load()
{
  return loadAllModules() && postLoadAllModules();
}

void
TseServer::unload()
{
  preUnloadAllModules();
  unloadAllModules();
}

bool
TseServer::loadAllModules()
{
  // VERY IMPORTANT !
  //
  // The order of instantiation must be:
  // Adapters, Services, Xforms, Managers

  // Initialize adapters
  //
  if (!loadModules<Adapter>(0, nullptr, "ADAPTERS", _adapterMap))
    return false; // failure

  LOG4CXX_INFO(logger, _adapterMap.size() << " Adapters initialized");

#ifdef _USERAWPOINTERS
  LOG4CXX_INFO(logger, "Raw pointers caches");
#else
  LOG4CXX_INFO(logger, "Shared pointers caches");
#endif // _USERAWPOINTERS

  // Initialize services
  //
  if (!loadModules<Service>(0, nullptr, "SERVICES", _serviceMap))
    return false; // failure

  LOG4CXX_INFO(logger, _serviceMap.size() << " Services initialized");

  // Initialize xforms
  //
  if (!loadModules<Xform>(0, nullptr, "XFORMS", _xformMap))
    return false; // failure

  LOG4CXX_INFO(logger, _xformMap.size() << " Xforms initialized");

  // Initialize managers
  //
  if (!loadModules<Manager>(0, nullptr, "MANAGERS", _managerMap))
    return false; // failure

  LOG4CXX_INFO(logger, _managerMap.size() << " Managers initialized");

  return true;
}

bool
TseServer::postLoadAllModules()
{
  // Initialize adapters
  //
  if (!postLoadModules<Adapter>(0, nullptr, "ADAPTERS", _adapterMap))
    return false; // failure

  LOG4CXX_INFO(logger, _adapterMap.size() << " Adapters post initialized");

  // Initialize services
  //
  if (!postLoadModules<Service>(0, nullptr, "SERVICES", _serviceMap))
    return false; // failure

  LOG4CXX_INFO(logger, _serviceMap.size() << " Services post initialized");

  // Initialize xforms
  //
  if (!postLoadModules<Xform>(0, nullptr, "XFORMS", _xformMap))
    return false; // failure

  LOG4CXX_INFO(logger, _xformMap.size() << " Xforms post initialized");

  // Initialize managers
  //
  if (!postLoadModules<Manager>(0, nullptr, "MANAGERS", _managerMap))
    return false; // failure

  LOG4CXX_INFO(logger, _managerMap.size() << " Managers post initialized");

  return true;
}

void
TseServer::preUnloadAllModules()
{
  LOG4CXX_DEBUG(logger, "PreUnloading managers");
  preUnloadModules(_managerMap);

  LOG4CXX_DEBUG(logger, "PreUnloading xforms");
  preUnloadModules(_xformMap);

  LOG4CXX_DEBUG(logger, "PreUnloading services");
  preUnloadModules(_serviceMap);

  LOG4CXX_DEBUG(logger, "PreUnloading adapters");
  preUnloadModules(_adapterMap);
}

void
TseServer::unloadAllModules()
{
  LOG4CXX_DEBUG(logger, "Unloading managers");
  unloadModules(_managerMap);

  LOG4CXX_DEBUG(logger, "Unloading xforms");
  unloadModules(_xformMap);

  LOG4CXX_DEBUG(logger, "Unloading services");
  unloadModules(_serviceMap);

  LOG4CXX_DEBUG(logger, "Unloading adapters");
  unloadModules(_adapterMap);
}

bool
TseServer::initialize(int argc, char* argv[], const std::vector<std::string>& cfgOverrides)
{
  // workaround for boost bug, causing race condition:
  boost::gregorian::greg_month::get_month_map_ptr();

  for (const std::string& override : cfgOverrides)
  {
    boost::char_separator<char> sep(".=");
    boost::tokenizer<boost::char_separator<char>> tokens(override, sep);

    boost::tokenizer<boost::char_separator<char>>::iterator k = tokens.begin();
    boost::tokenizer<boost::char_separator<char>>::iterator l = tokens.end();

    if (k == l)
      continue;

    ConfigMan::NameValue nv;

    nv.group = *k;

    k++;
    if (k == l)
      continue;

    nv.name = *k;

    k++;
    if (k == l)
      continue;

    nv.value = *k;

    //
    // Put everything else into the value, and remember
    // to put the .'s back in
    //
    k++;
    for (; k != l; ++k)
    {
      nv.value += ".";
      nv.value += *k;
    }

    _cfgOverrides.push_back(nv);
  }

  // Config
  //
  if (!initializeConfig())
    return false; // failure

  initializeGlobalConfigMan();

#ifdef CONFIG_HIERARCHY_REFACTOR
  ConfigurableValuesPool::configure();
  allocateAllConfigBundles();
#endif

  // Logger
  initializeLogger();

  // Log status
  //
  LOG4CXX_INFO(logger, "Config initialized");
  LOG4CXX_INFO(logger, "Logger initialized");

  _executableName.assign(boost::filesystem::path(argv[0]).filename().c_str());
  try
  {
    _processImageName.assign(boost::filesystem::canonical(argv[0]).filename().c_str());
  }
  catch (const boost::filesystem::filesystem_error& er)
  {
    LOG4CXX_DEBUG(logger, "TSE_SERVER PROCESS IMAGE NAME: " << er.what());
    _processImageName = "resolving image name failed.";
  }

  LOG4CXX_DEBUG(logger, "TSE_SERVER PROGRAM NAME: " << _executableName);

  if (fallback::fixed::wheelTimerTaskExecutor())
    TimerTaskExecutor::setInstance(new PriorityQueueTimerTaskExecutor);
  else
    TimerTaskExecutor::setInstance(new WheelTimerTaskExecutor);

  // Local Disk Cache (LDC)
  initializeDiskCache();
  LOG4CXX_INFO(logger, "DiskCache manager initialized");

  // MemCache Interface Distributed Cache
  if (!initializeMemCacheD())
    return false;

  LOG4CXX_INFO(logger, "MemCached initialized");

  // Abacus
  //
  if (abacusEnabledCfg.getValue())
  {
    TrxUtil::enableAbacus();
  }
  else
  {
    TrxUtil::disableAbacus();
  }

  LOG4CXX_INFO(logger, "Abacus " << (TrxUtil::isAbacusEnabled() ? "enabled" : "disabled"));

  // Thread Metrics
  ThreadPoolFactory::enableMetrics(tmEnabled.getValue());
  LOG4CXX_INFO(
      logger,
      "Thread Metrics " << (ThreadPoolFactory::isMetricsEnabled() ? "enabled" : "disabled"));

  initializeDebugFlags();

  initializeMetricsMan();
  LOG4CXX_INFO(logger, "MetricsMan initialized");

  initializeGlobal();
  LOG4CXX_INFO(logger, "Global initialized");

  if (!initializeDataManager())
  {
    LOG4CXX_FATAL(logger, "DataManager initialization failed");
    return false; // failure
  }

  LOG4CXX_INFO(logger, "DataManager initialized");

  if (!initializeASAP())
  {
    LOG4CXX_FATAL(logger, "ASAP initialization failed");
    return false; // failure
  }

  // Start watching disk space
  //
  if (initializeDiskspaceWatcher())
  {
    LOG4CXX_INFO(logger, "Disk space watcher initialized");
  }

  if ((_dynamicLoadThread = DynamicConfigLoader::startThread(_cfgFileName,
                                                             _name,
                                                             "OVERRIDE_CFGS",
                                                             "|",
                                                             _cfgOverrides,
                                                             TseServer::loadConfigWithOverrides)) ==
      nullptr)
  {
    LOG4CXX_FATAL(logger, "Dynamic Config Loader initialization failed");
    return false;
  }

  NotTimingOutDetector::start();

  Memory::configure();
  if (Memory::managerEnabled)
    _memoryManagerObserver.reset(new Memory::ManagerObserver);

  if (!Memory::changesFallback)
  {
    Memory::MemoryMonitor::createInstance();
  }

  MCPCarrierUtil::initialize(_config);

  Message::fillMsgErrCodeMap();

  // Initialize xrayContainerThread
  //
  if (!fallback::fixed::fallbackXrayJsonContainerInTseServer())
  {
    xray::asyncjsoncontainer::initializeWaitForSendThread(
        std::make_unique<xray::XraySender<xray::XrayClient>>());
    LOG4CXX_INFO(logger, "XrayWaitForSendThread created");
  }

  const bool ret = load();

  if (ret)
    _instance = this;

  return ret;
}

bool
TseServer::initializeConfig()
{
  // Build default if none provided
  //
  if (_cfgFileName.empty())
  {
    _cfgFileName = DEFAULT_CFG_FILE;
  }

  return loadConfigWithOverrides(_cfgFileName, _name, "OVERRIDE_CFGS", "|", _cfgOverrides, _config);
}

void
TseServer::initializeLogger()
{
  std::string logCfgFile = logCfg.getValue();
  if (!logCfg.isDefault())
  {
    log4cxx::xml::DOMConfigurator::configure(logCfgFile);
  }
  else
  {
    log4cxx::BasicConfigurator::configure();
  }
}

void
TseServer::initializeMetricsMan()
{
  _metricsMan.initialize(MetricsUtil::MAX_METRICS_ENUM);
}

void
TseServer::initializeDebugFlags()
{
  // DeleteList metrics
  unsigned int deleteListMetricsFlag = deleteMetrics.getValue();
  if (deleteListMetricsFlag != DeleteList::getDebugFlags())
  {
    DeleteList::setDebugFlags(deleteListMetricsFlag);
    LOG4CXX_INFO(logger, "DeleteList Metrics Flag: " << deleteListMetricsFlag);
  }
}

bool
TseServer::initializeDataManager()
{
  std::string dmKey = dataMgrKey.getValue();
  if (dataMgrKey.isDefault())
  {
    LOG4CXX_ERROR(logger, "Unable to find DataManager config entry [" << DATAMGR_KEY << "]");
    std::cerr << "Unable to find DataManager config entry [" << DATAMGR_KEY << "]" << std::endl;
    return false;
  }

  _dataManager = new DataManager(dmKey, _config);
  if (_dataManager == nullptr)
  {
    LOG4CXX_ERROR(logger, "Unable to new DataManager");
    std::cerr << "Unable to new DataManager" << std::endl;
    return false;
  }

  if (!_dataManager->initialize(0, nullptr))
  {
    LOG4CXX_ERROR(logger, "DataManager::initialize failed");
    std::cerr << "DataManager::initialize failed" << std::endl;
    return false;
  }

  return true;
}

void
TseServer::initializeDiskCache()
{
  DiskCache::initialize(_config);
}

bool
TseServer::initializeDiskspaceWatcher()
{
  return (!diskWatcherDisabled.getValue() &&
          (_diskThread = DiskspaceWatcher::startWatching(diskToWatch.getValue(),
                                                         diskCheckInterval.getValue(),
                                                         diskFreeThreshold.getValue(),
                                                         diskAlertEmail.getValue())) != nullptr);
}

bool
TseServer::initializeMemCacheD()
{
  bool rc(true);
  if (DiskCache::instance().getDistCacheEnabled())
  {
    if (!DiskCache::instance().servers().empty())
    {
      rc = DistCachePool::initialize(DiskCache::instance().servers(),
                                     DiskCache::instance().poolSize());
    }
    else
    {
      LOG4CXX_WARN(logger,
                   "DISK_CACHE_OPTIONS:DISTCACHE_SERVERS USE_DISTCACHE specified,"
                   "but no servers are defined. Can't initialize MEMCACHE");
    }
  }
  return rc;
}

void
TseServer::initializeGlobalConfigMan()
{
  Global::_configMan = &_config;
#ifndef CONFIG_HIERARCHY_REFACTOR
  fallback::configure(_config);
#endif
}

void
TseServer::initializeGlobal()
{
  Global::_metricsMan = &_metricsMan;

  // Get the transaction timeout from the config file so we can report the value
  uint32_t trxTimeout = trxTimeoutCfg.getValue();
  if (!trxTimeoutCfg.isDefault())
  {
    LOG4CXX_INFO(logger, "Transaction timeout set to " << trxTimeout << " seconds.");
  }

  if (trxAllocatorVal.getValue())
  {
    enable_custom_trx_allocator();
    const size_t megabytes = trxAllocatorMBLimit.getValue();
    if (!trxAllocatorMBLimit.isDefault())
    {
      set_custom_trx_allocator_memory_limit(megabytes);
    }
    const uint32_t sleepTime = gcSleepTime.getValue();
    if (!gcSleepTime.isDefault())
    {
      set_custom_trx_allocator_gc_sleep_time(sleepTime);
    }
    // set custom allocator log level if configured
    std::string logLevelStr = logLevelCfg.getValue();
    if (!logLevelCfg.isDefault())
    {
      log4cxx::LevelPtr logLevel = log4cxx::Level::toLevel(logLevelStr, log4cxx::Level::getError());
      set_custom_trx_allocator_log_level(logLevel->toInt());
    }
  }
  const bool allowHistorical = allowHistoricalValue.getValue();
  if (!allowHistoricalValue.isDefault())
  {
    if (allowHistorical != Global::_allowHistorical)
    {
      LOG4CXX_INFO(logger, "Turning Historical Transactions " << (allowHistorical ? "On" : "Off"));
      Global::_allowHistorical = allowHistorical;
    }
  }
  const uint16_t hasherMethod = hasherMethodCfg.getValue();
  if (!hasherMethodCfg.isDefault())
  {
    Global::setHasherMethod(hasherMethod);
    LOG4CXX_INFO(logger, "Hasher Method # " << hasherMethod);
  }

  DynamicConfigLoader::findLoadableItems();

#ifndef CONFIG_HIERARCHY_REFACTOR
  fallback::configure(_config);
#endif

  Global::_startTime = DateTime::localTime();
  Global::_adapters = &_adapterMap;
  Global::_managers = &_managerMap;
  Global::_services = &_serviceMap;
  Global::_xforms = &_xformMap;
}

template <class T>
bool
TseServer::loadModules(int argc,
                       char* argv[],
                       const std::string& group,
                       std::map<std::string, LoadableModule<T>>& modMap)

{
  const MallocContextDisabler context;

  typedef ConfigMan::NameValue NV;
  typedef std::vector<NV> NVVector;
  typedef std::vector<NV>::const_iterator NVVectorIter;

  NVVector nvs;
  if (_config.getValues(nvs, group) == false)
    return false; // failure

  for (NVVectorIter iter = nvs.begin(); iter != nvs.end(); ++iter)
  {
    NV entry = *iter;

    // The cfg lines should look like this
    //
    // ModuleName = sharedlib
    //
    std::string modName = entry.name;

    if (!loadModule(argc, argv, group, modMap, modName))
    {
      LOG4CXX_ERROR(logger, "loadModules() - Unable to load module " << modName);
      return false;
    }
  }

  LOG4CXX_DEBUG(logger, "loadModules() - Successfully loaded " << modMap.size() << " modules");

  return true; // success
}

template <class T>
bool
TseServer::loadModule(int argc,
                      char* argv[],
                      const std::string& group,
                      std::map<std::string, LoadableModule<T>>& modMap,
                      const std::string& name)
{
  LOG4CXX_DEBUG(logger, "loadModule() - Attempting to load module '" << name << "'");

  if (fromMap<T>(name, modMap) != nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "loadModule() - Cannot load module '"
                      << name << "', a module is already loaded with that name");
    return false;
  }

  std::string libName;
  if (!_config.getValue(name, libName, group))
  {
    LOG4CXX_ERROR(logger, "loadModule() - Unable to find shared library name '" << name << "'");
    return false;
  }

  // When we are using the override config files having a library name of
  // blank basically means 'dont load'
  if (libName.empty())
  {
    LOG4CXX_DEBUG(logger, "initModules() - Skipping module '" << name << "', no library name");
    return true;
  }

  if (!modMap[name].load(libName, *this, name, argc, argv))
  {
    LOG4CXX_ERROR(logger, "initModules() - Failed to load module '" << libName << "'");
    std::cerr << "initModules() - Failed to load module [" << libName << "]" << std::endl;

    modMap.erase(name);
    return false;
  }

  LOG4CXX_DEBUG(logger, "initModules() - Loaded module '" << libName << "'");

  return true; // success
}

template <class T>
bool
TseServer::postLoadModules(int argc,
                           char* argv[],
                           const std::string& group,
                           std::map<std::string, LoadableModule<T>>& modMap)
{
  typedef ConfigMan::NameValue NV;
  typedef std::vector<NV> NVVector;
  typedef std::vector<NV>::const_iterator NVVectorIter;

  const MallocContextDisabler context;

  NVVector nvs;
  if (_config.getValues(nvs, group) == false)
    return false; // failure

  for (NVVectorIter iter = nvs.begin(); iter != nvs.end(); ++iter)
  {
    NV entry = *iter;

    // The cfg lines should look like this
    //
    // ModuleName = sharedlib
    //
    std::string modName = entry.name;

    if (!postLoadModule(argc, argv, group, modMap, modName))
    {
      LOG4CXX_ERROR(logger, "postLoadModules() - Unable to post load module " << modName);
      return false;
    }
  }

  LOG4CXX_DEBUG(logger,
                "postLoadModules() - Successfully post loaded " << modMap.size() << " modules");

  return true; // success
}

template <class T>
bool
TseServer::postLoadModule(int argc,
                          char* argv[],
                          const std::string& group,
                          std::map<std::string, LoadableModule<T>>& modMap,
                          const std::string& name)
{
  LOG4CXX_DEBUG(logger, "postLoadModule() - Attempting to post load module '" << name << "'");

  if (fromMap<T>(name, modMap) == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "postLoadModule() - Cannot post load module '"
                      << name << "', a module is not loaded with that name");
    return false;
  }

  std::string libName;
  if (!_config.getValue(name, libName, group))
  {
    LOG4CXX_ERROR(logger, "postLoadModule() - Unable to find shared library name '" << name << "'");
    return false;
  }

  //    // When we are using the override config files having a library name of
  //    // blank basically means 'dont load'
  if (libName.empty())
  {
    LOG4CXX_DEBUG(logger, "postLoadModules() - Skipping module '" << name << "', no library name");
    return true;
  }

  if (!modMap[name].postLoad(libName, *this, name))
  {
    LOG4CXX_ERROR(logger, "postLoadModules() - Failed to post load module '" << libName << "'");
    std::cerr << "initModules() - Failed to post load module [" << libName << "]" << std::endl;

    modMap.erase(name);
    return false;
  }

  LOG4CXX_DEBUG(logger, "initModules() - PostLoaded module '" << libName << "'");

  return true; // success
}

template <class T>
void
TseServer::preUnloadModules(std::map<std::string, LoadableModule<T>>& modMap)
{
  if (modMap.empty())
    return; // success, nothing to do

  const MallocContextDisabler context;

  typename std::map<std::string, LoadableModule<T>>::reverse_iterator i = modMap.rbegin();
  typename std::map<std::string, LoadableModule<T>>::reverse_iterator j = modMap.rend();

  for (; i != j; ++i)
  {
    std::string modName = i->first;

    preUnloadModule(modMap, modName);
  }
}

template <class T>
void
TseServer::preUnloadModule(std::map<std::string, LoadableModule<T>>& modMap,
                           const std::string& name)
{
  typename std::map<std::string, LoadableModule<T>>::iterator i = modMap.find(name);

  if (i == modMap.end())
    return; // Decided this isnt necessarily an error

  LoadableModule<T>& mod = i->second;

  LOG4CXX_DEBUG(logger, "preunloading module '" << name << "'");
  mod.preUnload();
  LOG4CXX_DEBUG(logger, "preUnloadModule() - PreUnloaded module '" << name << "'");
}

template <class T>
void
TseServer::unloadModules(std::map<std::string, LoadableModule<T>>& modMap)
{
  if (modMap.empty())
    return; // success, nothing to do

  const MallocContextDisabler context;

  typename std::map<std::string, LoadableModule<T>>::reverse_iterator i = modMap.rbegin();
  typename std::map<std::string, LoadableModule<T>>::reverse_iterator j = modMap.rend();

  for (; i != j; ++i)
  {
    std::string modName = i->first;

    unloadModule(modMap, modName);
  }

  modMap.clear();
}

template <class T>
void
TseServer::unloadModule(std::map<std::string, LoadableModule<T>>& modMap, const std::string& name)
{
  typename std::map<std::string, LoadableModule<T>>::iterator i = modMap.find(name);

  if (i == modMap.end())
    return; // Decided this isnt necessarily an error

  LoadableModule<T>& mod = i->second;

  LOG4CXX_DEBUG(logger, "unloading module '" << name << "'");
  mod.unload();
  LOG4CXX_DEBUG(logger, "unloadModule() - Unloaded module '" << name << "'");
}

bool
TseServer::initializeASAP()
{
  if (!useasap.getValue())
    return true;

  bool rc = TseSrvStats::initialize(asapini.getValue());
  if (rc)
    LOG4CXX_INFO(logger, "ASAP initialized");

  return rc;
}

bool
TseServer::loadConfigWithOverrides(const std::string& cfgFile,
                                   const std::string& cfgFileOverrideGroup,
                                   const std::string& cfgFileOverrideKey,
                                   const char* cfgFileSeparator,
                                   const std::vector<ConfigMan::NameValue>& cfgOverrides,
                                   ConfigMan& config)
{
  // Note we cant assume logging has been initialized, write to std::cerr
  if (cfgFile.empty())
  {
    std::cerr << "loadConfigApplyOverrides - No config file was specified" << std::endl;
    return false; // Failure
  }

  // Load configuration
  //
  if (!config.read(cfgFile))
  {
    std::cerr << "loadConfigApplyOverrides - Error: failed to read config file: " << cfgFile
              << std::endl;
    return false; // failure
  }

  // We're going to apply the overrides twice,
  // the first time is in case the cfgoverridekey is specified we want to use the
  // overridden value to load the extra files.  The second time is to make sure all the
  // final values are correct.  Alternatively we could have searched the cfgOverrides for the
  // specific key and used it, but this is pretty much just as fast

  if (cfgOverrides.size() != 0)
  {
    if (!config.setValues(cfgOverrides, true))
    {
      std::cerr << "loadConfigApplyOverrides - Error: unable to set override values in config"
                << std::endl;
      return false;
    }
  }

  std::string overrideFiles;
  if (!config.getValue(cfgFileOverrideKey, overrideFiles, cfgFileOverrideGroup))
  {
    // We didnt find the key so nothing left to do
    return true;
  }

  if (overrideFiles.empty())
    return true; // success

  // Now we have a set of files to override with with, go through the set applying them one at a
  // time.
  boost::char_separator<char> sep(cfgFileSeparator);
  boost::tokenizer<boost::char_separator<char>> tokens(overrideFiles, sep);

  boost::tokenizer<boost::char_separator<char>>::iterator i = tokens.begin();
  boost::tokenizer<boost::char_separator<char>>::iterator j = tokens.end();

  if (i == j)
    return true; // No tokens so I guess we're ok, success

  for (; i != j; ++i)
  {
    const std::string& cfg = *i;
    ConfigMan tmp;

    if (!tmp.read(cfg))
    {
      std::cerr << "loadConfigApplyOverrides - Error: failed to read config file: " << cfg
                << std::endl;
      return false; // failure
    }

    std::vector<ConfigMan::NameValue> tmpArray;

    if (!tmp.getValues(tmpArray))
    {
      std::cerr << "loadConfigApplyOverrides - Error: reading values from config file: " << cfg
                << std::endl;
      return false; // failure
    }

    if (!tmpArray.empty()) // Should this be an error if it is empty?
    {
      if (!config.setValues(tmpArray, true))
      {
        std::cerr << "loadConfigApplyOverrides - Error: unable to set override values in config"
                  << std::endl;
        return false;
      }
    }
  }

  // Now do the final override application
  if (cfgOverrides.size() != 0)
  {
    config.setValues(cfgOverrides, true);
  }

  return true; // success
}

void
TseServer::injectCacheNotification(const std::string& entityType,
                                   const std::string& keyString,
                                   unsigned int processingDelay)
{
  static size_t orderno = 0;
  CacheNotifyInfo n;
  n.entityType() = entityType;
  n.keyString() = keyString;
  n.createDate() = DateTime::localTime().subtractSeconds(processingDelay + 60);
  {
    TSEWriteGuard<> w(_directInjectMutex);
    n.id() = _directInjectInfos.size() + 1;
    n.orderno() = ++orderno;
    _directInjectInfos.push_back(n);
  }
}

void
TseServer::siphonInjectedCacheNotifications(std::vector<CacheNotifyInfo>& infos)
{
  infos.clear();

  {
    TSEReadGuard<> r(_directInjectMutex);
    infos.swap(_directInjectInfos);
  }
}
}
