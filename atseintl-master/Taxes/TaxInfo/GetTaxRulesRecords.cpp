//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include "Common/FallbackUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Nation.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxReportingRecordInfo.h"
#include "DBAccess/TaxRulesRecord.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "TaxInfo/GetTaxRulesRecords.h"

#include <algorithm>
#include <tuple>

#include <boost/optional.hpp>

namespace tse
{

FIXEDFALLBACK_DECL(ATPCO_TAX_X1byCodeDAORefactor);

namespace
{

boost::optional<std::string> makeTaxType(TaxCode taxCode)
{
  assert (taxCode.size() >= 2);

  if (taxCode.size() == 2)
    return boost::optional<std::string>("001");

  return std::string("00") + taxCode[2]; // may render "00A"
}

LocType mapLocType(LocTypeCode c)
{
  switch (c)
  {
    case 'A':
      return IATA_AREA;
    case 'C':
    case 'P':
      return MARKET;
    case 'N':
      return NATION;
    case 'S':
      return STATE_PROVINCE;
    case 'Z':
    case 'U':
      return ZONE;
    default:
      return UNKNOWN_LOC;
  }
};

double intAndDecToDouble(int32_t val, int32_t decimals)
{
  return double(val) / pow(10, decimals);
}

std::string getTaxDescription(const TaxRulesRecord& rec, DataHandle& dataHandle, DateTime tktDate)
{
  std::string ans;
  const std::vector<const TaxReportingRecordInfo*> records = dataHandle.getTaxReportingRecord(
    rec.vendor(), rec.nation(), rec.taxCarrier(), rec.taxCode(), rec.taxType(), tktDate);

  if (!records.empty())
    ans = records.front()->taxName();

  return ans;
}

CurrencyCode currency(const TaxRulesRecord& rulesRec, DataHandle& dataHandle, DateTime tktDate)
{
  if (!rulesRec.taxCurrency().empty())
  {
    return rulesRec.taxCurrency();
  }
  else // a percent tax (exempt records already filtered out)
  {
    if (const Nation* nationRec = dataHandle.getNation(rulesRec.nation(), tktDate))
      return nationRec->primeCur();
    else
      return CurrencyCode(); // defensive
  }
}


TaxCodeReg*
convertToTaxRec(const TaxRulesRecord& rulesRec, DataHandle& dataHandle, DateTime tktDate)
{
  TaxCodeReg* ans = dataHandle.create<TaxCodeReg>();

  ans->taxType() = rulesRec.percentFlatTag();
  ans->taxCur() = currency(rulesRec, dataHandle, tktDate);

  if (rulesRec.percentFlatTag() == 'F')
    ans->taxAmt() = intAndDecToDouble(rulesRec.taxAmt(), rulesRec.taxCurDecimals());
  else
    ans->taxAmt() = intAndDecToDouble(rulesRec.taxPercent(), 6);

  {
    std::unique_ptr<TaxCodeGenText> vec(new TaxCodeGenText);
    vec->txtMsgs().push_back(getTaxDescription(rulesRec, dataHandle, tktDate));
    ans->taxCodeGenTexts().push_back(vec.get());
    vec.release();
  }
  ans->nation() = rulesRec.nation();

  std::tie(ans->firstTvlDate(), ans->lastTvlDate()) = makeTravelRange(rulesRec, tktDate);

  ans->posExclInd() = 'N';
  ans->posLocType() = mapLocType(rulesRec.posLocType());
  ans->posLoc() = rulesRec.posLoc();
  ans->poiExclInd() = 'N';
  ans->poiLocType() = mapLocType(rulesRec.poTktLocType());
  ans->poiLoc() = rulesRec.poTktLoc();

  if (rulesRec.taxPointTag() == 'S')
    ans->loc1Appl() = LocRestrictionValidator::TAX_ORIGIN;
  ans->loc1Type() = mapLocType(rulesRec.taxPointLoc1Type());
  ans->loc1() = rulesRec.taxPointLoc1();
  ans->loc1ExclInd() = 'N';

  return ans;
}

uint16_t adjustYear(uint16_t y)
{
  if (y > 0 && y < 2000)
    return uint16_t(2000 + y);
  else
    return y;
}

} // anonymous namespace

std::pair<DateTime, DateTime>
makeTravelRange(const TaxRulesRecord& rulesRec, const DateTime& tktDate)
{
  std::pair<DateTime, DateTime> ans;
  uint16_t firstY = adjustYear(rulesRec.tvlFirstYear());
  uint16_t firstM = rulesRec.tvlFirstMonth();
  uint16_t firstD = rulesRec.tvlFirstDay();
  uint16_t lastY = adjustYear(rulesRec.tvlLastYear());
  uint16_t lastM = rulesRec.tvlLastMonth();
  uint16_t lastD = rulesRec.tvlLastDay();

  if (!firstY && firstM && firstD && !lastY && lastM && lastD) // seasonal tax
  {
    if (firstM < lastM || (firstM == lastM && firstD < lastD))
    {
      ans.first = DateTime(tktDate.year(), firstM, firstD);
      ans.second = DateTime(tktDate.year(), lastM, lastD);
    }
    else
    {
      DateTime end(tktDate.year(), lastM, lastD);
      if (tktDate.date() <= end.date())
      {
        ans.first = DateTime(tktDate.year() - 1, firstM, firstD);
        ans.second = end;
      }
      else
      {
        ans.first = DateTime(tktDate.year(), firstM, firstD);
        ans.second = DateTime(tktDate.year() + 1, lastM, lastD);
      }
    }
  }
  else
  {
    if (firstY && firstM && firstD)
      ans.first = DateTime(firstY, firstM, firstD);
    else
      ans.first = DateTime::emptyDate();

    if (lastY && lastM && lastD)
      ans.second = DateTime(lastY, lastM, lastD);
    else
      ans.second = DateTime::emptyDate();
  }

  return ans;
}

std::vector<TaxCodeAndType> translateToAtpcoCodes(TaxCode taxCode)
{
  std::vector<TaxCodeAndType> ans;
  if (taxCode.equalToConst("US1"))
  {
    ans.push_back(TaxCodeAndType("US", "003"));
    ans.push_back(TaxCodeAndType("US", "004"));
  }
  else if (taxCode.equalToConst("US2"))
  {
    ans.push_back(TaxCodeAndType("US", "005"));
    ans.push_back(TaxCodeAndType("US", "006"));
  }
  else if (taxCode.size() >= 2)
  {
    TaxCode newCode = taxCode.substr(0, 2);
    if (boost::optional<std::string> newType = makeTaxType(taxCode))
      ans.push_back(TaxCodeAndType(newCode, *newType));
    else
      {} // unexpected tax code
  }
  else
  {
    // unexpected tax code
  }

  return ans;
}

const std::vector<TaxCodeReg*>&
getTaxRulesRecords(std::vector<TaxCodeReg*>& out, DataHandle& dataHandle, TaxCode taxCode,
                   DateTime ticketingDate)
{
  std::vector<TaxCodeReg*> ans;
  std::vector<TaxCodeAndType> codes = translateToAtpcoCodes(taxCode);

  if (fallback::fixed::ATPCO_TAX_X1byCodeDAORefactor())
  {
    for (const TaxCodeAndType& recCode : codes)
    {
      const std::vector<const TaxRulesRecord*>& taxRulesRecords =
        dataHandle.getTaxRulesRecordByCodeAndType(recCode.code, recCode.type, ticketingDate);

      for (const TaxRulesRecord* rulesRec : taxRulesRecords)
      {
        if (rulesRec->exemptTag() != 'X')
          ans.push_back(convertToTaxRec(*rulesRec, dataHandle, ticketingDate));
      }
    }
  }
  else
  {
    for (const TaxCodeAndType& recCode : codes)
    {
      const std::vector<const TaxRulesRecord*>& taxRulesRecords =
        dataHandle.getTaxRulesRecordByCode(recCode.code, ticketingDate);

      for (const TaxRulesRecord* rulesRec : taxRulesRecords)
      {
        if (recCode.type == rulesRec->taxType() && rulesRec->exemptTag() != 'X')
          ans.push_back(convertToTaxRec(*rulesRec, dataHandle, ticketingDate));
      }
    }
  }

  swap(out, ans);
  return out;
}

} // namespace tse

