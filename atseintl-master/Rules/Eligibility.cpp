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

#include "Rules/Eligibility.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/EligibilityInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleValidationChancelor.h"
#include "Util/BranchPrediction.h"

namespace tse
{

static Logger
logger("atseintl.Rules.Eligibility");

Record3ReturnTypes
Eligibility::validateFromFCO(PricingTrx& trx,
                             const RuleItemInfo* rule,
                             Itin& itin,
                             PaxTypeFare& ptFare,
                             const CategoryRuleInfo* ruleInfo,
                             const FareMarket& fareMarket)
{

  LOG4CXX_INFO(logger, " Entered Eligibility::createDiag301Header()");

  const EligibilityInfo* eligibilityInfo = dynamic_cast<const EligibilityInfo*>(rule);
  if (UNLIKELY(!eligibilityInfo))
    return FAIL;

  Record3ReturnTypes result = NOTPROCESSED;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic301 && diagFCFilter(trx, ptFare)))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic301);
    diagEnabled = true;

    (*diagPtr) << "CATEGORY 01 - ELIGIBILITY APPLICATION DIAGNOSTICS" << std::endl;
    (*diagPtr) << "QUALIFIED TO CAT " << ruleInfo->categoryNumber() << std::endl;

    (*diagPtr) << "PHASE: FARE COLLECTOR    R3 ITEM NUMBER: " << rule->itemNo() << std::endl;

    (*diagPtr) << ptFare.fareMarket()->origin()->loc() << " "
               << ptFare.fareMarket()->destination()->loc();
    if (!ptFare.fareClass().empty())
    {
      (*diagPtr) << " FC: " << ptFare.fareClass();
      std::string fareBasis = ptFare.createFareBasis(trx, false);
      (*diagPtr) << " FB: " << fareBasis;
    }
    else
      (*diagPtr) << " FC: TEMPCAT25";

    (*diagPtr) << std::endl;

    (*diagPtr) << "R2:FARERULE   :   " << ruleInfo->vendorCode() << " " << ruleInfo->tariffNumber()
               << " " << ruleInfo->carrierCode() << " " << ruleInfo->ruleNumber() << std::endl;

    diagPtr->flushMsg();
  }

  result = validate(trx, itin, ptFare, eligibilityInfo, fareMarket, true, false);
  if (UNLIKELY(diagEnabled))
  {
    diagPtr->activate();
    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return result;
}

bool
Eligibility::diagFCFilter(PricingTrx& trx, PaxTypeFare& ptFare) const
{

  const std::string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
  if (diagFareClass.empty() ||
      RuleUtil::matchFareClass(diagFareClass.c_str(), ptFare.fareClass().c_str()) ||
      (diagFareClass == "TEMPCAT25" && ptFare.fareClass().empty()))
    return true;

  return false;
}

Record3ReturnTypes
Eligibility::validate(PricingTrx& trx,
                      Itin& itin,
                      PaxTypeFare& paxTypeFare,
                      const RuleItemInfo* rule,
                      const FareMarket& fareMarket,
                      const bool& isQualifyingCat,
                      const bool& isCat15Qualifying)

{
  LOG4CXX_INFO(logger, " Entered Eligibility::validate()");

  const EligibilityInfo* eligibilityInfo = dynamic_cast<const EligibilityInfo*>(rule);

  if (UNLIKELY(!eligibilityInfo))
    return FAIL;

  _itin = &itin;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic301 &&
               diagFCFilter(trx, paxTypeFare)))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic301);
    diagEnabled = true;

    (*diagPtr) << "FARE PAX TYPE : " << paxTypeFare.fcasPaxType()
               << " MIN AGE : " << paxTypeFare.fcasMinAge()
               << " MAX AGE : " << paxTypeFare.fcasMaxAge() << std::endl
               << "R3 PAX TYPE   : " << eligibilityInfo->psgType()
               << " MIN AGE : " << eligibilityInfo->minAge()
               << " MAX AGE : " << eligibilityInfo->maxAge() << std::endl
               << "R3 ACCOUNT CODE/CORP ID : " << eligibilityInfo->acctCode() << std::endl;

    if (!paxTypeFare.isFareByRule() &&
        paxTypeFare.cat25Fare())
    {
      PaxTypeFare* correctPtf = paxTypeFare.cat25Fare();
      (*diagPtr) << "CAT25 FARE PAX TYPE : " << correctPtf->fcasPaxType() << std::endl;
    }
  }

  Record3ReturnTypes result = checkUnavailableAndText(eligibilityInfo);
  if (result == FAIL)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED "
                 << "- CHECK UNAVAILABLE" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }
  else if (result == SKIP)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - SKIP");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: SKIP "
                 << "- TEXT ONLY" << std::endl;
      diagPtr->flushMsg();
    }
    return SKIP;
  }

  result =
      checkPTC(eligibilityInfo, paxTypeFare, trx, factory, diagPtr, diagEnabled, isQualifyingCat);
  if (result == FAIL)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED "
                 << "- CHECK PTC" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }
  else if (result == SKIP)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - SKIP");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: SKIP "
                 << "- NO MATCH ON PSGR TYPE" << std::endl;
      diagPtr->flushMsg();
    }
    return SKIP;
  }

  result = checkAgeRestrictions(
      eligibilityInfo, paxTypeFare, factory, diagPtr, isQualifyingCat, diagEnabled);
  if (UNLIKELY(result == FAIL))
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED "
                 << "- AGE RESTRICTIONS" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }
  else if (UNLIKELY(result == SKIP))
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - SKIP");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: SKIP "
                 << "- NO MATCH ON PSGR TYPE" << std::endl;
      diagPtr->flushMsg();
    }
    return SKIP;
  }
  paxTypeFare.track("C01AGE");

  if (checkAccountCode(
          eligibilityInfo, paxTypeFare, trx, factory, isCat15Qualifying, diagPtr, diagEnabled) ==
      FAIL)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED "
                 << "- ACCOUNT CODE" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  if (checkPassengerStatus(eligibilityInfo, paxTypeFare, trx, factory, diagPtr, diagEnabled) ==
      FAIL)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED "
                 << "- PSGR STATUS" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - PASS");
  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "PASS" << std::endl;
    diagPtr->flushMsg();
  }

  paxTypeFare.track("C01VAL");
  return PASS;
}

