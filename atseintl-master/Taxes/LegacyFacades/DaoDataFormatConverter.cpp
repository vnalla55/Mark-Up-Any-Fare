// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the  program(s)
//  have been supplied.
//
// ----------------------------------------------------------------------------

#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "DBAccess/PaxTypeCodeInfo.h"
#include "DBAccess/SectorDetailInfo.h"
#include "DBAccess/ServiceBaggageInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/TaxReportingRecordInfo.h"
#include "DBAccess/TaxReissue.h"
#include "DBAccess/TaxRulesRecord.h"
#include "Taxes/AtpcoTaxes/Common/Consts.h"
#include "Taxes/AtpcoTaxes/Common/Convert.h"
#include "Taxes/AtpcoTaxes/Common/LocZone.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/Common/TaxableUnitTagSet.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/SafeEnums.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierApplication.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierFlight.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/PassengerTypeCode.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/ReportingRecord.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/RulesRecord.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/SectorDetail.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/ServiceBaggage.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/ServiceFeeSecurity.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/TaxReissue.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"

#include <boost/lexical_cast.hpp>

namespace tse
{
namespace
{

int16_t
getValidTravelYear(int16_t year)
{
  if (year != tax::BLANK_YEAR && year < 2000)
    return year + 2000;

  return year;
}

std::vector<tax::type::CarrierCode>
getTaxCarrierVector(std::vector<tse::CarrierCode> tseVec)
{
  std::vector<tax::type::CarrierCode> ret;
  ret.reserve(tseVec.size());

  for (const tse::CarrierCode& carrierCode : tseVec)
    ret.push_back(toTaxCarrierCode(carrierCode));

  return ret;
}

} // anonymous namespace


tax::type::Date
DaoDataFormatConverter::fromDate(const tse::DateTime& tseDT, bool isUpperLimit)
{
  if (tseDT.isPosInfinity())
  {
    return tax::type::Date::pos_infinity();
  }
  else if (UNLIKELY(tseDT.isNegInfinity()))
  {
    return (isUpperLimit ? tax::type::Date::pos_infinity() : tax::type::Date::neg_infinity());
  }
  else if (tseDT.isEmptyDate() || tseDT.isOpenDate() || tseDT.isNotADate() || !tseDT.isValid())
  {
    return tax::type::Date::blank_date();
  }

  return tax::type::Date(tseDT.year(), tseDT.month(), tseDT.day());
}

tax::type::Timestamp
DaoDataFormatConverter::fromTimestamp(const tse::DateTime& tseDT, bool isUpperLimit)
{
  tax::type::Date date = fromDate(tseDT, isUpperLimit);
  tax::type::Time time;

  if (tseDT.isPosInfinity())
  {
    time = tax::type::Time::blank_time();
  }
  else if (tseDT.isNegInfinity())
  {
    time = tax::type::Time::blank_time();
  }
  else if (tseDT.isEmptyDate() || tseDT.isOpenDate() || tseDT.isNotADate() || !tseDT.isValid())
  {
    time = tax::type::Time::blank_time();
  }
  else
  {
    time = tax::type::Time(static_cast<uint16_t>(tseDT.hours()),
                           static_cast<uint16_t>(tseDT.minutes()));
  }

  return tax::type::Timestamp(date, time);
}

void
DaoDataFormatConverter::setLocZone(tax::LocZone& locZone,
                                   tse::LocTypeCode type,
                                   uint32_t zone,
                                   tse::LocCode locCode)
{
  setTaxEnumValue(locZone.type(), type);

  std::string zoneStr = zone ? boost::lexical_cast<std::string>(zone) : std::string(locCode);
  codeFromString(zoneStr, locZone.code());
  // if conversion fails, we leave the previous value
}

namespace
{

tax::Tariff
toTariff(TariffNumber tnum, FareTariffInd tind)
{
  return tax::Tariff{tax::Convert::intToShort(tnum), toTaxFareTariff(tind)};
}

void
addTaxableUnit(tax::TaxableUnitTagSet& tuSet, const tax::type::TaxableUnit& taxableUnit, char tag)
{
  tax::type::TaxableUnitTag t;
  setTaxEnumValue(t, tag);

  if (t != tax::type::TaxableUnitTag::Blank)
    tuSet.setTag(taxableUnit);
}

} // anonymous namespace

void
DaoDataFormatConverter::convert(const tse::TaxRulesRecord& v2rr, tax::RulesRecord& rr)
{
  rr.taxName.nation() = toTaxNationCode(v2rr.nation());
  rr.taxName.taxCode() = toTaxCode(v2rr.taxCode());
  rr.taxName.taxType() = toTaxType(v2rr.taxType());
  setTaxEnumValue(rr.taxName.percentFlatTag(), v2rr.percentFlatTag());
  setTaxEnumValue(rr.taxName.taxPointTag(), v2rr.taxPointTag());
  rr.taxName.taxCarrier() = toTaxCarrierCode(v2rr.taxCarrier());

  rr.vendor = toTaxVendorCode(v2rr.vendor());
  rr.seqNo = v2rr.seqNo();
  setTaxEnumValue(rr.ticketedPointTag, v2rr.ticketedPointTag());
  setTaxEnumValue(rr.exemptTag, v2rr.exemptTag());
  rr.taxAmt = v2rr.taxAmt();
  rr.taxCurrency = toTaxCurrencyCode(v2rr.taxCurrency());
  rr.taxCurDecimals = v2rr.taxCurDecimals();
  rr.taxPercent = v2rr.taxPercent();
  rr.effDate = fromDate(v2rr.effDate());
  rr.discDate = fromDate(v2rr.discDate(), true);
  rr.expiredDate = fromTimestamp(v2rr.expireDate(), true);
  rr.firstTravelYear = v2rr.tvlFirstYear() != 0 ? v2rr.tvlFirstYear() : tax::BLANK_YEAR;
  rr.firstTravelMonth = v2rr.tvlFirstMonth();
  rr.firstTravelDay = v2rr.tvlFirstDay();
  rr.firstTravelDate = tax::type::Date::make_date(getValidTravelYear(rr.firstTravelYear),
                                                  rr.firstTravelMonth,
                                                  rr.firstTravelDay);
  rr.lastTravelYear = v2rr.tvlLastYear() != 0 ? v2rr.tvlLastYear() : tax::BLANK_YEAR;
  rr.lastTravelMonth = v2rr.tvlLastMonth();
  rr.lastTravelDay = v2rr.tvlLastDay();
  rr.lastTravelDate = tax::type::Date::make_date(getValidTravelYear(rr.lastTravelYear),
                                                 rr.lastTravelMonth,
                                                 rr.lastTravelDay);
  rr.histSaleEffDate = fromDate(v2rr.historicSaleEffDate());
  rr.histSaleDiscDate = fromDate(v2rr.historicSaleDiscDate(), true);
  rr.histTrvlEffDate = fromDate(v2rr.historicTrvlEffDate());
  rr.histTrvlDiscDate = fromDate(v2rr.historicTrvlDiscDate(), true);
  setTaxEnumValue(rr.travelDateTag, v2rr.travelDateAppTag());
  setTaxEnumValue(rr.rtnToOrig, v2rr.rtnToOrig());
  setTaxEnumValue(rr.jrnyInd, v2rr.jrnyInd());
  setLocZone(rr.pointOfTicketing, v2rr.poTktLocType(), v2rr.poTktLocZoneTblNo(), v2rr.poTktLoc());
  setLocZone(rr.pointOfDelivery,
             v2rr.poDeliveryLocType(),
             v2rr.poDeliveryZoneTblNo(),
             v2rr.poDeliveryLoc());
  setLocZone(rr.pointOfSale, v2rr.posLocType(), v2rr.posLocZoneTblNo(), v2rr.posLoc());
  setLocZone(rr.jrnyLocZone1, v2rr.jrnyLoc1Type(), v2rr.jrnyLoc1ZoneTblNo(), v2rr.jrnyLoc1());
  setLocZone(rr.jrnyLocZone2, v2rr.jrnyLoc2Type(), v2rr.jrnyLoc2ZoneTblNo(), v2rr.jrnyLoc2());
  setLocZone(rr.trvlWhollyWithin,
             v2rr.trvlWhollyWithinLocType(),
             v2rr.trvlWhollyWithinLocZoneTblNo(),
             v2rr.trvlWhollyWithinLoc());
  setLocZone(rr.jrnyIncludes,
             v2rr.jrnyIncludesLocType(),
             v2rr.jrnyIncludesLocZoneTblNo(),
             v2rr.jrnyIncludesLoc());
  setLocZone(rr.taxPointLocZone1,
             v2rr.taxPointLoc1Type(),
             v2rr.taxPointLoc1ZoneTblNo(),
             v2rr.taxPointLoc1());
  setLocZone(rr.taxPointLocZone2,
             v2rr.taxPointLoc2Type(),
             v2rr.taxPointLoc2ZoneTblNo(),
             v2rr.taxPointLoc2());
  setLocZone(rr.taxPointLocZone3,
             v2rr.taxPointLoc3Type(),
             v2rr.taxPointLoc3ZoneTblNo(),
             v2rr.taxPointLoc3());

  rr.stopoverTimeTag = toTaxStopoverTimeTag(v2rr.stopoverTimeTag());
  setTaxEnumValue(rr.stopoverTimeUnit, v2rr.stopoverTimeUnit());

  setTaxEnumValue(rr.taxPointLoc1TransferType, v2rr.taxPointLoc1TrnsfrType());
  setTaxEnumValue(rr.taxPointLoc1StopoverTag, v2rr.taxPointLoc1StopoverTag());
  setTaxEnumValue(rr.taxPointLoc1IntlDomInd, v2rr.taxPointLoc1IntDomInd());
  setTaxEnumValue(rr.taxPointLoc2IntlDomInd,v2rr.taxPointLoc2IntlDomInd());
  setTaxEnumValue(rr.taxPointLoc2StopoverTag, v2rr.taxPointLoc2StopoverTag());
  setTaxEnumValue(rr.taxPointLoc2Compare, v2rr.taxPointLoc2Compare());
  setTaxEnumValue(rr.taxPointLoc3GeoType, v2rr.taxPointLoc3GeoType());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag1());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag2());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag3());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag4());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag5());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag6());
  addConnectionsTag(rr.connectionsTags, v2rr.connectionsTag7());
  rr.currencyOfSale = toTaxCurrencyCode(v2rr.currencyOfSale());
  setTaxEnumValue(rr.taxApplicationLimit, v2rr.taxAppLimit());
  rr.carrierFlightItemBefore = v2rr.carrierFltItemNo1();
  rr.carrierFlightItemAfter = v2rr.carrierFltItemNo2();
  rr.carrierApplicationItem = v2rr.carrierApplItemNo1();
  addTaxableUnit(rr.applicableTaxableUnits, tax::type::TaxableUnit::YqYr, v2rr.taxableUnitTag1());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::TicketingFee, v2rr.taxableUnitTag2());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::OCFlightRelated, v2rr.taxableUnitTag3());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::OCTicketRelated, v2rr.taxableUnitTag4());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::OCMerchandise, v2rr.taxableUnitTag5());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::OCFareRelated, v2rr.taxableUnitTag6());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::BaggageCharge, v2rr.taxableUnitTag7());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::TaxOnTax, v2rr.taxableUnitTag8());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::Itinerary, v2rr.taxableUnitTag9());
  addTaxableUnit(
      rr.applicableTaxableUnits, tax::type::TaxableUnit::ChangeFee, v2rr.taxableUnitTag10());
  rr.minTax = v2rr.minTax();
  rr.maxTax = v2rr.maxTax();
  rr.minMaxCurrency = toTaxCurrencyCode(v2rr.minMaxCurrency());
  rr.minMaxDecimals = v2rr.minMaxDecimals();
  setTaxEnumValue(rr.tktValApplQualifier, v2rr.tktValApplQualifier());
  rr.tktValCurrency = toTaxCurrencyCode(v2rr.tktValCurrency());
  rr.tktValMin = v2rr.tktValMin();
  rr.tktValMax = v2rr.tktValMax();
  rr.tktValCurrDecimals = v2rr.tktValCurDecimals();
  rr.serviceBaggageItemNo = v2rr.serviceBaggageItemNo();
  rr.passengerTypeCodeItem = v2rr.psgrTypeCodeItemNo();
  rr.sectorDetailItemNo = v2rr.sectorDetailItemNo();
  setTaxEnumValue(rr.netRemitApplTag, v2rr.netRemitApplTag());
  rr.taxMatchingApplTag = toTaxMatchApplTag(v2rr.taxMatchingApplTag());
  setTaxEnumValue(rr.serviceBaggageApplTag, v2rr.serviceBaggageApplTag());
  rr.calcOrder = v2rr.calcOrder();
  setTaxEnumValue(rr.paidBy3rdPartyTag, v2rr.paidBy3rdPartyTag());

  setTaxEnumValue(rr.taxAppliesToTagInd, v2rr.taxAppliesToTagInd());
  setTaxEnumValue(rr.sectorDetailApplTag, v2rr.sectorDetailApplTag());
  rr.alternateRuleRefTag = v2rr.alternateRuleRefTag();
  rr.createDate = fromDate(v2rr.createDate());
  setTaxEnumValue(rr.taxRoundUnit, v2rr.taxRoundUnit());
  setTaxEnumValue(rr.taxRoundDir, v2rr.taxRoundDir());
  rr.taxProcessingApplTag = toTaxApplicationTag(v2rr.taxProcessingApplTag());
  rr.svcFeesSecurityItemNo = v2rr.svcFeesSecurityItemNo();
  setTaxEnumValue(rr.outputTypeIndicator, v2rr.outputTypeInd());
  setTaxEnumValue(rr.vatInclusiveInd, v2rr.vatInclusiveInd());

  // TODO uninitialized
  // taxTextTblItemNo
  // ratdDate
  // versionDate
  // lockDate
}

