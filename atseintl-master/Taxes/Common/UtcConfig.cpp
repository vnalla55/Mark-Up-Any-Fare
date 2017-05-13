// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Taxes/Common/UtcConfig.h"

#include <boost/algorithm/string.hpp>
#include <iostream>

namespace tse
{
namespace
{
const std::string SABRE_TAX_CODE_SEPARATOR{"|"};
}

UtcConfig::UtcConfig(PricingTrx& trx) : _trx(trx)
{
}

bool
UtcConfig::checkDate(const TaxSpecConfigReg* configReg) const
{
  return configReg->effDate() <= _trx.ticketingDate() &&
         configReg->discDate().date() >= _trx.ticketingDate().date();
}

std::set<std::string>
UtcConfig::split(const std::string& textToSplit) const
{
  std::set<std::string> result;
  boost::split(result, textToSplit, boost::is_any_of(SABRE_TAX_CODE_SEPARATOR));
  return result;
}

void
UtcConfig::addParam(const TaxSpecConfigReg* configReg)
{
  for (const auto& each : configReg->seqs())
  {
    _config[each->paramName()] = split(each->paramValue());
  }
}

void
UtcConfig::readConfig(const std::string& configName)
{
  DataHandle dataHandle(_trx.ticketingDate());
  dataHandle.setParentDataHandle(&_trx.dataHandle());

  for (const auto& each : dataHandle.getTaxSpecConfig(configName))
  {
    if (!checkDate(each))
      continue;

    addParam(each);
  }
}

const std::map<std::string, std::set<std::string>>&
UtcConfig::get() const
{
  return _config;
}

const std::set<std::string>&
UtcConfig::get(const std::string& paramName) const
{
  return _config[paramName];
}

std::ostream& operator<<(std::ostream& os, const UtcConfig& utcConfig)
{
  for (const auto& param : utcConfig.get())
  {
    os << param.first << "= [";
    for (const auto& val : param.second)
    {
      os << val << ' ';
    }
    os << "]\n";
  }

  return os;
}
}
