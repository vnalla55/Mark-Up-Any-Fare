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

#include "TaxDisplay/ViewX2TaxReissue.h"

#include "Common/SafeEnumToString.h"
#include "Rules/MathUtils.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/LineParams.h"

#include <boost/format.hpp>

#include <string>
#include <utility>
#include <vector>

namespace tax
{
namespace display
{
namespace
{

inline std::string getSeparatedCarriers(const std::vector<type::CarrierCode>& carrierCodes)
{
  std::string ret;
  for (const type::CarrierCode& carrierCode : carrierCodes)
  {
    if (ret.size() > 0)
      ret += ", ";

    ret += carrierCode.asString();
  }

  return ret;
}

} // anonymous namespace

bool ViewX2TaxReissue::body()
{
  using boost::format;

  for (const std::pair<type::Nation, ReissueViewData>& mapEntry : _data)
  {
    const std::vector<ReissueTaxData>& reissueTaxDataVec = mapEntry.second.reissueTaxDataVec;
    if (reissueTaxDataVec.empty())
      continue;

    const type::Nation& nationCode = mapEntry.first;
    const type::NationName& nationName = mapEntry.second.nationName;

    _formatter.addLine("COUNTRY CODE     COUNTRY NAME")
              .addLine(format("%2s               %s") % nationCode.asString() % nationName)
              .addSeparatorLine('.');


    for (const ReissueTaxData& reissueTaxData : reissueTaxDataVec)
    {
      const TaxReissue& reissue = *reissueTaxData.taxReissue;

      _formatter.addLine("DATABASE SEQ: " + std::to_string(reissue.seqNo))
                .addLine("01 TAX CODE/TYPE/TAX NAME-");
                              // * TC  /   TYP / TAX NAME
      _formatter.addLine(format("* %2s  /   %3s / %s") % reissue.taxCode.asString()
                                                       % reissue.taxType.asString()
                                                       % reissueTaxData.taxLabel);


      _formatter.addBlankLine()
                .addLine("02 TAX DETAIL-REFUND APPLICATION-")
                .addLine("* " + safeEnumToString(reissue.refundableTag))
                .addBlankLine()
                .addLine("03 SALE-REISSUE-");

      if (reissue.locType != type::ReissueLocTypeTag::Blank)
      {
        std::string statement = reissue.locExceptTag == type::LocExceptTag::Yes ?
                                  "* DOES NOT APPLY FOR REISSUES IN " :
                                  "* APPLIES FOR REISSUES IN ";

        Line line;
        line << std::move(statement)
             << (reissue.locType != type::ReissueLocTypeTag::Zone ? safeEnumToString(reissue.locType) : "")
             << " "
             << reissue.locCode;

        _formatter.addLine(std::move(line));
      }
      else
      {
        _formatter.addLine("NO SALE-REISSUE RESTRICTION.", LineParams::withLeftMargin(5));
      }


      _formatter.addBlankLine()
                .addLine("05 VALIDATING CARRIER-TICKETING REISSUE CARRIER-");
      if (!reissue.ticketingCarriers.empty())
      {
        Line line;
        line << "* "
             << (reissue.ticketingExceptTag == type::TicketingExceptTag::Yes ? "EXCEPT " : "")
             << "TICKETING REISSUE CARRIER/S "
             << getSeparatedCarriers(reissue.ticketingCarriers)
             << ".";

        _formatter.addLine(std::move(line));
      }
      else
      {
        _formatter.addLine("NO VALIDATING CARRIER-TICKETING REISSUE CARRIER RESTRICTION.", LineParams::withLeftMargin(5));
      }


      _formatter.addBlankLine()
                .addLine("06 CURRENCY-");

      if (!reissue.currency.empty())
      {
        format amountFormat("%." + std::to_string(reissue.currencyDec) + "f");
        Line line;
        line << "* TAX EXEMPT IF REISSUE IS LESS THAN "
             << reissue.currency.asString()
             << " "
             << (amountFormat % MathUtils::adjustDecimal(reissue.amount, reissue.currencyDec)).str()
             << ".";

        _formatter.addLine(std::move(line));
      }
      else
      {
        _formatter.addLine("NO CURRENCY RESTRICTION.", LineParams::withLeftMargin(5));
      }

      _formatter.addLine(".")
                .addLine(".");
    }
  }

  return true;
}

} // namespace display
} // namespace tax
