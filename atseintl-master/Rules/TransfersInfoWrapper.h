//-------------------------------------------------------------------
//
//  File:        TransfersInfoWrapper.h
//  Created:     May 24, 2006
//  Authors:     Andrew Ahmad
//
//  Description: Version 2 of Transfers processing code. This
//               second version is required for processing the
//               new Cat-09 record format as mandated by ATPCO.
//
//
//  Copyright Sabre 2004-2006
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

#include "Common/TseEnums.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "Rules/RuleConst.h"
#include "Rules/TransfersTravelSegWrapper.h"

#include <set>
#include <vector>

namespace tse
{
class DiagCollector;
class FarePath;
class FareUsage;
class Logger;
class Money;
class PaxType;
class PaxTypeFare;
class PricingTrx;
class PricingUnit;
class RuleItemCaller;
class RuleSetPreprocessor;
class TravelSeg;
class VCTR;

class TransfersInfoWrapper : public RuleItemInfo
{
  friend class TransfersInfoWrapperTest;

public:
  explicit TransfersInfoWrapper(FareUsage* fu = nullptr) : _fareUsage(fu) {}

  constexpr static int16_t TOTAL_MAX_EXCEED = 1;
  constexpr static int16_t TOTAL_IO_EXCEED = 2;

  const TransfersInfo1* trInfo() const { return _trInfo; }
  // Legacy method (hopefully will be merged with the next one once everything related is fixed.
  void setCurrentTrInfo(const TransfersInfo1* trInfo);
  void setCurrentTrInfo(const TransfersInfo1* ti,
                        const RuleItemCaller& ruleItemCaller,
                        bool isLocSwapped);

  const CategoryRuleInfo* crInfo() const { return _crInfo; }
  void crInfo(const CategoryRuleInfo* crInfo);

  void setNoTransfersMax(uint16_t num) { _noTransfersMax = num; }
  uint16_t noTransfersMax() const { return _noTransfersMax; }

  bool& isFareRule() const { return _isFareRule; }
  bool& isFareRule() { return _isFareRule; }

  bool& transfersRuleExistsInSet() const { return _transfersRuleExistsInSet; }
  bool& transfersRuleExistsInSet() { return _transfersRuleExistsInSet; }

  bool& ignoreNoMatch() const { return _ignoreNoMatch; }
  bool& ignoreNoMatch() { return _ignoreNoMatch; }

  bool& needToProcessResults() const { return _needToProcessResults; }
  bool& needToProcessResults() { return _needToProcessResults; }

  int16_t& initCharge1Count() const { return _charge1Count; }
  int16_t& initCharge2Count() const { return _charge2Count; }
  const int16_t charge2Count() const { return _charge2Count; }

  bool& applyLeastRestrictiveProvision() const { return _applyLeastRestrictiveProvision; }
  bool& applyLeastRestrictiveProvision() { return _applyLeastRestrictiveProvision; }

  int16_t& leastRestrictiveTransfersPermitted() const
  {
    return _leastRestrictiveTransfersPermitted;
  }
  int16_t& leastRestrictiveTransfersPermitted() { return _leastRestrictiveTransfersPermitted; }

  bool& leastRestrictiveTransfersUnlimited() const { return _leastRestrictiveTransfersUnlimited; }
  bool& leastRestrictiveTransfersUnlimited() { return _leastRestrictiveTransfersUnlimited; }

  bool& transferFCscope() const { return _transferFCscope; }
  bool& transferFCscope() { return _transferFCscope; }

  bool& transferFailsPU() const { return _transferFailsPU; }
  bool& transferFailsPU() { return _transferFailsPU; }

  bool& recurringFCscope() const { return _recurringFCscope; }
  bool& recurringFCscope() { return _recurringFCscope; }

  bool transferFCscopeReal(const PricingTrx& trx) const;

  bool& transferFCscopeInitial() const { return _transferFCscopeInitial; }
  bool& transferFCscopeInitial() { return _transferFCscopeInitial; }

