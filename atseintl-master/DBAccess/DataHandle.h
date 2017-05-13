
//----------------------------------------------------------------------------
//
//  File:           DataHandle.h
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
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DaoFilterIteratorUtils.h"
#include "DBAccess/DaoPredicates.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/RuleCategoryDescInfo.h"
#include "DBAccess/TSIInfo.h"
#include "Util/BranchPrediction.h"

#include <vector>
#include <utility>

namespace tse
{

class AddonCombFareClassInfo;
class AddonFareInfo;
class AddonZoneInfo;
class AirlineAllianceCarrierInfo;
class AirlineAllianceContinentInfo;
class AirlineCountrySettlementPlanInfo;
class AirlineInterlineAgreementInfo;
class ATPResNationZones;
class BankerSellRate;
class BankIdentificationInfo;
class BaseFareRule;
class BookingCodeExceptionSequenceList;
class Brand;
class BrandedCarrier;
class BrandedFare;
class BrandedFareApp;
class Cabin;
class CarrierApplicationInfo;
class CarrierCombination;
class CarrierFlight;
class CarrierMixedClass;
class CarrierPreference;
class Cat05OverrideCarrier;
class CircleTripProvision;
class CircleTripRuleItem;
class CommissionCap;
class CommissionContractInfo;
class CommissionProgramInfo;
class CommissionRuleInfo;
class ContractPreference;
class CopMinimum;
class CopParticipatingNation;
class CorpId;
class CountrySettlementPlanInfo;
class Currency;
class CurrencySelection;
class Customer;
class CustomerSecurityHandshakeInfo;
class CustomerActivationControl;
class DateOverrideRuleItem;
class DayTimeAppInfo;
class DBEGlobalClass;
class Differentials;
class DiscountInfo;
class DST;
class EligibilityInfo;
class EmdInterlineAgreementInfo;
class EndOnEnd;
class FareByRuleApp;
class FareByRuleCtrlInfo;
class FareByRuleItemInfo;
class FareCalcConfig;
class FareCalcConfigText;
class FareClassAppInfo;
class FareClassRestRule;
class FareDispCldInfPsgType;
class FareDispInclDsplType;
class FareDispInclFareType;
class FareDispInclRuleTrf;
class FareDisplayInclCd;
class FareDisplayPref;
class FareDisplayPrefSeg;
class FareDisplaySort;
class FareDisplayWeb;
class FareDispRec1PsgType;
class FareDispRec8PsgType;
class FareDispTemplate;
class FareDispTemplateSeg;
class FareFocusAccountCdInfo;
class FareFocusRoutingInfo;
class FareFocusLocationPairInfo;
class FareFocusDaytimeApplInfo;
class FareFocusDisplayCatTypeInfo;
class FareFocusPsgTypeInfo;
class FareFocusRuleInfo;
class FareFocusBookingCodeInfo;
class FareFocusCarrierInfo;
class FareFocusFareClassInfo;
class FareFocusLookupInfo;
class FareFocusRuleCodeInfo;
class FareFocusSecurityInfo;
class FareInfo;
class FareProperties;
class FareRetailerCalcInfo;
class FareRetailerResultingFareAttrInfo;
class FareRetailerRuleInfo;
class FareRetailerRuleLookupInfo;
class FareTypeMatrix;
class FareTypeQualifier;
class FareTypeTable;
struct FreqFlyerStatus;
class FreqFlyerStatusSeg;
class FDHeaderMsg;
class FDSFareBasisComb;
class FDSGlobalDir;
class FDSPsgType;
class FDSSorting;
class FDSuppressFare;
class FlightAppRule;
class FltTrkCntryGrp;
class FootNoteCtrlInfo;
class GeneralFareRuleInfo;
class GeneralRuleApp;
class GenSalesAgentInfo;
class GeoRuleItem;
class GlobalDir;
class GlobalDirSeg;
class Groups;
class IndustryFareAppl;
class IndustryFareBasisMod;
class IndustryPricingAppl;
class InterlineCarrierInfo;
class IntralineCarrierInfo;
class InterlineTicketCarrierInfo;
class InterlineTicketCarrierStatus;
class JointCarrier;
class LimitationFare;
class LimitationJrny;
class Loc;
class MarketRoutingInfo;
class MarkupControl;
class MarkupSecFilter;
class MarriedCabin;
class MaxPermittedAmountFareInfo;
class MaxStayRestriction;
class MerchActivationInfo;
class MerchCarrierPreferenceInfo;
class Mileage;
class MileageSubstitution;
class MileageSurchExcept;
class MinFareAppl;
class MinFareDefaultLogic;
class MinFareFareTypeGrp;
class MinFareRuleLevelExcl;
class MinStayRestriction;
class MultiAirportCity;
class MultiTransport;
class Nation;
class NegFareCalcInfo;
class NegFareRest;
class NegFareRestExt;
class NegFareRestExtSeq;
class NegFareSecurityInfo;
class NeutralValidatingAirlineInfo;
class NoPNRFareTypeGroup;
class NoPNROptions;
class NUCInfo;
class NvbNvaInfo;
class OpenJawRestriction;
class OpenJawRule;
class OptionalServicesActivationInfo;
class OptionalServicesConcur;
class OptionalServicesInfo;
class PaxTypeCodeInfo;
class PaxTypeInfo;
class PaxTypeMatrix;
class PenaltyInfo;
class PfcAbsorb;
class PfcCollectMeth;
class PfcEquipTypeExempt;
class PfcEssAirSvc;
class PfcPFC;
class PfcMultiAirport;
class PfcTktDesigExcept;
class PrintOption;
class ReissueSequence;
class RoundTripRuleItem;
class Routing;
class RoutingKeyInfo;
class RuleCatAlphaCode;
class RuleItemInfo;
class SalesNationRestr;
class SalesRestriction;
class SamePoint;
class SeasonalAppl;
class SeasonalityDOW;
class SeatCabinCharacteristicInfo;
class SectorDetailInfo;
class SectorSurcharge;
class ServiceBaggageInfo;
class ServiceGroupInfo;
class ServicesDescription;
class ServicesSubGroup;
class SpanishReferenceFareInfo;
class State;
class SubCodeInfo;
class SurfaceSectorExempt;
class SurfaceSectorExemptionInfo;
class SurfaceTransfersInfo;
class SvcFeesAccCodeInfo;
class SvcFeesCurrencyInfo;
class SvcFeesCxrResultingFCLInfo;
class SvcFeesFareIdInfo;
class SvcFeesFeatureInfo;
class SvcFeesResBkgDesigInfo;
class SvcFeesSecurityInfo;
class SvcFeesTktDesignatorInfo;
class TariffCrossRefInfo;
class TariffMileageAddon;
class TariffRuleRest;
class TaxAkHiFactor;
class TaxCarrierAppl;
class TaxCarrierFlightInfo;
class TaxCodeReg;
class TaxCodeTextInfo;
class TaxExemption;
class TaxNation;
class TaxReissue;
class TaxReportingRecordInfo;
class TaxRestrictionLocationInfo;
class TaxRulesRecord;
class TaxSegAbsorb;
class TaxSpecConfigReg;
class TaxText;
class TicketingFeesInfo;
class TicketStock;
class TktDesignatorExemptInfo;
class Tours;
class TpdPsr;
class TPMExclusion;
class TravelRestriction;
class ValueCodeAlgorithm;
class Waiver;
class YQYRFees;
class YQYRFeesNonConcur;
class ZoneInfo;

class DataHandle
{
public:
  DataHandle(DateTime date = DateTime::localTime(),
             size_t deleteListSize = 1,
             int trxId = -1,
             Memory::CompositeManager* parentManager = nullptr)
    : _deleteList(deleteListSize, parentManager),
      _parentDataHandle(nullptr),
      _ticketDate(date),
      _isFareDisplay(false),
      _isPortExchange(false),
      _isHistorical(isHistEnabled(date)),
      _useTLS(false),
      // trx id stays -1 if DataHandle is not Trx contained
      // otherwise it will be set in Trx constructor
      _trxId(trxId),
      _today(date.date())
  {
  }

