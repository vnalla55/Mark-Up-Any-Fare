//----------------------------------------------------------------------------
//
//  File:        TseServer.h
//  Description: TseServer class
//  Created:     Dec 11, 2003
//  Authors:     Mark Kasprowicz
//
//  Description: Main Tse Server class for the atseintl project
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
#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/Thread/TSEReadWriteLock.h"
#include "DBAccess/CacheNotifyInfo.h"
#include "Server/LoadableModule.h"

#include <map>
#include <memory>
#include <string>

class Adapter;
class Manager;

namespace tse
{
namespace Memory
{
class ManagerObserver;
}
class Service;
class Xform;
class DataManager;
class DiskspaceWatcher;
class DynamicConfigLoader;

class TseServer
{

public:
  // Default Config file
  static const std::string DEFAULT_CFG_FILE;

  //
  // Keys used for config file lookups
  //
  static const std::string LOG_CFG_KEY; // Ini Key for log4cxx config file
  static const std::string DATAMGR_KEY; // Ini key for the Data Manager
  static const std::string METRICS_RESPONSES; // Ini key for the MetricsMan count of responses
  static const std::string METRICS_FACTORS; // Ini key for the MetricsMan count of factors
  static const std::string ALLOW_HISTORICAL; // Ini key to turn historical on/off
  static const std::string ABACUS_ENABLED_KEY; // Ini key for Abacus enabled flag
  static const std::string THREAD_METRICS_KEY; // Ini key for Thread Metrics

  static const std::string DEBUG_FLAGS_SECTION; // Section for debugging flags
  static const std::string DELETELIST_METRICS_KEY; // Flag to control tracking of DeleteList metrics
  static const std::string DEBUG_EXCEPTIONS; // Ini key for debugging exceptions (turns off "catch")

  static const std::string DISK_CHECK_INTERVAL; // DiskspaceWatcher interval
  static const std::string DISK_FREE_THRESHOLD; // DiskspaceWatcher threshold
  static const std::string DISK_TO_WATCH; // DiskspaceWatcher disk to watch
  static const std::string DISK_ALERT_EMAIL; // DiskspaceWatcher alert email recipients
  static const std::string DISK_WATCHER_DISABLED; // DiskspaceWatcher disabled

  static const std::string INVOKE_FOREACH_USES_DATAHANDLE_KEY; // Flag to control invoke_foreach
                                                               // algorithms

  TseServer(const std::string& name = "TseServer");

  virtual ~TseServer();

  TseServer(const TseServer& rhs) = delete;
  TseServer& operator=(const TseServer& rhs) = delete;

  /**
   * Initialize the server
   *
   * @param argc   command line argument count
   * @param argv   command line arguments
   *
   * @return true if successful, false otherwise
   */
  bool initialize(int argc, char* argv[], const std::vector<std::string>& cfgOverrides);

  /**
   * Force the server to load all modules
   *
   * @return true if successful, false otherwise
   */
  bool load();

  void unload();

  /**
   *  Config file name
   */
  std::string& configName() { return _cfgFileName; }
  const std::string& configName() const { return _cfgFileName; }

  /**
   * Config object accessors
   *
   * @return Config object
   */
  ConfigMan& config() { return _config; }
  const ConfigMan& config() const { return _config; }

  /**
   * Server name accessors
   *
   * @return server name
   */
  std::string& name() { return _name; }
  const std::string& name() const { return _name; }

  /**
   * Retrieve an Adapter pointer with a given name
   *
   * @param name   Key Name
   *
   * @return Pointer if successful, 0 otherwise
   */
  Adapter* adapter(const std::string& aName) { return fromMap<Adapter>(aName, _adapterMap); }

  /**
   * Retrieve a Manager pointer with a given name
   *
   * @param name   Key Name
   *
   * @return Pointer if successful, 0 otherwise
   */
  Manager* manager(const std::string& aName) { return fromMap<Manager>(aName, _managerMap); }

  /**
   * Retrieve a Service pointer with a given name
   *
   * @param name   Key Name
   *
   * @return Pointer if successful, 0 otherwise
   */
  Service* service(const std::string& aName) { return fromMap<Service>(aName, _serviceMap); }

  /**
   * Retrieve an Xform pointer with a given name
   *
   * @param name   Key Name
   *
   * @return Pointer if successful, 0 otherwise
   */
  Xform* xform(const std::string& aName) { return fromMap<Xform>(aName, _xformMap); }

  static bool loadConfigWithOverrides(const std::string& cfgFile,
                                      const std::string& cfgFileOverrideGroup,
                                      const std::string& cfgFileOverrideKey,
                                      const char* cfgFileSeparator,
                                      const std::vector<ConfigMan::NameValue>& cfgOverrides,
                                      ConfigMan& config);

  void injectCacheNotification(const std::string& entityType,
                               const std::string& keyString,
                               unsigned int processingDelay);

  void siphonInjectedCacheNotifications(std::vector<CacheNotifyInfo>& infos);

  static TseServer* getInstance() { return _instance; }

  const std::string& getExecutableName() const { return _executableName; }
  const std::string& getProcessImageName() const { return _processImageName; }

private:
  using AdapterMap = std::map<std::string, LoadableModule<Adapter>>;
  using ManagerMap = std::map<std::string, LoadableModule<Manager>>;
  using ServiceMap = std::map<std::string, LoadableModule<Service>>;
  using XformMap = std::map<std::string, LoadableModule<Xform>>;