void
DaoDataFormatConverter::convert(const tse::TaxCarrierFlightInfo& v2cf, tax::CarrierFlight& cf)
{
  cf.vendor = toTaxVendorCode(v2cf.vendor());
  cf.itemNo = v2cf.itemNo();

  cf.segments.resize(v2cf.segs().size());
  for (const CarrierFlightSeg * seg: v2cf.segs())
  {
    tax::CarrierFlightSegment& cfs = cf.segments[seg->orderNo() - 1];

    cfs.marketingCarrier = toTaxCarrierCode(seg->marketingCarrier());
    cfs.operatingCarrier = toTaxCarrierCode(seg->operatingCarrier());

    if (seg->flt1() == 0)
    {
      cfs.flightFrom = 0; // invalid range
      cfs.flightTo = -1; // will match nothing
    }
    else if (seg->flt1() == -1)
    {
      cfs.flightFrom = -1; // match every flight
      cfs.flightTo = tax::MAX_FLIGHT_NUMBER;
    }
    else
    {
      cfs.flightFrom = seg->flt1();
      cfs.flightTo = (seg->flt2() == 0 ? seg->flt1() : seg->flt2());
    }
  }
}

void
DaoDataFormatConverter::convert(const tse::TaxCarrierAppl& from, tax::CarrierApplication& to)
{
  to.vendor = toTaxVendorCode(from.vendor());
  to.itemNo = from.itemNo();

  for (const TaxCarrierApplSeg* seg: from.segs())
  {
    assert (seg);
    to.entries.push_back(new tax::CarrierApplicationEntry);
    setTaxEnumValue(to.entries.back().applind, seg->applInd());
    to.entries.back().carrier = toTaxCarrierCode(seg->carrier());
  }
}