//------------------------
// Check Unavailable Text
//------------------------
Record3ReturnTypes
Eligibility::checkUnavailableAndText(const EligibilityInfo* eligibilityInfo) const
{
  //----------------------------------------------------------------------
  // Check the Unavailable Data Tag
  //----------------------------------------------------------------------
  /// @todo Move some levels up?

  // Incomplete data?
  if (eligibilityInfo->unavailTag() == dataUnavailable)
  {
    return FAIL; // Yes, fail this fare
  }

  if (eligibilityInfo->unavailTag() == textOnly)
  // Text data only?

  {
    return SKIP; // Yes, skip this category
  }

  return PASS;
}

bool
Eligibility::checkPTCType(PricingTrx& trx, PaxTypeFare& ptFare) const
{
  bool isCat25Fare = false;
  PaxTypeFare* correctPtf = nullptr;

  if (!ptFare.isFareByRule() && ptFare.cat25Fare())
    isCat25Fare = true;

  if (!isCat25Fare)
  {
    if (ptFare.fcasPaxType().empty() || ptFare.fcasPaxType() == ADULT)
    {
      return true;
    }
  }
  else
  {
    if (LIKELY(isCat25Fare))
    {
      correctPtf = ptFare.cat25Fare();
      if (correctPtf->fcasPaxType().empty() || correctPtf->fcasPaxType() == ADULT)
      {
        return true;
      }
    }
  }
  return false;
}

bool
Eligibility::matchPTCType(const EligibilityInfo* eligibilityInfo,
                          PricingTrx& trx,
                          PaxTypeFare& ptFare) const
{
  bool isCat25Fare = false;
  PaxTypeFare* correctPtf = nullptr;

  if (!ptFare.isFareByRule() && ptFare.cat25Fare())
    isCat25Fare = true;

  if (!isCat25Fare)
  {
    if (ptFare.fcasPaxType() == eligibilityInfo->psgType())
    {
      return true;
    }
  }
  else
  {
    if (isCat25Fare)
    {
      correctPtf = ptFare.cat25Fare();
      if (correctPtf->fcasPaxType() == eligibilityInfo->psgType())
      {
        return true;
      }
    }
  }
  return false;
}

//------------------------------
//  check passenger Type Code
//------------------------------
Record3ReturnTypes
Eligibility::checkPTC(const EligibilityInfo* eligibilityInfo,
                      PaxTypeFare& paxTypeFare,
                      PricingTrx& trx,
                      DCFactory* factory,
                      DiagCollector* diagPtr,
                      bool diagEnabled,
                      bool isQualifyingCat) const
{
  if (UNLIKELY(eligibilityInfo == nullptr))
  {
    return SKIP;
  }

  //-------------------------------------------------------------------------
  // Validate the Passenger Type Code (PTC)
  // If webFare, skip validation of cat 1 PTC
  // If fare is opaque, it may be filed as ADT and NEG
  //-------------------------------------------------------------------------
  if (!eligibilityInfo->psgType().empty())
  {
    if ((eligibilityInfo->minAge() != 0) || (eligibilityInfo->maxAge() != 0))
      paxTypeFare.setFoundCat1R3ChkAge();

    if (checkPTCType(trx, paxTypeFare))
    {
      if (eligibilityInfo->psgType() != ADULT)
      {
        if (UNLIKELY(paxTypeFare.needChkCat1R3Psg()))
        {
          const PaxTypeCode& r3PaxType = eligibilityInfo->psgType();
          const CarrierCode& ptCarrier = paxTypeFare.carrier();

          const PaxType* ptItem = PaxTypeUtil::isAnActualPaxInTrx(trx, ptCarrier, r3PaxType);
          if (ptItem != nullptr)
          {
            paxTypeFare.setFoundCat1R3NoADT();

            if (UNLIKELY(diagEnabled && diagPtr != nullptr))
            {
              (*diagPtr) << "NEED FARE FOR CAT1 R3 PSGR TYPE " << r3PaxType << std::endl;
            }
          }
        }

        if (isQualifyingCat)
        {
          if (UNLIKELY(diagEnabled && factory != nullptr && diagPtr != nullptr))
          {
            (*diagPtr) << "FAIL - PSGR TYPE NOT MATCH WITH INPUT PSGR TYPE" << std::endl;
          }
          return FAIL;
        }
        else
        {
          if (UNLIKELY(diagEnabled && factory != nullptr && diagPtr != nullptr))
          {
            (*diagPtr) << "SKIP - PSGR TYPE NOT MATCH WITH INPUT PSGR TYPE" << std::endl;
          }
          return SKIP;
        }
      }
    } // fcasPaxType empty or ADULT

    else
    {
      if (matchPTCType(eligibilityInfo, trx, paxTypeFare))
      {
        paxTypeFare.setFoundCat1R3NoADT();
      }
      else if (LIKELY(!paxTypeFare.isWebFare() || (paxTypeFare.isWebFare() && isQualifyingCat)))
      {
        if (!isQualifyingCat) // az
        { // az
          if (UNLIKELY(diagEnabled && factory != nullptr && diagPtr != nullptr))
          {
            (*diagPtr) << "SKIP - NOT MATCH ON PSGR TYPE" << std::endl;
          }
          return SKIP;
        } // az
        else // az
        { // az
          if (UNLIKELY(diagEnabled && factory != nullptr && diagPtr != nullptr))
          { // az
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR TYPE" << std::endl; // az
          } // az
          return FAIL; // az
        }
      }
    }

    if (UNLIKELY(diagEnabled && diagPtr != nullptr))
    {
      (*diagPtr) << " MATCH PSGR TYPE" << std::endl;
    }
  }

  paxTypeFare.track("C01PTC");
  return PASS;
}

