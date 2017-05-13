//----------------------------------------------------------------
//
//  File: SalesRestrictionRule.h
//  Authors:
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
//-----------------------------------------------------------------
#pragma once

#include "Common/CarrierUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingOptions.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag315Collector.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "Rules/IObserver.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleControllerDataAccess.h"
#include "Rules/RuleItem.h"
#include "Rules/SubjectObserved.h"

namespace boost
{
namespace logic
{
class tribool;
}
}

namespace tse
{
class PricingTrx;
class Itin;
class RuleItemInfo;
class DCFactory;
class Nation;
class Loc;
class TravelSeg;
class LocKey;
class Locale;
class PosPaxType;

class SalesRestrictionRule : public RuleApplicationBase,
                             public SubjectObserved<IObserver<NotificationType, const DateTime&>>
{
public:
  friend class SalesRestrictionTest;

  // Cat15 Fail Reason Codes
  enum Cat15FailReasons
  { SALES_RESTR_NO_ERROR = 0,
    SALES_RESTR_COUNTRY,
    SALES_RESTR_SALES_DATE,
    SALES_RESTR_FORM_OF_PAYMENT,
    SALES_RESTR_CURRENCY,
    SALES_RESTR_SOLD_TICKET,
    SALES_RESTR_CARRIER,
    SALES_RESTR_LOCALE,
    SALES_RESTR_SECURITY,
    SALES_RESTR_CRS_OTHER_CARRIER,
    SALES_RESTR_DATA_UNAVAILABLE,
    SALES_RESTR_TICKET,
    SALES_RESTR_PRIVATE_SECURITY,
    SALES_RESTR_TEXT_ONLY,
    SALES_RESTR_SKIP,
    SALES_RESTR_DATE_OVERRIDE,
    REFUND_SECURITY_PART_ONLY };

  static const PseudoCityCode OPT_IN_AGENCY;

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override
  {
    return PASS;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override
  {
    return PASS;
  }

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      FareUsage* fU,
                                      PaxTypeFare& paxTypeFare,
                                      const CategoryRuleInfo& cri,
                                      const CategoryRuleItemInfo* rule,
                                      const SalesRestriction* salesRestriction,
                                      bool isQualifiedCategory,
                                      bool& isCat15Security,
                                      bool skipCat15Security);

  // A version to handle CombinabilityRuleInfo and CombinabilityRuleItemInfo
  // in the same manner as above
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      FareUsage* fU,
                                      PaxTypeFare& paxTypeFare,
                                      const CombinabilityRuleInfo& cri,
                                      const CombinabilityRuleItemInfo* rule,
                                      const SalesRestriction* salesRestriction,
                                      bool isQualifiedCategory,
                                      bool& isCat15Security,
                                      bool skipCat15Security);

  bool validateFareGroup(PricingTrx& trx,
                         PaxTypeFare& paxTypeFare,
                         const SalesRestriction* salesRestriction,
                         std::vector<uint16_t>& actualPaxTypeItem) const;
  bool isWebFare(const PricingTrx& trx, const SalesRestriction* salesRestriction) const;

  bool checkSaleAndTktLocs(PricingTrx& trx) const;

protected:
  bool refundQualifyingSecurity(const PricingTrx& trx, bool isQualifiedCategory) const;

  virtual bool checkCountryRestPU(const PricingTrx& trx,
                                  const FareUsage* fareUsage,
                                  const SalesRestriction* salesRestriction) const;

  virtual bool checkCurrencyRestPU(const PricingTrx& trx,
                                   const FareUsage* fareUsage,
                                   const Indicator& currCntryInd,
                                   const CurrencyCode& paymentCurr) const;

public:
  virtual bool
  checkCountryRestriction(PricingTrx&, const FareUsage*, PaxTypeFare&, const SalesRestriction*);

  virtual bool checkSalesDate(PricingTrx& trx,
                              PaxTypeFare& paxTypeFare,
                              const SalesRestriction* salesRestriction) const;

  virtual bool checkFormOfPayment(const PricingTrx&,
                                  PaxTypeFare& paxTypeFare,
                                  const SalesRestriction*,
                                  FareUsage*);

  virtual bool
  checkCurrencyRestriction(PricingTrx&, const FareUsage*, PaxTypeFare&, const SalesRestriction*);

  virtual bool checkTicketElectronic(bool reqEtkt, Indicator canEtkt) const;

  virtual bool checkSelectSecurity(const PaxTypeFare&, const SalesRestriction*) const;

  virtual bool
  checkSoldTktRestriction(PricingTrx&, Itin&, PaxTypeFare&, const SalesRestriction*) const;

  bool checkCarrierRestriction(PricingTrx& trx,
                               Itin& itin,
                               PaxTypeFare& paxTypeFare,
                               const SalesRestriction* salesRestriction,
                               const FareUsage* fU);

protected:
  bool isOptIn(const LocCode& loc1, const LocCode& loc2) const;

  bool isAgentOptIn(const Agent&) const;

  virtual bool hasTvlyLocation(const PricingTrx&) const;

  bool determinePricingCurrency(const Nation& nation, CurrencyCode& pricingCurrency) const;

  bool validateCarrier(const PricingTrx& trx,
                       PaxTypeFare& paxTypeFare,
                       const SalesRestriction* salesRestriction);

  virtual bool validateCarrier(const PricingTrx& trx,
                               const CarrierCode& vcr,
                               const PaxTypeFare& paxTypeFare,
                               const SalesRestriction* salesRestriction,
                               const bool isWQTrx) const;

  bool validateCarriers(const PricingTrx& trx,
                        const Itin& itin,
                        const PaxTypeFare& paxTypeFare,
                        const SalesRestriction* salesRestriction,
                        const FareUsage* fu,
                        const bool isWQTrx);

  bool validateAllCarriersIS(PricingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             Itin& itin,
                             const SalesRestriction* salesRestriction) const;

  bool validateCarrierRestrictionSegment(const PricingTrx& trx,
                                         const Itin& itin,
                                         const PaxTypeFare& paxTypeFare,
                                         const SalesRestriction* salesRestriction,
                                         const bool isWQTrx) const;