void
DaoDataFormatConverter::convert(const tse::PaxTypeCodeInfo& v2pax, tax::PassengerTypeCodeItem& pas)
{
  setTaxEnumValue(pas.applTag, v2pax.applyTag());
  pas.passengerType = toTaxPassengerCode(v2pax.psgrType());
  pas.minimumAge = static_cast<int16_t>(v2pax.paxMinAge());
  pas.maximumAge = static_cast<int16_t>(v2pax.paxMaxAge());
  setTaxEnumValue(pas.statusTag, v2pax.paxStatus());
  setTaxEnumValue(pas.location.type(), v2pax.loc().locType());
  pas.location.code() = toTaxLocZoneCode(v2pax.loc().loc());
  setTaxEnumValue(pas.matchIndicator, v2pax.ptcMatchIndicator());
}

void
DaoDataFormatConverter::convert(const tse::SvcFeesSecurityInfo& v2Entry,
                                tax::ServiceFeeSecurityItem& taxEntry)
{
  taxEntry.travelAgencyIndicator = (v2Entry.travelAgencyInd() == ' ')
                                       ? tax::type::TravelAgencyIndicator::Blank
                                       : tax::type::TravelAgencyIndicator::Agency;
  taxEntry.carrierGdsCode = (v2Entry.carrierGdsCode());
  taxEntry.dutyFunctionCode = v2Entry.dutyFunctionCode();
  setLocZone(taxEntry.location, v2Entry.loc().locType(), 0, v2Entry.loc().loc());
  setTaxEnumValue(taxEntry.codeType, LocType(v2Entry.code().locType()));
  taxEntry.code = v2Entry.code().loc();
  setTaxEnumValue(taxEntry.viewBookTktInd, v2Entry.viewBookTktInd());
}

