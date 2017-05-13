//-------------------------------------------------------------------
//
//  File:        StopoversInfoWrapper.h
//  Created:     August 2, 2004
//  Authors:     Andrew Ahmad
//
//  Description:
//
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

#include "Common/TseEnums.h"
#include "Common/VCTR.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/StopoversInfoSeg.h"
#include "Rules/StopoverMaxExceeed.h"
#include "Rules/StopoversTravelSegWrapper.h"


namespace tse
{
class ChargeSODirect;
class DiagCollector;
class FareMarket;
class FarePath;
class FareUsage;
class PaxType;
class PaxTypeFare;
class PricingTrx;
class PricingUnit;
class RuleSetPreprocessor;
class StopoversInfo;
class TravelSeg;

class StopoversInfoWrapper : public RuleItemInfo
{
public:
  StopoversInfoWrapper()
    : _soInfo(nullptr),
      _crInfo(nullptr),
      _vctr(),
      _isFareRule(true),
      _stopoversRuleExistsInSet(false),
      _ignoreNoMatch(false),
      _needToProcessResults(false),
      _charge1FirstAmtCount(0),
      _charge1AddAmtCount(0),
      _charge1FirstNo(0),
      _charge1AddNo(0),
      _charge1FirstUnlimited(false),
      _charge1AddUnlimited(false),
      _charge2FirstAmtCount(0),
      _charge2AddAmtCount(0),
      _charge2FirstNo(0),
      _charge2AddNo(0),
      _charge2FirstUnlimited(false),
      _charge2AddUnlimited(false),
      _maxStopoversPermitted(0),
      _maxStopoversPermittedUnlimited(false),
      _applyLeastRestrictiveProvision(false),
      _leastRestrictiveStopoversPermitted(0),
      _leastRestrictiveStopoversUnlimited(false),
      _fareUsage(nullptr),
      _stopoverMaxExceeed()
  {
  }

  StopoversInfoWrapper(FareUsage* fu)
    : _soInfo(nullptr),
      _crInfo(nullptr),
      _vctr(),
      _isFareRule(true),
      _stopoversRuleExistsInSet(false),
      _ignoreNoMatch(false),
      _needToProcessResults(false),
      _charge1FirstAmtCount(0),
      _charge1AddAmtCount(0),
      _charge1FirstNo(0),
      _charge1AddNo(0),
      _charge1FirstUnlimited(false),
      _charge1AddUnlimited(false),
      _charge2FirstAmtCount(0),
      _charge2AddAmtCount(0),
      _charge2FirstNo(0),
      _charge2AddNo(0),
      _charge2FirstUnlimited(false),
      _charge2AddUnlimited(false),
      _maxStopoversPermitted(0),
      _maxStopoversPermittedUnlimited(false),
      _applyLeastRestrictiveProvision(false),
      _leastRestrictiveStopoversPermitted(0),
      _leastRestrictiveStopoversUnlimited(false),
      _fareUsage(fu),
      _stopoverMaxExceeed()
  {
  }

  virtual ~StopoversInfoWrapper() {}

  const static int16_t TOTAL_MAX_EXECEED;
  const static int16_t OUT_MAX_EXECEED;
  const static int16_t IN_MAX_EXECEED;
  const static int16_t TO_MANY_SO;

  void setRtw(bool rtw) { _stopoverMaxExceeed.setRtw(rtw); }

  const StopoversInfo* soInfo() const { return _soInfo; }

  // This method needs to do more than just set the member.
  // That is why the signature deviates from the standard convention.
  void soInfo(const StopoversInfo* soInfo);

  // const StopoversInfo*& soInfo() { return _soInfo; }

  const CategoryRuleInfo* crInfo() const { return _crInfo; }
  void crInfo(const CategoryRuleInfo* crInfo);

  bool& isFareRule() const { return _isFareRule; }
  bool& isFareRule() { return _isFareRule; }

  StopoversTravelSegWrapperMap& stopoversTravelSegWrappers() const
  {
    return _stopoversTravelSegWrappers;
  }
  StopoversTravelSegWrapperMap& stopoversTravelSegWrappers() { return _stopoversTravelSegWrappers; }