//---------------------------
// Validate Age Restrictions
//---------------------------
Record3ReturnTypes
Eligibility::checkAgeRestrictions(const EligibilityInfo* eligibilityInfo,
                                  PaxTypeFare& paxTypeFare,
                                  DCFactory* factory,
                                  DiagCollector* diagPtr,
                                  bool isQualifyingCat,
                                  bool diagEnabled) const
{

  if (!eligibilityInfo->psgType().empty())
  {
    if (!paxTypeFare.fcasPaxType().empty() &&
        (eligibilityInfo->minAge() != 0 || eligibilityInfo->maxAge() != 0) &&
        paxTypeFare.paxType() != nullptr &&
        (!paxTypeFare.isWebFare() || (paxTypeFare.isWebFare() && isQualifyingCat)))
    {
      const PaxTypeCode& r3PaxType = eligibilityInfo->psgType();
      const PaxType& paxType = *paxTypeFare.paxType();

      if (UNLIKELY(paxType.paxType() != r3PaxType))
      {
        if (!isQualifyingCat) // az
        { // az
          if (UNLIKELY(diagEnabled))
          {
            (*diagPtr) << "SKIP - NOT MATCH ON PSGR TYPE" << std::endl;
          }
          return SKIP;
        } // az
        else // az
        { // az
          if (UNLIKELY(diagEnabled))
          { // az
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR TYPE" << std::endl; // az
          } // az
          return FAIL; // az
        } // az
      }

      if (paxType.age() != 0)
      {
        if (paxType.age() < eligibilityInfo->minAge())
        {
          if (UNLIKELY(diagEnabled)) // az
          { // az
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR MIN AGE" << std::endl; // az
          } // az
          return FAIL; // az
        }

        if (paxType.age() > eligibilityInfo->maxAge())
        {
          if (UNLIKELY(diagEnabled)) // az
          { // az
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR MAX AGE" << std::endl; // az
          } // az
          return FAIL; // az
        }
        if (UNLIKELY(diagEnabled))
        {
          (*diagPtr) << " MATCH PSGR AGE" << std::endl;
        }
      }
    }
  }

  return PASS;
}

//--------------------------------------------------------------------------
// Validate the Account Code (Corporate Id)
//--------------------------------------------------------------------------
Record3ReturnTypes
Eligibility::checkAccountCode(const EligibilityInfo* eligibilityInfo,
                              PaxTypeFare& paxTypeFare,
                              PricingTrx& trx,
                              DCFactory* factory,
                              const bool& isCat15Qualifying,
                              DiagCollector* diagPtr,
                              bool diagEnabled) const
{
  // Check Record 3 Cat 1 Account Code against
  // Request Corporate Id (WPI) or Request Account Code (WPAC*)

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CHECK PSGR ELIGIBILITY : " << std::endl;
  }

  if (!eligibilityInfo->acctCode().empty()) // Account code field is populated ?
  {
    if (UNLIKELY(trx.isFlexFare() && hasChancelor() && _chancelor->hasPolicy(ELIGIBILITY_RULE) &&
        _chancelor->getPolicy(ELIGIBILITY_RULE).shouldPerform(_chancelor->getContext())))
    {
      PaxTypeFare* ptf = _chancelor->getContext()._paxTypeFare;
      if (checkFlexFareAcctCodeCorpId(eligibilityInfo, *ptf, trx, diagPtr, diagEnabled) == FAIL)
      {
        if (!isFlexFareGroupRequireAccCodeOrCorpId(trx))
          return FAIL;
        else if (shouldReturn(ELIGIBILITY_RULE))
          return FAIL;
      }
    }
    else if (trx.getRequest()->isMultiAccCorpId()) // Yes - multiple AccCode/CorpID request ?
    {
      if (checkMultiAccCodeCorpId(eligibilityInfo, paxTypeFare, trx, diagPtr, diagEnabled) == FAIL)
        return FAIL;
    }
    else // single AccCode/CorpID request
    {
      const std::string& corporateID = getCorporateID(trx);
      const std::string& accountCode = getAccountCode(trx);

      if (UNLIKELY(!corporateID.empty())) // Yes - Corporate ID input ?
      {
        if (eligibilityInfo->acctCode() != corporateID) // Yes - Corp ID matched ?
        {
          if (!checkCorpIdMatrix(eligibilityInfo, corporateID, paxTypeFare, trx))
          {
            if (UNLIKELY(diagEnabled))
            {
              (*diagPtr) << "FAIL - PSGR CORP ID NOT MATCHED : " << corporateID << std::endl;
            }
            return FAIL;
          }
        }
      }
      else if (UNLIKELY(!accountCode.empty())) // Account code input ?
      {
        if (eligibilityInfo->acctCode() != accountCode) // Yes - Account code matched ?
        {
          if (UNLIKELY(diagEnabled))
          {
            (*diagPtr) << "FAIL - PSGR ACCOUNT CODE NOT MATCHED : " << accountCode << std::endl;
          }
          return FAIL;
        }
      }
      else // Corporate ID or Account code input ?
      {
        if (UNLIKELY(diagEnabled))
        {
          (*diagPtr) << "FAIL - NEED VALID INPUT ACCOUNT CODE" << std::endl;
        }
        return FAIL;
      }
    }

    if (LIKELY(!eligibilityInfo->acctCode().empty())) // Account code field is populated ?
    {
      paxTypeFare.setMatchedCorpID();

      if (paxTypeFare.matchedAccCode().empty())
        paxTypeFare.matchedAccCode() = eligibilityInfo->acctCode().c_str();

      if (isCat15Qualifying)
      {
        paxTypeFare.setMatchedCat15QualifyingCorpID();
      }
    }
  }

  paxTypeFare.track("C01ACC");
  return PASS;
}

