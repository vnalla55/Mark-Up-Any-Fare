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

#include "Common/Config/ConfigurableValue.h"

#include <string>

namespace tse
{
class ConfigMan;
class Logger;

namespace fallback
{

class FallbackValue : public ConfigurableValue<bool>
{
  friend class FallbackValueTest;

public:
  FallbackValue(const std::string& option, bool value)
    : ConfigurableValue<bool>(FallbackValue::CONFIG_SECTION, option, value)
  {
  }

  static const std::string CONFIG_SECTION;

  bool get() const { return getValue(); }
};

} // fallback

} // tse

#define FIXEDFALLBACK_DEF(_name_, _config_label_, _value_)                                         \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace fixed                                                                                  \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  FallbackValue _name_(_config_label_, _value_);                                                   \
  }                                                                                                \
  bool _name_##_old() { return value::_name_.get(); }                                              \
  }                                                                                                \
  }

#else

#include "Common/Config/FallbackValueBase.h"

#include <string>

namespace tse
{

namespace fallback
{

class FallbackValue : public FallbackValueBase
{
  friend class FallbackValueTest;

public:
  FallbackValue(const std::string& option, bool value) : _option(option), _value(value) {}

  bool get() const { return _value; }
  void set(bool value) { _value = value; }
  virtual void set(const tse::ConfigMan* config) override;

private:
  std::string _option;
  bool _value;
};

} // fallback

} // tse

#define FIXEDFALLBACK_DEF(_name_, _config_label_, _value_)                                         \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace fixed                                                                                  \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  FallbackValue _name_(_config_label_, _value_);                                                   \
  }                                                                                                \
  bool _name_() { return value::_name_.get(); }                                                    \
  }                                                                                                \
  }

#endif
