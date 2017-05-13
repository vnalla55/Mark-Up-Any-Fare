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

#include "Xform/DynamicConfig.h"

#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TypeConvert.h"
#include "DataModel/Trx.h"

#include <boost/algorithm/string/replace.hpp>

#include <algorithm>
#include <stdexcept>

namespace tse
{
namespace
{
Logger
logger("atseintl.Xform.DynamicConfig");

class DynamicConfigError : public std::runtime_error
{
public:
  DynamicConfigError(const std::string& msg)
    : std::runtime_error("Error while parsing DynamicConfig tag: " + msg) {}
};
}

DynamicConfigInput::DynamicConfigInput()
  : _name(),
    _value(),
    _substitute("T"),
    _optional("N")
{
}

const std::string DynamicConfigHandler::DEFAULT_GROUP = "FALLBACK_SECTION";

bool
DynamicConfigHandler::check()
{
  return TrxUtil::isDynamicConfigOverrideEnabled(_trx);
}

void
DynamicConfigHandler::process(const DynamicConfigInput& input)
{
  try
  {
    parse(input);
    apply();
  }
  catch (DynamicConfigError& e)
  {
    LOG4CXX_WARN(logger, e.what());
    if (!_optional)
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, e.what());
  }
}

void
DynamicConfigHandler::parse(const DynamicConfigInput& input)
{
  // Parse it first to catch exceptions.
  parseOptional(input);

  // Parse it before Name and Value since it uses substitute value.
  parseSubstitute(input);

  parseNameValue(input);
}

void DynamicConfigHandler::apply()
{
  /*
     By default, <DynamicConfig> elements override the config for a single
     transaction only. However, due to server inner workings, this makes the
     request exceptionally slow (2-5x slower than usual). In many cases this
     disqualifies the DynamicConfig.

     Setting TSE_SERVER::DYNAMIC_CONFIG_OVERRIDE_PERMAMENT to Y selects
     another mode of DynamicConfig - permament changes. With this,
     <DynamicConfig> elements override the config globally and permamently.

     This method is way faster (negligible performance impact of
     DynamicConfig) but is inflexible and dangerous - especially when many
     concurrent requests are processed by the server. Due to this, it won't
     ever be enabled on any public server. You can, however, enable it locally
     and benefit from added performance, providing you process only one
     request at a time and take into account that the override changes are now
     permament.
  */
  const bool permament = TrxUtil::isDynamicConfigOverridePermament(_trx);

  if (!DynamicConfigLoader::overrideConfigValue(
      _trx.dynamicCfg(), _group, _name, _value, permament))
  {
    throw DynamicConfigError(_group + "\\" + _name + ": No such dynamic config");
  }

  if (permament)
  {
    LOG4CXX_ERROR(logger, "!!!!!! Permament change of " << _group << "::" << _name << " to '"
        << _value << "'. It will affect concurrent/future requests too!");

#ifdef CONFIG_HIERARCHY_REFACTOR
    assert(!Global::configUpdateInProgress().load(std::memory_order_relaxed));
    Global::configBundle().load(std::memory_order_relaxed)->fill(*Global::dynamicCfg());
#else
    DynamicConfigurableValuesPool::reset();
#endif

  }
  else
  {
#ifdef CONFIG_HIERARCHY_REFACTOR
    _trx.mutableConfigBundle().makeUnique();
    _trx.mutableConfigBundle().update(*_trx.dynamicCfg(), _group, _name);
#endif
    _trx.setDynamicCfgOverriden(true);
  }
}

void
DynamicConfigHandler::parseNameValue(const DynamicConfigInput& input)
{
  _group = DEFAULT_GROUP;
  _name = input.name();
  _value = input.value();

  if (_substitute)
  {
    std::replace(_name.begin(), _name.end(), ' ', '_');
    std::replace(_name.begin(), _name.end(), '/', '\\');
    boost::replace_all(_name, "::", "\\");
  }

  {
    const size_t sepPos = _name.find('\\');
    if (sepPos != std::string::npos)
    {
      _group.assign(_name, 0, sepPos);
      _name.erase(0, sepPos + 1);
    }
  }

  {
    const size_t sepPos = _name.rfind('\\');
    if (sepPos != std::string::npos)
    {
      if (!_value.empty())
        throw DynamicConfigError("Value attribute should be empty");

      _value.assign(_name, sepPos + 1, std::string::npos);
      _name.erase(sepPos);
    }
  }

  if (_name.empty())
    throw DynamicConfigError("Invalid value of Name attribute");

  if (_value.empty())
    throw DynamicConfigError("Invalid value of Value attribute");
}

void DynamicConfigHandler::parseSubstitute(const DynamicConfigInput& input)
{
  if (!TypeConvert::isBool(input.substitute()))
    throw DynamicConfigError("Invalid value of Substitute attribute");
  _substitute = TypeConvert::pssCharToBool(input.substitute()[0]);
}

void DynamicConfigHandler::parseOptional(const DynamicConfigInput& input)
{
  if (!TypeConvert::isBool(input.optional()))
    throw DynamicConfigError("Invalid value of Optional attribute");
  _optional = TypeConvert::pssCharToBool(input.optional()[0]);
}

}