bool
Eligibility::checkCorpIdMatrix(const EligibilityInfo* eligibilityInfo,
                               const std::string& corporateID,
                               const PaxTypeFare& paxTypeFare,
                               PricingTrx& trx) const
{
  return checkCorpIdMatrix(
      eligibilityInfo, corporateID, paxTypeFare, trx, paxTypeFare.fareMarket()->travelDate());
}

//--------------------------------------------------------------------------
// Validate the Corporate Id Matrix
//--------------------------------------------------------------------------
bool
Eligibility::checkCorpIdMatrix(const EligibilityInfo* eligibilityInfo,
                               const std::string& corporateID,
                               const PaxTypeFare& paxTypeFare,
                               PricingTrx& trx,
                               const DateTime& travelDate) const
{
  const std::vector<tse::CorpId*>& corpIds =
      trx.dataHandle().getCorpId(corporateID, paxTypeFare.carrier(), travelDate);

  if (corpIds.empty())
  {
    const std::vector<tse::CorpId*>& corpIdsOfBlankCxr =
        trx.dataHandle().getCorpId(corporateID, "", travelDate);
    return (matchCorpIdMatrix(eligibilityInfo, paxTypeFare, corpIdsOfBlankCxr));
  }
  else
  {
    return (matchCorpIdMatrix(eligibilityInfo, paxTypeFare, corpIds));
  }
}

//--------------------------------------------------------------------------
// match the Corporate Id Matrix
//--------------------------------------------------------------------------
bool
Eligibility::matchCorpIdMatrix(const EligibilityInfo* eligibilityInfo,
                               const PaxTypeFare& paxTypeFare,
                               const std::vector<tse::CorpId*>& corpIds) const
{
  std::vector<CorpId*>::const_iterator i = corpIds.begin();
  for (; i != corpIds.end(); ++i)
  {
    CorpId& corpId = **i;
    if (corpId.accountCode().empty())
      continue;
    if (!corpId.accountCode().empty() && corpId.accountCode() != eligibilityInfo->acctCode())
      continue;
    if (corpId.vendor() != paxTypeFare.vendor())
      continue;
    if (corpId.ruleTariff() != -1 && corpId.ruleTariff() != paxTypeFare.tcrRuleTariff())
      continue;
    if (!corpId.rule().empty() && corpId.rule() != paxTypeFare.ruleNumber())
      continue;

    paxTypeFare.track("C01CRPID");
    return true;
  }
  return false;
}

//-------------------------
// check passenger status
//-------------------------
Record3ReturnTypes
Eligibility::checkPassengerStatus(const EligibilityInfo* eligibilityInfo,
                                  PaxTypeFare& paxTypeFare,
                                  PricingTrx& trx,
                                  DCFactory* factory,
                                  DiagCollector* diagPtr,
                                  bool diagEnabled) const
{
  LocTypeCode paxLocType = eligibilityInfo->loc1().locType();
  LocCode paxLoc = eligibilityInfo->loc1().loc();

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "PASSENGER STATUS   : ";
    if (eligibilityInfo->psgStatus() == BLANK)
    {
      (*diagPtr) << "NONE" << std::endl;
    }
    else
    {
      (*diagPtr) << eligibilityInfo->psgStatus() << std::endl;
    }
    (*diagPtr) << "APPLICATION STATUS : ";
    if (eligibilityInfo->psgAppl() == RuleConst::NOT_ALLOWED)
    {
      (*diagPtr) << "NOT ALLOWED" << std::endl;
    }
    else
    {
      (*diagPtr) << "ALLOWED" << std::endl;
    }
    (*diagPtr) << "GEO LOCATION      : ";
    if (paxLocType == BLANK)
    {
      (*diagPtr) << "NONE" << std::endl;
    }
    else
    {
      (*diagPtr) << paxLocType << "-" << paxLoc << std::endl;
    }
  }

  if (!matchPassengerStatus(*eligibilityInfo, trx, paxTypeFare))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - NOT MATCH PASSENGER STATUS" << std::endl;
    }
    return FAIL;
  }

  paxTypeFare.track("C01PAX");
  return PASS;
}

