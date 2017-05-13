#pragma once

#include "Common/Logger.h"

#include <string>

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

  std::string getConfigMsg(const Trx* trx, const bool dynamicOnly);

  const std::string& getSection() const { return _section; }
  const std::string& getOption() const { return _option; }

  virtual void configure() = 0;
  virtual bool isDynamic() const = 0;

protected:
  virtual ~ConfigurableValueBase() {}

  ConfigurableValueBase(const ConfigurableValueBase&) = delete;
  ConfigurableValueBase& operator=(const ConfigurableValueBase&) = delete;

  virtual std::string getValueString(const Trx* trx) = 0;

  const std::string _section;
  const std::string _option;

  static Logger _logger;
  static Logger _tseServerlogger;
};
}