  bool& transferFailsPUInitial() const { return _transferFailsPUInitial; }
  bool& transferFailsPUInitial() { return _transferFailsPUInitial; }

  void setIsTransfer(TravelSeg* ts, FareUsage* fu, bool val) const;

  bool checkIsTransfer(TravelSeg* ts) const;

  void setPassedValidation(TravelSeg* ts,
                           FareUsage* fu,
                           bool passedByLeastRestrictive,
                           bool isRecurringFCScope) const;

  bool checkPassedValidation(TravelSeg* ts) const;

  void setRuleItemMatch(TravelSeg* ts,
                        FareUsage* fu,
                        const uint32_t itemNo,
                        const bool isTentativeMatch) const;

  uint32_t getRuleItemMatch(const TravelSeg* ts) const;

  uint32_t getRuleItemMatch(const TravelSeg* ts, bool& isTentativeMatch) const;

  void
  doRuleSetPreprocessing(PricingTrx& trx, const RuleSetPreprocessor& rsp, const PricingUnit& pu);

  bool addSurcharge(PricingTrx& trx,
                    TravelSeg* ts,
                    FareUsage* fu,
                    const PaxTypeCode& paxTypeCode,
                    const bool isSegmentSpecific = false,
                    const bool forceCharge2 = false) const;

  Record3ReturnTypes processResults(PricingTrx& trx, const PaxTypeFare& ptf);

  Record3ReturnTypes processResults(PricingTrx& trx,
                                    FarePath& fp,
                                    const PricingUnit& pu,
                                    const FareUsage& fu,
                                    const bool processCharges = true);

  void processSurcharges(PricingTrx& trx,
                         FarePath& fp,
                         const FareUsage& fareUsage,
                         DiagCollector* diagPtr);

  bool checkAllPassed(); // true if all travel segs have PASSed
  void clearResults();

  bool isSameVCTR(const FareUsage& fu) const;
  const VCTR& getVCTR() const;

  bool hasPricingUnitScope(const FareUsage* fu) const;

  void noMatchValidation(const PricingUnit& pu, const FareUsage& fu);

  void
  setMostRestrictiveMaxTransfer(PricingTrx& trx, PricingUnit* pu, const TransfersInfo1* trInfo);
  void setMostRestrictiveMaxTransferForIndAnd(PricingUnit* pu, const FareUsage& fareUsage);

  bool checkMaxExceed(const PricingUnit& pu) const;

  void setMaxExceeded(TravelSeg* ts, FareUsage* fu, const int16_t) const;

  bool isRelationAndExists() const { return _isRelationAndExists; }
  bool isRelationOrExists() const { return _isRelationOrExists; }

protected:
  struct Charge
  {
    explicit Charge(bool s);
    Charge(const MoneyAmount& a1,
           const MoneyAmount& a2,
           const CurrencyCode& c,
           const CurrencyNoDec& d,
           bool s = true);
    void assign(const MoneyAmount& a1,
                const MoneyAmount& a2,
                const CurrencyCode& c,
                const CurrencyNoDec& d,
                bool s = true);

    MoneyAmount first, second;
    CurrencyCode currency;
    CurrencyNoDec noDec;
    bool status;
  };

  struct Surcharge
  {
    const uint32_t matchRuleItemNo;
    const bool isCharge1;
  };

  using SurchargeVector = std::vector<Surcharge>;

  void detectRelationIndicators();
  void detectRelationIndicators(const CategoryRuleItemInfoSet& set);
  void calcNoTransfersForThenAnd(const RuleItemCaller& ruleItemCaller,
                                 bool isLocSwapped);

  void copyFrom(const RuleItemInfo& ruleItemInfo);

