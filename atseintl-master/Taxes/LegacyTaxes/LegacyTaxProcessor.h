//---------------------------------------------------------------------------
//  Copyright Sabre 2009
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
#ifndef LEGACY_TAX_PROCESSOR_H
#define LEGACY_TAX_PROCESSOR_H

#include "Common/Thread/TseThreadingConst.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

#include <vector>
#include <unordered_set>

namespace tse
{
class DiagCollector;
class FarePath;
class Itin;
class Logger;
class PricingTrx;
class TaxItinerary;
class TaxTrx;

struct compareMoneyAmount  : public std::binary_function<MoneyAmount, MoneyAmount, bool>
{
   bool operator()(const MoneyAmount a, const MoneyAmount b) const
   {
      MoneyAmount diff = b - a;
      if( fabs(diff) < EPSILON )
         return false;
      return a < b;
   }
};

enum class SettlementPlanGroup {REG_SP=0, TCH_SP};

// Object for tax amount and tax response for a given FarePath for given sp group
struct SettlementPlanTaxAmount
{
  TaxResponse* taxResponse = nullptr;
  MoneyAmount totalAmount; // (nuc-amount + tax-toal) * num-pax
  SettlementPlanTaxAmount(TaxResponse* tr, MoneyAmount m)
    :taxResponse(tr), totalAmount(m) {}
};

typedef std::map<SettlementPlanGroup,
        std::vector<SettlementPlanTaxAmount>> SettlementPlanTaxAmountGroupMap;

// Object for total amount for a validating carrier across FarePath for settlement plan/s
struct ValidatingCxrTotalAmount
{
  // Sum of all total amounts across farePaths for given sp group and cxr
  MoneyAmount totalAmount=0.0;
  std::vector<SettlementPlanTaxAmount> spTaxAmountCol;
};

class LegacyTaxProcessor
{
  friend class LegacyTaxProcessorTest;
public:
  typedef std::map<MoneyAmount, std::vector< std::map<SettlementPlanType, CarrierCode>>, compareMoneyAmount> PriceSpValCxrsMap;

  LegacyTaxProcessor(const TseThreadingConst::TaskId taskId, Logger& logger);
  virtual ~LegacyTaxProcessor() = default;
  bool process_OLD(PricingTrx& trx);
  bool process(PricingTrx& trx);
  bool process(TaxTrx& trx);
  bool process(FareDisplayTrx& trx);
  bool processOTA(PricingTrx& trx);

protected:
  void handleValidatingCarrierError(PricingTrx& trx);
  bool oneFpPerPaxType(PricingTrx& trx, Itin& itin) const;
  void handleTaxResponsesAllItins(PricingTrx& trx);
  void handleTaxResponsesAllItins_OLD(PricingTrx& trx);
  void addTaxResponsesToTrx(PricingTrx& trx);
  void handleAdjustedSellingLevelFarePaths(PricingTrx& trx);
  void handleAdjustedSellingLevelFarePaths_OLD(PricingTrx& trx);

  bool isNetRemitFPNotValid(const FarePath* fp) const
  {
    return(!fp->validatingCarriers().empty() &&
           fp->defaultValidatingCarrier().empty() &&
           fp->netRemitFarePath() != nullptr  &&
           fp->processed() == false  );
  }

  // ValidatingCxr
  void findLowestFareSolutionAfterTax(PricingTrx& trx,
                                      TaxMap::TaxFactoryMap& taxFactoryMap);

  void findLowestFareSolutionAfterTax_OLD(PricingTrx& trx,
                                      TaxMap::TaxFactoryMap& taxFactoryMap);

  void findLowestFareSolutionForPaxType(PricingTrx& trx,
                                        Itin& itin,
                                        TaxItinerary& taxItinerary,
                                        DiagCollector& diag);
  void findLowestFareSolutionForGroupPaxType(PricingTrx& trx,
                                             Itin& itin,
                                             TaxItinerary& taxItinerary,
                                             DiagCollector& diag);

  void processFinalFP(PricingTrx& trx,
                      FarePath& fPath,
                      const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                      std::vector<CarrierCode>& valCxrWithLowestTotalAmt,
                      CarrierCode& defVCxr,
                      CarrierCode& mktCxr,
                      const SettlementPlanType& primarySp);

  void findValCxrWithLowestTotal(PricingTrx& trx,
                                 FarePath& fp,
                                 std::vector<CarrierCode>& vcVect,
                                 std::set<FarePath*>& fpSet);

  void findValCxrWithLowestTotal(PricingTrx& trx,
                                 FarePath& fp,
                                 SettlementPlanValCxrsMap& spValM,
                                 std::set<FarePath*>& fpSet);

  void updateDefaultValidatingCarrier(PricingTrx& trx, Itin& itin, FarePath& fp) const;

  virtual void processAdjustedSellingLevelFarePath(PricingTrx& trx, FarePath& fp) const;

