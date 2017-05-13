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

#ifndef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/DynamicConfigurableValue.h"
#include "DataModel/Trx.h"

namespace tse
{
template <typename T, typename P, typename R>
class DynamicConfigurableValueImpl
    : public ConfigurableValue<T, DynamicConfigurableValueImpl<T, P, R> >,
      public DynamicConfigurableValue
{
  friend class ConfigurableValue<T, DynamicConfigurableValueImpl<T, P, R> >;
  friend class DynamicConfigurableValueTest;
  typedef ConfigurableValue<T, DynamicConfigurableValueImpl<T, P, R> > CVBase;

public:
  DynamicConfigurableValueImpl(const std::string& section,
                               const std::string& option,
                               const T& value)
    : CVBase(section, option, value)
  {
    DynamicConfigurableValuesPool::registerCfgValue(*this);
  }

  ~DynamicConfigurableValueImpl() { DynamicConfigurableValuesPool::unregisterCfgValue(*this); }

  bool isValid(const Trx* trx)
  {
    trx = getTrx(trx);
    checkTrx(trx);
    R referenceValue;
    return isValid(trx, referenceValue(trx));
  }

  bool isValid(const Trx* trx, const T& valueToCheck);

  T getValue(const Trx* trx);

private:
  const ConfigMan& configureGetConfigSource();
  bool isDynamic() const override;
  std::string getConfigMsgImpl(const Trx* trx) override;

  P _predicate;
};

template <typename T, typename P, typename R>
bool
DynamicConfigurableValueImpl<T, P, R>::isValid(const Trx* trx, const T& valueToCheck)
{
  return _predicate(valueToCheck, getValue(trx));
}

template <typename T, typename P, typename R>
T
DynamicConfigurableValueImpl<T, P, R>::getValue(const Trx* trx)
{
  if (LIKELY(!shouldReadFromConfig(trx)))
    return CVBase::getValue();

  // trx will now be non-null.

  T dynamicValue;
  this->getValueFrom(*trx->dynamicCfg(), dynamicValue);
  return dynamicValue;
}

template <typename T, typename P, typename R>
const ConfigMan&
DynamicConfigurableValueImpl<T, P, R>::configureGetConfigSource()
{
  return DynamicConfigurableValue::configureGetConfigSource(this->_section, this->_option);
}

template <typename T, typename P, typename R>
bool
DynamicConfigurableValueImpl<T, P, R>::isDynamic() const
{
  return true;
}

template <typename T, typename P, typename R>
std::string
DynamicConfigurableValueImpl<T, P, R>::getConfigMsgImpl(const Trx* trx)
{
  std::string configMsg(CVBase::getConfigMsgImpl(trx));
  configMsg += '/';
  configMsg += (isValid(trx) ? "APPLIED" : "NOT APPLIED");
  return configMsg;
}

} // tse

#endif


