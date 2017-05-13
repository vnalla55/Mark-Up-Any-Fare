//-------------------------------------------------------------------
//
//  File:        DiscountedFareController.h
//  Created:     May 12, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: Discounted fare factory
//
//  Updates:
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Fares/FareController.h"

#include <map>
#include <set>

namespace tse
{
class PricingTrx;
class FareMarket;
class PaxTypeFare;
class GeneralFareRuleInfo;
class CategoryRuleItemInfo;
class DiscountInfo;
class DiagCollector;

class DiscountedFareController : public FareController
{
  friend class DiscountedFareControllerTest;
  friend class MockDiscountedFareController;

  class FallBackOn
  {
  public:
    void operator()(DiscountedFareController& dfc,
                    FootNoteCtrlInfoVec& fnCtrlInfoVec,
                    PricingTrx& trx,
                    PaxTypeFare& paxTypeFare,
                    const TariffNumber& fareTariff,
                    const Footnote& footnote,
                    const uint16_t categoryNumber);
  };

  class FallBackOff
  {
  public:
    void operator()(DiscountedFareController& dfc,
                    FootNoteCtrlInfoVec& fnCtrlInfoVec,
                    PricingTrx& trx,
                    PaxTypeFare& paxTypeFare,
                    const TariffNumber& fareTariff,
                    const Footnote& footnote,
                    const uint16_t categoryNumber);
  };

public:
  DiscountedFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  DiscountedFareController(const DiscountedFareController&) = delete;
  DiscountedFareController& operator=(const DiscountedFareController&) = delete;

  bool process();

  void writeDiagnostics() const;

  void getFootNoteCtrlInfos(PaxTypeFare& paxTypeFare,
                            const uint16_t categoryNumber,
                            const Footnote& footnote,
                            const TariffNumber& fareTariff,
                            FootNoteCtrlInfoVec& fnCtrlInfo);

  bool getGenParam(const GeneralFareRuleInfo* ruleInfo,
                   PaxTypeFare& paxTypeFare,
                   const uint16_t categoryNumber,
                   TariffNumber& genTariff,
                   RuleNumber& genNumber);

  bool less_pt(const PaxTypeCode& x, const PaxTypeCode& y);
  void makeAlloc(PaxTypeFare* ptFare,
                 PaxTypeFare** newPTFare,
                 Fare** newFare,
                 FareInfo** fareInfo,
                 FareClassAppSegInfo** fcasInfo,
                 PaxTypeFareRuleData** ruleData);

  bool makeInfFare(PaxTypeFare& ptFare);

  using DiscRule = std::pair<const DiscountInfo*, PaxTypeFareRuleData*>;
  class DiscRules
  {
  public:
    using iter = std::vector<DiscRule>::iterator;

    const static int16_t CHK_ALL_EXIST_DISCRULE = -1;

    void addRule(PricingTrx& trx, const DiscountInfo* x, PaxTypeFareRuleData* y);
    bool hasPax(const PaxTypeCode& p, int16_t searchVecSz = CHK_ALL_EXIST_DISCRULE);
    //        bool hasDisc(const DiscountInfo *d);

    iter begin() { return _vec.begin(); }
    iter end() { return _vec.end(); }

    void clear() { _vec.clear(); }
    bool empty() const { return _vec.empty(); }
    size_t size() const { return _vec.size(); }

  private:
    std::vector<DiscRule> _vec;
  };

  static MoneyAmount calcPercentage(const MoneyAmount fareAmt, const MoneyAmount percent);

  // results in _calcMoney
  void calcAmount(PaxTypeFare& paxTypeFare, const DiscountInfo& discountInfo);

  const static char CALCULATED;
  const static char SPECIFIED;
  const static char ADD_CALCULATED_TO_SPECIFIED;
  const static char SUBTRACT_SPECIFIED_FROM_CALCULATED;

