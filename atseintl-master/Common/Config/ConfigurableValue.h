// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/TypedConfigurableValueBase.h"
#include "Common/Global.h"

namespace tse
{
class Trx;

template <typename T>
class ConfigurableValue : public TypedConfigurableValueBase<T>
{
  typedef TypedConfigurableValueBase<T> CVBaseImpl;

public:
  ConfigurableValue(const std::string& section,
                    const std::string& option,
                    const T& defaultValue = T())
    : TypedConfigurableValueBase<T>(section, option, defaultValue),
      _value(defaultValue)
  {
    ConfigurableValuesPool::registerCfgValue(*this);
  }

  ~ConfigurableValue()
  {
    ConfigurableValuesPool::unregisterCfgValue(*this);
  }

  const T& getValue() const { return _value; }
  const T& getValue(const Trx* /*trx*/) override final { return getValue(); }

  const std::string& getRawValue() const { return _rawValue; }
  bool isDefault() const { return _usingDefault; }

  bool set(const tse::ConfigMan* config)
  {
    const T prevValue = _value;

    CVBaseImpl::getValueFrom(*config, _value);

    return _value != prevValue;
  }

  void set(const T& value) { _value = value; }

  void configure() override final
  {
    {
      typename CVBaseImpl:: GetContext ctx;
      this->getValueFrom(Global::config(), ctx);
      this->logIfValueHasChanged(_value, ctx.value);

      _value = std::move(ctx.value);
      _rawValue = std::move(ctx.rawValue);
      _usingDefault = ctx.usingDefault;
    }

    LOG4CXX_INFO(CVBaseImpl::_logger,
                 CVBaseImpl::_section << ":" << CVBaseImpl::_option
                 << " set to: " << TypeConvert::valueToString(_value));
  }

  bool isDynamic() const override final { return false; }

private:
  T _value;
  std::string _rawValue;
  bool _usingDefault = true;
};

} // tse

#else

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TypeConvert.h"

#include <atomic>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

namespace tse
{
class Trx;

class ConfigurableValueBase
{
public:
  ConfigurableValueBase(const std::string& section, const std::string& option)
    : _section(section), _option(option)
  {
  }

  ConfigurableValueBase(const ConfigurableValueBase&) = delete;
  ConfigurableValueBase& operator=(const ConfigurableValueBase&) = delete;

  void resetConfigured(const bool dynamicOnly);
  std::string getConfigMsg(const Trx* trx, const bool dynamicOnly);
  const std::string& getSection() const { return _section; }
  const std::string& getOption() const { return _option; }

  // These 2 are initialized only after first getValue() call.
  // Please note that they won't be initialized in DynamicConfigurablueValue.
  const std::string& getRawValue() const { return _rawValue; }
  bool isDefault() const { return _usingDefault; }

protected:
  ~ConfigurableValueBase() = default;

  virtual bool isDynamic() const { return false; }
  virtual std::string getConfigMsgImpl(const Trx* trx) = 0;

  const std::string _section;
  const std::string _option;
  std::string _rawValue;
  std::atomic<bool> _isConfigured{false};
  bool _usingDefault;
  std::mutex _mutex;

  static Logger _logger;
};

// Change Self_ when deriving from this class.
template <typename T, typename Self_ = void>
class ConfigurableValue : public ConfigurableValueBase
{
  typedef typename std::conditional<std::is_same<Self_, void>::value,
                                    ConfigurableValue<T>,
                                    Self_>::type Self;

public:
  ConfigurableValue(const std::string& section,
                    const std::string& option,
                    const T& defaultValue = T())
    : ConfigurableValueBase(section, option), _value(defaultValue), _defaultValue(defaultValue)
  {
    AllConfigurableValuesPool::registerCfgValue(*this);
  }

  ~ConfigurableValue() { AllConfigurableValuesPool::unregisterCfgValue(*this); }

  const T& getValue();

protected:
  Self& self() { return static_cast<Self&>(*this); }

  struct GetContext
  {
    std::string rawValue;
    T value;
    bool usingDefault;
  };

  void getValueFrom(const ConfigMan& config, GetContext& output);
  void getValueFrom(const ConfigMan& config, T& output);
  const ConfigMan& configureGetConfigSource();
  void configure();

  virtual std::string getConfigMsgImpl(const Trx* trx) override;

  T _value;
  T _defaultValue;
};

template <typename T, typename S>
inline const T&
ConfigurableValue<T, S>::getValue()
{
  if (!_isConfigured.load(std::memory_order_acquire))
    self().configure();

  return _value;
}

template <typename T, typename S>
inline void
ConfigurableValue<T, S>::getValueFrom(const ConfigMan& config, GetContext& ctx)
{
  if (!config.getValue(_option, ctx.rawValue, _section))
  {
    LOG4CXX_ERROR(_logger,
                  "Missing value in " << _section << ":" << _option
                                      << ". Using default data instead.");
    ctx.usingDefault = true;
    ctx.value = _defaultValue;
    return;
  }
  if (!TypeConvert::stringToValue(ctx.rawValue, ctx.value))
  {
    LOG4CXX_ERROR(_logger,
                  "Invalid value " << ctx.rawValue << " in " << _section << ":" << _option
                                   << ". Using default data instead.");
    ctx.usingDefault = true;
    ctx.value = _defaultValue;
    return;
  }
  ctx.usingDefault = false;
}

template <typename T, typename S>
inline void
ConfigurableValue<T, S>::getValueFrom(const ConfigMan& config, T& output)
{
  GetContext ctx;
  getValueFrom(config, ctx);
  output = std::move(ctx.value);
}

template <typename T, typename S>
inline const ConfigMan&
ConfigurableValue<T, S>::configureGetConfigSource()
{
  return Global::config();
}

template <typename T, typename S>
inline void
ConfigurableValue<T, S>::configure()
{
  std::lock_guard<std::mutex> guard(_mutex);

  if (_isConfigured.load(std::memory_order_acquire))
    return;

  {
    GetContext ctx;
    self().getValueFrom(self().configureGetConfigSource(), ctx);

    _rawValue = std::move(ctx.rawValue);
    _value = std::move(ctx.value);
    _usingDefault = ctx.usingDefault;
  }

  LOG4CXX_INFO(_logger,
               _section << ":" << _option << " set to: " << TypeConvert::valueToString(_value));

  _isConfigured.store(true, std::memory_order_release);
}

template <typename T, typename S>
std::string
ConfigurableValue<T, S>::getConfigMsgImpl(const Trx* trx)
{
  std::string configMsg(this->_section);
  configMsg += '/';
  configMsg += this->_option;
  configMsg += '/';
  configMsg += TypeConvert::valueToString(getValue());
  std::replace(configMsg.begin(), configMsg.end(), '_', ' ');
  return configMsg;
}
}

#endif