  bool isHistEnabled(const DateTime& date) const
  {
    return (Global::allowHistorical() && date.isValid() && date < DateTime::localTime());
  }

  void refreshHist(DateTime date) { _isHistorical = isHistEnabled(date); }

  template <typename T>
  void get(T*& t)
  {
    t = static_cast<T*>(allocate(sizeof(T)));
    new (t) T;

    _deleteList.adoptPooled(t);
  }

  template <typename T>
  T* create()
  {
    T* t;
    get(t);
    return t;
  }

  void setParentDataHandle(DataHandle* parentDataHandle)
  {
    if (LIKELY(-1 == _trxId)
        && parentDataHandle
        && parentDataHandle->_trxId > -1)
    {
      _parentDataHandle = parentDataHandle;
    }
  }

private:
  void* allocate(size_t size)
  {
    void* ptr = ::malloc(size);

    if (UNLIKELY(nullptr == ptr))
      throw std::bad_alloc();

    return ptr;
  }

  void deallocate(void* ptr) { ::free(ptr); }

public:
  template <typename T, typename... Args>
  T& safe_create(Args&&... args)
  {
    T* result = static_cast<T*>(allocate(sizeof(T)));
    try
    {
      new (result) T(std::forward<Args>(args)...);
      _deleteList.adoptPooled(result);
      return *result;
    }
    catch (...)
    {
      deallocate(result);
      throw;
    }
  }

  void import(DataHandle& another) { _deleteList.import(another._deleteList); }

  void clear() { _deleteList.clear(); }

  void setTicketDate(const DateTime& date)
  {
    _ticketDate = date;
    _isHistorical = isHistEnabled(date);
  }

  const DateTime& ticketDate() const
  {
    if (UNLIKELY(_useTLS))
    {
      DateTime* tktDate = (DateTime*)pthread_getspecific(_threadSpecificTicketDateKey);
      if (tktDate)
        return *tktDate;
    }

    return _ticketDate;
  };

  const bool isHistorical() const
  {
    if (UNLIKELY(_useTLS))
    {
      return pthread_getspecific(_threadSpecificIsHistoricalKey) == nullptr ? false : true;
    }

    return _isHistorical;
  };

  template <typename RandomAccessContainer, typename Func, typename FilterPredicate>
  void
  forEachGeneral(const RandomAccessContainer& container, Func&& func, FilterPredicate predicate)
  {
    auto it = std::find_if(container.begin(), container.end(), predicate);
    while (it != container.end())
    {
      func(*it);
      it = std::find_if(it + 1, container.end(), predicate);
    }
  }

  bool& isFareDisplay() { return _isFareDisplay; }
  const bool& isFareDisplay() const { return _isFareDisplay; }

  bool& isPortExchange() { return _isPortExchange; }
  const bool& isPortExchange() const { return _isPortExchange; }

  void setIsFareDisplay(bool isFD = true) { _isFareDisplay = isFD; }

  const DateTime& getToday() const { return _today; }

  const State*
  getState(const NationCode& nationCode, const StateCode& stateCode, const DateTime& date);

  const std::vector<SurfaceSectorExemptionInfo*>&
  getSurfaceSectorExemptionInfo(const CarrierCode& carrierCode, const DateTime& ticketDate);

  std::vector<const tse::FDHeaderMsg*>& getHeaderMsgDataList(const PseudoCityCode& pseudoCityCode,
                                                             const PseudoCityType& pseudoCityType,
                                                             const Indicator& userApplType,
                                                             const std::string& userAppl,
                                                             const TJRGroup& tjrGroup,
                                                             const DateTime& date);

  std::vector<const tse::FDSuppressFare*>& getSuppressFareList(const PseudoCityCode& pseudoCityCode,
                                                               const Indicator pseudoCityType,
                                                               const TJRGroup& ssgGroupNo,
                                                               const CarrierCode& carrierCode,
                                                               const DateTime& date);

  const std::vector<CarrierCode>&
  getAddOnCarriersForMarket(const LocCode& market1, const LocCode& market2, const DateTime& date);

