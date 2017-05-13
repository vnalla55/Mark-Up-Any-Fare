// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/ViewX1TaxDetail.h"

#include "Common/MoneyUtil.h"
#include "Common/RulesRecordUtil.h"
#include "Common/TaxName.h"
#include "DataModel/Services/RulesRecord.h"
#include "TaxDisplay/ViewX1SequenceDetailBuilder.h"

#include <boost/format.hpp>

#include <string>

namespace tax
{
namespace display
{

namespace
{

inline void addCommonColumns(const RulesRecord& rulesRecord,
                             boost::format& f)
{
  f % rulesRecord.taxName.taxCarrier().asString()
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::YqYr)            ? "1" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::TicketingFee)    ? "2" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCFlightRelated) ? "3" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCTicketRelated) ? "4" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCMerchandise)   ? "5" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCFareRelated)   ? "6" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::BaggageCharge)   ? "7" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::TaxOnTax)        ? "8" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::Itinerary)       ? "9" : " ")
    % (rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::ChangeFee)       ? "10" : "  ")
    % rulesRecord.seqNo;
}

} // anonymous namespace


bool ViewX1TaxDetail::header()
{
  // Since this is sequence list, we can assume that all records have
  // the same country and tax. Also, there should be at least one record
  const TaxName& taxName = (*_rulesRecords.begin())->taxName;

  _formatter.addLine("COUNTRY NAME- " + _nationName);
  _formatter.addLine("COUNTRY CODE- " + taxName.nation().asString());
  _formatter.addLine("TAX CODE- " + taxName.taxCode().asString());
  _formatter.addLine("TAX TYPE- " + taxName.taxType().asString());
  _formatter.addLine("TAX NAME- " + (_taxLabel ? *_taxLabel : ""));

  return true;
}

bool ViewX1TaxDetail::body()
{
  _formatter.addBlankLine();
  _formatter.addLine("TAX              TAX  TAXABLE-UNIT-TAG#-SEE BELOW SEQUENCE", LineParams::withLeftMargin(6));
  _formatter.addLine("PCT/FLAT/CUR     CXR   1 2 3 4 5 6 7 8 9 10       NUMBER", LineParams::withLeftMargin(6));
  _formatter.addBlankLine();

  boost::format percentFormat("%-5u %-16s %-4s %2s%2s%2s%2s%2s%2s%2s%2s%2s%3s      %9u");
  boost::format flatFormat("%-5u %-13s%3s %-4s %2s%2s%2s%2s%2s%2s%2s%2s%2s%3s      %9u");
  boost::format exemptFormat("%-5u EXEMPTION        %-4s %2s%2s%2s%2s%2s%2s%2s%2s%2s%3s      %9u");

  unsigned int lineNo = 1;
  for (const std::shared_ptr<const RulesRecord>& rulesRecord : _rulesRecords)
  {
    boost::format* f;
    if (rulesRecord->exemptTag == type::ExemptTag::Exempt)
    {
      exemptFormat % lineNo++;
      f = &exemptFormat;
    }
    else if (rulesRecord->taxName.percentFlatTag() == type::PercentFlatTag::Percent)
    {
      double percent = amountToDouble(RulesRecordUtil::getTaxAmt(*rulesRecord)) * 100;
      std::string percentStr = (boost::format("%g") % percent).str();
      percentFormat % lineNo++ % percentStr.append("-PCT");
      f = &percentFormat;
    }
    else
    {
      boost::format amountFormat("%-13." + std::to_string(rulesRecord->taxCurDecimals) + "f");
      flatFormat % lineNo++
                 % (amountFormat % amountToDouble(RulesRecordUtil::getTaxAmt(*rulesRecord))).str()
                 % rulesRecord->taxCurrency.asString();

      f = &flatFormat;
    }

    addCommonColumns(*rulesRecord, *f);
    _formatter.addLine(f->str());

    if (_sequenceDetailBuilder) // combined view
      _sequenceDetailBuilder->build(*rulesRecord, _formatter)->body();
  }

  return true;
}

bool ViewX1TaxDetail::footer()
{
  _formatter.addBlankLine();
  _formatter.addLine("USE TX1*# WHERE # IS LINE NUMBER TO VIEW SPECIFIC DETAILS");
  _formatter.addLine("TAX TYPES REPRESENT DIFFERENT APPLICATIONS FOR THE SAME TAX CODE");
  _formatter.addLine("TAX TYPES 001-099: AIR TRAVEL TAXES");
  _formatter.addLine("TAX TYPES 100 AND ABOVE: ANCILLARY TAXES");
  _formatter.addLine("TAXABLE UNIT TAG 1: CARRIER IMPOSED YQ/YR FEES");
  _formatter.addLine("TAXABLE UNIT TAG 2: TICKETING FEES");
  _formatter.addLine("TAXABLE UNIT TAG 3: OPTIONAL SERVICES-FLIGHT RELATED");
  _formatter.addLine("TAXABLE UNIT TAG 4: OPTIONAL SERVICES-TICKET RELATED");
  _formatter.addLine("TAXABLE UNIT TAG 5: OPTIONAL SERVICES-MERCHANDISE");
  _formatter.addLine("TAXABLE UNIT TAG 6: OPTIONAL SERVICES-FARE RELATED");
  _formatter.addLine("TAXABLE UNIT TAG 7: BAGGAGE CHARGES");
  _formatter.addLine("TAXABLE UNIT TAG 8: TAX-ON-TAX");
  _formatter.addLine("TAXABLE UNIT TAG 9: ITINERARY");
  _formatter.addLine("TAXABLE UNIT TAG 10: CHANGE FEE");
  _formatter.addLine("CARRIER YY - ALL OTHER CARRIERS NOT SPECIFIED");
  _formatter.addLine("USE TXHELP FOR HELP ENTRY FORMATS");

  return true;
}

} // namespace display
} // namespace tax