  bool validateCarrierRestrictionFare(const PricingTrx& trx,
                                      const PaxTypeFare& paxTypeFare,
                                      const SalesRestriction* salesRestriction,
                                      const bool isWQTrx) const;

  virtual const Loc* determineCountryLoc(const PricingTrx& trx) const;

public:
  void getTrailerMsg(const SalesRestriction* salesRestriction);

  void setTrailerMsg(PaxTypeFare& paxTypeFare, FareUsage* fareUsage) const;

protected:
  bool checkLocations(const LocKey& locKey1,
                      const LocKey& locKey2,
                      const Loc* locPO,
                      const PricingTrx& trx,
                      const PaxTypeFare& paxTypeFare) const;

  bool checkCarrier(const PricingTrx& trx,
                    const Itin& itin,
                    const PaxTypeFare& paxTypeFare,
                    const SalesRestriction* salesRestriction) const;

  bool checkCarrierMatchForJointCarrier(const PricingTrx& trx,
                                        const Itin& itin,
                                        const PaxTypeFare& paxTypeFare,
                                        const SalesRestriction* salesRestriction) const;

  virtual bool checkRussianGroup(const Loc& loc1, const Loc& loc2) const;

  virtual bool checkUsTerritoryRule(const Loc& loc1, const Loc& loc2) const;

  virtual bool checkScandinavia(const Loc& loc1, const Loc& loc2) const;

  virtual const Loc* getLoc(const LocCode& loc, const PricingTrx& trx) const;

  virtual const Nation*
  getNation(const NationCode& nation, const DateTime& dt, const PricingTrx& trx) const;

public:
  const Agent* getAgent(const PricingTrx& trx, bool isQualifiedCategory) const;

protected:
  bool validateCarrierRestriction(const PricingTrx& trx,
                                  const std::vector<TravelSeg*>& travelSegments,
                                  const CarrierCode& carrier,
                                  const CarrierCode& otherCarrier) const;

  bool findCorpIdMatch(const PseudoCityCode& pcc,
                       const std::vector<Locale*>& locales,
                       const PosPaxType* posPaxType,
                       bool matchedCorpId) const;

  bool matchDeptCodeOAC(const PricingTrx& trx,
                        const Agent& agent,
                        const LocKey& locKey1,
                        const LocKey& locKey2) const;

  const bool checkPreConditions(Cat15FailReasons& failReason,
                                const SalesRestriction* salesRestriction,
                                const bool skipCat15SecurityCheck,
                                const PricingTrx& trx) const;

  Record3ReturnTypes determineNotValidReturnCode(const Cat15FailReasons& failReason,
                                                 const PaxTypeFare& paxTypeFare) const;

public:
  virtual bool isPropperGDS(const SalesRestriction* sr) const;

  virtual bool matchGDS(const SalesRestriction* sr, const Agent* agent) const;

  bool isFBRPrivateTariff(const PricingTrx& trx, const FareUsage* fu, const PaxTypeFare& ptf) const;

  bool getResultForDFF(const PricingTrx& trx, const FareUsage* fu, const PaxTypeFare& ptf) const;

  template <class T>
  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              FareUsage* fU,
                              PaxTypeFare& paxTypeFare,
                              const T& cri,
                              const typename T::item_info_type* rule,
                              const SalesRestriction* salesRestriction,
                              bool isQualifiedCategory,
                              bool& isCat15Security,
                              bool skipCat15Security,
                              bool& localeFailed)
  {
    // The security portion of category 15 will not be validated when both Category 35 and Category
    // 15
    //(Fare Rule, General Rule, and/or Footnote) are present as a Main categories.
    // Base fare of Cat 25 fare with DISC 'L/C/T' will have skipCat15Security indicator on.
    if (UNLIKELY(!salesRestriction))
      return SKIP;

    if (UNLIKELY(paxTypeFare.selectedFareNetRemit() && !isQualifiedCategory))
    {
      // do cat15 Footnote validation for the selected ticketing fare (cat35 NetRemit)
      return (validateSelectedNetRemitFare(trx, cri, rule, paxTypeFare, salesRestriction));
    }

    bool skipCat15SecurityCheck =
        (paxTypeFare.isNegotiated() && !isQualifiedCategory) || // Cat 35 fare with Main 15  OR
        (skipCat15Security && !isQualifiedCategory); // Base fare of Cat 25/Cat 35 fare with Main 15

    Cat15FailReasons failReason = SALES_RESTR_NO_ERROR;
    bool failedSabre = false;
    Record3ReturnTypes retCode = SKIP;

    bool result =
        checkPreConditions(failReason, salesRestriction, skipCat15SecurityCheck, trx) &&
        checkSecurityDataInMainCat15(salesRestriction, isQualifiedCategory, paxTypeFare, cri) &&
        checkPrivateTariffCarrierRestrictions(failReason,
                                              salesRestriction,
                                              skipCat15SecurityCheck,
                                              isQualifiedCategory,
                                              paxTypeFare,
                                              isCat15Security,
                                              cri,
                                              rule) &&
        checkSecurity(*this,
                      trx,
                      fU,
                      rule,
                      failReason,
                      isQualifiedCategory,
                      salesRestriction,
                      paxTypeFare,
                      cri,
                      skipCat15SecurityCheck,
                      localeFailed,
                      failedSabre);

    if (result && !refundQualifyingSecurity(trx, isQualifiedCategory))
      result = checkOtherRestrictions(
          *this, trx, failReason, itin, fU, paxTypeFare, salesRestriction, cri);

    if (result)
      retCode = _softPass ? SOFTPASS : PASS;
    else
      retCode = determineNotValidReturnCode(failReason, paxTypeFare);

    if (!isQualifiedCategory) // Only set when validating main category
      updatePaxTypeFareWhenMainCategory(
          trx, salesRestriction, failReason, localeFailed, paxTypeFare, cri, retCode, fU);
    
    updatePaxTypeFareWhenETktEnchancement(
        trx, salesRestriction, retCode, localeFailed, paxTypeFare, cri, isQualifiedCategory, fU);
        
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic315))
      runDiag315(*this,
                 trx,
                 paxTypeFare,
                 cri,
                 rule,
                 salesRestriction,
                 retCode,
                 failReason,
                 skipCat15SecurityCheck);

    return retCode;
  }