  const static char MATCH_ALWAYS;
  const static char MATCH_FARE_CLASS;
  const static char MATCH_FARE_TYPE;
  const static char MATCH_PAX_TYPE;
  const static char MUST_MATCH;
  const static char MUST_NOT_MATCH;
  const static std::string BLANK_CURRENCY;
  const static Indicator REQ_ACC_TVL; // for DiscountInfo.accInd()
  const static Indicator NOT_APPLY;
  const static MoneyAmount NO_MIN_AMT;

protected:
  template <class FallBackSwitch>
  void processFare(PaxTypeFare& ptFare);

  bool checkFare(PaxTypeFare* ptFare, DiagCollector& diag);

  bool getDiscInfoFromRule(PaxTypeFare& ptFare,
                           const CategoryRuleInfo* ruleInfo,
                           const bool isLocationSwapped,
                           bool allowDup = true);

  void
  processGeneralRule(const GeneralFareRuleInfo* gfrInfo, PaxTypeFare& ptFare, const uint16_t cat);

  PaxTypeFare* makeFare(PaxTypeFare* currPTFare, DiscRule& discRule, CalcMoney& cMoney);

  void addFare(PaxTypeFare* newPTFare);

  virtual bool validate(DiagCollector& diag,
                        PaxTypeFare& ptFare,
                        const CategoryRuleInfo& ruleInfo,
                        const DiscountInfo& discountInfo,
                        const CategoryRuleItemInfoSet& catRuleItemInfoSet);

  bool validateAccompanied(DiagCollector& diag,
                           const DiscountInfo& discountInfo,
                           PaxTypeFare& ptFare,
                           const CategoryRuleInfo& ruleInfo,
                           const PaxType& ptItem) const;

  bool validateCurrency(DiagCollector& diag,
                        const DiscountInfo& discountInfo,
                        const CategoryRuleItemInfoSet& catRuleItemInfoSet) const;

  void printPassValidate(DiagCollector& diag,
                         PaxTypeFare& ptFare,
                         const DiscountInfo& discountInfo,
                         const PaxType& ptItem) const;

  bool
  hasGoodBaseFare(const DiscountInfo& discountInfo, const PaxTypeFare& ptFare, DiagCollector& diag);

  PaxTypeFare* findCheapestMatchFare(const std::vector<PaxTypeFare*>& allPTF,
                                     const PaxTypeFare& rulePTF,
                                     const DiscountInfo& discountInfo) const;

  PaxTypeFare*
  findBaseFare(const PaxTypeFare& rulePTF,
               const DiscountInfo& discountInfo);

  bool
  matchFareDirectionality(const Directionality& basePTFDir, const Directionality& rulePTFDir) const;

  bool discAgeSensitive(const DiscountInfo& discInfo, const CarrierCode& carrier);

  void writeDiag219(DiagCollector& diag, PaxTypeFare& paxTypeFare) const;

  void writeDiag319(DiagCollector& diag,
                    const PaxTypeFare& paxTypeFare,
                    const CategoryRuleInfo& ruleInfo,
                    const DiscountInfo& discountInfo) const;

  void writeBaseFaresDiag319(DiagCollector& diag, std::vector<PaxTypeFare*>& ptFares) const;

  virtual const DiscountInfo*
  getDiscountInfo(const CategoryRuleInfo* ruleInfo, const CategoryRuleItemInfo* catRuleItemInfo);

  void createFares(PaxTypeFare& ptFare);

  void updateFareMarket(DiagCollector& diag);

  bool shouldProcessFare(const PaxTypeFare& ptFare) const;

  bool shouldProcessFbrFare(const PaxTypeFare& ptFare) const;

  void setNewPTFareCategoryStatus(PaxTypeFare& newPTFare,
                                  const PaxTypeFare& basePTFare,
                                  const uint32_t catNumber) const;

  std::vector<PaxTypeFare*> _discPaxTypeFares;

  bool _diag219On = false;
  bool _diag319On = false;
  DiscRules _discRules;
  DiagCollector* _diagPtr = nullptr;
  const bool _ageDefinedInTrxPax;
  std::map<PaxTypeCode, bool> _paxAgeSensitiveMap;
  std::map<const DiscountInfo*, PaxTypeFare*> _discountCalcHelper;
  bool _footNoteFailedFaresSorted = false;
};
} // namespace tse
