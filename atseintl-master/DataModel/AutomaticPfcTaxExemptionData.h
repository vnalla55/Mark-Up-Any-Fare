//----------------------------------------------------------------------------
//  Copyright Sabre 2013
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

#include "Common/TseCodeTypes.h"

#include <vector>

namespace tse
{

class AutomaticPfcTaxExemptionData
{
public:
  AutomaticPfcTaxExemptionData();

  enum ExemptionOption
  {
    EXEMPT_ALL,
    EXEMPT_INCLUDED,
    EXEMPT_EXCLUDED,
    NO_EXEMPT
  };

  typedef std::vector<TaxCode> ExemptionTaxCodes;

  bool& automaticPfcTaxExemptionEnabled() { return _automaticPfcTaxExemptionEnabled; }
  const bool& automaticPfcTaxExemptionEnabled() const { return _automaticPfcTaxExemptionEnabled; }

  ExemptionOption& taxExemptionOption() { return _taxExemptionOption; }
  const ExemptionOption& taxExemptionOption() const { return _taxExemptionOption; }

  ExemptionTaxCodes& exemptionTaxCodes() { return _exemptionTaxCodes; }
  const ExemptionTaxCodes& exemptionTaxCodes() const { return _exemptionTaxCodes; }

  ExemptionOption& pfcExemptionOption() { return _pfcExemptionOption; }
  const ExemptionOption& pfcExemptionOption() const { return _pfcExemptionOption; }

  bool firstTaxExempted();

private:
  bool _automaticPfcTaxExemptionEnabled;
  ExemptionOption _taxExemptionOption;
  ExemptionTaxCodes _exemptionTaxCodes;
  ExemptionOption _pfcExemptionOption;
  bool _firstTaxExempted;
};

} // namespace tse

