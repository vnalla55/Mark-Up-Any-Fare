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

#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/FallbackValue.h"

namespace tse
{
class ConfigMan;

namespace fallback
{

class DynamicConfigurableFallbackValue : public DynamicConfigurableFlagOn
{
  friend class DynamicConfigurableFallbackValueTest;

public:
  DynamicConfigurableFallbackValue(const std::string& option, bool value)
    : DynamicConfigurableFlagOn(FallbackValue::CONFIG_SECTION, option, value)
  {
  }

  bool get(const Trx* trx) { return getValue(trx); }
};

} // fallback

} // tse

#define FALLBACK_DEF(_name_, _config_label_, _value_)                                              \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  DynamicConfigurableFallbackValue _name_(_config_label_, _value_);                                \
  }                                                                                                \
  bool _name_##_old(const Trx* trx) { return value::_name_.get(trx); }                             \
  }

#else

#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/FallbackValueBase.h"

namespace tse
{

namespace fallback
{

class DynamicConfigurableFallbackValue : public FallbackValueBase,
                                         protected DynamicConfigurableFlagOn
{
  friend class DynamicConfigurableFallbackValueTest;

public:
  DynamicConfigurableFallbackValue(const std::string& option, bool value)
    : DynamicConfigurableFlagOn(CONFIG_SECTION, option, value)
  {
  }

  bool get(const Trx* trx) { return getValue(trx); }
  void set(bool value);
  virtual void set(const ConfigMan* config) override;
};

} // fallback

} // tse

#define FALLBACK_DEF(_name_, _config_label_, _value_)                                              \
  namespace fallback                                                                               \
  {                                                                                                \
  namespace value                                                                                  \
  {                                                                                                \
  DynamicConfigurableFallbackValue _name_(_config_label_, _value_);                                \
  }                                                                                                \
  bool _name_(const Trx* trx) { return value::_name_.get(trx); }                                   \
  }

#endif
