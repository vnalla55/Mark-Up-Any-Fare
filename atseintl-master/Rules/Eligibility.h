//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/FlexFares/Types.h"
#include "Rules/RuleApplicationBase.h"

namespace tse
{

class PaxTypeFare;
class FareMarket;
class PricingTrx;
class EligibilityInfo;
class DCFactory;
class Itin;
class CorpId;

class Eligibility : public RuleApplicationBase
{
  friend class EligibilityTest;

public:
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override
  {
    return PASS;
  }

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      PaxTypeFare& fare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket,
                                      const bool& isQualifyingCat,
                                      const bool& isCat15Qualifying);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override
  {
    return PASS;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage,
                              const bool& isQualifyingCat);

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage)
  {
    return PASS;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage,
                              const bool& isQualifyingCat,
                              const bool isQualiyfingCat27 = false);

  virtual Record3ReturnTypes checkAccountCode(const EligibilityInfo* eligibilityInfo,
                                              PaxTypeFare& paxTypeFare,
                                              PricingTrx& trx,
                                              DCFactory* factory,
                                              const bool& isCat15Qualifying,
                                              DiagCollector* diagPtr,
                                              bool diagEnabled) const;

  Record3ReturnTypes checkMultiAccCodeCorpId(const EligibilityInfo* eligibilityInfo,
                                             const PaxTypeFare& paxTypeFare,
                                             PricingTrx& trx,
                                             DiagCollector* diagPtr,
                                             bool diagEnabled) const;

  Record3ReturnTypes checkFlexFareAcctCodeCorpId(const EligibilityInfo* eligibilityInfo,
                                                 PaxTypeFare& paxTypeFare,
                                                 PricingTrx& trx,
                                                 DiagCollector* diagPtr,
                                                 bool diagEnabled) const;

  bool areFlexFaresAccCodesCorpIdsMatched(const EligibilityInfo* eligibilityInfo,
                                          PaxTypeFare& ptf,
                                          PricingTrx& trx,
                                          flexFares::Attribute attrName,
                                          std::vector<std::string>& diagStrVec) const;

  bool isSingleFlexFaresAccCodeCorpIdMatched(const EligibilityInfo* eligibilityInfo,
                                             PaxTypeFare& ptf,
                                             PricingTrx& trx,
                                             flexFares::Attribute attrName,
                                             const std::string& corpIdAccCodeStr) const;

  bool isFlexFareGroupRequireAccCodeOrCorpId(PricingTrx& trx) const;

  Record3ReturnTypes validateFromFCO(PricingTrx& trx,
                                     const RuleItemInfo* rule,
                                     Itin& itin,
                                     PaxTypeFare& ptFare,
                                     const CategoryRuleInfo* ruleInfo,
                                     const FareMarket& fareMarket);

protected:
  Record3ReturnTypes checkUnavailableAndText(const EligibilityInfo* eligibilityInfo) const;

  Record3ReturnTypes checkPTC(const EligibilityInfo* eligibilityInfo,
                              PaxTypeFare& paxTypeFare,
                              PricingTrx& trx,
                              DCFactory* factory,
                              DiagCollector* diagPtr,
                              bool diagEnabled,
                              bool isQualifyingCat) const;

  Record3ReturnTypes checkAgeRestrictions(const EligibilityInfo* eligibilityInfo,
                                          PaxTypeFare& paxTypeFare,
                                          DCFactory* factory,
                                          DiagCollector* diagPtr,
                                          bool isQualifyingCat,
                                          bool diagEnabled) const;

  virtual bool checkCorpIdMatrix(const EligibilityInfo* eligibilityInfo,
                                 const std::string& corporateID,
                                 const PaxTypeFare& paxTypeFare,
                                 PricingTrx& trx) const;

  bool checkCorpIdMatrix(const EligibilityInfo* eligibilityInfo,
                         const std::string& corporateID,
                         const PaxTypeFare& paxTypeFare,
                         PricingTrx& trx,
                         const DateTime& travelDate) const;

  bool matchCorpIdMatrix(const EligibilityInfo* eligibilityInfo,
                         const PaxTypeFare& paxTypeFare,
                         const std::vector<tse::CorpId*>& corpIds) const;

  Record3ReturnTypes checkPassengerStatus(const EligibilityInfo* eligibilityInfo,
                                          PaxTypeFare& paxTypeFare,
                                          PricingTrx& trx,
                                          DCFactory* factory,
                                          DiagCollector* diagPtr,
                                          bool diagEnabled) const;

  bool matchPassengerStatus(const EligibilityInfo& eligibilityInfo,
                            const PricingTrx& trx,
                            const PaxTypeFare& fare) const;

  bool isRexNewItinNeedKeepFare(const PricingTrx& trx) const;
  const std::string& getCorporateID(PricingTrx& trx) const;
  const std::string& getAccountCode(PricingTrx& trx) const;
  bool diagFCFilter(PricingTrx& trx, PaxTypeFare& ptFare) const;
  bool checkPTCType(PricingTrx& trx, PaxTypeFare& ptFare) const;
  bool
  matchPTCType(const EligibilityInfo* eligibilityInfo, PricingTrx& trx, PaxTypeFare& ptFare) const;

  const EligibilityInfo* _itemInfo = nullptr;
  Itin* _itin = nullptr;

};
} // namespace tse
