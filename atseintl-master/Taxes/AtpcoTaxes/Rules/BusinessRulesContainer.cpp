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
#include <boost/algorithm/string.hpp>
#include <utility>

#include "Common/Consts.h"
#include "DataModel/Services/RulesRecord.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/RequestLogicError.h"
#include "Rules/RulesBuilder.h"

namespace tax
{

BusinessRulesContainer::BusinessRulesContainer(RulesRecord const& rulesRecord,
                                               type::ProcessingGroup processignGroup)
  : _paymentRuleData(rulesRecord, processignGroup),
    _serviceBaggageItemNo(rulesRecord.serviceBaggageItemNo),
    _serviceBaggageApplTag(rulesRecord.serviceBaggageApplTag),
    _valid(true)
{
  _taxName = rulesRecord.taxName;
  _vendor = rulesRecord.vendor;

  if (!taxableUnits())
  {
    _valid = false;
    return;
  }

  splitRecord(rulesRecord);
}

BusinessRulesContainer::~BusinessRulesContainer()
{
}

void
BusinessRulesContainer::splitRecord(const RulesRecord& rulesRecord)
{
  RulesBuilder::buildRules(
      rulesRecord, taxableUnits(), _validatorsGroups, _limitGroup, _calculatorsGroups);
}

void
BusinessRulesContainer::createRuleNames(RuleIdNames& ruleNames)
{
  type::Index ruleId = 0;

  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXABLEUNITTAGRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "SPECIALTAXPROCESSINGRULE"));

  // Ticket Group (6)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "CURRENCYOFSALERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "THIRDPARTYTAGRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "POINTOFTICKETINGRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "SALEDATERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "SERVICEFEESECURITYRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "POINTOFSALERULE"));

  // Filler Group (3)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "FILLTIMESTOPOVERSRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TICKETEDPOINTRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "CONNECTIONSTAGSRULE"));

  // GeoPath Group (7)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "JOURNEYLOC1ASORIGINRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "APPLICATIONTAGRULE"));
  ruleNames.push_back(
      std::pair<type::Index, std::string>(++ruleId, "JOURNEYLOC2DESTINATIONTURNAROUNDRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TRAVELWHOLLYWITHINRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "JOURNEYINCLUDESRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "RETURNTOORIGINRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "CONTINUOUSJOURNEYRULE"));

  // OptionalService Group (2)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "OPTIONALSERVICETAGSRULE"));
  ruleNames.push_back(
      std::pair<type::Index, std::string>(++ruleId, "OPTIONALSERVICEPOINTOFDELIVERYRULE"));

  // TaxPointBegin Group (4)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC1RULE"));
  ruleNames.push_back(
      std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC1TRANSFERTYPERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC1STOPOVERTAGRULE"));
  ruleNames.push_back(
      std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC1INTERNATIONALDOMESTICRULE"));

  // TaxPointEnd Group (4)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC2RULE"));
  ruleNames.push_back(
      std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC2INTERNATIONALDOMESTICRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXLOC2COMPARERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXPOINTLOC3GEORULE"));

  // Itin Group (5)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TRAVELDATESRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "VALIDATINGCARRIERRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "CARRIERFLIGHTRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "PASSENGERTYPECODERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "SECTORDETAILRULE"));

  // Apply Group (14)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "SERVICEBAGGAGERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TICKETMINMAXVALUERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXONTAXRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "REPORTINGRECORDRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXONYQYRRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXONOPTIONALSERVICERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "FLATTAXRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "ALTERNATEREFAKHIFACTORSRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXONFARERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "PERCENTAGETAXRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXMINMAXVALUERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXAPPLICATIONLIMITRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXMATCHINGAPPLTAGRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXONCHANGEFEERULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXROUNDINGRULE"));

  // Final Group (2)
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "EXEMPTTAGRULE"));
  ruleNames.push_back(std::pair<type::Index, std::string>(++ruleId, "TAXCODECONVERSIONRULE"));
}

} // namespace tax