//--------------------------
// Match Passenger Status
//--------------------------
bool
Eligibility::matchPassengerStatus(const EligibilityInfo& eInfo,
                                  const PricingTrx& trx,
                                  const PaxTypeFare& fare) const
{
  Indicator paxInd = eInfo.psgStatus();
  StateCode stateCode("");

  if (fare.paxType() != nullptr)
    stateCode = fare.paxType()->stateCode();

  // if nothing specific to match against, match with what we've got
  if (paxInd == BLANK && stateCode.empty())
  {
    const PricingOptions& option = *trx.getOptions();

    if (UNLIKELY(!option.nationality().empty()))
    {
      paxInd = LocUtil::PAX_NATIONALITY;
      fare.track("C01PAXN");
    }
    else if (!option.residency().empty())
    {
      paxInd = LocUtil::PAX_RESIDENCY;
      fare.track("C01PAXR");
    }
    else if (UNLIKELY(!option.employment().empty()))
    {
      paxInd = LocUtil::PAX_EMPLOYEE;
      fare.track("C01PAXE");
    }
    else
      return true;
  }

  GeoTravelType geoTvlType = GeoTravelType::International;
  if (LIKELY(fare.fareMarket() != nullptr))
  {
    geoTvlType = fare.fareMarket()->geoTravelType();
  }

  return LocUtil::matchPaxStatus(
      eInfo.loc1(), eInfo.vendor(), paxInd, eInfo.psgAppl(), stateCode, trx, geoTvlType);
}

//------------------------------------------------------------------------------
Record3ReturnTypes
Eligibility::validate(PricingTrx& trx,
                      const RuleItemInfo* rule,
                      const FarePath& farePath,
                      const PricingUnit& pricingUnit,
                      const FareUsage& fareUsage,
                      const bool& isQualifyingCat)
{
  return validate(trx, rule, pricingUnit, fareUsage, isQualifyingCat);
}

