//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/RoutingUtil.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DBAForwardDecl.h"
#include "Fares/FareByRuleFareCreator.h"
#include "Fares/FareController.h"
#include "Fares/FareByRuleBaseFareValidator.h"

#include <map>

namespace tse
{
class Agent;
class BaseFareRule;
class CarrierPreference;
class FareByRuleApp;
class FareByRuleAppValidator;
class FareByRuleItemInfo;
class FareByRuleProcessingInfo;
class FareCollectorOrchestrator;
class FareMarket;
class FareTypeMatcher;
class Itin;
class PaxTypeFare;
class PricingTrx;
class QualifyFltAppRuleData;

typedef std::pair<std::vector<PaxTypeFare*>::const_iterator,
                  std::vector<PaxTypeFare*>::const_iterator> PTFRange;

//@class FareByRuleController
// This class processes Category 25 Fare By Rule.
// This is a controller class to use Category 25 to create fares. The processing
// starts from examining all Record 8s. All Record 8s are retrieved and attempted
// to be matched against the passenger's information at time of pricing.
// Once a Record 8 is matched, the next step is to match Record 2 for Category 25.
// The Record 2 Category 25 points to the applicable Record 3 information
// for Category 25.

class FareByRuleController : public FareController
{
  friend class FareByRuleControllerTest;

public:
  FareByRuleController(PricingTrx& trx,
                       const FareCollectorOrchestrator* fco,
                       Itin& itin,
                       FareMarket& fareMarket);

  // overrides to external services
  virtual const PaxTypeInfo* getPaxTypeInfo(const BaseFareRule& baseFareRule) const;
  virtual void getPricingCurrency(const NationCode& nation, CurrencyCode& currency) const;
  virtual bool matchNationCurrency(const NationCode& nation, const CurrencyCode& currency) const;
  virtual const FareByRuleItemInfo* getFareByRuleItem(const VendorCode& vendor, int itemNo) const;
  virtual const std::vector<const BaseFareRule*>&
  getBaseFareRule(const VendorCode& vendor, int itemNo) const;
  virtual const Loc* getLoc(const LocCode& locCode) const;
  virtual const LocCode getMultiTransportCity(const LocCode& locCode) const;
  virtual const std::vector<tse::FareByRuleApp*>&
  getFareByRuleApp(const std::string& corpId,
                   const AccountCode& accountCode,
                   const TktDesignator& tktDesignator,
                   std::vector<PaxTypeCode>& paxTypes) const;
  virtual const std::vector<const IndustryFareAppl*>*
  getIndustryFareAppl(Indicator selectionType) const;
  virtual const CarrierPreference* getCarrierPreference(const CarrierCode& carrier) const;
  virtual void putIntoPTF(PaxTypeFare& ptf, FareInfo& fareInfo);
  virtual bool fareByRuleAppValidate(FareByRuleAppValidator& validator,
                                     FareByRuleProcessingInfo& fbrProcessingInfo,
                                     std::map<std::string, bool>& ruleTariffMap) const;
  virtual bool getFareByRuleCtrlInfo(FareByRuleApp& fbrApp,
                                     FareByRuleCtrlInfoVec& fbrCtrlInfoVec,
                                     DiagManager& diagManager);
  virtual FareInfo* createFareInfo(const MoneyAmount& fareAmt,
                                   const CurrencyCode& currency,
                                   const DateTime& effDate,
                                   const DateTime& expDate);
  virtual void createFBRPaxTypeFareRuleData(PaxTypeFare& ptf, PaxTypeFare* baseFare);
  virtual uint16_t getTPM(const Loc& market1,
                          const Loc& market2,
                          const GlobalDirection& glbDir,
                          const DateTime& tvlDate) const;
  virtual void publishedFaresStep(FareMarket& targetMarket) const;
  // end of overides

  bool process();

  static uint16_t getMileage(FareMarket& fareMarket, PricingTrx& trx);

  static MoneyAmount calculateFareAmtPerMileage(MoneyAmount& fareAmount, uint16_t mileage);

  static bool analyzeValidatingCarriers(PricingTrx& trx, PaxTypeFare* ptFare);

  void initFareMarketCurrency();

  static FareClassAppSegInfoList::const_iterator
  matchFareClassAppInfoToPaxType(const FareClassAppInfo& fcAppInfo,
                                 const BaseFareRule& baseFareRule);

