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

#include "Diagnostic/DBDiagnostic.h"

#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/RulesRecord.h"
#include "Processor/GetRuleDescriptionFunctor.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/BusinessRule.h"
#include "Rules/TaxData.h"

#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>

namespace tax
{
const type::TaxPointTag DBDiagnostic::EMPTY_TAG = {};
const uint32_t DBDiagnostic::NUMBER = 831;

DBDiagnostic::DBDiagnostic(Services& services,
                           const boost::ptr_vector<Parameter>& parameters,
                           const type::Timestamp& ticketingDate)
  :
    _parameters(parameters),
    _ticketingDate(ticketingDate),
    _services(services),
    _filter(),
    _availableTags(),
    _formatter(),
    _columnNames(),
    _brIdNameformatter(),
    _brIdNameColumnNames()
{
  _formatter = boost::format("%|=6| %|=4| %|=4| %|=4| %|=3|");
  _columnNames = str(_formatter % "NATION" % "CODE" % "TYPE" % "TXPT" % "SEQ");

  _brIdNameformatter = boost::format("%|-10| %|-40|");
  _brIdNameColumnNames = str(_brIdNameformatter % "RULE ID" % "RULE NAME");

  // TODO use external values
  _availableTags.push_back(type::TaxPointTag::Sale);
  _availableTags.push_back(type::TaxPointTag::Arrival);
  _availableTags.push_back(type::TaxPointTag::Departure);
  _availableTags.push_back(type::TaxPointTag::Delivery);
}

DBDiagnostic::~DBDiagnostic(void)
{
}

void
DBDiagnostic::runAll()
{
  _helpPrinter.print(*this);
  _brPrinter.print(*this);
  _recordsPrinter.print(*this);

  printLine('-');
}

void
DBDiagnostic::applyParameters()
{
  bool isBrEnabled = false;

  for (Parameter const& parameter : _parameters)
  {
    if (parameter.name() == "HE" || parameter.name() == "HELP")
    {
      _helpPrinter.enable();
    }
    else if (parameter.name() == "BR")
    {
      isBrEnabled = true;
    }
  }

  bool isRecordsEnabled = _filter.applyParameters(_parameters);

  if (isBrEnabled)
  {
    _brPrinter.enable();
  }
  else if (isRecordsEnabled)
  {
    _recordsPrinter.enable();
  }
  else
  {
    _helpPrinter.enable();
  }
}

void
DBDiagnostic::printHeader()
{
  printLine('*');
  printHeaderShort("DIAGNOSTIC " + boost::lexical_cast<std::string>(NUMBER) + " - DB DATA");
  printLine('*');
}

void
DBDiagnostic::printHelp()
{
  const std::string indent(4, ' ');
  const std::string number = boost::lexical_cast<std::string>(NUMBER);
  printHeaderLong("HELP");

  _result
    << "HELP - HELP INFO\n"
    << "BR - ALL BUSINESS RULES\n"
    << "---\n"
    << "OUTPUT FILTERS - DISPLAY ONLY SEQUENCES MATCHING FILTERS:\n"
    << "NAXX - TAXES FOR NATION XX\n"
    << "PTX - TAXES IN TAXPOINT X\n"
    << "TCXX - TAXES FOR TAXCODE XX\n"
    << "TTXX - TAXES FOR TAXTYPE XX\n"
    << "SQXXX - TAXES FOR SEQ XXX\n"
    << "DE - ADD TO DISPLAY RECORDS DESCRIPTIONS\n"
    << indent << "/EG. " << number << "/NAPL/PTD/TCXW/TT001/SQ5343/DE\n";

  // TB - through DaoDiagnostics in AtpcoTaxorchestrator
  _result
    << "---\n"
    << "TBTTT/NANN - TABLE TTT RAW TAX DATA FOR NATION NN" << "\n"
    << "TBTTT/NANN/PTX - TABLE TTT RAW TAX DATA FOR NATION NN\n"
    << indent << "IN TAXPOINT X" << "\n"

    << "TBTTT/TXNNTCTT - TABLE TTT RAW TAX DATA FOR NATION NN\n"
    << indent << "TAXCODE TC TAXTYPE TT /EG. " << number << "/TXUSUS005/" << "\n"

    << "TBTTT/TXNNTCTT/SQNNN - TABLE TTT RAW TAX DATA FOR NATION NN\n"
    << indent << "TAXCODE TC TAXTYPE TT SEQ NNN\n"
    << indent << "/EG. " << number << "/TXUSUS005SQ100/" << "\n";
}

void
DBDiagnostic::printAllBusinessRules()
{
  printHeaderLong("BUSINESS RULES LIST");
  _result << _brIdNameColumnNames << "\n\n";

  RuleIdNames ruleIdNames;
  BusinessRulesContainer::createRuleNames(ruleIdNames);

  for (const RuleIdName& rule : ruleIdNames)
  {
    _result << str(_brIdNameformatter % rule.first % rule.second) << "\n";
  }
}

void
DBDiagnostic::printRecords()
{
  if(_filter.getNation().empty())
  {
    _result << " * ERROR: PLEASE SPECIFY NATION\n";
    return;
  }

  if((_filter.getTaxPointTag() != EMPTY_TAG) &&
     std::find(
       _availableTags.begin(), _availableTags.end(), _filter.getTaxPointTag()) == _availableTags.end())
  {
    _result << " * ERROR: WRONG TAXPOINTTAG\n";
    return;
  }

  printHeaderLong("TAX INFO");
  _result << _columnNames << "\n";

  if (_filter.getTaxPointTag() != EMPTY_TAG)
  {
    printFiltered(_filter.getTaxPointTag());
  }
  else
  {
    for (const tax::type::TaxPointTag& taxPointTag : _availableTags)
    {
      printFiltered(taxPointTag);
    }
  }
}

void
DBDiagnostic::printRecordsContainers(
    const std::vector<std::shared_ptr<BusinessRulesContainer>>& rulesContainers)
{
  for (const std::shared_ptr<BusinessRulesContainer>& rulesRecord : rulesContainers)
  {
    if (_filter.seqNoMatched(rulesRecord->seqNo()))
    {
      printRecord(*rulesRecord, _filter.isDescriptionEnabled());
    }
  }
}

void
DBDiagnostic::printFiltered(const tax::type::TaxPointTag& tag)
{
  RulesRecordsService::ByNationConstValue rulesContainersGroupedByTaxName =
      _services.rulesRecordsService().getTaxRulesContainers(
          _filter.getNation(), tag, _ticketingDate);

  static std::vector<type::ProcessingGroup> allGroups{type::ProcessingGroup::OC,
                                                      type::ProcessingGroup::OB,
                                                      type::ProcessingGroup::ChangeFee,
                                                      type::ProcessingGroup::Itinerary};
  for (TaxData const& taxData : *rulesContainersGroupedByTaxName)
  {
    if (_filter.taxCodeMatched(taxData.getTaxName().taxCode()) &&
        _filter.taxTypeMatched(taxData.getTaxName().taxType()))
    {
      for (const tax::type::ProcessingGroup processingGroup : allGroups)
        printRecordsContainers(taxData.getDateFilteredCopy(processingGroup, _ticketingDate));
    }
  }
}

void
DBDiagnostic::printRecord(const BusinessRulesContainer& rulesRecord, bool showDescription)
{
  printRecordData(rulesRecord);
  if (showDescription)
  {
    _result << "\n";
    printRecordDescription(rulesRecord);
    printLine('-');
  }
}

void
DBDiagnostic::printRecordData(const BusinessRulesContainer& rulesRecord)
{
  _result << _formatter % rulesRecord.taxName().nation() % rulesRecord.taxName().taxCode() %
    rulesRecord.taxName().taxType() %  rulesRecord.taxName().taxPointTag() %
    rulesRecord.seqNo() << "\n";
}

void
DBDiagnostic::printRecordDescription(const BusinessRulesContainer& rulesRecord)
{
  std::vector<std::string> descriptions;
  rulesRecord.getValidatorsGroups().foreach<GetRuleDescriptionFunctor>(
      true, true, _services, descriptions);
  rulesRecord.getCalculatorsGroups().foreach<GetRuleDescriptionFunctor>(
      _services, descriptions);

  for (const std::string& description : descriptions)
    _result << description << "\n";

  _result << "\n";
}

DBDiagnostic::Filter::Filter()
  : _nation(),
    _taxPointTag(EMPTY_TAG),
    _taxCode(),
    _taxType(),
    _seqNo(),
    _descriptionEnabled(false)
{}

bool
DBDiagnostic::Filter::applyParameters(const boost::ptr_vector<Parameter>& parameters)
{
  bool isRecordsEnabled = false;

  for (const Parameter& parameter: parameters)
  {
    if (parameter.name() == "NA")
    {
      isRecordsEnabled = true;
      type::Nation nation(UninitializedCode);
      codeFromString(parameter.value(), nation);
      _nation = nation;
    }
    else if (parameter.name() == "DE")
    {
      isRecordsEnabled = true;
      _descriptionEnabled = true;
    }
    else if (parameter.name() == "PT")
    {
      isRecordsEnabled = true;
      const char tag = parameter.value().at(0);
      _taxPointTag = static_cast<type::TaxPointTag>(tag);
    }
    else if (parameter.name() == "TC")
    {
      isRecordsEnabled = true;
      type::TaxCode taxCode(UninitializedCode);
      codeFromString(parameter.value(), taxCode);
      _taxCode = taxCode;
    }
    else if (parameter.name() == "TT")
    {
      isRecordsEnabled = true;
      type::TaxType taxType(UninitializedCode);
      codeFromString(parameter.value(), taxType);
      _taxType = taxType;
    }
    else if (parameter.name() == "SQ")
    {
      isRecordsEnabled = true;
      try
      {
        _seqNo = boost::lexical_cast<type::SeqNo>(parameter.value());
      }
      catch (boost::bad_lexical_cast&)
      {}
    }
  }

  return isRecordsEnabled;
}

const type::Nation&
DBDiagnostic::Filter::getNation() const
{
  return _nation;
}

const type::TaxPointTag&
DBDiagnostic::Filter::getTaxPointTag() const
{
  return _taxPointTag;
}

bool
DBDiagnostic::Filter::isDescriptionEnabled() const
{
  return _descriptionEnabled;
}

bool
DBDiagnostic::Filter::nationMatched(const type::Nation& nation_) const
{
  return (_nation.empty() || (_nation == nation_));
}

bool
DBDiagnostic::Filter::taxCodeMatched(const type::TaxCode& taxCode_) const
{
  return (_taxCode.empty() || (_taxCode == taxCode_));
}

bool
DBDiagnostic::Filter::taxTypeMatched(const type::TaxType& taxType_) const
{
  return (_taxType.empty() || (_taxType == taxType_));
}

bool
DBDiagnostic::Filter::seqNoMatched(const type::SeqNo& seqNo_) const
{
  return _seqNo == 0U || _seqNo == seqNo_;
}

}