  void setNetRemitAndNetFarePathForValidatingCxr(PricingTrx& trx,
                                                 Itin& itin,
                                                 FarePath& fp,
                                                 TaxItinerary& taxItinerary,
                                                 DiagCollector& diag) const;

  void findValCxrWithLowestTotal(PricingTrx& trx,
                                 FarePath& fp,
                                 std::vector<CarrierCode>& vcVect,
                                 std::set<FarePath*>& fpSet,
                                 MoneyAmount& currentLowTaxTotal);

  void findValCxrWithLowestTotal(PricingTrx& trx,
                                 FarePath& fp,
                                 SettlementPlanValCxrsMap& spValM,
                                 std::set<FarePath*>& fpSet,
                                 MoneyAmount& currentLowTaxTotal);

  void findValCxrWithLowestGroupTotal(PricingTrx& trx,
                                      Itin& itin,
                                      const std::vector<CarrierCode>& commonValCxrs,
                                      std::vector<CarrierCode>& vcVect);

  void findValCxrWithLowestGroupTotal(PricingTrx& trx,
                                      Itin& itin,
                                      const std::vector<CarrierCode>& commonValCxrs,
                                      SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt);

  FarePath* findFarePathWithValidatingCarrier(PricingTrx& trx, FarePath& farePath, const CarrierCode& cxr);

  FarePath* findFarePathWithDefaultValidatingCarrier(PricingTrx& trx,
                                                     TaxItinerary& taxItinerary,
                                                     FarePath& fp,
                                                     CarrierCode& defVCxr,
                                                     bool& needFPSwap,
                                                     DiagCollector& diag);

  FarePath* findFarePathWithNODefaultValidatingCarrier(PricingTrx& trx,
                                                       Itin& itin,
                                                       TaxItinerary& taxItinerary,
                                                       FarePath& fp,
                                                       std::vector<CarrierCode>& valCxrWithLowestTax,
                                                       bool& needFPSwap,
                                                       DiagCollector& diag);

  FarePath* findFarePathWithNODefaultValidatingCarrier(PricingTrx& trx,
                                                       Itin& itin,
                                                       TaxItinerary& taxItinerary,
                                                       FarePath& fp,
                                                       SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt,
                                                       bool& needFPSwap,
                                                       DiagCollector& diag,
                                                       const SettlementPlanType& sp);

  void findGroupFarePathWithDefaultValidatingCarrier(PricingTrx& trx,
                                                     Itin& itin,
                                                     TaxItinerary& taxItinerary,
                                                     CarrierCode& defVCxr,
                                                     std::vector<FarePath*>& finalFPVect,
                                                     bool& needFPSwap,
                                                     DiagCollector& diag);

  void findGroupFarePathWithNODefaultValidatingCarrier(PricingTrx& trx,
                                                       Itin& itin,
                                                       TaxItinerary& taxItinerary,
                                                       std::vector<CarrierCode>& valCxrWithLowestTax,
                                                       std::vector<FarePath*>& finalFPVect,
                                                       bool& needFPSwap,
                                                       DiagCollector& diag);

  void findGroupFarePathWithNODefaultValidatingCarrier(PricingTrx& trx,
                                                       Itin& itin,
                                                       TaxItinerary& taxItinerary,
                                                       SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                                                       std::vector<FarePath*>& finalFPVect,
                                                       bool& needFPSwap,
                                                       DiagCollector& diag,
                                                       const SettlementPlanType& primarySp);

  void setDefaultAndAlternateValCarriers(PricingTrx& trx,
                                         FarePath& farePath,
                                         std::vector<CarrierCode>& valCxrWithLowestTax,
                                         CarrierCode& defVCxr,
                                         CarrierCode& mktCxr);

  void setDefaultAndAlternateValCarriers(PricingTrx& trx,
                                         FarePath& farePath,
                                         SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt,
                                         const CarrierCode& defVCxr,
                                         const CarrierCode& mktCxr,
                                         const SettlementPlanType& primarySp);

  void setDefaultAndAlternateValCarriersForGroupPaxType(PricingTrx& trx,
                                                        std::vector<FarePath*>& finalFPVect,
                                                        CarrierCode& defVCxr,
                                                        CarrierCode& mktCxr);

  void setDefaultAndAlternateValCarriersForGroupPaxType(PricingTrx& trx,
                                                        std::vector<FarePath*>& finalFPVect,
                                                        const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                                                        CarrierCode& defVCxr,
                                                        CarrierCode& mktCxr,
                                                        const SettlementPlanType& primarySp);

  bool sameZeroTaxes(const TaxResponse& taxRspCurrent, const TaxResponse& taxRspInMap) const;
  bool sameTaxes(const TaxResponse& taxRspCurrent, const TaxResponse& taxRspInMap) const;
  void findCommonValidatingCxrs(PricingTrx& trx,
                                std::vector<FarePath*>& farePathVect,
                                std::vector<CarrierCode>& res) const;

