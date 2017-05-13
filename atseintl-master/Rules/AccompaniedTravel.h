//-------------------------------------------------------------------
////
////  Copyright Sabre 2004
////
////          The copyright to the computer program(s) herein
////          is the property of Sabre.
////          The program(s) may be used and/or copied only with
////          the written permission of Sabre or in accordance
////          with the terms and conditions stipulated in the
////          agreement/contract under which the program(s)
////          have been supplied.
////
////-------------------------------------------------------------------

#pragma once

#include "DataModel/AccTvlFarePath.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/SimplePaxTypeFare.h"
#include "DBAccess/AccompaniedTravelInfo.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleConst.h"


namespace tse
{

class PaxTypeFare;
class FareMarket;
class PricingTrx;
class Itin;
class PaxType;
class AirSeg;
class DiscountInfo;
class DiagCollector;

typedef const char* ACCTVL_VALID_RESULT;
typedef std::map<FareMarket*, const FareUsage*, std::less<void*> > FUMap;
typedef std::map<FareMarket*, const FareUsage*, std::less<void*> >::const_iterator FUMapCI;

class AccompaniedTravel : public RuleApplicationBase
{
  friend class AccompaniedTravelTest;

public:
  AccompaniedTravel() {}

  virtual ~AccompaniedTravel() {}

  /**
   * Two virtual functions of RuleApplicationBase
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* ruleInfo,
                              const FareMarket& fareMarket) override;

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      const RuleItemInfo* rule,
                                      const FarePath& farePath,
                                      const PricingUnit& pricingUnit,
                                      const FareUsage& fareUsage) override;

  /**
   *   @method validate
   *
   *   Description: Performs accompanied travel rule validations for
   *                all passengers travel within same PricingTrx during
   *                combinability process.
   *                This can be for Cat13 rule or Cat19-22 rule.
   *
   *   @param PricingTrx                - Pricing transaction
   *   @param std::vector<FarePath*>&   - FarePath of all passengers
   *   @param std::vector<FareUsage*>&  - Fare Usages of all passengers
   *
   *   @return bool - possible values are:
   *           true   Validation succeeded
   *           false  Validatoin failed
   *
   */
  bool validate(PricingTrx& trx, std::vector<FareUsage*>& fareUsages);

  /**
   *   @method validate
   *
   *   Description: Performs rule validations on PaxType to be discounted
   *                and PaxType of all accompanying passengers from
   *                PrincingTrx for Cat19-22.
   *                Requirement about passengers type, age and number of
   *                accompanied passenger against accompanying passenger
   *                will be validated
   *
   *   @param PricingTrx&         - Pricing transaction
   *   @param PaxType&            - PaxType of passenger qualifying discount
   *   @param DiscountInfo&       - Discount rule data
   *   @param PaxTypeFare&        - Passed in for diagnostic filtering purpose
   *
   *   @return Record3ReturnTypes - possible values are:
   *              PASS       validation succeeded
   *              FAIL       validation failed
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              const PaxType& myPaxType,
                              const DiscountInfo& discountInfo,
                              PaxTypeFare& paxTypeFare);
  /*
   *   Method for Fare display used to validate cat19-22 created fares
   */
  Record3ReturnTypes validate(const DiscountInfo& discountInfo, PaxTypeFare& paxTypeFare);

  /**
   *   @method validateInfFares
   *
   *   Description: For infant travel with 0 amount discount, it must use
   *                fare basis of accompanying adult
   *
   *   @param PricingTrx                - Pricing transaction
   *   @param FarePath&                 - FarePath of infant passenger(s)
   *   @param std::vector<FarePath*>&   - FarePath of all passengers
   *
   *   @return bool - possible values are:
   *           true   Validation succeeded
   *           false  Validatoin failed
   *
   */
  bool validateInfFares(const PricingTrx& trx,
                        const FarePath& infFP,
                        const std::vector<FarePath*>& farePathGrp);

  bool validateInfFares(const PricingTrx& trx, const FarePath& infFP, const FarePath& accPaxFP);

  void getZeroAmountFareUsages(std::vector<FareUsage*>& fuVec, const FarePath& farePath);

  void getFUMap(FUMap& fuMap, const FarePath& farePath);

  static uint32_t
  minNoAccPsgReq(const uint32_t& numAccompaniedPsg, const AccompaniedTravelInfo& accTvlInfo);

  bool validateAccTvl(PricingTrx& trx, AltPricingTrx::AccompRestrictionVec& accTvlData);

  bool validateAccTvl(PricingTrx& trx,
                      const std::vector<const std::string*>& accTvlData,
                      std::vector<bool>& resultVec);

  static void
  collectTrailerMsg(std::ostringstream& outputStr, const AccompaniedTravelInfo& accTvlInfo);

  static void collectTrailerMsg(std::ostringstream& outputStr, const DiscountInfo& discInfo);

  // Following are used to understand AccompaniedTravelInfo,
  // DiscountInfo,and DiscountSegInfo
  static constexpr Indicator notApply = ' ';
  static constexpr Indicator applyMust = ' '; // For ACCOMPANYING PSG.APPL
  static constexpr Indicator fareClass = 'F'; // For F/B byte
  static constexpr Indicator bookingCode = 'B'; // For F/B byte

protected:
  /**
   *   @method validate
   *   Description: Performs full rule validations with Cat13 rule item info
   *
   */
  Record3ReturnTypes validateFareUsages(PricingTrx& trx,
                                        const RuleItemInfo* ruleInfo,
                                        const FareUsage& fareUsage,
                                        const std::vector<FareUsage*>& fareUsages);

