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

#include "Common/TaxDetailsLevel.h"
#include "DataModel/Common/SafeEnums.h"
#include "DomainDataObjects/CalculationRestriction.h"
#include "DomainDataObjects/ExemptedRule.h"

#include <vector>

namespace tax
{

class ProcessingOptions
{
public:
  std::vector<ExemptedRule>& exemptedRules()
  {
    return _exemptedRules;
  };

  const std::vector<ExemptedRule>& exemptedRules() const
  {
    return _exemptedRules;
  };

  std::vector<CalculationRestriction>& calculationRestrictions()
  {
    return _calculationRestrictions;
  };

  const std::vector<CalculationRestriction>& calculationRestrictions() const
  {
    return _calculationRestrictions;
  };

  const TaxDetailsLevel& taxDetailsLevel() const { return _taxDetailsLevel; }
  TaxDetailsLevel& taxDetailsLevel() { return _taxDetailsLevel; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

  bool isAllowed(const TaxName& taxName) const;
  type::CalcRestriction isExempted(const type::SabreTaxCode& taxCode) const;

  bool& tch() { return _tch; }
  const bool& tch() const { return _tch; }

  bool usingRepricing() const { return _useRepricing; }
  void setUseRepricing(bool use) { _useRepricing = use; }

  void setRtw(bool val) { _rtw = val; }
  bool isRtw() const { return _rtw; }

  bool& applyUSCAGrouping() { return _applyUSCAGrouping; }
  bool applyUSCAGrouping() const { return _applyUSCAGrouping; }

  const std::vector<type::ProcessingGroup>& getProcessingGroups() const { return _groups; }
  void setProcessingGroups(const std::vector<type::ProcessingGroup>& groups) { _groups = groups; }
private:
  std::vector<ExemptedRule> _exemptedRules;
  std::vector<CalculationRestriction> _calculationRestrictions;
  TaxDetailsLevel _taxDetailsLevel;
  bool _tch {false};
  bool _rtw {false};
  bool _applyUSCAGrouping {true};
  bool _useRepricing {true};
  std::vector<type::ProcessingGroup> _groups;
};

} // namespace tax