  void getValidValCxr(PricingTrx& trx, FarePath& fp, std::vector<CarrierCode>& vcVect) const;
  void throwIfTicketingEntry(PricingTrx& trx, std::vector<CarrierCode>& vcVect);

  void throwIfTicketingEntry(PricingTrx& trx, const SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt);
  TaxResponse* getTaxResponseForSp(std::vector<TaxResponse*>& taxResponses,
                                   const SettlementPlanType& sp) const;
  SettlementPlanType findPrimarySp(const SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt) const;
  void setDefaultPerSp(PricingTrx& trx,
                       FarePath& farePath,
                       SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt,
                       const SettlementPlanType& primarySp) const;
  void setAlternatePerSp(PricingTrx& trx, FarePath& farePath,
                         TaxResponse* primaryTaxResposne,
                         const SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt) const;

  void copyAllValCxrsWithNoDefaultValCxr(FarePath& fp,
                                         const SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt) const;
  void findSpValCxrsWithLowestTotal(PricingTrx& trx,
                        FarePath& farePath,
                        SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt) const;
  void collectTaxAndBaggageForAdditionalFPs(PricingTrx& trx,
                                            Itin& itin,
                                            TaxItinerary& taxItinerary,
                                            DiagCollector& diag,
                                            FarePath* finalFp,
                                            FarePath& fp,
                                            TaxResponse* taxResponse,
                                            const CarrierCode& cxr,
                                            bool& needFPSwap) const;
  void findSpValCxrsForFinalFp(PricingTrx& trx,
                               FarePath& finalFp,
                               const SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt) const;
  void findSpValCxrsForFinalFp(PricingTrx& trx,
                               FarePath& finalFp,
                               const SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt,
                               const std::vector<CarrierCode>& commonValCxrs) const;
  bool isTaxCollectedForSp(const std::vector<TaxResponse*>& taxResponses,
                           const SettlementPlanType& sp,
                           const CarrierCode& valCxr) const;
  void modifyItinTaxResponseForFinalFarePath(FarePath& finalFp) const;
  bool isFinalSettlementPlanInTaxResponse(TaxResponse& taxResp,
                                          const SettlementPlanValCxrsMap& spValCxrs) const;
  bool handleNetRemitForMultiSp(FarePath& finalFp,
                                const SettlementPlanValCxrsMap& spValCxrWithLowestTotalAmt) const;
  bool suppressSettlementPlanGTC(PricingTrx& trx,
                                 const SettlementPlanType& sp,
                                 size_t numOfSpInFinalSolution) const;

  void checkTaxComponentsAndMerge(
      std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
      const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol) const;

  void setSpValCxrsMap(
    const CarrierCode& cxr,
    const std::vector<SettlementPlanTaxAmount>& spTaxAmountCol,
    SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const;

  void processValidatingCxrTotalAmount(
      const std::map<CarrierCode, ValidatingCxrTotalAmount>& valCxrTotalAmtPerCxr,
      SettlementPlanValCxrsMap& spValCxrsWithLowestTotalAmt) const;

  bool checkSpIndex(
    const TaxResponse& taxResp,
    const SettlementPlanType& sp,
    size_t& ind) const;

  bool checkSettlementPlanHierarchy(
      const std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
      const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol,
      SettlementPlanType& higherSp) const;

  void clearSettlementPlanData(
    std::vector<SettlementPlanTaxAmount>& v,
    const SettlementPlanType& sp) const;

  bool canMergeTaxResponses(
      const std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
      const std::vector<SettlementPlanTaxAmount>& nextSpTaxAmountCol) const;

  MoneyAmount getSpTotalAmount(const std::vector<SettlementPlanTaxAmount>& spTaxAmtCol) const;

  void processSettlementPlanTaxData(
      MoneyAmount& currentLowestTotal,
      std::vector<SettlementPlanTaxAmount>& currentSpTaxAmountCol,
      const SettlementPlanTaxAmountGroupMap& spTaxAmtGroup) const;

  void findSpTotalAmount(
    const FarePath& farePath,
    std::vector<TaxResponse*>& taxResponses,
    SettlementPlanTaxAmountGroupMap& spTaxAmtGroup) const;

  void addTaxInfoResponsesToTrx(PricingTrx& trx) const;

  void setTaxResponseInItin(const PricingTrx& trx,
                            FarePath& farePath,
                            TaxResponse* taxResponse) const;

private:
  const TseThreadingConst::TaskId _taskId;
  Logger& _logger;

  LegacyTaxProcessor(const LegacyTaxProcessor&);
  LegacyTaxProcessor& operator=(const LegacyTaxProcessor&);
};

} // namespace tse

#endif // LEGACY_TAX_PROCESSOR_H
