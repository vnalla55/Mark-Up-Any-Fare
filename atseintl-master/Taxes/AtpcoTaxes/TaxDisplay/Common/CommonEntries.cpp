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

#include "TaxDisplay/Common/CommonEntries.h"

#include "Common/MoneyUtil.h"
#include "Common/RulesRecordUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Services/ReportingRecord.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/TaxCodeTextService.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"
#include "TaxDisplay/Response/FixedWidth.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/LineParams.h"
#include "TaxDisplay/Response/ResponseFormatter.h"

#include <boost/algorithm/string.hpp>

#include <vector>

namespace tax
{
namespace display
{
namespace
{

void fillWithTaxCodeText(const ReportingRecord& reportingRecord,
                         const TaxCodeTextService& taxCodeTextService,
                         type::Index itemNo,
                         ResponseFormatter& formatter)
{
  Line dateLine;
  type::Date effDate = reportingRecord.effDate.date();
  type::Date discDate = reportingRecord.discDate.date();

  dateLine << "EFF DATE-";
  if (!effDate.is_pos_infinity())
  {
    dateLine << boost::to_upper_copy(effDate.format("%d%b%Y"));
  }
  else
  {
    dateLine << fixedWidth(' ', 9);
  }

  dateLine << " - DISC DATE-";
  if (!discDate.is_pos_infinity())
  {
    dateLine << boost::to_upper_copy(discDate.format("%d%b%Y"));
  }
  else
  {
    dateLine << fixedWidth(' ', 9);
  }

  formatter.addLine(dateLine);

  std::vector<std::string> data;
  taxCodeTextService.getTaxCodeText(itemNo, reportingRecord.vendor, data);
  if (!data.empty())
  {
    for (std::string dataStr : data)
    {
      formatter.addLine(dataStr);
    }
  }
  else
  {
    formatter.addLine("**NO DATA INFORMATION PROVIDED", LineParams::withLeftMargin(3));
  }
}

} // anonymous namespace


void CommonEntries::taxDetails(const ReportingRecord& record,
                               const LocService& locService,
                               const TaxCodeTextService& taxCodeTextService,
                               const TaxDisplayRequest& request,
                               ResponseFormatter& formatter)
{
  Line lineNation;
  lineNation << fixedWidth(record.nation.asString(), 16)
             << locService.getNationName(record.nation);

  Line lineTax;

  if (request.isUserTN())
  {
    lineTax << fixedWidth(record.taxCode.asString(),    22);
  }
  else
  {
    lineTax << fixedWidth(record.taxCode.asString(),    11)
            << fixedWidth(record.taxType.asString(),    11);
  }

  lineTax << fixedWidth((record.taxCarrier == "YY" ? "ALL CXRS" : record.taxCarrier.asString()), 11)
          << fixedWidth(RulesRecordUtil::getVendorFullStr(record.vendor), 11);

  formatter.addLine("COUNTRY CODE    COUNTRY NAME");
  formatter.addLine(lineNation);
  formatter.addSeparatorLine('.');

  if (request.isUserTN())
    formatter.addLine("TAX CODE              CARRIER    SOURCE");
  else
    formatter.addLine("TAX CODE   TAX TYPE   CARRIER    SOURCE");

  formatter.addLine(lineTax);
  formatter.addSeparatorLine('.');

  std::string taxLabel = "TAX NAME- ";
  if (!record.entries.empty())
    taxLabel += record.entries.front().taxLabel;

  formatter.addLine(taxLabel);
  formatter.addSeparatorLine('.');

  std::string strVat = record.isVatTax? "YES" : "NO";
  std::string strCommissionable = record.isCommissionable? "YES" : "NO";
  std::string strInterlineable = record.isInterlineable? "YES" : "NO";

  Line line(LineParams::withTruncateFlag());
  line << "VAT-" << fixedWidth(strVat, 6);
  line << " TAX OR CHARGE-";
  line << fixedWidth(safeEnumToString(record.taxOrChargeTag), 7);
  line << " COMMISSIONABLE-" << strCommissionable;
  formatter.addLine(line);
  line.clear();

  line << "INTERLINEABLE-" << fixedWidth(strInterlineable, 5);
  line << "REFUNDABLE-" << safeEnumToString(record.refundableTag);
  formatter.addLine(line);
  line.clear();

  line << "ACCOUNTABLE DOCUMENT TAG-";
  line << safeEnumToString(record.accountableDocTag);
  formatter.addLine(line);
  formatter.addSeparatorLine('.');

  if (request.isUserSabre())
  {
    Line line;
    line << "RECORD EFFECTIVE DATE- ";
    line << boost::to_upper_copy(record.effDate.date().format("%d%b%Y"));
    line << "  DISCONTINUE EFFECTIVE DATE- ";
    line << boost::to_upper_copy(record.discDate.date().format("%d%b%Y"));
    formatter.addLine(line);
    formatter.addSeparatorLine('.');
  }


  formatter.addLine("TAX CATEGORIES-REFER TO TXHELP FOR SELECTIVE DISPLAY");
  formatter.addLine("1-DEFINITION                          2-APPLICABLE TO");
  formatter.addLine("3-TAX RATE                            4-EXEMPTIONS");
  formatter.addLine("5-COLLECTION/REMITTANCE               6-TAX AUTHORITY");
  formatter.addLine("7-COMMENTS                            8-SPECIAL INSTRUCTIONS");
  formatter.addLine("9-REPORTING");
  formatter.addSeparatorLine('.');

  taxDetailsCategories(record, taxCodeTextService, request, formatter);
}

void CommonEntries::taxDetailsCategories(const ReportingRecord& record,
                                         const TaxCodeTextService& taxCodeTextService,
                                         const TaxDisplayRequest& request,
                                         ResponseFormatter& formatter)
{
  formatter.addLine("*1 DEFINITION-");
  fillWithTaxCodeText(
      record, taxCodeTextService, record.taxTextItemNo, formatter);
  formatter.addSeparatorLine('.');

  if (request.x2categories[static_cast<size_t>(X2Category::APPLICABLE_TO)])
  {
    formatter.addLine("*2 APPLICABLE TO-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxApplicableToItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::TAX_RATE)])
  {
    formatter.addLine("*3 TAX RATE-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxRateItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::EXEMPTIONS)])
  {
    formatter.addLine("*4 EXEMPTIONS-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxExemptionsItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::COLLECTION_REMITTANCE)])
  {
    formatter.addLine("*5 COLLECTION/REMITTANCE-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxCollectRemitItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::TAX_AUTHORITY)])
  {
    formatter.addLine("*6 TAX AUTHORITY-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxingAuthorityItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::COMMENTS)])
  {
    formatter.addLine("*7 COMMENTS-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxCommentsItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::SPECIAL_INSTRUCTIONS)])
  {
    formatter.addLine("*8 SPECIAL INSTRUCTIONS-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.taxSpecialInstructionsItemNo, formatter);
    formatter.addSeparatorLine('.');
  }

  if (request.x2categories[static_cast<size_t>(X2Category::REPORTING)])
  {
    formatter.addLine("*9 REPORTING-");
    fillWithTaxCodeText(
        record, taxCodeTextService, record.reportingTextItemNo, formatter);
    formatter.addSeparatorLine('.');
  }
}

Line CommonEntries::roundingInfo(const TaxRoundingInfoService& roundingInfoService,
                                 type::Nation nationCode)
{
  type::MoneyAmount unit = -1;
  type::TaxRoundingDir dir(type::TaxRoundingDir::Blank);
  roundingInfoService.getNationRoundingInfo(nationCode, unit, dir);

  std::ostringstream roundingInfo;
  switch(dir)
  {
  case type::TaxRoundingDir::RoundUp:
    roundingInfo << "COUNTRY TAX ROUNDING - ROUND UP TO NEXT " << amountToDouble(unit);
    break;
  case type::TaxRoundingDir::RoundDown:
    roundingInfo << "COUNTRY TAX ROUNDING - ROUND DOWN TO NEXT " << amountToDouble(unit);
    break;
  case type::TaxRoundingDir::Nearest:
    roundingInfo << "COUNTRY TAX ROUNDING - ROUND TO NEAREST " << amountToDouble(unit);
    break;
  case type::TaxRoundingDir::NoRounding:
    roundingInfo << "COUNTRY TAX ROUNDING - NO ROUNDING APPLIES";
    break;
  default:
    roundingInfo << "NO ROUNDING INFO";
    break;
  }

  return Line(roundingInfo.str());
}

std::string CommonEntries::getErrorNoData(const TaxDisplayRequest& request)
{
  if (!request.taxType.empty() && !request.hasSetAnyCarrierCode())
    return "NO TAX CODE/TAX TYPE DATA EXISTS";

  if (!request.taxType.empty() && request.hasSetAnyCarrierCode())
    return "NO TAX CODE/TAX TYPE/CXR CARRIER CODE DATA EXISTS";

  if (request.taxType.empty() && request.hasSetAnyCarrierCode())
    return "NO TAX CODE/CXR CARRIER CODE DATA EXISTS";

  return "NO TAX CODE DATA EXISTS";
}

} /* namespace display */
} /* namespace tax */
