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
#include "Rules/RulesBuilder.h"

namespace tax
{

class BaggageRulesBuilder : public RulesBuilder
{
public:
  BaggageRulesBuilder(const RulesRecord& rulesRecord,
                      const TaxableUnitTagSet& taxableUnits);
  void addValidators() override;
  void addCalculators() override;
};

} /* namespace tax */