  static constexpr char CALCULATED = 'C';
  static constexpr char SPECIFIED = 'S';
  static constexpr char CREATE_RT_FROM_OW = 'D';
  static constexpr char ADD_SPECIFIED_TO_CALCULATED = 'A';
  static constexpr char SUBTRACT_SPECIFIED_FROM_CALCULATED = 'M';
  static constexpr char SELECT_HIGHEST = 'H';
  static constexpr char SELECT_LOWEST = 'L';
  static constexpr char NOT_APPLICABLE_X = 'X';
  static constexpr char NOT_APPLICABLE_N = 'N';
  static constexpr char ADD_SPECIFIED_TO_BASE_CALC_PERCENTAGE = 'B';
  static constexpr char SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE = 'G';
  static constexpr char BLANK = ' ';
  static constexpr char ASTERISK = '*';
  static const char* BLANK_CURRENCY;
  static const char* MILEAGE_ROUTING;

protected:
  bool processRule(FareByRuleProcessingInfo& fbrProcessingInfo,
                   const FareByRuleCtrlInfo& crInfo,
                   bool isLocationSwapped);

  bool processCategoryRuleItemInfo(CategoryRuleItemInfo* catRuleItemInfo,
                                   CategoryRuleItemInfoSet* ruleItemInfoSet,
                                   std::vector<CategoryRuleItemInfo>* segQual,
                                   FareByRuleProcessingInfo& fbrProcessingInfo,
                                   const FareByRuleCtrlInfo& crInfo,
                                   PaxTypeFare& dummyPtFare,
                                   bool isLocationSwapped,
                                   DiagManager& diag,
                                   bool& isSoftPassFor25,
                                   Record3ReturnTypes& retCode);

  void processSpecifiedFare(FareByRuleProcessingInfo& fbrProcessingInfo);
  void processFare(FareByRuleProcessingInfo& fbrProcessingInfo, PaxTypeFare& dummyPtFare);
  void processSpecifiedFareWithBlankCurrency(FareByRuleProcessingInfo& fbrProcessingInfo);
  void processSpecifiedFare_K(FareByRuleProcessingInfo& fbrProcessingInfo);
  void processSpecifiedFare_EF(FareByRuleProcessingInfo& fbrProcessingInfo, uint16_t mileage);

  void processCalculatedFare(FareByRuleProcessingInfo& fbrProcessingInfo);
  bool invalidDCT(const PaxTypeFare& ptFare, const FareByRuleItemInfo& fbrItemInfo) const;
  bool matchFareToFbrItem(const PaxTypeFare& ptFare,
                          const FareByRuleProcessingInfo& fbrProcessingInfo,
                          const FareTypeMatcher& ftMatcher);
  void printT989Header(DiagManager& diag, int fbrItemInfoNo) const;
  void selectCalculatedFares(std::vector<PaxTypeFare*>& processFares,
                             const std::vector<const BaseFareRule*>& baseFareRules,
                             FareByRuleProcessingInfo& fbrProcessingInfo);
  void createCalculatedFares(const std::vector<PaxTypeFare*>& processFares,
                             FareByRuleProcessingInfo& fbrProcessingInfo);
  bool finalizeFareCreation(PaxTypeFare* ptFare, FareByRuleProcessingInfo& fbrProcessingInfo);

  bool calculateFareAmt(PaxTypeFare& paxTypeFare,
                        const FareByRuleItemInfo& fbrItemInfo,
                        FareByRuleProcessingInfo& fbrProcessingInfo);

  static bool
  fareMatchesRecord3(const FareByRuleItemInfo& fbrItemInfo, const FareByRuleApp& fbrApp);

  bool ensureMinMaxRange(bool useRecord3,
                         MoneyAmount& fareAmt,
                         MoneyAmount minFareAmount,
                         MoneyAmount maxFareAmount,
                         const CurrencyCode& currency,
                         const FareByRuleItemInfo& fbrItemInfo);

  bool ensureMinMaxRangeHighest(bool useRecord3,
                                MoneyAmount& fareAmt,
                                MoneyAmount minFareAmount,
                                MoneyAmount maxFareAmount,
                                const CurrencyCode& currency,
                                const FareByRuleItemInfo& fbrItemInfo);

  bool ensureMinMaxRangeLowest(bool useRecord3,
                               MoneyAmount& fareAmt,
                               MoneyAmount minFareAmount,
                               MoneyAmount maxFareAmount,
                               const CurrencyCode& currency,
                               const FareByRuleItemInfo& fbrItemInfo);

  bool ensureMinMaxRangeOther(bool useRecord3,
                              MoneyAmount& fareAmt,
                              MoneyAmount minFareAmount,
                              MoneyAmount maxFareAmount,
                              const CurrencyCode& currency,
                              const FareByRuleItemInfo& fbrItemInfo);

  bool findOtherFares(const FareByRuleItemInfo& fbrItemInfo,
                      const std::vector<PaxTypeFare*>*& otherFares,
                      bool& sorted);

