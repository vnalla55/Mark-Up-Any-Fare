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

#include "TaxDisplay/ViewX1TaxTable.h"

#include "Common/RulesRecordUtil.h"
#include "Common/SafeEnumToString.h"
#include "Common/TaxName.h"
#include "TaxDisplay/Common/CommonEntries.h"
#include "Util/BranchPrediction.h"

#include <boost/format.hpp>

namespace tax
{
namespace display
{

bool ViewX1TaxTable::header()
{
  if (_singleNationData)
  {
    _formatter.addLine("COUNTRY NAME- " + _singleNationData->name);
    _formatter.addLine("COUNTRY CODE- " + _singleNationData->code.asString());
  }
  else
  {
    _formatter.addLine("COUNTRY NAME- ");
    _formatter.addLine("COUNTRY CODE- ");
  }

  return true;
}

bool ViewX1TaxTable::body()
{
  _formatter.addBlankLine();
  _formatter.addLine("COUNTRY    TAX  TAX  PERCENT    TAX        SOURCE", LineParams::withLeftMargin(5));
  _formatter.addLine("CODE       CODE TYPE FLAT       POINT", LineParams::withLeftMargin(5));
  _formatter.addBlankLine();

  boost::format rowFormat("%-4u %-10s %-4s %-4s %-10s %-10s %s");
  unsigned int lineNo = 1;
  for (const DataRow& dataRow : _dataSet)
  {
    const TaxName& taxName = std::get<const TaxName&>(dataRow);
    rowFormat % lineNo++
              % taxName.nation().asString()
              % taxName.taxCode().asString()
              % taxName.taxType().asString()
              % safeEnumToString(taxName.percentFlatTag())
              % safeEnumToString(taxName.taxPointTag())
              % RulesRecordUtil::getVendorFullStr(std::get<const type::Vendor&>(dataRow));

    _formatter.addLine(rowFormat.str());

    const type::TaxLabel* label = std::get<const type::TaxLabel*>(dataRow);
    LineParams labelParams;
    labelParams.setLeftMargin(18);
    labelParams.setParagraphIndent(-11);
    _formatter.addLine("TAX NAME: " + (label ? *label : ""), labelParams);
  }

  return true;
}

bool ViewX1TaxTable::footer()
{
  _formatter.addBlankLine();
  if (_singleNationData)
  {
    _formatter.addLine(CommonEntries::roundingInfo(_taxRoundingInfoService, _singleNationData->code));
    if (!_singleNationData->message.empty())
      _formatter.addLine(_singleNationData->message);
  }
  else
  {
    _formatter.addLine("COUNTRY TAX ROUNDING -");
  }
  _formatter.addLine("USE TX1*# WHERE # IS LINE NUMBER TO VIEW SPECIFIC DETAILS");
  _formatter.addLine("TAX TYPES REPRESENT DIFFERENT APPLICATIONS FOR THE SAME TAX CODE");
  _formatter.addLine("TAX TYPES 001-099: AIR TRAVEL TAXES");
  _formatter.addLine("TAX TYPES 100 AND ABOVE: ANCILLARY TAXES");
  _formatter.addLine("USE TXHELP FOR HELP ENTRY FORMATS");

  return true;
}

} // namespace display
} // namespace tax
