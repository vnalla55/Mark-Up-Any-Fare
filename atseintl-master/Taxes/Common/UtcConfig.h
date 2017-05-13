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

#pragma once

#include <map>
#include <set>
#include <iosfwd>

namespace tse
{
class PricingTrx;
class TaxSpecConfigReg;

class UtcConfig
{
  PricingTrx& _trx;
  mutable std::map<std::string, std::set<std::string>> _config;

  UtcConfig(const UtcConfig&) = delete;
  UtcConfig& operator=(const UtcConfig&) = delete;

  bool checkDate(const TaxSpecConfigReg* configReg) const;

  std::set<std::string> split(const std::string& textToSplit) const;

  void addParam(const TaxSpecConfigReg* configReg);

public:
  explicit UtcConfig(PricingTrx& trx);

  virtual void readConfig(const std::string& configName);

  virtual const std::map<std::string, std::set<std::string>>& get() const;

  virtual const std::set<std::string>& get(const std::string& paramName) const;
};

std::ostream& operator<<(std::ostream& os, const UtcConfig& utcConfig);
}