//------------------------------------------------------------------------------
Record3ReturnTypes
Eligibility::validate(PricingTrx& trx,
                      const RuleItemInfo* rule,
                      const PricingUnit& pricingUnit,
                      const FareUsage& fareUsage,
                      const bool& isQualifyingCat,
                      const bool isQualifyingCat27)
{

  // this function should be called for Qualifying Cat1 only ..
  LOG4CXX_INFO(logger, " Entered Eligibility::revalidate() PricingUnit");

  const EligibilityInfo* eligibilityInfo = dynamic_cast<const EligibilityInfo*>(rule);

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic301))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic301);
    diagEnabled = true;
  }

  if (UNLIKELY(!eligibilityInfo))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "UNABLE TO PROCESS - DATA ERROR "
                 << "R3:" << rule->itemNo() << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  const PaxType* paxType =
      (isQualifyingCat27) ? fareUsage.paxTypeFare()->actualPaxType() : pricingUnit.paxType();

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "PRICING UNIT VALIDATION " << std::endl;

    if (fareUsage.cat25Fare())
    {
      (*diagPtr) << "FAREPATH PAX TYPE : "
                 << fareUsage.cat25Fare()->fcasPaxType(); // Cat 25 dummy rec1 pax type
    }
    else
      (*diagPtr) << "FAREPATH PAX TYPE : "
                 << fareUsage.paxTypeFare()->fcasPaxType(); // Rec1 Pax Type

    (*diagPtr) << " AGE : " << paxType->age() << std::endl
               << "R3 PAX TYPE       : " << eligibilityInfo->psgType()
               << " MIN AGE : " << eligibilityInfo->minAge()
               << " MAX AGE : " << eligibilityInfo->maxAge() << std::endl
               << "R3 ACCOUNT CODE/CORP ID : " << eligibilityInfo->acctCode() << std::endl;
  }

  Record3ReturnTypes result = checkUnavailableAndText(eligibilityInfo);
  if (UNLIKELY(result == FAIL))
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED "
                 << "- CHECK UNAVAILABLE" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }
  else if (result == SKIP)
  {
    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - SKIP");

    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: SKIP "
                 << "- TEXT ONLY" << std::endl;
      diagPtr->flushMsg();
    }
    return SKIP;
  }

  bool isWebFare = fareUsage.paxTypeFare()->isWebFare();

  if (!eligibilityInfo->psgType().empty())
  {
    const PaxTypeFare* correctPtf = nullptr;
    correctPtf = fareUsage.cat25Fare() ? fareUsage.cat25Fare() : fareUsage.paxTypeFare();

    if (((correctPtf->fcasPaxType().empty() || correctPtf->fcasPaxType() == ADULT) &&
         eligibilityInfo->psgType() != ADULT) ||
        ((!(correctPtf->fcasPaxType().empty() || correctPtf->fcasPaxType() == ADULT)) &&
         eligibilityInfo->psgType() != correctPtf->fcasPaxType()))

    {

      if (LIKELY(!isWebFare || (isWebFare && isQualifyingCat)))
      {

        if (!isQualifyingCat)
        {
          if (UNLIKELY(diagEnabled))
          {
            (*diagPtr) << "SKIP - NOT MATCH ON PSGR TYPE" << std::endl;

            diagPtr->flushMsg();
          }
          return SKIP;
        }
        else
        {
          if (UNLIKELY(diagEnabled))
          {
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR TYPE" << std::endl;
            diagPtr->flushMsg();
          }
          return FAIL;
        }
      } // web fare if
    } // if part

    // All match scenario's
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " MATCH PSGR TYPE" << std::endl;
    }

    if (!fareUsage.paxTypeFare()->fcasPaxType().empty() &&
        (eligibilityInfo->minAge() != 0 || eligibilityInfo->maxAge() != 0) &&
        (!isWebFare || (isWebFare && isQualifyingCat)))
    {
      // Validating Age Restrictions
      const PaxTypeCode& r3PaxType = eligibilityInfo->psgType();
      const CarrierCode& ptCarrier = fareUsage.paxTypeFare()->carrier();

      const PaxType* ptItem = PaxTypeUtil::isAnActualPaxInTrx(trx, ptCarrier, r3PaxType);

      if (ptItem == nullptr)
      {
        if (!isQualifyingCat) // az
        { // az
          if (UNLIKELY(diagEnabled))
          {
            (*diagPtr) << "SKIP - NOT MATCH ON PSGR AGE" << std::endl;
            diagPtr->flushMsg();
          }
          return SKIP;
        } // az
        else // az
        { // az
          if (UNLIKELY(diagEnabled))
          { // az
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR AGE" << std::endl; // az
            diagPtr->flushMsg(); // az
          } // az
          return FAIL; // az
        } // az
      }

      if (ptItem->age() != 0)
      {
        if (ptItem->age() < eligibilityInfo->minAge())
        {
          if (UNLIKELY(diagEnabled))
          { // az
            (*diagPtr) << "FAIL - NOT MATCH ON PSGR MIN AGE" << std::endl; // az
            diagPtr->flushMsg(); // az
          } // az
          return FAIL; // az
        }
        if (ptItem->age() > eligibilityInfo->maxAge())
        {
          if (UNLIKELY(diagEnabled))
          { // az

            (*diagPtr) << "FAIL - NOT MATCH ON PSGR MAX AGE" << std::endl; // az
            diagPtr->flushMsg(); // az
          } // az
          return FAIL; // az
        }
        if (UNLIKELY(diagEnabled))
        {
          (*diagPtr) << " MATCH PSGR AGE" << std::endl;
        }
      }
    }
  }

  //--------------------------------------------------------------------------
  // Validate the Account Code (Corporate Id)
  //--------------------------------------------------------------------------

  // Check Record 3 Cat 1 Account Code against
  // Request Corporate Id (WPI) or Request Account Code (WPAC*)
  if (!eligibilityInfo->acctCode().empty()) // Account code field is populated ?
  {
    if (UNLIKELY(trx.isFlexFare() && hasChancelor() && _chancelor->hasPolicy(ELIGIBILITY_RULE) &&
        _chancelor->getPolicy(ELIGIBILITY_RULE).shouldPerform(_chancelor->getContext())))
    {
      PaxTypeFare* ptf = _chancelor->getContext()._paxTypeFare;
      if (checkFlexFareAcctCodeCorpId(eligibilityInfo, *ptf, trx, diagPtr, diagEnabled) == FAIL)
      {
        if (!isFlexFareGroupRequireAccCodeOrCorpId(trx))
          return FAIL;
        else if (shouldReturn(ELIGIBILITY_RULE))
          return FAIL;
      }
    }
    else if (trx.getRequest()->isMultiAccCorpId()) // Yes - multiple AccCode/CorpID request ?
    {
      if (checkMultiAccCodeCorpId(
              eligibilityInfo, *fareUsage.paxTypeFare(), trx, diagPtr, diagEnabled) == FAIL)
        return FAIL;
    }
    else // single AccCode/CorpId request
    {
      const std::string& corporateID = getCorporateID(trx);
      const std::string& accountCode = getAccountCode(trx);

      if (!corporateID.empty()) // Yes - Corporate ID input ?
      {
        if (eligibilityInfo->acctCode() != corporateID) // Yes - Corp ID matched ?
        {
          if (!checkCorpIdMatrix(eligibilityInfo, corporateID, *fareUsage.paxTypeFare(), trx))
          {
            if (UNLIKELY(diagEnabled))
            {
              (*diagPtr) << "FAIL - PSGR CORP ID NOT MATCHED : " << corporateID << std::endl;
              diagPtr->flushMsg();
            }
            return FAIL;
          }
        }
      }
      else if (!accountCode.empty()) // Account code input ?
      {
        if (eligibilityInfo->acctCode() != accountCode) // Yes - Account code matched ?
        {
          if (UNLIKELY(diagEnabled))
          {
            (*diagPtr) << "FAIL - PSGR ACCOUNT CODE NOT MATCHED : " << accountCode << std::endl;
            diagPtr->flushMsg();
          }
          return FAIL;
        }
      }
      else // Corporate ID or Account code input ?
      {
        if (UNLIKELY(diagEnabled))
        {
          (*diagPtr) << "FAIL - NEED VALID INPUT ACCOUNT CODE" << std::endl;
          diagPtr->flushMsg();
        }
        return FAIL;
      }
    }
  }

  // check passenger status
  //
  LocTypeCode paxLocType = eligibilityInfo->loc1().locType();
  LocCode paxLoc = eligibilityInfo->loc1().loc();

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "PASSENGER STATUS   : ";
    if (eligibilityInfo->psgStatus() == BLANK)
    {
      (*diagPtr) << "NONE" << std::endl;
    }
    else
    {
      (*diagPtr) << eligibilityInfo->psgStatus() << std::endl;
    }
    (*diagPtr) << "APPLICATION STATUS : ";
    if (eligibilityInfo->psgAppl() == RuleConst::NOT_ALLOWED)
    {
      (*diagPtr) << "NOT ALLOWED" << std::endl;
    }
    else
    {
      (*diagPtr) << "ALLOWED" << std::endl;
    }
    (*diagPtr) << "GEO LOCATION      : ";
    if (paxLocType == BLANK)
    {
      (*diagPtr) << "NONE" << std::endl;
    }
    else
    {
      (*diagPtr) << paxLocType << "-" << paxLoc << std::endl;
    }
  }

  if (UNLIKELY(!matchPassengerStatus(*eligibilityInfo, trx, *fareUsage.paxTypeFare())))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - NOT MATCH PASSENGER STATUS" << std::endl;
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - PASS");

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "PASS" << std::endl;
    diagPtr->flushMsg();
  }

  fareUsage.paxTypeFare()->track("C01VALPU");
  return PASS;
}