  bool createSpecifiedFare(FareByRuleProcessingInfo& fbrProcessingInfo,
                           MoneyAmount& fareAmt,
                           const CurrencyCode& currency);
  void setPtfFlags(PaxTypeFare& ptf, const FareByRuleApp& fbrApp) const;

  bool matchFareToRule(PaxTypeFare& ptFare,
                       const BaseFareRule& baseFareRule,
                       RoutingUtil::PaxTypeFareMap* routingPTFareMap);
  bool matchBaseFareFareRecord(const PaxTypeFare& ptFare, const BaseFareRule& baseFareRule);
  bool matchPaxType(const PaxTypeFare& ptFare, const BaseFareRule& baseFareRule);
  bool matchBaseFareRecord1(const PaxTypeFare& ptFare, const BaseFareRule& baseFareRule);
  bool matchRouting(PaxTypeFare& ptFare,
                    const BaseFareRule& baseFareRule,
                    RoutingUtil::PaxTypeFareMap* routingPTFareMap);
  bool matchFootnoteAndBkgCode(PaxTypeFare& ptFare, const BaseFareRule& baseFareRule);
  bool matchMinMaxFareAmount(const PaxTypeFare& ptFare, const BaseFareRule& baseFareRule);

  bool createCalculatedFare(PaxTypeFare* baseFare,
                            PaxTypeFare& calculatedFare,
                            const FareByRuleProcessingInfo& fbrProcessingInfo);

  static const std::vector<PaxTypeFare*>* findFaresFromPaxType(const FareMarket& fareMarket);

  PTFRange determinePTFRange(const BaseFareRule& baseFareRule);
  PTFRange findFaresFromPaxType(FareMarket& fareMarket, const BaseFareRule& baseFareRule);
  RoutingUtil::PaxTypeFareMap* createPaxTypeFareMap(const PTFRange& range) const;

  bool findPaxType(const PaxTypeCode& paxTypeCode,
                   const std::vector<PaxTypeBucket>& paxTypeCortege) const;

  bool marketsMatch(const LocCode& market1, const LocCode& market2) const;

  static bool carriersMatch(const CarrierCode& carrier1, const CarrierCode& carrier2);

  bool isDirectionPass(const CategoryRuleItemInfo& rule, bool isLocationSwapped) const;

  bool isDirectionPassForFD(const CategoryRuleItemInfo& rule, bool isLocationSwapped) const;

  FareMarket* cloneFareMarket(const CarrierCode& newCarrier) const;

  MoneyAmount findMinAndMaxFares(const std::vector<PaxTypeFare*>* otherFares,
                                 bool sorted,
                                 const FareByRuleItemInfo& fbrItemInfo,
                                 const CurrencyCode& currency,
                                 const TariffNumber& baseRuleTariff);

  bool isMinAndMaxFare(const PaxTypeFare& ptf,
                       MoneyAmount minFareRange,
                       MoneyAmount maxFareRange,
                       const FareByRuleItemInfo& fbrItemInfo,
                       const CurrencyCode& currency,
                       const TariffNumber& baseRuleTariff) const;

  void changePaxType(const BaseFareRule& baseFareRule, FareMarket& fareMarket) const;
  void setCarrierPref(const CarrierCode& ruleCarrier);
  bool matchFbrPref(const VendorCode& fareVendor) const;

  const std::vector<FareByRuleApp*>&
  getFareByRuleApp(std::vector<FareByRuleApp*>& filteredFBRApp) const;
  void processFareByRuleApp(FareByRuleApp* fbrApp,
                            DiagManager& diag,
                            std::map<std::string, bool>& ruleTariffMap);
  bool isValidForDiagnostic(const FareByRuleApp* fbrApp) const;

  bool matchSpecifiedCurrency(PaxTypeFare& baseFare, const FareByRuleItemInfo& fbrItemInfo) const;

  enum BkcMatchResult
  {
    BKC_PASSED,
    BKC_NEED_AVAIL,
    BKC_FAILED
  };

  BkcMatchResult findPrimeBookingCode(const std::vector<BookingCode>& primeBkcVec,
                                      bool ignoreBkcAvail,
                                      BookingCode& bkc);
  bool checkBookingCodeAvail(BookingCode bkc, bool& matchedBkcAvail,bool& passedBkcAvail);
  bool matchBookingCode(BookingCode bookingCode1, BookingCode bookingCode2, PaxTypeFare& ptFare);
  bool checkAvailability(const BookingCode& bookingCode);

  bool checkAvail(const BookingCode& bookingCode,
                  const std::vector<ClassOfService*>& cosVec,
                  uint16_t numSeatsRequired);

