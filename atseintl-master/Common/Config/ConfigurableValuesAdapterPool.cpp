//----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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


#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/ConfigurableValuesAdapterPool.h"

#include "Common/Config/ConfigurableValueAdapterBase.h"

namespace tse
{

FlatSet<ConfigurableValueAdapterBase*>& ConfigurableValuesAdapterPool::getAdapterPool()
{
  static FlatSet<ConfigurableValueAdapterBase*> pool;

  return pool;
}

void ConfigurableValuesAdapterPool::registerCfgValue(ConfigurableValueAdapterBase& adapter)
{
  getAdapterPool().insert(&adapter);
}

void ConfigurableValuesAdapterPool::unregisterCfgValue(ConfigurableValueAdapterBase& adapter)
{
  getAdapterPool().erase(&adapter);
}

void ConfigurableValuesAdapterPool::collectConfigMsg(const Trx& trx, std::vector<std::string>& messages)
{
  size_t oldSize = messages.size();

  for (ConfigurableValueAdapterBase* cvAdapter : ConfigurableValuesAdapterPool::getAdapterPool())
  {
    std::string msg = cvAdapter->getConfigMsg(&trx, true);
    if (!msg.empty())
      messages.push_back(std::move(msg));
  }

  std::sort(messages.begin() + oldSize, messages.end());
}

} // ns tse

#endif