  void loadFaresForMarket(const LocCode& market1,
                          const LocCode& market2,
                          const std::vector<CarrierCode>& cxr);

  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const DateTime& date);

  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const DateTime& startDate,
                                                          const DateTime& endDate);

  const std::vector<const FareInfo*>& getBoundFaresByMarketCxr(const LocCode& market1,
                                                               const LocCode& market2,
                                                               const CarrierCode& cxr,
                                                               const DateTime& startDate,
                                                               const DateTime& endDate);

  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const VendorCode& vendor,
                                                          const DateTime& ticketDate);

  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const VendorCode& vendor);

  const std::vector<const FareInfo*>& getBoundFaresByMarketCxr(const LocCode& market1,
                                                               const LocCode& market2,
                                                               const CarrierCode& cxr,
                                                               const DateTime& date);

  const std::vector<double>* getParameterBeta(const int& timeDiff,
                                              const int& mileage,
                                              const char& direction,
                                              const char& APSatInd);

  bool isRuleInFareMarket(const LocCode& market1,
                          const LocCode& market2,
                          const CarrierCode& cxr,
                          const RuleNumber ruleNumber);

  const std::vector<const FareClassAppInfo*>& getFareClassApp(const VendorCode& vendor,
                                                              const CarrierCode& carrier,
                                                              const TariffNumber& ruleTariff,
                                                              const RuleNumber& ruleNumber,
                                                              const FareClassCode& fareClass);

  const std::vector<const FareClassAppInfo*>& getFareClassAppByTravelDT(const VendorCode& vendor,
                                                              const CarrierCode& carrier,
                                                              const TariffNumber& ruleTariff,
                                                              const RuleNumber& ruleNumber,
                                                              const FareClassCode& fareClass,
                                                              DateTime travelDate);

  const std::vector<TariffCrossRefInfo*>& getTariffXRef(const VendorCode& vendor,
                                                        const CarrierCode& carrier,
                                                        const RecordScope& crossRefType);

  const std::vector<TariffCrossRefInfo*>& getTariffXRefByFareTariff(const VendorCode& vendor,
                                                                    const CarrierCode& carrier,
                                                                    const RecordScope& crossRefType,
                                                                    const TariffNumber& fareTariff,
                                                                    const DateTime& date);

  const std::vector<TariffCrossRefInfo*>& getTariffXRefByRuleTariff(const VendorCode& vendor,
                                                                    const CarrierCode& carrier,
                                                                    const RecordScope& crossRefType,
                                                                    const TariffNumber& ruleTariff,
                                                                    const DateTime& date);

  const std::vector<TariffCrossRefInfo*>&
  getTariffXRefByGenRuleTariff(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const RecordScope& crossRefType,
                               const TariffNumber& ruleTariff,
                               const DateTime& date);

  const std::vector<TariffCrossRefInfo*>&
  getTariffXRefByAddonTariff(const VendorCode& vendor,
                             const CarrierCode& carrier,
                             const RecordScope& crossRefType,
                             const TariffNumber& addonTariff,
                             const DateTime& date = DateTime::emptyDate());

  const Indicator getTariffInhibit(const VendorCode& vendor,
                                   const Indicator tariffCrossRefType,
                                   const CarrierCode& carrier,
                                   const TariffNumber& fareTariff,
                                   const TariffCode& ruleTariffCode);

  const std::vector<CarrierCode>& getCarriersForMarket(const LocCode& market1,
                                                       const LocCode& market2,
                                                       bool includeAddon,
                                                       const DateTime& date);

  // equivalent to getNUC(...).front(), but more efficient if
  // you only want the first NUCInfo
  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date);

  const Loc* getLoc(const LocCode& locCode, const DateTime& date);

  const TaxNation* getTaxNation(const NationCode& nation, const DateTime& date);

  const std::vector<const TaxReportingRecordInfo*>
  getTaxReportingRecord(const VendorCode& vendor,
                        const NationCode& nation,
                        const CarrierCode& taxCarrier,
                        const TaxCode& taxCode,
                        const TaxType& taxType,
                        const DateTime& date);

  const std::vector<const TaxReportingRecordInfo*>
  getAllTaxReportingRecords(const TaxCode& taxCode);

  const std::vector<const TaxRulesRecord*>&
  getTaxRulesRecord(const NationCode& nation, const Indicator& taxPointTag, const DateTime& date);

  const std::vector<const TaxRulesRecord*>&
  getTaxRulesRecordByCode(const TaxCode& taxCode, const DateTime& date);

  // DEPRECATED
  // Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
  const std::vector<const TaxRulesRecord*>&
  getTaxRulesRecordByCodeAndType(const TaxCode& taxCode, const TaxType& taxType, const DateTime& date);

  const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& taxCode, const DateTime& date);

  const std::vector<TaxReissue*>& getTaxReissue(const TaxCode& taxCode, const DateTime& date);

  const std::vector<TaxExemption*>& getTaxExemption(const TaxCode& taxCode,
      const PseudoCityCode& channelId,
      const DateTime& date);

  const std::vector<ATPResNationZones*>& getATPResNationZones(const NationCode& key);

  const std::vector<BankerSellRate*>&
  getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date);

  FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
  getBankerSellRateRange(const CurrencyCode& primeCur,
                         const CurrencyCode& cur,
                         const DateTime& date);

  const std::vector<Nation*>& getAllNation(const DateTime& date);

  const Nation* getNation(const NationCode& nationCode, const DateTime& date);

  const Currency* getCurrency(const CurrencyCode& currency);

  const std::vector<CurrencySelection*>&
  getCurrencySelection(const NationCode& nation, const DateTime& date);

  const FareTypeMatrix* getFareTypeMatrix(const FareType& key, const DateTime& date);

  const std::vector<FareTypeMatrix*>& getAllFareTypeMatrix(const DateTime& date);

  const std::vector<CombinabilityRuleInfo*>& getCombinabilityRule(const VendorCode& vendor,
                                                                  const CarrierCode& carrier,
                                                                  const TariffNumber& ruleTariff,
                                                                  const RuleNumber& rule,
                                                                  const DateTime& tvlDate);

  const std::vector<GeneralFareRuleInfo*>& getAllGeneralFareRule(const VendorCode vendor,
                                                                 const CarrierCode carrier,
                                                                 const TariffNumber ruleTariff,
                                                                 const RuleNumber rule,
                                                                 const CatNumber category);

  const std::vector<GeneralFareRuleInfo*>&
  getGeneralFareRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& rule,
                     const CatNumber& category,
                     const DateTime& date,
                     const DateTime& applDate = DateTime::emptyDate());

  const DateTime& getVoluntaryChangesConfig(const CarrierCode& carrier,
                                            const DateTime& currentTktDate,
                                            const DateTime& originalTktIssueDate);

  const DateTime& getVoluntaryRefundsConfig(const CarrierCode& carrier,
                                            const DateTime& currentTktDate,
                                            const DateTime& originalTktIssueDate);

  const GlobalDir* getGlobalDir(const GlobalDirection& globalDir, const DateTime& date);

  const std::vector<GlobalDirSeg*>& getGlobalDirSeg(const DateTime& date);

  const std::vector<MinFareRuleLevelExcl*>&
  getMinFareRuleLevelExcl(const VendorCode& vendor,
                          int textTblItemNo,
                          const CarrierCode& governingCarrier,
                          const TariffNumber& rule,
                          const DateTime& date);

  const std::vector<MinFareAppl*>& getMinFareAppl(const VendorCode& textTblVendor,
                                                  int textTblItemNo,
                                                  const CarrierCode& governingCarrier,
                                                  const DateTime& date);

  const std::vector<MinFareDefaultLogic*>&
  getMinFareDefaultLogic(const VendorCode& vendor, const CarrierCode& carrier);

  const std::vector<CopMinimum*>& getCopMinimum(const NationCode& key, const DateTime& date);

  const CopParticipatingNation* getCopParticipatingNation(const NationCode& nation,
                                                          const NationCode& copNation,
                                                          const DateTime& date);

  const std::vector<LimitationFare*>& getFCLimitation(const DateTime& date);

  const std::vector<LimitationJrny*>& getJLimitation(const DateTime& date);

  const std::vector<Mileage*>& getMileage(const LocCode& origin,
                                          const LocCode& destination,
                                          const DateTime& dateTime,
                                          Indicator mileageType = 'T');

  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& destination,
                            Indicator mileageType,
                            const GlobalDirection globalDirection,
                            const DateTime& dateTime);

  const OpenJawRule* getOpenJawRule(const VendorCode& vendor, const int itemNo);

  const BookingCodeExceptionSequenceList& getBookingCodeExceptionSequence(const VendorCode& vendor,
                                                                          const int itemNo,
                                                                          bool filterExpireDate);

  const BookingCodeExceptionSequenceList&
  getBookingCodeExceptionSequence(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  Indicator conventionNo,
                                  const DateTime& date,
                                  bool& isRuleZero,
                                  bool filterExpireDate);

  const std::vector<const PaxTypeCodeInfo*> getPaxTypeCode(const VendorCode& vendor, int itemNo);

  const std::vector<const TaxCodeTextInfo*> getTaxCodeText(const VendorCode& vendor, int itemNo);

  const PaxTypeInfo* getPaxType(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);

  const std::vector<PaxTypeInfo*>& getAllPaxType();

  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& paxTypeCode);

  const EndOnEnd* getEndOnEnd(const VendorCode& vendor, const int itemNo);

  const std::vector<CarrierCombination*>&
  getCarrierCombination(const VendorCode& vendor, const int itemNo);

  const std::vector<CarrierMixedClass*>&
  getCarrierMixedClass(const CarrierCode& carrier, const DateTime& date);

  bool corpIdExists(const std::string& corpId, const DateTime& tvlDate);

  const std::vector<tse::CorpId*>&
  getCorpId(const std::string& corpId, const CarrierCode& carrier, const DateTime& tvlDate);

  const std::vector<tse::FareByRuleApp*>& getFareByRuleApp(const CarrierCode& carrier,
                                                           const std::string& corpId,
                                                           const AccountCode& accountCode,
                                                           const TktDesignator& tktDesignator,
                                                           const DateTime& tvlDate,
                                                           std::vector<PaxTypeCode>& paxTypes);

  const FlightAppRule* getFlightAppRule(const VendorCode& vendor, int itemNo);

  const std::vector<DateOverrideRuleItem*>&
  getDateOverrideRuleItem(const VendorCode& vendor,
                          int itemNumber,
                          const DateTime& applDate = DateTime::emptyDate());

  const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemNumber);

  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date);

  // Validate Nation or State in Area, SubArea or Zone
  bool isNationInArea(const NationCode& nation, const LocCode& area);

  bool isNationInSubArea(const NationCode& nation, const LocCode& subArea);

  bool isNationInZone(const VendorCode& vendor, int zone, char zoneType, const NationCode& nation);

  bool isStateInArea(const std::string& nationState, const LocCode& area);

  bool isStateInSubArea(const std::string& nationState, const LocCode& subArea);

  bool
  isStateInZone(const VendorCode& vendor, int zone, char zoneType, const std::string& nationState);

  // Get VendorType from VendorCrossRef table
  char getVendorType(const VendorCode& vendor);

  const std::vector<Routing*>& getRouting(const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& routingTariff,
                                          const RoutingNumber& routingNumber,
                                          const DateTime& date);

  const MarketRoutingInfo& getMarketRoutingInfo(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const RoutingNumber& routing,
                                                const TariffNumber& routingTariff,
                                                const LocCode& market1,
                                                const LocCode& market2,
                                                const bool getSingles,
                                                const bool getDoubles);

  const RoundTripRuleItem* getRoundTripRuleItem(const VendorCode& vendor, int itemNo);

  const CircleTripRuleItem* getCircleTripRuleItem(const VendorCode& vendor, int itemNo);

  const std::vector<FareClassRestRule*>& getFareClassRestRule(const VendorCode& vendor, int itemNo);

  const std::vector<OpenJawRestriction*>&
  getOpenJawRestriction(const VendorCode& vendor, int itemNo);

  const std::vector<TariffRuleRest*>& getTariffRuleRest(const VendorCode& vendor, int itemNo);

  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date);

  const std::vector<const JointCarrier*>&
  getJointCarrier(const VendorCode& vendor, int itemNo, const DateTime& date);

  const std::vector<const SamePoint*>&
  getSamePoint(const VendorCode& vendor, int itemNo, const DateTime& date);

  const std::vector<const BaseFareRule*>&
  getBaseFareRule(const VendorCode& vendor, int itemNo, const DateTime& date);

  const EligibilityInfo* getEligibility(const VendorCode& vendor, int itemNo);

  const SeasonalAppl* getSeasonalAppl(const VendorCode& vendor, int itemNo);

  const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city);

  const std::vector<MultiAirportCity*>& getMultiCityAirport(const LocCode& locCode);

  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate);

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate);

  const std::vector<MultiTransport*>& getMultiTransportLocs(const LocCode& city,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate);

  const LocCode getMultiTransportCity(const LocCode& locCode);

  GeneralRuleApp* getGeneralRuleApp(const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& tariffNumber,
                                    const RuleNumber& ruleNumber,
                                    CatNumber catNum);

  // a more efficient variation of getGeneralRuleApp that
  // just gets the rule number and tariff number of the
  // first matching GeneralRuleApp
  bool getGeneralRuleAppTariffRule(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& tariffNumber,
                                   const RuleNumber& ruleNumber,
                                   CatNumber catNum,
                                   RuleNumber& ruleNumOut,
                                   TariffNumber& tariffNumOut);

  const std::vector<GeneralRuleApp*>& getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                                                                 const CarrierCode& carrier,
                                                                 const TariffNumber& tariffNumber,
                                                                 const RuleNumber& ruleNumber,
                                                                 DateTime tvlDate);

  GeneralRuleApp* getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const TariffNumber& tariffNumber,
                                             const RuleNumber& ruleNumber,
                                             CatNumber catNum,
                                             DateTime tvlDate);

    // a more efficient variation of getGeneralRuleApp that
    // just gets the rule number and tariff number of the
    // first matching GeneralRuleApp
    bool getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const TariffNumber& tariffNumber,
                                              const RuleNumber& ruleNumber,
                                              CatNumber catNum,
                                              RuleNumber& ruleNumOut,
                                              TariffNumber& tariffNumOut,
                                              DateTime tvlDate);

    const std::vector<FareByRuleCtrlInfo*>& getAllFareByRuleCtrl(const VendorCode& vendor,
                                                                 const CarrierCode& carrier,
                                                                 const TariffNumber& tariffNumber,
                                                                 const RuleNumber& ruleNumber);
  const std::vector<FareByRuleCtrlInfo*>& getFareByRuleCtrl(const VendorCode& vendor,
                                                            const CarrierCode& carrier,
                                                            const TariffNumber& tariffNumber,
                                                            const RuleNumber& ruleNumber,
                                                            const DateTime& tvlDate);

  const FareByRuleItemInfo* getFareByRuleItem(const VendorCode& vendor, const int itemNo);

  const FltTrkCntryGrp* getFltTrkCntryGrp(const CarrierCode& carrier, const DateTime& date);

  const DiscountInfo* getDiscount(const VendorCode& vendor, int itemNo, int category);

  const std::vector<const IndustryPricingAppl*>&
  getIndustryPricingAppl(const CarrierCode& carrier,
                         const GlobalDirection& globalDir,
                         const DateTime& date);

  const std::vector<const IndustryFareAppl*>&
  getIndustryFareAppl(Indicator selectionType, const CarrierCode& carrier, const DateTime& date);

  bool isMultilateral(const VendorCode& vendor,
                      const RuleNumber& rule,
                      const LocCode& loc1,
                      const LocCode& loc2,
                      const DateTime& date);

  const std::vector<const IndustryFareBasisMod*>&
  getIndustryFareBasisMod(const CarrierCode& carrier,
                          Indicator userApplType,
                          const UserApplCode& userAppl,
                          const DateTime& date);

  const std::vector<FootNoteCtrlInfo*>& getAllFootNoteCtrl(const VendorCode vendor,
                                                           const CarrierCode carrier,
                                                           const TariffNumber tariffNumber,
                                                           const Footnote footnote,
                                                           int category);

  const std::vector<FootNoteCtrlInfo*>& getFootNoteCtrl(const VendorCode& vendor,
                                                        const CarrierCode& carrier,
                                                        const TariffNumber& fareTariff,
                                                        const Footnote& footnote,
                                                        const int category,
                                                        const DateTime& date);

  const std::vector<AddonZoneInfo*>& getAddOnZone(const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const LocCode& market,
                                                  const DateTime& date = DateTime::emptyDate());

  const std::vector<AddonZoneInfo*>& getAddOnZone(const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& fareTariff,
                                                  const AddonZone& zone,
                                                  const DateTime& date);

  const TSIInfo* getTSI(int key);

  const AddonFareClassCombMultiMap& getAddOnCombFareClass(const VendorCode& vendor,
                                                          const TariffNumber& tariff,
                                                          const CarrierCode& carrier,
                                                          const DateTime& ticketDate);

  const AddonFareClassCombMultiMap& getAddOnCombFareClass(const VendorCode& vendor,
                                                          const TariffNumber& tariff,
                                                          const CarrierCode& carrier);

  const std::vector<AddonCombFareClassInfo*>&
  getAddOnCombFareClassHistorical(const VendorCode& vendor,
                                  TariffNumber fareTariff,
                                  const CarrierCode& carrier);

  const std::vector<AddonFareInfo*>& getAddOnFare(const LocCode& key,
                                                  const CarrierCode& carrier,
                                                  const DateTime& date = DateTime::emptyDate());

  const std::vector<AddonFareInfo*>& getAddOnFare(const LocCode& gatewayMarket,
                                                  const LocCode& interiorMarket,
                                                  const CarrierCode& carrier,
                                                  const RecordScope& crossRefType,
                                                  const DateTime& date);

  const std::vector<Differentials*>& getDifferentials(const CarrierCode& key, const DateTime& date);

  const DayTimeAppInfo* getDayTimeAppInfo(const VendorCode& vendor, int itemNo);

  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity);

  const std::vector<NoPNROptions*>&
  getNoPNROptions(const Indicator& userApplType, const UserApplCode& userAppl);

  const NoPNRFareTypeGroup* getNoPNRFareTypeGroup(const int fareTypeGroup);

  const NegFareRest* getNegFareRest(const VendorCode& vendor, int itemNo);

  const NegFareRestExt* getNegFareRestExt(const VendorCode& vendor, int itemNo);

  const std::vector<NegFareRestExtSeq*>& getNegFareRestExtSeq(const VendorCode& vendor, int itemNo);

  const MaxStayRestriction* getMaxStayRestriction(const VendorCode& vendor, int itemNo);

  const MinStayRestriction* getMinStayRestriction(const VendorCode& vendor, int itemNo);

  const SalesRestriction* getSalesRestriction(const VendorCode& vendor, int itemNo);

  const TravelRestriction* getTravelRestriction(const VendorCode& vendor, int itemNo);

  const Groups* getGroups(const VendorCode& vendor, int itemNo);

  const Tours* getTours(const VendorCode& vendor, int itemNo);

  const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                        const CarrierCode& carrier,
                                        Indicator area1,
                                        Indicator area2,
                                        const DateTime& date);

  const SurfaceSectorExempt*
  getSurfaceSectorExempt(const LocCode& origLoc, const LocCode& destLoc, const DateTime& date);

  const MileageSubstitution* getMileageSubstitution(const LocCode& key, const DateTime& date);

  const TariffMileageAddon* getTariffMileageAddon(const CarrierCode& carrier,
                                                  const LocCode& unpublishedAddonLoc,
                                                  const GlobalDirection& globalDir,
                                                  const DateTime& date);

  const PfcPFC* getPfcPFC(const LocCode& key, const DateTime& date);

  const std::vector<PfcPFC*>& getAllPfcPFC();

  const std::vector<PfcEquipTypeExempt*>& getAllPfcEquipTypeExemptData();

  const PfcMultiAirport* getPfcMultiAirport(const LocCode& key, const DateTime& date);

  const std::vector<PfcMultiAirport*>& getAllPfcMultiAirport();

  const std::vector<PfcEssAirSvc*>&
  getPfcEssAirSvc(const LocCode& easHubArpt, const LocCode& easArpt, const DateTime& date);

  const std::vector<PfcEssAirSvc*>& getAllPfcEssAirSvc();

  const std::vector<PfcCollectMeth*>&
  getPfcCollectMeth(const CarrierCode& carrier, const DateTime& date);

  const std::vector<PfcCollectMeth*>& getAllPfcCollectMeth(const DateTime& date);

  const std::vector<PfcAbsorb*>&
  getPfcAbsorb(const LocCode& pfcAirport, const CarrierCode& localCarrier, const DateTime& date);

  const std::vector<PfcAbsorb*>& getAllPfcAbsorb();

  const PfcEquipTypeExempt*
  getPfcEquipTypeExempt(const EquipmentType& equip, const StateCode& state, const DateTime& date);

  const std::vector<const PfcTktDesigExcept*>&
  getPfcTktDesigExcept(const CarrierCode& carrier, const DateTime& date);

  const std::vector<SurfaceTransfersInfo*>&
  getSurfaceTransfers(const VendorCode& vendor, int itemNo);

  const CircleTripProvision*
  getCircleTripProvision(const LocCode& market1, const LocCode& market2, const DateTime& date);

  const MinFareFareTypeGrp* getMinFareFareTypeGrp(const std::string& key, const DateTime& date);

  const std::vector<SalesNationRestr*>&
  getSalesNationRestr(const NationCode& key, const DateTime& date);

  const std::vector<CountrySettlementPlanInfo*>&
  getCountrySettlementPlans(const NationCode& countryCode);

  const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor,
                        int itemNo,
                        const DateTime& applDate = DateTime::emptyDate());

  const std::vector<MileageSurchExcept*>& getMileageSurchExcept(const VendorCode& vendor,
                                                                int textTblItemNo,
                                                                const CarrierCode& governingCarrier,
                                                                const TariffNumber& ruleTariff,
                                                                const RuleNumber& rule,
                                                                const DateTime& date);

  const CarrierFlight* getCarrierFlight(const VendorCode& vendor, int itemNo);

  const std::vector<TaxAkHiFactor*>& getTaxAkHiFactor(const LocCode& key, const DateTime& date);

  const std::vector<TaxSegAbsorb*>&
  getTaxSegAbsorb(const CarrierCode& carrier, const DateTime& date);

  const std::vector<NegFareSecurityInfo*>& getNegFareSecurity(const VendorCode& vendor, int itemNo);

  const std::vector<NegFareCalcInfo*>& getNegFareCalc(const VendorCode& vendor, int itemNo);

  const std::vector<const SectorDetailInfo*> getSectorDetail(const VendorCode& vendor, int itemNo);

  const std::vector<SectorSurcharge*>&
  getSectorSurcharge(const CarrierCode& key, const DateTime& date);

  const std::vector<CustomerActivationControl*>&
  getCustomerActivationControl(const std::string& projectCode);

  const std::vector<MarkupControl*>&
  getMarkupBySecondSellerId(const PseudoCityCode& pcc,
                            const PseudoCityCode& homePCC,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& ruleTariff,
                            const RuleNumber& rule,
                            int seqNo,
                            long secondarySellerId,
                            const DateTime& date = DateTime::emptyDate());

  const std::vector<MarkupControl*>&
  getMarkupBySecurityItemNo(const PseudoCityCode& pcc,
                            const PseudoCityCode& homePCC,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& ruleTariff,
                            const RuleNumber& rule,
                            int seqNo,
                            const DateTime& date = DateTime::emptyDate());

  const std::vector<MarkupControl*>& getMarkupByPcc(const PseudoCityCode& pcc,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& ruleTariff,
                                                    const RuleNumber& rule,
                                                    int seqNo,
                                                    const DateTime& date);

  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key);

  const std::vector<CommissionCap*>&
  getCommissionCap(const CarrierCode& carrier, const CurrencyCode& cur, const DateTime& date);

  const std::vector<MarkupSecFilter*>& getMarkupSecFilter(const VendorCode& vendor,
                                                          const CarrierCode& carrier,
                                                          const TariffNumber& ruleTariff,
                                                          const RuleNumber& rule);

  const std::vector<DBEGlobalClass*>& getDBEGlobalClass(const DBEClass& key);

  const std::vector<TicketStock*>& getTicketStock(int key, const DateTime& date);

  const std::vector<AddonZoneInfo*>&
  getAddonZoneSITA(const LocCode& loc, const VendorCode& vendor, const CarrierCode& carrier);

  const RuleItemInfo* getRuleItemInfo(const CategoryRuleInfo* rule,
                                      const CategoryRuleItemInfo* item,
                                      const DateTime& applDate = DateTime::emptyDate());

  const std::vector<RoutingKeyInfo*>&
  getRoutingForMarket(const LocCode& market1, const LocCode& market2, const CarrierCode& carrier);

  const std::vector<FareDisplayPref*>& getFareDisplayPref(const Indicator& userApplType,
                                                          const UserApplCode& userAppl,
                                                          const Indicator& pseudoCityType,
                                                          const PseudoCityCode& pseudoCity,
                                                          const TJRGroup& tjrGroup);

  const std::vector<FareDisplayPrefSeg*>& getFareDisplayPrefSeg(const Indicator& userApplType,
                                                                const UserApplCode& userAppl,
                                                                const Indicator& pseudoCityType,
                                                                const PseudoCityCode& pseudoCity,
                                                                const TJRGroup& tjrGroup);

  const std::vector<FareDisplayInclCd*>& getFareDisplayInclCd(const Indicator& userApplType,
                                                              const UserApplCode& userAppl,
                                                              const Indicator& pseudoCityType,
                                                              const PseudoCityCode& pseudoCity,
                                                              const InclusionCode& inclusionCode);

  const std::vector<FareDispInclRuleTrf*>&
  getFareDispInclRuleTrf(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const Indicator& pseudoCityType,
                         const PseudoCityCode& pseudoCity,
                         const InclusionCode& inclusionCode);

  const std::vector<FareDispInclFareType*>&
  getFareDispInclFareType(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const InclusionCode& inclusionCode);

  const std::vector<FareDispInclDsplType*>&
  getFareDispInclDsplType(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const InclusionCode& inclusionCode);

  const std::vector<FareDisplayWeb*>& getFareDisplayWebForCxr(const CarrierCode& carrier);

  const std::vector<FareDisplayWeb*>& getFareDisplayWeb(const Indicator& dispInd,
                                                        const VendorCode& vendor,
                                                        const CarrierCode& carrier,
                                                        const TariffNumber& ruleTariff,
                                                        const RuleNumber& rule,
                                                        const PaxTypeCode& paxTypeCode);

  const std::set<std::pair<PaxTypeCode, VendorCode> >&
  getFareDisplayWebPaxForCxr(const CarrierCode& carrier);

  const RuleCategoryDescInfo* getRuleCategoryDesc(const CatNumber& key);

  const ZoneInfo*
  getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date);

  const ZoneInfo*
  getZoneFareFocusGroup(const VendorCode& vendor,
                        const Zone& zone,
                        Indicator zoneType,
                        const DateTime& date,
                        bool fareFocusGroup);

  const std::vector<FareDisplaySort*>& getFareDisplaySort(const Indicator& userApplType,
                                                          const UserApplCode& userAppl,
                                                          const Indicator& pseudoCityType,
                                                          const PseudoCityCode& pseudoCity,
                                                          const TJRGroup& tjrGroup,
                                                          const DateTime& travelDate);

  const std::vector<FDSPsgType*>& getFDSPsgType(const Indicator& userApplType,
                                                const UserApplCode& userAppl,
                                                const Indicator& pseudoCityType,
                                                const PseudoCityCode& pseudoCity,
                                                const TJRGroup& tjrGroup,
                                                const Indicator& fareDisplayType,
                                                const Indicator& domIntlAppl,
                                                const uint64_t& seqno);

  const std::vector<FDSGlobalDir*>& getFDSGlobalDir(const Indicator& userApplType,
                                                    const UserApplCode& userAppl,
                                                    const Indicator& pseudoCityType,
                                                    const PseudoCityCode& pseudoCity,
                                                    const TJRGroup& tjrGroup,
                                                    const Indicator& fareDisplayType,
                                                    const Indicator& domIntlAppl,
                                                    const DateTime& versionDate,
                                                    const uint64_t& seqno,
                                                    const DateTime& createDate);

  const std::vector<FDSFareBasisComb*>& getFDSFareBasisComb(const Indicator& userApplType,
                                                            const UserApplCode& userAppl,
                                                            const Indicator& pseudoCityType,
                                                            const PseudoCityCode& pseudoCity,
                                                            const TJRGroup& tjrGroup,
                                                            const Indicator& fareDisplayType,
                                                            const Indicator& domIntlAppl,
                                                            const uint64_t& seqno);

  const std::vector<FDSSorting*>& getFDSSorting(const Indicator& userApplType,
                                                const UserApplCode& userAppl,
                                                const Indicator& pseudoCityType,
                                                const PseudoCityCode& pseudoCity,
                                                const TJRGroup& tjrGroup,
                                                const Indicator& fareDisplayType,
                                                const Indicator& domIntlAppl,
                                                const uint64_t& seqno);

  const std::vector<BrandedFareApp*>& getBrandedFareApp(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const CarrierCode& carrier,
                                                        const DateTime& travelDate);

  const std::vector<Brand*>& getBrands(const Indicator& userApplType,
                                       const UserApplCode& userAppl,
                                       const CarrierCode& carrier,
                                       const DateTime& travelDate);

  const Brand* getBrand(const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const CarrierCode& carrier,
                        const BrandCode& brandId,
                        const DateTime& travelDate);

  std::vector<const FreqFlyerStatusSeg*>
  getFreqFlyerStatusSegs(const CarrierCode carrier, const DateTime& date);

  std::vector<const FreqFlyerStatus*> getFreqFlyerStatuses(const CarrierCode carrier,
                                                           const DateTime& date = DateTime(),
                                                           bool useHistorical = false);

  const std::vector<BrandedCarrier*>& getBrandedCarriers();

  const std::vector<BrandedFare*>&
  getBrandedFare(const VendorCode& vendor, const CarrierCode& carrier);

  const std::vector<BrandedCarrier*>& getS8BrandedCarriers();

  const std::vector<SvcFeesFareIdInfo*>&
  getSvcFeesFareIds(const VendorCode& vendor, long long itemNo);

  const std::vector<SvcFeesFeatureInfo*>&
  getSvcFeesFeature(const VendorCode& vendor, long long itemNo);

  const std::vector<RuleCatAlphaCode*>& getRuleCatAlphaCode(const AlphaCode& key);

  const std::vector<FareDispRec1PsgType*>&
  getFareDispRec1PsgType(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const Indicator& pseudoCityType,
                         const PseudoCityCode& pseudoCity,
                         const InclusionCode& inclusionCode);

  const std::vector<FareDispRec8PsgType*>&
  getFareDispRec8PsgType(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const Indicator& pseudoCityType,
                         const PseudoCityCode& pseudoCity,
                         const InclusionCode& inclusionCode);

  const std::vector<FareDispTemplate*>&
  getFareDispTemplate(const int& templateID, const Indicator& templateType);

  const std::vector<FareDispTemplateSeg*>&
  getFareDispTemplateSeg(const int& templateID, const Indicator& templateType);

  const std::vector<FareDispCldInfPsgType*>&
  getFareDispCldInfPsgType(const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const InclusionCode& inclusionCode,
                           const Indicator& psgTypeInd);

  const FareProperties* getFareProperties(const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariff,
                                          const RuleNumber& rule);

  const PrintOption*
  getPrintOption(const VendorCode& vendor, const std::string& fareSource, const DateTime& date);

  const ValueCodeAlgorithm* getValueCodeAlgorithm(const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const std::string& name,
                                                  const DateTime& date);

  const std::vector<YQYRFeesNonConcur*>&
  getYQYRFeesNonConcur(const CarrierCode& carrier, const DateTime& date);

  const std::vector<YQYRFees*>& getYQYRFees(const CarrierCode& key);

  const std::vector<MarriedCabin*>& getMarriedCabins(const CarrierCode& carrier,
                                                     const BookingCode& premiumCabin,
                                                     const DateTime& versionDate);

  const std::vector<ContractPreference*>& getContractPreferences(const PseudoCityCode& pseudoCity,
                                                                 const CarrierCode& carrier,
                                                                 const RuleNumber& rule,
                                                                 const DateTime& versionDate);

  const FareCalcConfigText& getMsgText(const Indicator userApplType,
                                       const UserApplCode& userAppl,
                                       const PseudoCityCode& pseudoCity);

  const std::vector<FareTypeQualifier*>& getFareTypeQualifier(const Indicator& userApplType,
                                                              const UserApplCode& userAppl,
                                                              const FareType& qualifier);

  const std::vector<ReissueSequence*>& getReissue(const VendorCode& vendor,
                                                  int itemNo,
                                                  const DateTime& date,
                                                  const DateTime& applDate = DateTime::emptyDate());

  const std::vector<Waiver*>& getWaiver(const VendorCode& vendor,
                                        int itemNo,
                                        const DateTime& date,
                                        const DateTime& applDate = DateTime::emptyDate());

  const std::vector<FareTypeTable*>&
  getFareTypeTable(const VendorCode& vendor,
                   int itemNo,
                   const DateTime& ticketDate,
                   const DateTime& applDate = DateTime::emptyDate());

  const SeasonalityDOW* getSeasonalityDOW(const VendorCode& vendor,
                                          int itemNo,
                                          const DateTime& ticketDate,
                                          const DateTime& applDate = DateTime::emptyDate());

  // gets records from given date to given date (default is +infinity)
  // including historical entries, if date is in past
  const std::vector<NUCInfo*>&
  getNUCAllCarriers(const CurrencyCode& currency,
                    const DateTime& from_date,
                    const DateTime& to_date = boost::date_time::pos_infin);

  const std::vector<TicketingFeesInfo*>& getTicketingFees(const VendorCode& vendor,
                                                          const CarrierCode& validatingCarrier,
                                                          const DateTime& date);

  const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const ServiceTypeCode& serviceTypeCode,
                                              const ServiceGroup& serviceGroup,
                                              const DateTime& date);

  const std::vector<OptionalServicesConcur*>&
  getOptionalServicesConcur(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const ServiceTypeCode& serviceTypeCode,
                            const DateTime& date);

  const std::vector<OptionalServicesInfo*>&
  getOptionalServicesInfo(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const ServiceTypeCode& serviceTypeCode,
                          const ServiceSubTypeCode& serviceSubTypeCode,
                          Indicator fltTktMerchInd,
                          const DateTime& date);

  const std::vector<OptionalServicesInfo*>&
  getOptionalServicesMktInfo(const VendorCode& vendor,
                             const CarrierCode& carrier,
                             const LocCode& loc1,
                             const LocCode& loc2,
                             const ServiceTypeCode& serviceTypeCode,
                             const ServiceSubTypeCode& serviceSubTypeCode,
                             Indicator fltTktMerchInd,
                             const DateTime& date);

  const std::vector<const ServiceBaggageInfo*>
  getServiceBaggage(const VendorCode& vendor, int itemNo);

  const ServicesDescription* getServicesDescription(const ServiceGroupDescription& value);

  const ServicesSubGroup*
  getServicesSubGroup(const ServiceGroup& serviceGroup, const ServiceGroup& serviceSubGroup);

  const std::vector<MerchActivationInfo*>&
  getMerchActivation(uint64_t productId,
                     const CarrierCode& carrier,
                     const PseudoCityCode& pseudoCity = EMPTY_STRING(),
                     const DateTime& date = DateTime::emptyDate());

  const MerchCarrierPreferenceInfo*
  getMerchCarrierPreference(const CarrierCode& carrier, const ServiceGroup& groupCode);

  const std::vector<SvcFeesSecurityInfo*>&
  getSvcFeesSecurity(const VendorCode& vendor, const int itemNo);

  const std::vector<SvcFeesAccCodeInfo*>&
  getSvcFeesAccountCode(const VendorCode& vendor, const int itemNo);

  const std::vector<SvcFeesCurrencyInfo*>&
  getSvcFeesCurrency(const VendorCode& vendor, const int itemNo);

  const std::vector<SvcFeesCxrResultingFCLInfo*>&
  getSvcFeesCxrResultingFCL(const VendorCode& vendor, const int itemNo);

  const std::vector<SvcFeesResBkgDesigInfo*>&
  getSvcFeesResBkgDesig(const VendorCode& vendor, const int itemNo);

  const std::vector<SvcFeesTktDesignatorInfo*>&
  getSvcFeesTicketDesignator(const VendorCode& vendor, const int itemNo);

  bool isCxrActivatedForServiceFee(const CarrierCode& validatingCarrier, const DateTime& date);

  const std::vector<ServiceGroupInfo*>& getAllServiceGroups();

  const std::vector<OptionalServicesActivationInfo*>&
  getOptServiceActivation(Indicator crs,
                          const UserApplCode& userCode,
                          const std::string& application);

  const TaxCarrierFlightInfo* getTaxCarrierFlight(const VendorCode& vendor, int itemNo);

  const TaxText* getTaxText(const VendorCode& vendor, int itemNo);

  const TaxCarrierAppl* getTaxCarrierAppl(const VendorCode& vendor, int itemNo);

  const std::vector<CurrencyCode>& getAllCurrencies();

  const TaxRestrictionLocationInfo*
  getTaxRestrictionLocation(const TaxRestrictionLocation& location);

  const std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name);

  bool isCarrierUSDot(const CarrierCode& carrier);

  bool isCarrierCTA(const CarrierCode& carrier);

  const DeleteList& deleteList() const { return _deleteList; }

  DeleteList& deleteList() { return _deleteList; }

  const std::vector<TPMExclusion*>& getTPMExclus(const CarrierCode& carrier);

  const std::vector<InterlineCarrierInfo*>&
  getInterlineCarrier();

  const std::vector<IntralineCarrierInfo*>&
  getIntralineCarrier();

  const std::vector<InterlineTicketCarrierInfo*>&
  getInterlineTicketCarrier(const CarrierCode& carrier, const DateTime& date);

  const InterlineTicketCarrierStatus*
  getInterlineTicketCarrierStatus(const CarrierCode& carrier, const CrsCode& crsCode);

  bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                              const DSTGrpCode& dstgrp2,
                              short& utcoffset,
                              const DateTime& dateTime1,
                              const DateTime& dateTime2);

  bool& useTLS() { return _useTLS; }

  const bool useTLS() const { return _useTLS; }

  const std::vector<NvbNvaInfo*>& getNvbNvaInfo(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const TariffNumber& tarrif,
                                                const RuleNumber& rule);

  const std::vector<SeatCabinCharacteristicInfo*>&
  getSeatCabinCharacteristicInfo(const CarrierCode& carrier,
                                 const Indicator& codeType,
                                 const DateTime& travelDate);

  const std::vector<Cat05OverrideCarrier*>& getCat05OverrideCarrierInfo(const PseudoCityCode& pcc);

  const std::vector<AirlineAllianceCarrierInfo*>&
  getAirlineAllianceCarrier(const CarrierCode& carrierCode);

  const std::vector<AirlineAllianceCarrierInfo*>&
  getGenericAllianceCarrier(const GenericAllianceCode& genericAllianceCode);

  const std::vector<AirlineAllianceContinentInfo*>&
  getAirlineAllianceContinent(const GenericAllianceCode& genericAllianceCode,
                              bool reduceTemporaryVectorsFallback = false);

  template <typename Func>
  void forEachAirlineAllianceContinent(const GenericAllianceCode& genericAllianceCode, Func&& func)
  {
    const auto& continents = getAirlineAllianceContinent(genericAllianceCode);
    const DateTime& ticketingDate = this->ticketDate();

    if (isHistorical())
    {
      auto it =
          std::find_if(continents.begin(),
                       continents.end(),
                       IsEffectiveHist<AirlineAllianceContinentInfo>(ticketingDate, ticketingDate));
      while (it != continents.end())
      {
        func(*it);
        it = std::find_if(it + 1,
                          continents.end(),
                          IsEffectiveH<AirlineAllianceContinentInfo>(ticketingDate, ticketingDate));
      }
    }
    else
    {
      forEachGeneral(continents,
                     std::forward<Func>(func),
                     IsEffectiveG<AirlineAllianceContinentInfo>(ticketingDate));
    }
  }

  const std::vector<TktDesignatorExemptInfo*>& getTktDesignatorExempt(const CarrierCode& carrier);

  const FareFocusRuleInfo* getFareFocusRule(uint64_t fareFocusRuleId,
                                            DateTime adjustedTicketDate);

  const FareFocusAccountCdInfo* getFareFocusAccountCd(uint64_t accountCdItemNo,
                                                      DateTime adjustedTicketDate);

  const FareFocusRoutingInfo* getFareFocusRouting(uint64_t routingItemNo,
                                                      DateTime adjustedTicketDate);

  const FareFocusLocationPairInfo* getFareFocusLocationPair(uint64_t locationPairItemNo,
                                                        DateTime adjustedTicketDate);

  const FareFocusDaytimeApplInfo* getFareFocusDaytimeAppl(uint64_t dayTimeApplItemNo,
                                                          DateTime createDate);

  const FareFocusDisplayCatTypeInfo* getFareFocusDisplayCatType(uint64_t displayCatTypeItemNo,
                                                            DateTime adjustedTicketDate);

  const std::vector<FareFocusBookingCodeInfo*>& getFareFocusBookingCode(uint64_t bookingCodeItemNo,
                                                                        DateTime adjustedTicketDate);

  const std::vector<FareFocusFareClassInfo*>& getFareFocusFareClass(uint64_t fareClassItemNo,
                                                                    DateTime adjustedTicketDate);

  const FareFocusLookupInfo* getFareFocusLookup(const PseudoCityCode& pcc);

  const FareFocusSecurityInfo* getFareFocusSecurity(uint64_t securityItemNo,
                                                    DateTime adjustedTicketDate);

  const FareRetailerRuleLookupInfo* getFareRetailerRuleLookup(Indicator applicationType,
                                                              const PseudoCityCode& sourcePcc,
                                                              const PseudoCityCode& pcc);

  const FareRetailerRuleInfo* getFareRetailerRule(uint64_t fareRetailerRuleId,
                                                  DateTime adjustedTicketDate);

  const FareRetailerResultingFareAttrInfo* getFareRetailerResultingFareAttr(uint64_t resultingFareAttrItemNo,
                                                                            DateTime adjustedTicketDate);

  const std::vector<FareRetailerCalcInfo*>& getFareRetailerCalc(uint64_t fareRetailerCalcItemNo,
                                                                DateTime adjustedTicketDate);

  const std::vector<FareFocusCarrierInfo*>& getFareFocusCarrier(uint64_t carrierItemNo,
                                                                DateTime adjustedTicketDate);

  const std::vector<FareFocusRuleCodeInfo*>& getFareFocusRuleCode(uint64_t ruleCdItemNo,
                                                                  DateTime adjustedTicketDate);

  const std::vector<CommissionContractInfo*>& getCommissionContract(const VendorCode& vendor,
                                                                    const CarrierCode& carrier,
                                                                    const PseudoCityCode& pcc);

  const std::vector<CommissionProgramInfo*>& getCommissionProgram(const VendorCode& vendor,
                                                                  uint64_t contractId);

  const std::vector<CommissionRuleInfo*>& getCommissionRule(const VendorCode& vendor,
                                                            uint64_t programId);

  const std::vector<CustomerSecurityHandshakeInfo*>&
    getCustomerSecurityHandshake(const PseudoCityCode& pcc,
                                 const Code<8>& productCD,
                                 const DateTime& dateTime);

  const std::vector<class RBDByCabinInfo*>&
    getRBDByCabin(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  DateTime tvlDate);

  const BankIdentificationInfo*
  getBankIdentification(const FopBinNumber& binNumber, const DateTime& date);

  const std::vector<GenSalesAgentInfo*>& getGenSalesAgents(const CrsCode& gds,
                                                           const NationCode& country,
                                                           const SettlementPlanType& settlementPlan,
                                                           const CarrierCode& validatingCxr);

  const std::vector<GenSalesAgentInfo*>& getGenSalesAgents(const CrsCode& gds,
                                                           const NationCode& country,
                                                           const SettlementPlanType& settlementPlan);

  const std::vector<AirlineCountrySettlementPlanInfo*>&
  getAirlineCountrySettlementPlans(const NationCode& country,
                                   const CrsCode& gds,
                                   const CarrierCode& airline,
                                   const SettlementPlanType& spType);

  const std::vector<AirlineCountrySettlementPlanInfo*>&
  getAirlineCountrySettlementPlans(const NationCode& country,
                                   const CrsCode& gds,
                                   const SettlementPlanType& spType);

  const std::vector<AirlineCountrySettlementPlanInfo*>&
  getAirlineCountrySettlementPlans(const CrsCode& gds,
                                   const NationCode& country,
                                   const CarrierCode& airline);

  const std::vector<AirlineInterlineAgreementInfo*>&
  getAirlineInterlineAgreements(const NationCode& country,
                                const CrsCode& gds,
                                const CarrierCode& validatingCarrier);

  const std::vector<NeutralValidatingAirlineInfo*>&
  getNeutralValidatingAirlines(const NationCode& country,
                               const CrsCode& gds,
                               const SettlementPlanType& spType);

  const std::vector<EmdInterlineAgreementInfo*>&
  getEmdInterlineAgreements(const NationCode& country,
                            const CrsCode& gds,
                            const CarrierCode& validatingCarrier);

  const FareFocusPsgTypeInfo*
  getFareFocusPsgType(uint64_t psgTypeItemNo, DateTime adjustedTicketDate);

  const std::vector<SpanishReferenceFareInfo*>&
  getSpanishReferenceFare(const CarrierCode& tktCarrier, const CarrierCode& fareCarrier,
                          const LocCode& sourceLoc, const LocCode& destLoc, const DateTime& date);

  const std::vector<MaxPermittedAmountFareInfo*>&
  getMaxPermittedAmountFare(const Loc& origin, const Loc& dest, const DateTime& date);

  static pthread_key_t& threadSpecificTicketDateKey()
  {
    return _threadSpecificTicketDateKey;
  }

  static pthread_key_t& threadSpecificIsHistoricalKey()
  {
    return _threadSpecificIsHistoricalKey;
  }

  static pthread_key_t initTicketDateKey();
  static pthread_key_t initIsHistoricalKey();
  static void destroyTicketDate(void* ptr);
  static void destroyIsHistorical(void* ptr);

private:
  DataHandle(const DataHandle& rhs);
  DataHandle& operator=(const DataHandle& rhs);

  DeleteList _deleteList;

  DataHandle* _parentDataHandle;
  DateTime _ticketDate;
  bool _isFareDisplay;
  bool _isPortExchange;
  bool _isHistorical;
  bool _useTLS; // use of Thread-local storage to save ticketing date for each tread
  int _trxId;
  DateTime _today;

  static pthread_key_t _threadSpecificTicketDateKey;
  static pthread_key_t _threadSpecificIsHistoricalKey;
};
} // namespace tse
