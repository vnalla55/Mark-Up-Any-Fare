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

#include "Common/Config/ConfigurableValuesPool.h"

#include <boost/algorithm/string/case_conv.hpp>

#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/ConfigurableValueBase.h"

namespace tse
{
template <bool dynamic>
FlatSet<ConfigurableValueBase*>&
ConfigurableValuesPoolBase<dynamic>::getPool()
{
  // For the purpose of unit tests: never destroy the pool.
  static const auto pool = new FlatSet<ConfigurableValueBase*>;
  return *pool;
}

template <bool dynamic>
void
ConfigurableValuesPoolBase<dynamic>::registerCfgValue(ConfigurableValueBase& cv)
{
  getPool().insert(&cv);
}

template <bool dynamic>
void
ConfigurableValuesPoolBase<dynamic>::unregisterCfgValue(ConfigurableValueBase& cv)
{
  getPool().erase(&cv);
}

template <bool dynamic>
ConfigValueSet
ConfigurableValuesPoolBase<dynamic>::gather()
{
  ConfigValueSet set;
  std::locale locale;

  for (const auto cv : getPool())
  {
    set.unsafe_emplace(boost::to_upper_copy(cv->getSection(), locale),
                       boost::to_upper_copy(cv->getOption(), locale));
  }

  set.order();
  return set;
}

template <bool dynamic>
void
ConfigurableValuesPoolBase<dynamic>::configure()
{
  for (const auto cv : getPool())
    cv->configure();
}

template <bool dynamic>
void
ConfigurableValuesPoolBase<dynamic>::reconfigure(const std::string& section,
                                                 const std::string& option)
{
  // maybe it will be better to replace foreach to find for performance
  for (const auto cv : getPool())
  {
    if (cv->getOption() == option && cv->getSection() == section)
      cv->configure();
  }
}

// instantiate both pools
template struct ConfigurableValuesPoolBase<false>;
template struct ConfigurableValuesPoolBase<true>;

} // ns tse

#else

#include "Common/Config/ConfigurableValue.h"
#include "Common/HandlesHolder.h"

namespace tse
{
template <bool>
static HandlesHolder<ConfigurableValueBase>&
getPool()
{
  // We want these containers to have infinite lifetime as they are referred from
  // global and static variables' destructors
  static HandlesHolder<ConfigurableValueBase>* pool = new HandlesHolder<ConfigurableValueBase>;
  return *pool;
}

template <bool dynamicOnly>
void
ConfigurableValuesPool<dynamicOnly>::registerCfgValue(ConfigurableValueBase& cv)
{
  getPool<dynamicOnly>().insert(&cv);
}

template <bool dynamicOnly>
void
ConfigurableValuesPool<dynamicOnly>::unregisterCfgValue(ConfigurableValueBase& cv)
{
  getPool<dynamicOnly>().erase(&cv);
}

template <bool dynamicOnly>
ConfigValueSet
ConfigurableValuesPool<dynamicOnly>::gather()
{
  ConfigValueSet set;
  std::locale locale;

  getPool<dynamicOnly>().forEach([&](ConfigurableValueBase* cv)
  {
    set.unsafe_emplace(boost::to_upper_copy(cv->getSection(), locale),
                       boost::to_upper_copy(cv->getOption(), locale));
  });

  set.order();
  return set;
}

template <bool dynamicOnly>
void
ConfigurableValuesPool<dynamicOnly>::collectConfigMsg(const Trx& trx,
                                                      std::vector<std::string>& messages)
{
  size_t oldSize = messages.size();

  getPool<dynamicOnly>().forEach([&](ConfigurableValueBase* cv)
  {
   std::string msg = cv->getConfigMsg(&trx, dynamicOnly);
   if (!msg.empty())
     messages.push_back(msg);
  });

  std::sort(messages.begin() + oldSize, messages.end());
}

template <bool dynamicOnly>
void
ConfigurableValuesPool<dynamicOnly>::reset()
{
  getPool<dynamicOnly>().forEach([](ConfigurableValueBase* cv)
  {
    cv->resetConfigured(dynamicOnly);
  });
}

template <bool dynamicOnly>
void
ConfigurableValuesPool<dynamicOnly>::reset(const std::string& section, const std::string& option)
{
  getPool<dynamicOnly>().forEach([&](ConfigurableValueBase* cv)
                                 {
                                   if (cv->getOption() == option)
                                     if (cv->getSection() == section)
                                       cv->resetConfigured(dynamicOnly);
                                 });
}

// instantiate both pools
template class ConfigurableValuesPool<false>;
template class ConfigurableValuesPool<true>;

}

#endif