bool
Eligibility::isRexNewItinNeedKeepFare(const PricingTrx& trx) const
{
  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    const RexPricingTrx* rexTrx = static_cast<const RexPricingTrx*>(&trx);

    if (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
    {
      return (_itin) ? rexTrx->needRetrieveKeepFare(rexTrx->getItinPos(_itin))
                     : rexTrx->needRetrieveKeepFare();
    }
  }

  return false;
}

const std::string&
Eligibility::getCorporateID(PricingTrx& trx) const
{
  if (UNLIKELY(isRexNewItinNeedKeepFare(trx)))
    return (static_cast<RexPricingRequest*>(trx.getRequest()))->newCorporateID();
  else
    return trx.getRequest()->corporateID();
}

const std::string&
Eligibility::getAccountCode(PricingTrx& trx) const
{
  if (UNLIKELY(isRexNewItinNeedKeepFare(trx)))
    return (static_cast<RexPricingRequest*>(trx.getRequest()))->newAccountCode();
  else
    return trx.getRequest()->accountCode();
}

Record3ReturnTypes
Eligibility::checkMultiAccCodeCorpId(const EligibilityInfo* eligibilityInfo,
                                     const PaxTypeFare& paxTypeFare,
                                     PricingTrx& trx,
                                     DiagCollector* diagPtr,
                                     bool diagEnabled) const
{
  const std::vector<std::string>& corpIdVec = trx.getRequest()->corpIdVec();
  const std::vector<std::string>& accCodeVec = trx.getRequest()->accCodeVec();

  // if here was NO CorpId/AccCode in the request and there is AccCode/CorpId in the Rule/Cat1
  // display the diagnostic and return FAIL
  // for future removal of single AccCode/CorpId logic
  if (UNLIKELY(corpIdVec.empty() && accCodeVec.empty()))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - NEED VALID INPUT ACCOUNT CODE" << std::endl;
    }
    return FAIL;
  }

  std::vector<std::string>::const_iterator vecIter;
  bool isMatched = false;

  // loop through CorpId vector and set isMatched to true if
  // requested CorpId and fare CorpId (or CorpId matrix) was matched
  for (vecIter = corpIdVec.begin(); vecIter != corpIdVec.end() && !isMatched; vecIter++)
  {
    isMatched = (eligibilityInfo->acctCode() == *vecIter ||
                 checkCorpIdMatrix(eligibilityInfo, *vecIter, paxTypeFare, trx));
  }

  // loop through AccCode vector and set isMatched to true if
  // requested AccCode and fare AccCode was matched
  for (vecIter = accCodeVec.begin(); vecIter != accCodeVec.end() && !isMatched; vecIter++)
  {
    isMatched = (eligibilityInfo->acctCode() == *vecIter); // Yes - Account code matched ?
  }

  // if NO AccCode/CorpId was matched display diagnostic and return FAIL
  if (!isMatched)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - PSGR ACC CODE/CORP ID NOT MATCHED";

      if (!corpIdVec.empty())
      {
        (*diagPtr) << std::endl << "  CORP IDS:";

        for (vecIter = corpIdVec.begin(); vecIter != corpIdVec.end(); vecIter++)
          (*diagPtr) << " " << (*vecIter);
      }

      if (!accCodeVec.empty())
      {
        (*diagPtr) << std::endl << "  ACC CODES:";

        for (vecIter = accCodeVec.begin(); vecIter != accCodeVec.end(); vecIter++)
          (*diagPtr) << std::endl << "             " << (*vecIter);
      }

      (*diagPtr) << std::endl;
    }
    return FAIL;
  }

  // if the processing is at this point the CorpId/AccCode was matched
  paxTypeFare.track("C01MACC");
  return PASS;
}

