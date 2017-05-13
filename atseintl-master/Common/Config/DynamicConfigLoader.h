//-------------------------------------------------------------------
//
//  File:        DynamicCfgLoader
//  Created:     Dec 10, 2010
//  Authors:     Okan Okcu
//
//  Copyright Sabre 2009
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigBundle.h"
#endif
#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/Thread/TimerTask.h"

#include <vector>

namespace tse
{
typedef bool (*LOAD_CONFIG_WITH_OVERRIDES)(const std::string&,
                                           const std::string&,
                                           const std::string&,
                                           const char*,
                                           const std::vector<ConfigMan::NameValue>&,
                                           ConfigMan&);

class DynamicConfigLoader : public TimerTask
{
public:
  DynamicConfigLoader(std::string& cfgFile,
                      std::string& cfgFileOverrideGroup,
                      const char* cfgFileOverrideKey,
                      const char* cfgFileSeparator,
                      const std::vector<ConfigMan::NameValue>& cfgOverrides,
                      LOAD_CONFIG_WITH_OVERRIDES callback);

  DynamicConfigLoader(const DynamicConfigLoader&) = delete;
  DynamicConfigLoader& operator=(const DynamicConfigLoader&) = delete;

  ~DynamicConfigLoader();

  static DynamicConfigLoader* startThread(std::string& cfgFile,
                                          std::string& cfgFileOverrideGroup,
                                          const char* cfgFileOverrideKey,
                                          const char* cfgFileSeparator,
                                          const std::vector<ConfigMan::NameValue>& cfgOverrides,
                                          LOAD_CONFIG_WITH_OVERRIDES callback);

  static bool isDynamicLoadable(const std::string& label, const std::string& section);

  static void stopThread();

  static bool loadNewConfig(std::string& output)
  {
    if (_instance)
      return _instance->updateDynamicConfig(output);

    return false;
  }

  static bool overrideConfigValue(Global::configPtr& config,
                                  const std::string& group,
                                  const std::string& name,
                                  const std::string& value,
                                  bool permament = false);
  static Global::configPtr cloneOverridenConfig(Global::configPtr& config);

  virtual void run() override;

  static bool isActive() { return _instance; }
  static bool findLoadableItems();

protected:
  std::string _cfgFile;
  std::string _cfgFileOverrideGroup;
  std::string _cfgFileOverrideKey;
  std::string _cfgFileSeparator;
  const std::vector<ConfigMan::NameValue> _cfgOverrides;
#ifdef CONFIG_HIERARCHY_REFACTOR
  ConfigBundle _cfgBundles[2];
#endif
  uint8_t _cfgBundle = 0;

  time_t _lastUpdateAttempt = 0;
  time_t _lastUpdate = 0;

  virtual void onCancel() override;

  int updateDynamicEntries(const ConfigMan& sourceCfg, ConfigMan& destCfg, std::string& output);
  bool updateDynamicConfig(std::string& output);
  static void generateNewConfig(const ConfigMan& sourceCfg, Global::configPtr& newCfg);
  void debugMsg();

private:
  friend class DynamicConfigLoaderTest;
  friend class TrxUtilTest;

  DynamicConfigLoader(std::string& cfgFile,
                      std::string& cfgFileOverrideGroup,
                      const char* cfgFileOverrideKey,
                      const char* cfgFileSeparator,
                      const std::vector<ConfigMan::NameValue>& cfgOverrides,
                      LOAD_CONFIG_WITH_OVERRIDES callback,
                      int) // used in test
      : TimerTask(),
        _cfgFile(cfgFile),
        _cfgFileOverrideGroup(cfgFileOverrideGroup),
        _cfgFileOverrideKey(cfgFileOverrideKey),
        _cfgFileSeparator(cfgFileSeparator),
        _cfgOverrides(cfgOverrides),
        _callback(callback)
  {
    init();
  }

  void init();

  static DynamicConfigLoader* _instance;

  LOAD_CONFIG_WITH_OVERRIDES _callback;
};
}
