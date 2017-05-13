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
#include "DataModel/RequestResponse/InputProcessingOptions.h"
#include "DomainDataObjects/ProcessingOptions.h"
#include "Factories/ExemptedRuleFactory.h"
#include "Factories/CalculationRestrictionFactory.h"
#include "Factories/FactoryUtils.h"
#include "Factories/ProcessingOptionsFactory.h"

namespace tax
{

namespace
{

TaxDetailsLevel createDetailsLevel(const boost::optional<InputTaxDetailsLevel>& input)
{
  TaxDetailsLevel ans;

  if (input)
  {
    if (input->allDetails())
      return TaxDetailsLevel::all();

    ans.calc = input->calcDetails();
    ans.geo = input->geoDetails();
    ans.taxOnTax = input->taxOnTaxDetails();
    ans.taxOnFare = input->taxOnFaresDetails();
    ans.taxOnYqYr = input->taxOnYQYRDetails();
    ans.taxOnOc = input->taxOnOptionalServiceDetails();
    ans.taxOnExchangeReissue = input->taxOnExchangeReissueDetails();
  }

  return ans;
}

type::ProcessingGroup asProcessingGroup(unsigned i)
{
  switch (i)
  {
    case  0: return type::ProcessingGroup::OC;
    case  1: return type::ProcessingGroup::OB;
    case  2: return type::ProcessingGroup::ChangeFee;
    case  3: return type::ProcessingGroup::Itinerary;
    case  4: return type::ProcessingGroup::Baggage;
    default: throw std::runtime_error("Bad processing group");
  }
}

std::vector<type::ProcessingGroup>
createGroupsFromInput(const std::vector<InputApplyOn>& applicableGroups)
{
  std::vector<type::ProcessingGroup> ans;

  if (applicableGroups.empty())
  {
    for (unsigned i = 0; i < static_cast<unsigned>(type::ProcessingGroup::GroupsCount); ++i)
      ans.push_back(asProcessingGroup(i));
  }

  for (InputApplyOn const& applyOn : applicableGroups)
    ans.push_back(asProcessingGroup(applyOn._group));

  return ans;
}

} // anonymous namespace

ProcessingOptions
ProcessingOptionsFactory::createFromInput(const InputProcessingOptions& inputProcessingOptions)
{
  ProcessingOptions result;

  create<ExemptedRuleFactory>(inputProcessingOptions.exemptedRules(), result.exemptedRules());
  create<CalculationRestrictionFactory>(inputProcessingOptions.calculationRestrictions(),
                                        result.calculationRestrictions());

  result.taxDetailsLevel() = createDetailsLevel(inputProcessingOptions.taxDetailsLevel());
  result.applyUSCAGrouping() = inputProcessingOptions._applyUSCAgrouping;
  result.tch() = inputProcessingOptions._tch;
  result.setRtw(inputProcessingOptions._rtw);
  result.setProcessingGroups(createGroupsFromInput(inputProcessingOptions.getApplicableGroups()));
  result.setUseRepricing(inputProcessingOptions._useRepricing);

  return result;
}

} // namespace tax