  bool& stopoversRuleExistsInSet() const { return _stopoversRuleExistsInSet; }
  bool& stopoversRuleExistsInSet() { return _stopoversRuleExistsInSet; }

  bool& ignoreNoMatch() const { return _ignoreNoMatch; }
  bool& ignoreNoMatch() { return _ignoreNoMatch; }

  bool& needToProcessResults() const { return _needToProcessResults; }
  bool& needToProcessResults() { return _needToProcessResults; }

  const int16_t charge2FirstAmtCount() const { return _charge2FirstAmtCount; }

  const int16_t charge2AddAmtCount() const { return _charge2AddAmtCount; }

  bool& applyLeastRestrictiveProvision() const { return _applyLeastRestrictiveProvision; }
  bool& applyLeastRestrictiveProvision() { return _applyLeastRestrictiveProvision; }

  int16_t& leastRestrictiveStopoversPermitted() const
  {
    return _leastRestrictiveStopoversPermitted;
  }
  int16_t& leastRestrictiveStopoversPermitted() { return _leastRestrictiveStopoversPermitted; }

  bool& leastRestrictiveStopoversUnlimited() const { return _leastRestrictiveStopoversUnlimited; }
  bool& leastRestrictiveStopoversUnlimited() { return _leastRestrictiveStopoversUnlimited; }

  FareUsage* fareUsage() const { return _fareUsage; }
  FareUsage*& fareUsage() { return _fareUsage; }

  void setIsStopover(TravelSeg* ts, FareUsage* fu) const;

  bool checkIsStopover(TravelSeg* ts) const;

  void setPassedValidation(TravelSeg* ts,
                           FareUsage* fu,
                           bool passedByLeastRestrictive,
                           TravelSeg* arnk_ts = nullptr) const;

  bool checkPassedValidation(TravelSeg* ts) const;

  void setRuleItemMatch(TravelSeg* ts,
                        FareUsage* fu,
                        const uint32_t itemNo,
                        const bool isTentativeMatch,
                        bool surfaceNegativeMatch = false) const;

  bool checkMatched(TravelSeg* ts) const;

  void setMatched(TravelSeg* ts, FareUsage* fu, StopoversTravelSegWrapper::MatchType val) const;

  void setMaxExceeded(TravelSeg* ts, FareUsage* fu, const int16_t&) const;

  bool& maxStopoversPermittedUnlimited() const { return _maxStopoversPermittedUnlimited; }
  bool& maxStopoversPermittedUnlimited() { return _maxStopoversPermittedUnlimited; }

  int16_t& maxStopoversPermitted() const { return _maxStopoversPermitted; }
  int16_t& maxStopoversPermitted() { return _maxStopoversPermitted; }

  uint32_t getRuleItemMatch(const TravelSeg* ts) const;

  uint32_t getRuleItemMatch(const TravelSeg* ts, bool& isTentativeMatch) const;

  void
  doRuleSetPreprocessing(PricingTrx& trx, const RuleSetPreprocessor& rsp, const PricingUnit& pu);

  bool addSurcharge(PricingTrx& trx,
                    TravelSeg* ts,
                    FareUsage* fu,
                    const FarePath& fp,
                    const StopoversInfoSeg* soInfoSeg,
                    const bool isSegmentSpecific,
                    const bool forceAddAmt,
                    const bool chargeFromFirstInbound,
                    const bool forceFirstAmt,
                    const bool doAdd) const;

  void setMaxStopoversPermittedUnlimited() const { _maxStopoversPermittedUnlimited = true; }

  Record3ReturnTypes processResults(PricingTrx& trx, PaxTypeFare& ptf);

  Record3ReturnTypes processResults(PricingTrx& trx,
                                    FarePath& fp,
                                    const PricingUnit& pu,
                                    const FareUsage& fu,
                                    const bool processCharges = true);

  void processSurcharges(PricingTrx& trx,
                         FarePath& fp,
                         const FareUsage& fareUsage,
                         DiagCollector* diagPtr,
                         bool isCmdPricing = false);

  bool checkAllPassed(); // true if all travel segs have PASSed
  void clearResults(bool skipInd = false);