  /**
   *   @method validate
   *
   *   Description: Performs Cat19-22 rule validations on discounted
   *                PaxTypeFare with PaxTypeFares of all passengers
   *                during combinability process
   *
   */
  Record3ReturnTypes validate(PricingTrx& trx,
                              const PaxTypeFare& myPaxTypeFare,
                              const std::vector<PaxTypeFare*>& paxTypeFares,
                              const DiscountInfo& discInfo);

  Record3ReturnTypes validateSameSegReq(const std::vector<AirSeg*>& myAirSeg,
                                        const std::vector<AirSeg*>& accAirSeg,
                                        const AccompaniedTravelInfo& accTvlInfo,
                                        ACCTVL_VALID_RESULT& accTvlResult) const;

  ACCTVL_VALID_RESULT
  qualifyAccPsgTypeFare(const PaxType& paxType,
                        const bool& chkFareClassBkCds,
                        const PaxTypeFare& paxTypeFare,
                        const AccompaniedTravelInfo accTvlInfo,
                        PricingTrx& trx,
                        DiagCollector* diag = nullptr) const;

  ACCTVL_VALID_RESULT
  useSameRule(const PaxTypeFare& myPaxTypeFare, const PaxTypeFare& accPaxTypeFare);

  bool checkFareClassBkgCode(const PaxTypeFare& paxTypeFare,
                             const AccompaniedTravelInfo& accTvlInfo,
                             PricingTrx& trx,
                             DiagCollector* diag = nullptr) const;

  bool isSameAirSeg(const AirSeg& airSeg1, const AirSeg& airSeg2) const;

  bool qualifyPsgTypeAge(const PaxType& paxType,
                         const int minAge,
                         const int maxAge,
                         const PaxTypeCode& requiredPaxType) const;

  bool needCombinabiltyCheck(const AccompaniedTravelInfo& accTvlInfo) const;
  bool needCombinabiltyCheck(const DiscountInfo& discountInfo) const;

  void displayAccTvlRuleToDiag(const AccompaniedTravelInfo& accTvlInfo, DiagCollector& diag);

  void displayDiscountRuleToDiag(const DiscountInfo& discountInfo, DiagCollector& diag);

  void getAccTvlPaxTypeFares(const SimplePaxTypeFare& currPaxTypeFare,
                             std::vector<const SimplePaxTypeFare*>& accPaxTypeFares,
                             const AccTvlFarePath* currentFarePath,
                             const std::vector<const AccTvlFarePath*>& accFarePathVec) const;

  bool validateAccTvl(PricingTrx& trx,
                      const std::vector<const AccTvlFarePath*>& accFarePathVec,
                      std::vector<bool>& resultVec) const;

  bool validateAccTvl(PricingTrx& trx,
                      const SimplePaxTypeFare& paxTypeFare,
                      const std::vector<const SimplePaxTypeFare*>& accPaxTypeFares) const;

  bool foundMatchedPsg(const SimpleAccTvlRule& accTvlRule,
                       const SimplePaxTypeFare& myPTFare,
                       const std::vector<const SimplePaxTypeFare*>& accPaxTypeFares) const;

  bool matchAccPsg(const SimpleAccTvlRule& accTvlRule,
                   const SimplePaxTypeFare& myPTFare,
                   const SimplePaxTypeFare& accPTFare) const;

  void diagnosis(const SimplePaxTypeFare& paxTypeFare, DiagCollector& diag) const;

  void diagnosis(const SimpleAccTvlRule& accTvlRule, DiagCollector& diag) const;

  void diagnosis(const SimplePaxTypeFare& paxTypeFare,
                 const std::vector<const SimplePaxTypeFare*>& accPaxTypeFares,
                 DiagCollector& diag) const;

  void diagFareUsage(DiagCollector& diag, const FareUsage& fareUsage) const;

  static const ACCTVL_VALID_RESULT accTvlPASS;
  static const ACCTVL_VALID_RESULT failReasonNoAccPsg;
  static const ACCTVL_VALID_RESULT failReasonPsgType;
  static const ACCTVL_VALID_RESULT failReasonAge;
  static const ACCTVL_VALID_RESULT failReasonFB;
  static const ACCTVL_VALID_RESULT failReasonSameSeg;
  static const ACCTVL_VALID_RESULT failReasonSameCpmt;
  static const ACCTVL_VALID_RESULT failReasonSameRuleNum;
  static const ACCTVL_VALID_RESULT failReasonSameRuleTariff;
  static const ACCTVL_VALID_RESULT failReasonSameRuleCarr;
  static const ACCTVL_VALID_RESULT failReasonEmptySeg;
  static const ACCTVL_VALID_RESULT failReasonEmptyAccSeg;
  static const ACCTVL_VALID_RESULT failReasonGeoVia1;
  static const ACCTVL_VALID_RESULT failReasonGeoVia2;
  static const ACCTVL_VALID_RESULT failReasonPaxTypeNoMatch;
  static const ACCTVL_VALID_RESULT failReasonMUSTNOT;
  static const std::string DIAG_FAIL_MSG;
  static const std::string LOG_FAIL_MSG;

private:
  static const int16_t minInfAccompFareAge;
};

} // namespace tse

