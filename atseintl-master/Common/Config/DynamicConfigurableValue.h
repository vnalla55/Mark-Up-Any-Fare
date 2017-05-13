//----------------------------------------------------------------------------
//
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
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/ConfigurableValueUtil.h"
#include "Common/Config/TypedConfigurableValueBase.h"
#include "Common/Global.h"
#include "DataModel/Trx.h"

#include <limits>
#include <stdint.h>

namespace tse
{
void allocateAllConfigBundles();

template <typename T>
class DynamicConfigurableValue : public TypedConfigurableValueBase<T>
{
  friend void allocateAllConfigBundles();
  typedef TypedConfigurableValueBase<T> CVBaseImpl;

public:
  DynamicConfigurableValue(const std::string& section,
                           const std::string& option,
                           const T& defaultValue = T())
    : TypedConfigurableValueBase<T>(section, option, defaultValue)
  {
    DynamicConfigurableValuesPool::registerCfgValue(*this);
    DynamicConfigurableValuesBundlePool<T>::registerInBundle(*this);
  }

  ~DynamicConfigurableValue()
  {
    DynamicConfigurableValuesBundlePool<T>::unregisterInBundle(*this);
    DynamicConfigurableValuesPool::unregisterCfgValue(*this);
  }

  const T& getValue(const Trx* trx) override final
  {
    trx = ConfigurableValueUtil::getTrx(trx, true);
    return trx->configBundle().get<T>(_index);
  }

  void configure() override final {}

  bool isDynamic() const override final { return true; }
  uint32_t index() const { return _index; }

  // Used only in tests.
  void set(const T& value)
  {
    Global::configBundle().load(std::memory_order_relaxed)->get<T>(_index) = value;
  }

private:
  uint32_t _index = std::numeric_limits<uint32_t>::max();
};

} // tse
#else

#include <string>

namespace tse
{
class ConfigMan;
class Trx;

class DynamicConfigurableValue
{
public:
  DynamicConfigurableValue() = default;
  DynamicConfigurableValue(const DynamicConfigurableValue&) = delete;
  DynamicConfigurableValue& operator=(const DynamicConfigurableValue&) = delete;

protected:
  ~DynamicConfigurableValue() = default;

  static const Trx* getTrx(const Trx* trx);
  static void checkTrx(const Trx* trx);
  bool shouldReadFromConfig(const Trx*& trx);
  const ConfigMan& configureGetConfigSource(const std::string& section, const std::string& name);
};

} // tse

#endif