  const VCTR& getVCTR() const;

  bool chargeForPaxType(PricingTrx& trx,
                        const Indicator& chargeAppl,
                        const PaxTypeFare& fare,
                        const PaxType& paxType,
                        const bool step = false) const;

  void setMostRestrictiveMaxStop(PricingUnit* pu, const StopoversInfo* soInfo);
  bool checkArunkForcedPass(const TravelSeg* ts) const;
  void sumNumStopoversMax(const uint32_t relationalInd, const StopoversInfo* soInfo);

  typedef std::map<const StopoversInfoSeg*, ChargeSODirect*> SoMapSegmentDirectionalityCharge;
  typedef SoMapSegmentDirectionalityCharge::const_iterator SoMapSegmentDirectionalityChargeIC;
  typedef SoMapSegmentDirectionalityCharge::iterator SoMapSegmentDirectionalityChargeI;

  mutable SoMapSegmentDirectionalityCharge _infoSegsDirectionalityCharge;

protected:
  void copyFrom(const RuleItemInfo& ruleItemInfo);

  bool selectApplCharge(PricingTrx& trx,
                        int16_t& chargeNum,
                        MoneyAmount& firstAmt,
                        CurrencyCode& firstCur,
                        int16_t& firstNoDec,
                        MoneyAmount& addAmt,
                        CurrencyCode& addCur,
                        int16_t& addNoDec,
                        const PaxTypeFare& fare,
                        const PaxType& paxType) const;

  bool applyPaxTypeDiscount(PricingTrx& trx,
                            MoneyAmount& firstAmt,
                            CurrencyCode& firstCur,
                            int16_t& firstNoDec,
                            MoneyAmount& addAmt,
                            CurrencyCode& addCur,
                            int16_t& addNoDec,
                            const FareUsage& fareUsage,
                            const PaxType& paxType,
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

  virtual bool numberOfChargesOneOrTwoApplied(const int16_t chargeNum) const;

  virtual bool checkIfChargeIsApplicable(const StopoversInfoSeg* soInfoSeg,
                                         const int16_t chargeNum,
                                         const FMDirection dir,
                                         const bool forceAddAmt = false) const;

  virtual void updateDirectionalityChargeIfApplicable(PricingTrx& trx,
                                                      const StopoversInfoSeg* soInfoSeg,
                                                      const int16_t chargeNum,
                                                      const FMDirection dir,
                                                      const bool forceAddAmt = false) const;

  void accumulateDirCharge(ChargeSODirect* chDirSo,
                           const int16_t chargeNum,
                           const bool forceAddAmt,
                           const FMDirection dir) const;

  const StopoversInfo* _soInfo;
  const CategoryRuleInfo* _crInfo;
  VCTR _vctr;

  mutable bool _isFareRule; // false means General Rule

  mutable StopoversTravelSegWrapperMap _stopoversTravelSegWrappers;
  mutable bool _stopoversRuleExistsInSet;
  mutable bool _ignoreNoMatch;
  mutable bool _needToProcessResults;

  mutable int16_t _charge1FirstAmtCount;
  mutable int16_t _charge1AddAmtCount;
  mutable int16_t _charge1FirstNo;
  mutable int16_t _charge1AddNo;
  mutable bool _charge1FirstUnlimited;
  mutable bool _charge1AddUnlimited;

  mutable int16_t _charge2FirstAmtCount;
  mutable int16_t _charge2AddAmtCount;
  mutable int16_t _charge2FirstNo;
  mutable int16_t _charge2AddNo;
  mutable bool _charge2FirstUnlimited;
  mutable bool _charge2AddUnlimited;

  mutable int16_t _maxStopoversPermitted;
  mutable bool _maxStopoversPermittedUnlimited;

  mutable bool _applyLeastRestrictiveProvision;
  mutable int16_t _leastRestrictiveStopoversPermitted;
  mutable bool _leastRestrictiveStopoversUnlimited;
  mutable FareUsage* _fareUsage;
  StopoverMaxExceeed _stopoverMaxExceeed;

  bool checkArnkMatch(const StopoversTravelSegWrapper& stsw) const;

};

} // namespace tse

