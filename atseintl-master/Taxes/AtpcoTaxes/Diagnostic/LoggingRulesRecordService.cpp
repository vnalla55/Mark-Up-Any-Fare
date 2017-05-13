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
#include "Common/MoneyUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/CompactOptional.h"
#include "DataModel/Common/OptionalCode.h"
#include "DataModel/Services/RulesRecord.h"
#include "Diagnostic/LoggingRulesRecordService.h"

#include <boost/format.hpp>
#include <cassert>
#include <set>
#include <sstream>

namespace tax
{

namespace
{

using boost::format;

struct RulesRecordLess
{
  bool operator()(const RulesRecord& l, const RulesRecord& r) const
  {
    return std::tie(l.taxName, l.seqNo) < std::tie(r.taxName, r.seqNo);
    // skipping vendor: dupplicates /w diff vendor impossible?
  }
};

typedef std::set<RulesRecord, RulesRecordLess> RulesRecordCache;

void printConnectionTags(std::ostringstream& ans, const RulesRecord& rec)
{
  int i = 0;
  for (type::ConnectionsTag tag : rec.connectionsTags)
  {
    ++i;
    ans << format("CONNECTIONSTAG%1%=\"%2%\" ") % i % tag;
  }
}

void printTUT(std::ostringstream& ans, const RulesRecord& rec)
{
  for (int i = 1; i <= 10; ++i)
  {
    if (rec.applicableTaxableUnits.hasBit(i))
    {
      ans << format("TAXABLEUNITTAG%1%=\"X\" ") % i;
    }
  }
}

template <typename T>
std::string toString(const T& v)
{
  std::ostringstream ans;
  ans << v;
  return ans.str();
}

std::string toString(const type::MoneyAmount& v)
{
  std::ostringstream str;
  if (v.denominator() == 1)
    str << v.numerator() << ".";
  else
    str << std::fixed << std::setprecision(8) << amountToDouble(v);
  return str.str();
}

template <typename T, typename U>
void print(std::ostringstream& ans, std::string tagName, const T& val, const U& defval)
{
  if (val != defval)
    ans << format("%1%=\"%2%\" ") % tagName % toString(val);
}

template <typename T>
void print(std::ostringstream& ans, std::string tagName, const T& val)
{
  ans << format("%1%=\"%2%\" ") % tagName % toString(val);
}

template <typename T>
void printEnum(std::ostringstream& ans, std::string tagName, const T& val)
{
  if (val != T::Blank)
    ans << format("%1%=\"%2%\" ") % tagName % toString(val);
}

void printLocZone(std::ostringstream& ans, std::string tagName, const LocZone& val)
{
  printEnum(ans, tagName + "TYPE", val.type());
  print(ans, tagName, val.code(), UninitializedCode);
  print(ans, tagName + "ZONETBLNO", val.code(), UninitializedCode);
}


std::string toXml(const RulesRecordCache& cache)
{
  std::ostringstream ans;
  for (const RulesRecord& rec : cache)
  {
    ans << "  <RulesRecord ";

    ans << format("NATION=\"%1%\" PERCENTFLATTAG=\"%2%\" TAXCODE=\"%3%\" TAXTYPE=\"%4%\" "
                  "VENDOR=\"%5%\" TAXPOINTTAG=\"%6%\" SEQNO=\"%7%\" ")
           % rec.taxName.nation()
           % rec.taxName.percentFlatTag()
           % rec.taxName.taxCode()
           % rec.taxName.taxType()
           % rec.vendor
           % rec.taxName.taxPointTag()
           % rec.seqNo;

    print(ans, "TAXCARRIER", rec.taxName.taxCarrier());
    print(ans, "TAXREMITTANCEID", rec.taxName.taxRemittanceId());

    if (rec.taxName.percentFlatTag() == type::PercentFlatTag::Flat)
      print(ans, "TAXAMT", rec.taxAmt);
    else
      print(ans, "TAXPERCENT", rec.taxPercent);

    print(ans, "TAXCURRENCY", rec.taxCurrency, UninitializedCode);
    print(ans, "TAXCURDECIMALS", rec.taxCurDecimals);
    print(ans, "RTNTOORIG", rec.rtnToOrig, type::RtnToOrig::Blank);
    print(ans, "EXEMPTTAG", rec.exemptTag, type::ExemptTag::Blank);
    print(ans, "EFFDATE", rec.effDate, type::Date());
    print(ans, "DISCDATE", rec.discDate, type::Date());
    print(ans, "EXPIREDATE", rec.expiredDate, type::Timestamp());
    print(ans, "TVLFIRSTYEAR", rec.firstTravelYear, 0);
    print(ans, "TVLLASTYEAR", rec.lastTravelYear, 0);
    print(ans, "TVLFIRSTMONTH", rec.firstTravelMonth, 0);
    print(ans, "TVLLASTMONTH", rec.lastTravelMonth, 0);
    print(ans, "TVLFIRSTDAY", rec.firstTravelDay, 0);
    print(ans, "TVLLASTDAY", rec.lastTravelDay, 0);
    printEnum(ans, "TRAVELDATEAPPTAG", rec.travelDateTag);
    print(ans, "TICKETEDPOINTTAG", rec.ticketedPointTag, type::TicketedPointTag::MatchTicketedPointsOnly);
    print(ans, "JRNYIND", rec.jrnyInd, type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ);
    printLocZone(ans, "JRNYLOC1", rec.jrnyLocZone1);
    printLocZone(ans, "JRNYLOC2", rec.jrnyLocZone2);
    printLocZone(ans, "TRVLWHOLLYWITHINLOC", rec.trvlWhollyWithin);
    printLocZone(ans, "JRNYINCLUDESLOC", rec.jrnyIncludes);
    printEnum(ans, "TAXPOINTLOC1TRNSFRTYPE", rec.taxPointLoc1TransferType);
    printEnum(ans, "TAXPOINTLOC1STOPOVERTAG", rec.taxPointLoc1StopoverTag);
    printEnum(ans, "TAXPOINTLOC2STOPOVERTAG", rec.taxPointLoc2StopoverTag);
    print(ans, "HISTORICSALEEFFDATE", rec.histSaleEffDate, type::Date());
    print(ans, "HISTORICSALEDISCDATE", rec.histSaleDiscDate, type::Date());
    print(ans, "HISTORICTRVLEFFDATE", rec.histTrvlEffDate, type::Date());
    print(ans, "HISTORICTRVLDISCDATE", rec.histTrvlDiscDate, type::Date());
    printLocZone(ans, "TAXPOINTLOC1", rec.taxPointLocZone1);
    printLocZone(ans, "TAXPOINTLOC2", rec.taxPointLocZone2);
    printLocZone(ans, "TAXPOINTLOC3", rec.taxPointLocZone3);
    printEnum(ans, "TAXPOINTLOC2INTLDOMIND", rec.taxPointLoc2IntlDomInd);
    print(ans, "STOPOVERTIMETAG", rec.stopoverTimeTag, UninitializedCode);
    printEnum(ans, "STOPOVERTIMEUNIT", rec.stopoverTimeUnit);
    printEnum(ans, "TAXPOINTLOC1INTDOMIND", rec.taxPointLoc1IntlDomInd);
    printEnum(ans, "TAXPOINTLOC2COMPARE", rec.taxPointLoc2Compare);
    printEnum(ans, "TAXPOINTLOC3GEOTYPE", rec.taxPointLoc3GeoType);
    print(ans, "CURRENCYOFSALE", rec.currencyOfSale, UninitializedCode);
    printLocZone(ans, "POTKTLOC", rec.pointOfTicketing);
    printLocZone(ans, "POSLOC", rec.pointOfSale);
    print(ans, "TAXAPPLIMIT", rec.taxApplicationLimit, type::TaxApplicationLimit::Unlimited);
    print(ans, "CARRIERFLTITEMNO1", rec.carrierFlightItemBefore, 0u);
    print(ans, "CARRIERFLTITEMNO2", rec.carrierFlightItemAfter, 0u);
    print(ans, "CARRIERAPPLITEMNO1", rec.carrierApplicationItem, 0u);
    print(ans, "MINTAX", rec.minTax, 0u);
    print(ans, "MAXTAX", rec.maxTax, 0u);
    print(ans, "MINMAXCURRENCY", rec.minMaxCurrency, UninitializedCode);
    print(ans, "MINMAXDECIMALS", rec.minMaxDecimals, 0u);
    printEnum(ans, "TKTVALAPPLQUALIFIER", rec.tktValApplQualifier);
    print(ans, "TKTVALCURRENCY", rec.tktValCurrency, UninitializedCode);
    print(ans, "TKTVALMIN", rec.tktValMin, 0u);
    print(ans, "TKTVALMAX", rec.tktValMax, 0u);
    print(ans, "TKTVALCURDECIMALS", rec.tktValCurrDecimals);
    print(ans, "SERVICEBAGGAGEITEMNO", rec.serviceBaggageItemNo, 0u);
    print(ans, "PSGRTYPECODEITEMNO", rec.passengerTypeCodeItem, 0u);
    printEnum(ans, "NETREMITAPPLTAG", rec.netRemitApplTag);
    print(ans, "SECTORDETAILITEMNO", rec.sectorDetailItemNo, 0u);
    print(ans, "ALTERNATERULEREFTAG", rec.alternateRuleRefTag, 0);
    printEnum(ans, "SECTORDETAILAPPLTAG", rec.sectorDetailApplTag);
    print(ans, "TAXMATCHINGAPPLTAG", rec.taxMatchingApplTag, UninitializedCode);
    printEnum(ans, "SERVICEBAGGAGEAPPLTAG", rec.serviceBaggageApplTag);
    print(ans, "CALCORDER", rec.calcOrder, 1);
    printEnum(ans, "TAXAPPLIESTOTAGIND", rec.taxAppliesToTagInd);
    printEnum(ans, "PAIDBY3RDPARTYTAG", rec.paidBy3rdPartyTag);
    printEnum(ans, "TAXROUNDUNIT", rec.taxRoundUnit);
    printEnum(ans, "TAXROUNDDIR", rec.taxRoundDir);
    printLocZone(ans, "PODELIVERYLOC", rec.pointOfDelivery);
    print(ans, "TAXPROCESSINGAPPLTAG", rec.taxProcessingApplTag, UninitializedCode);
    print(ans, "SVCFEESSECURITYITEMNO", rec.svcFeesSecurityItemNo, 0u);

    printConnectionTags(ans, rec);
    printTUT(ans, rec);

    ans << "/>\n";
  }

  return ans.str();
}

} // anonymous namespace

class LoggingRulesRecordService::Impl
{
  mutable RulesRecordCache _rulesRecords;
  friend class LoggingRulesRecordService;
};

LoggingRulesRecordService::LoggingRulesRecordService(std::unique_ptr<BaseService> base)
: _base{std::move(base)}
, _impl {new Impl{}}
{
}

LoggingRulesRecordService::LoggingRulesRecordService(BaseService& base)
: _base{base}
, _impl {new Impl{}}
{
}

LoggingRulesRecordService::~LoggingRulesRecordService() {}

auto LoggingRulesRecordService::impl() const -> const Impl&
{
  assert (_impl);
  return *_impl;
}

auto LoggingRulesRecordService::impl() -> Impl&
{
  assert (_impl);
  return *_impl;
}

auto LoggingRulesRecordService::getTaxRulesContainers(const type::Nation& nation,
                                                      const type::TaxPointTag& taxPointTag,
                                                      const type::Timestamp& ticketingDate) const
                                                      -> ByNationConstValue
{
  std::vector<RulesRecord> recs = _base().getTaxRecords(nation, taxPointTag, ticketingDate);
  impl()._rulesRecords.insert(recs.cbegin(), recs.cend());
  return _base().getTaxRulesContainers(nation, taxPointTag, ticketingDate);
}

auto LoggingRulesRecordService::getTaxRecord(type::Nation nation,
                                             type::TaxCode taxCode,
                                             type::TaxType taxType,
                                             type::Index seqNo,
                                             const type::Timestamp& ticketingDate) const
                                             -> SharedTaxRulesRecord // nullable
{
  SharedTaxRulesRecord ans = _base().getTaxRecord(nation, taxCode, taxType, seqNo, ticketingDate);
  if (ans)
    impl()._rulesRecords.insert(*ans);
  return ans;
}

auto LoggingRulesRecordService::getTaxRecords(const type::Nation& nation,
                                              const type::TaxPointTag& taxPointTag,
                                              const type::Timestamp& ticketingDate) const
                                              -> std::vector<RulesRecord>
{
  std::vector<RulesRecord> ans = _base().getTaxRecords(nation, taxPointTag, ticketingDate);
  impl()._rulesRecords.insert(ans.cbegin(), ans.cend());
  return ans;
}

std::shared_ptr<const std::vector<LoggingRulesRecordService::SharedTaxRulesRecord>>
LoggingRulesRecordService::getTaxRecords(const type::TaxCode& taxCode,
                                         const type::Timestamp& ticketingDate) const
{
  std::shared_ptr<const std::vector<SharedTaxRulesRecord>> ans =
      _base().getTaxRecords(taxCode, ticketingDate);

  if (ans)
  {
    for (const SharedTaxRulesRecord& record : *ans)
    {
      if (record)
      {
        impl()._rulesRecords.insert(*record);
      }
    }
  }

  return ans;
}

std::string LoggingRulesRecordService::getLog()
{
  return toXml(impl()._rulesRecords);
}

} // namespace tax

