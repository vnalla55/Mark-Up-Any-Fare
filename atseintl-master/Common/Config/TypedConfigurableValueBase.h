#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigurableValueBase.h"
#include "Common/TypeConvert.h"

#include <string>

namespace tse
{
class Trx;

template <typename T>
class TypedConfigurableValueBase : public ConfigurableValueBase
{
public:
  TypedConfigurableValueBase(const std::string& section,
                            const std::string& option,
                            const T& defaultValue = T())
    : ConfigurableValueBase(section, option), _defaultValue(defaultValue)
  {
  }

  using ValueType = T;

  virtual const T& getValue(const Trx* trx) = 0;

  void getValueFrom(const ConfigMan& config, T& output)
  {
    GetContext ctx;
    getValueFrom(config, ctx);
    output = std::move(ctx.value);
  }

  void logIfValueHasChanged(const T& oldValue, const T& newValue) {}

protected:
  std::string getValueString(const Trx* trx) override final
  {
    return TypeConvert::valueToString(getValue(trx));
  }

  struct GetContext
  {
    std::string rawValue;
    T value;
    bool usingDefault;
  };

  void getValueFrom(const ConfigMan& config, GetContext& ctx)
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

  T _defaultValue;
};

template <>
inline void
TypedConfigurableValueBase<bool>::logIfValueHasChanged(const bool& oldValue, const bool& newValue)
{
  if (newValue != oldValue)
    LOG4CXX_INFO(_tseServerlogger, "Setting " << _option << " - " << (newValue ? "On" : "Off"));
}

} // tse
