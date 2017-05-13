// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/Types.h"

#include <boost/noncopyable.hpp>
#include <vector>

namespace tax
{

class ConfigService : public boost::noncopyable
{
public:
  ConfigService() :
    _hpsDomGroupingEnabled(false),
    _hpsIntlGroupingEnabled(false),
    _hpsUseFlightRanges(false)
  {}
  virtual ~ConfigService(void) {}

  void setHpsDomGroupingEnabled(bool value) { _hpsDomGroupingEnabled = value; }
  void setHpsIntlGroupingEnabled(bool value) { _hpsIntlGroupingEnabled = value; }
  void setHpsUseFlightRanges(bool value) { _hpsUseFlightRanges = value; }

  bool getHpsDomGroupingEnabled() const { return _hpsDomGroupingEnabled; }
  bool getHpsIntlGroupingEnabled() const { return _hpsIntlGroupingEnabled; }
  bool getHpsUseFlightRanges() const { return _hpsUseFlightRanges; }

  std::vector<type::Nation>& travelDateDependantTaxNations()
  {
    return _travelDateDependantTaxNations;
  }
  const std::vector<type::Nation>& travelDateDependantTaxNations() const
  {
    return _travelDateDependantTaxNations;
  }

  std::vector<type::Nation>& flightNoDependantTaxNations()
  {
    return _flightNoDependantTaxNations;
  }
  const std::vector<type::Nation>& flightNoDependantTaxNations() const
  {
    return _flightNoDependantTaxNations;
  }

  std::vector<type::Nation>& sameDayDepartureTaxNations()
  {
    return _sameDayDepartureTaxNations;
  }
  const std::vector<type::Nation>& sameDayDepartureTaxNations() const
  {
    return _sameDayDepartureTaxNations;
  }

private:
  bool _hpsDomGroupingEnabled;
  bool _hpsIntlGroupingEnabled;
  bool _hpsUseFlightRanges;

  std::vector<type::Nation> _travelDateDependantTaxNations;
  std::vector<type::Nation> _flightNoDependantTaxNations;
  std::vector<type::Nation> _sameDayDepartureTaxNations;
};
}