  bool selectApplCharge(PricingTrx& trx,
                        MoneyAmount& charge1Amt,
                        CurrencyCode& charge1Cur,
                        int16_t& charge1NoDec,
                        MoneyAmount& charge2Amt,
                        CurrencyCode& charge2Cur,
                        int16_t& charge2NoDec,
                        const PaxTypeFare& fare,
                        const PaxTypeCode& paxTypeCode) const;

  Charge selectApplChargeImpl(PricingTrx& trx,
                              const PaxTypeFare& fare,
                              const PaxTypeCode& paxTypeCode) const;

  bool convertChargeToNUC(PricingTrx& trx, bool isInternational, Charge& charge) const;

  bool applyPaxTypeDiscount(PricingTrx& trx,
                            MoneyAmount& charge1Amt,
                            CurrencyCode& charge1Cur,
                            int16_t& charge1NoDec,
                            MoneyAmount& charge2Amt,
                            CurrencyCode& charge2Cur,
                            int16_t& charge2NoDec,
                            const FareUsage& fareUsage,
                            const PaxTypeCode& paxTypeCode,
                            const Indicator appl,
                            double& multiplier) const;

  bool convertDiscountToNUCamounts(PricingTrx& trx,
                                   MoneyAmount& firstAmt,
                                   CurrencyCode& firstCur,
                                   int16_t& firstNoDec,
                                   MoneyAmount& addAmt,
                                   CurrencyCode& addCur,
                                   int16_t& addNoDec,
                                   PaxTypeFare& fare,
                                   double& multiplier) const;

  bool chargeForPaxType(PricingTrx& trx,
                        const Indicator& chargeAppl,
                        const PaxTypeFare& fare,
                        const PaxTypeCode& paxTypeCode) const;

  void collectMaxTransfersAllow(PricingTrx& trx,
                                PricingUnit& pu,
                                const FareUsage& fu,
                                bool isCmdPricing) const;

  void collectMaxPassedTransfers(PricingTrx& trx,
                                 const CategoryRuleItemInfoSet& categoryRuleItemInfoS,
                                 int16_t& ruleSetTransferTotal) const;

  bool anyTablePasses(PricingTrx& trx, const uint32_t itemNo) const;

  void collectTransfers(const PricingUnit& pu) const;

  virtual const RuleItemInfo*
  getRuleItemInfo(PricingTrx& trx, const CategoryRuleItemInfo* cri) const;

  TransfersTravelSegWrapper& getOrCreateTsWrapper(TravelSeg* ts, FareUsage* fu) const;

  const TransfersInfo1* _trInfo = nullptr;
  const CategoryRuleInfo* _crInfo = nullptr;
  VCTR _vctr;

  mutable bool _isFareRule = true; // false means General Rule

  mutable TransfersTravelSegWrapperMap _transfersTravelSegWrappers;
  mutable bool _transfersRuleExistsInSet = false;
  mutable bool _ignoreNoMatch = false;
  mutable bool _needToProcessResults = false;

  mutable int16_t _charge1Count = 0;
  mutable int16_t _charge2Count = 0;
  mutable int16_t _charge1Max = 0;
  mutable int16_t _charge2Max = 0;
  mutable bool _charge1Unlimited = false;
  mutable bool _charge2Unlimited = false;

  mutable bool _applyLeastRestrictiveProvision = false;
  mutable int16_t _leastRestrictiveTransfersPermitted = 0;
  mutable bool _leastRestrictiveTransfersUnlimited = false;

  mutable SurchargeVector _surcharges;

  mutable std::set<const FareUsage*> _fareUsageWithPricingUnitScope;

  mutable bool _transferFCscope = false;
  mutable bool _transferFailsPU = false;
  mutable bool _recurringFCscope = false;
  mutable bool _transferFCscopeInitial = false;
  mutable bool _transferFailsPUInitial = false;

  bool _isRelationAndExists = false;
  bool _isRelationOrExists = false;

  uint16_t _noTransfersMax = RuleConst::MAX_NUMBER_XX;

  FareUsage* _fareUsage;

private:
  static Logger _logger;
};

} // tse
