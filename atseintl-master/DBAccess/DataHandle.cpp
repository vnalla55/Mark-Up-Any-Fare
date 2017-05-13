//----------------------------------------------------------------------------
//
//  File:           DataHandle.cpp
//
//  Description:    Entry point to the database access layer.
//
//  Updates:
//
//  Copyright Sabre 2004
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the progr2am(s)
//  have been supplied.
//----------------------------------------------------------------------------
#include "DBAccess/DataHandle.h"

#include "Common/FallbackUtil.h"
#include "DBAccess/AccompaniedTravelInfo.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/Brand.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/Deposits.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/DSTDAO.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/Groups.h"
#include "DBAccess/HipMileageExceptInfo.h"
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/RuleApplication.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "DBAccess/SvcFeesFeatureInfo.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "DBAccess/Tours.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/TravelRestriction.h"
#include "DBAccess/TSSCache.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"
#include "DBAccess/VisitAnotherCountry.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"



#include <algorithm>
#include <functional>

namespace tse
{
class SpanishReferenceFareInfo;

const State*
DataHandle::getState(const NationCode& nationCode, const StateCode& stateCode, const DateTime& date)
{
  return getStateData(nationCode, stateCode, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SurfaceSectorExemptionInfo*>&
DataHandle::getSurfaceSectorExemptionInfo(const CarrierCode& carrierCode,
                                          const DateTime& ticketDate)
{
  return getSurfaceSectorExemptionInfoData(carrierCode, _deleteList, ticketDate, isHistorical());
}

std::vector<const tse::FDSuppressFare*>&
DataHandle::getSuppressFareList(const PseudoCityCode& pseudoCityCode,
                                const Indicator pseudoCityType,
                                const TJRGroup& ssgGroupNo,
                                const CarrierCode& carrierCode,
                                const DateTime& travelDate)
{
  return getSuppressFareListData(pseudoCityCode,
                                 pseudoCityType,
                                 ssgGroupNo,
                                 carrierCode,
                                 travelDate,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical());
}

std::vector<const tse::FDHeaderMsg*>&
DataHandle::getHeaderMsgDataList(const PseudoCityCode& pseudoCityCode,
                                 const PseudoCityType& pseudoCityType,
                                 const Indicator& userApplType,
                                 const std::string& userAppl,
                                 const TJRGroup& tjrGroup,
                                 const DateTime& date)
{
  return getHeaderMsgDataListData(
      pseudoCityCode, pseudoCityType, userApplType, userAppl, tjrGroup, date, _deleteList);
}

const RuleItemInfo*
DataHandle::getRuleItemInfo(const CategoryRuleInfo* const rule,
                            const CategoryRuleItemInfo* const item,
                            const DateTime& applDate)
{
  const RuleItemInfo* res = nullptr;

  const VendorCode& vendor = rule->vendorCode();

  uint16_t cat(static_cast<uint16_t>(item->itemcat()));

  switch (cat)
  {
  case ELIGIBILITY_RULE:
  case FLIGHT_APPLICATION_RULE:
  case ADVANCE_RESERVATION_RULE:
  case STOPOVER_RULE:
  case TRANSFER_RULE:
  case SURCHARGE_RULE:
  case SALE_RESTRICTIONS_RULE:
  case CHILDREN_DISCOUNT_RULE:
  case TOUR_DISCOUNT_RULE:
  case AGENTS_DISCOUNT_RULE:
  case OTHER_DISCOUNT_RULE:
  case NEGOTIATED_RULE:
  case MINIMUM_STAY_RULE:
  case MAXIMUM_STAY_RULE:
  case PENALTIES_RULE:
  case SEASONAL_RULE:
  case MISC_FARE_TAG:
  case TRAVEL_RESTRICTIONS_RULE:
  case DAY_TIME_RULE:
  case BLACKOUTS_RULE:
    res = tsscache::getRuleItem(
        cat, vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical(), _trxId);
    break;

  case ACCOMPANIED_PSG_RULE:
  {
    res =
        getAccompaniedTravelData(vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  case HIP_RULE:
  {
    res =
        getHipMileageExceptData(vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break; // penalties - TODO
  case TICKET_ENDORSMENT_RULE:
  {
    res = getTicketEndorsementsData(
        vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  /*
    case FARE_BY_RULE:
    break; // fare by rule - use getFareByRule instead
  */
  case 26:
  {
    res = getGroupsData(vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  case TOURS_RULE:
  {
    res = getToursData(vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  case 28:
  {
    res = getVisitAnotherCountryData(
        vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  case 29:
  {
    res = getDepositsData(vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  case VOLUNTARY_EXCHANGE_RULE:
  {
    res = getVoluntaryChangesData(
        vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical(), applDate);
  }
  break;
  case VOLUNTARY_REFUNDS_RULE:
  {
    res = getVoluntaryRefundsData(
        vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical(), applDate);
  }
  break;
  case 50:
  {
    res = getRuleApplicationData(vendor, item->itemNo(), _deleteList, ticketDate(), isHistorical());
  }
  break;
  default:
    res = nullptr;
  }

  return res;
}

void
DataHandle::loadFaresForMarket(const LocCode& market1,
                               const LocCode& market2,
                               const std::vector<CarrierCode>& cxr)
{
  loadFaresForMarketData(market1, market2, cxr, ticketDate(), isHistorical());
}

const std::vector<const FareInfo*>&
DataHandle::getFaresByMarketCxr(const LocCode& market1,
                                const LocCode& market2,
                                const CarrierCode& cxr,
                                const DateTime& date)
{
  return getFaresByMarketCxr(market1, market2, cxr, date, date);
}

const std::vector<const FareInfo*>&
DataHandle::getBoundFaresByMarketCxr(const LocCode& market1,
                                     const LocCode& market2,
                                     const CarrierCode& cxr,
                                     const DateTime& date)
{
  return getBoundFaresByMarketCxr(market1, market2, cxr, date, date);
}

/**
 * Get fares for a given city pair and carrier.
 */
const std::vector<const FareInfo*>&
DataHandle::getFaresByMarketCxr(const LocCode& market1,
                                const LocCode& market2,
                                const CarrierCode& cxr,
                                const DateTime& startDate,
                                const DateTime& endDate)
{
  return getFaresByMarketCxrData(market1,
                                 market2,
                                 cxr,
                                 startDate,
                                 endDate,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical(),
                                 _isFareDisplay);
}

const std::vector<const FareInfo*>&
DataHandle::getBoundFaresByMarketCxr(const LocCode& market1,
                                     const LocCode& market2,
                                     const CarrierCode& cxr,
                                     const DateTime& startDate,
                                     const DateTime& endDate)
{
  return getBoundFaresByMarketCxrData(market1,
                                      market2,
                                      cxr,
                                      startDate,
                                      endDate,
                                      _deleteList,
                                      ticketDate(),
                                      isHistorical(),
                                      _isFareDisplay);
}

const std::vector<double>*
DataHandle::getParameterBeta(const int& timeDiff,
                             const int& mileage,
                             const char& direction,
                             const char& APSatInd)
{
  return getParameterBetaData(_deleteList, timeDiff, mileage, direction, APSatInd);
}

/**
 * Get fares for a given city pair, carrier and vendor.
 * For use by Add-on construction process only --
 * doesn't filter by date and doesn't inhibit display only fares.
 */
const std::vector<const FareInfo*>&
DataHandle::getFaresByMarketCxr(const LocCode& market1,
                                const LocCode& market2,
                                const CarrierCode& cxr,
                                const VendorCode& vendor,
                                const DateTime& ticketDate)
{
  return getFaresByMarketCxrData(
      market1, market2, cxr, vendor, _deleteList, ticketDate, isHistEnabled(ticketDate));
}

const std::vector<const FareInfo*>&
DataHandle::getFaresByMarketCxr(const LocCode& market1,
                                const LocCode& market2,
                                const CarrierCode& cxr,
                                const VendorCode& vendor)
{
  return getFaresByMarketCxr(market1, market2, cxr, vendor, ticketDate());
}

bool
DataHandle::isRuleInFareMarket(const LocCode& market1,
                               const LocCode& market2,
                               const CarrierCode& cxr,
                               const RuleNumber ruleNumber)
{
  return isRuleInFareMarketData(market1, market2, cxr, ruleNumber, ticketDate(), isHistorical());
}

const std::vector<CarrierCode>&
DataHandle::getCarriersForMarket(const LocCode& market1,
                                 const LocCode& market2,
                                 bool includeAddon,
                                 const DateTime& date)
{
  return getCarriersForMarketData(
      market1, market2, includeAddon, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<const FareClassAppInfo*>&
DataHandle::getFareClassApp(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& ruleTariff,
                            const RuleNumber& ruleNumber,
                            const FareClassCode& fareClass)
{
  //getRBDByCabin("ATP", "MD");
  return tsscache::getFareClassApp(vendor,
                                   carrier,
                                   ruleTariff,
                                   ruleNumber,
                                   fareClass,
                                   _deleteList,
                                   ticketDate(),
                                   isHistorical(),
                                   isFareDisplay(),
                                   _trxId);
}

const std::vector<const FareClassAppInfo*>&
DataHandle::getFareClassAppByTravelDT(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& ruleTariff,
                            const RuleNumber& ruleNumber,
                            const FareClassCode& fareClass,
                            DateTime travelDate)
{
  //getCommissionContract("COS", "AA", "DP83");
  //getCommissionProgram("COS", 2);
  //getCommissionRule("COS", 2);
  return tsscache::getFareClassAppDataByTravelDate(vendor,
                                                   carrier,
                                                   ruleTariff,
                                                   ruleNumber,
                                                   fareClass,
                                                   travelDate,
                                                   _deleteList,
                                                   ticketDate(),
                                                   isHistorical(),
                                                   isFareDisplay(),
                                                   _trxId);
  /*
  return getFareClassAppDataByTravelDT(
                                      vendor,
                                      carrier,
                                      ruleTariff,
                                      ruleNumber,
                                      fareClass,
                                      travelDate,
                                      _deleteList,
                                      ticketDate(),
                                      isHistorical(),
                                      isFareDisplay());
  */
}

const std::vector<TariffCrossRefInfo*>&
DataHandle::getTariffXRef(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const RecordScope& crossRefType)
{
  return tsscache::getTariffXRef(tsscache::TariffCrossRefCall,
                                 vendor,
                                 carrier,
                                 crossRefType,
                                 -1,
                                 DateTime::emptyDate(),
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical(),
                                 _trxId);
}

const std::vector<TariffCrossRefInfo*>&
DataHandle::getTariffXRefByFareTariff(const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const RecordScope& crossRefType,
                                      const TariffNumber& fareTariff,
                                      const DateTime& date)
{
  return tsscache::getTariffXRef(tsscache::TariffXRefByFareTariffCall,
                                 vendor,
                                 carrier,
                                 crossRefType,
                                 fareTariff,
                                 date,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical(),
                                 _trxId);
}

const std::vector<TariffCrossRefInfo*>&
DataHandle::getTariffXRefByRuleTariff(const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const RecordScope& crossRefType,
                                      const TariffNumber& ruleTariff,
                                      const DateTime& date)
{
  return tsscache::getTariffXRef(tsscache::TariffXRefByRuleTariffCall,
                                 vendor,
                                 carrier,
                                 crossRefType,
                                 ruleTariff,
                                 date,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical(),
                                 _trxId);
}

const std::vector<TariffCrossRefInfo*>&
DataHandle::getTariffXRefByGenRuleTariff(const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const RecordScope& crossRefType,
                                         const TariffNumber& ruleTariff,
                                         const DateTime& date)
{
  return tsscache::getTariffXRef(tsscache::TariffXRefByGenRuleTariffCall,
                                 vendor,
                                 carrier,
                                 crossRefType,
                                 ruleTariff,
                                 date,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical(),
                                 _trxId);
}

const std::vector<TariffCrossRefInfo*>&
DataHandle::getTariffXRefByAddonTariff(const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const RecordScope& crossRefType,
                                       const TariffNumber& addonTariff,
                                       const DateTime& date)
{
  return tsscache::getTariffXRef(tsscache::TariffXRefByAddonTariffCall,
                                 vendor,
                                 carrier,
                                 crossRefType,
                                 addonTariff,
                                 date,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical(),
                                 _trxId);
}

const Indicator
DataHandle::getTariffInhibit(const VendorCode& vendor,
                             const Indicator tariffCrossRefType,
                             const CarrierCode& carrier,
                             const TariffNumber& fareTariff,
                             const TariffCode& ruleTariffCode)
{
  return tsscache::getTariffInhibit(vendor,
                                    tariffCrossRefType,
                                    carrier,
                                    fareTariff,
                                    ruleTariffCode,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical(),
                                    _trxId);
}

NUCInfo*
DataHandle::getNUCFirst(const CurrencyCode& currency,
                        const CarrierCode& carrier,
                        const DateTime& date)
{
  if (!isHistorical() && date < _today)
    setTicketDate(date);

  return tsscache::getNUCFirst(
      currency, carrier, date, _isPortExchange, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const Loc*
DataHandle::getLoc(const LocCode& locCode, const DateTime& date)
{
  int trxId(_trxId);
  DeleteList* deleteList(&_deleteList);
  if (-1 == _trxId)
  {
    DataHandle* dataHandle(TseCallableTrxTask::getCurrentDataHandle());
    if (dataHandle)
    {
      trxId = dataHandle->_trxId;
      deleteList = &dataHandle->_deleteList;
    }
  }

  return tsscache::getLoc(locCode, date, *deleteList, ticketDate(), isHistorical(), trxId);
}

const TaxNation*
DataHandle::getTaxNation(const NationCode& key, const DateTime& date)
{
  return getTaxNationData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<const TaxReportingRecordInfo*>
DataHandle::getTaxReportingRecord(const VendorCode& vendor,
                                  const NationCode& nation,
                                  const CarrierCode& taxCarrier,
                                  const TaxCode& taxCode,
                                  const TaxType& taxType,
                                  const DateTime& date)
{
  return getTaxReportingRecordData(vendor,
                                   nation,
                                   taxCarrier,
                                   taxCode,
                                   taxType,
                                   date,
                                   _deleteList,
                                   ticketDate(),
                                   isHistorical());
}

const std::vector<const TaxReportingRecordInfo*>
DataHandle::getAllTaxReportingRecords(const TaxCode& taxCode)
{
  return getAllTaxReportingRecordData(taxCode,
                                      _deleteList);
}

const std::vector<const TaxRulesRecord*>&
DataHandle::getTaxRulesRecord(const NationCode& nation,
                              const Indicator& taxPointTag,
                              const DateTime& date)
{
  return getTaxRulesRecordData(
      nation, taxPointTag, date, _deleteList, ticketDate(), isHistorical());
}


const std::vector<const TaxRulesRecord*>&
DataHandle::getTaxRulesRecordByCode(const TaxCode& taxCode,
                                    const DateTime& date)
{
  return getTaxRulesRecordByCodeData(
      taxCode, date, _deleteList, ticketDate(), isHistorical());
}

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
const std::vector<const TaxRulesRecord*>&
DataHandle::getTaxRulesRecordByCodeAndType(const TaxCode& taxCode,
                                           const TaxType& taxType,
                                           const DateTime& date)
{
  return getTaxRulesRecordByCodeAndTypeData(
      taxCode, taxType, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<TaxCodeReg*>&
DataHandle::getTaxCode(const TaxCode& taxCode,
                       const DateTime& date)
{
  return tsscache::getTaxCode(taxCode,
                              date,
                              _deleteList,
                              ticketDate(),
                              isHistorical(),
                              _trxId);
}


const std::vector<TaxReissue*>&
DataHandle::getTaxReissue(const TaxCode& taxCode, const DateTime& date)
{
  return getTaxReissueData(taxCode, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<TaxExemption*>&
DataHandle::getTaxExemption(const TaxCode& taxCode,
    const PseudoCityCode& channelId,
    const DateTime& date)
{
  return getTaxExemptionData(taxCode, channelId, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<ATPResNationZones*>&
DataHandle::getATPResNationZones(const NationCode& key)
{
  int trxId(_trxId);
  DeleteList* deleteList(&_deleteList);
  if (LIKELY(-1 == _trxId))
  {
    DataHandle* dataHandle(TseCallableTrxTask::getCurrentDataHandle());
    if (LIKELY(dataHandle))
    {
      trxId = dataHandle->_trxId;
      deleteList = &dataHandle->_deleteList;
    }
  }
  return tsscache::getATPResNationZones(key, *deleteList, trxId);
}

const std::vector<BankerSellRate*>&
DataHandle::getBankerSellRate(const CurrencyCode& primeCur,
                              const CurrencyCode& cur,
                              const DateTime& date)
{
  if (UNLIKELY(!isHistorical() && date < _today))
    setTicketDate(date);

  return getBankerSellRateData(primeCur, cur, date, _deleteList, ticketDate(), isHistorical());
}

FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
DataHandle::getBankerSellRateRange(const CurrencyCode& primeCur,
                                   const CurrencyCode& cur,
                                   const DateTime& date)
{
  if (UNLIKELY(!isHistorical() && date < _today))
    setTicketDate(date);

  return getBankerSellRateDataRange(primeCur, cur, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<Nation*>&
DataHandle::getAllNation(const DateTime& date)
{
  if (UNLIKELY(!isHistorical() && date < _today))
    setTicketDate(date);
  return tsscache::getAllNations(ticketDate(), _deleteList, isHistorical(), _trxId);
}

const Nation*
DataHandle::getNation(const NationCode& nationCode,
                      const DateTime& date)
{
  if (!isHistorical() && date < _today)
    setTicketDate(date);
  int trxId(_trxId);
  DeleteList* deleteList(&_deleteList);
  if (-1 == _trxId)
  {
    DataHandle* dataHandle(TseCallableTrxTask::getCurrentDataHandle());
    if (dataHandle)
    {
      trxId = dataHandle->_trxId;
      deleteList = &dataHandle->_deleteList;
    }
  }
  return tsscache::getNation(nationCode, date, *deleteList, ticketDate(), isHistorical(), trxId);
}

const Currency*
DataHandle::getCurrency(const CurrencyCode& currencyCode)
{
    // Always the most current
    DateTime localTime = DateTime::localTime();
    return tsscache::getCurrency( currencyCode, localTime, _deleteList, localTime, false, _trxId );
}

const std::vector<CurrencySelection*>&
DataHandle::getCurrencySelection(const NationCode& nation, const DateTime& date)
{
  return getCurrencySelectionData(nation, date, _deleteList, ticketDate(), isHistorical());
}

const FareTypeMatrix*
DataHandle::getFareTypeMatrix(const FareType& key, const DateTime& date)
{
  return tsscache::getFareTypeMatrix(key, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<FareTypeMatrix*>&
DataHandle::getAllFareTypeMatrix(const DateTime& date)
{
  return getAllFareTypeMatrixData(date, _deleteList);
}

const std::vector<CombinabilityRuleInfo*>&
DataHandle::getCombinabilityRule(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& ruleTariff,
                                 const RuleNumber& rule,
                                 const DateTime& date)
{
  return tsscache::getCombinabilityRule(
      vendor, carrier, ruleTariff, rule, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<CountrySettlementPlanInfo*>&
DataHandle::getCountrySettlementPlans(const NationCode& countryCode)
{
  return getCountrySettlementPlanData(countryCode, _deleteList, ticketDate(), isHistorical());
}

const std::vector<GeneralFareRuleInfo*>&
DataHandle::getAllGeneralFareRule(const VendorCode vendor,
                                  const CarrierCode carrier,
                                  const TariffNumber ruleTariff,
                                  const RuleNumber rule,
                                  const CatNumber category)
{
  return getGeneralFareRuleData(vendor, carrier, ruleTariff, rule, category);
}

const std::vector<GeneralFareRuleInfo*>&
DataHandle::getGeneralFareRule(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& ruleTariff,
                               const RuleNumber& rule,
                               const CatNumber& category,
                               const DateTime& date,
                               const DateTime& applDate)
{
  return tsscache::getGeneralFareRule(vendor,
                                      carrier,
                                      ruleTariff,
                                      rule,
                                      category,
                                      date,
                                      applDate,
                                      ticketDate(),
                                      isHistorical(),
                                      isFareDisplay(),
                                      _deleteList,
                                      _trxId);
}

const GlobalDir*
DataHandle::getGlobalDir(const GlobalDirection& key, const DateTime& date)
{
  if (!isHistorical() && date < _today)
    setTicketDate(date);

  return tsscache::getGlobalDir(key, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<GlobalDirSeg*>&
DataHandle::getGlobalDirSeg(const DateTime& date)
{
  if (!isHistorical() && date < _today)
    setTicketDate(date);

  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getGlobalDirSeg(date, deleteList, ticketDate(), isHistorical(), trxId);
}

const std::vector<MinFareRuleLevelExcl*>&
DataHandle::getMinFareRuleLevelExcl(const VendorCode& vendor,
                                    int textTblItemNo,
                                    const CarrierCode& governingCarrier,
                                    const TariffNumber& ruleTariff,
                                    const DateTime& date)
{
  return getMinFareRuleLevelExclData(vendor,
                                     textTblItemNo,
                                     governingCarrier,
                                     ruleTariff,
                                     date,
                                     _deleteList,
                                     ticketDate(),
                                     isHistorical());
}

const std::vector<MinFareAppl*>&
DataHandle::getMinFareAppl(const VendorCode& textTblVendor,
                           int textTblItemNo,
                           const CarrierCode& governingCarrier,
                           const DateTime& date)
{
  return getMinFareApplData(textTblVendor,
                            textTblItemNo,
                            governingCarrier,
                            date,
                            _deleteList,
                            ticketDate(),
                            isHistorical());
}

const std::vector<MinFareDefaultLogic*>&
DataHandle::getMinFareDefaultLogic(const VendorCode& vendor, const CarrierCode& carrier)
{
  return getMinFareDefaultLogicData(vendor, carrier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<CopMinimum*>&
DataHandle::getCopMinimum(const NationCode& key, const DateTime& date)
{
  return getCopMinimumData(key, date, _deleteList, ticketDate(), isHistorical());
}

const CopParticipatingNation*
DataHandle::getCopParticipatingNation(const NationCode& nation,
                                      const NationCode& copNation,
                                      const DateTime& date)
{
  return getCopParticipatingNationData(
      nation, copNation, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<LimitationFare*>&
DataHandle::getFCLimitation(const DateTime& date)
{
  return tsscache::getFCLimitation(date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<LimitationJrny*>&
DataHandle::getJLimitation(const DateTime& date)
{
  return getJLimitationData(date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<Mileage*>&
DataHandle::getMileage(const LocCode& origin,
                       const LocCode& destination,
                       const DateTime& dateTime,
                       Indicator mileageType)
{
  if (!isHistorical() && (dateTime < _today))
  {
    setTicketDate(dateTime);
  }
  return getMileageData(
      origin, destination, mileageType, _deleteList, ticketDate(), isHistorical());
}

const Mileage*
DataHandle::getMileage(const LocCode& origin,
                       const LocCode& destination,
                       Indicator mileageType,
                       const GlobalDirection globalDirection,
                       const DateTime& dateTime)
{
  if (!isHistorical() && (dateTime < _today))
  {
    setTicketDate(dateTime);
  }
  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getMileage(origin,
                              destination,
                              mileageType,
                              globalDirection,
                              deleteList,
                              ticketDate(),
                              isHistorical(),
                              trxId);
}

const OpenJawRule*
DataHandle::getOpenJawRule(const VendorCode& vendor,
                           const int itemNo)
{
  return tsscache::getOpenJawRule(vendor,
                                  itemNo,
                                  _deleteList,
                                  ticketDate(),
                                  isHistorical(),
                                  _trxId);
}

const BookingCodeExceptionSequenceList&
DataHandle::getBookingCodeExceptionSequence(const VendorCode& vendor,
                                            const int itemNo,
                                            bool filterExpireDate)
{
  return tsscache::getBookingCodeExceptionSequence(
      vendor, itemNo, filterExpireDate, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const BookingCodeExceptionSequenceList&
DataHandle::getBookingCodeExceptionSequence(const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& ruleTariff,
                                            const RuleNumber& rule,
                                            Indicator conventionNo,
                                            const DateTime& date,
                                            bool& isRuleZero,
                                            bool filterExpireDate)
{
  return tsscache::getBookingCodeExceptionSequence(vendor,
                                                   carrier,
                                                   ruleTariff,
                                                   rule,
                                                   conventionNo,
                                                   date,
                                                   isRuleZero,
                                                   filterExpireDate,
                                                   _deleteList,
                                                   ticketDate(),
                                                   isHistorical(),
                                                   _trxId);
}

const std::vector<const PaxTypeCodeInfo*>
DataHandle::getPaxTypeCode(const VendorCode& vendor, int itemNo)
{
  return getPaxTypeCodeData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<const TaxCodeTextInfo*>
DataHandle::getTaxCodeText(const VendorCode& vendor, int itemNo)
{
  return getTaxCodeTextData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const PaxTypeInfo*
DataHandle::getPaxType(const PaxTypeCode& paxType, const VendorCode& vendor)
{
  return tsscache::getPaxType(paxType, vendor, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<PaxTypeInfo*>&
DataHandle::getAllPaxType()
{
  return getAllPaxTypeData(_deleteList, ticketDate(), isHistorical());
}

const std::vector<const PaxTypeMatrix*>&
DataHandle::getPaxTypeMatrix(const PaxTypeCode& key)
{
  return tsscache::getPaxTypeMatrix(key, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const EndOnEnd*
DataHandle::getEndOnEnd(const VendorCode& vendor, int itemNo)
{
  return tsscache::getEndOnEnd(vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<CarrierCombination*>&
DataHandle::getCarrierCombination(const VendorCode& vendor, int itemNo)
{
  return tsscache::getCarrierCombination(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<CarrierMixedClass*>&
DataHandle::getCarrierMixedClass(const CarrierCode& carrier, const DateTime& date)
{
  return getCarrierMixedClassData(carrier, date, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::corpIdExists(const std::string& corpId, const DateTime& tvlDate)
{
  return corpIdExistsData(corpId, tvlDate, ticketDate(), isHistorical());
}

const std::vector<CorpId*>&
DataHandle::getCorpId(const std::string& corpId,
                      const CarrierCode& carrier,
                      const DateTime& tvlDate)
{
  return tsscache::getCorpId(
      corpId, carrier, tvlDate, ticketDate(), isHistorical(), _deleteList, _trxId);
}

const std::vector<tse::FareByRuleApp*>&
DataHandle::getFareByRuleApp(const CarrierCode& carrier,
                             const std::string& corpId,
                             const AccountCode& accountCode,
                             const TktDesignator& tktDesignator,
                             const DateTime& tvlDate,
                             std::vector<PaxTypeCode>& paxTypes)
{
  return tsscache::getFareByRuleApp(carrier,
                                    corpId,
                                    accountCode,
                                    tktDesignator,
                                    tvlDate,
                                    paxTypes,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical(),
                                    isFareDisplay(),
                                    _trxId);
}

const FlightAppRule*
DataHandle::getFlightAppRule(const VendorCode& vendor, int itemNo)
{
  return static_cast<const FlightAppRule*>(tsscache::getRuleItem(
      FLIGHT_APPLICATION_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const std::vector<DateOverrideRuleItem*>&
DataHandle::getDateOverrideRuleItem(const VendorCode& vendor, int itemNo, const DateTime& applDate)
{
  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getDateOverrideRuleItem(
      vendor, itemNo, applDate, deleteList, ticketDate(), isHistorical(), trxId);
}

const std::vector<GeoRuleItem*>&
DataHandle::getGeoRuleItem(const VendorCode& vendor, int itemNo)
{
  return tsscache::getGeoRuleItem(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const Cabin*
DataHandle::getCabin(const CarrierCode& carrier,
                     const BookingCode& classOfService,
                     const DateTime& date)
{
  return tsscache::getCabin(
      carrier, classOfService, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

bool
DataHandle::isNationInArea(const NationCode& nation, const LocCode& area)
{
  return isNationInAreaData(nation, area, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isNationInSubArea(const NationCode& nation, const LocCode& subArea)
{
  return isNationInSubAreaData(nation, subArea, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isNationInZone(const VendorCode& vendor,
                           int zone,
                           char zoneType,
                           const NationCode& nation)
{
  return isNationInZoneData(
      vendor, zone, zoneType, nation, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isStateInArea(const std::string& nationState, const LocCode& area)
{
  NationCode nation(nationState.substr(0, 2));
  StateCode state(nationState.substr(2, 2));
  return isStateInAreaData(nation, state, area, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isStateInSubArea(const std::string& nationState, const LocCode& subArea)
{
  NationCode nation(nationState.substr(0, 2));
  StateCode state(nationState.substr(2, 2));
  return isStateInSubAreaData(nation, state, subArea, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isStateInZone(const VendorCode& vendor,
                          int zone,
                          char zoneType,
                          const std::string& nationState)
{
  NationCode nation(nationState.substr(0, 2));
  StateCode state(nationState.substr(2, 2));
  return isStateInZoneData(
      vendor, zone, zoneType, nation, state, _deleteList, ticketDate(), isHistorical());
}

// Get VendorType from VendorCrossRef table
char
DataHandle::getVendorType(const VendorCode& vendor)
{
  return tsscache::getVendorType(vendor, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<Routing*>&
DataHandle::getRouting(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& routingTariff,
                       const RoutingNumber& routingNumber,
                       const DateTime& date)
{
  return tsscache::getRouting(vendor,
                              carrier,
                              routingTariff,
                              routingNumber,
                              date,
                              _deleteList,
                              ticketDate(),
                              isHistorical(),
                              _trxId);
}

const MarketRoutingInfo&
DataHandle::getMarketRoutingInfo(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const RoutingNumber& routing,
                                 const TariffNumber& routingTariff,
                                 const LocCode& market1,
                                 const LocCode& market2,
                                 const bool getSingles,
                                 const bool getDoubles)
{
  return getMarketRoutingInfoData(vendor,
                                  carrier,
                                  routing,
                                  routingTariff,
                                  market1,
                                  market2,
                                  getSingles,
                                  getDoubles,
                                  _deleteList);
}

const RoundTripRuleItem*
DataHandle::getRoundTripRuleItem(const VendorCode& vendor, int itemNo)
{
  return getRoundTripRuleItemData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const CircleTripRuleItem*
DataHandle::getCircleTripRuleItem(const VendorCode& vendor, int itemNo)
{
  return tsscache::getCircleTripRuleItem(vendor,
                                         itemNo,
                                         _deleteList,
                                         ticketDate(),
                                         isHistorical(),
                                         _trxId);
}

const std::vector<FareClassRestRule*>&
DataHandle::getFareClassRestRule(const VendorCode& vendor,
                                 int itemNo)
{
  return tsscache::getFareClassRestRule(vendor,
                                        itemNo,
                                        _deleteList,
                                        ticketDate(),
                                        isHistorical(),
                                        _trxId);
}

const std::vector<OpenJawRestriction*>&
DataHandle::getOpenJawRestriction(const VendorCode& vendor,
                                  int itemNo)
{
  return tsscache::getOpenJawRestriction(vendor,
                                         itemNo,
                                         _deleteList,
                                         ticketDate(),
                                         isHistorical(),
                                         _trxId);
}

const std::vector<TariffRuleRest*>&
DataHandle::getTariffRuleRest(const VendorCode& vendor, int itemNo)
{
  return tsscache::getTariffRuleRest(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const CarrierPreference*
DataHandle::getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
{
  return tsscache::getCarrierPreference(
      carrier, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<const JointCarrier*>&
DataHandle::getJointCarrier(const VendorCode& vendor, int itemNo, const DateTime& date)
{
  return getJointCarrierData(vendor, itemNo, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<const SamePoint*>&
DataHandle::getSamePoint(const VendorCode& vendor, int itemNo, const DateTime& date)
{
  return tsscache::getSamePoint(
      vendor, itemNo, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<const BaseFareRule*>&
DataHandle::getBaseFareRule(const VendorCode& vendor, int itemNo, const DateTime& date)
{
  return tsscache::getBaseFareRule(
      vendor, itemNo, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const EligibilityInfo*
DataHandle::getEligibility(const VendorCode& vendor, int itemNo)
{
  return static_cast<const EligibilityInfo*>(tsscache::getRuleItem(
      ELIGIBILITY_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const SeasonalAppl*
DataHandle::getSeasonalAppl(const VendorCode& vendor, int itemNo)
{
  return static_cast<const SeasonalAppl*>(tsscache::getRuleItem(
      SEASONAL_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const std::vector<MultiAirportCity*>&
DataHandle::getMultiAirportCity(const LocCode& city)
{
  return tsscache::getMultiAirportCity(city, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<MultiAirportCity*>&
DataHandle::getMultiCityAirport(const LocCode& locCode)
{
  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getMultiCityAirport(locCode, deleteList, ticketDate(), isHistorical(), trxId);
}

const std::vector<MultiTransport*>&
DataHandle::getMultiTransportCity(const LocCode& locCode,
                                  const CarrierCode& carrierCode,
                                  GeoTravelType tvlType,
                                  const DateTime& tvlDate)
{
  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getMultiTransportCity(locCode,
                                         carrierCode,
                                         tvlType,
                                         tvlDate,
                                         deleteList,
                                         ticketDate(),
                                         isHistorical(),
                                         trxId);
}

const LocCode
DataHandle::getMultiTransportCityCode(const LocCode& locCode,
                                      const CarrierCode& carrierCode,
                                      GeoTravelType tvlType,
                                      const DateTime& tvlDate)
{
  return tsscache::getMultiTransportCityCode(locCode,
                                             carrierCode,
                                             tvlType,
                                             tvlDate,
                                             ticketDate(),
                                             isHistorical(),
                                             _trxId);
}

const std::vector<MultiTransport*>&
DataHandle::getMultiTransportLocs(const LocCode& city,
                                  const CarrierCode& carrierCode,
                                  GeoTravelType tvlType,
                                  const DateTime& tvlDate)
{
  return getMultiTransportLocsData(
      city, carrierCode, tvlType, tvlDate, _deleteList, ticketDate(), isHistorical());
}

const LocCode
DataHandle::getMultiTransportCity(const LocCode& locCode)
{
  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getMultiTransportCity(locCode,
                                         deleteList,
                                         ticketDate(),
                                         isHistorical(),
                                         trxId);
}

GeneralRuleApp*
DataHandle::getGeneralRuleApp(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& tariffNumber,
                              const RuleNumber& ruleNumber,
                              CatNumber catNum)
{
  return tsscache::getGeneralRuleApp(vendor,
                                     carrier,
                                     tariffNumber,
                                     ruleNumber,
                                     catNum,
                                     ticketDate(),
                                     isHistorical(),
                                     _deleteList,
                                     _trxId);
}

bool
DataHandle::getGeneralRuleAppTariffRule(const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        const TariffNumber& tariffNumber,
                                        const RuleNumber& ruleNumber,
                                        CatNumber catNum,
                                        RuleNumber& ruleNumOut,
                                        TariffNumber& tariffNumOut)
{
  return tsscache::getGeneralRuleAppTariffRule(vendor,
                                               carrier,
                                               tariffNumber,
                                               ruleNumber,
                                               catNum,
                                               ruleNumOut,
                                               tariffNumOut,
                                               _deleteList,
                                               ticketDate(),
                                               isHistorical(),
                                               _trxId);
}

const std::vector<GeneralRuleApp*>&
DataHandle::getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& tariffNumber,
                              const RuleNumber& ruleNumber,
                              DateTime tvlDate)
{
  return getGeneralRuleAppByTvlDateData(
      vendor, carrier, tariffNumber, ruleNumber, _deleteList, ticketDate(), isHistorical(), tvlDate);
}

GeneralRuleApp*
DataHandle::getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const TariffNumber& tariffNumber,
                                       const RuleNumber& ruleNumber,
                                       CatNumber catNum,
                                       DateTime tvlDate)
{
  return tsscache::getGeneralRuleAppByTvlDate(vendor,
                                              carrier,
                                              tariffNumber,
                                              ruleNumber,
                                              catNum,
                                              tvlDate,
                                              ticketDate(),
                                              isHistorical(),
                                              _deleteList,
                                              _trxId);
}

bool
DataHandle::getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber& tariffNumber,
                                                 const RuleNumber& ruleNumber,
                                                 CatNumber catNum,
                                                 RuleNumber& ruleNumOut,
                                                 TariffNumber& tariffNumOut,
                                                 DateTime tvlDate)
{
  return tsscache::getGeneralRuleAppTariffRuleByTvlDate(vendor,
                                                        carrier,
                                                        tariffNumber,
                                                        ruleNumber,
                                                        catNum,
                                                        tvlDate,
                                                        ruleNumOut,
                                                        tariffNumOut,
                                                        _deleteList,
                                                        ticketDate(),
                                                        isHistorical(),
                                                        _trxId);
}

const std::vector<FareByRuleCtrlInfo*>&
DataHandle::getAllFareByRuleCtrl(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& tariffNumber,
                                 const RuleNumber& ruleNumber)
{
  return getFareByRuleCtrlData(vendor, carrier, tariffNumber, ruleNumber);
}

const std::vector<FareByRuleCtrlInfo*>&
DataHandle::getFareByRuleCtrl(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& tariffNumber,
                              const RuleNumber& ruleNumber,
                              const DateTime& tvlDate)
{
  return tsscache::getFareByRuleCtrl(vendor,
                                     carrier,
                                     tariffNumber,
                                     ruleNumber,
                                     tvlDate,
                                     _deleteList,
                                     ticketDate(),
                                     isHistorical(),
                                     isFareDisplay(),
                                     _trxId);
}

const FareByRuleItemInfo*
DataHandle::getFareByRuleItem(const VendorCode& vendor, const int itemNo)
{
  return tsscache::getFareByRuleItem(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const FltTrkCntryGrp*
DataHandle::getFltTrkCntryGrp(const CarrierCode& carrier, const DateTime& date)
{
  return getFltTrkCntryGrpData(carrier, date, _deleteList, ticketDate(), isHistorical());
}

const DiscountInfo*
DataHandle::getDiscount(const VendorCode& vendor, int itemNo, int category)
{
  const bool isDiscountCat = category >= CHILDREN_DISCOUNT_RULE &&
                             category <= OTHER_DISCOUNT_RULE;

  if (!isDiscountCat)
    return nullptr;

  return static_cast<const DiscountInfo*>(tsscache::getRuleItem(
      static_cast<uint16_t>(category), vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const std::vector<const IndustryPricingAppl*>&
DataHandle::getIndustryPricingAppl(const CarrierCode& carrier,
                                   const GlobalDirection& globalDir,
                                   const DateTime& date)
{
  return getIndustryPricingApplData(
      carrier, globalDir, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<const IndustryFareAppl*>&
DataHandle::getIndustryFareAppl(Indicator selectionType,
                                const CarrierCode& carrier,
                                const DateTime& date)
{
  return getIndustryFareApplData(
      selectionType, carrier, date, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isMultilateral(const VendorCode& vendor,
                           const RuleNumber& rule,
                           const LocCode& loc1,
                           const LocCode& loc2,
                           const DateTime& date)
{
  return isMultilateralData(vendor, rule, loc1, loc2, date, ticketDate(), isHistorical());
}

const std::vector<const IndustryFareBasisMod*>&
DataHandle::getIndustryFareBasisMod(const CarrierCode& carrier,
                                    Indicator userApplType,
                                    const UserApplCode& userAppl,
                                    const DateTime& date)
{
  return tsscache::getIndustryFareBasisMod(carrier,
                                           userApplType,
                                           userAppl,
                                           date,
                                           _deleteList,
                                           ticketDate(),
                                           isHistorical(),
                                           _trxId);
}

const std::vector<FootNoteCtrlInfo*>&
DataHandle::getAllFootNoteCtrl(const VendorCode vendor,
                               const CarrierCode carrier,
                               const TariffNumber tariffNumber,
                               const Footnote footnote,
                               int category)
{
  return getFootNoteCtrlData(vendor, carrier, tariffNumber, footnote, category);
}

const std::vector<FootNoteCtrlInfo*>&
DataHandle::getFootNoteCtrl(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& fareTariff,
                            const Footnote& footnote,
                            const int category,
                            const DateTime& date)
{
  return tsscache::getFootNoteCtrl(vendor,
                                   carrier,
                                   fareTariff,
                                   footnote,
                                   category,
                                   date,
                                   ticketDate(),
                                   isHistorical(),
                                   isFareDisplay(),
                                   _deleteList,
                                   _trxId);
}

const std::vector<CarrierCode>&
DataHandle::getAddOnCarriersForMarket(const LocCode& market1,
                                      const LocCode& market2,
                                      const DateTime& date)
{
  return getAddOnCarriersForMarketData(
      market1, market2, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AddonZoneInfo*>&
DataHandle::getAddOnZone(const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const LocCode& market,
                         const DateTime& date)
{
  return getAddOnZoneData(vendor, carrier, market, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AddonZoneInfo*>&
DataHandle::getAddOnZone(const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const TariffNumber& fareTariff,
                         const AddonZone& zone,
                         const DateTime& date)
{
  return getAddOnZoneData(
      vendor, carrier, fareTariff, zone, date, _deleteList, ticketDate(), isHistorical());
}

const TSIInfo*
DataHandle::getTSI(int key)
{
  return tsscache::getTSI(key, _deleteList, _trxId);
}

const AddonFareClassCombMultiMap&
DataHandle::getAddOnCombFareClass(const VendorCode& vendor,
                                  const TariffNumber& fareTariff,
                                  const CarrierCode& carrier,
                                  const DateTime& ticketDate)
{
  return getAddOnCombFareClassData(
      vendor, fareTariff, carrier, _deleteList, ticketDate, isHistorical());
}

const AddonFareClassCombMultiMap&
DataHandle::getAddOnCombFareClass(const VendorCode& vendor,
                                  const TariffNumber& fareTariff,
                                  const CarrierCode& carrier)
{
  return getAddOnCombFareClass(vendor, fareTariff, carrier, ticketDate());
}

const std::vector<AddonCombFareClassInfo*>&
DataHandle::getAddOnCombFareClassHistorical(const VendorCode& vendor,
                                            TariffNumber fareTariff,
                                            const CarrierCode& carrier)
{
  return getAddOnCombFareClassHistoricalData(
      vendor, fareTariff, carrier, _deleteList, ticketDate());
}

const std::vector<AddonFareInfo*>&
DataHandle::getAddOnFare(const LocCode& interiorMarket,
                         const CarrierCode& carrier,
                         const DateTime& date)
{
  return getAddOnFareData(
      interiorMarket, carrier, date, _deleteList, ticketDate(), isHistorical(), isFareDisplay());
}

const std::vector<AddonFareInfo*>&
DataHandle::getAddOnFare(const LocCode& gatewayMarket,
                         const LocCode& interiorMarket,
                         const CarrierCode& carrier,
                         const RecordScope& crossRefType,
                         const DateTime& date)
{
  return getAddOnFareData(gatewayMarket,
                          interiorMarket,
                          carrier,
                          crossRefType,
                          date,
                          _deleteList,
                          ticketDate(),
                          isHistorical());
}

const std::vector<Differentials*>&
DataHandle::getDifferentials(const CarrierCode& key, const DateTime& date)
{
  return getDifferentialsData(key, date, _deleteList, ticketDate(), isHistorical());
}

const DayTimeAppInfo*
DataHandle::getDayTimeAppInfo(const VendorCode& vendor, int itemNo)
{
  return static_cast<const DayTimeAppInfo*>(tsscache::getRuleItem(
      DAY_TIME_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const std::vector<FareCalcConfig*>&
DataHandle::getFareCalcConfig(const Indicator& userApplType,
                              const UserApplCode& userAppl,
                              const PseudoCityCode& pseudoCity)
{
  return getFareCalcConfigData(
      userApplType, userAppl, pseudoCity, _deleteList, ticketDate(), isHistorical());
}

const std::vector<NoPNROptions*>&
DataHandle::getNoPNROptions(const Indicator& userApplType, const UserApplCode& userAppl)
{
  return getNoPNROptionsData(userApplType, userAppl, _deleteList, ticketDate(), isHistorical());
}

const NoPNRFareTypeGroup*
DataHandle::getNoPNRFareTypeGroup(const int fareTypeGroup)
{
  return getNoPNRFareTypeGroupData(fareTypeGroup, _deleteList, ticketDate(), isHistorical());
}

const NegFareRest*
DataHandle::getNegFareRest(const VendorCode& vendor, int itemNo)
{
  return static_cast<const NegFareRest*>(tsscache::getRuleItem(
      NEGOTIATED_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const NegFareRestExt*
DataHandle::getNegFareRestExt(const VendorCode& vendor, int itemNo)
{
  return getNegFareRestExtData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<NegFareRestExtSeq*>&
DataHandle::getNegFareRestExtSeq(const VendorCode& vendor, int itemNo)
{
  return getNegFareRestExtSeqData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const MaxStayRestriction*
DataHandle::getMaxStayRestriction(const VendorCode& vendor, int itemNo)
{
  return static_cast<const MaxStayRestriction*>(tsscache::getRuleItem(
      MAXIMUM_STAY_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const MinStayRestriction*
DataHandle::getMinStayRestriction(const VendorCode& vendor, int itemNo)
{
  return static_cast<const MinStayRestriction*>(tsscache::getRuleItem(
      MINIMUM_STAY_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const SalesRestriction*
DataHandle::getSalesRestriction(const VendorCode& vendor, int itemNo)
{
  return tsscache::getSalesRestriction(vendor,
                                       itemNo,
                                       _deleteList,
                                       ticketDate(),
                                       isHistorical(),
                                       _trxId);
}

const TravelRestriction*
DataHandle::getTravelRestriction(const VendorCode& vendor, int itemNo)
{
  return static_cast<const TravelRestriction*>(tsscache::getRuleItem(
      TRAVEL_RESTRICTIONS_RULE, vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId));
}

const Groups*
DataHandle::getGroups(const VendorCode& vendor, int itemNo)
{
  return getGroupsData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const Tours*
DataHandle::getTours(const VendorCode& vendor, int itemNo)
{
  return getToursData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<TpdPsr*>&
DataHandle::getTpdPsr(Indicator applInd,
                      const CarrierCode& carrier,
                      Indicator area1,
                      Indicator area2,
                      const DateTime& date)
{
  return getTpdPsrData(
      applInd, carrier, area1, area2, date, _deleteList, ticketDate(), isHistorical());
}

const SurfaceSectorExempt*
DataHandle::getSurfaceSectorExempt(const LocCode& origLoc,
                                   const LocCode& destLoc,
                                   const DateTime& date)
{
  return getSurfaceSectorExemptData(
      origLoc, destLoc, date, _deleteList, ticketDate(), isHistorical());
}

const MileageSubstitution*
DataHandle::getMileageSubstitution(const LocCode& key, const DateTime& date)
{
  return getMileageSubstitutionData(key, date, _deleteList, ticketDate(), isHistorical());
}

const TariffMileageAddon*
DataHandle::getTariffMileageAddon(const CarrierCode& carrier,
                                  const LocCode& unpublishedAddonLoc,
                                  const GlobalDirection& globalDir,
                                  const DateTime& date)
{
  return getTariffMileageAddonData(
      carrier, unpublishedAddonLoc, globalDir, date, _deleteList, ticketDate(), isHistorical());
}

const PfcPFC*
DataHandle::getPfcPFC(const LocCode& key, const DateTime& date)
{
  return getPfcPFCData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<PfcPFC*>&
DataHandle::getAllPfcPFC()
{
  return getAllPfcPFCData(_deleteList);
}

const PfcMultiAirport*
DataHandle::getPfcMultiAirport(const LocCode& key, const DateTime& date)
{
  return getPfcMultiAirportData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<PfcMultiAirport*>&
DataHandle::getAllPfcMultiAirport()
{
  return getAllPfcMultiAirportData(_deleteList);
}

const std::vector<PfcEssAirSvc*>&
DataHandle::getPfcEssAirSvc(const LocCode& easHubArpt, const LocCode& easArpt, const DateTime& date)
{
  return getPfcEssAirSvcData(easHubArpt, easArpt, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<PfcEssAirSvc*>&
DataHandle::getAllPfcEssAirSvc()
{
  return getAllPfcEssAirSvcData(_deleteList);
}

const std::vector<PfcCollectMeth*>&
DataHandle::getPfcCollectMeth(const CarrierCode& carrier, const DateTime& date)
{
  return getPfcCollectMethData(carrier, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<PfcCollectMeth*>&
DataHandle::getAllPfcCollectMeth(const DateTime& date)
{
  return getAllPfcCollectMethData(_deleteList);
}

const std::vector<PfcAbsorb*>&
DataHandle::getPfcAbsorb(const LocCode& pfcAirport,
                         const CarrierCode& localCarrier,
                         const DateTime& date)
{
  return getPfcAbsorbData(
      pfcAirport, localCarrier, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<PfcAbsorb*>&
DataHandle::getAllPfcAbsorb()
{
  return getAllPfcAbsorbData(_deleteList);
}

const PfcEquipTypeExempt*
DataHandle::getPfcEquipTypeExempt(const EquipmentType& equip,
                                  const StateCode& state,
                                  const DateTime& date)
{
  return getPfcEquipTypeExemptData(equip, state, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<PfcEquipTypeExempt*>&
DataHandle::getAllPfcEquipTypeExemptData()
{
  return tse::getAllPfcEquipTypeExemptData(_deleteList);
}

const std::vector<const PfcTktDesigExcept*>&
DataHandle::getPfcTktDesigExcept(const CarrierCode& carrier, const DateTime& date)
{
  return getPfcTktDesigExceptData(carrier, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SurfaceTransfersInfo*>&
DataHandle::getSurfaceTransfers(const VendorCode& vendor, int itemNo)
{
  return getSurfaceTransfersData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const CircleTripProvision*
DataHandle::getCircleTripProvision(const LocCode& market1,
                                   const LocCode& market2,
                                   const DateTime& date)
{
  return getCircleTripProvisionData(
      market1, market2, date, _deleteList, ticketDate(), isHistorical());
}

const MinFareFareTypeGrp*
DataHandle::getMinFareFareTypeGrp(const std::string& key, const DateTime& date)
{
  return getMinFareFareTypeGrpData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SalesNationRestr*>&
DataHandle::getSalesNationRestr(const NationCode& key, const DateTime& date)
{
  return getSalesNationRestrData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<CarrierApplicationInfo*>&
DataHandle::getCarrierApplication(const VendorCode& vendor, int itemNo, const DateTime& applDate)
{
  return tsscache::getCarrierApplication(
      vendor, itemNo, applDate, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<MileageSurchExcept*>&
DataHandle::getMileageSurchExcept(const VendorCode& vendor,
                                  int textTblItemNo,
                                  const CarrierCode& governingCarrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  const DateTime& date)
{
  return tsscache::getMileageSurchExcept(vendor,
                                         textTblItemNo,
                                         governingCarrier,
                                         ruleTariff,
                                         rule,
                                         date,
                                        _deleteList,
                                        ticketDate(),
                                        isHistorical(),
                                        _trxId);
}

const CarrierFlight*
DataHandle::getCarrierFlight(const VendorCode& vendor, int itemNo)
{
  return tsscache::getCarrierFlight(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<TaxAkHiFactor*>&
DataHandle::getTaxAkHiFactor(const LocCode& key, const DateTime& date)
{
  return getTaxAkHiFactorData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<TaxSegAbsorb*>&
DataHandle::getTaxSegAbsorb(const CarrierCode& carrier, const DateTime& date)
{
  return getTaxSegAbsorbData(carrier, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<NegFareSecurityInfo*>&
DataHandle::getNegFareSecurity(const VendorCode& vendor, int itemNo)
{
  return tsscache::getNegFareSecurity(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<NegFareCalcInfo*>&
DataHandle::getNegFareCalc(const VendorCode& vendor, int itemNo)
{
  return getNegFareCalcData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<const SectorDetailInfo*>
DataHandle::getSectorDetail(const VendorCode& vendor, int itemNo)
{
  return getSectorDetailData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SectorSurcharge*>&
DataHandle::getSectorSurcharge(const CarrierCode& key, const DateTime& date)
{
  return tsscache::getSectorSurcharge(key, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<CustomerActivationControl*>&
DataHandle::getCustomerActivationControl(const std::string& projectCode)
{
  const DateTime localTime = DateTime::localTime();
  return getCustomerActivationControlData(projectCode, localTime, _deleteList, localTime, false);
}

const std::vector<MarkupControl*>&
DataHandle::getMarkupBySecondSellerId(const PseudoCityCode& pcc,
                                      const PseudoCityCode& homePCC,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const TariffNumber& ruleTariff,
                                      const RuleNumber& rule,
                                      int seqNo,
                                      long secondarySellerId,
                                      const DateTime& date)
{
  return tsscache::getMarkupBySecondSellerId(pcc,
                                             homePCC,
                                             vendor,
                                             carrier,
                                             ruleTariff,
                                             rule,
                                             seqNo,
                                             secondarySellerId,
                                             date,
                                             _deleteList,
                                             ticketDate(),
                                             isHistorical(),
                                             _trxId);
}

const std::vector<MarkupControl*>&
DataHandle::getMarkupBySecurityItemNo(const PseudoCityCode& pcc,
                                      const PseudoCityCode& homePCC,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const TariffNumber& ruleTariff,
                                      const RuleNumber& rule,
                                      int seqNo,
                                      const DateTime& date)
{
  return tsscache::getMarkupBySecurityItemNo(pcc,
                                             homePCC,
                                             vendor,
                                             carrier,
                                             ruleTariff,
                                             rule,
                                             seqNo,
                                             date,
                                             _deleteList,
                                             ticketDate(),
                                             isHistorical(),
                                             _trxId);
}

const std::vector<MarkupControl*>&
DataHandle::getMarkupByPcc(const PseudoCityCode& pcc,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const TariffNumber& ruleTariff,
                           const RuleNumber& rule,
                           int seqNo,
                           const DateTime& date)
{
  return tsscache::getMarkupByPcc(pcc,
                                  vendor,
                                  carrier,
                                  ruleTariff,
                                  rule,
                                  seqNo,
                                  date,
                                  _deleteList,
                                  ticketDate(),
                                  isHistorical(),
                                  _trxId);
}

const std::vector<Customer*>&
DataHandle::getCustomer(const PseudoCityCode& key)
{
  return getCustomerData(key, _deleteList, ticketDate(), isHistorical());
}

const std::vector<CommissionCap*>&
DataHandle::getCommissionCap(const CarrierCode& carrier,
                             const CurrencyCode& cur,
                             const DateTime& date)
{
  return getCommissionCapData(carrier, cur, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<MarkupSecFilter*>&
DataHandle::getMarkupSecFilter(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& ruleTariff,
                               const RuleNumber& rule)
{
  return tsscache::getMarkupSecFilter(
      vendor, carrier, ruleTariff, rule, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<DBEGlobalClass*>&
DataHandle::getDBEGlobalClass(const DBEClass& key)
{
  int trxId(_trxId);
  DeleteList* deleteList(&_deleteList);
  if (LIKELY(-1 == _trxId))
  {
    DataHandle* dataHandle(TseCallableTrxTask::getCurrentDataHandle());
    if (LIKELY(dataHandle))
    {
      trxId = dataHandle->_trxId;
      deleteList = &dataHandle->_deleteList;
    }
  }
  return tsscache::getDBEGlobalClass(key, *deleteList, ticketDate(), isHistorical(), trxId);
}

const std::vector<TicketStock*>&
DataHandle::getTicketStock(int key, const DateTime& date)
{
  return getTicketStockData(key, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AddonZoneInfo*>&
DataHandle::getAddonZoneSITA(const LocCode& loc,
                             const VendorCode& vendor,
                             const CarrierCode& carrier)
{
  return getAddonZoneSITAData(loc, vendor, carrier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<RoutingKeyInfo*>&
DataHandle::getRoutingForMarket(const LocCode& market1,
                                const LocCode& market2,
                                const CarrierCode& carrier)
{
  return getRoutingForMarketData(
      market1, market2, carrier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<FareDisplayPref*>&
DataHandle::getFareDisplayPref(const Indicator& userApplType,
                               const UserApplCode& userAppl,
                               const Indicator& pseudoCityType,
                               const PseudoCityCode& pseudoCity,
                               const TJRGroup& tjrGroup)
{
  return getFareDisplayPrefData(
      userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup, _deleteList);
}

const std::vector<FareDisplayPrefSeg*>&
DataHandle::getFareDisplayPrefSeg(const Indicator& userApplType,
                                  const UserApplCode& userAppl,
                                  const Indicator& pseudoCityType,
                                  const PseudoCityCode& pseudoCity,
                                  const TJRGroup& tjrGroup)
{
  return getFareDisplayPrefSegData(
      userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup, _deleteList);
}

const std::vector<FareDisplayInclCd*>&
DataHandle::getFareDisplayInclCd(const Indicator& userApplType,
                                 const UserApplCode& userAppl,
                                 const Indicator& pseudoCityType,
                                 const PseudoCityCode& pseudoCity,
                                 const InclusionCode& inclusionCode)
{
  return getFareDisplayInclCdData(userApplType,
                                  userAppl,
                                  pseudoCityType,
                                  pseudoCity,
                                  inclusionCode,
                                  _deleteList,
                                  ticketDate(),
                                  isHistorical());
}

const std::vector<FareDispInclRuleTrf*>&
DataHandle::getFareDispInclRuleTrf(const Indicator& userApplType,
                                   const UserApplCode& userAppl,
                                   const Indicator& pseudoCityType,
                                   const PseudoCityCode& pseudoCity,
                                   const InclusionCode& inclusionCode)
{
  return getFareDispInclRuleTrfData(userApplType,
                                    userAppl,
                                    pseudoCityType,
                                    pseudoCity,
                                    inclusionCode,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical());
}

const std::vector<FareDispInclFareType*>&
DataHandle::getFareDispInclFareType(const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    const Indicator& pseudoCityType,
                                    const PseudoCityCode& pseudoCity,
                                    const InclusionCode& inclusionCode)
{
  return getFareDispInclFareTypeData(userApplType,
                                     userAppl,
                                     pseudoCityType,
                                     pseudoCity,
                                     inclusionCode,
                                     _deleteList,
                                     ticketDate(),
                                     isHistorical());
}

const std::vector<FareDispInclDsplType*>&
DataHandle::getFareDispInclDsplType(const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    const Indicator& pseudoCityType,
                                    const PseudoCityCode& pseudoCity,
                                    const InclusionCode& inclusionCode)
{
  return getFareDispInclDsplTypeData(userApplType,
                                     userAppl,
                                     pseudoCityType,
                                     pseudoCity,
                                     inclusionCode,
                                     _deleteList,
                                     ticketDate(),
                                     isHistorical());
}

const std::vector<FareDisplayWeb*>&
DataHandle::getFareDisplayWebForCxr(const CarrierCode& carrier)
{
  return getFareDisplayWebForCxrData(carrier, _deleteList);
}

const std::vector<FareDisplayWeb*>&
DataHandle::getFareDisplayWeb(const Indicator& dispInd,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& ruleTariff,
                              const RuleNumber& rule,
                              const PaxTypeCode& paxTypeCode)
{
  return getFareDisplayWebData(
      dispInd, vendor, carrier, ruleTariff, rule, paxTypeCode, _deleteList);
}

const std::set<std::pair<PaxTypeCode, VendorCode> >&
DataHandle::getFareDisplayWebPaxForCxr(const CarrierCode& carrier)
{
  return getFareDisplayWebPaxForCxrData(carrier, _deleteList);
}

const RuleCategoryDescInfo*
DataHandle::getRuleCategoryDesc(const CatNumber& key)
{
  return getRuleCategoryDescData(key, _deleteList);
}

const ZoneInfo*
DataHandle::getZone(const VendorCode& vendor,
                    const Zone& zone,
                    Indicator zoneType,
                    const DateTime& date)
{
  return tsscache::getZone(
      vendor, zone, zoneType, date, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const ZoneInfo*
DataHandle::getZoneFareFocusGroup(const VendorCode& vendor,
                                  const Zone& zone,
                                  Indicator zoneType,
                                  const DateTime& date,
                                  bool fareFocusGroup)
{
  return getZoneFareFocusGroupData(vendor, zone, zoneType, date, _deleteList, ticketDate(), isHistorical(), fareFocusGroup);
}

const std::vector<FareDisplaySort*>&
DataHandle::getFareDisplaySort(const Indicator& userApplType,
                               const UserApplCode& userAppl,
                               const Indicator& pseudoCityType,
                               const PseudoCityCode& pseudoCity,
                               const TJRGroup& tjrGroup,
                               const DateTime& travelDate)
{
  return getFareDisplaySortData(
      userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup, travelDate, _deleteList);
}

const std::vector<FDSPsgType*>&
DataHandle::getFDSPsgType(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const TJRGroup& tjrGroup,
                          const Indicator& fareDisplayType,
                          const Indicator& domIntlAppl,
                          const uint64_t& seqno)
{
  return getFDSPsgTypeData(userApplType,
                           userAppl,
                           pseudoCityType,
                           pseudoCity,
                           tjrGroup,
                           fareDisplayType,
                           domIntlAppl,
                           seqno,
                           _deleteList);
}

const std::vector<FDSGlobalDir*>&
DataHandle::getFDSGlobalDir(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const TJRGroup& tjrGroup,
                            const Indicator& fareDisplayType,
                            const Indicator& domIntlAppl,
                            const DateTime& versionDate,
                            const uint64_t& seqno,
                            const DateTime& createDate)
{
  return getFDSGlobalDirData(userApplType,
                             userAppl,
                             pseudoCityType,
                             pseudoCity,
                             tjrGroup,
                             fareDisplayType,
                             domIntlAppl,
                             versionDate,
                             seqno,
                             createDate,
                             _deleteList);
}

const std::vector<FDSFareBasisComb*>&
DataHandle::getFDSFareBasisComb(const Indicator& userApplType,
                                const UserApplCode& userAppl,
                                const Indicator& pseudoCityType,
                                const PseudoCityCode& pseudoCity,
                                const TJRGroup& tjrGroup,
                                const Indicator& fareDisplayType,
                                const Indicator& domIntlAppl,
                                const uint64_t& seqno)
{
  return getFDSFareBasisCombData(userApplType,
                                 userAppl,
                                 pseudoCityType,
                                 pseudoCity,
                                 tjrGroup,
                                 fareDisplayType,
                                 domIntlAppl,
                                 seqno,
                                 _deleteList);
}

const std::vector<FDSSorting*>&
DataHandle::getFDSSorting(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const TJRGroup& tjrGroup,
                          const Indicator& fareDisplayType,
                          const Indicator& domIntlAppl,
                          const uint64_t& seqno)
{
  return getFDSSortingData(userApplType,
                           userAppl,
                           pseudoCityType,
                           pseudoCity,
                           tjrGroup,
                           fareDisplayType,
                           domIntlAppl,
                           seqno,
                           _deleteList);
}

const std::vector<BrandedFareApp*>&
DataHandle::getBrandedFareApp(const Indicator& userApplType,
                              const UserApplCode& userAppl,
                              const CarrierCode& carrier,
                              const DateTime& travelDate)
{
  return getBrandedFareAppData(
      userApplType, userAppl, carrier, travelDate, _deleteList, ticketDate(), isHistorical());
}

const std::vector<Brand*>&
DataHandle::getBrands(const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const CarrierCode& carrier,
                      const DateTime& travelDate)
{
  return getBrandsData(
      userApplType, userAppl, carrier, travelDate, _deleteList, ticketDate(), isHistorical());
}

const Brand*
DataHandle::getBrand(const Indicator& userApplType,
                     const UserApplCode& userAppl,
                     const CarrierCode& carrier,
                     const BrandCode& brandId,
                     const DateTime& travelDate)
{
  const std::vector<Brand*>& brands = getBrands(userApplType, userAppl, carrier, travelDate);

  std::vector<Brand*>::const_iterator i = brands.begin();
  std::vector<Brand*>::const_iterator e = brands.end();

  for (; i != e; i++)
    if ((*i)->brandId() == brandId)
      return *i;
  return nullptr;
}

std::vector<const FreqFlyerStatusSeg*>
DataHandle::getFreqFlyerStatusSegs(const CarrierCode carrier, const DateTime& date)
{
  return getFreqFlyerStatusSegsData(_deleteList, carrier, date, ticketDate(), isHistorical());
}

std::vector<const FreqFlyerStatus*>
DataHandle::getFreqFlyerStatuses(const CarrierCode carrier,
                                 const DateTime& date,
                                 bool useHistorical)
{
  return getFreqFlyerStatusesData(
      _deleteList, carrier, date, ticketDate(), useHistorical && isHistorical());
}

const std::vector<BrandedCarrier*>&
DataHandle::getBrandedCarriers()
{
  return getBrandedCarriersData(_deleteList);
}

const std::vector<BrandedFare*>&
DataHandle::getBrandedFare(const VendorCode& vendor, const CarrierCode& carrier)
{
  return getBrandedFareData(vendor, carrier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<BrandedCarrier*>&
DataHandle::getS8BrandedCarriers()
{
  return getS8BrandedCarriersData(_deleteList);
}

const std::vector<SvcFeesFareIdInfo*>&
DataHandle::getSvcFeesFareIds(const VendorCode& vendor, long long itemNo)
{
  return getSvcFeesFareIdData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SvcFeesFeatureInfo*>&
DataHandle::getSvcFeesFeature(const VendorCode& vendor, long long itemNo)
{
  return getSvcFeesFeatureData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<RuleCatAlphaCode*>&
DataHandle::getRuleCatAlphaCode(const AlphaCode& key)
{
  return getRuleCatAlphaCodeData(key, _deleteList, ticketDate(), isHistorical());
}

const std::vector<FareDispRec1PsgType*>&
DataHandle::getFareDispRec1PsgType(const Indicator& userApplType,
                                   const UserApplCode& userAppl,
                                   const Indicator& pseudoCityType,
                                   const PseudoCityCode& pseudoCity,
                                   const InclusionCode& inclusionCode)
{
  return getFareDispRec1PsgTypeData(userApplType,
                                    userAppl,
                                    pseudoCityType,
                                    pseudoCity,
                                    inclusionCode,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical());
}

const std::vector<FareDispRec8PsgType*>&
DataHandle::getFareDispRec8PsgType(const Indicator& userApplType,
                                   const UserApplCode& userAppl,
                                   const Indicator& pseudoCityType,
                                   const PseudoCityCode& pseudoCity,
                                   const InclusionCode& inclusionCode)
{
  return getFareDispRec8PsgTypeData(userApplType,
                                    userAppl,
                                    pseudoCityType,
                                    pseudoCity,
                                    inclusionCode,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical());
}

const std::vector<FareDispTemplate*>&
DataHandle::getFareDispTemplate(const int& templateID, const Indicator& templateType)
{
  return getFareDispTemplateData(
      templateID, templateType, _deleteList, ticketDate(), isHistorical());
}

const std::vector<FareDispTemplateSeg*>&
DataHandle::getFareDispTemplateSeg(const int& templateID, const Indicator& templateType)
{
  return getFareDispTemplateSegData(
      templateID, templateType, _deleteList, ticketDate(), isHistorical());
}

const std::vector<FareDispCldInfPsgType*>&
DataHandle::getFareDispCldInfPsgType(const Indicator& userApplType,
                                     const UserApplCode& userAppl,
                                     const Indicator& pseudoCityType,
                                     const PseudoCityCode& pseudoCity,
                                     const InclusionCode& inclusionCode,
                                     const Indicator& psgTypeInd)
{
  return getFareDispCldInfPsgTypeData(userApplType,
                                      userAppl,
                                      pseudoCityType,
                                      pseudoCity,
                                      inclusionCode,
                                      psgTypeInd,
                                      _deleteList,
                                      ticketDate(),
                                      isHistorical());
}

const FareProperties*
DataHandle::getFareProperties(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& tariff,
                              const RuleNumber& rule)
{
  return getFarePropertiesData(
      vendor, carrier, tariff, rule, _deleteList, ticketDate(), isHistorical());
}

const PrintOption*
DataHandle::getPrintOption(const VendorCode& vendor,
                           const std::string& fareSource,
                           const DateTime& date)
{
  return getPrintOptionData(vendor, fareSource, date, _deleteList, ticketDate(), isHistorical());
}

const ValueCodeAlgorithm*
DataHandle::getValueCodeAlgorithm(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const std::string& name,
                                  const DateTime& date)
{
  return getValueCodeAlgorithmData(
      vendor, carrier, name, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<YQYRFeesNonConcur*>&
DataHandle::getYQYRFeesNonConcur(const CarrierCode& carrier, const DateTime& date)
{
  return getYQYRFeesNonConcurData(carrier, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<YQYRFees*>&
DataHandle::getYQYRFees(const CarrierCode& key)
{
  return getYQYRFeesData(key, _deleteList, ticketDate(), isHistorical());
}

const std::vector<MarriedCabin*>&
DataHandle::getMarriedCabins(const CarrierCode& carrier,
                             const BookingCode& premiumCabin,
                             const DateTime& versionDate)
{
  return tsscache::getMarriedCabins(
      carrier, premiumCabin, versionDate, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<ContractPreference*>&
DataHandle::getContractPreferences(const PseudoCityCode& pseudoCity,
                                   const CarrierCode& carrier,
                                   const RuleNumber& rule,
                                   const DateTime& versionDate)
{
  return getContractPreferencesData(
      pseudoCity, carrier, rule, versionDate, _deleteList, ticketDate(), isHistorical());
}

const FareCalcConfigText&
DataHandle::getMsgText(const Indicator userApplType,
                       const UserApplCode& userAppl,
                       const PseudoCityCode& pseudoCity)
{
  return getMsgTextData(userApplType, userAppl, pseudoCity, _deleteList);
}

const std::vector<FareTypeQualifier*>&
DataHandle::getFareTypeQualifier(const Indicator& userApplType,
                                 const UserApplCode& userAppl,
                                 const FareType& qualifier)
{
  return getFareTypeQualifierData(
      userApplType, userAppl, qualifier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<ReissueSequence*>&
DataHandle::getReissue(const VendorCode& vendor,
                       int itemNo,
                       const DateTime& date,
                       const DateTime& applDate)
{
  return getReissueData(vendor, itemNo, date, _deleteList, isHistorical(), applDate);
}

const std::vector<Waiver*>&
DataHandle::getWaiver(const VendorCode& vendor,
                      int itemNo,
                      const DateTime& date,
                      const DateTime& applDate)
{
  return getWaiverData(vendor, itemNo, _deleteList, date, isHistorical(), applDate);
}

const std::vector<FareTypeTable*>&
DataHandle::getFareTypeTable(const VendorCode& vendor,
                             int itemNo,
                             const DateTime& ticketDate,
                             const DateTime& applDate)
{
  return getFareTypeTableData(vendor, itemNo, _deleteList, ticketDate, isHistorical(), applDate);
}

const SeasonalityDOW*
DataHandle::getSeasonalityDOW(const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate,
                              const DateTime& applDate)
{
  return getSeasonalityDOWData(vendor, itemNo, _deleteList, ticketDate, isHistorical(), applDate);
}

// gets records that were effective in given period of time
// between from_date and to_date, for all carrier codes
const std::vector<NUCInfo*>&
DataHandle::getNUCAllCarriers(const CurrencyCode& currency,
                              const DateTime& from_date,
                              const DateTime& to_date)
{
  return getNUCAllCarriersData(currency, from_date, to_date, _deleteList);
}

const std::vector<TicketingFeesInfo*>&
DataHandle::getTicketingFees(const VendorCode& vendor,
                             const CarrierCode& validatingCarrier,
                             const DateTime& date)
{
  return getTicketingFeesData(
      vendor, validatingCarrier, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SubCodeInfo*>&
DataHandle::getSubCode(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const ServiceTypeCode& serviceTypeCode,
                       const ServiceGroup& serviceGroup,
                       const DateTime& date)
{
  return getSubCodeData(vendor,
                        carrier,
                        serviceTypeCode,
                        serviceGroup,
                        date,
                        _deleteList,
                        ticketDate(),
                        isHistorical());
}

const std::vector<OptionalServicesConcur*>&
DataHandle::getOptionalServicesConcur(const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const ServiceTypeCode& serviceTypeCode,
                                      const DateTime& date)
{
  return getOptionalServicesConcurData(
      vendor, carrier, serviceTypeCode, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<OptionalServicesInfo*>&
DataHandle::getOptionalServicesInfo(const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const ServiceTypeCode& serviceTypeCode,
                                    const ServiceSubTypeCode& serviceSubTypeCode,
                                    Indicator fltTktMerchInd,
                                    const DateTime& date)
{
  return getOptionalServicesData(vendor,
                                 carrier,
                                 serviceTypeCode,
                                 serviceSubTypeCode,
                                 fltTktMerchInd,
                                 date,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical());
}

const std::vector<OptionalServicesInfo*>&
DataHandle::getOptionalServicesMktInfo(const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const LocCode& loc1,
                                       const LocCode& loc2,
                                       const ServiceTypeCode& serviceTypeCode,
                                       const ServiceSubTypeCode& serviceSubTypeCode,
                                       Indicator fltTktMerchInd,
                                       const DateTime& date)
{
  return getOptionalServicesMktData(vendor,
                                    carrier,
                                    loc1,
                                    loc2,
                                    serviceTypeCode,
                                    serviceSubTypeCode,
                                    fltTktMerchInd,
                                    date,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical());
}

const std::vector<const ServiceBaggageInfo*>
DataHandle::getServiceBaggage(const VendorCode& vendor, int itemNo)
{
  return getServiceBaggageData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const ServicesDescription*
DataHandle::getServicesDescription(const ServiceGroupDescription& value)
{
  return getServicesDescriptionData(value, _deleteList, ticketDate(), isHistorical());
}

const ServicesSubGroup*
DataHandle::getServicesSubGroup(const ServiceGroup& serviceGroup,
                                const ServiceGroup& serviceSubGroup)
{
  return getServicesSubGroupData(
      serviceGroup, serviceSubGroup, _deleteList, ticketDate(), isHistorical());
}

const std::vector<MerchActivationInfo*>&
DataHandle::getMerchActivation(uint64_t productId,
                               const CarrierCode& carrier,
                               const PseudoCityCode& pseudoCity,
                               const DateTime& date)
{
  return getMerchActivationData(
      productId, carrier, pseudoCity, date, _deleteList, ticketDate(), isHistorical());
}

const MerchCarrierPreferenceInfo*
DataHandle::getMerchCarrierPreference(const CarrierCode& carrier, const ServiceGroup& groupCode)
{
  return getMerchCarrierPreferenceData(
      carrier, groupCode, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SvcFeesSecurityInfo*>&
DataHandle::getSvcFeesSecurity(const VendorCode& vendor, const int itemNo)
{
  return tsscache::getSvcFeesSecurity(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<SvcFeesAccCodeInfo*>&
DataHandle::getSvcFeesAccountCode(const VendorCode& vendor, const int itemNo)
{
  return getSvcFeesAccountCodeData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SvcFeesCurrencyInfo*>&
DataHandle::getSvcFeesCurrency(const VendorCode& vendor, const int itemNo)
{
  return getSvcFeesCurrencyData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SvcFeesCxrResultingFCLInfo*>&
DataHandle::getSvcFeesCxrResultingFCL(const VendorCode& vendor,
                                      const int itemNo)
{
  return tsscache::getSvcFeesCxrResultingFCL(vendor,
                                             itemNo,
                                             _deleteList,
                                             ticketDate(),
                                             isHistorical(),
                                             _trxId);
}

const std::vector<SvcFeesResBkgDesigInfo*>&
DataHandle::getSvcFeesResBkgDesig(const VendorCode& vendor, const int itemNo)
{
  return getSvcFeesResBkgDesigData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SvcFeesTktDesignatorInfo*>&
DataHandle::getSvcFeesTicketDesignator(const VendorCode& vendor, const int itemNo)
{
  return getSvcFeesTktDesignatorData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::isCxrActivatedForServiceFee(const CarrierCode& validatingCarrier, const DateTime& date)
{
  const std::vector<ServiceFeesCxrActivation*>& activatedCxrs = getServiceFeesCxrActivationData(
      validatingCarrier, date, _deleteList, ticketDate(), isHistorical());
  if (activatedCxrs.empty())
    return false;
  return true;
}

const std::vector<ServiceGroupInfo*>&
DataHandle::getAllServiceGroups()
{
  return getAllServiceGroupData(_deleteList, isHistorical());
}

const std::vector<OptionalServicesActivationInfo*>&
DataHandle::getOptServiceActivation(Indicator crs,
                                    const UserApplCode& userCode,
                                    const std::string& application)
{
  return getOptServiceActivationData(
      crs, userCode, application, _deleteList, ticketDate(), isHistorical());
}

const TaxCarrierFlightInfo*
DataHandle::getTaxCarrierFlight(const VendorCode& vendor, int itemNo)
{
  return getTaxCarrierFlightData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const TaxText*
DataHandle::getTaxText(const VendorCode& vendor, int itemNo)
{
  return getTaxTextData(vendor, itemNo, _deleteList, ticketDate(), isHistorical());
}

const TaxCarrierAppl*
DataHandle::getTaxCarrierAppl(const VendorCode& vendor, int itemNo)
{
  return tsscache::getTaxCarrierAppl(
      vendor, itemNo, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<CurrencyCode>&
DataHandle::getAllCurrencies()
{
  return getAllCurrenciesData(_deleteList, isHistorical());
}

const TaxRestrictionLocationInfo*
DataHandle::getTaxRestrictionLocation(const TaxRestrictionLocation& location)
{
  return getTaxRestrictionLocationData(location, _deleteList, ticketDate(), isHistorical());
}

const std::vector<TaxSpecConfigReg*>&
DataHandle::getTaxSpecConfig(const TaxSpecConfigName& name)
{
  bool useParentDH(_parentDataHandle);
  int trxId(useParentDH ? _parentDataHandle->_trxId : _trxId);
  DeleteList& deleteList(useParentDH ? _parentDataHandle->deleteList() : _deleteList);

  return tsscache::getTaxSpecConfig(name, deleteList, ticketDate(), isHistorical(), trxId);
}

bool
DataHandle::isCarrierUSDot(const CarrierCode& carrier)
{
  return getUSDotCarrierData(carrier, ticketDate(), isHistorical());
}

bool
DataHandle::isCarrierCTA(const CarrierCode& carrier)
{
  return getCTACarrierData(carrier, ticketDate(), isHistorical());
}

void DataHandle::destroyTicketDate(void*)
{
  pthread_key_delete(threadSpecificTicketDateKey());
}

void DataHandle::destroyIsHistorical(void*)
{
  pthread_key_delete(threadSpecificIsHistoricalKey());
}

pthread_key_t DataHandle::_threadSpecificTicketDateKey =  DataHandle::initTicketDateKey();
pthread_key_t DataHandle::_threadSpecificIsHistoricalKey =  DataHandle::initIsHistoricalKey();

pthread_key_t DataHandle::initTicketDateKey()
{
  pthread_key_t key;
  pthread_key_create(&key, destroyTicketDate);
  return key;
}

pthread_key_t DataHandle::initIsHistoricalKey()
{
  pthread_key_t key;
  pthread_key_create(&key, destroyIsHistorical);
  return key;
}


const DateTime&
DataHandle::getVoluntaryChangesConfig(const CarrierCode& carrier,
                                      const DateTime& currentTktDate,
                                      const DateTime& originalTktIssueDate)
{
  return getVoluntaryChangesConfigData(carrier, currentTktDate, originalTktIssueDate, _deleteList);
}

const DateTime&
DataHandle::getVoluntaryRefundsConfig(const CarrierCode& carrier,
                                      const DateTime& currentTktDate,
                                      const DateTime& originalTktIssueDate)
{
  return getVoluntaryRefundsConfigData(carrier, currentTktDate, originalTktIssueDate, _deleteList);
}

const std::vector<TPMExclusion*>&
DataHandle::getTPMExclus(const CarrierCode& carrier)
{
  return getTPMExclusionsData(carrier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<InterlineCarrierInfo*>&
DataHandle::getInterlineCarrier()
{
  return getInterlineCarrierData(_deleteList, isHistorical());
}

const std::vector<IntralineCarrierInfo*>&
DataHandle::getIntralineCarrier()
{
  return getIntralineCarrierData(_deleteList, isHistorical());
}

const std::vector<InterlineTicketCarrierInfo*>&
DataHandle::getInterlineTicketCarrier(const CarrierCode& carrier, const DateTime& date)
{
  return getInterlineTicketCarrierData(carrier, date, _deleteList, date, false);
}

const InterlineTicketCarrierStatus*
DataHandle::getInterlineTicketCarrierStatus(const CarrierCode& carrier, const CrsCode& crsCode)
{
  const DateTime localTime = DateTime::localTime();
  const std::vector<InterlineTicketCarrierStatus*>& ietStatusVec =
      getInterlineTicketCarrierStatusData(
          carrier, crsCode, localTime, _deleteList, localTime, false);
  if (ietStatusVec.empty())
    return nullptr;
  else
    return ietStatusVec.front();

  // return getInterlineTicketCarrierStatusData(carrier, crsCode, DateTime::localTime() ,
  // _deleteList, DateTime::localTime() ,false);
}

const std::vector<NvbNvaInfo*>&
DataHandle::getNvbNvaInfo(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& tarrif,
                          const RuleNumber& rule)
{
  return getNvbNvaData(vendor, carrier, tarrif, rule, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SeatCabinCharacteristicInfo*>&
DataHandle::getSeatCabinCharacteristicInfo(const CarrierCode& carrier,
                                           const Indicator& codeType,
                                           const DateTime& travelDate)
{
  return getSeatCabinCharacteristicData(
      carrier, codeType, travelDate, _deleteList, ticketDate(), isHistorical());
}

bool
DataHandle::getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                                   const DSTGrpCode& dstgrp2,
                                   short& utcoffset,
                                   const DateTime& dateTime1,
                                   const DateTime& dateTime2)
{
  return tsscache::getUtcOffsetDifference(
      dstgrp1, dstgrp2, utcoffset, dateTime1, dateTime2, _deleteList, _trxId);
}

const std::vector<Cat05OverrideCarrier*>&
DataHandle::getCat05OverrideCarrierInfo(const PseudoCityCode& pcc)
{
  return getInfiniCat05OverrideData(pcc, _deleteList);
}

const BankIdentificationInfo*
DataHandle::getBankIdentification(const FopBinNumber& binNumber, const DateTime& date)
{
  return getBankIdentificationData(binNumber, date, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AirlineAllianceCarrierInfo*>&
DataHandle::getAirlineAllianceCarrier(const CarrierCode& carrierCode)
{
  return getAirlineAllianceCarrierData(carrierCode, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AirlineAllianceCarrierInfo*>&
DataHandle::getGenericAllianceCarrier(const GenericAllianceCode& genericAllianceCode)
{
  return getGenericAllianceCarrierData(
      genericAllianceCode, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AirlineAllianceContinentInfo*>&
DataHandle::getAirlineAllianceContinent(const GenericAllianceCode& genericAllianceCode,
                                        bool reduceTemporaryVectorsFallback)
{
  return getAirlineAllianceContinentData(genericAllianceCode,
                                         _deleteList,
                                         ticketDate(),
                                         isHistorical(),
                                         reduceTemporaryVectorsFallback);
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
DataHandle::getAirlineCountrySettlementPlans(const NationCode& country,
                                             const CrsCode& gds,
                                             const CarrierCode& airline,
                                             const SettlementPlanType& spType)
{
  return getAirlineCountrySettlementPlanData(
      country, gds, airline, spType, _deleteList, ticketDate(), isHistorical());
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
DataHandle::getAirlineCountrySettlementPlans(const NationCode& country,
                                             const CrsCode& gds,
                                             const SettlementPlanType& spType)
{
  const DateTime& today = DateTime::localTime();
  return getAirlineCountrySettlementPlanData(country, gds, spType, _deleteList, today);
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
DataHandle::getAirlineCountrySettlementPlans(const CrsCode& gds,
                                             const NationCode& country,
                                             const CarrierCode& airline)
{
  const DateTime& today = DateTime::localTime();
  return getAirlineCountrySettlementPlanData(gds, country, airline, _deleteList, today);
}

const std::vector<AirlineInterlineAgreementInfo*>&
DataHandle::getAirlineInterlineAgreements(const NationCode& country,
                                          const CrsCode& gds,
                                          const CarrierCode& validatingCarrier)
{
  return getAirlineInterlineAgreementData(
      country, gds, validatingCarrier, _deleteList, ticketDate(), isHistorical());
}

const std::vector<NeutralValidatingAirlineInfo*>&
DataHandle::getNeutralValidatingAirlines(const NationCode& country,
                                         const CrsCode& gds,
                                         const SettlementPlanType& spType)
{
  return getNeutralValidatingAirlineData(
      country, gds, spType, _deleteList, ticketDate(), isHistorical());
}

const std::vector<TktDesignatorExemptInfo*>&
DataHandle::getTktDesignatorExempt(const CarrierCode& carrier)
{
  return getTktDesignatorExemptData(carrier, _deleteList, ticketDate(), isHistorical());
}

const FareFocusRuleInfo*
DataHandle::getFareFocusRule(uint64_t fareFocusRuleId,
                             DateTime adjustedTicketDate)
{
  return tsscache::getFareFocusRule(fareFocusRuleId,
                                    adjustedTicketDate,
                                    _deleteList,
                                    ticketDate(),
                                    isHistorical(),
                                    _trxId);
}

const FareFocusAccountCdInfo*
DataHandle::getFareFocusAccountCd(uint64_t accountCdItemNo,
                                  DateTime adjustedTicketDate)
{
  return getFareFocusAccountCdData(accountCdItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const FareFocusRoutingInfo*
DataHandle::getFareFocusRouting(uint64_t routingItemNo,
                                  DateTime adjustedTicketDate)
{
  return getFareFocusRoutingData(routingItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const FareFocusLocationPairInfo*
DataHandle::getFareFocusLocationPair(uint64_t locationPairItemNo,
                                  DateTime adjustedTicketDate)
{
  return getFareFocusLocationPairData(locationPairItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const FareFocusDaytimeApplInfo*
DataHandle::getFareFocusDaytimeAppl(uint64_t dayTimeApplItemNo,
                                  DateTime createDate)
{
  return getFareFocusDaytimeApplData(dayTimeApplItemNo, createDate, _deleteList, ticketDate(), isHistorical());
}

const FareFocusDisplayCatTypeInfo*
DataHandle::getFareFocusDisplayCatType(uint64_t displayCatTypeItemNo,
                                  DateTime adjustedTicketDate)
{
  return getFareFocusDisplayCatTypeData(displayCatTypeItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const std::vector<class FareFocusBookingCodeInfo*>&
DataHandle::getFareFocusBookingCode(uint64_t bookingCodeItemNo,
                                    DateTime adjustedTicketDate)
{
  return getFareFocusBookingCodeData(bookingCodeItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const std::vector<FareFocusFareClassInfo*>&
DataHandle::getFareFocusFareClass(uint64_t fareClassItemNo,
                                  DateTime adjustedTicketDate)
{
  return getFareFocusFareClassData(fareClassItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const FareFocusLookupInfo*
DataHandle::getFareFocusLookup(const PseudoCityCode& pcc)
{
  return getFareFocusLookupData(pcc, _deleteList, ticketDate(), isHistorical());
}

const FareFocusSecurityInfo*
DataHandle::getFareFocusSecurity(uint64_t securityItemNo,
                                 DateTime adjustedTicketDate)
{
  return tsscache::getFareFocusSecurity(securityItemNo,
                                        adjustedTicketDate,
                                        _deleteList,
                                        ticketDate(),
                                        isHistorical(),
                                        _trxId);
}


const FareRetailerRuleLookupInfo*
DataHandle::getFareRetailerRuleLookup(Indicator applicationType,
                                      const PseudoCityCode& sourcePcc,
                                      const PseudoCityCode& pcc)
{
  return getFareRetailerRuleLookupData(applicationType, sourcePcc, pcc, _deleteList, ticketDate(), isHistorical());
}

const FareRetailerRuleInfo*
DataHandle::getFareRetailerRule(uint64_t fareRetailerRuleId,
                                DateTime adjustedTicketDate)
{
  return getFareRetailerRuleData(fareRetailerRuleId,
                                 adjustedTicketDate,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical());
}

const FareRetailerResultingFareAttrInfo*
DataHandle::getFareRetailerResultingFareAttr(uint64_t resultingFareAttrItemNo,
                                             DateTime adjustedTicketDate)
{
  return getFareRetailerResultingFareAttrData(resultingFareAttrItemNo,
                                              adjustedTicketDate,
                                              _deleteList,
                                              ticketDate(),
                                              isHistorical());
}

const std::vector<FareRetailerCalcInfo*>&
DataHandle::getFareRetailerCalc(uint64_t fareRetailerCalcItemNo,
                                DateTime adjustedTicketDate)
{
  return getFareRetailerCalcData(fareRetailerCalcItemNo,
                                 adjustedTicketDate,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical());
}

const std::vector<FareFocusCarrierInfo*>&
DataHandle::getFareFocusCarrier(uint64_t carrierItemNo,
                                DateTime adjustedTicketDate)
{
  return getFareFocusCarrierData(carrierItemNo,
                                 adjustedTicketDate,
                                 _deleteList,
                                 ticketDate(),
                                 isHistorical());
}

const std::vector<FareFocusRuleCodeInfo*>&
DataHandle::getFareFocusRuleCode(uint64_t ruleCdItemNo,
                                 DateTime adjustedTicketDate)
{
  return getFareFocusRuleCodeData(ruleCdItemNo,
                                  adjustedTicketDate,
                                  _deleteList,
                                  ticketDate(),
                                  isHistorical());
}

const std::vector<CommissionContractInfo*>& DataHandle::getCommissionContract(const VendorCode& vendor,
                                                                              const CarrierCode& carrier,
                                                                              const PseudoCityCode& pcc)
{
  return getCommissionContractData(vendor, carrier, pcc, _deleteList, ticketDate(), isHistorical());
}

const std::vector<CommissionProgramInfo*>& DataHandle::getCommissionProgram(const VendorCode& vendor,
                                                                            uint64_t contractId)
{
  return getCommissionProgramData(vendor, contractId, _deleteList, ticketDate(), isHistorical());
}

const std::vector<CommissionRuleInfo*>& DataHandle::getCommissionRule(const VendorCode& vendor,
                                                                      uint64_t programId)
{
  return getCommissionRuleData(vendor, programId, _deleteList, ticketDate(), isHistorical());
}

const std::vector<CustomerSecurityHandshakeInfo*>&
DataHandle::getCustomerSecurityHandshake(const PseudoCityCode& pcc,
                                         const Code<8>& productCD,
                                         const DateTime& dateTime)
{
  return getCustomerSecurityHandshakeData(pcc, productCD, _deleteList, ticketDate(), dateTime, isHistorical());
}

const std::vector<class RBDByCabinInfo*>&
DataHandle::getRBDByCabin(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          DateTime tvlDate)
{
  return tsscache::getRBDByCabin(vendor, carrier, tvlDate, _deleteList, ticketDate(), isHistorical(), _trxId);
}

const std::vector<GenSalesAgentInfo*>&
DataHandle::getGenSalesAgents(const CrsCode& gds,
                              const NationCode& country,
                              const SettlementPlanType& settlementPlan,
                              const CarrierCode& validatingCxr)
{
  return getGenSalesAgentData(
      _deleteList, gds, country, settlementPlan, validatingCxr, ticketDate(), isHistorical());
}

const std::vector<GenSalesAgentInfo*>&
DataHandle::getGenSalesAgents(const CrsCode& gds,
                              const NationCode& country,
                              const SettlementPlanType& settlementPlan)
{
  const DateTime today = DateTime::localTime();
  return getGenSalesAgentData(_deleteList, gds, country, settlementPlan, today);
}

const std::vector<EmdInterlineAgreementInfo*>&
DataHandle::getEmdInterlineAgreements(const NationCode& country,
                                      const CrsCode& gds,
                                      const CarrierCode& validatingCarrier)
{
  return getEmdInterlineAgreementData(
      country, gds, validatingCarrier, _deleteList, ticketDate(), isHistorical());
}

const FareFocusPsgTypeInfo*
DataHandle::getFareFocusPsgType(uint64_t psgTypeItemNo,
                                  DateTime adjustedTicketDate)
{
  return getFareFocusPsgTypeData(
      psgTypeItemNo, adjustedTicketDate, _deleteList, ticketDate(), isHistorical());
}

const std::vector<SpanishReferenceFareInfo*>&
DataHandle::getSpanishReferenceFare(const CarrierCode& tktCarrier, const CarrierCode& fareCarrier,
                                    const LocCode& sourceLoc, const LocCode& destLoc,
                                    const DateTime& date)
{
  return getSpanishReferenceFareData(tktCarrier, fareCarrier, sourceLoc, destLoc, _deleteList, date,
                                     ticketDate(), isHistorical());
}

const std::vector<MaxPermittedAmountFareInfo*>&
DataHandle::getMaxPermittedAmountFare(const Loc& origin, const Loc& dest, const DateTime& date)
{
  return getMaxPermittedAmountFareData(
      origin, dest, _deleteList, date, ticketDate(), isHistorical());
}

} // namespace tse