protected:
  bool checkCarrierMatch(const CarrierCode& cxr,
                         const CarrierCode& restriction,
                         const PricingTrx& trx) const;

  bool validateCarrierExcludePublishing(const PricingTrx& trx,
                                        const CarrierCode& vcr,
                                        const PaxTypeFare& paxTypeFare,
                                        const SalesRestriction* salesRestriction) const;

  bool isRtw(const PricingTrx& trx) const { return trx.getOptions() && trx.getOptions()->isRtw(); }

  template <class T>
  void setFrInCat15(const LocKey& locKey1,
                    const LocKey& locKey2,
                    const bool matchSaleRestriction,
                    const bool matchTktRestriction,
                    const T& cri,
                    const Agent& agent,
                    PaxTypeFare& paxTypeFare) const
  {
    // The Cat15 "N FR" restriction will be checked only:
    //    - in standalone cat15 (reuse processing);
    //    - in qualified cat15 to cat25.
    if (UNLIKELY(agent.cwtUser() && paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF &&
                  (matchSaleRestriction || matchTktRestriction) &&
                  ((locKey1.locType() == LOCTYPE_NATION && locKey1.loc() == NATION_FRANCE) ||
                   (locKey2.locType() == LOCTYPE_NATION && locKey2.loc() == NATION_FRANCE)) &&
                  (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE ||
                   cri.categoryNumber() == RuleConst::FARE_BY_RULE)))
    {
      paxTypeFare.fare()->setNationFRInCat15();

      if (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE)
      {
        // save NationFRInCat15 for 'REUSE of cat15 Footnote/fareRule/genRule'
        if (typeid(cri) == typeid(FootNoteCtrlInfo))
          paxTypeFare.fare()->setNationFRInCat15Fn();
        else if (paxTypeFare.fare()->isCat15GeneralRuleProcess())
          paxTypeFare.fare()->setNationFRInCat15Gr();
        else
          paxTypeFare.fare()->setNationFRInCat15Fr();
      }
    }
  }

  template <class T>
  bool checkSecurityDataInMainCat15(const SalesRestriction* salesRestriction,
                                    const bool isQualifiedCategory,
                                    PaxTypeFare& paxTypeFare,
                                    const T& cri) const
  {
    // Check for security data in Main Cat 15
    if (!isQualifiedCategory && (salesRestriction->carrierCrsInd() != RuleConst::BLANK ||
                                 salesRestriction->tvlAgentSaleInd() != RuleConst::BLANK ||
                                 salesRestriction->locales().size() > 0))
    {
      paxTypeFare.fare()->setCat15HasSecurity(true);
      if (ruleDataAccess())
        ruleDataAccess()->setTmpResultCat15HasSecurity(true);

      // save SecurityInCat15 for 'REUSE of cat15 Footnote/fareRule/genRule'
      if (UNLIKELY(typeid(cri) == typeid(FootNoteCtrlInfo)))
        paxTypeFare.fare()->setSecurityInCat15Fn();
      else if (paxTypeFare.fare()->isCat15GeneralRuleProcess())
        paxTypeFare.fare()->setSecurityInCat15Gr();
      else
        paxTypeFare.fare()->setSecurityInCat15Fr();
    }

    return true;
  }

  //----------------------------------------------------------------------------
  // validateSelectedNetRemitFare()
  //----------------------------------------------------------------------------
  template <class T>
  Record3ReturnTypes validateSelectedNetRemitFare(PricingTrx& trx,
                                                  const T& cri,
                                                  const typename T::item_info_type* rule,
                                                  PaxTypeFare& paxTypeFare,
                                                  const SalesRestriction* salesRestriction) const
  {
    Record3ReturnTypes retCode = SKIP;
    Cat15FailReasons failReason = SALES_RESTR_NO_ERROR;

    if (salesRestriction->unavailTag() == RuleConst::DATA_UNAVAILABLE)
    {
      failReason = SALES_RESTR_DATA_UNAVAILABLE; // fail this cat15
    }

    else if (salesRestriction->unavailTag() == RuleConst::TEXT_ONLY)
    {
      failReason = SALES_RESTR_TEXT_ONLY; // skip this cat15
    }

    if (failReason == SALES_RESTR_NO_ERROR && !checkSalesDate(trx, paxTypeFare, salesRestriction))
    {
      failReason = SALES_RESTR_SALES_DATE;
    }

    if (failReason == SALES_RESTR_NO_ERROR)
    {
      retCode = PASS;
    }
    else if (failReason == SALES_RESTR_TEXT_ONLY || failReason == SALES_RESTR_DATE_OVERRIDE)
    {
      retCode = SKIP;
    }
    else
    {
      retCode = FAIL;
    }

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic315))
      runDiag315(*this, trx, paxTypeFare, cri, rule, salesRestriction, retCode, failReason, false);

    return retCode;
  }

