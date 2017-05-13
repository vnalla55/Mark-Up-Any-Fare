//----------------------------------------------------------------------------
//
//  File:           DAOInterface.h
//
//  Description:    Functions to connect DataHandle to the DAOs
//
//  Updates:
//
//  Copyright Sabre 2007
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DaoFilterIteratorUtils.h"
#include "DBAccess/Record2Types.h"

#include <utility>
#include <vector>

namespace tse
{

class AccompaniedTravelInfo;
class AddonCombFareClassInfo;
class AddonFareClassCombMultiMap;
class AddonFareInfo;
class AddonZoneInfo;
class AdvResTktInfo;
class AirlineAllianceCarrierInfo;
class AirlineAllianceContinentInfo;
class AirlineInterlineAgreementInfo;
class AirlineCountrySettlementPlanInfo;
class ATPResNationZones;
class BankerSellRate;
class BankIdentificationInfo;
class BaseFareRule;
class BlackoutInfo;
class BookingCodeConv;
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
class ContractPreference;
class CopMinimum;
class CopParticipatingNation;
class CorpId;
class CountrySettlementPlanInfo;
class Currency;
class CurrencySelection;
class Customer;
class CustomerActivationControl;
class DateOverrideRuleItem;
class DayTimeAppInfo;
class DateTime;
class DBEGlobalClass;
class DeleteList;
class Deposits;
class Differentials;
class DiscountInfo;
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
class FareInfo;
class FareProperties;
class FareTypeMatrix;
class FareTypeQualifier;
class FareTypeTable;
class FDHeaderMsg;
class FDSFareBasisComb;
class FDSGlobalDir;
class FDSPsgType;
class FDSSorting;
class FDSuppressFare;
class FlightAppRule;
class FltTrkCntryGrp;
class FootNoteCtrlInfo;
class FreqFlyerStatusSeg;
class GeneralFareRuleInfo;
class GeneralRuleApp;
class GenSalesAgentInfo;
class GeoRuleItem;
class GlobalDir;
class GlobalDirSeg;
class Groups;
class HipMileageExceptInfo;
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
class MiscFareTag;
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
class PfcMultiAirport;
class PfcPFC;
class PfcTktDesigExcept;
class PrintOption;
class ReissueSequence;
class RoundTripRuleItem;
class Routing;
class RoutingKeyInfo;
class RuleApplication;
class RuleCatAlphaCode;
class RuleCategoryDescInfo;
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
class ServiceFeesCxrActivation;
class ServiceGroupInfo;
class ServicesDescription;
class ServicesSubGroup;
class SpanishReferenceFareInfo;
class State;
class StopoversInfo;
class SubCodeInfo;
class SurchargesInfo;
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
class TicketEndorsementsInfo;
class TicketingFeesInfo;
class TicketStock;
class TktDesignatorExemptInfo;
class Tours;
class TpdPsr;
class TPMExclusion;
class TransfersInfo1;
class TravelRestriction;
class TSIInfo;
class USDotCarrier;
class ValueCodeAlgorithm;
class VisitAnotherCountry;
class VoluntaryChangesInfo;
class VoluntaryRefundsInfo;
class Waiver;
class YQYRFees;
class YQYRFeesNonConcur;
class ZoneInfo;
struct FreqFlyerStatus;

typedef std::vector<double> Beta;
const Beta*
getParameterBetaData(DeleteList& del,
                     const int& timeDiff,
                     const int& mileage,
                     const char& direction,
                     const char& APSatInd);

bool
corpIdExistsData(const std::string& corpId,
                 const DateTime& tvlDate,
                 const DateTime& ticketDate,
                 bool isHistorical);

bool
isNationInAreaData(const NationCode& nation,
                   const LocCode& area,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

bool
isNationInSubAreaData(const NationCode& nation,
                      const LocCode& subArea,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

bool
isNationInZoneData(const VendorCode& vendor,
                   int zone,
                   char zoneType,
                   const NationCode& nation,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

bool
isStateInAreaData(const NationCode& nation,
                  const StateCode& state,
                  const LocCode& area,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

bool
isStateInSubAreaData(const NationCode& nation,
                     const StateCode& state,
                     const LocCode& subArea,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

bool
isStateInZoneData(const VendorCode& vendor,
                  int zone,
                  char zoneType,
                  const NationCode& nation,
                  const StateCode& state,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const std::vector<ATPResNationZones*>&
getATPResNationZonesData(const NationCode& key, DeleteList& deleteList);

const AccompaniedTravelInfo*
getAccompaniedTravelData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<CarrierCode>&
getAddOnCarriersForMarketData(const LocCode& market1,
                              const LocCode& market2,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const AddonFareClassCombMultiMap&
getAddOnCombFareClassData(const VendorCode& vendor,
                          const TariffNumber& fareTariff,
                          const CarrierCode& carrier,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<AddonCombFareClassInfo*>&
getAddOnCombFareClassHistoricalData(const VendorCode& vendor,
                                    TariffNumber fareTariff,
                                    const CarrierCode& carrier,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate);

const std::vector<AddonFareInfo*>&
getAddOnFareData(const LocCode& gatewayMarket,
                 const LocCode& interiorMarket,
                 const CarrierCode& carrier,
                 const RecordScope& crossRefType,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<AddonFareInfo*>&
getAddOnFareData(const LocCode& interiorMarket,
                 const CarrierCode& carrier,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical,
                 bool isFareDisplay);

const std::vector<AddonZoneInfo*>&
getAddOnZoneData(const VendorCode& vendor,
                 const CarrierCode& carrier,
                 const TariffNumber& fareTariff,
                 const AddonZone& zone,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<AddonZoneInfo*>&
getAddOnZoneData(const VendorCode& vendor,
                 const CarrierCode& carrier,
                 const LocCode& market,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<AddonZoneInfo*>&
getAddonZoneSITAData(const LocCode& loc,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const AdvResTktInfo*
getAdvResTktData(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

std::vector<const FreqFlyerStatusSeg*>
getFreqFlyerStatusSegsData(DeleteList& del,
                           const CarrierCode carrier,
                           const DateTime& date,
                           const DateTime& ticketDate,
                           bool isHistorical);

std::vector<const FreqFlyerStatus*>
getFreqFlyerStatusesData(DeleteList& del,
                         const CarrierCode carrier,
                         const DateTime& date,
                         const DateTime& ticketDate,
                         bool useHistorical);

const std::vector<CurrencyCode>&
getAllCurrenciesData(DeleteList& deleteList, bool isHistorical);

const std::vector<FareTypeMatrix*>&
getAllFareTypeMatrixData(const DateTime& date, DeleteList& deleteList);

const std::vector<Nation*>&
getAllNationData(const DateTime& ticketDate, DeleteList& deleteList, bool isHistorical);

const std::vector<PaxTypeInfo*>&
getAllPaxTypeData(DeleteList& deleteList, const DateTime& ticketDate, bool isHistorical);

const std::vector<BankerSellRate*>&
getBankerSellRateData(const CurrencyCode& primeCur,
                      const CurrencyCode& cur,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

FilterItRange<BankerSellRate, std::pair<CommonContext, CurrencyCode>>
getBankerSellRateDataRange(const CurrencyCode& primeCur,
                           const CurrencyCode& cur,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<const BaseFareRule*>&
getBaseFareRuleData(const VendorCode& vendor,
                    int itemNo,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const BlackoutInfo*
getBlackoutData(const VendorCode& vendor,
                int itemNo,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const BookingCodeExceptionSequenceList&
getBookingCodeExceptionSequenceData(const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& ruleTariff,
                                    const RuleNumber& rule,
                                    Indicator conventionNo,
                                    const DateTime& date,
                                    bool& isRuleZero,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical,
                                    bool filterExpireDate);

const std::vector<BookingCodeConv*>&
getBookingCodeConvData(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       Indicator conventionNo,
                       const DateTime& date,
                       bool& isRuleZero,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const BookingCodeExceptionSequenceList&
getBookingCodeExceptionSequenceData(const VendorCode& vendor,
                                    const int itemNo,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical,
                                    bool filterExpireDate);

const Brand*
getBrandData(const Indicator& userApplType,
             const UserApplCode& userAppl,
             const CarrierCode& carrier,
             const BrandCode& brandId,
             const DateTime& travelDate);

const std::vector<BrandedFareApp*>&
getBrandedFareAppData(const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const CarrierCode& carrier,
                      const DateTime& travelDate,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<Brand*>&
getBrandsData(const Indicator& userApplType,
              const UserApplCode& userAppl,
              const CarrierCode& carrier,
              const DateTime& travelDate,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

const std::vector<BrandedCarrier*>&
getBrandedCarriersData(DeleteList& deleteList);

const std::vector<BrandedFare*>&
getBrandedFareData(const VendorCode& vendor,
                   const CarrierCode& carrier,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const std::vector<class CustomerSecurityHandshakeInfo*>&
getCustomerSecurityHandshakeData(const PseudoCityCode& pcc,
                                 const Code<8>& productCD,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 const DateTime& dateTime,
                                 bool isHistorical);

const std::vector<BrandedCarrier*>&
getS8BrandedCarriersData(DeleteList& deleteList);

const std::vector<SvcFeesFareIdInfo*>&
getSvcFeesFareIdData(const VendorCode& vendor,
                     long long itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<SvcFeesFeatureInfo*>&
getSvcFeesFeatureData(const VendorCode& vendor,
                      long long itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<CountrySettlementPlanInfo*>&
getCountrySettlementPlanData(const NationCode& countryCode,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical);

const Cabin*
getCabinData(const CarrierCode& carrier,
             const BookingCode& classOfService,
             const DateTime& date,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical);

const std::vector<CarrierApplicationInfo*>&
getCarrierApplicationData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical,
                          const DateTime& applDate = DateTime::emptyDate());

const std::vector<CarrierCombination*>&
getCarrierCombinationData(const VendorCode& vendor,
                          const int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const CarrierFlight*
getCarrierFlightData(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<CarrierMixedClass*>&
getCarrierMixedClassData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const CarrierPreference*
getCarrierPreferenceData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<CarrierCode>&
getCarriersForMarketData(const LocCode& market1,
                         const LocCode& market2,
                         bool includeAddon,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const CircleTripProvision*
getCircleTripProvisionData(const LocCode& market1,
                           const LocCode& market2,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const CircleTripRuleItem*
getCircleTripRuleItemData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<CombinabilityRuleInfo*>&
getCombinabilityRuleData(const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const TariffNumber& ruleTariff,
                         const RuleNumber& rule,
                         const DateTime& tvlDate,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<CommissionCap*>&
getCommissionCapData(const CarrierCode& carrier,
                     const CurrencyCode& cur,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<ContractPreference*>&
getContractPreferencesData(const PseudoCityCode& pseudoCity,
                           const CarrierCode& carrier,
                           const RuleNumber& rule,
                           const DateTime& versionDate,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<CopMinimum*>&
getCopMinimumData(const NationCode& key,
                  const DateTime& date,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const CopParticipatingNation*
getCopParticipatingNationData(const NationCode& nation,
                              const NationCode& copNation,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<tse::CorpId*>&
getCorpIdData(const std::string& corpId,
              const CarrierCode& carrier,
              const DateTime& tvlDate,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

const Currency*
getCurrencyData(const CurrencyCode& currencyCode,
                const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const std::vector<CurrencySelection*>&
getCurrencySelectionData(const NationCode& nation,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<Customer*>&
getCustomerData(const PseudoCityCode& key,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const std::vector<DBEGlobalClass*>&
getDBEGlobalClassData(const DBEClass& key,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<DateOverrideRuleItem*>&
getDateOverrideRuleItemData(const VendorCode& vendor,
                            int itemNo,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical,
                            const DateTime& applDate = DateTime::emptyDate());

const DayTimeAppInfo*
getDayTimeAppInfoData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const Deposits*
getDepositsData(const VendorCode& vendor,
                int itemNo,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const std::vector<Differentials*>&
getDifferentialsData(const CarrierCode& key,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const DiscountInfo*
getDiscountData(const VendorCode& vendor,
                int itemNo,
                int category,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const EligibilityInfo*
getEligibilityData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const EndOnEnd*
getEndOnEndData(const VendorCode& vendor,
                int itemNo,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const std::vector<LimitationFare*>&
getFCLimitationData(const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const std::vector<FDSFareBasisComb*>&
getFDSFareBasisCombData(const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const Indicator& pseudoCityType,
                        const PseudoCityCode& pseudoCity,
                        const TJRGroup& tjrGroup,
                        const Indicator& fareDisplayType,
                        const Indicator& domIntlAppl,
                        const uint64_t& seqno,
                        DeleteList& deleteList);

const std::vector<FDSGlobalDir*>&
getFDSGlobalDirData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    const Indicator& pseudoCityType,
                    const PseudoCityCode& pseudoCity,
                    const TJRGroup& tjrGroup,
                    const Indicator& fareDisplayType,
                    const Indicator& domIntlAppl,
                    const DateTime& versionDate,
                    const uint64_t& seqno,
                    const DateTime& createDate,
                    DeleteList& deleteList);

const std::vector<FDSPsgType*>&
getFDSPsgTypeData(const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const Indicator& pseudoCityType,
                  const PseudoCityCode& pseudoCity,
                  const TJRGroup& tjrGroup,
                  const Indicator& fareDisplayType,
                  const Indicator& domIntlAppl,
                  const uint64_t& seqno,
                  DeleteList& deleteList);

const std::vector<FDSSorting*>&
getFDSSortingData(const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const Indicator& pseudoCityType,
                  const PseudoCityCode& pseudoCity,
                  const TJRGroup& tjrGroup,
                  const Indicator& fareDisplayType,
                  const Indicator& domIntlAppl,
                  const uint64_t& seqno,
                  DeleteList& deleteList);

const std::vector<tse::FareByRuleApp*>&
getFareByRuleAppData(const CarrierCode& carrier,
                     const std::string& corpId,
                     const AccountCode& accountCode,
                     const TktDesignator& tktDesignator,
                     const DateTime& tvlDate,
                     std::vector<PaxTypeCode>& paxTypes,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical,
                     bool isFareDisplay);

const std::vector<FareByRuleApp*>&
getFareByRuleAppRuleTariffData(const CarrierCode& carrier,
                               const TktDesignator& tktDesignator,
                               const DateTime& tvlDate,
                               std::vector<PaxTypeCode>& paxTypes,
                               const std::vector<CorpId*>& corpIds,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical,
                               bool isFareDisplay);

const std::vector<FareByRuleCtrlInfo*>&
getFareByRuleCtrlData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber);

const std::vector<FareByRuleCtrlInfo*>&
getFareByRuleCtrlData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber,
                      const DateTime& tvlDate,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      bool isFareDisplay);

const FareByRuleItemInfo*
getFareByRuleItemData(const VendorCode& vendor,
                      const int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<FareCalcConfig*>&
getFareCalcConfigData(const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const PseudoCityCode& pseudoCity,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<class FareFocusBookingCodeInfo*>&
getFareFocusBookingCodeData(uint64_t bookingCodeItemNo,
                            DateTime adjustedTicketDate,
                            DeleteList& deleteList,
                            DateTime ticketDate,
                            bool isHistorical);

const std::vector<class FareFocusFareClassInfo*>&
getFareFocusFareClassData(uint64_t fareClassItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical);

const class FareFocusLookupInfo*
getFareFocusLookupData(const PseudoCityCode& pcc,
                       DeleteList& deleteList,
                       DateTime ticketDate,
                       bool isHistorical);

const class FareFocusRuleInfo*
getFareFocusRuleData(uint64_t fareFocusRuleId,
                     DateTime adjustedTicketDate,
                     DeleteList& deleteList,
                     DateTime ticketDate,
                     bool isHistorical);

const class FareRetailerRuleLookupInfo*
getFareRetailerRuleLookupData(Indicator applicationType,
                              const PseudoCityCode& sourcePcc,
                              const PseudoCityCode& pcc,
                              DeleteList& deleteList,
                              DateTime ticketDate,
                              bool isHistorical);

const class FareRetailerRuleInfo*
getFareRetailerRuleData(uint64_t fareRetailerRuleId,
                        DateTime adjustedTicketDate,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical);

const class FareRetailerResultingFareAttrInfo*
getFareRetailerResultingFareAttrData(uint64_t resultingFareAttrItemNo,
                                     DateTime adjustedTicketDate,
                                     DeleteList& deleteList,
                                     DateTime ticketDate,
                                     bool isHistorical);

const std::vector<class FareRetailerCalcInfo*>&
getFareRetailerCalcData(uint64_t fareRetailerCalcItemNo,
                        DateTime adjustedTicketDate,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical);

const class FareFocusAccountCdInfo*
getFareFocusAccountCdData(uint64_t securityItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical);

const class FareFocusRoutingInfo*
getFareFocusRoutingData(uint64_t routingItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical);

const class FareFocusLocationPairInfo*
getFareFocusLocationPairData(uint64_t locationPairItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical);

const class FareFocusDaytimeApplInfo*
getFareFocusDaytimeApplData(uint64_t dayTimeApplItemNo,
                          DateTime createDate,
                          DeleteList& deleteList,
                          DateTime expireDate,
                          bool isHistorical);

const class FareFocusDisplayCatTypeInfo*
getFareFocusDisplayCatTypeData(uint64_t displayCatTypeItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical);

const class FareFocusPsgTypeInfo*
getFareFocusPsgTypeData(uint64_t psgTypeItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical);

const std::vector<class FareFocusCarrierInfo*>&
getFareFocusCarrierData(uint64_t carrierItemNo,
                        DateTime adjustedTicketDate,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical);

const std::vector<class FareFocusRuleCodeInfo*>&
getFareFocusRuleCodeData(uint64_t ruleCdItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical);

const class FareFocusSecurityInfo*
getFareFocusSecurityData(uint64_t securityItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical);

const std::vector<class RBDByCabinInfo*>&
getRBDByCabinData(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  DateTime tvlDate,
                  DeleteList& deleteList,
                  DateTime ticketDate,
                  bool isHistorical);

const std::vector<NoPNROptions*>&
getNoPNROptionsData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const NoPNRFareTypeGroup*
getNoPNRFareTypeGroupData(const int fareTypeGroup,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<const FareClassAppInfo*>&
getFareClassAppData(const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const TariffNumber& ruleTariff,
                    const RuleNumber& ruleNumber,
                    const FareClassCode& fareClass,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    bool isFareDisplay);

const std::vector<const FareClassAppInfo*>&
getFareClassAppDataByTravelDT(const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const TariffNumber& ruleTariff,
                    const RuleNumber& ruleNumber,
                    const FareClassCode& fareClass,
                    DateTime& travelDate,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    bool isFareDisplay);


const std::vector<FareClassRestRule*>&
getFareClassRestRuleData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<FareDispCldInfPsgType*>&
getFareDispCldInfPsgTypeData(const Indicator& userApplType,
                             const UserApplCode& userAppl,
                             const Indicator& pseudoCityType,
                             const PseudoCityCode& pseudoCity,
                             const InclusionCode& inclusionCode,
                             const Indicator& psgTypeInd,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical);

const std::vector<FareDispInclDsplType*>&
getFareDispInclDsplTypeData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const InclusionCode& inclusionCode,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const std::vector<FareDispInclFareType*>&
getFareDispInclFareTypeData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const InclusionCode& inclusionCode,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const std::vector<FareDispInclRuleTrf*>&
getFareDispInclRuleTrfData(const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const InclusionCode& inclusionCode,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<FareDispRec1PsgType*>&
getFareDispRec1PsgTypeData(const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const InclusionCode& inclusionCode,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<FareDispRec8PsgType*>&
getFareDispRec8PsgTypeData(const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const InclusionCode& inclusionCode,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<FareDispTemplate*>&
getFareDispTemplateData(const int& templateID,
                        const Indicator& templateType,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<FareDispTemplateSeg*>&
getFareDispTemplateSegData(const int& templateID,
                           const Indicator& templateType,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<FareDisplayInclCd*>&
getFareDisplayInclCdData(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const Indicator& pseudoCityType,
                         const PseudoCityCode& pseudoCity,
                         const InclusionCode& inclusionCode,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<FareDisplayPref*>&
getFareDisplayPrefData(const Indicator& userApplType,
                       const UserApplCode& userAppl,
                       const Indicator& pseudoCityType,
                       const PseudoCityCode& pseudoCity,
                       const TJRGroup& tjrGroup,
                       DeleteList& deleteList);

const std::vector<FareDisplayPrefSeg*>&
getFareDisplayPrefSegData(const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const TJRGroup& tjrGroup,
                          DeleteList& deleteList);

const std::vector<FareDisplaySort*>&
getFareDisplaySortData(const Indicator& userApplType,
                       const UserApplCode& userAppl,
                       const Indicator& pseudoCityType,
                       const PseudoCityCode& pseudoCity,
                       const TJRGroup& tjrGroup,
                       const DateTime& travelDate,
                       DeleteList& deleteList);

const std::vector<FareDisplayWeb*>&
getFareDisplayWebData(const Indicator& dispInd,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      const PaxTypeCode& paxTypeCode,
                      DeleteList& deleteList);

const std::vector<FareDisplayWeb*>&
getFareDisplayWebForCxrData(const CarrierCode& carrier, DeleteList& deleteList);

const std::set<std::pair<PaxTypeCode, VendorCode> >&
getFareDisplayWebPaxForCxrData(const CarrierCode& carrier, DeleteList& deleteList);

const FareProperties*
getFarePropertiesData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const FareTypeMatrix*
getFareTypeMatrixData(const FareType& key,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<FareTypeQualifier*>&
getFareTypeQualifierData(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const FareType& qualifier,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<const FareInfo*>&
getFaresByMarketCxrData(const LocCode& market1,
                        const LocCode& market2,
                        const CarrierCode& cxr,
                        const VendorCode& vendor,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<const FareInfo*>&
getFaresByMarketCxrHistData(const LocCode& market1,
                            const LocCode& market2,
                            const CarrierCode& cxr,
                            const VendorCode& vendor,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const std::vector<const FareInfo*>&
getFaresByMarketCxrData(const LocCode& market1,
                        const LocCode& market2,
                        const CarrierCode& cxr,
                        const DateTime& startDate,
                        const DateTime& endDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        bool isFareDisplay);

const std::vector<const FareInfo*>&
getBoundFaresByMarketCxrData(const LocCode& market1,
                             const LocCode& market2,
                             const CarrierCode& cxr,
                             const DateTime& startDate,
                             const DateTime& endDate,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical,
                             bool isFareDisplay);

const std::vector<const FareInfo*>&
getFaresByMarketCxrHistData(const LocCode& market1,
                            const LocCode& market2,
                            const CarrierCode& cxr,
                            const DateTime& startDate,
                            const DateTime& endDate,
                            DeleteList& deleteList,
                            const DateTime& ticketDate);

const FlightAppRule*
getFlightAppRuleData(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const FltTrkCntryGrp*
getFltTrkCntryGrpData(const CarrierCode& carrier,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<FootNoteCtrlInfo*>&
getFootNoteCtrlData(const VendorCode vendor,
                    const CarrierCode carrier,
                    const TariffNumber tariffNumber,
                    const Footnote footnote,
                    int category);

const std::vector<FootNoteCtrlInfo*>&
getFootNoteCtrlData(const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const TariffNumber& fareTariff,
                    const Footnote& footnote,
                    const int category,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    bool isFareDisplay);

const std::vector<GeneralFareRuleInfo*>&
getGeneralFareRuleData(const VendorCode vendor,
                       const CarrierCode carrier,
                       const TariffNumber ruleTariff,
                       const RuleNumber rule,
                       const CatNumber category);

const std::vector<GeneralFareRuleInfo*>&
getGeneralFareRuleData(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       const CatNumber& category,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical,
                       bool isFareDisplay);

const std::vector<GeneralFareRuleInfo*>&
getGeneralFareRuleBackDatingData(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& ruleTariff,
                                 const RuleNumber& rule,
                                 const CatNumber& category,
                                 const DateTime& applDate,
                                 DeleteList& deleteList,
                                 bool isFareDisplay);

GeneralRuleApp*
getGeneralRuleAppData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber,
                      CatNumber catNum,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

bool
getGeneralRuleAppTariffRuleData(const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& tariffNumber,
                                const RuleNumber& ruleNumber,
                                CatNumber catNum,
                                RuleNumber& ruleNumOut,
                                TariffNumber& tariffNumOut,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical);

const std::vector<GeneralRuleApp*>&
getGeneralRuleAppByTvlDateData(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& tariffNumber,
                               const RuleNumber& ruleNumber,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical,
                               DateTime tvlDate);

GeneralRuleApp*
getGeneralRuleAppByTvlDateData(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& tariffNumber,
                               const RuleNumber& ruleNumber,
                               CatNumber catNum,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical,
                               DateTime tvlDate);

bool
getGeneralRuleAppTariffRuleByTvlDateData(const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& tariffNumber,
                                         const RuleNumber& ruleNumber,
                                         CatNumber catNum,
                                         RuleNumber& ruleNumOut,
                                         TariffNumber& tariffNumOut,
                                         DeleteList& deleteList,
                                         const DateTime& ticketDate,
                                         bool isHistorical,
                                         DateTime tvlDate);

const std::vector<tse::GeoRuleItem*>&
getGeoRuleItemData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const GlobalDir*
getGlobalDirData(const GlobalDirection& key,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<GlobalDirSeg*>&
getGlobalDirSegData(const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const Groups*
getGroupsData(const VendorCode& vendor,
              int itemNo,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

std::vector<const tse::FDHeaderMsg*>&
getHeaderMsgDataListData(const PseudoCityCode& pseudoCityCode,
                         const PseudoCityType& pseudoCityType,
                         const Indicator& userApplType,
                         const std::string& userAppl,
                         const TJRGroup& tjrGroup,
                         const DateTime& date,
                         DeleteList& deleteList);

const HipMileageExceptInfo*
getHipMileageExceptData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<const IndustryFareAppl*>&
getIndustryFareApplData(Indicator selectionType,
                        const CarrierCode& carrier,
                        const DateTime& date,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<const IndustryFareAppl*>&
getIndustryFareApplData(Indicator selectionType,
                        const CarrierCode& carrier,
                        const DateTime& startDate,
                        const DateTime& endDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<const IndustryFareBasisMod*>&
getIndustryFareBasisModData(const CarrierCode& carrier,
                            Indicator userApplType,
                            const UserApplCode& userAppl,
                            const DateTime& date,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const std::vector<const IndustryPricingAppl*>&
getIndustryPricingApplData(const CarrierCode& carrier,
                           const GlobalDirection& globalDir,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<LimitationJrny*>&
getJLimitationData(const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const std::vector<const JointCarrier*>&
getJointCarrierData(const VendorCode& vendor,
                    int itemNo,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const Loc*
getLocData(const LocCode& locCode,
           const DateTime& date,
           DeleteList& deleteList,
           const DateTime& ticketDate,
           bool isHistorical);

NationCode
getLocNationData(const LocCode& loc,
                 const DateTime& date,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<CustomerActivationControl*>&
getCustomerActivationControlData(const std::string& projectCode,
                                 const DateTime& date,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical);

const std::vector<MarkupControl*>&
getMarkupBySecondSellerIdData(const PseudoCityCode& pcc,
                                 const PseudoCityCode& homePCC,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& ruleTariff,
                                 const RuleNumber& rule,
                                 int seqNo,
                                 long secondarySellerId,
                                 const DateTime& date,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical);

const std::vector<MarkupControl*>&
getMarkupBySecurityItemNoData(const PseudoCityCode& pcc,
                                 const PseudoCityCode& homePCC,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& ruleTariff,
                                 const RuleNumber& rule,
                                 int seqNo,
                                 const DateTime& date,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical);

const std::vector<MarkupControl*>&
getMarkupByPccData(const PseudoCityCode& pcc,
                   const VendorCode& vendor,
                   const CarrierCode& carrier,
                   const TariffNumber& ruleTariff,
                   const RuleNumber& rule,
                   int seqNo,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const std::vector<MarkupSecFilter*>&
getMarkupSecFilterData(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<MarriedCabin*>&
getMarriedCabinsData(const CarrierCode& carrier,
                     const BookingCode& premiumCabin,
                     const DateTime& versionDate,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<MaxPermittedAmountFareInfo*>&
getMaxPermittedAmountFareData(const Loc& origin,
                              const Loc& dest,
                              DeleteList& deleteList,
                              const DateTime& date,
                              const DateTime& ticketDate,
                              bool isHistorical);

const MaxStayRestriction*
getMaxStayRestrictionData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<Mileage*>&
getMileageData(const LocCode& origin,
               const LocCode& destination,
               Indicator mileageType,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const Mileage*
getMileageData(const LocCode& origin,
               const LocCode& dest,
               Indicator mileageType,
               const GlobalDirection globalDir,
               const DateTime& date);

const MileageSubstitution*
getMileageSubstitutionData(const LocCode& key,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<MileageSurchExcept*>&
getMileageSurchExceptData(const VendorCode& vendor,
                          int textTblItemNo,
                          const CarrierCode& governingCarrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber& rule,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<MinFareAppl*>&
getMinFareApplData(const VendorCode& textTblVendor,
                   int textTblItemNo,
                   const CarrierCode& governingCarrier,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const std::vector<MinFareDefaultLogic*>&
getMinFareDefaultLogicData(const VendorCode& vendor,
                           const CarrierCode& carrier,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const MinFareFareTypeGrp*
getMinFareFareTypeGrpData(const std::string& key,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<MinFareRuleLevelExcl*>&
getMinFareRuleLevelExclData(const VendorCode& vendor,
                            int textTblItemNo,
                            const CarrierCode& governingCarrier,
                            const TariffNumber& ruleTariff,
                            const DateTime& date,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const MinStayRestriction*
getMinStayRestrictionData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const MiscFareTag*
getMiscFareTagData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const FareCalcConfigText&
getMsgTextData(const Indicator userApplType,
               const UserApplCode& userAppl,
               const PseudoCityCode& pseudoCity,
               DeleteList& deleteList);

const std::vector<MultiAirportCity*>&
getMultiAirportCityData(const LocCode& city,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<MultiAirportCity*>&
getMultiCityAirportData(const LocCode& locCode,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<MultiTransport*>&
getMultiTransportCityData(const LocCode& locCode,
                          const CarrierCode& carrierCode,
                          GeoTravelType tvlType,
                          const DateTime& tvlDate,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const LocCode
getMultiTransportCityData(const LocCode& locCode,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const LocCode
getMultiTransportCityCodeData(const LocCode& locCode,
                              const CarrierCode& carrierCode,
                              GeoTravelType tvlType,
                              const DateTime& tvlDate,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<MultiTransport*>&
getMultiTransportLocsData(const LocCode& city,
                          const CarrierCode& carrierCode,
                          GeoTravelType tvlType,
                          const DateTime& tvlDate,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<NUCInfo*>&
getNUCAllCarriersData(const CurrencyCode& currency,
                      const DateTime& from_date,
                      const DateTime& to_date,
                      DeleteList& deleteList);

NUCInfo*
getNUCFirstData(const CurrencyCode& currency,
                const CarrierCode& carrier,
                const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const Nation*
getNationData(const NationCode& nationCode,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

const std::vector<NegFareCalcInfo*>&
getNegFareCalcData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const NegFareRest*
getNegFareRestData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const NegFareRestExt*
getNegFareRestExtData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<NegFareRestExtSeq*>&
getNegFareRestExtSeqData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<NegFareSecurityInfo*>&
getNegFareSecurityData(const VendorCode& vendor,
                       int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<OpenJawRestriction*>&
getOpenJawRestrictionData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const OpenJawRule*
getOpenJawRuleData(const VendorCode& vendor,
                   const int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const std::vector<const PaxTypeCodeInfo*>
getPaxTypeCodeData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const std::vector<const TaxCodeTextInfo*>
getTaxCodeTextData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const PaxTypeInfo*
getPaxTypeData(const PaxTypeCode& paxType,
               const VendorCode& vendor,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const std::vector<const PaxTypeMatrix*>&
getPaxTypeMatrixData(const PaxTypeCode& key,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const PenaltyInfo*
getPenaltyData(const VendorCode& vendor,
               int itemNo,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const std::vector<PfcAbsorb*>&
getPfcAbsorbData(const LocCode& pfcAirport,
                 const CarrierCode& localCarrier,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<PfcAbsorb*>&
getAllPfcAbsorbData(DeleteList& deleteList);

const std::vector<PfcCollectMeth*>&
getPfcCollectMethData(const CarrierCode& carrier,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<PfcCollectMeth*>&
getAllPfcCollectMethData(DeleteList& del);

const PfcEquipTypeExempt*
getPfcEquipTypeExemptData(const EquipmentType& equip,
                          const StateCode& state,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<PfcEssAirSvc*>&
getPfcEssAirSvcData(const LocCode& easHubArpt,
                    const LocCode& easArpt,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const std::vector<PfcEssAirSvc*>&
getAllPfcEssAirSvcData(DeleteList& deleteList);

const PfcMultiAirport*
getPfcMultiAirportData(const LocCode& key,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<PfcMultiAirport*>&
getAllPfcMultiAirportData(DeleteList& deleteList);

const PfcPFC*
getPfcPFCData(const LocCode& key,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

const std::vector<PfcPFC*>&
getAllPfcPFCData(DeleteList& deleteList);

const std::vector<PfcEquipTypeExempt*>&
getAllPfcEquipTypeExemptData(DeleteList& del);

const std::vector<const PfcTktDesigExcept*>&
getPfcTktDesigExceptData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<ReissueSequence*>&
getReissueData(const VendorCode& vendor,
               int itemNo,
               const DateTime& ticketDate,
               DeleteList& deleteList,
               bool isHistorical,
               const DateTime& applDate = DateTime::emptyDate());

const RoundTripRuleItem*
getRoundTripRuleItemData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const std::vector<Routing*>&
getRoutingData(const VendorCode& vendor,
               const CarrierCode& carrier,
               const TariffNumber& routingTariff,
               const RoutingNumber& routingNumber,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const std::vector<RoutingKeyInfo*>&
getRoutingForMarketData(const LocCode& market1,
                        const LocCode& market2,
                        const CarrierCode& carrier,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const MarketRoutingInfo&
getMarketRoutingInfoData(const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const RoutingNumber& routing,
                         TariffNumber routingTariff,
                         const LocCode& market1,
                         const LocCode& market2,
                         const bool getSingles,
                         const bool getDoubles,
                         DeleteList& deleteList);

const RuleApplication*
getRuleApplicationData(const VendorCode& vendor,
                       int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<RuleCatAlphaCode*>&
getRuleCatAlphaCodeData(const AlphaCode& key,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const RuleCategoryDescInfo*
getRuleCategoryDescData(const CatNumber& key, DeleteList& deleteList);

const std::vector<SalesNationRestr*>&
getSalesNationRestrData(const NationCode& key,
                        const DateTime& date,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const SalesRestriction*
getSalesRestrictionData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<const SamePoint*>&
getSamePointData(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const SeasonalAppl*
getSeasonalApplData(const VendorCode& vendor,
                    int itemNo,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const std::vector<const SectorDetailInfo*>
getSectorDetailData(const VendorCode& vendor,
                    int itemNo,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const std::vector<SectorSurcharge*>&
getSectorSurchargeData(const CarrierCode& key,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<SpanishReferenceFareInfo*>&
getSpanishReferenceFareData(CarrierCode tktCarrier, const CarrierCode& fareCarrier,
                            const LocCode& sourceLoc, const LocCode& destLoc,
                            DeleteList& deleteList, const DateTime& date,
                            const DateTime& ticketDate, bool isHistorical);

const State*
getStateData(const NationCode& nationCode,
             const StateCode& stateCode,
             const DateTime& date,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical);

const StopoversInfo*
getStopoversData(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

std::vector<const tse::FDSuppressFare*>&
getSuppressFareListData(const PseudoCityCode& pseudoCityCode,
                        const Indicator pseudoCityType,
                        const TJRGroup& ssgGroupNo,
                        const CarrierCode& carrierCode,
                        const DateTime& travelDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const SurchargesInfo*
getSurchargesData(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const SurfaceSectorExempt*
getSurfaceSectorExemptData(const LocCode& origLoc,
                           const LocCode& destLoc,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<SurfaceTransfersInfo*>&
getSurfaceTransfersData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<SurfaceSectorExemptionInfo*>&
getSurfaceSectorExemptionInfoData(const CarrierCode& carrierCode,
                                  DeleteList& deleteList,
                                  const DateTime& ticketDate,
                                  bool isHistorical);

const TSIInfo*
getTSIData(int key, DeleteList& deleteList);

const Indicator
getTariffInhibitData(const VendorCode& vendor,
                     const Indicator tariffCrossRefType,
                     const CarrierCode& carrier,
                     const TariffNumber& fareTariff,
                     const TariffCode& ruleTariffCode,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const TariffMileageAddon*
getTariffMileageAddonData(const CarrierCode& carrier,
                          const LocCode& unpublishedAddonLoc,
                          const GlobalDirection& globalDir,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<TariffRuleRest*>&
getTariffRuleRestData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<TariffCrossRefInfo*>&
getTariffXRefData(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  const RecordScope& crossRefType,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByAddonTariffData(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const RecordScope& crossRefType,
                               const TariffNumber& addonTariff,
                               const DateTime& date,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical);

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByFareTariffData(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const RecordScope& crossRefType,
                              const TariffNumber& fareTariff,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByGenRuleTariffData(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const RecordScope& crossRefType,
                                 const TariffNumber& ruleTariff,
                                 const DateTime& date,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical);

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByRuleTariffData(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const RecordScope& crossRefType,
                              const TariffNumber& ruleTariff,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<TaxAkHiFactor*>&
getTaxAkHiFactorData(const LocCode& key,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<const TaxReportingRecordInfo*>
getTaxReportingRecordData(const VendorCode& vendor,
                          const NationCode& nation,
                          const CarrierCode& taxCarrier,
                          const TaxCode& taxCode,
                          const TaxType& taxType,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<const TaxReportingRecordInfo*>
getAllTaxReportingRecordData(const TaxCode& taxCode,
                             DeleteList& deleteList);

const std::vector<const TaxRulesRecord*>&
getTaxRulesRecordData(const NationCode& nation,
                      const Indicator& taxPointTag,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<const TaxRulesRecord*>&
getTaxRulesRecordByCodeData(const TaxCode& taxCode,
                            const DateTime& date,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
const std::vector<const TaxRulesRecord*>&
getTaxRulesRecordByCodeAndTypeData(const TaxCode& taxCode,
                                   const TaxType& taxType,
                                   const DateTime& date,
                                   DeleteList& deleteList,
                                   const DateTime& ticketDate,
                                   bool isHistorical);

const std::vector<TaxCodeReg*>&
getTaxCodeData(const TaxCode& taxCode,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const TaxNation*
getTaxNationData(const NationCode& key,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical);

const std::vector<TaxReissue*>&
getTaxReissueData(const TaxCode& taxCode,
                  const DateTime& date,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const std::vector<TaxExemption*>&
getTaxExemptionData(const TaxCode& taxCode,
                  const PseudoCityCode& channelId,
                  const DateTime& date,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const std::vector<TaxSegAbsorb*>&
getTaxSegAbsorbData(const CarrierCode& carrier,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical);

const TicketEndorsementsInfo*
getTicketEndorsementsData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<TicketStock*>&
getTicketStockData(int key,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const Tours*
getToursData(const VendorCode& vendor,
             int itemNo,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical);

const std::vector<TpdPsr*>&
getTpdPsrData(Indicator applInd,
              const CarrierCode& carrier,
              Indicator area1,
              Indicator area2,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

const TransfersInfo1*
getTransfers1Data(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical);

const TravelRestriction*
getTravelRestrictionData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const VisitAnotherCountry*
getVisitAnotherCountryData(const VendorCode& vendor,
                           int itemNo,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const VoluntaryChangesInfo*
getVoluntaryChangesData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        const DateTime& applDate = DateTime::emptyDate());

const VoluntaryRefundsInfo*
getVoluntaryRefundsData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        const DateTime& applDate = DateTime::emptyDate());

const std::vector<Waiver*>&
getWaiverData(const VendorCode& vendor,
              int itemNo,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical,
              const DateTime& applDate = DateTime::emptyDate());

const std::vector<FareTypeTable*>&
getFareTypeTableData(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical,
                     const DateTime& applDate = DateTime::emptyDate());

const SeasonalityDOW*
getSeasonalityDOWData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      const DateTime& applDate = DateTime::emptyDate());

const std::vector<YQYRFees*>&
getYQYRFeesData(const CarrierCode& key,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical);

const std::vector<YQYRFeesNonConcur*>&
getYQYRFeesNonConcurData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical);

const ZoneInfo*
getZoneData(const VendorCode& vendor,
            const Zone& zone,
            Indicator zoneType,
            const DateTime& date,
            DeleteList& deleteList,
            const DateTime& ticketDate,
            bool isHistorical);

const ZoneInfo*
getZoneFareFocusGroupData(const VendorCode& vendor,
                         const Zone& zone,
                         Indicator zoneType,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical,
                         bool fareFocusGroup);

const std::vector<class CommissionContractInfo*>& getCommissionContractData(const VendorCode& vendor,
                                                                            const CarrierCode& carrier,
                                                                            const PseudoCityCode& pcc,
                                                                            DeleteList& deleteList,
                                                                            DateTime ticketDate,
                                                                            bool isHistorical);

const std::vector<class CommissionProgramInfo*>& getCommissionProgramData(const VendorCode& vendor,
                                                                          uint64_t contractId,
                                                                          DeleteList& deleteList,
                                                                          DateTime ticketDate,
                                                                          bool isHistorical);

const std::vector<class CommissionRuleInfo*>& getCommissionRuleData(const VendorCode& vendor,
                                                                    uint64_t programId,
                                                                    DeleteList& deleteList,
                                                                    DateTime ticketDate,
                                                                    bool isHistorical);

bool
isMultilateralData(const VendorCode& vendor,
                   const RuleNumber& rule,
                   const LocCode& loc1,
                   const LocCode& loc2,
                   const DateTime& date,
                   const DateTime& ticketDate,
                   bool isHistorical);

bool
isMultilateralData(const VendorCode& vendor,
                   const RuleNumber& rule,
                   const LocCode& loc1,
                   const LocCode& loc2,
                   const DateTime& startDate,
                   const DateTime& endDate,
                   const DateTime& ticketDate,
                   bool isHistorical);

bool
isNationInAreaData(const NationCode& nation, const LocCode& area);

bool
isNationInSubAreaData(const NationCode& nation, const LocCode& subArea);

bool
isNationInZoneData(const VendorCode& vendor,
                   int zone,
                   char zoneType,
                   const NationCode& nation,
                   const DateTime& ticketDate,
                   bool isHistorical);

bool
isRuleInFareMarketData(const LocCode& market1,
                       const LocCode& market2,
                       const CarrierCode& cxr,
                       const RuleNumber ruleNumber,
                       const DateTime& ticketDate,
                       bool isHistorical);

bool
isStateInAreaData(const std::string& nationState, const LocCode& area);

bool
isStateInSubAreaData(const std::string& nationState, const LocCode& subArea);

bool
isStateInZoneData(const VendorCode& vendor,
                  int zone,
                  char zoneType,
                  const std::string& nationState,
                  const DateTime& ticketDate,
                  bool isHistorical);

void
loadFaresForMarketData(const LocCode& market1,
                       const LocCode& market2,
                       const std::vector<CarrierCode>& cxr,
                       const DateTime& ticketDate,
                       bool isHistorical);

const DateTime&
getVoluntaryChangesConfigData(const CarrierCode& carrier,
                              const DateTime& currentTktDate,
                              const DateTime& originalTktIssueDate,
                              DeleteList& deleteList);
const DateTime&
getVoluntaryRefundsConfigData(const CarrierCode& carrierCode,
                              const DateTime& currentTktDate,
                              const DateTime& originalTktIssueDate,
                              DeleteList& deleteList);

const std::vector<TicketingFeesInfo*>&
getTicketingFeesData(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<SubCodeInfo*>&
getSubCodeData(const VendorCode& vendor,
               const CarrierCode& carrier,
               const ServiceTypeCode& serviceTypeCode,
               const ServiceGroup& serviceGroup,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const std::vector<OptionalServicesConcur*>&
getOptionalServicesConcurData(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const ServiceTypeCode& serviceTypeCode,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<OptionalServicesInfo*>&
getOptionalServicesData(const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const ServiceTypeCode& serviceTypeCode,
                        const ServiceSubTypeCode& serviceSubTypeCode,
                        Indicator fltTktMerchInd,
                        const DateTime& date,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const std::vector<OptionalServicesInfo*>&
getOptionalServicesMktData(const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const LocCode& loc1,
                           const LocCode& loc2,
                           const ServiceTypeCode& serviceTypeCode,
                           const ServiceSubTypeCode& serviceSubTypeCode,
                           Indicator fltTktMerchInd,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<MerchActivationInfo*>&
getMerchActivationData(uint64_t productId,
                       const CarrierCode& carrier,
                       const PseudoCityCode& pseudoCity,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const MerchCarrierPreferenceInfo*
getMerchCarrierPreferenceData(const CarrierCode& carrier,
                              const ServiceGroup& groupCode,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<SvcFeesSecurityInfo*>&
getSvcFeesSecurityData(const VendorCode& vendor,
                       const int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<SvcFeesAccCodeInfo*>&
getSvcFeesAccountCodeData(const VendorCode& vendor,
                          const int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<SvcFeesCurrencyInfo*>&
getSvcFeesCurrencyData(const VendorCode& vendor,
                       const int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical);

const std::vector<SvcFeesCxrResultingFCLInfo*>&
getSvcFeesCxrResultingFCLData(const VendorCode& vendor,
                              const int itemNo,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<SvcFeesResBkgDesigInfo*>&
getSvcFeesResBkgDesigData(const VendorCode& vendor,
                          const int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<SvcFeesTktDesignatorInfo*>&
getSvcFeesTktDesignatorData(const VendorCode& vendor,
                            const int itemNo,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const std::vector<ServiceFeesCxrActivation*>&
getServiceFeesCxrActivationData(const CarrierCode& validatingCarrier,
                                const DateTime& date,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical);

const std::vector<const ServiceBaggageInfo*>
getServiceBaggageData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<TPMExclusion*>&
getTPMExclusionsData(const CarrierCode& carrier,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const PrintOption*
getPrintOptionData(const VendorCode& vendor,
                   const std::string& fareSource,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical);

const TaxRestrictionLocationInfo*
getTaxRestrictionLocationData(const TaxRestrictionLocation& location,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

std::vector<TaxSpecConfigReg*>&
getTaxSpecConfigData(const TaxSpecConfigName& name,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical);

const ValueCodeAlgorithm*
getValueCodeAlgorithmData(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const std::string& name,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<ServiceGroupInfo*>&
getAllServiceGroupData(DeleteList& deleteList, bool isHistorical);

const std::vector<OptionalServicesActivationInfo*>&
getOptServiceActivationData(Indicator crs,
                            const UserApplCode& userCode,
                            const std::string& application,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical);

const TaxCarrierFlightInfo*
getTaxCarrierFlightData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

const TaxText*
getTaxTextData(const VendorCode& vendor,
               int itemNo,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical);

const TaxCarrierAppl*
getTaxCarrierApplData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical);

const std::vector<InterlineCarrierInfo*>&
getInterlineCarrierData(DeleteList& deleteList,
                        bool isHistorical);

const std::vector<IntralineCarrierInfo*>&
getIntralineCarrierData(DeleteList& deleteList,
                        bool isHistorical);

const std::vector<InterlineTicketCarrierInfo*>&
getInterlineTicketCarrierData(const CarrierCode& carrier,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<InterlineTicketCarrierStatus*>&
getInterlineTicketCarrierStatusData(const CarrierCode& carrier,
                                    const CrsCode& crsCode,
                                    const DateTime& date,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical);

const std::vector<NvbNvaInfo*>&
getNvbNvaData(const VendorCode& vendor,
              const CarrierCode& carrier,
              const TariffNumber& ruleTariff,
              const RuleNumber& rule,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical);

const ServicesDescription*
getServicesDescriptionData(const ServiceGroupDescription& value,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const ServicesSubGroup*
getServicesSubGroupData(const ServiceGroup& serviceGroup,
                        const ServiceGroup& serviceSubGroup,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical);

bool
getUSDotCarrierData(const CarrierCode& carrier, const DateTime& ticketDate, bool isHistorical);

bool
getCTACarrierData(const CarrierCode& carrier, const DateTime& ticketDate, bool isHistorical);

const std::vector<SeatCabinCharacteristicInfo*>&
getSeatCabinCharacteristicData(const CarrierCode& carrier,
                               const Indicator& codeType,
                               const DateTime& travelDate,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical);

const std::vector<Cat05OverrideCarrier*>&
getInfiniCat05OverrideData(const PseudoCityCode& pcc, DeleteList& deleteList);

const BankIdentificationInfo*
getBankIdentificationData(const FopBinNumber& binNumber,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical);

const std::vector<AirlineAllianceCarrierInfo*>&
getAirlineAllianceCarrierData(const CarrierCode& carrierCode,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<AirlineAllianceCarrierInfo*>&
getGenericAllianceCarrierData(const GenericAllianceCode& genericAllianceCode,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical);

const std::vector<AirlineAllianceContinentInfo*>&
getAirlineAllianceContinentData(const GenericAllianceCode& genericAllianceCode,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical,
                                bool reduceTemporaryVectorsFallback = false);
const std::vector<TktDesignatorExemptInfo*>&
getTktDesignatorExemptData(const CarrierCode& carrier,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical);

const std::vector<GenSalesAgentInfo*>&
getGenSalesAgentData(DeleteList& deleteList,
                     const CrsCode& gds,
                     const NationCode& country,
                     const SettlementPlanType& settlementPlan,
                     const CarrierCode& validatingCxr,
                     const DateTime& ticketDate,
                     bool isHistorical);

const std::vector<GenSalesAgentInfo*>&
getGenSalesAgentData(DeleteList& deleteList,
                     const CrsCode& gds,
                     const NationCode& country,
                     const SettlementPlanType& settlementPlan,
                     const DateTime& date);

const std::vector<AirlineCountrySettlementPlanInfo*>&
getAirlineCountrySettlementPlanData(const NationCode& country,
                                    const CrsCode& gds,
                                    const CarrierCode& airline,
                                    const SettlementPlanType& spType,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical);

const std::vector<AirlineCountrySettlementPlanInfo*>&
getAirlineCountrySettlementPlanData(const NationCode& country,
                                    const CrsCode& gds,
                                    const SettlementPlanType& spType,
                                    DeleteList& deleteList,
                                    const DateTime& date);

const std::vector<AirlineCountrySettlementPlanInfo*>&
getAirlineCountrySettlementPlanData(const CrsCode& gds,
                                    const NationCode& country,
                                    const CarrierCode& airline,
                                    DeleteList& deleteList,
                                    const DateTime& date);

const std::vector<AirlineInterlineAgreementInfo*>&
getAirlineInterlineAgreementData(const NationCode& country,
                                 const CrsCode& gds,
                                 const CarrierCode& validatingCarrier,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical);

const std::vector<NeutralValidatingAirlineInfo*>&
getNeutralValidatingAirlineData(const NationCode& country,
                                const CrsCode& gds,
                                const SettlementPlanType& spType,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical);

const std::vector<EmdInterlineAgreementInfo*>&
getEmdInterlineAgreementData(const NationCode& country,
                             const CrsCode& gds,
                             const CarrierCode& validatingCarrier,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical);

} // tse
