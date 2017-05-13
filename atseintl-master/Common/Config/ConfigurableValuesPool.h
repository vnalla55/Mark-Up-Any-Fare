//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include <string>
#include <utility>
#include <vector>

#include "Util/FlatSet.h"

namespace tse
{
class ConfigurableValueBase;
class Trx;

using ConfigValueSet = FlatSet<std::pair<std::string, std::string>>;

#ifdef CONFIG_HIERARCHY_REFACTOR

template <bool dynamic>
struct ConfigurableValuesPoolBase
{
  static FlatSet<ConfigurableValueBase*>& getPool();
  static void registerCfgValue(ConfigurableValueBase& cv);
  static void unregisterCfgValue(ConfigurableValueBase& cv);
  static ConfigValueSet gather();
  static void configure();
  static void reconfigure(const std::string& section, const std::string& option);
};

using ConfigurableValuesPool = ConfigurableValuesPoolBase<false>;
using DynamicConfigurableValuesPool = ConfigurableValuesPoolBase<true>;

#else

template<bool dynamicOnly>
struct ConfigurableValuesPool
{
  static void registerCfgValue(ConfigurableValueBase& cv);
  static void unregisterCfgValue(ConfigurableValueBase& cv);
  static ConfigValueSet gather();
  static void collectConfigMsg(const Trx& trx, std::vector<std::string>& messages);
  static void reset();
  static void reset(const std::string& section, const std::string& option);
};

typedef ConfigurableValuesPool<false> AllConfigurableValuesPool;
typedef ConfigurableValuesPool<true> DynamicConfigurableValuesPool;

#endif

} // ns tse