public:
  template <class T>
  bool setFlags(const PricingTrx& trx,
                PaxTypeFare& paxTypeFare,
                const T& cri,
                const SalesRestriction* salesRestriction,
                bool& mustBeSold,
                bool& mustBeTicket,
                bool& matchSaleRestriction,
                bool& matchTktRestriction,
                bool& negativeMatchOptIn,
                bool& negativeMatchPCC,
                bool isQualifiedCategory) const
  {
    const Agent* agent = getAgent(trx, isQualifiedCategory);
    const bool agentOptIn = (agent != nullptr && isAgentOptIn(*agent));
    const bool setWarningForWq =
        trx.noPNRPricing() && cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE;
    bool match = false;
    const bool vendorSITA = salesRestriction->vendor() == SITA_VENDOR_CODE;

    //------------------------------------------------------------------------
    // Loop through all Locale items to validate them
    //------------------------------------------------------------------------
    std::vector<Locale*>::const_iterator iterB = salesRestriction->locales().begin();
    std::vector<Locale*>::const_iterator iterE = salesRestriction->locales().end();
    for (; iterB != iterE; ++iterB)
    {
      const LocKey& locKey1 = (*iterB)->loc1();
      const LocKey& locKey2 = (*iterB)->loc2();
      const Indicator locAppl = (*iterB)->locAppl();

      bool matchOptIn = false;

      if (UNLIKELY((matchSaleRestriction && (locAppl == RuleConst::TKTS_MAY_ONLY_BE_SOLD ||
                                              locAppl == RuleConst::TKTS_MAY_NOT_BE_SOLD)) ||
                    (matchTktRestriction && (locAppl == RuleConst::TKTS_MAY_ONLY_BE_ISSUED ||
                                             locAppl == RuleConst::TKTS_MAY_NOT_BE_ISSUED))))

      {
        if (setWarningForWq)
          paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);
        continue;
      }

      if (locAppl == RuleConst::TKTS_MAY_ONLY_BE_SOLD)
      {
        if (UNLIKELY(setWarningForWq))
          paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

        mustBeSold = true;
      }
      else if (UNLIKELY(locAppl == RuleConst::TKTS_MAY_ONLY_BE_ISSUED))
      {
        if (setWarningForWq)
          paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);

        mustBeTicket = true;
      }

      if (locKey1.locType() == RuleConst::TRAVEL_AGENCY ||
          locKey1.locType() == RuleConst::HOME_TRAVEL_AGENCY ||
          locKey1.locType() == RuleConst::IATA_TVL_AGENCY_NO ||
          locKey1.locType() == RuleConst::HOME_IATA_TVL_AGENCY_NO)
      {
        //----------------------
        // Travel Agency request
        //----------------------
        if (!agent->tvlAgencyPCC().empty())
        {
          if (locKey1.locType() == RuleConst::TRAVEL_AGENCY)
          {
            if (locKey1.loc() == agent->tvlAgencyPCC() || locKey2.loc() == agent->tvlAgencyPCC() ||
                (matchOptIn = (agentOptIn && isOptIn(locKey1.loc(), locKey2.loc()))) == true)
            {
              match = true;
            }
          }
          else if (locKey1.locType() == RuleConst::HOME_TRAVEL_AGENCY)
          {
            if ((!agent->mainTvlAgencyPCC().empty() &&
                 (locKey1.loc() == agent->mainTvlAgencyPCC() ||
                  locKey2.loc() == agent->mainTvlAgencyPCC())) ||
                locKey1.loc() == agent->tvlAgencyPCC() || locKey2.loc() == agent->tvlAgencyPCC() ||
                (matchOptIn = (agentOptIn && isOptIn(locKey1.loc(), locKey2.loc()))) == true)
            // Some carrier filed incorrect cat15 data with "U" (assumed "T").
            {
              match = true;
            }
          }
          else if (locKey1.locType() == RuleConst::IATA_TVL_AGENCY_NO) // tvlAgencyIATA()
          {
            // DB is fixed for vendor='ATP' (remove 1st '0')

            if ((locKey1.loc() == agent->tvlAgencyIATA()) ||
                (locKey2.loc() == agent->tvlAgencyIATA()))
            {
              match = true;
            }
          }
          else //  homeAgencyIATA()
          {
            if ((locKey1.loc() == agent->homeAgencyIATA()) ||
                (locKey2.loc() == agent->homeAgencyIATA()))
            {
              match = true;
            }
          }
        }
      }
      else if (locKey1.locType() == RuleConst::DUTY_CODE)
      {
        if (UNLIKELY((locKey1.loc()[7] == agent->agentDuty()[0]) ||
                      (locKey2.loc()[7] == agent->agentDuty()[0])))
        {
          match = true;
        }
      }
      else if (locKey1.locType() == RuleConst::FUNCTIONAL_CODE)
      {
        if (locKey1.loc().size() == 1)
        {
          if ((locKey1.loc() == agent->agentFunctions()) ||
              (locKey2.loc() == agent->agentFunctions()))
          {
            match = true;
          }
        }
        // The type code V is supposed to refer to three alpha codes, like HDQ, XTM, ATX etc.
        else
        {
          match = matchDeptCodeOAC(trx, *agent, locKey1, locKey2);
        }
      }
      else // Types: A, C, N, P, S and Z
      {
        const Loc* locPO = nullptr;

        if (UNLIKELY((locAppl == RuleConst::TKTS_MAY_ONLY_BE_ISSUED) ||
                      (locAppl == RuleConst::TKTS_MAY_NOT_BE_ISSUED)))
        {
          // check ticketing point
          if (!trx.getRequest()->ticketPointOverride().empty()) // Point of Ticketing overriden
          {
            locPO = getLoc(trx.getRequest()->ticketPointOverride(), trx);
          }
          else
          {
            locPO = agent->agentLocation();
          }
        }
        else // otherwise, check point of sale
        {
          if (!trx.getRequest()->salePointOverride().empty()) // Point of Sale overriden
          {
            locPO = getLoc(trx.getRequest()->salePointOverride(), trx);
          }
          else
          {
            locPO = agent->agentLocation();
          }
        }

        if (locPO != nullptr && checkLocations(locKey1, locKey2, locPO, trx, paxTypeFare))
        {
          match = true;
        }
      }

      if (match)
      {
        if (locAppl == RuleConst::TKTS_MAY_NOT_BE_SOLD || // Value 'N'
            locAppl == RuleConst::TKTS_MAY_NOT_BE_ISSUED) // Value 'X'
        {
          if (matchOptIn) // Even match negatively by OptInAgency, there may be positive filing for
          // PCC.
          {
            negativeMatchOptIn = true;
          }
          else // Even match negatively by PCC, there may be positive filing for OptInAgency.
          {
            if (UNLIKELY(!agentOptIn)) // Agent is not OptIn, no need to continue
              return false;

            negativeMatchPCC = true;
          }

          if (UNLIKELY(negativeMatchPCC && negativeMatchOptIn))
            return false;

          match = false;
          continue;
        }
        else if (LIKELY(locAppl == RuleConst::TKTS_MAY_ONLY_BE_SOLD)) // Value 'Y'
        {
          matchSaleRestriction = true;

          if (matchOptIn) // Match positively by OptInAgency, turn off previous negative match PCC
          {
            negativeMatchPCC = false;
          }
          else // Match positively by PCC, turn off any previous negative match OptIn
          {
            negativeMatchOptIn = false;
          }
        }
        else if (locAppl == RuleConst::TKTS_MAY_ONLY_BE_ISSUED) // Value 'T'
        {
          matchTktRestriction = true;

          if (matchOptIn) // Match positively by OptInAgency, turn off previous negative match PCC
          {
            negativeMatchPCC = false;
          }
          else // Match positively by PCC, turn off any previous negative match OptIn
          {
            negativeMatchOptIn = false;
          }
        }

        if (!vendorSITA || (matchSaleRestriction && matchTktRestriction))
        {
          setFrInCat15(locKey1,
                       locKey2,
                       matchSaleRestriction,
                       matchTktRestriction,
                       cri,
                       *agent,
                       paxTypeFare);
          break;
        }

        match = false;
      }
    }
    return true;
  }