void
DaoDataFormatConverter::convert(const tse::TaxReissue& v2Reissue,
                                tax::TaxReissue& taxReissue)
{
  taxReissue.amount = tax::doubleToAmount(v2Reissue.taxAmt());
  taxReissue.currency = toTaxCurrencyCode(v2Reissue.currencyCode());
  taxReissue.currencyDec = v2Reissue.currencyNoDec();
  setTaxEnumValue(taxReissue.locExceptTag, v2Reissue.reissueExclLocInd());
  taxReissue.locCode = v2Reissue.reissueLoc();
  setTaxEnumValue(taxReissue.locType, v2Reissue.reissueLocType());
  setTaxEnumValue(taxReissue.refundableTag, v2Reissue.refundInd());
  taxReissue.seqNo = v2Reissue.seqNo();
  taxReissue.taxCode = toTaxCode(v2Reissue.taxCode());
  taxReissue.taxType = toTaxType(v2Reissue.taxType());
  taxReissue.ticketingCarriers = getTaxCarrierVector(v2Reissue.validationCxr());
  setTaxEnumValue(taxReissue.ticketingExceptTag, v2Reissue.tktlCxrExclInd());
}

void
DaoDataFormatConverter::convert(const tse::ServiceBaggageInfo& v2Entry,
                                tax::ServiceBaggageEntry& taxEntry)
{
  setTaxEnumValue(taxEntry.applTag, v2Entry.applyTag());
  taxEntry.taxTypeSubcode = v2Entry.taxTypeSubCode();
  taxEntry.taxCode = toTaxCode(v2Entry.taxCode());
  setTaxEnumValue(taxEntry.optionalServiceTag, v2Entry.svcType());
  taxEntry.group = v2Entry.attrGroup();
  taxEntry.subGroup = v2Entry.attrSubGroup();
  taxEntry.feeOwnerCarrier = toTaxCarrierCode(v2Entry.feeOwnerCxr());
}

