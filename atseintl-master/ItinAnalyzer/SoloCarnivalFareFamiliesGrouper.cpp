// Copyright Sabre 2011
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.

#include "ItinAnalyzer/SoloCarnivalFareFamiliesGrouper.h"

#include "Common/Config/ConfigurableValue.h"
#include "ItinAnalyzer/SoloCarnivalFareFamiliesGrouperImpl.h"

namespace tse
{
namespace
{
ConfigurableValue<bool>
regroupThruFamilies("SOLO_CARNIVAL_OPT", "REGROUP_THRU_FAMILIES", false);
}

void
SoloCarnivalFareFamiliesGrouper::group(std::vector<tse::Itin*>& input)
{
  if (regroupThruFamilies.getValue())
  {
    SoloCarnivalFareFamiliesGrouperImpl::groupThruFamilies(input);
  }
  else
  {
    SoloCarnivalFareFamiliesGrouperImpl::groupWithinFamilies(input);
  }
}
} // namespace