protected:
  template <class T>
  bool checkPrivateTariffCarrierRestrictions(Cat15FailReasons& failReason,
                                             const SalesRestriction* salesRestriction,
                                             const bool skipCat15SecurityCheck,
                                             const bool isQualifiedCategory,
                                             const PaxTypeFare& paxTypeFare,
                                             bool& isCat15Security,
                                             const T& cri,
                                             const typename T::item_info_type* rule) const
  {
    // check carrier restrictions for the private tariff
    // skip this check for Cat 35 fare when these fields are in main Cat 15.
    if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && // Private fare
        !skipCat15SecurityCheck && // Not skip Cat 15 security portion
        rule->relationalInd() != CategoryRuleItemInfo::AND) // Should be THEN/OR string
    {
      // Check for security data
      if (salesRestriction->carrierCrsInd() == RuleConst::BLANK &&
          salesRestriction->tvlAgentSaleInd() == RuleConst::BLANK &&
          salesRestriction->locales().size() == 0)
      {
        if (typeid(cri) != typeid(FootNoteCtrlInfo) && !isQualifiedCategory &&
            (!paxTypeFare.fare()->isCat15GeneralRuleProcess() ||
             !paxTypeFare.fare()->cat15HasSecurity()))
        {
          failReason = SALES_RESTR_PRIVATE_SECURITY;
          return false;
        }
      }

      else if (!isQualifiedCategory)
        isCat15Security = true; // set up this indicator only for the main cat15 category
    }

    return true;
  }

  template <class T>
  void updatePaxTypeFareWhenMainCategory(PricingTrx& trx,
                                         const SalesRestriction* salesRestriction,
                                         const Cat15FailReasons& failReason,
                                         const bool localeFailed,
                                         PaxTypeFare& paxTypeFare,
                                         const T& cri,
                                         const Record3ReturnTypes& retCode,
                                         const FareUsage* fU) const
  {
    bool isPrivateTariff = false;
    if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF ||
        isFBRPrivateTariff(trx, fU, paxTypeFare))
      isPrivateTariff = true;
    if ((retCode == FAIL) && // Failed for reasons other than the below HARD FAIL reasons
        !((failReason == SALES_RESTR_DATA_UNAVAILABLE) || // UnAvailableTag
          (failReason == SALES_RESTR_CRS_OTHER_CARRIER) || // Carrier Restriction Byte 26,27-29
          (failReason ==
           SALES_RESTR_SALES_DATE) || // Earliest/Latest Ticketing Dates Byte 12-15/20-23
          (failReason ==
           SALES_RESTR_SECURITY) || // Sales Restriction including Travel Agency Restriction
          (failReason ==
           SALES_RESTR_PRIVATE_SECURITY) || // Missing Security in THEN for the private tariff
          (localeFailed && isPrivateTariff))) // Private Fare fails Locale Byte 73-84
    {
      paxTypeFare.setCat15SoftPass(true);
    }

    if (!paxTypeFare.cat25Fare())
      paxTypeFare.setCat15SecurityFail(localeFailed || (failReason == SALES_RESTR_SECURITY));
      
    // save the latestTktDate, need to be 23:59 to ensure correct time stamp
    if (retCode == PASS && salesRestriction->latestTktDate().isValid())
    {
      DateTime ltd = DateTime(salesRestriction->latestTktDate().date(), 23, 59, 0);

      paxTypeFare.fare()->latestTktDT().setWithEarlier(ltd);

      // save LatestTktDT for 'REUSE of cat15 Footnote/fareRule/genRule'
      if (typeid(cri) == typeid(FootNoteCtrlInfo))
        paxTypeFare.fare()->latestTktDTFootN().setWithEarlier(ltd);

      else if (UNLIKELY(paxTypeFare.fare()->isCat15GeneralRuleProcess()))
        paxTypeFare.fare()->latestTktDTGenR().setWithEarlier(ltd);

      else
        paxTypeFare.fare()->latestTktDTFareR().setWithEarlier(ltd);
    }
  }

  template <class T>
  void updatePaxTypeFareWhenETktEnchancement(PricingTrx& trx,
                                             const SalesRestriction* salesRestriction,
                                             const Record3ReturnTypes& retCode,
                                             const bool localeFailed,
                                             PaxTypeFare& paxTypeFare,
                                             const T& cri,
                                             const bool isQualifiedCategory,
                                             const FareUsage* fU) const
  {
    // E_ticketing enhancement. Only set Warning Etkt MSG when:
    //     -when validating main category or qualified for C25;
    //     - pricing, shopping entries (WP, WPNC, WPA, WPNC,JR. WPNI, JA.) for all users.

    if (retCode == PASS &&
        (!isQualifiedCategory || cri.categoryNumber() == RuleConst::FARE_BY_RULE) &&
        !(trx.getRequest()->isTicketEntry() || // W'
          trx.noPNRPricing())) // WQ
    {
      if (UNLIKELY(salesRestriction->tktIssElectronic() == RuleConst::REQUIRED &&
                    trx.getRequest()->isETktOffAndNoOverride()))
      {
        paxTypeFare.fare()->setWarningEtktInCat15();
        if (!isQualifiedCategory && fU == nullptr)
        {
          // save indicators for 'REUSE' of cat15 Footnote/fareRule/genRule
          // 'reuse' is active only for Prevalidation/Normal validation phases
          if (typeid(cri) == typeid(FootNoteCtrlInfo))
            paxTypeFare.fare()->setWarningEtktInCat15Fn();
          else if (paxTypeFare.fare()->isCat15GeneralRuleProcess())
            paxTypeFare.fare()->setWarningEtktInCat15Gr();
          else
            paxTypeFare.fare()->setWarningEtktInCat15Fr();
        }
      }
    }
  }