void
DaoDataFormatConverter::convert(const tse::SectorDetailInfo& v2Entry,
                                tax::SectorDetailEntry& taxEntry)
{
  setTaxEnumValue(taxEntry.applTag, v2Entry.applyTag());
  taxEntry.equipmentCode = v2Entry.equipmentCode();
  taxEntry.fareOwnerCarrier = toTaxCarrierCode(v2Entry.fareOwningCxr());
  setTaxEnumValue(taxEntry.cabinCode, v2Entry.cabinCode());
  taxEntry.fareType = v2Entry.fareType().c_str();
  taxEntry.rule = v2Entry.rule();
  taxEntry.reservationCodes[0] = v2Entry.rbd1();
  taxEntry.reservationCodes[1] = v2Entry.rbd2();
  taxEntry.reservationCodes[2] = v2Entry.rbd3();
  taxEntry.tariff = toTariff(v2Entry.fareTariff(), v2Entry.fareTariffInd());
  taxEntry.fareBasisTktDesignator = v2Entry.fareBasisTktDesignator();
  taxEntry.ticketCode = v2Entry.tktCode().c_str();
  taxEntry.seqNo = v2Entry.seqNo();
}

void
DaoDataFormatConverter::addConnectionsTag(std::set<tax::type::ConnectionsTag>& tagSet, char tag)
{
  if (tag != ' ')
    tagSet.insert(static_cast<tax::type::ConnectionsTag>(tag));
}

void
DaoDataFormatConverter::convert(const tse::TaxReportingRecordInfo& v2Entry,
                                tax::ReportingRecordEntry& taxEntry)
{
  taxEntry.taxLabel = v2Entry.taxName();
}