Record3ReturnTypes
Eligibility::checkFlexFareAcctCodeCorpId(const EligibilityInfo* eligibilityInfo,
                                         PaxTypeFare& ptf,
                                         PricingTrx& trx,
                                         DiagCollector* diagPtr,
                                         bool diagEnabled) const
{
  if (!_chancelor)
    return FAIL;

  // if TotalAttrs' corp ID and account code fields are empty, that means
  // there was NO CorpId/AccCode in the flex fare request. Since  there
  // is AccCode/CorpId in the Rule/Cat1, then display the diagnostic
  // and return FAIL
  if ((!trx.getFlexFaresTotalAttrs().isValidationNeeded<flexFares::CORP_IDS>()) &&
      (!trx.getFlexFaresTotalAttrs().isValidationNeeded<flexFares::ACC_CODES>()))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - NEED VALID FLEX FARE ACCOUNT CODE/CORP ID" << std::endl;
    }
    return FAIL;
  }

  std::vector<std::string> corpIdDiagVec;
  std::vector<std::string> accCodeDiagVec;
  bool validationPassed = false;

  if (areFlexFaresAccCodesCorpIdsMatched(
          eligibilityInfo, ptf, trx, flexFares::ACC_CODES, accCodeDiagVec))
  {
    validationPassed = true;
  }
  else
  {
    validationPassed = areFlexFaresAccCodesCorpIdsMatched(
        eligibilityInfo, ptf, trx, flexFares::CORP_IDS, corpIdDiagVec);
  }

  // if NO AccCode/CorpId was matched display diagnostic and return FAIL
  if (!validationPassed)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - FLEXFARE ACC CODE/CORP ID NOT MATCHED WITH "
                 << eligibilityInfo->acctCode() << "\n";

      // corp Ids are not empty in flex fare groups
      if (trx.getFlexFaresTotalAttrs().isValidationNeeded<flexFares::CORP_IDS>())
      {
        (*diagPtr) << "  CORP IDS:";

        for (const std::string& corpIds : corpIdDiagVec)
        {
          (*diagPtr) << " " << corpIds;
        }
        (*diagPtr) << std::endl;
      }

      // account codes are not empty in in flex fare groups
      if (trx.getFlexFaresTotalAttrs().isValidationNeeded<flexFares::ACC_CODES>())
      {
        (*diagPtr) << "  ACC CODES:";

        for (const std::string& accCode : accCodeDiagVec)
        {
          (*diagPtr) << " " << accCode;
        }
        (*diagPtr) << std::endl;
      }
    }
    return FAIL;
  }

  // if the processing is at this point the CorpId/AccCode were matched
  ptf.track("C01FACC");
  return PASS;
}

bool
Eligibility::areFlexFaresAccCodesCorpIdsMatched(const EligibilityInfo* eligibilityInfo,
                                                PaxTypeFare& ptf,
                                                PricingTrx& trx,
                                                flexFares::Attribute attrName,
                                                std::vector<std::string>& diagStrVec) const
{
  if (!_chancelor)
    return false;

  RuleValidationContext::ContextType contextType = _chancelor->getContext()._contextType;
  flexFares::GroupId groupId = _chancelor->getContext()._groupId;

  bool isMatched = false;

  if (contextType == RuleValidationContext::FARE_MARKET)
  {
    const flexFares::AttrComplexStats<std::string>& allGroups =
        (attrName == flexFares::ACC_CODES)
            ? trx.getFlexFaresTotalAttrs().getAllGroups<flexFares::ACC_CODES>()
            : trx.getFlexFaresTotalAttrs().getAllGroups<flexFares::CORP_IDS>();

    for (const flexFares::AttrComplexStats<std::string>::value_type& group : allGroups)
    {
      if (isSingleFlexFaresAccCodeCorpIdMatched(eligibilityInfo, ptf, trx, attrName, group.first))
      {
        isMatched = true;
        if (attrName == flexFares::ACC_CODES)
          break;
      }
      // No match so save them in a vector for diag display
      // and save them now so there's no need to loop thru both
      // TotalAttrs and the GroupsData's data structure again
      // later in the diag display
      diagStrVec.push_back(group.first);
    }
  }
  else // for fare usage context type
  {
    const std::set<std::string>* stringSet =
        (attrName == flexFares::CORP_IDS)
            ? &trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId)
            : &trx.getRequest()->getFlexFaresGroupsData().getAccCodes(groupId);

    for (const std::string& acctCodeCorpId : *stringSet)
    {
      if (isSingleFlexFaresAccCodeCorpIdMatched(
              eligibilityInfo, ptf, trx, attrName, acctCodeCorpId))
      {
        isMatched = true;
        // For fare usage, we can always break the loop in case of a match
        // (even for corpIds), because we already know for which group we
        // validate the fare
        break;
      }
      diagStrVec.push_back(acctCodeCorpId);
    }
  }

  return isMatched;
}

bool
Eligibility::isSingleFlexFaresAccCodeCorpIdMatched(const EligibilityInfo* eligibilityInfo,
                                                   PaxTypeFare& ptf,
                                                   PricingTrx& trx,
                                                   flexFares::Attribute attrName,
                                                   const std::string& corpIdAccCodeStr) const
{
  if (!_chancelor)
    return false;

  bool isMatched = false;

  if (eligibilityInfo->acctCode() == corpIdAccCodeStr)
  {
    isMatched = true;
  }
  else // not matched
  {
    if (attrName == flexFares::CORP_IDS)
    {
      isMatched = checkCorpIdMatrix(eligibilityInfo, corpIdAccCodeStr, ptf, trx);
    }
  }

  if (isMatched)
  {
    _chancelor->getMutableMonitor().notify(RuleValidationMonitor::VALIDATION_RESULT,
                                           _chancelor->getContext(),
                                           corpIdAccCodeStr,
                                           ptf.getMutableFlexFaresValidationStatus(),
                                           (attrName == flexFares::ACC_CODES));
  }
  return isMatched;
}

bool
Eligibility::isFlexFareGroupRequireAccCodeOrCorpId(PricingTrx& trx) const
{
  //_matchEmptyAccCode - set to true if there is a flex fare group that does not require any acc
  //code/corp id
  return (!trx.getFlexFaresTotalAttrs().matchEmptyAccCode());
}

} // tse