  void
  displayBkcAvail(FareByRuleProcessingInfo& fbrProcessingInfo, const BaseFareRule& baseFareRule);
  void displayBkcAvail(DiagManager& diag, const BaseFareRule& baseFareRule);
  static void displayBkcAvail(DiagManager& diag, bool matched, bool passed, Indicator bkgCode);

  std::vector<FareByRuleApp*>&
  getFareByRuleAppForMultiAC(const TktDesignator& tktDesignator,
                             std::vector<PaxTypeCode>& paxTypes,
                             const std::vector<std::string>& corpIdVec,
                             const std::vector<std::string>& accCodeVec) const;

  const std::vector<FareByRuleApp*>&
  getAllFareByRuleApp(const TktDesignator& tktDesignator, std::vector<PaxTypeCode>& paxTypes) const;

  bool checkDisplayType(const PaxTypeFare& ptf) const;
  void updateFareMarket();
  void setCombinedPercent(FareByRuleProcessingInfo& fbrProcessingInfo);
  DataHandle _dataHandle;

private:
  void addBaseFareInfoBkcAvailMap(PaxTypeFare*, BookingCode);
  bool matchBaseFareVendor(const VendorCode& baseFareVendor, const VendorCode& fbrAppVendor) const;
  bool matchBaseFareSecurity(const PaxTypeFare& ptFare,
                             const FareByRuleProcessingInfo& fbrProcessingInfo);
  bool isVendorATP(const VendorCode& vendor) const;
  bool isVendorSITA(const VendorCode& vendor) const;
  bool isVendorSMFA(const VendorCode& vendor) const;
  bool isVendorSMFC(const VendorCode& vendor) const;
  virtual GeneralFareRuleInfo* getCat35Rec2(const PaxTypeFare& ptFare) const;
  void addToFailBaseFareList(const PaxTypeFare& ptFare, const char* msg);
  void baseFareFailedValidation(const PaxTypeFare& ptFare);

  void printFareHeader(DiagManager& diag,
                       PaxTypeFare* ptFare,
                       FareByRuleProcessingInfo& fbrProcessingInfo);
  Agent* getRecord8Agent(const VendorCode& vendor);
  bool checkQualifierPassed(const CategoryRuleItemInfo* const catRuleItemInfo,
                            DiagManager& diag,
                            bool qualifierPasses);

  void diagDisplayRelation(const CategoryRuleItemInfo* const catRuleItemInfo,
                           DiagManager& diag,
                           Record3ReturnTypes retCode);
  bool runCat35If1PreValidation(FareByRuleProcessingInfo& fbrProcessingInfo,
                                PaxTypeFare& dummyPtFare);
  void displayCat35If1PreValidation(FareByRuleProcessingInfo& fbrProcessingInfo) const;

  FareByRuleFareCreator _creator;
  FareByRuleBaseFareValidator _baseFareValidator;
  bool _isIndApplQueried = false;
  const std::vector<const IndustryFareAppl*>* _multiAppls = nullptr;
  const std::vector<const IndustryFareAppl*>* _indAppls = nullptr;
  const CarrierPreference* _carrierPref = nullptr;
  CurrencyCode _fmCurrency;
  CurrencyCode _fmOrigCurr;
  CurrencyCode _fmDestCurr;
  bool _isMinMaxFare = false;
  bool _qMatchCorpID = false; // qualified CAT1 matched Cord ID
  bool _qMatchNationFR = false; // qualified CAT15 matched 'Nation France'
  bool _qEtktWarning = false; // qualified CAT15 has Etkt warning.
  VecMap<uint32_t, QualifyFltAppRuleData*> _qualifyFltAppRuleDataMap;
  bool _matchedBkc1Avail = false;
  bool _passedBkc1Avail = false;
  bool _matchedBkc2Avail = false;
  bool _passedBkc2Avail = false;
  bool _diag325Requested;
  bool _diagWithRuleNumber;
  bool _diagWithR3Cat25;
  bool _diagWithT989;
  std::map<PaxTypeFare*, std::set<BookingCode> > _baseFareInfoBkcAvailMap;
  mutable std::map<const CarrierCode, FareMarket*> _carrierToAltFareMarketMap;
  std::vector<PaxTypeFare*> _fbrPaxTypeFares;
  std::vector<std::string> _failBaseFareList;
  bool _displayFareHdr = true;
  std::map<const VendorCode, Agent*> _r8AgentMap;
  const FareCollectorOrchestrator* _fco = nullptr;
  bool _spanishLargeFamilyDiscountApplied = false;
  MoneyAmount _largeFamilyDiscountPercent = 0;
  bool _spanishDiscountApplied = false;
  std::string _qNewAcctCode;
  bool _fallbackCat25baseFarePrevalidation = false;
};
} // tse