void
DaoDataFormatConverter::convert(const std::vector<const tse::TaxReportingRecordInfo*>& v2Data,
                                tax::ReportingRecord& taxData)
{
  if (v2Data.empty())
    return;

  const TaxReportingRecordInfo& v2Entry = *v2Data.front();

  taxData.vendor = toTaxVendorCode(v2Entry.vendor());
  taxData.nation = toTaxNationCode(v2Entry.nationCode());
  taxData.taxCarrier = toTaxCarrierCode(v2Entry.taxCarrier());
  taxData.taxCode = toTaxCode(v2Entry.taxCode());
  taxData.taxType = toTaxType(v2Entry.taxType());
  taxData.effDate = fromTimestamp(v2Entry.effDate(), true);
  taxData.discDate = fromTimestamp(v2Entry.discDate(), true);
  taxData.isVatTax = v2Entry.vatInd() == 'Y';
  taxData.isCommissionable = v2Entry.commisionableTaxTag() == 'Y';
  taxData.isInterlineable = v2Entry.interlineAbleTaxTag() == 'Y';
  setTaxEnumValue(taxData.taxOrChargeTag, v2Entry.taxCharge());
  setTaxEnumValue(taxData.refundableTag, v2Entry.refundableTaxTag());
  setTaxEnumValue(taxData.accountableDocTag, v2Entry.accountableDocTaxTag());
  taxData.taxTextItemNo = v2Entry.taxTextItemNo();
  taxData.taxApplicableToItemNo = v2Entry.taxApplicableToTblNo();
  taxData.taxRateItemNo = v2Entry.taxRateTextTblNo();
  taxData.taxExemptionsItemNo = v2Entry.taxExemptionTextTblNo();
  taxData.taxCollectRemitItemNo = v2Entry.taxCollectNrEmmitTblNo();
  taxData.taxingAuthorityItemNo = v2Entry.taxingAuthorityTextTblNo();
  taxData.taxCommentsItemNo = v2Entry.taxCommentsTextTblNo();
  taxData.taxSpecialInstructionsItemNo= v2Entry.taxSplInstructionsTblNo();

  convertEntries(v2Data, taxData);
}

void
DaoDataFormatConverter::convert(const tse::TaxReportingRecordInfo& v2Entry,
                                tax::ReportingRecord& taxData)
{

  taxData.vendor = toTaxVendorCode(v2Entry.vendor());
  taxData.nation = toTaxNationCode(v2Entry.nationCode());
  taxData.taxCarrier = toTaxCarrierCode(v2Entry.taxCarrier());
  taxData.taxCode = toTaxCode(v2Entry.taxCode());
  taxData.taxType = toTaxType(v2Entry.taxType());
  taxData.effDate = fromTimestamp(v2Entry.effDate(), true);
  taxData.discDate = fromTimestamp(v2Entry.discDate(), true);
  taxData.isVatTax = v2Entry.vatInd() == 'Y';
  taxData.isCommissionable = v2Entry.commisionableTaxTag() == 'Y';
  taxData.isInterlineable = v2Entry.interlineAbleTaxTag() == 'Y';
  setTaxEnumValue(taxData.taxOrChargeTag, v2Entry.taxCharge());
  setTaxEnumValue(taxData.refundableTag, v2Entry.refundableTaxTag());
  setTaxEnumValue(taxData.accountableDocTag, v2Entry.accountableDocTaxTag());
  taxData.taxTextItemNo = v2Entry.taxTextItemNo();
  taxData.taxApplicableToItemNo = v2Entry.taxApplicableToTblNo();
  taxData.taxRateItemNo = v2Entry.taxRateTextTblNo();
  taxData.taxExemptionsItemNo = v2Entry.taxExemptionTextTblNo();
  taxData.taxCollectRemitItemNo = v2Entry.taxCollectNrEmmitTblNo();
  taxData.taxingAuthorityItemNo = v2Entry.taxingAuthorityTextTblNo();
  taxData.taxCommentsItemNo = v2Entry.taxCommentsTextTblNo();
  taxData.taxSpecialInstructionsItemNo= v2Entry.taxSplInstructionsTblNo();
  taxData.entries.push_back(new tax::ReportingRecord::entry_type);
  convert(v2Entry, taxData.entries.back());
}

} // tse
