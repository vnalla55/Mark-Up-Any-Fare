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

#include "Common/Config/DynamicConfigLoader.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Logger.h"
#include "Common/Thread/TimerTaskExecutor.h"
#include "Common/TseUtil.h"
#include "Util/Algorithm/Container.h"

#include <boost/algorithm/string/case_conv.hpp>

#include <sstream>

namespace tse
{
namespace
{
Logger
logger("atseintl.Server.DynamicConfigLoader");

ConfigValueSet _dynamicLoadables;

// Every 60 seconds, check if the old config is still used
ConfigurableValue<uint32_t>
_dualConfigCheckWait("TSE_SERVER", "DUAL_CONFIG_CHECK_WAIT", 60);

// Wait maximum of 5 minutes to disable old config usage even if it is still
// used (should never happen)
ConfigurableValue<uint32_t>
_keepOldConfigMaxWait("TSE_SERVER", "KEEP_OLD_CONFIG_MAX_WAIT", 300);

// Wait minimum of 10 seconds between update attempts even if it was unsuccessful.
// ConfigMan fails too frequent attempts
ConfigurableValue<uint32_t>
_minUpdateInterval("TSE_SERVER", "MIN_CONFIG_UPDATE_INTERVAL", 10);
}

DynamicConfigLoader* DynamicConfigLoader::_instance;

DynamicConfigLoader::DynamicConfigLoader(std::string& cfgFile,
                                         std::string& cfgFileOverrideGroup,
                                         const char* cfgFileOverrideKey,
                                         const char* cfgFileSeparator,
                                         const std::vector<ConfigMan::NameValue>& cfgOverrides,
                                         LOAD_CONFIG_WITH_OVERRIDES callback)
  : TimerTask(TimerTask::REPEATING, std::chrono::seconds(_dualConfigCheckWait.getValue())),
    _cfgFile(cfgFile),
    _cfgFileOverrideGroup(cfgFileOverrideGroup),
    _cfgFileOverrideKey(cfgFileOverrideKey),
    _cfgFileSeparator(cfgFileSeparator),
    _cfgOverrides(cfgOverrides),
    _callback(callback)
{
  init();
  TimerTaskExecutor::instance().scheduleNow(*this);
}

DynamicConfigLoader::~DynamicConfigLoader()
{
  TimerTaskExecutor::instance().cancelAndWait(*this);
}

void
DynamicConfigLoader::init()
{
  generateNewConfig(Global::config(), Global::dynamicCfg());
#ifdef CONFIG_HIERARCHY_REFACTOR
  _cfgBundles[0].fill(*Global::dynamicCfg());
  _cfgBundle = 0;
  Global::configBundle().store(&_cfgBundles[0], std::memory_order_relaxed);
#endif

  LOG4CXX_INFO(logger, "Dynamic Config has been initialized.");
}

DynamicConfigLoader*
DynamicConfigLoader::startThread(std::string& cfgFile,
                                 std::string& cfgFileOverrideGroup,
                                 const char* cfgFileOverrideKey,
                                 const char* cfgFileSeparator,
                                 const std::vector<ConfigMan::NameValue>& cfgOverrides,
                                 LOAD_CONFIG_WITH_OVERRIDES callback)
{
  if (_dynamicLoadables.empty())
  {
    LOG4CXX_INFO(logger, "No dynamic loadable items found.");
    return nullptr;
  }

  if (!_instance)
    _instance = new DynamicConfigLoader(cfgFile,
                                        cfgFileOverrideGroup,
                                        cfgFileOverrideKey,
                                        cfgFileSeparator,
                                        cfgOverrides,
                                        callback);
  return _instance;
}

bool
DynamicConfigLoader::isDynamicLoadable(const std::string& label, const std::string& section)
{
  std::locale locale;
  return alg::contains(
      _dynamicLoadables,
      std::make_pair(boost::to_upper_copy(section, locale), boost::to_upper_copy(label, locale)));
}

bool
DynamicConfigLoader::findLoadableItems()
{
  _dynamicLoadables = DynamicConfigurableValuesPool::gather();
  return !_dynamicLoadables.empty();
}

int
DynamicConfigLoader::updateDynamicEntries(const ConfigMan& sourceCfg,
                                          ConfigMan& destCfg,
                                          std::string& output)
{
  int updateCount = 0;

  for (const auto& value : _dynamicLoadables)
  {
    std::string oldValue;
    if (!destCfg.getValue(value.second, oldValue, value.first))
      CONFIG_MAN_LOG_KEY_ERROR(logger, value.second, value.first);

    std::string newValue;
    if (!sourceCfg.getValue(value.second, newValue, value.first))
      CONFIG_MAN_LOG_KEY_ERROR(logger, value.second, value.first);

    if (newValue == oldValue)
      continue;

    destCfg.setValue(value.second, newValue, value.first, true);
    output += "Changed " + value.first + ":" + value.second + " from " + oldValue + " to " +
              newValue + "\n";

    ++updateCount;
  }

  return updateCount;
}

bool
DynamicConfigLoader::updateDynamicConfig(std::string& output)
{
  if (Global::configUpdateInProgress())
    output = "Still using dual configurations, cannot load new configuration right now.\n";
  else if ((time(nullptr) - _lastUpdateAttempt) < _minUpdateInterval.getValue())
    output =
        "Too frequent configuration update attempts, cannot load new configuration right now.\n";
  else
  {
    _lastUpdateAttempt = time(nullptr);
    ConfigMan updatedCfg;
    if ((*_callback)(_cfgFile,
                     _cfgFileOverrideGroup,
                     _cfgFileOverrideKey,
                     _cfgFileSeparator.c_str(),
                     _cfgOverrides,
                     updatedCfg))
    {
      // first clone the current config, and then make sure only items under
      // [DYNAMIC_LOADABLE] are modified
      generateNewConfig(*Global::dynamicCfg(), Global::newDynamicCfg());
      const int updateCount = updateDynamicEntries(updatedCfg, *Global::newDynamicCfg(), output);

      if (updateCount)
      {
        output = boost::lexical_cast<std::string>(updateCount) + " entries updated:\n\n" + output;
        output += "\nConfiguration update was successful.";

        _lastUpdate = time(nullptr);
        Global::configUpdateInProgress() = true;
#ifdef CONFIG_HIERARCHY_REFACTOR
        _cfgBundle ^= 1;
        _cfgBundles[_cfgBundle].fill(*Global::newDynamicCfg());
        Global::configBundle().store(&_cfgBundles[_cfgBundle], std::memory_order_release);
#endif

        LOG4CXX_INFO(logger, output);
        return true;
      }
      else
        output = "No configuration changes were detected.\n";
    }
  }

  output += "Configuration update failed.";

  LOG4CXX_ERROR(logger, output);

  return false;
}

void
DynamicConfigLoader::generateNewConfig(const ConfigMan& sourceCfg, Global::configPtr& newCfg)
{
  std::vector<ConfigMan::NameValue> entries;
  Global::configPtr cfgPtr(new ConfigMan);

  if (!sourceCfg.getValues(entries))
    LOG4CXX_ERROR(logger, "DynamicConfigLoader::generateNewConfig - error getting config values");

  (*cfgPtr).setValues(entries);

  newCfg = cfgPtr;
}

void
DynamicConfigLoader::debugMsg()
{
  long count = Global::dynamicCfg().use_count() - 1;
  if (Global::dynamicCfg() == Global::newDynamicCfg())
    --count; // happens after an update

  std::stringstream msg;
  msg << "Current cfg usage: " << count;

  if (Global::configUpdateInProgress())
    msg << "; New cfg usage: " << Global::newDynamicCfg().use_count() - 1;

  LOG4CXX_DEBUG(logger, msg.str());
}

bool
DynamicConfigLoader::overrideConfigValue(Global::configPtr& config,
                                         const std::string& group,
                                         const std::string& name,
                                         const std::string& value,
                                         bool permament)
{
  if (!isDynamicLoadable(name, group))
    return false;

  if (!permament && (config == Global::dynamicCfg() || config == Global::newDynamicCfg()))
    generateNewConfig(*config, config);

  config->setValue(name, value, group, true);
  return true;
}

Global::configPtr
DynamicConfigLoader::cloneOverridenConfig(Global::configPtr& config)
{
  if (config == Global::dynamicCfg() || config == Global::newDynamicCfg())
  {
    // Config was not overriden (or overriden permamently), no cloning is necessary.
    return config;
  }

  Global::configPtr cloned;
  generateNewConfig(*config, cloned);
  return cloned;
}

void DynamicConfigLoader::stopThread()
{
  if (_instance)
  {
    delete _instance;
    _instance = nullptr;
  }
}

void
DynamicConfigLoader::onCancel()
{
  LOG4CXX_INFO(logger, "Dynamic Loader has been shut down.");
}

void
DynamicConfigLoader::run()
{
  if (IS_DEBUG_ENABLED(logger))
    debugMsg();

  if (!Global::configUpdateInProgress())
    return;

  const bool timeOut = ((time(nullptr) - _lastUpdate) > _keepOldConfigMaxWait.getValue());
  if (timeOut && !Global::dynamicCfg().unique()) // Should never happen
    LOG4CXX_FATAL(logger, "Max wait period passed, old cfg still had references.");

  if (!Global::dynamicCfg().unique() && !timeOut)
    return;

  Global::dynamicCfg() = Global::newDynamicCfg();
#ifdef CONFIG_HIERARCHY_REFACTOR
  _cfgBundles[_cfgBundle ^ 1].clear();
#else
  DynamicConfigurableValuesPool::reset();
#endif
  Global::configUpdateInProgress().store(false, std::memory_order_release);
}

}
