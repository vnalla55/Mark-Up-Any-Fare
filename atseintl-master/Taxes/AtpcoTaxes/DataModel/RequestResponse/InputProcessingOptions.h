// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/RequestResponse/InputApplyOn.h"
#include "DataModel/RequestResponse/InputExemptedRule.h"
#include "DataModel/RequestResponse/InputCalculationRestriction.h"
#include "DataModel/RequestResponse/InputTaxDetailsLevel.h"

namespace tax
{
class InputProcessingOptions
{
public:
  boost::ptr_vector<InputExemptedRule>& exemptedRules()
  {
    return _exemptedRules;
  };

  const boost::ptr_vector<InputExemptedRule>& exemptedRules() const
  {
    return _exemptedRules;
  };

  boost::ptr_vector<InputCalculationRestriction>& calculationRestrictions()
  {
    return _calculatioRestrictions;
  };

  const boost::ptr_vector<InputCalculationRestriction>& calculationRestrictions() const
  {
    return _calculatioRestrictions;
  };

  boost::optional<InputTaxDetailsLevel>& taxDetailsLevel() { return _taxDetailsLevel; }
  const boost::optional<InputTaxDetailsLevel>& taxDetailsLevel() const { return _taxDetailsLevel; }

  const std::vector<InputApplyOn>& getApplicableGroups() const { return _applicableGroups; }
  void addApplicableGroup(const InputApplyOn& apply) { _applicableGroups.push_back(apply); }
  // only for TestServer use
  std::vector<InputApplyOn>& getMutableApplicableGroups() { return _applicableGroups; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

  bool _tch {false};
  bool _rtw {false};
  bool _applyUSCAgrouping {true};
  bool _useRepricing {true};
  type::Index _geoPathsSize {0};
  type::Index _geoPathMappingsSize {0};
  type::Index _faresSize {0};
  type::Index _farePathsSize {0};
  type::Index _flightsSize {0};
  type::Index _flightPathsSize {0};
  type::Index _yqYrsSize {0};
  type::Index _yqYrPathsSize {0};
  type::Index _itinsSize {0};

private:
  boost::ptr_vector<InputExemptedRule> _exemptedRules;
  boost::ptr_vector<InputCalculationRestriction> _calculatioRestrictions;
  boost::optional<InputTaxDetailsLevel> _taxDetailsLevel;
  std::vector<InputApplyOn> _applicableGroups;
};

} // namespace tax

