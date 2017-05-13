#pragma once

#include "Common/TSSCacheCommon.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/DSTDAO.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/TravelRestriction.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <cstdint>

namespace tse
{

namespace tsscache
{

// RuleItem cache
struct RuleItemEntry : public Entry
{
  bool equal(uint16_t cat,
             const VendorCode& vendor,
             int itemNo,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _cat == cat && _itemNo == itemNo && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor;
  }
  int _itemNo = 0;
  const RuleItemInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const RuleItemInfo*
getRuleItem(uint16_t cat,
            const VendorCode& vendor,
            int itemNo,
            DeleteList& deleteList,
            const DateTime& ticketDate,
            bool isHistorical,
            int trxId)
{
  typedef RuleItemEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, cat, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const RuleItemInfo* result(nullptr);
  switch (cat)
  {
  case ELIGIBILITY_RULE:
    result = getEligibilityData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case FLIGHT_APPLICATION_RULE:
    result = getFlightAppRuleData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case ADVANCE_RESERVATION_RULE:
    result = getAdvResTktData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case STOPOVER_RULE:
    result = getStopoversData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case TRANSFER_RULE:
    result = getTransfers1Data(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case SURCHARGE_RULE:
    result = getSurchargesData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case SALE_RESTRICTIONS_RULE:
    result = getSalesRestrictionData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case CHILDREN_DISCOUNT_RULE:
  case TOUR_DISCOUNT_RULE:
  case AGENTS_DISCOUNT_RULE:
  case OTHER_DISCOUNT_RULE:
    result = getDiscountData(vendor, itemNo, cat, deleteList, ticketDate, isHistorical);
    break;
  case NEGOTIATED_RULE:
    result = getNegFareRestData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case MINIMUM_STAY_RULE:
    result = getMinStayRestrictionData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case MAXIMUM_STAY_RULE:
    result = getMaxStayRestrictionData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case PENALTIES_RULE:
    result = getPenaltyData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case SEASONAL_RULE:
    result = getSeasonalApplData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case MISC_FARE_TAG:
    result = getMiscFareTagData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case TRAVEL_RESTRICTIONS_RULE:
    result = getTravelRestrictionData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case DAY_TIME_RULE:
    result = getDayTimeAppInfoData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  case BLACKOUTS_RULE:
    result = getBlackoutData(vendor, itemNo, deleteList, ticketDate, isHistorical);
    break;
  default:
    std::cerr << __FUNCTION__ << " not implemented " << cat << std::endl;
    break;
  }
  if (LIKELY(entry))
  {
    entry->_cat = cat;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_vendor = vendor;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// GFR cache
typedef std::vector<GeneralFareRuleInfo*> GFRVector;

struct GFREntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             CatNumber cat,
             uint64_t dateInt,
             uint64_t applDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             bool isFareDisplay,
             ...) const
  {
    return _ruleTariff == ruleTariff && _cat == cat && _dateInt == dateInt &&
           _applDateInt == applDateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _isFareDisplay == isFareDisplay &&
           _carrier == carrier && _rule == rule && _vendor == vendor;
  }
  TariffNumber _ruleTariff = 0;
  uint64_t _dateInt = 0;
  uint64_t _applDateInt = 0;
  bool _isFareDisplay = false;
  CarrierCode _carrier;
  RuleNumber _rule;
  const GFRVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const GFRVector&
getGeneralFareRule(const VendorCode& vendor,
                   const CarrierCode& carrier,
                   TariffNumber ruleTariff,
                   const RuleNumber& rule,
                   CatNumber cat,
                   const DateTime& date,
                   const DateTime& applDate,
                   const DateTime& ticketDate,
                   bool isHistorical,
                   bool isFareDisplay,
                   DeleteList& deleteList,
                   int trxId)
{
  typedef GFREntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t applDateInt(applDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           cat,
                                           dateInt,
                                           applDateInt,
                                           ticketDateInt,
                                           isHistorical,
                                           isFareDisplay));
  if (hit)
  {
    return *entry->_result;
  }
  const GFRVector* result(nullptr);
  if (UNLIKELY((VOLUNTARY_EXCHANGE_RULE == cat || VOLUNTARY_REFUNDS_RULE == cat) && !applDate.isEmptyDate()))
  {
    const GFRVector* tmpVec(
        &getGeneralFareRuleBackDatingData(
            vendor, carrier, ruleTariff, rule, cat, applDate, deleteList, isFareDisplay));
    if (!tmpVec->empty())
    {
      result = tmpVec;
    }
  }
  if (LIKELY(nullptr == result))
  {
    result = &getGeneralFareRuleData(vendor,
                                     carrier,
                                     ruleTariff,
                                     rule,
                                     cat,
                                     date,
                                     deleteList,
                                     ticketDate,
                                     isHistorical,
                                     isFareDisplay);
  }
  if (LIKELY(entry))
  {
    entry->_cat = static_cast<uint16_t>(cat);
    entry->_ruleTariff = ruleTariff;
    entry->_dateInt = dateInt;
    entry->_applDateInt = applDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_isFareDisplay = isFareDisplay;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_rule = rule;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// FN cache
typedef std::vector<FootNoteCtrlInfo*> FNVector;
struct FNEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber fareTariff,
             const Footnote& footnote,
             int cat,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             bool isFareDisplay,
             ...) const
  {
    return _fareTariff == fareTariff && _cat == cat && _dateInt == dateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _isFareDisplay == isFareDisplay && _vendor == vendor && _carrier == carrier &&
           _footnote == footnote;
  }
  CarrierCode _carrier;
  TariffNumber _fareTariff = 0;
  Footnote _footnote;
  uint64_t _dateInt = 0;
  bool _isFareDisplay = false;
  const FNVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FNVector&
getFootNoteCtrl(const VendorCode& vendor,
                const CarrierCode& carrier,
                TariffNumber fareTariff,
                const Footnote& footnote,
                int cat,
                const DateTime& date,
                const DateTime& ticketDate,
                bool isHistorical,
                bool isFareDisplay,
                DeleteList& deleteList,
                int trxId)
{
  typedef FNEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           fareTariff,
                                           footnote,
                                           cat,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical,
                                           isFareDisplay));
  if (hit)
  {
    return *entry->_result;
  }
  const FNVector* result(&getFootNoteCtrlData(vendor,
                                              carrier,
                                              fareTariff,
                                              footnote,
                                              cat,
                                              date,
                                              deleteList,
                                              ticketDate,
                                              isHistorical,
                                              isFareDisplay));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_fareTariff = fareTariff;
    entry->_footnote = footnote;
    entry->_cat = static_cast<uint16_t>(cat);
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_isFareDisplay = isFareDisplay;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// CorpId cache
typedef std::vector<CorpId*> CorpIdVector;

struct CorpIdEntry : public Entry
{
  bool equal(const std::string& corpId,
             const CarrierCode& carrier,
             uint64_t tvlDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _tvlDateInt == tvlDateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _corpId == corpId && _carrier == carrier;
  }
  Code<10> _corpId;
  CarrierCode _carrier;
  uint64_t _tvlDateInt = 0;
  const CorpIdVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CorpIdVector&
getCorpId(const std::string& corpId,
          const CarrierCode& carrier,
          const DateTime& tvlDate,
          const DateTime& ticketDate,
          bool isHistorical,
          DeleteList& deleteList,
          int trxId)
{
  // assert(corpId.length() <= 10);
  typedef CorpIdEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t tvlDateInt(tvlDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, corpId, carrier, tvlDateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const CorpIdVector* result(
      &getCorpIdData(corpId, carrier, tvlDate, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_tvlDateInt = tvlDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_corpId = corpId;
    entry->_carrier = carrier;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// GeneralRuleApp cache
struct GeneralRuleAppEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber tariffNumber,
             const RuleNumber& rule,
             CatNumber cat,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _cat == cat && _tariffNumber == tariffNumber && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _carrier == carrier &&
           _rule == rule;
  }
  CarrierCode _carrier;
  TariffNumber _tariffNumber = 0;
  RuleNumber _rule;
  GeneralRuleApp* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline GeneralRuleApp*
getGeneralRuleApp(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  TariffNumber tariffNumber,
                  const RuleNumber& rule,
                  CatNumber cat,
                  const DateTime& ticketDate,
                  bool isHistorical,
                  DeleteList& deleteList,
                  int trxId)
{
  typedef GeneralRuleAppEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, carrier, tariffNumber, rule, cat, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  GeneralRuleApp* result(getGeneralRuleAppData(
      vendor, carrier, tariffNumber, rule, cat, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_cat = static_cast<uint16_t>(cat);
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_tariffNumber = tariffNumber;
    entry->_rule = rule;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// GeneralRuleAppTariffRule cache
struct GeneralRuleAppTariffRuleEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber tariffNumber,
             const RuleNumber& rule,
             CatNumber cat,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _tariffNumber == tariffNumber && _cat == cat && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _carrier == carrier &&
           _rule == rule;
  }
  CarrierCode _carrier;
  TariffNumber _tariffNumber = 0;
  RuleNumber _rule;
  bool _result = false;
  RuleNumber _ruleOut;
  TariffNumber _tariffNumOut = 0;
  static TSSCacheEnum _cacheIndex;
};

inline bool
getGeneralRuleAppTariffRule(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            TariffNumber tariffNumber,
                            const RuleNumber& rule,
                            CatNumber cat,
                            RuleNumber& ruleOut,
                            TariffNumber& tariffNumOut,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical,
                            int trxId)
{
  typedef GeneralRuleAppTariffRuleEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, carrier, tariffNumber, rule, cat, ticketDateInt, isHistorical));
  if (hit)
  {
    ruleOut = entry->_ruleOut;
    tariffNumOut = entry->_tariffNumOut;
    return entry->_result;
  }
  bool result(getGeneralRuleAppTariffRuleData(vendor,
                                              carrier,
                                              tariffNumber,
                                              rule,
                                              cat,
                                              ruleOut,
                                              tariffNumOut,
                                              deleteList,
                                              ticketDate,
                                              isHistorical));
  if (LIKELY(entry))
  {
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_tariffNumber = tariffNumber;
    entry->_rule = rule;
    entry->_cat = static_cast<uint16_t>(cat);
    entry->_ruleOut = ruleOut;
    entry->_tariffNumOut = tariffNumOut;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// Currency cache
struct CurrencyEntry : public Entry
{
  bool equal(const CurrencyCode& currencyCode,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _currencyCode == currencyCode;
  }
  CurrencyCode _currencyCode;
  uint64_t _dateInt = 0;
  const Currency* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const Currency*
getCurrency(const CurrencyCode& currencyCode,
            const DateTime& date,
            DeleteList& deleteList,
            const DateTime& ticketDate,
            bool isHistorical,
            int id)
{
  typedef CurrencyEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(
      CacheType::processEntry(hit, trxId, currencyCode, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const Currency* result(getCurrencyData(currencyCode, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_currencyCode = currencyCode;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// CarrierPreference cache
struct CarrierPreferenceEntry : public Entry
{
  bool equal(const CarrierCode& carrier,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _carrier == carrier;
  }
  CarrierCode _carrier;
  uint64_t _dateInt = 0;
  const CarrierPreference* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CarrierPreference*
getCarrierPreference(const CarrierCode& carrier,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical,
                     int trxId)
{
  typedef CarrierPreferenceEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, carrier, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const CarrierPreference* result(
      getCarrierPreferenceData(carrier, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_carrier = carrier;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// Loc cache
struct LocEntry : public Entry
{
  bool
  equal(const LocCode& locCode, uint64_t dateInt, uint64_t ticketDateInt, bool isHistorical)
      const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _locCode == locCode;
  }
  LocCode _locCode;
  uint64_t _dateInt = 0;
  const Loc* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const Loc*
getLoc(const LocCode& locCode,
       const DateTime& date,
       DeleteList& deleteList,
       const DateTime& ticketDate,
       bool isHistorical,
       int id)
{
  typedef LocEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(
      CacheType::processEntry(hit, trxId, locCode, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const Loc* result(getLocData(locCode, date, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_locCode = locCode;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// FareClassApp cache
typedef std::vector<const FareClassAppInfo*> FareClassAppInfoVector;

struct FareClassAppEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             const FareClassCode& fareClass,
             uint64_t ticketDateInt,
             bool isHistorical,
             bool isFareDisplay,
             ...) const
  {
    return _ruleTariff == ruleTariff
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _isFareDisplay == isFareDisplay
           && _rule == rule
           && _fareClass == fareClass
           && _carrier == carrier
           && _vendor == vendor;
  }
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  FareClassCode _fareClass;
  bool _isFareDisplay = false;
  const FareClassAppInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareClassAppInfoVector&
getFareClassApp(const VendorCode& vendor,
                const CarrierCode& carrier,
                TariffNumber ruleTariff,
                const RuleNumber& rule,
                const FareClassCode& fareClass,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical,
                bool isFareDisplay,
                int trxId)
{
  // assert(fareClass.length() <= 8);
  typedef FareClassAppEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           fareClass,
                                           ticketDateInt,
                                           isHistorical,
                                           isFareDisplay));
  if (hit)
  {
    return *entry->_result;
  }
  const FareClassAppInfoVector* result(&getFareClassAppData(vendor,
                                                            carrier,
                                                            ruleTariff,
                                                            rule,
                                                            fareClass,
                                                            deleteList,
                                                            ticketDate,
                                                            isHistorical,
                                                            isFareDisplay));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_fareClass = fareClass;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_isFareDisplay = isFareDisplay;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// PaxType cache
struct PaxTypeEntry : public Entry
{
  bool equal(const PaxTypeCode& paxType,
             const VendorCode& vendor,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _paxType == paxType && _vendor == vendor;
  }
  PaxTypeCode _paxType;
  const PaxTypeInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const PaxTypeInfo*
getPaxType(const PaxTypeCode& paxType,
           const VendorCode& vendor,
           DeleteList& deleteList,
           const DateTime& ticketDate,
           bool isHistorical,
           int id)
{
  typedef PaxTypeEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, paxType, vendor, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const PaxTypeInfo* result(getPaxTypeData(paxType, vendor, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_paxType = paxType;
    entry->_vendor = vendor;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// FareTypeMatrix cache
struct FareTypeMatrixEntry : public Entry
{
  bool
  equal(const FareType& key, uint64_t dateInt, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _key == key;
  }
  Code<3> _key;
  uint64_t _dateInt = 0;
  const FareTypeMatrix* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareTypeMatrix*
getFareTypeMatrix(const FareType& key,
                  const DateTime& date,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical,
                  int trxId)
{
  // assert(key.length() <= 3);
  typedef FareTypeMatrixEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit, trxId, key, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const FareTypeMatrix* result(
      getFareTypeMatrixData(key, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_key = key;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// MarkupBySecondSellerId cache
typedef std::vector<MarkupControl*> MarkupControlVector;

struct MarkupBySecondSellerIdEntry : public Entry
{
  bool equal(const PseudoCityCode& pcc,
             const PseudoCityCode& homePCC,
             const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             int seqNo,
             long secondarySellerId,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _ruleTariff == ruleTariff && _seqNo == seqNo &&
           _secondarySellerId == secondarySellerId && _dateInt == dateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _pcc == pcc &&
           _homePCC == homePCC && _vendor == vendor && _carrier == carrier && _rule == rule;
  }
  PseudoCityCode _pcc;
  PseudoCityCode _homePCC;
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  int64_t _seqNo = 0;
  long _secondarySellerId = 0;
  uint64_t _dateInt = 0;
  const MarkupControlVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MarkupControlVector&
getMarkupBySecondSellerId(const PseudoCityCode& pcc,
                          const PseudoCityCode& homePCC,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          TariffNumber ruleTariff,
                          const RuleNumber& rule,
                          int seqNo,
                          long secondarySellerId,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical,
                          int trxId)
{
  typedef MarkupBySecondSellerIdEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           pcc,
                                           homePCC,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           seqNo,
                                           secondarySellerId,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MarkupControlVector* result(nullptr);
  result = &getMarkupBySecondSellerIdData(pcc,
                                          homePCC,
                                          vendor,
                                          carrier,
                                          ruleTariff,
                                          rule,
                                          seqNo,
                                          secondarySellerId,
                                          date,
                                          deleteList,
                                          ticketDate,
                                          isHistorical);
  if (entry)
  {
    entry->_pcc = pcc;
    entry->_homePCC = homePCC;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_seqNo = seqNo;
    entry->_secondarySellerId = secondarySellerId;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MarkupBySecurityItemNo cache
struct MarkupBySecurityItemNoEntry : public Entry
{
  bool equal(const PseudoCityCode& pcc,
             const PseudoCityCode& homePCC,
             const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             int seqNo,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _ruleTariff == ruleTariff && _seqNo == seqNo && _dateInt == dateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _pcc == pcc &&
           _homePCC == homePCC && _vendor == vendor && _carrier == carrier && _rule == rule;
  }
  PseudoCityCode _pcc;
  PseudoCityCode _homePCC;
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  int64_t _seqNo = 0;
  uint64_t _dateInt = 0;
  const MarkupControlVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MarkupControlVector&
getMarkupBySecurityItemNo(const PseudoCityCode& pcc,
                          const PseudoCityCode& homePCC,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber& rule,
                          int seqNo,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical,
                          int trxId)
{
  typedef MarkupBySecurityItemNoEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           pcc,
                                           homePCC,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           seqNo,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MarkupControlVector* result(nullptr);
  result = &getMarkupBySecurityItemNoData(pcc,
                                          homePCC,
                                          vendor,
                                          carrier,
                                          ruleTariff,
                                          rule,
                                          seqNo,
                                          date,
                                          deleteList,
                                          ticketDate,
                                          isHistorical);
  if (entry)
  {
    entry->_pcc = pcc;
    entry->_homePCC = homePCC;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_seqNo = seqNo;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MarkupByPcc cache
struct MarkupByPccEntry : public Entry
{
  bool equal(const PseudoCityCode& pcc,
             const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             int seqNo,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _ruleTariff == ruleTariff && _seqNo == seqNo && _dateInt == dateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _pcc == pcc &&
           _vendor == vendor && _carrier == carrier && _rule == rule;
  }
  PseudoCityCode _pcc;
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  int64_t _seqNo = 0;
  uint64_t _dateInt = 0;
  const MarkupControlVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MarkupControlVector&
getMarkupByPcc(const PseudoCityCode& pcc,
               const VendorCode& vendor,
               const CarrierCode& carrier,
               TariffNumber ruleTariff,
               const RuleNumber& rule,
               int seqNo,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical,
               int trxId)
{
  typedef MarkupByPccEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           pcc,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           seqNo,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MarkupControlVector* result(&getMarkupByPccData(pcc,
                                                        vendor,
                                                        carrier,
                                                        ruleTariff,
                                                        rule,
                                                        seqNo,
                                                        date,
                                                        deleteList,
                                                        ticketDate,
                                                        isHistorical));
  if (LIKELY(entry))
  {
    entry->_pcc = pcc;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_seqNo = seqNo;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// NegFareSecurity cache
typedef std::vector<NegFareSecurityInfo*> NegFareSecurityInfoVector;

struct NegFareSecurityEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const NegFareSecurityInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const NegFareSecurityInfoVector&
getNegFareSecurity(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical,
                   int trxId)
{
  typedef NegFareSecurityEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const NegFareSecurityInfoVector* result(
      &getNegFareSecurityData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// GeoRuleItem cache
typedef std::vector<GeoRuleItem*> GeoRuleItemVector;

struct GeoRuleItemEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const GeoRuleItemVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const GeoRuleItemVector&
getGeoRuleItem(const VendorCode& vendor,
               int itemNo,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical,
               int trxId)
{
  typedef GeoRuleItemEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const GeoRuleItemVector* result(
      &getGeoRuleItemData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MultiTransportCityCode cache

struct MultiTransportCityCodeEntry : public Entry
{
  bool equal(const LocCode& locCode,
             const CarrierCode& carrier,
             GeoTravelType tvlType,
             uint64_t tvlDateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _tvlDateInt == tvlDateInt
           && _tvlType == tvlType
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _locCode == locCode
           && _carrier == carrier;
  }
  LocCode _locCode;
  CarrierCode _carrier;
  GeoTravelType _tvlType = GeoTravelType::UnknownGeoTravelType;
  uint64_t _tvlDateInt = 0;
  LocCode _result;
  static TSSCacheEnum _cacheIndex;
};

inline LocCode
getMultiTransportCityCode(const LocCode& locCode,
                          const CarrierCode& carrier,
                          GeoTravelType tvlType,
                          const DateTime& tvlDate,
                          const DateTime& ticketDate,
                          bool isHistorical,
                          int id)
{
  typedef MultiTransportCityCodeEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t tvlDateInt(tvlDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           locCode,
                                           carrier,
                                           tvlType,
                                           tvlDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  LocCode result(getMultiTransportCityCodeData(locCode,
                                               carrier,
                                               tvlType,
                                               tvlDate,
                                               ticketDate,
                                               isHistorical));
  if (entry)
  {
    entry->_locCode = locCode;
    entry->_carrier = carrier;
    entry->_tvlType = tvlType;
    entry->_tvlDateInt = tvlDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// SamePoint cache
typedef std::vector<const SamePoint*> SamePointVector;

struct SamePointEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const SamePointVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const SamePointVector&
getSamePoint(const VendorCode& vendor,
             int itemNo,
             const DateTime&,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical,
             int trxId)
{
  typedef SamePointEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const SamePointVector* result(
      &getSamePointData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// DST cache
struct DSTEntry : public Entry
{
  bool equal(const DSTGrpCode& dstGrp) const { return _dstGrp == dstGrp; }
  DSTGrpCode _dstGrp;
  const DST* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const DST*
getDstData(const DSTGrpCode& dstGrp, DeleteList& deleteList, int trxId)
{
  // assert(dstGrp.length() <= 4);
  typedef DSTEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  EntryType* entry(CacheType::processEntry(hit, trxId, dstGrp));
  if (hit)
  {
    return entry->_result;
  }
  const DST* result(DSTDAO::instance().get(deleteList, dstGrp));
  if (LIKELY(entry))
  {
    entry->_dstGrp = dstGrp;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// TariffInhibit cache
struct TariffInhibitEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             Indicator tariffCrossRefType,
             const CarrierCode& carrier,
             TariffNumber fareTariff,
             const TariffCode& ruleTariffCode,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _tariffCrossRefType == tariffCrossRefType && _fareTariff == fareTariff &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _vendor == vendor &&
           _carrier == carrier && _ruleTariffCode == ruleTariffCode;
  }
  Indicator _tariffCrossRefType = 0;
  CarrierCode _carrier;
  TariffNumber _fareTariff = 0;
  TariffCode _ruleTariffCode;
  Indicator _result = 0;
  static TSSCacheEnum _cacheIndex;
};

inline Indicator
getTariffInhibit(const VendorCode& vendor,
                 Indicator tariffCrossRefType,
                 const CarrierCode& carrier,
                 TariffNumber fareTariff,
                 const TariffCode& ruleTariffCode,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical,
                 int id)
{
  typedef TariffInhibitEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           tariffCrossRefType,
                                           carrier,
                                           fareTariff,
                                           ruleTariffCode,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  Indicator result(getTariffInhibitData(vendor,
                                        tariffCrossRefType,
                                        carrier,
                                        fareTariff,
                                        ruleTariffCode,
                                        deleteList,
                                        ticketDate,
                                        isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_tariffCrossRefType = tariffCrossRefType;
    entry->_carrier = carrier;
    entry->_fareTariff = fareTariff;
    entry->_ruleTariffCode = ruleTariffCode;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// CarrierCombination cache
typedef std::vector<CarrierCombination*> CarrierCombinationVector;

struct CarrierCombinationEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const CarrierCombinationVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CarrierCombinationVector&
getCarrierCombination(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      int trxId)
{
  typedef CarrierCombinationEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const CarrierCombinationVector* result(
      &getCarrierCombinationData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// TaxCarrierAppl cache
struct TaxCarrierApplEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const TaxCarrierAppl* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const TaxCarrierAppl*
getTaxCarrierAppl(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical,
                  int trxId)
{
  typedef TaxCarrierApplEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const TaxCarrierAppl* result(
      getTaxCarrierApplData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// EndOnEnd cache
struct EndOnEndEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const EndOnEnd* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const EndOnEnd*
getEndOnEnd(const VendorCode& vendor,
            int itemNo,
            DeleteList& deleteList,
            const DateTime& ticketDate,
            bool isHistorical,
            int trxId)
{
  typedef EndOnEndEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const EndOnEnd* result(getEndOnEndData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}
// TariffRuleRest cache
typedef std::vector<TariffRuleRest*> TariffRuleRestVector;

struct TariffRuleRestEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const TariffRuleRestVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const TariffRuleRestVector&
getTariffRuleRest(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical,
                  int trxId)
{
  typedef TariffRuleRestEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const TariffRuleRestVector* result(
      &getTariffRuleRestData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// TariffCrossRef cache
enum TariffCrossRefCallType
{
  TariffCrossRefCallNone = -1,
  TariffCrossRefCall,
  TariffXRefByFareTariffCall,
  TariffXRefByRuleTariffCall,
  TariffXRefByGenRuleTariffCall,
  TariffXRefByAddonTariffCall
};
typedef const std::vector<TariffCrossRefInfo*> TariffCrossRefInfoVector;

struct TariffCrossRefEntry : public Entry
{
  bool equal(TariffCrossRefCallType call,
             const VendorCode& vendor,
             const CarrierCode& carrier,
             RecordScope crossRefType,
             TariffNumber tariff,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _call == call && _crossRefType == crossRefType && _tariff == tariff &&
           _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _carrier == carrier;
  }
  TariffCrossRefCallType _call = TariffCrossRefCallType::TariffCrossRefCallNone;
  CarrierCode _carrier;
  RecordScope _crossRefType = RecordScope::INTERNATIONAL;
  TariffNumber _tariff = -1;
  uint64_t _dateInt = 0;
  TariffCrossRefInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const TariffCrossRefInfoVector&
getTariffXRef(TariffCrossRefCallType call,
              const VendorCode& vendor,
              const CarrierCode& carrier,
              RecordScope crossRefType,
              TariffNumber tariff,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical,
              int trxId)
{
  typedef TariffCrossRefEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           call,
                                           vendor,
                                           carrier,
                                           crossRefType,
                                           tariff,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const TariffCrossRefInfoVector* result(nullptr);
  switch (call)
  {
  case TariffCrossRefCall:
    result =
        &getTariffXRefData(vendor, carrier, crossRefType, deleteList, ticketDate, isHistorical);
    break;
  case TariffXRefByFareTariffCall:
    result = &getTariffXRefByFareTariffData(
                 vendor, carrier, crossRefType, tariff, date, deleteList, ticketDate, isHistorical);
    break;
  case TariffXRefByRuleTariffCall:
    result = &getTariffXRefByRuleTariffData(
                 vendor, carrier, crossRefType, tariff, date, deleteList, ticketDate, isHistorical);
    break;
  case TariffXRefByGenRuleTariffCall:
    result = &getTariffXRefByGenRuleTariffData(
                 vendor, carrier, crossRefType, tariff, date, deleteList, ticketDate, isHistorical);
    break;
  case TariffXRefByAddonTariffCall:
    result = &getTariffXRefByAddonTariffData(
                 vendor, carrier, crossRefType, tariff, date, deleteList, ticketDate, isHistorical);
    break;
  default:
    std::cerr << __FUNCTION__ << "-invalid call type:" << call << std::endl;
    break;
  }
  if (entry)
  {
    entry->_call = call;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_crossRefType = crossRefType;
    entry->_tariff = tariff;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// FareByRuleItem cache
struct FareByRuleItemEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _vendor == vendor;
  }
  int _itemNo = 0;
  const FareByRuleItemInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareByRuleItemInfo*
getFareByRuleItem(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical,
                  int trxId)
{
  typedef FareByRuleItemEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const FareByRuleItemInfo* result(
      getFareByRuleItemData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// Routing cache
typedef std::vector<Routing*> RoutingVector;

struct RoutingEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber routingTariff,
             const RoutingNumber& routingNumber,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _routingTariff == routingTariff && _dateInt == dateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _vendor == vendor &&
           _carrier == carrier && _routingNumber == routingNumber;
  }
  CarrierCode _carrier;
  TariffNumber _routingTariff = 0;
  RoutingNumber _routingNumber;
  uint64_t _dateInt = 0;
  const RoutingVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const RoutingVector&
getRouting(const VendorCode& vendor,
           const CarrierCode& carrier,
           TariffNumber routingTariff,
           const RoutingNumber& routingNumber,
           const DateTime& date,
           DeleteList& deleteList,
           const DateTime& ticketDate,
           bool isHistorical,
           int trxId)
{
  typedef RoutingEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           routingTariff,
                                           routingNumber,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const RoutingVector* result(&getRoutingData(vendor,
                                              carrier,
                                              routingTariff,
                                              routingNumber,
                                              date,
                                              deleteList,
                                              ticketDate,
                                              isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_routingNumber = routingNumber;
    entry->_routingTariff = routingTariff;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// DateOverrideRuleItem cache
typedef std::vector<DateOverrideRuleItem*> DateOverrideRuleItemVector;

struct DateOverrideRuleItemEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             int itemNo,
             uint64_t applDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _itemNo == itemNo && _applDateInt == applDateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor;
  }
  int _itemNo = 0;
  uint64_t _applDateInt = 0;
  const DateOverrideRuleItemVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const DateOverrideRuleItemVector&
getDateOverrideRuleItem(const VendorCode& vendor,
                        int itemNo,
                        const DateTime& applDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        int trxId)
{
  typedef DateOverrideRuleItemEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t applDateInt(applDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, itemNo, applDateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const DateOverrideRuleItemVector* result(
      &getDateOverrideRuleItemData(vendor, itemNo, deleteList, ticketDate, isHistorical, applDate));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_applDateInt = applDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// NUCFirst cache
struct NUCFirstEntry : public Entry
{
  bool equal(const CurrencyCode& currency,
             const CarrierCode& carrier,
             uint64_t dateInt,
             bool isPortExchange,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isPortExchange == isPortExchange && _isHistorical == isHistorical &&
           _currency == currency && _carrier == carrier;
  }
  CurrencyCode _currency;
  CarrierCode _carrier;
  uint64_t _dateInt = 0;
  bool _isPortExchange = false;
  NUCInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline NUCInfo*
getNUCFirst(const CurrencyCode& currency,
            const CarrierCode& carrier,
            const DateTime& date,
            bool isPortExchange,
            DeleteList& deleteList,
            const DateTime& ticketDate,
            bool isHistorical,
            int id)
{
  typedef NUCFirstEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(CacheType::processEntry(
      hit, trxId, currency, carrier, dateInt, isPortExchange, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  NUCInfo* result(nullptr);
  if (UNLIKELY(isPortExchange))
  {
    result = getNUCFirstData(currency, carrier, date, deleteList, date, isHistorical);
  }
  else
  {
    result = getNUCFirstData(currency, carrier, date, deleteList, ticketDate, isHistorical);
  }
  if (LIKELY(entry))
  {
    entry->_currency = currency;
    entry->_carrier = carrier;
    entry->_dateInt = dateInt;
    entry->_isPortExchange = isPortExchange;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// Zone cache
struct ZoneEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const Zone& zone,
             Indicator zoneType,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _zoneType == zoneType && _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _zone == zone && _vendor == vendor;
  }
  Zone _zone;
  Indicator _zoneType = 0;
  uint64_t _dateInt = 0;
  const ZoneInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const ZoneInfo*
getZone(const VendorCode& vendor,
        const Zone& zone,
        Indicator zoneType,
        const DateTime& date,
        DeleteList& deleteList,
        const DateTime& ticketDate,
        bool isHistorical,
        int id)
{
  typedef ZoneEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (LIKELY(-1 == trxId))
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, zone, zoneType, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const ZoneInfo* result(
      getZoneData(vendor, zone, zoneType, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_zone = zone;
    entry->_zoneType = zoneType;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// BaseFareRule cache
typedef const std::vector<const BaseFareRule*> BaseFareRuleVector;

struct BaseFareRuleEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             int itemNo,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _itemNo == itemNo && _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor;
  }
  int _itemNo = 0;
  uint64_t _dateInt = 0;
  BaseFareRuleVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline BaseFareRuleVector&
getBaseFareRule(const VendorCode& vendor,
                int itemNo,
                const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical,
                int trxId)
{
  typedef BaseFareRuleEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  BaseFareRuleVector* result(
      &getBaseFareRuleData(vendor, itemNo, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// SectorSurcharge cache
typedef const std::vector<SectorSurcharge*> SectorSurchargeVector;

struct SectorSurchargeEntry : public Entry
{
  bool equal(const CarrierCode& carrier,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _carrier == carrier;
  }
  CarrierCode _carrier;
  uint64_t _dateInt = 0;
  SectorSurchargeVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline SectorSurchargeVector&
getSectorSurcharge(const CarrierCode& carrier,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical,
                   int trxId)
{
  typedef SectorSurchargeEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(
      CacheType::processEntry(hit, trxId, carrier, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  SectorSurchargeVector* result(
      &getSectorSurchargeData(carrier, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_carrier = carrier;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// ATPResNationZones cache
// can save pointer to vector and return it even to local DataHandle
// since there is no filtering in DAO and vector returned by DAO is a vector from the cache
// which is not destroyed by DeleteList destructor
typedef std::vector<ATPResNationZones*> ATPResNationZonesVector;

struct ATPResNationZonesEntry : public Entry
{
  bool equal(const NationCode& nation) const { return _nation == nation; }
  NationCode _nation;
  const ATPResNationZonesVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const ATPResNationZonesVector&
getATPResNationZones(const NationCode& nation,
                     DeleteList& deleteList,
                     int trxId)
{
  typedef ATPResNationZonesEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  EntryType* entry(CacheType::processEntry(hit, trxId, nation));
  if (UNLIKELY(hit))
  {
    return *entry->_result;
  }
  const ATPResNationZonesVector* result(&getATPResNationZonesData(nation, deleteList));
  if (UNLIKELY(entry))
  {
    entry->_nation = nation;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MultiAirportCity cache
typedef const std::vector<MultiAirportCity*> MultiAirportCityVector;

struct MultiAirportCityEntry : public Entry
{
  bool equal(const LocCode& city, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _city == city;
  }
  LocCode _city;
  MultiAirportCityVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline MultiAirportCityVector&
getMultiAirportCity(const LocCode& city,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    int trxId)
{
  typedef MultiAirportCityEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit, trxId, city, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  MultiAirportCityVector* result(
      &getMultiAirportCityData(city, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_city = city;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// FCLimitation cache
typedef const std::vector<LimitationFare*> LimitationFareVector;

struct FCLimitationEntry : public Entry
{
  bool equal(uint64_t dateInt, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical;
  }
  uint64_t _dateInt = 0;
  LimitationFareVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline LimitationFareVector&
getFCLimitation(const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical,
                int trxId)
{
  typedef FCLimitationEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit, trxId, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  LimitationFareVector* result(&getFCLimitationData(date, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// GlobalDir cache
struct GlobalDirEntry : public Entry
{
  bool
  equal(GlobalDirection globalDir, uint64_t dateInt, uint64_t ticketDateInt, bool isHistorical)
      const
  {
    return _globalDir == globalDir && _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical;
  }
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  uint64_t _dateInt = 0;
  const GlobalDir* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const GlobalDir*
getGlobalDir(GlobalDirection globalDir,
             const DateTime& date,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical,
             int id)
{
  typedef GlobalDirEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(
      CacheType::processEntry(hit, trxId, globalDir, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const GlobalDir* result(getGlobalDirData(globalDir, date, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_globalDir = globalDir;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// GlobalDirSeg cache
typedef std::vector<GlobalDirSeg*> GlobalDirSegVector;

struct GlobalDirSegEntry : public Entry
{
  bool equal(uint64_t dateInt, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt && _isHistorical == isHistorical;
  }
  uint64_t _dateInt = 0;
  const GlobalDirSegVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const GlobalDirSegVector&
getGlobalDirSeg(const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical,
                int trxId)
{
  typedef GlobalDirSegEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit, trxId, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const GlobalDirSegVector* result(
      &getGlobalDirSegData(date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    GlobalDirSegVector tmp;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// FareByRuleApp cache
typedef std::vector<tse::FareByRuleApp*> FareByRuleAppVector;

struct FareByRuleAppEntry : public Entry
{
  bool equal(const CarrierCode& carrier,
             const std::string& corpId,
             const AccountCode& accountCode,
             const TktDesignator& tktDesignator,
             uint64_t tvlDateInt,
             const std::vector<PaxTypeCode>& paxTypes,
             uint64_t ticketDateInt,
             bool isHistorical,
             bool isFareDisplay,
             ...) const
  {
    return _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _isFareDisplay == isFareDisplay && _carrier == carrier && _corpId == corpId &&
           _accountCode == accountCode && _tktDesignator == tktDesignator &&
           _tvlDateInt == tvlDateInt && _paxTypes == paxTypes;
  }
  CarrierCode _carrier;
  std::string _corpId;
  AccountCode _accountCode;
  TktDesignator _tktDesignator;
  uint64_t _tvlDateInt = 0;
  std::vector<PaxTypeCode> _paxTypes;
  bool _isFareDisplay = false;
  const FareByRuleAppVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareByRuleAppVector&
getFareByRuleApp(const CarrierCode& carrier,
                 const std::string& corpId,
                 const AccountCode& accountCode,
                 const TktDesignator& tktDesignator,
                 const DateTime& tvlDate,
                 std::vector<PaxTypeCode>& paxTypes,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical,
                 bool isFareDisplay,
                 int trxId)
{
  typedef FareByRuleAppEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t tvlDateInt(tvlDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           carrier,
                                           corpId,
                                           accountCode,
                                           tktDesignator,
                                           tvlDateInt,
                                           paxTypes,
                                           ticketDateInt,
                                           isHistorical,
                                           isFareDisplay));
  if (UNLIKELY(hit))
  {
    return *entry->_result;
  }
  const FareByRuleAppVector* result(&getFareByRuleAppData(carrier,
                                                          corpId,
                                                          accountCode,
                                                          tktDesignator,
                                                          tvlDate,
                                                          paxTypes,
                                                          deleteList,
                                                          ticketDate,
                                                          isHistorical,
                                                          isFareDisplay));
  if (UNLIKELY(entry))
  {
    entry->_carrier = carrier;
    entry->_corpId = corpId;
    entry->_accountCode = accountCode;
    entry->_tktDesignator = tktDesignator;
    entry->_tvlDateInt = tvlDateInt;
    entry->_paxTypes = paxTypes;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_isFareDisplay = isFareDisplay;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// FareByRuleCtrl cache
typedef std::vector<FareByRuleCtrlInfo*> FareByRuleCtrlInfoVector;

struct FareByRuleCtrlEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber tariffNumber,
             const RuleNumber& ruleNumber,
             uint64_t tvlDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             bool isFareDisplay,
             ...) const
  {
    return _tariffNumber == tariffNumber && _tvlDateInt == tvlDateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _isFareDisplay == isFareDisplay && _vendor == vendor && _carrier == carrier &&
           _ruleNumber == ruleNumber;
  }
  CarrierCode _carrier;
  TariffNumber _tariffNumber = 0;
  RuleNumber _ruleNumber;
  uint64_t _tvlDateInt = 0;
  bool _isFareDisplay = false;
  const FareByRuleCtrlInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareByRuleCtrlInfoVector&
getFareByRuleCtrl(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  TariffNumber tariffNumber,
                  const RuleNumber& ruleNumber,
                  const DateTime& tvlDate,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical,
                  bool isFareDisplay,
                  int trxId)
{
  typedef FareByRuleCtrlEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t tvlDateInt(tvlDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           tariffNumber,
                                           ruleNumber,
                                           tvlDateInt,
                                           ticketDateInt,
                                           isHistorical,
                                           isFareDisplay));
  if (hit)
  {
    return *entry->_result;
  }
  const FareByRuleCtrlInfoVector* result(&getFareByRuleCtrlData(vendor,
                                                                carrier,
                                                                tariffNumber,
                                                                ruleNumber,
                                                                tvlDate,
                                                                deleteList,
                                                                ticketDate,
                                                                isHistorical,
                                                                isFareDisplay));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_tariffNumber = tariffNumber;
    entry->_ruleNumber = ruleNumber;
    entry->_tvlDateInt = tvlDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_isFareDisplay = isFareDisplay;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// VendorType cache
struct VendorTypeEntry : public Entry
{
  bool equal(const VendorCode& vendor, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _vendor == vendor;
  }
  char _result = 0;
  static TSSCacheEnum _cacheIndex;
};

inline char
getVendorType(const VendorCode& vendor,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical,
              int trxId)
{
  typedef VendorTypeEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit, trxId, vendor, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  char result(getVendorTypeData(vendor, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// TSI cache
struct TSIEntry : public Entry
{
  bool equal(int key) const { return _key == key; }
  int _key = -1;
  const TSIInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const TSIInfo*
getTSI(int key, DeleteList& deleteList, int trxId)
{
  typedef TSIEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  EntryType* entry(CacheType::processEntry(hit, trxId, key));
  if (hit)
  {
    return entry->_result;
  }
  const TSIInfo* result(getTSIData(key, deleteList));
  if (LIKELY(entry))
  {
    entry->_key = key;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// Mileage cache

typedef std::vector<Mileage*> MileageVector;

struct MileageEntry : public Entry
{
  bool equal(const LocCode& origin,
             const LocCode& destination,
             Indicator mileageType,
             GlobalDirection globalDirection,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _mileageType == mileageType && _globalDirection == globalDirection &&
           _origin == origin && _destination == destination;
  }
  LocCode _origin;
  LocCode _destination;
  Indicator _mileageType = 0;
  GlobalDirection _globalDirection = GlobalDirection::NO_DIR;
  const Mileage* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const Mileage*
getMileage(const LocCode& origin,
           const LocCode& destination,
           Indicator mileageType,
           GlobalDirection globalDirection,
           DeleteList& deleteList,
           const DateTime& ticketDate,
           bool isHistorical,
           int id)
{
  typedef MileageEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(CacheType::processEntry(
      hit, trxId, origin, destination, mileageType, globalDirection, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const MileageVector& mileageVector(
      getMileageData(origin, destination, mileageType, deleteList, ticketDate, isHistorical));
  const Mileage* result(nullptr);
  for (const auto elem : mileageVector)
  {
    if (elem->globaldir() == globalDirection)
    {
      result = elem;
      break;
    }
  }
  if (LIKELY(entry))
  {
    entry->_origin = origin;
    entry->_destination = destination;
    entry->_mileageType = mileageType;
    entry->_globalDirection = globalDirection;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// CarrierFlight cache
struct CarrierFlightEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const
  {
    return _itemNo == itemNo && _isHistorical == isHistorical && _ticketDateInt == ticketDateInt &&
           _vendor == vendor;
  }
  int _itemNo = -1;
  const CarrierFlight* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CarrierFlight*
getCarrierFlight(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical,
                 int id)
{
  typedef CarrierFlightEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  int trxId(id);
  if (-1 == trxId)
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(
      CacheType::processEntry(hit, trxId, vendor, itemNo, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const CarrierFlight* result(
      getCarrierFlightData(vendor, itemNo, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// BookingCodeException cache

struct BookingCodeExceptionEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             Indicator conventionNo,
             uint64_t dateInt,
             bool filterExpireDate,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _ruleTariff == ruleTariff && _conventionNo == conventionNo && _dateInt == dateInt &&
           _filterExpireDate == filterExpireDate && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _carrier == carrier &&
           _rule == rule;
  }
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  Indicator _conventionNo = 0;
  uint64_t _dateInt = 0;
  bool _isRuleZero = false;
  bool _filterExpireDate = false;
  const BookingCodeExceptionSequenceList* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const BookingCodeExceptionSequenceList&
getBookingCodeExceptionSequence(const VendorCode& vendor,
                                const CarrierCode& carrier,
                                TariffNumber ruleTariff,
                                const RuleNumber& rule,
                                Indicator conventionNo,
                                const DateTime& date,
                                bool& isRuleZero,
                                bool filterExpireDate,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical,
                                int trxId)
{
  typedef BookingCodeExceptionEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           conventionNo,
                                           dateInt,
                                           filterExpireDate,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    isRuleZero = entry->_isRuleZero;
    return *entry->_result;
  }
  const BookingCodeExceptionSequenceList* result(
      &getBookingCodeExceptionSequenceData(vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           conventionNo,
                                           date,
                                           isRuleZero,
                                           deleteList,
                                           ticketDate,
                                           isHistorical,
                                           filterExpireDate));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_conventionNo = conventionNo;
    entry->_dateInt = dateInt;
    entry->_isRuleZero = isRuleZero;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_filterExpireDate = filterExpireDate;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// BookingCodeException2 cache

struct BookingCodeException2Entry : public Entry
{
  bool equal(const VendorCode& vendor,
             int itemNo,
             bool filterExpireDate,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _itemNo == itemNo && _filterExpireDate == filterExpireDate &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _vendor == vendor;
  }
  int _itemNo = 0;
  bool _filterExpireDate = false;
  const BookingCodeExceptionSequenceList* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const BookingCodeExceptionSequenceList&
getBookingCodeExceptionSequence(const VendorCode& vendor,
                                int itemNo,
                                bool filterExpireDate,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical,
                                int trxId)
{
  typedef BookingCodeException2Entry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, itemNo, filterExpireDate, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const BookingCodeExceptionSequenceList* result(
      &getBookingCodeExceptionSequenceData(
          vendor, itemNo, deleteList, ticketDate, isHistorical, filterExpireDate));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_filterExpireDate = filterExpireDate;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// Cabin cache

struct CabinEntry : public Entry
{
  bool equal(const CarrierCode& carrier,
             const BookingCode& classOfService,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _carrier == carrier &&
           _classOfService == classOfService;
  }
  CarrierCode _carrier;
  BookingCode _classOfService;
  uint64_t _dateInt = 0;
  const Cabin* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const Cabin*
getCabin(const CarrierCode& carrier,
         const BookingCode& classOfService,
         const DateTime& date,
         DeleteList& deleteList,
         const DateTime& ticketDate,
         bool isHistorical,
         int trxId)
{
  typedef CabinEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, carrier, classOfService, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const Cabin* result(
      getCabinData(carrier, classOfService, date, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_carrier = carrier;
    entry->_classOfService = classOfService;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// AddonCombFareClass cache

struct AddonCombFareClassEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             TariffNumber fareTariff,
             const CarrierCode& carrier,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _fareTariff == fareTariff && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _carrier == carrier;
  }
  TariffNumber _fareTariff = -1;
  CarrierCode _carrier;
  const AddonFareClassCombMultiMap* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const AddonFareClassCombMultiMap&
getAddOnCombFareClass(const VendorCode& vendor,
                      TariffNumber fareTariff,
                      const CarrierCode& carrier,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      int trxId)
{
  typedef AddonCombFareClassEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, fareTariff, carrier, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const AddonFareClassCombMultiMap* result(
      &getAddOnCombFareClassData(
          vendor, fareTariff, carrier, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_fareTariff = fareTariff;
    entry->_carrier = carrier;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// AddonFare cache
typedef std::vector<AddonFareInfo*> AddonFareInfoVector;

struct AddonFareEntry : public Entry
{
  bool equal(const LocCode& gatewayMarket,
             const LocCode& interiorMarket,
             const CarrierCode& carrier,
             RecordScope crossRefType,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _crossRefType == crossRefType && _dateInt == dateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical &&
           _gatewayMarket == gatewayMarket && _interiorMarket == interiorMarket &&
           _carrier == carrier;
  }
  LocCode _gatewayMarket;
  LocCode _interiorMarket;
  CarrierCode _carrier;
  RecordScope _crossRefType = RecordScope::INTERNATIONAL;
  uint64_t _dateInt = 0;
  const AddonFareInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const AddonFareInfoVector&
getAddOnFare(const LocCode& gatewayMarket,
             const LocCode& interiorMarket,
             const CarrierCode& carrier,
             RecordScope crossRefType,
             const DateTime& date,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical,
             int trxId)
{
  typedef AddonFareEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           gatewayMarket,
                                           interiorMarket,
                                           carrier,
                                           crossRefType,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const AddonFareInfoVector* result(&getAddOnFareData(gatewayMarket,
                                                      interiorMarket,
                                                      carrier,
                                                      crossRefType,
                                                      date,
                                                      deleteList,
                                                      ticketDate,
                                                      isHistorical));
  if (entry)
  {
    entry->_gatewayMarket = gatewayMarket;
    entry->_interiorMarket = interiorMarket;
    entry->_carrier = carrier;
    entry->_crossRefType = crossRefType;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// CombinabilityRuleInfo cache
typedef std::vector<CombinabilityRuleInfo*> CombinabilityRuleInfoVector;

struct CombinabilityRuleInfoEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _ruleTariff == ruleTariff && _dateInt == dateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _rule == rule &&
           _carrier == carrier;
  }
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  uint64_t _dateInt = 0;
  const CombinabilityRuleInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CombinabilityRuleInfoVector&
getCombinabilityRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     TariffNumber ruleTariff,
                     const RuleNumber& rule,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical,
                     int trxId)
{
  typedef CombinabilityRuleInfoEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, carrier, ruleTariff, rule, dateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const CombinabilityRuleInfoVector* result(
      &getCombinabilityRuleData(
          vendor, carrier, ruleTariff, rule, date, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// CarrierApplicationInfo cache
typedef std::vector<CarrierApplicationInfo*> CarrierApplicationInfoVector;

struct CarrierApplicationInfoEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             int itemNo,
             uint64_t applDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _itemNo == itemNo && _applDateInt == applDateInt && _ticketDateInt == ticketDateInt &&
           _ticketDateInt == ticketDateInt && _isHistorical == isHistorical && _vendor == vendor;
  }
  int _itemNo = 0;
  uint64_t _applDateInt = 0;
  const CarrierApplicationInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CarrierApplicationInfoVector&
getCarrierApplication(const VendorCode& vendor,
                      int itemNo,
                      const DateTime& applDate,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      int trxId)
{
  typedef CarrierApplicationInfoEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t applDateInt(applDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, itemNo, applDateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const CarrierApplicationInfoVector* result(
      &getCarrierApplicationData(vendor, itemNo, deleteList, ticketDate, isHistorical, applDate));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_applDateInt = applDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MarkupSecFilter cache
typedef std::vector<MarkupSecFilter*> MarkupSecFilterVector;

struct MarkupSecFilterEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _ruleTariff == ruleTariff && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _vendor == vendor && _rule == rule &&
           _carrier == carrier;
  }
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  const MarkupSecFilterVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MarkupSecFilterVector&
getMarkupSecFilter(const VendorCode& vendor,
                   const CarrierCode& carrier,
                   TariffNumber ruleTariff,
                   const RuleNumber& rule,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical,
                   int trxId)
{
  typedef MarkupSecFilterEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, vendor, carrier, ruleTariff, rule, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MarkupSecFilterVector* result(
      &getMarkupSecFilterData(
          vendor, carrier, ruleTariff, rule, deleteList, ticketDate, isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MarriedCabin cache
typedef std::vector<MarriedCabin*> MarriedCabinVector;

struct MarriedCabinEntry : public Entry
{
  bool equal(const CarrierCode& carrier,
             const BookingCode& premiumCabin,
             uint64_t versionDateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _versionDateInt == versionDateInt && _ticketDateInt == ticketDateInt &&
           _isHistorical == isHistorical && _premiumCabin == premiumCabin && _carrier == carrier;
  }
  CarrierCode _carrier;
  BookingCode _premiumCabin;
  uint64_t _versionDateInt = 0;
  const MarriedCabinVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MarriedCabinVector&
getMarriedCabins(const CarrierCode& carrier,
                 const BookingCode& premiumCabin,
                 const DateTime& versionDate,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical,
                 int trxId)
{
  typedef MarriedCabinEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t versionDateInt(versionDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(
      hit, trxId, carrier, premiumCabin, versionDateInt, ticketDateInt, isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MarriedCabinVector* result(
      &getMarriedCabinsData(
          carrier, premiumCabin, versionDate, deleteList, ticketDate, isHistorical));
  if (entry)
  {
    entry->_carrier = carrier;
    entry->_premiumCabin = premiumCabin;
    entry->_versionDateInt = versionDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// UtcOffsetDifference cache

struct UtcOffsetDifferenceEntry : public Entry
{
  bool equal(const DSTGrpCode& dstgrp1,
             const DSTGrpCode& dstgrp2,
             uint64_t dateTimeInt1,
             uint64_t dateTimeInt2) const

  {
    return _dateTimeInt1 == dateTimeInt1 && _dateTimeInt2 == dateTimeInt2 && _dstgrp1 == dstgrp1 &&
           _dstgrp2 == dstgrp2;
  }
  DSTGrpCode _dstgrp1;
  DSTGrpCode _dstgrp2;
  uint64_t _dateTimeInt1 = 0;
  uint64_t _dateTimeInt2 = 0;
  short _utcoffset = 0;
  bool _result = false;
  static TSSCacheEnum _cacheIndex;
};

inline bool
getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                       const DSTGrpCode& dstgrp2,
                       short& utcoffset,
                       const DateTime& dateTime1,
                       const DateTime& dateTime2,
                       DeleteList& deleteList,
                       int id)
{
  typedef UtcOffsetDifferenceEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateTimeInt1(dateTime1.getIntRep());
  uint64_t dateTimeInt2(dateTime2.getIntRep());
  int trxId(id);
  if (UNLIKELY(-1 == trxId))
  {
    trxId = TseCallableTrxTask::getCurrentTrxId();
  }
  EntryType* entry(
      CacheType::processEntry(hit, trxId, dstgrp1, dstgrp2, dateTimeInt1, dateTimeInt2));
  if (UNLIKELY(hit))
  {
    utcoffset = entry->_utcoffset;
    return entry->_result;
  }
  bool result(false);
  const DST* dst1(getDstData(dstgrp1, deleteList, trxId));
  const DST* dst2(getDstData(dstgrp2, deleteList, trxId));
  if (LIKELY(dst1 && dst2))
  {
    short min1(dst1->utcoffset(dateTime1));
    short min2(dst2->utcoffset(dateTime2));
    utcoffset = static_cast<short>(min1 - min2);
    result = true;
    if (UNLIKELY(entry))
    {
      entry->_dstgrp1 = dstgrp1;
      entry->_dstgrp2 = dstgrp2;
      entry->_dateTimeInt1 = dateTimeInt1;
      entry->_dateTimeInt2 = dateTimeInt2;
      entry->_utcoffset = utcoffset;
      entry->_result = result;
      entry->_trxId = trxId;
    }
  }
  return result;
}

// SvcFeesSecurity cache

typedef std::vector<SvcFeesSecurityInfo*> SvcFeesSecurityInfoVector;

struct SvcFeesSecurityEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int64_t itemNo, uint64_t ticketDateInt, bool isHistorical)
      const

  {
    return _vendor == vendor
           && _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  int64_t _itemNo = 0;
  const SvcFeesSecurityInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const SvcFeesSecurityInfoVector&
getSvcFeesSecurity(const VendorCode& vendor,
                   int64_t itemNo,
                   DeleteList& deleteList,
                   DateTime ticketDate,
                   bool isHistorical,
                   int trxId)
{
  typedef SvcFeesSecurityEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const SvcFeesSecurityInfoVector* result(&getSvcFeesSecurityData(vendor,
                                                                  itemNo,
                                                                  deleteList,
                                                                  ticketDate,
                                                                  isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// TaxSpecConfig cache

typedef std::vector<TaxSpecConfigReg*> TaxSpecConfigVector;

struct TaxSpecConfigEntry : public Entry
{
  bool
  equal(const TaxSpecConfigName& taxSpecConfigName, uint64_t ticketDateInt, bool isHistorical)
      const

  {
    return _taxSpecConfigName == taxSpecConfigName
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  BoostString _taxSpecConfigName;
  const TaxSpecConfigVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const TaxSpecConfigVector&
getTaxSpecConfig(const TaxSpecConfigName& taxSpecConfigName,
                 DeleteList& deleteList,
                 DateTime ticketDate,
                 bool isHistorical,
                 int trxId)
{
  typedef TaxSpecConfigEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           taxSpecConfigName,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const TaxSpecConfigVector* result(&getTaxSpecConfigData(taxSpecConfigName,
                                                          deleteList,
                                                          ticketDate,
                                                          isHistorical));
  if (LIKELY(entry))
  {
    entry->_taxSpecConfigName = taxSpecConfigName;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MileageSurchExcept cache

typedef std::vector<MileageSurchExcept*> MileageSurchExceptVector;

struct MileageSurchExceptEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             int64_t textTblItemNo,
             const CarrierCode& governingCarrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _ticketDateInt == ticketDateInt
           && _dateInt == dateInt
           && _ruleTariff == ruleTariff
           && _textTblItemNo == textTblItemNo
           && _isHistorical == isHistorical
           && _vendor == vendor
           && _governingCarrier == governingCarrier
           && _rule == rule;
  }
  int64_t _textTblItemNo = 0;
  CarrierCode _governingCarrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  uint64_t _dateInt = 0;
  const MileageSurchExceptVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MileageSurchExceptVector&
getMileageSurchExcept(const VendorCode& vendor,
                      int64_t textTblItemNo,
                      const CarrierCode& governingCarrier,
                      TariffNumber ruleTariff,
                      const RuleNumber& rule,
                      DateTime date,
                      DeleteList& deleteList,
                      DateTime ticketDate,
                      bool isHistorical,
                      int trxId)
{
  typedef MileageSurchExceptEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           textTblItemNo,
                                           governingCarrier,
                                           ruleTariff,
                                           rule,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MileageSurchExceptVector* result(&getMileageSurchExceptData(vendor,
                                                                    textTblItemNo,
                                                                    governingCarrier,
                                                                    ruleTariff,
                                                                    rule,
                                                                    date,
                                                                    deleteList,
                                                                    ticketDate,
                                                                    isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_textTblItemNo = textTblItemNo;
    entry->_governingCarrier = governingCarrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// PaxTypeMatrix cache

typedef std::vector<const PaxTypeMatrix*> PaxTypeMatrixVector;

struct PaxTypeMatrixEntry : public Entry
{
  bool equal(const PaxTypeCode& key, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _key == key
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  PaxTypeCode _key;
  const PaxTypeMatrixVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const PaxTypeMatrixVector&
getPaxTypeMatrix(const PaxTypeCode& key,
                 DeleteList& deleteList,
                 DateTime ticketDate,
                 bool isHistorical,
                 int trxId)
{
  typedef PaxTypeMatrixEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           key,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const PaxTypeMatrixVector* result(&getPaxTypeMatrixData(key,
                                                          deleteList,
                                                          ticketDate,
                                                          isHistorical));
  if (LIKELY(entry))
  {
    entry->_key = key;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MultiCityAirport cache

typedef std::vector<MultiAirportCity*> MultiCityAirportVector;

struct MultiCityAirportEntry : public Entry
{
  bool equal(const LocCode& locCode, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _locCode == locCode
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  LocCode _locCode;
  const MultiCityAirportVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MultiCityAirportVector&
getMultiCityAirport(const LocCode& locCode,
                    DeleteList& deleteList,
                    DateTime ticketDate,
                    bool isHistorical,
                    int trxId)
{
  typedef MultiCityAirportEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           locCode,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MultiCityAirportVector* result(&getMultiCityAirportData(locCode,
                                                                deleteList,
                                                                ticketDate,
                                                                isHistorical));
  if (LIKELY(entry))
  {
    entry->_locCode = locCode;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// OpenJawRule cache

struct OpenJawRuleEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor;
  }
  int _itemNo = 0;
  const OpenJawRule* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const OpenJawRule* getOpenJawRule(const VendorCode& vendor,
                                         int itemNo,
                                         DeleteList& deleteList,
                                         DateTime ticketDate,
                                         bool isHistorical,
                                         int trxId)
{
  typedef OpenJawRuleEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const OpenJawRule* result(getOpenJawRuleData(vendor,
                                               itemNo,
                                               deleteList,
                                               ticketDate,
                                               isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// CircleTripRuleItem cache

struct CircleTripRuleItemEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor;
  }
  int _itemNo = 0;
  const CircleTripRuleItem* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const CircleTripRuleItem* getCircleTripRuleItem(const VendorCode& vendor,
                                                       int itemNo,
                                                       DeleteList& deleteList,
                                                       DateTime ticketDate,
                                                       bool isHistorical,
                                                       int trxId)
{
  typedef CircleTripRuleItemEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const CircleTripRuleItem* result(getCircleTripRuleItemData(vendor,
                                                             itemNo,
                                                             deleteList,
                                                             ticketDate,
                                                             isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// OpenJawRestriction cache

typedef std::vector<OpenJawRestriction*> OpenJawRestrictionVector;

struct OpenJawRestrictionEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor;
  }
  int _itemNo = 0;
  const OpenJawRestrictionVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const OpenJawRestrictionVector&
getOpenJawRestriction(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      DateTime ticketDate,
                      bool isHistorical,
                      int trxId)
{
  typedef OpenJawRestrictionEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const OpenJawRestrictionVector* result(&getOpenJawRestrictionData(vendor,
                                                                    itemNo,
                                                                    deleteList,
                                                                    ticketDate,
                                                                    isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// DBEGlobalClass cache

typedef std::vector<DBEGlobalClass*> DBEGlobalClassVector;

struct DBEGlobalClassEntry : public Entry
{
  bool equal(const DBEClass& key, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _key == key
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  DBEClass _key;
  const DBEGlobalClassVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const DBEGlobalClassVector&
getDBEGlobalClass(const DBEClass& key,
                  DeleteList& deleteList,
                  DateTime ticketDate,
                  bool isHistorical,
                  int trxId)
{
  typedef DBEGlobalClassEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           key,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const DBEGlobalClassVector* result(&getDBEGlobalClassData(key,
                                                            deleteList,
                                                            ticketDate,
                                                            isHistorical));
  if (LIKELY(entry))
  {
    entry->_key = key;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// FareClassRestRule cache

typedef std::vector<FareClassRestRule*> FareClassRestRuleVector;

struct FareClassRestRuleEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor;
  }
  int _itemNo = 0;
  const FareClassRestRuleVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareClassRestRuleVector&
getFareClassRestRule(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     DateTime ticketDate,
                     bool isHistorical,
                     int trxId)
{
  typedef FareClassRestRuleEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const FareClassRestRuleVector* result(&getFareClassRestRuleData(vendor,
                                                                  itemNo,
                                                                  deleteList,
                                                                  ticketDate,
                                                                  isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// IndustryFareBasisMod cache

typedef std::vector<const IndustryFareBasisMod*> IndustryFareBasisModVector;

struct IndustryFareBasisModEntry : public Entry
{
  bool equal(const CarrierCode& carrier,
             Indicator userApplType,
             const UserApplCode& userAppl,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _userApplType == userApplType
           && _dateInt == dateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _userAppl == userAppl
           && _carrier == carrier;
  }
  CarrierCode _carrier;
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  uint64_t _dateInt = 0;
  const IndustryFareBasisModVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const IndustryFareBasisModVector&
getIndustryFareBasisMod(const CarrierCode& carrier,
                        Indicator userApplType,
                        const UserApplCode& userAppl,
                        DateTime date,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical,
                        int trxId)
{
  typedef IndustryFareBasisModEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  uint64_t dateInt(date.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           carrier,
                                           userApplType,
                                           userAppl,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const IndustryFareBasisModVector* result(&getIndustryFareBasisModData(carrier,
                                                                        userApplType,
                                                                        userAppl,
                                                                        date,
                                                                        deleteList,
                                                                        ticketDate,
                                                                        isHistorical));
  if (LIKELY(entry))
  {
    entry->_carrier = carrier;
    entry->_userApplType = userApplType;
    entry->_userAppl = userAppl;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// TaxCodeReg cache

typedef std::vector<TaxCodeReg*> TaxCodeRegVector;

struct TaxCodeRegEntry : public Entry
{
  bool
  equal(const TaxCode& taxCode, uint64_t dateInt, uint64_t ticketDateInt, bool isHistorical)
      const

  {
    return _dateInt == dateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _taxCode == taxCode;
  }
  TaxCode _taxCode;
  uint64_t _dateInt = 0;
  const TaxCodeRegVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const TaxCodeRegVector&
getTaxCode(const TaxCode& taxCode,
           const DateTime& date,
           DeleteList& deleteList,
           DateTime ticketDate,
           bool isHistorical,
           int trxId)
{
  typedef TaxCodeRegEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  uint64_t dateInt(date.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           taxCode,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (UNLIKELY(hit))
  {
    return *entry->_result;
  }
  const TaxCodeRegVector* result(&getTaxCodeData(taxCode,
                                                 date,
                                                 deleteList,
                                                 ticketDate,
                                                 isHistorical));
  if (UNLIKELY(entry))
  {
    entry->_taxCode = taxCode;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// SvcFeesCxrResultingFCLInfo cache

typedef std::vector<SvcFeesCxrResultingFCLInfo*> SvcFeesCxrResultingFCLInfoVector;

struct SvcFeesCxrResultingFCLInfoEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor;
  }
  int _itemNo = 0;
  const SvcFeesCxrResultingFCLInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const SvcFeesCxrResultingFCLInfoVector&
getSvcFeesCxrResultingFCL(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical,
                          int trxId)
{
  typedef SvcFeesCxrResultingFCLInfoEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const SvcFeesCxrResultingFCLInfoVector* result(&getSvcFeesCxrResultingFCLData(vendor,
                                                                                itemNo,
                                                                                deleteList,
                                                                                ticketDate,
                                                                                isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// Nation cache

struct NationEntry : public Entry
{
  bool equal(const NationCode& nationCode,
             uint64_t dateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _dateInt == dateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _nationCode == nationCode;
  }
  NationCode _nationCode;
  uint64_t _dateInt = 0;
  const Nation* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const Nation* getNation(const NationCode& nationCode,
                               DateTime date,
                               DeleteList& deleteList,
                               DateTime ticketDate,
                               bool isHistorical,
                               int trxId)
{
  typedef NationEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t dateInt(date.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           nationCode,
                                           dateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const Nation* result(getNationData(nationCode,
                                     date,
                                     deleteList,
                                     ticketDate,
                                     isHistorical));
  if (entry)
  {
    entry->_nationCode = nationCode;
    entry->_dateInt = dateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// AllNations cache

typedef std::vector<Nation*> NationVector;

struct AllNationsEntry : public Entry
{
  bool equal(uint64_t ticketDateInt, bool isHistorical) const

  {
    return _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  const NationVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const NationVector&
getAllNations(DateTime ticketDate,
              DeleteList& deleteList,
              bool isHistorical,
              int trxId)
{
  typedef AllNationsEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           ticketDateInt,
                                           isHistorical));
  if (UNLIKELY(hit))
  {
    return *entry->_result;
  }
  const NationVector* result(&getAllNationData(ticketDate, deleteList, isHistorical));
  if (UNLIKELY(entry))
  {
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// MultiTransportCityLocCode cache

struct MultiTransportCityLocCodeEntry : public Entry
{
  bool equal(const LocCode& locCode, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _locCode == locCode;
  }
  LocCode _locCode;
  LocCode _result;
  static TSSCacheEnum _cacheIndex;
};

inline LocCode getMultiTransportCity(const LocCode& locCode,
                                     DeleteList& deleteList,
                                     DateTime ticketDate,
                                     bool isHistorical,
                                     int trxId)
{
  typedef MultiTransportCityLocCodeEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           locCode,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const LocCode result(getMultiTransportCityData(locCode,
                                                 deleteList,
                                                 ticketDate,
                                                 isHistorical));
  if (entry)
  {
    entry->_locCode = locCode;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// MultiTransportCityVector cache

typedef std::vector<MultiTransport*> MultiTransportCityVector;
struct MultiTransportCityVectorEntry : public Entry
{
  bool equal(const LocCode& locCode,
             const CarrierCode& carrier,
             GeoTravelType tvlType,
             uint64_t tvlDateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const

  {
    return _tvlDateInt == tvlDateInt
           && _ticketDateInt == ticketDateInt
           && _tvlType == tvlType
           && _isHistorical == isHistorical
           && _locCode == locCode
           && _carrier == carrier;
  }
  LocCode _locCode;
  CarrierCode _carrier;
  GeoTravelType _tvlType = GeoTravelType::UnknownGeoTravelType;
  uint64_t _tvlDateInt = 0;
  const MultiTransportCityVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const MultiTransportCityVector&
getMultiTransportCity(const LocCode& locCode,
                      const CarrierCode& carrier,
                      GeoTravelType tvlType,
                      DateTime tvlDate,
                      DeleteList& deleteList,
                      DateTime ticketDate,
                      bool isHistorical,
                      int trxId)
{
  typedef MultiTransportCityVectorEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t tvlDateInt(tvlDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           locCode,
                                           carrier,
                                           tvlType,
                                           tvlDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const MultiTransportCityVector* result(&getMultiTransportCityData(locCode,
                                                                    carrier,
                                                                    tvlType,
                                                                    tvlDate,
                                                                    deleteList,
                                                                    ticketDate,
                                                                    isHistorical));
  if (LIKELY(entry))
  {
    entry->_locCode = locCode;
    entry->_carrier = carrier;
    entry->_tvlType = tvlType;
    entry->_tvlDateInt = tvlDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// SalesRestriction cache

struct SalesRestrictionEntry : public Entry
{
  bool
  equal(const VendorCode& vendor, int itemNo, uint64_t ticketDateInt, bool isHistorical) const

  {
    return _itemNo == itemNo
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor;
  }
  int _itemNo = 0;
  const SalesRestriction* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const SalesRestriction* getSalesRestriction(const VendorCode& vendor,
                                                   int itemNo,
                                                   DeleteList& deleteList,
                                                   DateTime ticketDate,
                                                   bool isHistorical,
                                                   int trxId)
{
  typedef SalesRestrictionEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           itemNo,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const SalesRestriction* result(getSalesRestrictionData(vendor,
                                                         itemNo,
                                                         deleteList,
                                                         ticketDate,
                                                         isHistorical));
  if (LIKELY(entry))
  {
    entry->_vendor = vendor;
    entry->_itemNo = itemNo;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// FareFocusSecurity cache

struct FareFocusSecurityEntry : public Entry
{
  bool equal(uint64_t securityItemNo,
             uint64_t adjustedTicketDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _securityItemNo == securityItemNo
           && _adjustedTicketDateInt == adjustedTicketDateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  uint64_t _securityItemNo = 0;
  uint64_t _adjustedTicketDateInt = 0;
  const FareFocusSecurityInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareFocusSecurityInfo*
getFareFocusSecurity(uint64_t securityItemNo,
                     DateTime adjustedTicketDate,
                     DeleteList& deleteList,
                     DateTime ticketDate,
                     bool isHistorical,
                     int trxId)
{
  typedef FareFocusSecurityEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t adjustedTicketDateInt(adjustedTicketDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           securityItemNo,
                                           adjustedTicketDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const FareFocusSecurityInfo* result(getFareFocusSecurityData(securityItemNo,
                                                               adjustedTicketDate,
                                                               deleteList,
                                                               ticketDate,
                                                               isHistorical));
  if (entry)
  {
    entry->_securityItemNo = securityItemNo;
    entry->_adjustedTicketDateInt = adjustedTicketDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// FareFocusRule cache

struct FareFocusRuleEntry : public Entry
{
  bool equal(uint64_t fareFocusRuleId,
             uint64_t adjustedTicketDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const

  {
    return _fareFocusRuleId == fareFocusRuleId
           && _adjustedTicketDateInt == adjustedTicketDateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical;
  }
  uint64_t _fareFocusRuleId = 0;
  uint64_t _adjustedTicketDateInt = 0;
  const FareFocusRuleInfo* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareFocusRuleInfo*
getFareFocusRule(uint64_t fareFocusRuleId,
                 DateTime adjustedTicketDate,
                 DeleteList& deleteList,
                 DateTime ticketDate,
                 bool isHistorical,
                 int trxId)
{
  typedef FareFocusRuleEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t adjustedTicketDateInt(adjustedTicketDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           fareFocusRuleId,
                                           adjustedTicketDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  const FareFocusRuleInfo* result(getFareFocusRuleData(fareFocusRuleId,
                                                       adjustedTicketDate,
                                                       deleteList,
                                                       ticketDate,
                                                       isHistorical));
  if (entry)
  {
    entry->_fareFocusRuleId = fareFocusRuleId;
    entry->_adjustedTicketDateInt = adjustedTicketDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// FareClassAppByTravelDate cache
typedef std::vector<const FareClassAppInfo*> FareClassAppInfoVector;

struct FareClassAppByTravelDateEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber ruleTariff,
             const RuleNumber& rule,
             const FareClassCode& fareClass,
             uint64_t travelDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             bool isFareDisplay,
             ...) const
  {
    return _ruleTariff == ruleTariff
           && _travelDateInt == travelDateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _isFareDisplay == isFareDisplay
           && _rule == rule
           && _fareClass == fareClass
           && _carrier == carrier
           && _vendor == vendor;
  }
  CarrierCode _carrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  FareClassCode _fareClass;
  uint64_t _travelDateInt = 0;
  bool _isFareDisplay = false;
  const FareClassAppInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const FareClassAppInfoVector&
getFareClassAppDataByTravelDate(const VendorCode& vendor,
                                const CarrierCode& carrier,
                                TariffNumber ruleTariff,
                                const RuleNumber& rule,
                                const FareClassCode& fareClass,
                                DateTime travelDate,
                                DeleteList& deleteList,
                                DateTime ticketDate,
                                bool isHistorical,
                                bool isFareDisplay,
                                int trxId)
{
  // assert(fareClass.length() <= 8);
  typedef FareClassAppByTravelDateEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t travelDateInt(travelDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           ruleTariff,
                                           rule,
                                           fareClass,
                                           travelDateInt,
                                           ticketDateInt,
                                           isHistorical,
                                           isFareDisplay));
  if (hit)
  {
    return *entry->_result;
  }
  const FareClassAppInfoVector* result(&getFareClassAppDataByTravelDT(vendor,
                                                                      carrier,
                                                                      ruleTariff,
                                                                      rule,
                                                                      fareClass,
                                                                      travelDate,
                                                                      deleteList,
                                                                      ticketDate,
                                                                      isHistorical,
                                                                      isFareDisplay));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_ruleTariff = ruleTariff;
    entry->_rule = rule;
    entry->_fareClass = fareClass;
    entry->_travelDateInt = travelDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_isFareDisplay = isFareDisplay;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// RBDByCabin cache
typedef std::vector<RBDByCabinInfo*> RBDByCabinInfoVector;

struct RBDByCabinEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             uint64_t travelDateInt,
             uint64_t ticketDateInt,
             bool isHistorical) const
  {
    return _travelDateInt == travelDateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _carrier == carrier
           && _vendor == vendor;
  }
  CarrierCode _carrier;
  uint64_t _travelDateInt = 0;
  const RBDByCabinInfoVector* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline const RBDByCabinInfoVector&
getRBDByCabin(const VendorCode& vendor,
              const CarrierCode& carrier,
              DateTime travelDate,
              DeleteList& deleteList,
              DateTime ticketDate,
              bool isHistorical,
              int trxId)
{
  typedef RBDByCabinEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t travelDateInt(travelDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           travelDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return *entry->_result;
  }
  const RBDByCabinInfoVector* result(&getRBDByCabinData(vendor,
                                                        carrier,
                                                        travelDate,
                                                        deleteList,
                                                        ticketDate,
                                                        isHistorical));
  if (entry)
  {
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_travelDateInt = travelDateInt;
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return *result;
}

// GeneralRuleAppByTvlDate cache
struct GeneralRuleAppByTvlDateEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber tariffNumber,
             const RuleNumber& rule,
             CatNumber cat,
             uint64_t tvlDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _cat == cat
           && _tariffNumber == tariffNumber
           && _tvlDateInt == tvlDateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor
           && _carrier == carrier
           && _rule == rule;
  }
  CarrierCode _carrier;
  TariffNumber _tariffNumber = 0;
  RuleNumber _rule;
  uint64_t _tvlDateInt = 0;
  GeneralRuleApp* _result = nullptr;
  static TSSCacheEnum _cacheIndex;
};

inline GeneralRuleApp* getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  TariffNumber tariffNumber,
                                                  const RuleNumber& rule,
                                                  CatNumber cat,
                                                  DateTime tvlDate,
                                                  DateTime ticketDate,
                                                  bool isHistorical,
                                                  DeleteList& deleteList,
                                                  int trxId)
{
  typedef GeneralRuleAppByTvlDateEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t ticketDateInt(ticketDate.getIntRep());
  uint64_t tvlDateInt(tvlDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           tariffNumber,
                                           rule,
                                           cat,
                                           tvlDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    return entry->_result;
  }
  GeneralRuleApp* result(getGeneralRuleAppByTvlDateData(vendor,
                                                        carrier,
                                                        tariffNumber,
                                                        rule,
                                                        cat,
                                                        deleteList,
                                                        ticketDate,
                                                        isHistorical,
                                                        tvlDate));
  if (entry)
  {
    entry->_cat = static_cast<uint16_t>(cat);
    entry->_ticketDateInt = ticketDateInt;
    entry->_tvlDateInt = tvlDateInt;
    entry->_isHistorical = isHistorical;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_tariffNumber = tariffNumber;
    entry->_rule = rule;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

// GeneralRuleAppTariffRuleByTvlDate cache
struct GeneralRuleAppTariffRuleByTvlDateEntry : public Entry
{
  bool equal(const VendorCode& vendor,
             const CarrierCode& carrier,
             TariffNumber tariffNumber,
             const RuleNumber& rule,
             CatNumber cat,
             uint64_t tvlDateInt,
             uint64_t ticketDateInt,
             bool isHistorical,
             ...) const
  {
    return _tariffNumber == tariffNumber
           && _cat == cat
           && _tvlDateInt == tvlDateInt
           && _ticketDateInt == ticketDateInt
           && _isHistorical == isHistorical
           && _vendor == vendor
           && _carrier == carrier
           && _rule == rule;
  }
  CarrierCode _carrier;
  TariffNumber _tariffNumber = 0;
  RuleNumber _rule;
  uint64_t _tvlDateInt = 0;
  bool _result = false;
  RuleNumber _ruleOut;
  TariffNumber _tariffNumOut = 0;
  static TSSCacheEnum _cacheIndex;
};

inline bool getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 TariffNumber tariffNumber,
                                                 const RuleNumber& rule,
                                                 CatNumber cat,
                                                 DateTime tvlDate,
                                                 RuleNumber& ruleOut,
                                                 TariffNumber& tariffNumOut,
                                                 DeleteList& deleteList,
                                                 DateTime ticketDate,
                                                 bool isHistorical,
                                                 int trxId)
{
  typedef GeneralRuleAppTariffRuleByTvlDateEntry EntryType;
  typedef Cache<EntryType> CacheType;
  bool hit(false);
  uint64_t tvlDateInt(tvlDate.getIntRep());
  uint64_t ticketDateInt(ticketDate.getIntRep());
  EntryType* entry(CacheType::processEntry(hit,
                                           trxId,
                                           vendor,
                                           carrier,
                                           tariffNumber,
                                           rule,
                                           cat,
                                           tvlDateInt,
                                           ticketDateInt,
                                           isHistorical));
  if (hit)
  {
    ruleOut = entry->_ruleOut;
    tariffNumOut = entry->_tariffNumOut;
    return entry->_result;
  }
  bool result(getGeneralRuleAppTariffRuleByTvlDateData(vendor,
                                                       carrier,
                                                       tariffNumber,
                                                       rule,
                                                       cat,
                                                       ruleOut,
                                                       tariffNumOut,
                                                       deleteList,
                                                       ticketDate,
                                                       isHistorical,
                                                       tvlDate));
  if (entry)
  {
    entry->_ticketDateInt = ticketDateInt;
    entry->_isHistorical = isHistorical;
    entry->_vendor = vendor;
    entry->_carrier = carrier;
    entry->_tariffNumber = tariffNumber;
    entry->_rule = rule;
    entry->_cat = static_cast<uint16_t>(cat);
    entry->_tvlDateInt = tvlDateInt;
    entry->_ruleOut = ruleOut;
    entry->_tariffNumOut = tariffNumOut;
    entry->_result = result;
    entry->_trxId = trxId;
  }
  return result;
}

}// tsscache

}// tse