public:
  bool isSoftPass() const { return _softPass; }

private:
  bool _softPass = false;
  Fare::FopStatus _forbiddenFop;

public:
  class Diag355
  {
  public:
    Diag355(PricingTrx& trx,
            const SalesRestriction* salesRestriction,
            const CategoryRuleItemInfo* rule,
            const PaxTypeFare& paxTypeFare);
    ~Diag355();
    void printFailMsgAgencyInfo();
    void printFailMsgRelationalAndCarriers();
    void printFailMsgCarriers();
    void printFailMsgTvlAgentSaleInd();
    void printFailMsgPartitionId();
    void printPassMsg();

  protected:
    bool _diagEnabled;
    DCFactory* _factory;
    DiagCollector* _diagPtr;
    PricingTrx* _trx;
    const SalesRestriction* _salesRestriction;
    const CategoryRuleItemInfo* _rule;
    const PaxTypeFare* _paxTypeFare;

  private:
    Diag355();
    friend class SalesRestrictionTest;
  }; // Diag355

  boost::logic::tribool privateTariffCheck(const PricingTrx& trx,
                                           const PaxTypeFare& paxTypeFare,
                                           const SalesRestriction* salesRestriction,
                                           Diag355& diag355) const;

  boost::logic::tribool publicTariffCheck(const PricingTrx& trx,
                                          const PaxTypeFare& paxTypeFare,
                                          const SalesRestriction* salesRestriction,
                                          Diag355& diag355) const;

}; // SalesRestrictionRule

