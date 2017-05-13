//-------------------------------------------------------------------
//
//  Description: Fare Collector Orchestrator
//
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
//-------------------------------------------------------------------

#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/TSEAlgorithms.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "Fares/FareOrchestrator.h"

#include <boost/iterator/filter_iterator.hpp>

namespace tse
{
class AddonFareController;
class AltPricingTrx;
class CarrierFareController;
class DataHandle;
class DiagCollector;
class ExchangePricingTrx;
class ExcItin;
class Fare;
class FareDisplayTrx;
class FareMarket;
class FlightFinderTrx;
class IndustryFareController;
class MetricsTrx;
class NoPNRPricingTrx;
class PaxTypeFareRuleData;
class PricingTrx;
class RefundPricingTrx;
class RepricingTrx;
class RexBaseTrx;
class RexExchangeTrx;
class RexPricingTrx;
class RexShoppingTrx;
class ShoppingTrx;
class StructuredRuleTrx;

class FareCollectorOrchestrator final : public FareOrchestrator
{
  friend class FareCollectorOrchestratorTest;
  friend class DummyFareTest;
  friend class FFFailedFaresTest;
public:
  FareCollectorOrchestrator(const std::string& name, TseServer& server);
  virtual ~FareCollectorOrchestrator();

  virtual bool process(MetricsTrx& trx) override;
  virtual bool process(PricingTrx& trx) override;
  virtual bool process(ShoppingTrx& trx) override;
  virtual bool process(RepricingTrx& trx) override;
  virtual bool process(FareDisplayTrx& trx) override;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(FlightFinderTrx& trx) override;
  virtual bool process(RexShoppingTrx& trx) override;
  virtual bool process(RexBaseTrx& trx);
  virtual bool process(RefundPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(StructuredRuleTrx& trx) override;

  void publishedFaresStep(PricingTrx& trx,
                          Itin& itin,
                          FareMarket& fareMarket,
                          bool toSort = false) const;
  void publishedFaresStepFareDisplay(PricingTrx& trx,
                                     Itin& itin,
                                     FareMarket& fareMarket,
                                     bool toSort = false) const;

  static void sortStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void selectCurrencyStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

private:
  void fillDummyFM(PricingTrx& trx);

  bool removeCat5(PricingTrx& trx);

  void allFareMarketSteps(PricingTrx& trx, Itin& itin, FareMarket& fareMarket) const;

  void allFareMarketStepsFareDisplay(PricingTrx& trx, Itin& itin, FareMarket& fareMarket) const;

  static void diagnostic451Step(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  bool processFRRAdjustedSellingLevel(FareDisplayTrx& trx);
  static void ruleValidator(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void releaseFFinderInvalidFares(FlightFinderTrx& trx,
                                         Itin& itin,
                                         FareMarket& fareMarket,
                                         DiagCollector& diag,
                                         std::map<PaxTypeCode, bool>& psgPrevalidationStatus);

  static void validateFFinderPsgType(FlightFinderTrx& trx,
                                     Itin& itin,
                                     FareMarket& fareMarket,
                                     DiagCollector& diag,
                                     std::map<PaxTypeCode, bool>& psgPrevalidationStatus);

  static void
  setPrevalidationStatusAndFBRBaseFares(FlightFinderTrx& trx,
                                        std::vector<PaxTypeFare*>& allPaxTypeFares,
                                        DiagCollector& diag,
                                        std::map<PaxTypeCode, bool>& psgPrevalidationStatus,
                                        std::set<const PaxTypeFare*>& baseFares,
                                        bool& hasIndustryFareFailed);

  static void releaseFaresStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void InvokeStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void setProcessedStepForDummy(PricingTrx& trx, bool mark);
  static void setProcessedStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket, bool set);
  static void markProcessedStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void markProcessedStepDummy(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void unmarkProcessedStepDummy(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void fillDummyFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static bool selectCurrency(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  bool findPublishedAddOnIndustryFares(PricingTrx& trx,
                                       Itin& itin,
                                       FareMarket& fareMarket,
                                       CarrierFareController& carrierFareController,
                                       AddonFareController& addonFareController,
                                       IndustryFareController& indFareController,
                                       bool sort = true) const;

  static bool
  findDiscountedFares(PricingTrx& trx, Itin& itin, FareMarket& fareMarket, bool sort = false);
  bool
  findFareByRuleFares(PricingTrx& trx, Itin& itin, FareMarket& fareMarket, bool sort = false) const;
  static bool
  findNegotiatedFares(PricingTrx& trx, Itin& itin, FareMarket& fareMarket, bool sort = false);

  static void
  findDuplicateFareMarkets(PricingTrx& trx,
                           std::map<std::string, std::vector<FareMarket*>>& fmMap,
                           std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs,
                           std::map<FareMarket*, std::vector<int>>* fmOrigBrands,
                           ErrorResponseException::ErrorResponseCode* duplicateStatus = nullptr);

  static void mergeBrands(std::vector<int>& brand1, std::vector<int>& brand2);

  static void
  mergeValidatingCarriers(std::vector<CarrierCode>& vcs1, const std::vector<CarrierCode>& vcs2);

  static std::string getConnectingCity(std::vector<TravelSeg*>& travelSegVec);

  void processInterlineBrandedFares(PricingTrx& trx);

  void validateCommonFareMarkets(PricingTrx& trx,
                                 std::map<std::string, std::vector<FareMarket*>>& fmMap);

  void restoreOrigBrands(std::map<std::string, std::vector<FareMarket*>>& fmMap,
                         const std::map<FareMarket*, std::vector<int>>* fmOrigBrands) const;

  void restoreOrigValidatingCarriers(
      std::map<std::string, std::vector<FareMarket*>>& fmMap,
      const std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs) const;

  void restorePtfBrands(FareMarket* fm,
                        const std::vector<int>& mergedBrands,
                        const std::vector<int>& origBrands) const;

  void restorePtfVCs(FareMarket* fm,
                     const std::vector<CarrierCode>& mergedVCs,
                     const std::vector<CarrierCode>& origVCs) const;

  void copyDuplicateFareMarkets(
      PricingTrx& trx,
      std::map<std::string, std::vector<FareMarket*>>& fmMap,
      const std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs = nullptr,
      const std::map<FareMarket*, std::vector<int>>* fmOrigBrands = nullptr) const;

  void copyFareMarket(PricingTrx& trx, FareMarket& src, FareMarket& dest) const;

  PaxTypeFare* copyPaxTypeFare(DataHandle& dataHandle,
                               PaxTypeFare& paxTypeFare,
                               FareMarket& fareMarket,
                               std::map<Fare*, Fare*>& fareMap,
                               std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap,
                               PricingTrx& trx,
                               uint16_t mileageSrc,
                               uint16_t mileageDest) const;

  PaxTypeFareRuleData* copyPaxTypeFareRuleData(DataHandle& dataHandle,
                                               PaxTypeFareRuleData* from,
                                               FareMarket& dest,
                                               std::map<Fare*, Fare*>& fareMap,
                                               std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap,
                                               PricingTrx& trx,
                                               uint16_t mileageSrc,
                                               uint16_t mileageDest) const;

  static void setupFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void setCxrPref(PricingTrx& trx, FareMarket& fm);

  static void checkFaresStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void checkBrandedFareDataStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  void preloadFares(PricingTrx& trx);
  void fillFaresWithBrands_impl(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets);
  void fillFaresWithBrands(PricingTrx& trx);
  bool isThruFareMarket(PricingTrx& trx, FareMarket& fm);
  void failSoftPassedBrands(FareMarket& fareMarket);

  static void initAltDates(const ShoppingTrx& trx, FareMarket& fm);

  static bool checkAltDateIsNOTEffectiveFare(PaxTypeFare* ptFare);
  static bool altDateCAT15Validation(PricingTrx& trx, FareMarket& dest, PaxTypeFare* ptFare);

  bool isDSSCallNeeded(FareDisplayTrx& trx);

  static bool findSpecialRouting(const FareMarket& fareMarket, const bool checkEmptyRouting = true);

  static bool findSpecialRouting(const std::vector<PaxTypeFare*>& paxTypeFares,
                                 const bool checkEmptyRouting = true);

  static void findFaresWithUniqueRoutings(PricingTrx& trx, FareMarket& fareMarket);

  static bool
  specialRoutingFoundInRequestedFares(PricingTrx& trx, DiagCollector& diag, FareMarket& fareMarket);

  bool validateFareTypePricing(PricingTrx& trx);

  static void countFares(PricingTrx& trx);
  static void removeDuplicateFares(PricingTrx& trx, Itin& itin, FareMarket& fm);
  static void invalidateFaresWithoutBrands(PricingTrx& trx, Itin& itin, FareMarket& fm);
  static void checkFareByRuleJCBFares(PricingTrx& trx, Itin& itin, FareMarket& fm);
  static bool dupFare(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2, const PricingTrx& trx);
  /* RemovedDuplicateFare assumes passed removedPTFVect to be sorted */
  class RemovedDuplicateFare
  {
  public:
    RemovedDuplicateFare(std::vector<PaxTypeFare*>& removedPTFVect)
    : _removedPTFVect(removedPTFVect)
    {
    }
    bool operator()(PaxTypeFare* ptf)
    {
      if (LIKELY(_removedPTFVect.empty()))
        return false;
      return std::binary_search(_removedPTFVect.begin(), _removedPTFVect.end(), ptf);
    }
  private:
    std::vector<PaxTypeFare*>& _removedPTFVect;
  };

  bool process(RexPricingTrx& trx, const FareMarket::FareRetrievalFlags fareRetrievalFlag);

  struct IsKeepFare
  {
    IsKeepFare(const PaxTypeFare& excItinFare, const DateTime& retrievalDate);

    bool operator()(PaxTypeFare* fare);

  private:
    const PaxTypeFare& _excItinFare;
    const DateTime& _retrievalDate;
    PaxTypeCode _excItinFarePaxType;
  };

  struct IsExpndKeepFare
  {
    IsExpndKeepFare(const PaxTypeFare& excItinFare, const DateTime& retrievalDate);

    bool operator()(PaxTypeFare* fare);

  private:
    const PaxTypeFare& _excItinFare;
    const DateTime& _retrievalDate;
    PaxTypeCode _excItinFarePaxType;
  };

  void cloneNewFMsWithFareApplication(RexPricingTrx& trx, Itin& newItin);

  bool keepFareAloneInFM(RexPricingTrx& trx,
                         std::vector<FareMarket*>& dupFareMarkets,
                         FareMarket& fareMarket);
  void retrieveOtherThenKeepFare(RexPricingTrx& trx,
                                 std::vector<FareMarket*>& dupFareMarkets,
                                 FareMarket& fareMarket,
                                 const DateTime& date,
                                 FareMarket::FareRetrievalFlags flags,
                                 bool& currFareMarketUpdated);
  FareMarket& duplicateFareMarket(RexPricingTrx& trx,
                                  std::vector<FareMarket*>& dupFareMarkets,
                                  const FareMarket& origFM,
                                  const DateTime& date,
                                  FareMarket::FareRetrievalFlags flags);
  void updateFaresRetrievalInfo(RexBaseTrx& trx);
  void updateFaresRetrievalInfo(RexPricingTrx& trx);
  void updateKeepFareRetrievalInfo(RexPricingTrx& trx, FareMarket& fareMarket) const;
  void setKeepFareRetrievalInfo(RexPricingTrx& trx, FareMarket& fareMarket) const;
  bool historicalCommence(const FareMarket::FareRetrievalFlags& flags,
                          const DateTime& retrievDt,
                          const RexPricingTrx& trx) const;

  template <typename Op>
  void forEachKeepFare(std::vector<PaxTypeFare*>::iterator begin,
                       std::vector<PaxTypeFare*>::iterator end,
                       const PaxTypeFare& excItinFare,
                       Op operation) const;
  template <typename Op>
  void forEachExpndKeepFare(std::vector<PaxTypeFare*>::iterator begin,
                            std::vector<PaxTypeFare*>::iterator end,
                            const PaxTypeFare& excItinFare,
                            Op operation) const;

  const PaxTypeFare*
  getKeepFareForNewFareMarket(const RexPricingTrx& trx, const FareMarket* fareMarket) const;

  bool isGIValid(NoPNRPricingTrx& noPNRTrx);

  void collectFaresStep(PricingTrx& trx,
                        std::map<std::string, std::vector<FareMarket*>>& fmMap,
                        std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs = nullptr,
                        std::map<FareMarket*, std::vector<int>>* fmOrigBrands = nullptr);

  void releaseCheckSortMarkStepForDummy(PricingTrx& trx);

  void releaseCheckSortMarkStep(
      PricingTrx& trx,
      std::map<std::string, std::vector<FareMarket*>>& fmMap,
      const std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs = nullptr,
      const std::map<FareMarket*, std::vector<int>>* fmOrigBrands = nullptr);

  static int removeNotEffPTFares(PricingTrx& trx, std::vector<PaxTypeFare*>& ptFareVec);
  static void performDiagnostic981(PricingTrx& trx, FareMarket* fm);
  static void
  performDiagnostic981(PricingTrx& trx, std::map<PaxTypeFare*, PaxTypeFare*>& ptFareTDateMap);
  static void accumulateDataDiag981(PaxTypeFare* ptFare);
  static std::map<PaxTypeFare*, PaxTypeFare*>& getAccaccumulateDataDiag981();
  static void clearAccaccumulateDataDiag981();

  void duplicateAndOverrideFareMarket(RexPricingTrx& trx,
                                      std::vector<FareMarket*>& dupFareMarkets,
                                      const FareMarket& origFM,
                                      const DateTime& date,
                                      FareMarket::FareRetrievalFlags flags);
  void retrieveOtherThenKeepFareForMip(RexPricingTrx& trx,
                                       std::vector<FareMarket*>& dupFareMarkets,
                                       FareMarket& fareMarket,
                                       const DateTime& date,
                                       FareMarket::FareRetrievalFlags flags,
                                       bool& currFareMarketUpdated);
  void retrieveOtherThenKeepFareFacade(RexPricingTrx& trx,
                                       std::vector<FareMarket*>& dupFareMarkets,
                                       FareMarket& fareMarket,
                                       const DateTime& date,
                                       FareMarket::FareRetrievalFlags flags,
                                       bool& currFareMarketUpdated);

  static bool retrieveNegFares(const PricingTrx& trx);
  bool
  retrieveFbrFares(const PricingTrx& trx, bool isSolTrx, FareMarket::SOL_FM_TYPE fmTypeSol) const;
  void splitSharedFareMarketsWithDiffROE(RexExchangeTrx& rexExcTrx);
  void cloneFareMarket(RexExchangeTrx& rexExcTrx, FareMarket*& origFM, FareMarket*& newFM);
  void invalidateSomeFareMarkets(RexPricingTrx& trx);
  int16_t findMaxSegmentOrderForFlownInExc(const ExcItin* excItin) const;

  void recalculateCat25FareAmount(PricingTrx& trx,
                                  Itin* itin,
                                  FareMarket& fareMarket,
                                  const CurrencyCode& currencyCode,
                                  const FareByRuleItemInfo& fbrItemInfo,
                                  uint16_t mileage,
                                  PaxTypeFare& paxTypeFare) const;

  /**
   * @return true if in SOL and faremarket applicable sops are online.
   */
  bool
  checkAllApplicableSopsAreOnlineInSol(const PricingTrx& trx, const FareMarket& fareMarket) const;

  void removeFaresInvalidForHip(RexBaseTrx& trx);

  void processNewItins(RexPricingTrx& trx);
  void processNewItin(RexPricingTrx& trx,
                      Itin& newItin,
                      std::vector<std::vector<FareMarket*>>& dupFareMarketsVec);
  void updateNewItinRelatedFareMarkets(RexPricingTrx& trx,
                                       Itin& newItin,
                                       std::vector<std::vector<FareMarket*>>& dupFareMarketsVec);

  template <typename Buckets, typename Validator>
  void collectValidNegFares(const std::vector<Buckets>& buckets,
                            Validator validator,
                            std::set<PaxTypeFare*>& result) const;

  static const FareType DUMMY_FARE_TYPE;

  FareCollectorOrchestrator(const FareCollectorOrchestrator& rhs);
  FareCollectorOrchestrator& operator=(const FareCollectorOrchestrator& rhs);
  FareCollectorOrchestrator(TseServer& server, const std::string& name) // for unit tests only!!!
      : FareOrchestrator(name, server, TseThreadingConst::FARESC_TASK)
  {
  }

  TrxItinFareMarketFunctor allFareMarketStepsFunctor();
  static void applyDiscountPercentToFare(MoneyAmount discountPercent, MoneyAmount& fareAmount);
  void applySpanishFamilyDisountToFaresForIS(PricingTrx& trx,
                                             PaxTypeFare* ptf,
                                             MoneyAmount percentDiscount) const;
  void checkSpanishDiscountForIS(PricingTrx& trx, Itin& itin, FareMarket& fareMarket) const;
  TrxItinFareMarketFunctor checkSpanishDiscountForISFunctor();

  static bool isSpanishFamilyDiscountAlreadyApplied(const PaxTypeFare*);
  void invalidateContinentBasedFares(PricingTrx& trx, FareMarket& fareMarket) const;
  void verifySpecificFareBasis(PricingTrx& trx, FareMarket& fareMarket) const;

  template <typename Buckets>
  void setUpNegFaresPostValidation(const FareMarket&,
                                   const Buckets&,
                                   const PricingTrx&,
                                   const std::vector<Itin*>& owningItins,
                                   std::set<PaxTypeFare*>&,
                                   bool&) const;

  static bool _indicator;
  static std::map<PaxTypeFare*, PaxTypeFare*> accDiag981ptFaresMap;
  bool _preloadFares = false;
  bool _skipFmAddonConstructionWhereAllSopsAreOnlineSol = false; // ACMS
  bool _skipFareByRuleConstructionSolFM = false; // ACMS
};

template <typename Op>
void
FareCollectorOrchestrator::forEachKeepFare(std::vector<PaxTypeFare*>::iterator begin,
                                           std::vector<PaxTypeFare*>::iterator end,
                                           const PaxTypeFare& excItinFare,
                                           Op operation) const
{
  IsKeepFare isKeepFare(excItinFare, excItinFare.retrievalDate());

  boost::filter_iterator<IsKeepFare, std::vector<PaxTypeFare*>::iterator> filter_begin(
      isKeepFare, begin, end),
      filter_end(isKeepFare, end, end);

  std::for_each(filter_begin, filter_end, operation);
}

template <typename Op>
void
FareCollectorOrchestrator::forEachExpndKeepFare(std::vector<PaxTypeFare*>::iterator begin,
                                                std::vector<PaxTypeFare*>::iterator end,
                                                const PaxTypeFare& excItinFare,
                                                Op operation) const
{
  IsExpndKeepFare isExpndKeepFare(excItinFare, excItinFare.retrievalDate());

  boost::filter_iterator<IsExpndKeepFare, std::vector<PaxTypeFare*>::iterator> filter_begin(
      isExpndKeepFare, begin, end),
      filter_end(isExpndKeepFare, end, end);

  std::for_each(filter_begin, filter_end, operation);
}

template <typename Buckets, typename Validator>
void
FareCollectorOrchestrator::collectValidNegFares(const std::vector<Buckets>& bucketsVec,
                                                Validator validator,
                                                std::set<PaxTypeFare*>& result) const
{
  result.clear();
  std::set<PaxTypeFare*> partialResult;
  for (const Buckets& buckets : bucketsVec)
  {
    buckets->collectValidNegFares(validator, partialResult);
    result.insert(partialResult.begin(), partialResult.end());
  }
}

} // tse
