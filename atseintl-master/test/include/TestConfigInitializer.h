//----------------------------------------------------------------------------
//  Copyright Sabre 2011
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
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/FallbackValueBase.h"
#include "Common/Global.h"

class TestConfigInitializer
{
public:
  TestConfigInitializer(tse::ConfigMan* config)
  {
    assert(!created());
    created() = true;

    GlobalImpl::_configMan = config;
    tse::Global::configPtr sharedPointer(config, [](tse::ConfigMan*){}); // Don't delete.
    GlobalImpl::_dynamicCfg = sharedPointer;
    GlobalImpl::_newDynamicCfg = sharedPointer;
#ifdef CONFIG_HIERARCHY_REFACTOR
    tse::ConfigurableValuesPool::configure();
    tse::allocateAllConfigBundles();
    _configBundle.fill(*config);
    GlobalImpl::_configBundle.store(&_configBundle, std::memory_order_relaxed);
#else
    tse::fallback::configure(*config);
#endif
  }

  TestConfigInitializer() : TestConfigInitializer(&_config) {}

  ~TestConfigInitializer()
  {
    assert(created());
    created() = false;

    GlobalImpl::_configMan = nullptr;
    GlobalImpl::_dynamicCfg = nullptr;
    GlobalImpl::_newDynamicCfg = nullptr;
#ifdef CONFIG_HIERARCHY_REFACTOR
    GlobalImpl::_configBundle.store(nullptr, std::memory_order_relaxed);
#else
    tse::AllConfigurableValuesPool::reset();
#endif
  }

  template <typename T>
  static bool setValue(const std::string& name,
                       const T& value,
                       const std::string& group,
                       const bool overwrite = true,
                       const bool reconfigure = true)
  {
#ifdef CONFIG_HIERARCHY_REFACTOR
    if (GlobalImpl::config().setValue(name, value, group, overwrite))
    {
      if (reconfigure)
      {
        tse::ConfigurableValuesPool::reconfigure(group, name);
        if (const auto configBundle = GlobalImpl::_configBundle.load(std::memory_order_relaxed))
          configBundle->update(*GlobalImpl::_dynamicCfg, group, name);
      }
      return true;
    }
    return false;
#else
    tse::AllConfigurableValuesPool::reset(group, name);
    return GlobalImpl::config().setValue(name, value, group, overwrite);
#endif
  }

protected:
  class GlobalImpl : public tse::Global
  {
  public:
    using Global::_configMan;
    using Global::_dynamicCfg;
    using Global::_newDynamicCfg;
#ifdef CONFIG_HIERARCHY_REFACTOR
    using Global::_configBundle;
#endif
  };

  tse::ConfigMan _config;
#ifdef CONFIG_HIERARCHY_REFACTOR
  tse::ConfigBundle _configBundle;
#endif

  static bool& created()
  {
    static bool c = false;
    return c;
  }
};