//----------------------------------------------------------------------------
// checkSaleSecurity()
//----------------------------------------------------------------------------
template <class SalesRestrictionRuleT, class InfoT>
bool
checkSaleSecurity(const SalesRestrictionRuleT& salesRestrictionRule,
                  PricingTrx& trx,
                  const PaxTypeFare& paxTypeFare,
                  const InfoT& cri,
                  const typename InfoT::item_info_type* rule,
                  const SalesRestriction* salesRestriction,
                  bool& failedSabre,
                  bool isFareDisplay,
                  bool isQualifiedCategory)
{
  using boost::logic::tribool;
  typename SalesRestrictionRuleT::Diag355 diag355(
      trx, salesRestriction, &rule->categoryRuleItemInfo(), paxTypeFare);

  // Check Travel Agency  Restrictions - Sale
  const Agent* agent = salesRestrictionRule.getAgent(trx, isQualifiedCategory);

  if (!salesRestrictionRule.matchGDS(salesRestriction, agent))
    return false;

  if (!agent->tvlAgencyPCC().empty() || !agent->tvlAgencyIATA().empty())
  {
    if ((salesRestriction->tvlAgentSaleInd() == RuleConst::MAY_NOT_BE_SOLD_BY_TA))
    {
      diag355.printFailMsgAgencyInfo();
      return false;
    }
    if (salesRestriction->carrierCrsInd() == RuleConst::MUST_BE_SOLD_VIA_CARRIER)
    {
      if (LIKELY(!isFareDisplay || paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF))
      {
        diag355.printFailMsgAgencyInfo();
        return false;
      }
    }
    paxTypeFare.track("C15AGENT");
  }

  // Check  Carrier Restrictions - Sale
  else
  {
    // The following logic regarding "AND" condition was adopted from ATSE 1.0

    // Perform validation of carrier restriction if not 'AND' condition
    //  neither footnote

    // NOTE: When two or more Cat 15's are concatenated by "AND",
    //       the second and subsequent Cat 15's are inheriting the
    //       carrierCrsInd indicator from the first Cat 15, even
    //       if that indicator is coded in the second cat 15.

    if (rule->relationalInd() != CategoryRuleItemInfo::AND &&
        (typeid(cri) != typeid(FootNoteCtrlInfo)))
    {
      // check carrier restrictions for the private tariff or public tariff
      tribool ret =
          (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
              ? salesRestrictionRule.privateTariffCheck(trx, paxTypeFare, salesRestriction, diag355)
              : salesRestrictionRule.publicTariffCheck(trx, paxTypeFare, salesRestriction, diag355);

      if (!indeterminate(ret))
        return ret;
    }
    paxTypeFare.track("C15CARRIER");
  }

  if (UNLIKELY(!salesRestrictionRule.isPropperGDS(salesRestriction)))
  {
    diag355.printFailMsgCarriers();
    failedSabre = true;
    return false;
  }

  diag355.printPassMsg();
  return true;
}

//----------------------------------------------------------------------------
// checkTicketRestriction()
//----------------------------------------------------------------------------
template <class SalesRestrictionRuleT, class InfoT>
bool
checkTicketRestriction(const SalesRestrictionRuleT& salesRestrictionRule,
                       PricingTrx& trx,
                       FareUsage* fU,
                       PaxTypeFare& paxTypeFare,
                       const SalesRestriction* salesRestriction,
                       const InfoT& cri)
{
  bool isQualifiedCat12 = (cri.categoryNumber() == RuleConst::SURCHARGE_RULE);
  bool isQualifiedCat25 = (cri.categoryNumber() == RuleConst::FARE_BY_RULE);
  bool isCat15Main = (cri.categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE);

  if (UNLIKELY(trx.noPNRPricing()))
  {
    bool isRestriction = (salesRestriction->tktIssPta() != RuleConst::BLANK ||
                          salesRestriction->tktIssMail() != RuleConst::BLANK ||
                          salesRestriction->tktIssElectronic() != RuleConst::BLANK);

    if (isRestriction && isCat15Main)
    {
      paxTypeFare.warningMap().set(WarningMap::cat15_warning_2, true);
    }
  }

  // set flag for paper tkt surcharge
  if (isQualifiedCat12)
  {
    // TODO why not for all RuleConst?
    if (salesRestriction->tktIssElectronic() == RuleConst::NOT_ALLOWED ||
        salesRestriction->tktIssElectronic() == RuleConst::REQUIRED)
    {
      if (!trx.getRequest()->isElectronicTicket())
      {
        if (fU != nullptr)
          fU->setPaperTktSurchargeMayApply(true);
        else
          paxTypeFare.setPaperTktSurchargeMayApply(true);
      }
    }
  }

  paxTypeFare.setElectronicTktable(salesRestriction->tktIssElectronic() == RuleConst::NOT_ALLOWED);

  if (!salesRestrictionRule.checkTicketElectronic(trx.getRequest()->isElectronicTicket(),
                                                  salesRestriction->tktIssElectronic()))
  {
    // E_ticketing enhancement. We may pass (sending a Warning MSG) it when:
    //     -pricing, shopping entry,
    //     -main Cat15 or qualified to C25
    //     -ETKT is required in Rule and isETktOffAndNoOverride()==true

    if (!(isCat15Main || isQualifiedCat25) ||
        (salesRestriction->tktIssElectronic() != RuleConst::REQUIRED) ||
        !trx.getRequest()->isETktOffAndNoOverride() || trx.getRequest()->isTicketEntry()) // W'
      return false;
  }

  // may still have a segment w/ paper surcharge
  if (isQualifiedCat12 && salesRestriction->tktIssElectronic() == RuleConst::REQUIRED)
  {
    // looping through each segment for the current Fare Component scope to check E_tkt

    std::vector<TravelSeg*>::const_iterator itB = paxTypeFare.fareMarket()->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator itE = paxTypeFare.fareMarket()->travelSeg().end();

    for (; itB != itE; ++itB)
    {
      if (UNLIKELY(!(*itB)->isAir()))
        continue;

      if (UNLIKELY(!trx.getRequest()->isElectronicTicket())) // Temp fix
      {
        if (fU != nullptr)
          fU->setPaperTktSurchargeMayApply(true);
        else
          paxTypeFare.setPaperTktSurchargeMayApply(true);
        return false;
      }
    } // endfor -each seg
  } // endif - check all segs

  if (UNLIKELY((salesRestriction->tktIssMail() == RuleConst::NOT_ALLOWED &&
                 trx.getRequest()->isTicketByMail()) ||
                (salesRestriction->tktIssMail() == RuleConst::REQUIRED &&
                 !trx.getRequest()->isTicketByMail())))
  {
    return false;
  }

  if (salesRestriction->tktIssPta() == RuleConst::NOT_ALLOWED) //  "N"
  {
    // THE POINT OF SALE AND POINT OF TKT SHOULD BE THE SAME (the same city).
    // THEY ARE ALWAYS THE SAME WITHOUT OVERRIDING.
    return salesRestrictionRule.checkSaleAndTktLocs(trx);
  }

  return true;
}

//----------------------------------------------------------------------------
// Check Locales
//----------------------------------------------------------------------------
template <class SalesRestrictionRuleT, class InfoT>
bool
checkLocaleItems(const SalesRestrictionRuleT& salesRestrictionRule,
                 const PricingTrx& trx,
                 FareUsage* fU,
                 PaxTypeFare& paxTypeFare,
                 const InfoT& cri,
                 const SalesRestriction* salesRestriction,
                 bool isQualifiedCategory)
{
  if (salesRestriction->locales().size() == 0) // Any Locale item?
    return true;

  bool mustBeSold = false;
  bool mustBeTicket = false;
  bool matchSaleRestriction = false;
  bool matchTktRestriction = false;
  bool negativeMatchOptIn = false;
  bool negativeMatchPCC = false;

  if (UNLIKELY(!salesRestrictionRule.setFlags(trx,
                                               paxTypeFare,
                                               cri,
                                               salesRestriction,
                                               mustBeSold,
                                               mustBeTicket,
                                               matchSaleRestriction,
                                               matchTktRestriction,
                                               negativeMatchOptIn,
                                               negativeMatchPCC,
                                               isQualifiedCategory)))
    return false;

  // At this point, there was either a match found or neither of the locale
  // items matched the Agent's GeoData
  //------------------------------------------------------------------------
  if (negativeMatchPCC || negativeMatchOptIn)
  {
    return false;
  }

  if (matchSaleRestriction || matchTktRestriction)
  {
    if (UNLIKELY((mustBeSold && !matchSaleRestriction) || (mustBeTicket && !matchTktRestriction)))
    {
      return false;
    }
  }
  else // No locale matched
  {
    bool isPrivateTariff = (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF);
    if (salesRestrictionRule.isFBRPrivateTariff(trx, fU, paxTypeFare))
      isPrivateTariff = true;

    if (isPrivateTariff)
    {
      if (isQualifiedCategory)
        return false; // Must match at least one 'must be sold' item
      return (salesRestrictionRule.getResultForDFF(trx, fU, paxTypeFare)); // Must match at least
                                                                           // one 'must be sold'
      // item if not DFF fare
    }
    else // Public tariff
    {
      if (mustBeSold || mustBeTicket)
      {
        return false; // Fail if there was a 'must be sold' or a 'must be ticket' item
      }
    }
  }
  static const std::string C15LOCALE("C15LOCALE");
  paxTypeFare.track(C15LOCALE);
  return true;
}

//----------------------------------------------------------------------------
// checkSecurityRestriction()
//----------------------------------------------------------------------------
template <class SalesRestrictionRuleT, class InfoT>
bool
checkSecurityRestriction(const SalesRestrictionRuleT& salesRestrictionRule,
                         PricingTrx& trx,
                         FareUsage* fU,
                         PaxTypeFare& paxTypeFare,
                         const InfoT& cri,
                         const typename InfoT::item_info_type* rule,
                         const SalesRestriction* salesRestriction,
                         bool& failedSabre,
                         bool isFareDisplay,
                         bool isQualifiedCategory,
                         bool checkLocale = true)
{
  //--------------------------------------------------------------------------
  // Validate Travel Agency and Carrier Restrictions - Sale
  //--------------------------------------------------------------------------
  if (!checkSaleSecurity(salesRestrictionRule,
                         trx,
                         paxTypeFare,
                         cri,
                         rule,
                         salesRestriction,
                         failedSabre,
                         isFareDisplay,
                         isQualifiedCategory))
  {
    return false;
  }
  //--------------------------------------------------------------------------
  // Validate Travel Agency Restrictions - Select
  //--------------------------------------------------------------------------
  if (UNLIKELY(!salesRestrictionRule.checkSelectSecurity(paxTypeFare, salesRestriction)))
  {
    return false;
  }

  //--------------------------------------------------------------------------
  // Validate the Locale items
  //--------------------------------------------------------------------------
  if (checkLocale)
  {
    if (!checkLocaleItems(
            salesRestrictionRule, trx, fU, paxTypeFare, cri, salesRestriction, isQualifiedCategory))
    {
      return false;
    }
  }
  return true;
}

template <class SalesRestrictionRuleT, class InfoT>
bool
checkSecurity(const SalesRestrictionRuleT& salesRestrictionRule,
              PricingTrx& trx,
              FareUsage* fU,
              const typename InfoT::item_info_type* rule,
              typename SalesRestrictionRuleT::Cat15FailReasons& failReason,
              const bool isQualifiedCategory,
              const SalesRestriction* salesRestriction,
              PaxTypeFare& paxTypeFare,
              const InfoT& cri,
              const bool skipCat15SecurityCheck,
              bool& localeFailed,
              bool& failedSabre)
{
  // Check securtiy first
  // Skip Cat 15 Security portion check in main Cat 15 for Cat 35 fare
  if (!skipCat15SecurityCheck)
  {
    // Check Locale part first
    if (!checkLocaleItems(
            salesRestrictionRule, trx, fU, paxTypeFare, cri, salesRestriction, isQualifiedCategory))
    {
      localeFailed = true;
    }

    // Check other security portion
    if (!checkSecurityRestriction(salesRestrictionRule,
                                  trx,
                                  fU,
                                  paxTypeFare,
                                  cri,
                                  rule,
                                  salesRestriction,
                                  failedSabre,
                                  false,
                                  isQualifiedCategory,
                                  false))
    {
      failReason = SalesRestrictionRuleT::SALES_RESTR_SECURITY; // fail it
      return false;
    }

    if (localeFailed)
    {
      failReason = SalesRestrictionRuleT::SALES_RESTR_LOCALE;
      return false;
    }
  }

  return true;
}

template <class SalesRestrictionRuleT, class InfoT>
bool
checkOtherRestrictions(SalesRestrictionRuleT& salesRestrictionRule,
                       PricingTrx& trx,
                       typename SalesRestrictionRuleT::Cat15FailReasons& failReason,
                       Itin& itin,
                       FareUsage* fU,
                       PaxTypeFare& paxTypeFare,
                       const SalesRestriction* salesRestriction,
                       const InfoT& cri)
{
  if (!salesRestrictionRule.checkCountryRestriction(trx, fU, paxTypeFare, salesRestriction))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_COUNTRY;
    return false;
  }
  if (!salesRestrictionRule.checkSalesDate(trx, paxTypeFare, salesRestriction))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_SALES_DATE;
    return false;
  }
  if (UNLIKELY(!salesRestrictionRule.checkFormOfPayment(trx, paxTypeFare, salesRestriction, fU)))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_FORM_OF_PAYMENT;
    return false;
  }
  if (UNLIKELY(
          !salesRestrictionRule.checkCurrencyRestriction(trx, fU, paxTypeFare, salesRestriction)))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_CURRENCY;
    return false;
  }
  if (!checkTicketRestriction(salesRestrictionRule, trx, fU, paxTypeFare, salesRestriction, cri))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_TICKET;
    return false;
  }
  if (!salesRestrictionRule.checkSoldTktRestriction(trx, itin, paxTypeFare, salesRestriction))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_SOLD_TICKET;
    return false;
  }
  if (!salesRestrictionRule.checkCarrierRestriction(trx, itin, paxTypeFare, salesRestriction, fU))
  {
    failReason = SalesRestrictionRuleT::SALES_RESTR_CARRIER;
    return false;
  }

  if (!trx.noPNRPricing() && !salesRestrictionRule.isSoftPass())
    salesRestrictionRule.setTrailerMsg(paxTypeFare, fU);

  return true;
}
class SalesRestrictionRuleWrapper
{
public:
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      FareUsage* fU,
                                      PaxTypeFare& paxTypeFare,
                                      const CategoryRuleInfo& cri,
                                      const CategoryRuleItemInfo* rule,
                                      const SalesRestriction* salesRestriction,
                                      bool isQualifiedCategory,
                                      bool& isCat15Security,
                                      bool skipCat15Security);

  // A version to handle CombinabilityRuleInfo and CombinabilityRuleItemInfo
  // in the same manner as above
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      FareUsage* fU,
                                      PaxTypeFare& paxTypeFare,
                                      const CombinabilityRuleInfo& cri,
                                      const CombinabilityRuleItemInfo* rule,
                                      const SalesRestriction* salesRestriction,
                                      bool isQualifiedCategory,
                                      bool& isCat15Security,
                                      bool skipCat15Security);

  void setRuleDataAccess(RuleControllerDataAccess* ruleDataAccess);

  SalesRestrictionRule& getSalesRestrictionRule() { return _rule; }

private:
  SalesRestrictionRule _rule;
};
} // namespace tse

