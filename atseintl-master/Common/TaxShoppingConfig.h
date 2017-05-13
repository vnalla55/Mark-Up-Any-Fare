#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"

#include <vector>

namespace tse
{

class ConfigMan;

class TaxShoppingConfig
{
 public:
  TaxShoppingConfig(ConfigMan& config)
  {
    init(config);
  }

  const NationCodesVec& getTvlDateDepTaxNations() const
  {
    return _tvlDateDepTaxNations;
  }
  const NationCodesVec& getFltNoDepTaxNations() const
  {
    return _fltNoDepTaxNations;
  }
  const NationCodesVec& getSameDayDepTaxNations() const
  {
    return _sameDayDepTaxNations;
  }

  const TaxCodesVec& roundTransitMinutesTaxCodes() const { return _roundTransitMinutesTaxCodes; }

 protected:
  TaxShoppingConfig() {}
  void init(ConfigMan& config);
 private:
  NationCodesVec _tvlDateDepTaxNations;
  NationCodesVec _fltNoDepTaxNations;
  NationCodesVec _sameDayDepTaxNations;
  TaxCodesVec    _roundTransitMinutesTaxCodes;
};

}