  std::string _name;
  std::string _cfgFileName;
  std::string _executableName;
  std::string _processImageName;

  ConfigMan _config;
  MetricsMan _metricsMan;

  DataManager* _dataManager = nullptr;

  // Map files to hold LoadableModules
  //
  AdapterMap _adapterMap;
  ManagerMap _managerMap;
  ServiceMap _serviceMap;
  XformMap _xformMap;

  DynamicConfigLoader* _dynamicLoadThread = nullptr;
  DiskspaceWatcher* _diskThread = nullptr;
  std::unique_ptr<Memory::ManagerObserver> _memoryManagerObserver;

  static TseServer* _instance;

  // Allow direct injection of cache notifications (for testing)
  //
  TSEReadWriteLock _directInjectMutex;
  std::vector<CacheNotifyInfo> _directInjectInfos;

  std::vector<ConfigMan::NameValue> _cfgOverrides;

  void initializeLogger();

  /**
   * Initialize disk space watcher
   *
   * @return true is successful, false otherwise
   */
  bool initializeDiskspaceWatcher();

  void initializeMetricsMan();

  void initializeDebugFlags();

  /**
   * Initialize MemCacheCxx
   *
   * @return true when successful, false otherwise
   */
  bool initializeMemCacheD();

  /**
   * Initialize DataManager
   *
   * @return true is successful, false otherwise
   */
  bool initializeDataManager();

  /**
   * Initialize ASAP stuff
   *
   * @return true is successful, false otherwise
   */
  bool initializeASAP();

  /**
   * Load all the server modules
   *
   * @return true if successful, false otherwise
   */
  bool loadAllModules();

  /**
   * PostLoad all the server modules
   *
   * @return true if successful, false otherwise
   */
  bool postLoadAllModules();

  void preUnloadAllModules();

  void unloadAllModules();

  /**
   * Loads LoadableModule objects to point to the given class
   *
   * @param T      Class the modules will hold
   * @param argc   argc to pass to the initialize function
   * @param argv   argv to pass to the initialize function
   * @param group  Config file group name that specifies modules to load
   * @param modMap Map that will hold the created objects
   *
   * @return true if successful, false otherwise
   */
  template <class T>
  bool loadModules(int argc,
                   char* argv[],
                   const std::string& group,
                   std::map<std::string, LoadableModule<T> >& modMap);

  /**
   * Loads LoadableModule objects to point to the given class
   *
   * @param T      Class the modules will hold
   * @param argc   argc to pass to the initialize function
   * @param argv   argv to pass to the initialize function
   * @param group  Config file group name that specifies modules to load
   * @param modMap Map that will hold the created objects
   * @param name   Config file group name that specifies modules to load
   *
   * @return true if successful, false otherwise
   */
  template <class T>
  bool loadModule(int argc,
                  char* argv[],
                  const std::string& group,
                  std::map<std::string, LoadableModule<T> >& modMap,
                  const std::string& name);

  /**
   * Post Loads LoadableModule objects to point to the given class
   *
   * @param T      Class the modules will hold
   * @param argc   argc to pass to the initialize function
   * @param argv   argv to pass to the initialize function
   * @param group  Config file group name that specifies modules to load
   * @param modMap Map that will hold the created objects
   *
   * @return true if successful, false otherwise
   */
  template <class T>
  bool postLoadModules(int argc,
                       char* argv[],
                       const std::string& group,
                       std::map<std::string, LoadableModule<T> >& modMap);

  /**
   * PostLoads LoadableModule objects to point to the given class
   *
   * @param T      Class the modules will hold
   * @param argc   argc to pass to the initialize function
   * @param argv   argv to pass to the initialize function
   * @param group  Config file group name that specifies modules to load
   * @param modMap Map that will hold the created objects
   * @param name   Config file group name that specifies modules to load
   *
   * @return true if successful, false otherwise
   */
  template <class T>
  bool postLoadModule(int argc,
                      char* argv[],
                      const std::string& group,
                      std::map<std::string, LoadableModule<T> >& modMap,
                      const std::string& name);

  template <class T>
  void unloadModules(std::map<std::string, LoadableModule<T>>& modMap);

  template <class T>
  void unloadModule(std::map<std::string, LoadableModule<T>>& modMap, const std::string& name);

  template <class T>
  void preUnloadModules(std::map<std::string, LoadableModule<T>>& modMap);

  template <class T>
  void preUnloadModule(std::map<std::string, LoadableModule<T>>& modMap, const std::string& name);

  /**
   * Retrieves a LoadableModule instance pointer with a designated name
   *
   * @param T      class type of the pointer
   * @param key    Name to search for
   * @param modMap Map to search
   *
   * @return Pointer if successful, 0 otherwise
   */
  template <class T>
  T* fromMap(const std::string& key, std::map<std::string, LoadableModule<T> >& modMap)
  {

    typename std::map<std::string, LoadableModule<T> >::iterator i = modMap.find(key);

    if (i == modMap.end())
      return nullptr;

    LoadableModule<T>& mod = i->second;

    return mod.instance();
  }
protected:
  void initializeGlobal();
  void initializeGlobalConfigMan();
  void initializeDiskCache();
  bool initializeConfig();
}; // End class TseServer
} // End namespace tse
