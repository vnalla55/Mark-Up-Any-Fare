//-------------------------------------------------------------------
//
//  File:         AccompaniedTravel.cpp
//  Author:       Simon Li
//  Created:      08/12/2004
//  Description:  Accompanied Travel Restriction category is used when
//                travel accompanying restrictions are specifically
//                stated in a rule, Cat13 or Cat19-22.
//                The restrictions include accompanying passenger type, age,
//                Fare Class/Booking Code, number of accompanying passengers,
//                same Sector, Compartment and Rule requirements.
//                The rule data class for Cat13 is AccompaniedTravelInfo, one
//                for Cat19-22 is DiscountInfo.
//                This is a Rule Application class that supports one public
//                validate function for Fare Market on Cat13, one for Discount
//                fare creation on Cat19-22, and one for Fare Usage combinabilty
//                process on both Cat13 and Cat19-22.
//
//  Copyright Sabre 2004
//
//        The copyright to the computer program(s) herein
//        is the property of Sabre.
//        The program(s) may be used and/or copied only with
//        the written permission of Sabre or in accordance
//        with the terms and conditions stipulated in the
//        agreement/contract under which the program(s)
//        have been supplied.
//
//-------------------------------------------------------------------

#include "Rules/AccompaniedTravel.h"

#include "Common/AccTvlDetailIn.h"
#include "Common/DateTime.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SimplePaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/DiscountSegInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{

static Logger
logger("atseintl.Rules.AccompaniedTravel");

const ACCTVL_VALID_RESULT AccompaniedTravel::accTvlPASS = "MATCH ALL";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonNoAccPsg = "NUM OF ACCOMP PSG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonPsgType = "TYPE OF ACCOMP PSG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonAge = "AGE OF ACCOMP PSG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonFB = "FARE CLASS/BOOKING CODE";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonSameSeg = "SAME SECTOR";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonSameCpmt = "SAME COMPARTMENT";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonSameRuleNum = "SAME RULE NUMBER";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonSameRuleTariff = "SAME RULE TARIFF";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonSameRuleCarr = "SAME RULE CARRIER";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonEmptySeg = "EMPTY ACCOMPANIED TVL SEG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonEmptyAccSeg = "EMPTY ACCOMPANYING TVL SEG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonGeoVia1 = "ON GEO VIA1 SEG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonGeoVia2 = "ON GEO VIA2 SEG";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonPaxTypeNoMatch =
    "NO MATCH ON ACCOMPANIED PSG TYPE";
const ACCTVL_VALID_RESULT AccompaniedTravel::failReasonMUSTNOT = "MATCH ON MUST NOT";
const std::string AccompaniedTravel::DIAG_FAIL_MSG = " ACCOMPANIED TRAVEL: FAIL - ";
const std::string AccompaniedTravel::LOG_FAIL_MSG = " Leaving AccompaniedTravel::validate() - FAIL";

const int16_t AccompaniedTravel::minInfAccompFareAge = 12;

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validate()
//
// validate()   Performs rule validations for Cat13 AccompaniedTravel
//              on a FareMarket. We will validate PaxTypeCode, Age and number
//              of Accompanied/Accompanying passengers.
//
//  @param PricingTrx&          - Pricing transaction
//  @param Itin&                - itinerary
//  @param PaxTypeFare&         - reference to Pax Type Fare
//  @param RuleItemInfo*        - Record 2 Rule Item Segment Info
//                                will dynamic_cast to <AccompaniedTravelInfo*>
//  @param FareMarket&          - Fare Market
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
AccompaniedTravel::validate(PricingTrx& trx,
                            Itin& itin,
                            const PaxTypeFare& paxTypeFare,
                            const RuleItemInfo* rule,
                            const FareMarket& fareMarket)
{

  LOG4CXX_INFO(logger, " Entered AccompaniedTravel::validate() for Fare Market");

  if (!paxTypeFare.actualPaxType())
    return FAIL;

  const AccompaniedTravelInfo* restrictionInfo = dynamic_cast<const AccompaniedTravelInfo*>(rule);

  if (!restrictionInfo)
  {
    LOG4CXX_ERROR(logger, "Not valid AccompaniedTravelInfo");
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);
    return FAIL;
  }
  const AccompaniedTravelInfo& accTvlInfo = *restrictionInfo;

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;

  if (trx.diagnostic().shouldDisplay(
          paxTypeFare, RuleConst::ACCOMPANIED_PSG_RULE, paxTypeFare.actualPaxType()->paxType()))
  {
    diag.enable(Diagnostic313, Diagnostic863);
  }

  if (diag.isActive())
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic863)
    {
      diag << " \n \n WPA ACCOMPANIED TRAVEL RESTRICTIONS - CAT13\n";
      diag.printLine();

      if (paxTypeFare.fareMarket())
      {
        diag << (*paxTypeFare.fareMarket());
      }
      diag << paxTypeFare;
    }
    displayAccTvlRuleToDiag(accTvlInfo, diag);
  }

  //----------------------------------------------------------------
  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  //----------------------------------------------------------------
  const Record3ReturnTypes unavailableFlag = validateUnavailableDataTag(accTvlInfo.unavailTag());
  if (unavailableFlag == FAIL)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL: FAIL - DATA UNAVAILABLE" << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);
    return FAIL;
  }
  else if (unavailableFlag == SKIP)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL: SKIP - TXT DATA ONLY" << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, " Leaving AccompaniedTravel::validate() - SKIP");

    return SKIP;
  }

  //----------------------------------------------------------
  // Diagnostic this paxType
  //----------------------------------------------------------
  if (UNLIKELY(diag.isActive()))
  {
    const PaxType& myPaxType = *(paxTypeFare.actualPaxType());
    diag << " ACCOMPANIED PSG TYPE - " << myPaxType.paxType() << std::endl;

    // Any actual type name under carrier
    std::map<CarrierCode, std::vector<PaxType*>*>::const_iterator myActualPaxTypeI =
        myPaxType.actualPaxType().begin();

    for (; myActualPaxTypeI != myPaxType.actualPaxType().end(); myActualPaxTypeI++)
    {
      std::vector<PaxType*>::const_iterator paxTypeI;
      paxTypeI = (*myActualPaxTypeI).second->begin();
      for (; paxTypeI != (*myActualPaxTypeI).second->end(); paxTypeI++)
      {
        diag << "   CARRIER " << (*myActualPaxTypeI).first << "    " << (*paxTypeI)->number()
             << "  ACTUAL TYPE - " << (*paxTypeI)->paxType() << std::endl;
      }
    }
  }

  BaseExchangeTrx* excTrx = dynamic_cast<BaseExchangeTrx*>(&trx);
  bool chkExcAccPaxType = false;

  //----------------------------------------------------------------
  // For Fare Market validation we can check passenger type, age
  // requirement and if there is enough accompanying passenger
  //----------------------------------------------------------------
  if (trx.paxType().size() < 2)
  {
    if (trx.paxType().front()->number() > 1)
    {
      // we could pass if same type passenger accompanying, although weird
    }
    else if (excTrx && !excTrx->accompanyPaxType().empty())
    {
      chkExcAccPaxType = true;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << DIAG_FAIL_MSG << failReasonNoAccPsg << std::endl;
        diag.flushMsg();
      }
      LOG4CXX_INFO(logger, LOG_FAIL_MSG);

      return FAIL;
    }
  }

  //------------------------------------------------------
  // Initialize validation result as PASS
  //------------------------------------------------------
  ACCTVL_VALID_RESULT accTvlResult = accTvlPASS;

  //---------------------------------------------------------------
  // Depends on accompanying passenger MUST or MUST_NOT apply, we treat
  // the matching result differently
  //---------------------------------------------------------------
  const bool isMustApply = (accTvlInfo.accPsgAppl() == applyMust);

  //-----------------------------------------------------------------------
  // trx has all passengers' PaxType vector, we check how many passengers
  // are accompanied & how many are qualified to be accompanying passenger
  //-----------------------------------------------------------------------
  uint32_t numOfAccompaniedPsg = 0;
  uint32_t numOfAccompanyingPsg = 0;
  bool groupAccompanyingTvl = false;

  const PaxTypeCode& myPaxTypeCode = trx.paxType().size() == 1
                                         ? trx.paxType()[0]->paxType()
                                         : paxTypeFare.actualPaxType()->paxType();

  std::vector<PaxType*>::const_iterator paxTypeEndI = trx.paxType().end();

  std::vector<PaxType*>::const_iterator paxTypeI = trx.paxType().begin();

  std::vector<PaxType*> tmpPaxTypes;
  if (chkExcAccPaxType)
  {
    tmpPaxTypes.insert(tmpPaxTypes.end(), trx.paxType().begin(), trx.paxType().end());
    tmpPaxTypes.insert(
        tmpPaxTypes.end(), excTrx->accompanyPaxType().begin(), excTrx->accompanyPaxType().end());
    paxTypeI = tmpPaxTypes.begin();
    paxTypeEndI = tmpPaxTypes.end();
  }

  for (; paxTypeI != paxTypeEndI; paxTypeI++)
  {
    if (*paxTypeI == nullptr)
      continue;

    if ((*paxTypeI)->paxType() == myPaxTypeCode)
    {
      numOfAccompaniedPsg += (*paxTypeI)->number();
      if (isMustApply == (accTvlPASS == qualifyAccPsgTypeFare(*(*paxTypeI),
                                                              true, // chkFareClassBkgCds
                                                              paxTypeFare,
                                                              accTvlInfo,
                                                              trx,
                                                              &diag)))
      {
        groupAccompanyingTvl = true;
        numOfAccompanyingPsg += (*paxTypeI)->number() - 1;
        numOfAccompaniedPsg -= (*paxTypeI)->number() - 1;
      }
      continue;
    }

    accTvlResult = qualifyAccPsgTypeFare(*(*paxTypeI),
                                         false, // no chkFareClassBkgCds
                                         paxTypeFare,
                                         accTvlInfo,
                                         trx,
                                         &diag);

    //---------------------------------------------------------------
    // For MUST apply, we want the result be PASS
    // For MUST_NOT apply, we want the result not be PASS
    //---------------------------------------------------------------
    if (isMustApply != (accTvlResult == accTvlPASS))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "  " << (*paxTypeI)->number();
        diag << "  FAILED QUALIFICATION - " << accTvlResult;
        diag << "   " << (*paxTypeI)->paxType() << std::endl;
      }
      //-------------------------------------------------------------
      // For MUST NOT, any FAIL will fail the whole
      // For MUST, FAIL will only disqualify this PaxType
      //-------------------------------------------------------------
      if (!isMustApply)
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << DIAG_FAIL_MSG << accTvlResult << std::endl;
          diag.flushMsg();
        }
        LOG4CXX_INFO(logger, LOG_FAIL_MSG);

        return FAIL;
      }
      continue;
    }

    // Count qualified accompanying psg
    // except INFANT does not count for anything,
    // unless it is coded in rule
    if ((*paxTypeI)->paxTypeInfo()->isInfant() && accTvlInfo.accPsgType() != INFANT &&
        (*paxTypeI)->paxType() != accTvlInfo.accPsgType())
    {
      continue;
    }

    if (UNLIKELY(diag.isActive()))
    {
      diag << "  " << (*paxTypeI)->number();
      diag << "  SUCCEEDED QUALIFICATION - ";
      diag << (*paxTypeI)->paxType() << std::endl;
    }

    numOfAccompanyingPsg += (*paxTypeI)->number();
    if (numOfAccompaniedPsg == 0)
    {
      if (((*paxTypeI)->number() > 1) &&
          PaxTypeUtil::isAnActualPax(**paxTypeI, paxTypeFare.carrier(), myPaxTypeCode, 0, 0) &&
          isMustApply == checkFareClassBkgCode(paxTypeFare, accTvlInfo, trx, nullptr))
      {
        // this PaxType can be priced if grouped
        groupAccompanyingTvl = true;
      }
    }
    continue;
  } // Loop of PaxType vector

  if ((numOfAccompaniedPsg == 0 && !groupAccompanyingTvl) ||
      (groupAccompanyingTvl && numOfAccompanyingPsg < 1))
  {
    //-------------------------------------------------------------
    // Could not see accompanied paxType in trx that applys the rule
    // We do not qualify this
    //-------------------------------------------------------------
    if (UNLIKELY(diag.isActive()))
    {
      diag << DIAG_FAIL_MSG << failReasonPaxTypeNoMatch << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);

    return FAIL;
  }

  if (numOfAccompaniedPsg == 0 && numOfAccompanyingPsg > 0)
  {
    numOfAccompanyingPsg--;
    numOfAccompaniedPsg++;
  }

  // Compare qualified accompanying passenger with min number of
  // accompanying passenger required

  const uint32_t minNoAccPsg = minNoAccPsgReq(numOfAccompaniedPsg, accTvlInfo);
  if (numOfAccompanyingPsg < minNoAccPsg)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << DIAG_FAIL_MSG << failReasonNoAccPsg << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);

    return FAIL;
  }

  // set flag to require re-validation during pricing,
  // for Fare class/Booking code, same sector/compartment/rule requirement
  // For WPA, we always need cat13AccFareBreak so that we would have
  // accompanied travel restriction information stored

  if ((trx.altTrxType() != PricingTrx::WP) || needCombinabiltyCheck(accTvlInfo))
  {
    if (chkExcAccPaxType)
      // for EXCHANGE, we do not price accomapny Pax, so no way to
      // revalidate FareUsages at GroupFarePath
      (const_cast<PaxTypeFare&>(paxTypeFare)).setAccTvlNotValidated(true);
    else
      (const_cast<PaxTypeFare&>(paxTypeFare)).setAccSameFareBreak(true);

    if (diag.isActive())
    {
      diag << " ACCOMPANIED TRAVEL: PASS W/ COMBINABILITY REQ" << std::endl;
      diag.flushMsg();
    }
  }
  else
  {
    if (diag.isActive())
    {
      diag << " ACCOMPANIED TRAVEL: PASS" << std::endl;
      diag.flushMsg();
    }
  }

  LOG4CXX_INFO(logger, " Leaving AccompaniedTravel::validate() - PASS");

  // for WQ (NoPNR) transaction, add warning message
  if (dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr)
  {
    paxTypeFare.fare()->warningMap().set(WarningMap::cat13_warning);
  }
  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validate()
//
// validate()   Performs rule validations for accompanied travel restrictions
//              on a PricingUnit. This should not need to be called, otherwise
//              we just return PASS
//
//  @param PricingTrx&          - Pricing transaction
//  @param RuleItemInfo*        - Record 2 Rule Item Segment Info
//                                need dynamic_cast to <TravelRestriction*>
//  @param FarePath&            - Fare Path
//  @param PricingUnit&         - Pricing unit
//  @param FareUsage&           - Fare Usage
//
//  @return Record3ReturnTypes - possible values are:
//                               PASS
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
AccompaniedTravel::validate(PricingTrx& trx,
                            const RuleItemInfo* rule,
                            const FarePath& farePath,
                            const PricingUnit& pricingUnit,
                            const FareUsage& fareUsage)
{
  return PASS; // this is now a dummy function
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validate()
//
// validate() Performs rule validations for travel restrictions
//            during fare usage combinability process based
//            on FarePaths and FareUsages for all passengers.
//            This is a re-validation for Cat13 and Cat19-22 when
//            all passenger fares and farepath in trx are created.
//            This is only called with same FareBreak check
//
//  @param PricingTrx&               - Pricing transaction
//  @param std::vector<FareUsage*>&  - Fare Usages of all psgs travel together
//
//  @return bool - possible values are:
//            true      Validation succeeded
//            false     Validation failed
//
//-------------------------------------------------------------------
bool
AccompaniedTravel::validate(PricingTrx& trx, std::vector<FareUsage*>& fareUsages)
{
  // There could be more than one fare that need accompanied travel validation
  // We will go through the FareUsage vector to find and validate everyone
  // Because the data format similarity, we will try to share codes as much as
  // we can among validation for Cat13 and Cat19-22.

  std::vector<PaxTypeFare*> paxTypeFares; // In case FareClass/BookingCode
  // restrictions of Cat19-22 apply,
  // we fill and use this to validate

  // Fare Usage requiring accompanied travel is the first one
  FareUsage& currentFU = *fareUsages.front();

  //----------------------------------------------------------------
  // The AccSameFareBreak() flag can actually be set for
  // Cat13 or/and Cat19-22.
  //----------------------------------------------------------------
  PaxTypeFare& ptf = *((PaxTypeFare*)(currentFU.paxTypeFare()));
  // lint -e{530}
  PaxTypeFareRuleData* paxTypeFareRuleData =
      ptf.paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
  if (UNLIKELY(paxTypeFareRuleData != nullptr))
  {
    //---------------------------------------------------------
    // There is Cat13 request
    //---------------------------------------------------------
    const RuleItemInfo* ruleInfo = paxTypeFareRuleData->ruleItemInfo();
    const AccompaniedTravelInfo* accTvlInfo = dynamic_cast<const AccompaniedTravelInfo*>(ruleInfo);

    DCFactory* factory = DCFactory::instance();
    DiagCollector* diagPtr = factory->create(trx);
    DiagCollector& diag = *diagPtr;

    diag.enable(Diagnostic313);
    if (UNLIKELY(diag.isActive()))
    {
      diag << "    \n    \n";

      diag << "******FARE USAGES AT SAME FARE BREAK***************" << std::endl;
      std::vector<FareUsage*>::const_iterator fuI = fareUsages.begin();
      const std::vector<FareUsage*>::const_iterator fuEndI = fareUsages.end();

      for (; fuI != fuEndI; fuI++)
      {
        diagFareUsage(diag, **fuI);
      }
      diag.printLine();

      diag << "CATEGORY 13 ACCOMPANIED TRAVEL DIAGNOSTICS" << std::endl;
      diag.printLine();
      diag << "PHASE: FARE COMBINABILITY     R3 ITEM NUMBER: ";
      diag << "PHASE: FARE COMBINABILITY     R3 ITEM NUMBER: ";
      if (ruleInfo)
        diag << ruleInfo->itemNo() << std::endl;
      else
        diag << "RULE INFO EMPTY\n";

      if (accTvlInfo != nullptr)
      {
        displayAccTvlRuleToDiag(*accTvlInfo, diag);
      }
      diag.flushMsg();
    }

    if (accTvlInfo && validateFareUsages(trx, ruleInfo, currentFU, fareUsages) == FAIL)
    {
      // Special case, this function not called from rule
      // controller, so we do command pricing check here.
      // lint -e{530}
      PaxTypeFare* ptfCmdPricing = currentFU.paxTypeFare();
      if (ptfCmdPricing->isCmdPricing())
      {
        ptfCmdPricing->setCmdPrcFailedFlag(RuleConst::ACCOMPANIED_PSG_RULE);
      }
      else
        return false;
    }
  }

  //---------------------------------------------------------
  // Cat19-22
  //---------------------------------------------------------
  if (LIKELY(currentFU.paxTypeFare()->isDiscounted()))
  {
    try
    {
      // lint -e{530}
      const DiscountInfo& discountInfo(currentFU.paxTypeFare()->discountInfo());

      //------------------------------------------------------------
      // Cat 19 - 22, Discounted fare, defination of accompanying
      // passenger type, age and fareclass/bookingcode info is
      // different with Cat13.
      // We will validate those and number requirement in one
      // function, and then create a Cat13 ruleItemInfo with sector,
      // rule, etc. rest copied to and call Cat13 validation, so that
      // we maintain less codes.
      //------------------------------------------------------------

      DCFactory* factory = DCFactory::instance();
      DiagCollector* diagPtr = factory->create(trx);
      DiagCollector& diag = *diagPtr;

      // Have tried to output diagnosis to 319; but since the
      // diagnosis during discount fare creation was too big that
      // this often can not be seen, we decided to output diagnostic
      // of Cat19-22 during combinability to 313
      diag.enable(Diagnostic313);
      if (UNLIKELY(diag.isActive()))
      {
        diag << "    " << std::endl << "    " << std::endl;
        diag << "CATEGORY 19-22 DISCOUNT FARE ACCOMPANIED TRAVEL DIAGNOSTICS" << std::endl;
        diag << "***************************************************************" << std::endl;
        diag << "PHASE: FARE COMBINABILITY     R3 ITEM NUMBER: " << discountInfo.itemNo()
             << std::endl;

        diagFareUsage(diag, currentFU);
        diag.flushMsg();
      }

      //------------------------------------------------------------
      // Passenger type, age, number and Fare Class,Booking Code
      // If FareClass/BookingCode is not required to be validated,
      // we skip this part, so not to repeat validation we have
      // already done during discount fare creation,
      // but we need repeat validation if this finction
      // is processing the number of fareUsage's < the total PAX types number.
      //------------------------------------------------------------
      if (discountInfo.fareClassBkgCodeInd() != notApply ||
          fareUsages.size() != trx.paxType().size())
      {
        if (paxTypeFares.empty()) // only initial once
        {
          std::vector<FareUsage*>::const_iterator fareUsgI = fareUsages.begin();
          const std::vector<FareUsage*>::const_iterator fareUsageEndI = fareUsages.end();

          for (; fareUsgI != fareUsageEndI; fareUsgI++)
          {
            paxTypeFares.push_back((PaxTypeFare*)((*fareUsgI)->paxTypeFare()));
          }
        }
        if (validate(trx, *(currentFU.paxTypeFare()), paxTypeFares, discountInfo) != PASS)
        {
          PaxTypeFare* ptfCmdPricing = currentFU.paxTypeFare();
          if (ptfCmdPricing->isCmdPricing())
          {
            ptfCmdPricing->setCmdPrcFailedFlag(RuleConst::CHILDREN_DISCOUNT_RULE);
          }
          else
          {
            return false;
          }
        }
      }

      //------------------------------------------------------------
      // The rest, share Cat13 validation, so to maintain less codes
      //------------------------------------------------------------
      AccompaniedTravelInfo accTvlInfo;

      accTvlInfo.itemNo() = discountInfo.itemNo();
      accTvlInfo.unavailTag() = discountInfo.unavailtag();
      accTvlInfo.geoTblItemNoVia1() = discountInfo.geoTblItemNo();
      accTvlInfo.geoTblItemNoVia2() = 0;
      accTvlInfo.accTvlAllSectors() = discountInfo.accTvlAllSectors();
      accTvlInfo.accTvlOut() = discountInfo.accTvlOut();
      accTvlInfo.accTvlOneSector() = discountInfo.accTvlOneSector();
      accTvlInfo.accTvlSameCpmt() = discountInfo.accTvlSameCpmt();
      accTvlInfo.accTvlSameRule() = discountInfo.accTvlSameRule();
      accTvlInfo.accPsgId() = notApply;
      accTvlInfo.accPsgAppl() = applyMust;
      accTvlInfo.accPsgType() = "";
      accTvlInfo.minAge() = 0;
      accTvlInfo.maxAge() = 0;
      accTvlInfo.minNoPsg() = 0;
      accTvlInfo.maxNoPsg() = 0;
      // !! we will ignore FareClass or BookingCode requirement now
      accTvlInfo.fareClassBkgCdInd() = notApply;

      if (validateFareUsages(
              trx, static_cast<const RuleItemInfo*>(&accTvlInfo), currentFU, fareUsages) == FAIL)
      {
        // Special case, this function not called from rule
        // controller, so we do command pricing check here.
        PaxTypeFare* ptfCmdPricing = currentFU.paxTypeFare();
        if (ptfCmdPricing->isCmdPricing())
        {
          ptfCmdPricing->setCmdPrcFailedFlag(RuleConst::CHILDREN_DISCOUNT_RULE);
        }
        else
          return false;
      }
    } // endtry - access discountInfo
    catch (...) { return false; }
  }

  return true;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validate()
//
// validate()    This is called by combinability validation that performs
//               full rule validations for travel restrictions on Cat13.
//               If the Cat13 ruleItemInfo was created from accompanied travel
//               restriction on Cat19-22 (discount fare), FareClass/BookingCode
//               indicator of it will be not set, so that we do not need to
//               validate accompanying passenger type, age,
//               FareClass/BookingCode or number of accompanied passenger
//               against number of accompanying passenger any more. Those
//               has been processed in another function.
//               Process on GeoTblItem was first implemented but currently
//               commented out, because of the fact that there is no clear
//               definition on how to support it yet.
//
//  @param PricingTrx&               - Pricing transaction
//  @param RuleItemInfo*             - Record 2 Rule Item Segment Info
//                                  will dynamic_cast to AccompaniedTravelInfo*
//  @param FarePath&                 - Fare Path of accompanied passenger
//  @param FareUsage&                - Fare Usage of accompanied passenger
//  @param std::vector<FareUsage*>&  - Fare usages of all passengers
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
AccompaniedTravel::validateFareUsages(PricingTrx& trx,
                                      const RuleItemInfo* rule,
                                      const FareUsage& myFareUsage,
                                      const std::vector<FareUsage*>& fareUsages)
{
  LOG4CXX_INFO(logger, " Entered AccompaniedTravel::validate() for Combinability");

  if (UNLIKELY(!myFareUsage.paxTypeFare()->actualPaxType()))
    return FAIL;

  const AccompaniedTravelInfo* restrictionInfo = dynamic_cast<const AccompaniedTravelInfo*>(rule);

  if (UNLIKELY(!restrictionInfo))
  {
    LOG4CXX_ERROR(logger, "Not valid AccompaniedTravelInfo");
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);
    return FAIL;
  }
  const AccompaniedTravelInfo& accTvlInfo = *restrictionInfo;

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic313);

  //----------------------------------------------------------------
  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  //----------------------------------------------------------------
  const Record3ReturnTypes unavailableFlag = validateUnavailableDataTag(accTvlInfo.unavailTag());
  if (UNLIKELY(unavailableFlag == FAIL))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL: FAIL - DATA UNAVAILABLE" << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);
    return FAIL;
  }
  else if (UNLIKELY(unavailableFlag == SKIP))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL: SKIP - TXT DATA ONLY" << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, " Leaving AccompaniedTravel::validate() - SKIP");
    return SKIP;
  }

  //------------------------------------------------------
  // Initialize validation result
  //------------------------------------------------------
  ACCTVL_VALID_RESULT accTvlResult = accTvlPASS;

  //----------------------------------------------------------------
  // Retrieve the accompanying passengers from std::vector<FareUsage*>.
  // In a group of accompanied, accompanying passengers situation,
  // by comparing &myFareUsage and (*iterFareUsage), we know
  // who are the others (accompanying ones).
  // We will get number of accompanied and accompanying passengers,
  // and fare usages of accompanying passengers from fareUsages
  // passed in, for qualification.
  //----------------------------------------------------------------
  const bool isMustApply = (accTvlInfo.accPsgAppl() == applyMust);

  std::vector<FareUsage*> accPaxFareUsage;

  std::vector<FareUsage*>::const_iterator accFareUsageEndI = fareUsages.end();
  std::vector<FareUsage*>::const_iterator accFareUsageI = fareUsages.begin();

  //----------------------------------------------------------------------
  // This function is only called during combinability check. To get here,
  // we have previously checked passenger type, age and number of passengers
  // except we did not have FareClass or BookingCode information that time.
  // Only when FareClass or BookingCode is required to be checked, we
  // need to revalidate Age, PaxTypeCode, FareClass/BookingCode as well
  // as the number of Accompanied/Accompanying Passenger here
  //----------------------------------------------------------------
  if (accTvlInfo.fareClassBkgCdInd() != notApply || accTvlInfo.accTvlSameRule() != notApply)
  {
    uint32_t numOfAccompaniedPsg = myFareUsage.paxTypeFare()->actualPaxType()->number();
    uint32_t numOfAccompanyingPsg = 0;

    for (; accFareUsageI != accFareUsageEndI; accFareUsageI++)
    {
      if ((*accFareUsageI) != &myFareUsage)
      {
        if (LIKELY((*accFareUsageI)->paxTypeFare() != nullptr &&
            (*accFareUsageI)->paxTypeFare()->actualPaxType() != nullptr))
        {
          // lint -e{578}
          const PaxType& paxType = *((*accFareUsageI)->paxTypeFare()->actualPaxType());
          /*
          // Those with same PaxType as mine will be seen as
          // accompanied passenger as well
          if (paxType.paxType() == myFareUsage.paxTypeFare()->actualPaxType()->paxType())
          {
              numOfAccompaniedPsg += paxType.number();
              continue;
          }
          */

          accTvlResult = qualifyAccPsgTypeFare(
              paxType, true, *(*accFareUsageI)->paxTypeFare(), accTvlInfo, trx, &diag);

          if (UNLIKELY(isMustApply != (accTvlResult == accTvlPASS)))
          {
            if (!isMustApply)
              accTvlResult = failReasonMUSTNOT;

            if (UNLIKELY(diag.isActive()))
            {
              diag << "  " << paxType.number();
              diag << "  FAILED QUALIFICATION - " << accTvlResult;
              diag << "   " << paxType.paxType() << std::endl;
            }

            //------------------------------------------------------
            // For MUST NOT, any FAIL will fail the whole
            // For MUST, FAIL will only disqualify this PaxType
            //------------------------------------------------------
            if (!isMustApply)
              return FAIL;

            continue;
          }

          if (UNLIKELY(accTvlInfo.accTvlSameRule() != notApply &&
              accTvlPASS !=
                  useSameRule(*myFareUsage.paxTypeFare(), *(*accFareUsageI)->paxTypeFare())))
            continue;

          // A common sense, Infant can not accompany anyone
          // unless this rule is for Infant accompanying passenger
          if (LIKELY(paxType.paxTypeInfo().infantInd() != tse::YES || accTvlInfo.accPsgType() == INFANT ||
              accTvlInfo.accPsgType() == paxType.paxType()))
          {
            numOfAccompanyingPsg += paxType.number();
            accPaxFareUsage.push_back((*accFareUsageI));
          }
        } // actualPaxType() != 0
      } // ((*accFareUsageI) != &myFareUsage)
    } // loop of FareUsages

    if (UNLIKELY(diag.isActive()))
    {
      diag << " NUM OF ACCOMPANIED PSG - " << numOfAccompaniedPsg;
      diag << "  TYPE - " << myFareUsage.paxTypeFare()->actualPaxType()->paxType() << std::endl;
    }

    //-------------------------------------------------------------
    if (UNLIKELY(numOfAccompaniedPsg > 1 && isMustApply &&
        accTvlPASS == qualifyAccPsgTypeFare(*myFareUsage.paxTypeFare()->actualPaxType(),
                                            true,
                                            *myFareUsage.paxTypeFare(),
                                            accTvlInfo,
                                            trx,
                                            &diag)))
    {
      // group accompanying travel
      numOfAccompanyingPsg += numOfAccompaniedPsg - 1;
      numOfAccompaniedPsg = 1;
    }

    // Get required minimum number of accompanying passengers
    //-------------------------------------------------------------
    uint32_t minNoAccPsg = minNoAccPsgReq(numOfAccompaniedPsg, accTvlInfo);

    //-------------------------------------------------------------
    // Check once now to catch possible validation failure earlier,
    // and again after accompanying passenger qualification
    //-------------------------------------------------------------
    if (UNLIKELY(numOfAccompanyingPsg < minNoAccPsg))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << DIAG_FAIL_MSG << failReasonNoAccPsg << std::endl;
        diag.flushMsg();
      }
      LOG4CXX_INFO(logger, LOG_FAIL_MSG);

      return FAIL;
    }
  } // fareClassBkgCdInd() != notApply
  else
  {
    // only get all the fareUsages that is not mine for same
    // sector, rule, compartment, cabin check
    for (; accFareUsageI != accFareUsageEndI; accFareUsageI++)
    {
      if ((*accFareUsageI) != &myFareUsage)
      {
        // A common sense, Infant can not accompany anyone
        // unless this rule is for Infant accompanying passenger
        const PaxType& paxType = *((*accFareUsageI)->paxTypeFare()->actualPaxType());
        if (paxType.paxTypeInfo().infantInd() != tse::YES || accTvlInfo.accPsgType() == INFANT)
        {
          accPaxFareUsage.push_back((*accFareUsageI));
        }
      }
    }
  }

  //----------------------------------------------------------
  // Next step, same travel segments/compartment requirement
  // Regardless MUST or MUST_NOT, these requirements must be
  // met by all accompanying passengers
  //----------------------------------------------------------
  accTvlResult = accTvlPASS; // reset the result

  accFareUsageI = accPaxFareUsage.begin();
  accFareUsageEndI = accPaxFareUsage.end();

  for (; accFareUsageI != accFareUsageEndI; accFareUsageI++)
  {
    const PaxTypeFare& myPaxTypeFare = *(myFareUsage.paxTypeFare());
    const PaxTypeFare& accPaxTypeFare = *((*accFareUsageI)->paxTypeFare());
    const PaxType& accPaxType = *(accPaxTypeFare.actualPaxType());

    //------------------------------------------------------------
    // Same Compartment requirment
    // Compartment is same for all sectors for a PaxTypeFare
    // We check here instead of within each air sectors.
    // APTCO requirement is ignored
    //------------------------------------------------------------
    if (LIKELY(accTvlInfo.accTvlSameCpmt() != notApply))
    {
      if (myPaxTypeFare.cabin() != accPaxTypeFare.cabin())
      {
        accTvlResult = failReasonSameCpmt;
        break;
      }
    }

    //------------------------------------------------------------------
    // Although ATPCO described how to validate with ALL, OUT and
    // ONE Sector in rule info, Sabre system currently does not
    // support accompanied and accompanying passengers only use
    // same out sectors.
    // Also neither GeoVia1 or GeoVia2 is supported.
    // However, seeing OUT always be notApply (BLANK) and GeoVia1,
    // GeoVia2 be 0 now, we keep the functions implemented here
    // to support them for future use when we need to check them.
    //------------------------------------------------------------------
    if (LIKELY(accTvlInfo.accTvlAllSectors() != notApply ||
        (accTvlInfo.accTvlOut() != notApply && myFareUsage.isOutbound()) ||
        accTvlInfo.accTvlOneSector() != notApply))
    {
      std::vector<AirSeg*> myAirSeg;
      std::vector<AirSeg*> accAirSeg;

      //-----------------------------------------
      // Get accompanying passenger's AirSeg
      // we use all segments within FareUsage
      //-----------------------------------------
      if (UNLIKELY((*accFareUsageI)->travelSeg().empty()))
      {
        accTvlResult = failReasonEmptySeg;
        break;
      }
      std::vector<TravelSeg*>::const_iterator accTvlSegI;
      accTvlSegI = (*accFareUsageI)->travelSeg().begin();
      for (; accTvlSegI != (*accFareUsageI)->travelSeg().end(); accTvlSegI++)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*accTvlSegI);
        if (airSeg != nullptr)
        {
          accAirSeg.push_back(airSeg);
        }
      }

      //--------------------------------------------------------------
      // When there is no valid geoTblItemNoVia1 or geoTblItemNoVia2,
      // we use all segments within FareUsage
      //--------------------------------------------------------------
      if (LIKELY(accTvlInfo.geoTblItemNoVia1() == 0 && accTvlInfo.geoTblItemNoVia2() == 0))
      {
        if (UNLIKELY(myFareUsage.travelSeg().empty()))
        {
          accTvlResult = failReasonEmptySeg;
          break;
        }
        std::vector<TravelSeg*>::const_iterator myTvlSegI;
        myTvlSegI = myFareUsage.travelSeg().begin();
        for (; myTvlSegI != myFareUsage.travelSeg().end(); myTvlSegI++)
        {
          AirSeg* airSeg = dynamic_cast<AirSeg*>(*myTvlSegI);
          if (airSeg != nullptr)
          {
            myAirSeg.push_back(airSeg);
          }
        }
        if (validateSameSegReq(myAirSeg, accAirSeg, accTvlInfo, accTvlResult) == FAIL)
        {
          // no matter APPLY_MUST or MUST_NOT, see this as failure;
          // accTvlResult should have been set a failReason already,
          // otherwise we set a default failReason
          if (accTvlResult == accTvlPASS)
          {
            accTvlResult = failReasonSameSeg;
          }
          break;
        }
      } // Via1 == 0 && Via2 == 0
#if 0
            // Same as Phase 1, we decided not to support GeoItemTbl now
            // We will let validation pass
            else
            {
                std::vector<TravelSeg*> myGeoTvlSeg;

                //----------------------------------------------------
                // get my(accompanied) travel segments according to
                // VIA1 and VIA2, and do validation
                //----------------------------------------------------
                bool fltStopCheck = false;
                TSICode tsiReturn;
                LocKey locKey1Return;
                LocKey locKey2Return;
                bool origCheck = true;
                bool destCheck = false;
                RuleConst::TSIScopeParamType scopeParam = RuleConst::TSI_SCOPE_PARAM_JOURNEY;  // Default is for JOURNEY based rule, but we do have information on FareComponent as well
                DateTime tktDT = TrxUtil::getTicketingDT(trx);

                // @TODO get TSI, if it is for SUB_JOURNEY, loop PricingUnit
                // vector of myFarePath, find the PricingUnit that contains
                // myFareUsage

                if (accTvlInfo.geoTblItemNoVia1() != 0)
                {
                    if (!RuleUtil::validateGeoRuleItem(accTvlInfo.geoTblItemNoVia1(),
                                           myPaxTypeFare.vendor(),
                                           scopeParam,
                                           trx,
                                           &farePath,
                                           0,        /* pricing unit */
                                           myPaxTypeFare.fareMarket(),
                                           tktDT,
                                           myGeoTvlSeg, // this will contain the results
                                           origCheck,
                                           destCheck,
                                           fltStopCheck,
                                           tsiReturn,
                                           locKey1Return,
                                           locKey2Return,
                                           Diagnostic313))
                    {
                        accTvlResult = failReasonGeoVia1;
                        break;
                    }
                    // Tried to dynamic_cast to AirSeg Vectors
                    std::vector<TravelSeg*>::const_iterator myGeoTvlSegI;
                    myGeoTvlSegI = myGeoTvlSeg.begin();
                    for (;  myGeoTvlSegI != myGeoTvlSeg.end(); myGeoTvlSegI++)
                    {
                        AirSeg* airSeg = dynamic_cast<AirSeg*>(*myGeoTvlSegI);
                        if (airSeg != 0)
                        {
                            myAirSeg.push_back(airSeg);
                        }
                    }

                    if (validateSameSegReq(myAirSeg, accAirSeg, accTvlInfo, accTvlResult) == FAIL)
                    {
                        // no matter APPLY_MUST or MUST_NOT, see this as failure
                        // accTvlResult should have been set failReason already
                        if (accTvlResult == accTvlPASS)
                        {
                            accTvlResult = failReasonSameSeg;
                        }
                        break;
                    }
                }
                if (accTvlInfo.geoTblItemNoVia2() != 0)
                {
                    if (!RuleUtil::validateGeoRuleItem(accTvlInfo.geoTblItemNoVia2(),
                                           myPaxTypeFare.vendor(),
                                           scopeParam,
                                           trx,
                                           &farePath,
                                           0,        /* pricing unit */
                                           myPaxTypeFare.fareMarket(),
                                           tktDT,
                                           myGeoTvlSeg, // this will contain the results
                                           origCheck,
                                           destCheck,
                                           fltStopCheck,
                                           tsiReturn,
                                           locKey1Return,
                                           locKey2Return,
                                           Diagnostic313))
                    {
                        accTvlResult = failReasonGeoVia2;
                        break;
                    }
                    // Tried to dynamic_cast to AirSeg Vectors
                    myAirSeg.clear();
                    std::vector<TravelSeg*>::const_iterator myGeoTvlSegI;
                    myGeoTvlSegI = myGeoTvlSeg.begin();
                    for (;  myGeoTvlSegI != myGeoTvlSeg.end(); myGeoTvlSegI++)
                    {
                        AirSeg* airSeg = dynamic_cast<AirSeg*>(*myGeoTvlSegI);
                        if (airSeg != 0)
                        {
                            myAirSeg.push_back(airSeg);
                        }
                    }
                    if (validateSameSegReq(myAirSeg, accAirSeg, accTvlInfo, accTvlResult) == FAIL)
                    {
                        // no matter APPLY_MUST or MUST_NOT, see this as failure
                        // accTvlResult should have been set failReason already
                        if (accTvlResult == accTvlPASS)
                        {
                            accTvlResult = failReasonSameSeg;
                        }
                        break;
                    }
                }
            }
#endif // do not support GeoTblItem for now
    } // accTvlAllSectors or accTvlOut or accTvlOneSector
    // or accTvlSameRule != notApply

    //-----------------------------------------------------------
    // OK, accompanying passenger(s) of this FareUsage qualified
    //-----------------------------------------------------------
    if (UNLIKELY(diag.isActive()))
    {
      diag << "  " << accPaxType.number();
      diag << "  SUCCEEDED QUALIFICATION - ";
      diag << accPaxType.paxType() << std::endl;
    }
  }

  //---------------------------------------------------------------
  //---------------------------------------------------------------
  if (accTvlResult != accTvlPASS)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << DIAG_FAIL_MSG << accTvlResult << std::endl;
      diag.flushMsg();
    }
    LOG4CXX_INFO(logger, LOG_FAIL_MSG);

    return FAIL;
  }

  LOG4CXX_INFO(logger, " Leaving AccompaniedTravel::validate() - PASS");
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << " ACCOMPANIED TRAVEL: PASS " << std::endl;
    diag.flushMsg();
  }

  // for WQ (NoPNR) transaction, add warning message
  if (UNLIKELY(dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr))
  {
    myFareUsage.paxTypeFare()->fare()->warningMap().set(WarningMap::cat13_warning);
  }
  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validate()
//
// validate()   Performs AccompaniedTravel rule validation for DiscountRule
//              (Cat19-22) during Fare creation;
//              At this stage, only PaxTypeCode and Age of all passengers are
//              available for validation. It needs to meet the number of
//              accompanied passenger against number of accompanying passenger
//              requirement.
//
//  @param PricingTrx&            - Pricing transaction
//  @param PaxType&               - reference to PaxType for discount
//  @param DiscountInfo&          - Discount Rule Item Info
//  @param PaxTypeFare&           - For diagnostic purpose
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
AccompaniedTravel::validate(PricingTrx& trx,
                            const PaxType& myPaxType,
                            const DiscountInfo& discountInfo,
                            PaxTypeFare& paxTypeFare)
{ // lint !e578

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;

  if (trx.diagnostic().shouldDisplay(
          paxTypeFare, RuleConst::CHILDREN_DISCOUNT_RULE, myPaxType.paxType()))
  {
    diag.enable(Diagnostic319, Diagnostic863);
  }

  if (diag.isActive())
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic863)
    {
      diag << " \n \n WPA ACCOMPANIED TRAVEL RESTRICTIONS - CAT19-22\n";
      diag.printLine();

      if (paxTypeFare.fareMarket())
      {
        diag << (*paxTypeFare.fareMarket());
      }
      diag << paxTypeFare;
    }
    displayDiscountRuleToDiag(discountInfo, diag);
  }

  //---------------------------------------------------------------
  // Diagnosis the PaxType to qualify for discount
  //---------------------------------------------------------------
  const PaxTypeCode& myPaxTypeCode = myPaxType.paxType();
  if (diag.isActive())
  {
    diag << " ACCOMPANIED PSG TYPE - " << myPaxTypeCode << std::endl;
  }

  //---------------------------------------------------------------
  // If there is no Discount Seg, apply system assumption
  // that there is no restriction, return true
  //---------------------------------------------------------------
  if (discountInfo.segs().empty())
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL : PASS - NO DISCOUNT RESTRICT" << std::endl;
      diag.flushMsg();
    }
    return PASS;
  }

  const bool isNoPNRTransaction = (dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr);

  //------------------------------------------------------
  // Initialize validation result as PASS
  //------------------------------------------------------
  ACCTVL_VALID_RESULT accTvlResult = accTvlPASS;
  BaseExchangeTrx* excTrx = dynamic_cast<BaseExchangeTrx*>(&trx);
  const bool chkExcAccPaxType = (excTrx && !excTrx->accompanyPaxType().empty());

  //----------------------------------------------------------------
  // We will check if there is enough accompanying passenger
  // We can find all PaxType information in PricingTrx
  //----------------------------------------------------------------
  if (UNLIKELY((discountInfo.category() == RuleConst::CHILDREN_DISCOUNT_RULE) &&
      (trx.paxType().size() < 2) && !chkExcAccPaxType))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL : PRICED ALONE - PASS WITH WARNING\n";
      diag.flushMsg();
    }

    paxTypeFare.setAccTvlWarning(true);
    // for WQ (NoPNR) transaction, add warning message
    if (isNoPNRTransaction)
    {
      paxTypeFare.fare()->warningMap().set(WarningMap::cat19_22_warning);
    }
    return PASS;
  }

  const CarrierCode& carrier = paxTypeFare.carrier();

  // For WPA, we always need AccSameFareBreak so that we would have
  // accompanied travel restriction information stored
  if (UNLIKELY(trx.altTrxType() != PricingTrx::WP))
  {
    paxTypeFare.setAccSameFareBreak(true);
  }

  uint16_t qualifiedPaxTypeCount = 0; // we would not check the number of
  // accompanied against number of accompanying if there
  // is more than one PaxType qualified, because we
  // do not know who is using the fare at final stage.
  // such as INF may not use CNN fare in the end.

  // Depends on accompanying passenger MUST or MUST_NOT apply, we treat
  // the matching result differently
  const bool isMustApply = (discountInfo.accPsgAppl() == applyMust);

  std::vector<PaxType*> tmpPaxTypes;
  if (UNLIKELY(chkExcAccPaxType))
  {
    tmpPaxTypes.insert(tmpPaxTypes.end(), trx.paxType().begin(), trx.paxType().end());
    tmpPaxTypes.insert(
        tmpPaxTypes.end(), excTrx->accompanyPaxType().begin(), excTrx->accompanyPaxType().end());
  }

  //-----------------------------------------------------------------------
  // If any of the Discount Seg match, we PASS
  // In other words, it is OR relationship for qualifying discount:
  // e.g. there are two discount segments,
  //   "Discount Seg 1 PASS" OR "Discount Seg 2 PASS" THEN PASS
  //-----------------------------------------------------------------------
  std::vector<DiscountSegInfo*>::const_iterator discSegI;

  discSegI = discountInfo.segs().begin();
  uint16_t segIndex = 0;
  for (; discSegI != discountInfo.segs().end(); discSegI++, segIndex++)
  {
    const DiscountSegInfo& discSegInfo = *(*discSegI);

    //------------------------------------------------------------------
    // Tried to qualify by type, age and number of passengers
    // which we can find the information from PaxType
    //------------------------------------------------------------------

    uint32_t numOfAccompaniedPsg = 0;
    uint32_t numOfAccompanyingPsg = 0;

    uint32_t numOfType1AccompanyingPsg = 0;
    uint32_t numOfType2AccompanyingPsg = 0;
    uint32_t numOfType3AccompanyingPsg = 0;
    const bool needCheckType1AccPsg = !discSegInfo.accPsgType1().empty();
    const bool needCheckType2AccPsg = !discSegInfo.accPsgType2().empty();
    const bool needCheckType3AccPsg = !discSegInfo.accPsgType3().empty();

    std::vector<PaxType*>::const_iterator paxTypeI = trx.paxType().begin();
    std::vector<PaxType*>::const_iterator paxTypeIEnd = trx.paxType().end();

    if (UNLIKELY(chkExcAccPaxType))
    {
      paxTypeI = tmpPaxTypes.begin();
      paxTypeIEnd = tmpPaxTypes.end();
    }

    for (; paxTypeI != paxTypeIEnd; paxTypeI++)

    {
      if (UNLIKELY(*paxTypeI == nullptr))
        continue;

      // During discounted fare creation, INF can use CNN fare
      // Because we do not want INF using CNN fare causing the failure
      // on accompanied psg count, accompanying psg count, we would not
      // do the account check
      const PaxType& paxType = *(*paxTypeI);

      if (myPaxTypeCode == CHILD && paxType.paxTypeInfo().childInd() == tse::YES)
      {
        qualifiedPaxTypeCount++;
        numOfAccompaniedPsg += paxType.number();
        continue;
      }

      if (PaxTypeUtil::isAnActualPax(
              paxType, carrier, myPaxTypeCode, discountInfo.minAge(), discountInfo.maxAge()))
      {
        qualifiedPaxTypeCount++;
        numOfAccompaniedPsg += paxType.number();
        continue;
      }

      // Only INFANT does not count, everybody else may count
      if (paxType.paxTypeInfo().infantInd() != tse::YES)
      {
        numOfAccompanyingPsg += paxType.number();
      }

      if (needCheckType1AccPsg &&
          isMustApply == PaxTypeUtil::isAnActualPax(paxType,
                                                    carrier,
                                                    discSegInfo.accPsgType1(),
                                                    discSegInfo.minAge1(),
                                                    discSegInfo.maxAge1()))
      {
        numOfType1AccompanyingPsg += paxType.number();
        if (UNLIKELY(diag.isActive()))
        {
          diag << " " << paxType.number();
          diag << " ACC PSG QUALIFIED WITH TYPE - " << paxType.paxType() << std::endl;
        }
      }
      else if (needCheckType2AccPsg &&
               isMustApply == PaxTypeUtil::isAnActualPax(paxType,
                                                         carrier,
                                                         discSegInfo.accPsgType2(),
                                                         discSegInfo.minAge2(),
                                                         discSegInfo.maxAge2()))
      {
        numOfType2AccompanyingPsg += paxType.number();
        if (UNLIKELY(diag.isActive()))
        {
          diag << " " << paxType.number();
          diag << " ACC PSG QUALIFIED WITH TYPE - " << paxType.paxType() << std::endl;
        }
      }
      else if (needCheckType3AccPsg &&
               isMustApply == PaxTypeUtil::isAnActualPax(paxType,
                                                         carrier,
                                                         discSegInfo.accPsgType3(),
                                                         discSegInfo.minAge3(),
                                                         discSegInfo.maxAge3()))
      {
        numOfType3AccompanyingPsg += paxType.number();
        if (UNLIKELY(diag.isActive()))
        {
          diag << " " << paxType.number();
          diag << " ACC PSG QUALIFIED WITH TYPE - " << paxType.paxType() << std::endl;
        }
      }
    } // loop of trx.PaxType

    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED PSG NUM - " << numOfAccompaniedPsg << std::endl;
    }

    if (UNLIKELY(numOfAccompaniedPsg == 0))
    {
      //-----------------------------------------------------
      // Could not see the paxType of PaxTypeFare in trx
      // We do not qualify this
      //-----------------------------------------------------
      if (UNLIKELY(diag.isActive()))
      {
        diag << " DISCOUNT FARE ACCOMPANIED TRAVEL: FAIL - " << failReasonPaxTypeNoMatch
             << std::endl;
        diag.flushMsg();
      }

      return FAIL;
    }

    //---------------------------------------------------------------------
    // APTCO Requires,
    // MUST: Only all PaxType in discSegInfo are found, we declaim PASS
    // MUST_NOT: Only all PaxType in discSegInfo are found, we declaim FAIL
    //---------------------------------------------------------------------
    if (UNLIKELY(!isMustApply))
    {
      if ((!needCheckType1AccPsg || numOfType1AccompanyingPsg == 0) &&
          (!needCheckType2AccPsg || numOfType2AccompanyingPsg == 0) &&
          (!needCheckType3AccPsg || numOfType3AccompanyingPsg == 0))
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " CONDITION " << segIndex;
          diag << ": FAIL - " << failReasonMUSTNOT << std::endl;
        }
        continue;
      }
      // Now every one of numberOfAccompanyingPsg is good to be count
      // against number of passenger requirement
    }
    else
    {
      if ((needCheckType1AccPsg && numOfType1AccompanyingPsg == 0) ||
          (needCheckType2AccPsg && numOfType2AccompanyingPsg == 0) ||
          (needCheckType3AccPsg && numOfType3AccompanyingPsg == 0))
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " CONDITION " << segIndex;
          diag << ": FAIL - " << failReasonPsgType << std::endl;
        }
        continue;
      }

      if (needCheckType1AccPsg || needCheckType2AccPsg || needCheckType3AccPsg)
      {
        // For MUST, we add every qualified types' passenger together
        numOfAccompanyingPsg =
            numOfType1AccompanyingPsg + numOfType2AccompanyingPsg + numOfType3AccompanyingPsg;
      }
    }

    if (diag.isActive())
    {
      diag << " TOTAL " << numOfAccompanyingPsg;
      diag << " ACC PSG SUCCEEDED QUALIFICATION" << std::endl;
    }

    //------------------------------------------------------------------
    // Compare qualified accompanying passenger with min/max number of
    // accompanying passenger required
    // If both min/max number is 0, as long as number of accompanying psg
    // is not 0, let pass (different with cat13 min/max number
    //------------------------------------------------------------------
    // According to BA: we will skip validation/process of the CAT 19
    //  Accompanying MIN (MINNOPSG) and MAX (MAXNOPSG) fields.
    //  const bool needPsgCountChk = (qualifiedPaxTypeCount == 1);
    //  if ( numOfAccompanyingPsg == 0
    //          ||
    //      (needPsgCountChk && (discSegInfo.minNoPsg() > 0) &&
    //       numOfAccompanyingPsg < numOfAccompaniedPsg * discSegInfo.minNoPsg())
    //          ||
    //      (needPsgCountChk && (discSegInfo.maxNoPsg() > 0) &&
    //      numOfAccompanyingPsg > numOfAccompaniedPsg * discSegInfo.maxNoPsg())
    //    )
    //  {
    //      accTvlResult = failReasonNoAccPsg;
    //      if (diag.isActive())
    //      {
    //        diag << " CONDITION " << segIndex << ": FAIL - " << accTvlResult << std::endl;
    //      }
    //    continue;
    //  }

    if (UNLIKELY(diag.isActive()))
    {
      diag << " CONDITION " << segIndex << " SKIPPED MIN NO/MAX NO CHECK " << accTvlResult
           << std::endl;
    }

    // We just past this DiscountSeg, since this is OR relationship
    // we are good to return
    if (diag.isActive())
    {
      diag << " CONDITION " << segIndex << ": PASS" << std::endl;
    }
    break; // break the Loop of DiscountSegInfo vector
  } // Loop of DiscountSegInfo vector

  if (discSegI == discountInfo.segs().end())
  {
    // No discount seg matched
    //
    // Part of allowing pricing seperately, if there is not ADT accompanying
    // we can pass this with warning
    const PaxType* ptItem = PaxTypeUtil::isAdultInTrx(trx);

    if (ptItem == nullptr)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " ACCOMPANIED TRAVEL : NO ADULT - PASS WITH WARNING\n";
        diag.flushMsg();
      }
      paxTypeFare.setAccTvlWarning(true);
      // for WQ (NoPNR) transaction, add warning message
      if (isNoPNRTransaction)
      {
        paxTypeFare.fare()->warningMap().set(WarningMap::cat19_22_warning);
      }
      return PASS;
    }

    if (UNLIKELY(diag.isActive()))
    {
      diag << " DISCOUNT FARE ACCOMPANIED TRAVEL: FAIL" << std::endl;
      diag.flushMsg();
    }
    return FAIL;
  }

  // Here we can not yet, but when PaxTypeFare is build, we
  // need to set flag to require re-validation during pricing,
  // for Fare class/Booking code, same sector/compartment/rule requirement
  // We will put down that in Diagnostic output

  if (UNLIKELY(diag.isActive()))
  {
    if (needCombinabiltyCheck(discountInfo))
    {
      diag << " DISCOUNT FARE ACCOMPANIED TRAVEL: PASS W/ COMBINABILITY REQ" << std::endl;
    }
    else
    {
      diag << " DISCOUNT FARE ACCOMPANIED TRAVEL: PASS" << std::endl;
    }

    diag.flushMsg();
  }

  // for WQ (NoPNR) transaction, add warning message
  if (UNLIKELY(isNoPNRTransaction))
  {
    paxTypeFare.fare()->warningMap().set(WarningMap::cat19_22_warning);
  }
  return PASS;
}
Record3ReturnTypes
AccompaniedTravel::validate(const DiscountInfo& discountInfo, PaxTypeFare& ptFare)
{
  if (discountInfo.segs().empty())
    return PASS;

  std::set<PaxTypeCode> validPaxTypes;
  if (ptFare.actualPaxType() != nullptr)
    validPaxTypes.insert(ptFare.actualPaxType()->paxType());
  else
    validPaxTypes.insert(ADULT);

  if (ptFare.isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPTFare = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPTFare && fbrPTFare->fbrApp()->segCnt())
    {
      const std::vector<PaxTypeCode>& secPaxTypesVec = fbrPTFare->fbrApp()->secondaryPaxTypes();
      for (const auto paxTypeCode : secPaxTypesVec)
        validPaxTypes.insert(paxTypeCode);
    }
  }

  std::set<PaxTypeCode>::const_iterator itB = validPaxTypes.begin();
  std::set<PaxTypeCode>::const_iterator itE = validPaxTypes.end();

  for (; itB != itE; itB++)
  {
    std::vector<DiscountSegInfo*>::const_iterator discSegI = discountInfo.segs().begin();
    std::vector<DiscountSegInfo*>::const_iterator discSegE = discountInfo.segs().end();
    for (; discSegI != discSegE; discSegI++)
    {
      const DiscountSegInfo& discSegInfo = *(*discSegI);
      if (discSegInfo.accPsgType1().empty() && discSegInfo.accPsgType2().empty() &&
          discSegInfo.accPsgType3().empty())
      {
        return PASS;
      }
      if (!discSegInfo.accPsgType1().empty())
      {
        if (discSegInfo.accPsgType1().equalToConst("ADT")) // No restriction on the passenger type
        {
          if (PaxTypeUtil::isAdult(*itB, discountInfo.vendor()))
            return PASS;
        }
        else if (discSegInfo.accPsgType1() == *itB)
          return PASS;
      }
      if (!discSegInfo.accPsgType2().empty())
      {
        if (discSegInfo.accPsgType2().equalToConst("ADT")) // No restriction on the passenger type
        {
          if (PaxTypeUtil::isAdult(*itB, discountInfo.vendor()))
            return PASS;
        }
        else if (discSegInfo.accPsgType2() == *itB)
          return PASS;
      }
      if (!discSegInfo.accPsgType3().empty())
      {
        if (discSegInfo.accPsgType3().equalToConst("ADT")) // No restriction on the passenger type
        {
          if (PaxTypeUtil::isAdult(*itB, discountInfo.vendor()))
            return PASS;
        }
        else if (discSegInfo.accPsgType3() == *itB)
          return PASS;
      }
    } // discSegI
  } // itB
  return FAIL;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validate()
//
// validate()   Performs accompanied restriction validation for DiscountRule
//              (Cat19-22) on passenger type, age, FareClass/BookingCode and
//              qualified numbers when FareClass or BookingCode of all
//              passengers are available. (During Combinability validation)
//
//  @param PricingTrx&                - Pricing transaction
//  @param PaxTypeFare&               - reference to Pax Type Fare
//  @param std::vector<PaxTypeFare*>& - all Pax Type Fare together
//  @param DiscountInfo&              - Discount Rule Item Info
//
//  @return boolean - possible values are:
//                        true    validation succeeded
//                        false   validation failed
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
AccompaniedTravel::validate(PricingTrx& trx,
                            const PaxTypeFare& myPaxTypeFare,
                            const std::vector<PaxTypeFare*>& paxTypeFares,
                            const DiscountInfo& discountInfo)
{ // lint !e578
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic313);
  if (diag.isActive())
  {
    displayDiscountRuleToDiag(discountInfo, diag);
  }

  //---------------------------------------------------------------
  // Diagnosis the PaxType to qualify for discount
  //---------------------------------------------------------------
  const PaxTypeCode& myPaxTypeCode = myPaxTypeFare.actualPaxType()->paxType();
  if (diag.isActive())
  {
    diag << " ACCOMPANIED PSG TYPE - " << myPaxTypeCode << std::endl;
  }

  //---------------------------------------------------------------
  // If there is no Discount Seg, apply system assumption
  // that there is no restriction, return true
  //---------------------------------------------------------------
  if (discountInfo.segs().empty())
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL : PASS - NO DISCOUNT RESTRICT" << std::endl;
      diag.flushMsg();
    }
    return PASS;
  }

  //------------------------------------------------------
  // Initialize validation result as PASS
  //------------------------------------------------------
  ACCTVL_VALID_RESULT accTvlResult = accTvlPASS;

  //----------------------------------------------------------------
  // We will check if there is enough accompanying passenger
  // We can find all PaxType information in PricingTrx
  //----------------------------------------------------------------
  if (trx.paxType().size() < 2)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " ACCOMPANIED TRAVEL : FAIL - " << failReasonNoAccPsg << std::endl;
      diag.flushMsg();
    }

    return FAIL;
  }

  // Depends on accompanying passenger MUST or MUST_NOT apply, we treat
  // the matching result differently
  const bool isMustApply = (discountInfo.accPsgAppl() == applyMust);

  //-----------------------------------------------------------------------
  // If anyone of the Discount Segs matches, we PASS
  // In another word, it is OR relationship:
  // e.g. there are two discount segments,
  //   "Discount Seg 1 PASS" OR "Discount Seg 2 PASS" THEN PASS
  //----------------------------------------------------------------------

  std::vector<DiscountSegInfo*>::const_iterator discSegI = discountInfo.segs().begin();
  std::vector<DiscountSegInfo*>::const_iterator discSegEndI = discountInfo.segs().end();

  uint16_t segIndex = 0;
  for (; discSegI != discSegEndI; discSegI++, segIndex++)
  {
    const DiscountSegInfo& discSegInfo = *(*discSegI);

    //------------------------------------------------------------------
    // Tried to qualify by type, age and number of passengers
    // which we can find the information from PaxType, plus FareClass or
    // BookingCode which we can find the information from PaxTypeFare
    //------------------------------------------------------------------

    uint32_t numOfAccompaniedPsg = 0;
    uint32_t numOfAccompanyingPsg = 0;

    uint32_t numOfType1AccompanyingPsg = 0;
    uint32_t numOfType2AccompanyingPsg = 0;
    uint32_t numOfType3AccompanyingPsg = 0;
    const bool needCheckType1AccPsg = !discSegInfo.accPsgType1().empty();
    const bool needCheckType2AccPsg = !discSegInfo.accPsgType2().empty();
    const bool needCheckType3AccPsg = !discSegInfo.accPsgType3().empty();

    std::vector<PaxTypeFare*>::const_iterator paxTypeFareI;

    paxTypeFareI = paxTypeFares.begin();

    for (; paxTypeFareI != paxTypeFares.end(); paxTypeFareI++)
    {
      if (*paxTypeFareI == nullptr || (*paxTypeFareI)->actualPaxType() == nullptr)
        continue;

      // lint -e{578}
      const PaxType& paxType = *((*paxTypeFareI)->actualPaxType());

      if (qualifyPsgTypeAge(paxType, discountInfo.minAge(), discountInfo.maxAge(), myPaxTypeCode))
      {
        numOfAccompaniedPsg += paxType.number();
        continue;
      }

      //-------------------------------------------------------
      // Check Booking Code or Fare Class
      //-------------------------------------------------------
      if (discountInfo.fareClassBkgCodeInd() == fareClass)
      {
        if (!RuleUtil::matchFareClass(discSegInfo.fareClass().c_str(),
                                      (*paxTypeFareI)->fare()->fareClass().c_str()))
        {
          continue;
        }
      }
      else if (discountInfo.fareClassBkgCodeInd() == bookingCode)
      {
        std::vector<BookingCode> accBkgCodeVec;

        if (!(*paxTypeFareI)->getPrimeBookingCode(accBkgCodeVec))
          continue; // seen as not match

        //--------------------------------------------------------------
        // If any of accompanying passenger bookingcode have a match
        // in the discSegInfo.bookingCode(), we will see as match
        //--------------------------------------------------------------
        std::vector<BookingCode>::const_iterator accBkgCodeI;

        accBkgCodeI = accBkgCodeVec.begin();

        for (; accBkgCodeI != accBkgCodeVec.end(); accBkgCodeI++)
        {
          if (*accBkgCodeI == discSegInfo.bookingCode())
          {
            break;
          }
        }
        if (accBkgCodeI == accBkgCodeVec.end())
        {
          // no match
          continue;
        }
      }

      // Only INFANT does not count, everybody else may count
      if (paxType.paxTypeInfo().infantInd() != tse::YES)
      {
        numOfAccompanyingPsg += paxType.number();
      }

      const CarrierCode& carrier = (*paxTypeFareI)->carrier();
      if (needCheckType1AccPsg &&
          isMustApply == PaxTypeUtil::isAnActualPax(paxType,
                                                    carrier,
                                                    discSegInfo.accPsgType1(),
                                                    discSegInfo.minAge1(),
                                                    discSegInfo.maxAge1()))
      {
        numOfType1AccompanyingPsg += paxType.number();
      }
      else if (needCheckType2AccPsg &&
               isMustApply == PaxTypeUtil::isAnActualPax(paxType,
                                                         carrier,
                                                         discSegInfo.accPsgType2(),
                                                         discSegInfo.minAge2(),
                                                         discSegInfo.maxAge2()))
      {
        numOfType2AccompanyingPsg += paxType.number();
      }
      else if (needCheckType3AccPsg &&
               isMustApply == PaxTypeUtil::isAnActualPax(paxType,
                                                         carrier,
                                                         discSegInfo.accPsgType3(),
                                                         discSegInfo.minAge3(),
                                                         discSegInfo.maxAge3()))
      {
        numOfType3AccompanyingPsg += paxType.number();
      }
    } // loop of PaxTypeFare vector

    if (numOfAccompaniedPsg == 0)
    {
      //-----------------------------------------------------
      // Could not see the paxType of PaxTypeFare in trx
      // We do not qualify this
      //-----------------------------------------------------
      if (UNLIKELY(diag.isActive()))
      {
        diag << " DISCOUNT FARE ACCOMPANIED TRAVEL: FAIL - " << failReasonPaxTypeNoMatch
             << std::endl;
        diag.flushMsg();
      }

      return FAIL;
    }

    //-------------------------------------------------------------
    // APTCO Requires,
    // MUST: Only all PaxType in discSegInfo are found, we declaim PASS
    // MUST_NOT: Only all PaxType in discSegInfo are found, we declaim FAIL
    //-------------------------------------------------------------
    if (discountInfo.accPsgAppl() != applyMust)
    {
      if ((!needCheckType1AccPsg || numOfType1AccompanyingPsg == 0) &&
          (!needCheckType2AccPsg || numOfType2AccompanyingPsg == 0) &&
          (!needCheckType3AccPsg || numOfType3AccompanyingPsg == 0))
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " CONDITION " << segIndex << ": FAIL - " << failReasonMUSTNOT << std::endl;
        }
        continue;
      }
      // Now every one of numberOfAccompanyingPsg is good to be count
      // against number of passenger requirement
    }
    else
    {
      if ((needCheckType1AccPsg && numOfType1AccompanyingPsg == 0) ||
          (needCheckType2AccPsg && numOfType2AccompanyingPsg == 0) ||
          (needCheckType3AccPsg && numOfType3AccompanyingPsg == 0))
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " CONDITION " << segIndex << ": FAIL - " << failReasonPsgType << std::endl;
        }
        continue;
      }
      // For MUST, we add every qualified types' passenger together
      // All PaxType could be empty, i.e. cares about FareClass only
      if (needCheckType1AccPsg || needCheckType2AccPsg || needCheckType3AccPsg)
        numOfAccompanyingPsg =
            numOfType1AccompanyingPsg + numOfType2AccompanyingPsg + numOfType3AccompanyingPsg;
    }

    if (UNLIKELY(diag.isActive()))
    {
      diag << "  " << numOfAccompanyingPsg;
      diag << "  ACC PSG SUCCEEDED QUALIFICATION" << std::endl;
    }

    //-------------------------------------------------------------
    // Compare qualified accompanying passenger with min/max number of
    // accompanying passenger required
    // If both min/max number is 0, as long as number of accompanying psg
    // is not 0, let pass (different with cat13 min/max number)
    //-------------------------------------------------------------
    if (numOfAccompanyingPsg == 0 ||
        (discSegInfo.minNoPsg() > 0 &&
         numOfAccompanyingPsg < numOfAccompaniedPsg * discSegInfo.minNoPsg()) ||
        (discSegInfo.maxNoPsg() > 0 &&
         numOfAccompanyingPsg > numOfAccompaniedPsg * discSegInfo.maxNoPsg()))
    {
      accTvlResult = failReasonNoAccPsg;
      if (UNLIKELY(diag.isActive()))
      {
        diag << " CONDITION " << segIndex << ": FAIL - " << accTvlResult << std::endl;
      }
      continue;
    }

    // We just past this DiscountSeg, since this is OR relationship
    // we are good to return
    break; // break the Loop of DiscountSegInfo vector
  } // Loop of DiscountSegInfo vector

  if (discSegI == discountInfo.segs().end())
  {
    // No discount seg matched
    if (UNLIKELY(diag.isActive()))
    {
      diag << " DISCOUNT FARE ACCOMPANIED TRAVEL: FAIL" << std::endl;
      diag.flushMsg();
    }
    return FAIL;
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << " ACCOMPANIED TRAVEL: PASS" << std::endl;
    diag.flushMsg();
  }

  // for WQ (NoPNR) transaction, add warning message
  if (dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr)
  {
    myPaxTypeFare.fare()->warningMap().set(WarningMap::cat19_22_warning);
  }
  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::checkFareClassBkgCode()
//
// checkFareClassOrBookingCode()  check if passenger's Fare Class or
//                                Booking Code matches definition on
//                                Cat13 Accompanied Travel rule info
//
//  @param PaxTypeFare&             - PaxTypeFare of accompanying passenger
//  @param AccompaniedTravelInfo&   - Accompanied Travel Rule Item Info
//
//  @return boolean - possible values are:
//                   true         matched
//                   false        did not match
//
// </PRE>
//-------------------------------------------------------------------
bool
AccompaniedTravel::checkFareClassBkgCode(const PaxTypeFare& paxTypeFare,
                                         const AccompaniedTravelInfo& accTvlInfo,
                                         PricingTrx& trx,
                                         DiagCollector* diag /*=0*/) const
{ // lint !e578

  if (accTvlInfo.fareClassBkgCds().empty() || accTvlInfo.fareClassBkgCdInd() == ' ')
    return true;

  if (accTvlInfo.fareClassBkgCdInd() == fareClass)
  {
    if (paxTypeFare.fare() == nullptr)
      return false;

    std::vector<FareClassCode>::const_iterator fareClassI;
    fareClassI = accTvlInfo.fareClassBkgCds().begin();

    for (; fareClassI != accTvlInfo.fareClassBkgCds().end(); fareClassI++)
    {
      if (RuleUtil::matchFareClass(fareClassI->c_str(), paxTypeFare.fare()->fareClass().c_str()))
        return true;
    }

    if (UNLIKELY(diag))
    {
      (*diag) << " ACCOMPANYING PSG FARE CLASS - " << paxTypeFare.fare()->fareClass() << "\n";
    }

    return false; // did not find match
  }
  else
  {
    // Booking Code requirement
    std::vector<BookingCode> accBkgCodeVec;

    if (trx.getRequest()->isLowFareRequested()) // WPNC entry
    {
      const TravelSeg* primarySec = paxTypeFare.fareMarket()->primarySector();
      std::vector<TravelSeg*>::const_iterator tvlSegIter =
          paxTypeFare.fareMarket()->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tvlSegIterE =
          paxTypeFare.fareMarket()->travelSeg().end();

      BookingCode bc;
      uint16_t indx = 0;
      bool match = false;

      const PaxTypeFare::SegmentStatusVec& statusSVec = paxTypeFare.segmentStatus();
      if (statusSVec.empty())
      {
        // not initialized yet, let it pass
        return true;
      }

      for (; tvlSegIter != tvlSegIterE; ++tvlSegIter, ++indx)
      {
        if (*tvlSegIter == primarySec)
        {
          if (statusSVec[indx]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
            bc = statusSVec[indx]._bkgCodeReBook;
          else
            bc = primarySec->getBookingCode();

          match = true;
          break;
        }
      }
      if (!match)
        return false;

      const std::vector<FareClassCode>& ruleBCVec = accTvlInfo.fareClassBkgCds();
      std::vector<FareClassCode>::const_iterator ruleBCI = ruleBCVec.begin();
      std::vector<FareClassCode>::const_iterator ruleBCE = ruleBCVec.end();

      for (; ruleBCI != ruleBCE; ++ruleBCI)
      {
        if ((*ruleBCI)[0] == bc[0])
        {
          return true;
        }
      }

      return false;
    }
    else // do regular Booking code validation
    {
      const TravelSeg* primarySec = paxTypeFare.fareMarket()->primarySector();
      if (primarySec)
      {
        BookingCode bc = primarySec->getBookingCode();

        const std::vector<FareClassCode>& ruleBCVec = accTvlInfo.fareClassBkgCds();
        std::vector<FareClassCode>::const_iterator ruleBCI = ruleBCVec.begin();
        std::vector<FareClassCode>::const_iterator ruleBCE = ruleBCVec.end();

        //--------------------------------------------------------------
        // If any of accompanying passenger bookingcode have a match in the
        // accTvlInfo.fareClassBkgCds() vector, we will see as match
        //--------------------------------------------------------------
        for (; ruleBCI != ruleBCE; ++ruleBCI)
        {
          if ((*ruleBCI)[0] == bc[0])
          {
            return true;
          }
        }
      }
      return false;
    }
  }
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validateSameSegReq()
//
// validateSameSegReq()  check if passenger meets same segment requirement
//
//  @param std::vector<AirSeg*>& myAirSeg  - AirSeg of accompanied passenger
//  @param std::vector<AirSeg*>& accAirSeg - AirSeg of accompanying passenger
//  @param AccompaniedTravelInfo&   - Accompanied Travel Rule Item Info
//  @param ACCTVL_VALID_RESULT&     - Failure reason, IN & OUT
//
//  @return Record3ReturnTypes - possible values are:
//                   PASS        matched same seg requirement
//                   FAIL        did not match same seg requirement
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
AccompaniedTravel::validateSameSegReq(const std::vector<AirSeg*>& myAirSeg,
                                      const std::vector<AirSeg*>& accAirSeg,
                                      const AccompaniedTravelInfo& accTvlInfo,
                                      ACCTVL_VALID_RESULT& accTvlResult) const
{
  if (UNLIKELY(accAirSeg.empty()))
  {
    return FAIL;
  }

  const bool reqOneSectorOnly = (accTvlInfo.accTvlOneSector() != notApply);

  std::vector<AirSeg*>::const_iterator myAirSegI;
  std::vector<AirSeg*>::const_iterator accAirSegI;

  myAirSegI = myAirSeg.begin();
  accAirSegI = accAirSeg.begin();

  // Accompaning Passenger may have more travel segments in front or
  // between the accompanied passenger segments
  // find the segment that is same as begin() of accompanied passenger
  // first, and then compare each segment of accompanied passenger
  // with accompanying passenger
  for (; myAirSegI != myAirSeg.end(); myAirSegI++)
  {
    while ((accAirSegI != accAirSeg.end()) && !isSameAirSeg(*(*myAirSegI), *(*accAirSegI)))
    {
      accAirSegI++;
    }

    // passed requirement for this sector
    if (UNLIKELY(reqOneSectorOnly))
    {
      if (accAirSegI == accAirSeg.end())
      {
        // no match for this seg, looking for next one
        accAirSegI = accAirSeg.begin();
        continue;
      }

      // matched one
      accTvlResult = accTvlPASS;
      return PASS;
    }
    else if (accAirSegI == accAirSeg.end())
    {
      // could not find same segment
      return FAIL;
    }
    accAirSegI++; // found same seg of this one, compare next
  } // myAirSeg loop

  if (UNLIKELY(reqOneSectorOnly))
  {
    return FAIL; // if we got here, it means nothing has passed
  }
  else
  {
    return PASS; // nothing failed
  }
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::isSameAirSeg()
//
// isSameAirSeg()   if two segments have same CarrierCode, FlightNumber
//                  and departDT, we say they are same travel segment
//                  Not sure if this is what all other category rule expected,
//                  did not put this into AirSeg or TravelSeg class.
//
//  @param AirSeg&   airSeg1     - First AirSeg to be compared
//  @param AirSeg&   airSeg2     - The other AirSeg to be compared
//
//  @return boolean - possible values are:
//                   true         match
//                   false        did not match
//
// </PRE>
//-------------------------------------------------------------------
bool
AccompaniedTravel::isSameAirSeg(const AirSeg& airSeg1, const AirSeg& airSeg2) const
{
  if (airSeg1.carrier() == airSeg2.carrier() && airSeg1.flightNumber() == airSeg2.flightNumber() &&
      airSeg1.departureDT() == airSeg2.departureDT())
  {
    return true;
  }
  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::minNoAccPsgReq()
//
//  minNoAccPsgReq()  calculate required minimum number of qualified
//                    accompanying passengers according to number of
//                    accompanied passengers and Cat13 rule item info
//
//  @param  uint32_t& numAccompaniedPsg
//  @param  AccompaniedTravelInfo&          - Rule Item info
//
//  @return uint32_t    - Minimum number of qualified accompanying passengers
//                        required
//
// </PRE>
//-------------------------------------------------------------------
uint32_t
AccompaniedTravel::minNoAccPsgReq(const uint32_t& numAccompaniedPsg,
                                  const AccompaniedTravelInfo& accTvlInfo)
{
  if (LIKELY(!(accTvlInfo.minNoPsg() > 0) && !(accTvlInfo.maxNoPsg() > 0)))
  {
    // one-on-one
    return numAccompaniedPsg;
  }

  uint32_t noByMinRule = 0;
  uint32_t noByMaxRule = 0;

  if (accTvlInfo.minNoPsg() > 0)
  {
    noByMinRule = static_cast<uint32_t>(accTvlInfo.minNoPsg()) * numAccompaniedPsg;
  }
  if (accTvlInfo.maxNoPsg() > 0)
  {
    noByMaxRule = numAccompaniedPsg / static_cast<uint32_t>(accTvlInfo.maxNoPsg());
    if ((numAccompaniedPsg % static_cast<uint32_t>(accTvlInfo.maxNoPsg())) != 0)
    {
      noByMaxRule++;
    }
  }
  return ((noByMinRule > noByMaxRule) ? noByMinRule : noByMaxRule);
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::qualifyAccPsgTypeFare()
//
// qualifyAccPsgTypeFare()   Qualify if an accompanying passenger meet
//                           type, age and optional FareClass/BookingCode
//                           requirement.
//                           Accompanying passengers' FareClass/BookingCode
//                           can be NULL during fare market validation for
//                           accompanied passenger; it would not be NULL during
//                           combinability process.
//
//  @param PaxType&                - PaxType of Accompanying passenger
//  @param chkFareClassBkgCds&     - If need to chk FareClass/BkgCd match
//  @param PaxTypeFare&            - Accompanying passenger's PaxTypeFare
//  @param AccompaniedTravelInfo&  - Cat13 Rule Item Info
//
//  @return ACCTVL_VALID_RESULT   - possible values are:
//                 accTvlPASS       qualified
//                 others           failed with failReason returned
//
// </PRE>
//-------------------------------------------------------------------
ACCTVL_VALID_RESULT
AccompaniedTravel::qualifyAccPsgTypeFare(const PaxType& paxType,
                                         const bool& chkFareClassBkgCds,
                                         const PaxTypeFare& paxTypeFare,
                                         const AccompaniedTravelInfo accTvlInfo,
                                         PricingTrx& trx,
                                         DiagCollector* diag /* = 0*/) const
{ // lint !e578
  bool isPaxTypeMatch = true;
  if (UNLIKELY(!accTvlInfo.accPsgType().empty()))
  {
    isPaxTypeMatch =
        PaxTypeUtil::isAnActualPax(paxType, paxTypeFare.carrier(), accTvlInfo.accPsgType(), 0, 0);
  }

  if (UNLIKELY(isPaxTypeMatch != true))
  {
    return failReasonPsgType;
  }

  if (UNLIKELY((paxType.age() > 0 || paxType.paxTypeInfo().infantInd() == tse::YES) &&
      ((accTvlInfo.minAge() > 0 && paxType.age() < accTvlInfo.minAge()) ||
       (accTvlInfo.maxAge() > 0 && paxType.age() > accTvlInfo.maxAge()))))
  {
    return failReasonAge;
  }

  if (UNLIKELY(chkFareClassBkgCds && !accTvlInfo.fareClassBkgCds().empty() &&
      checkFareClassBkgCode(paxTypeFare, accTvlInfo, trx, diag) != true))
  {
    return failReasonFB;
  }

  return accTvlPASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::qualifyPsgTypeAge()
//
// qualifyPsgTypeAge()  See if a PaxType meet minAge, maxAge and PaxTypeCode
//                      requirement
//
//  @param PaxType&            - PaxType to be qualified
//  @param int  minAge         - required minimum age
//  @param int  maxAge         - required maximum age
//  @param int  PaxTypeCode&   - required PaxTypeCode
//
//  @return boolean       -  possible values are
//               true       qualified
//               false      not qualified
//
// </PRE>
//-------------------------------------------------------------------
bool
AccompaniedTravel::qualifyPsgTypeAge(const PaxType& paxType,
                                     const int minAge,
                                     const int maxAge,
                                     const PaxTypeCode& requiredPaxType) const
{ // lint !e578
  const PaxTypeInfo& accPsgTypeInfo = paxType.paxTypeInfo();

  //-----------------------------------------------------------------
  // ATPCO: Everyone is default as ADULT, no matter its PaxType
  // (except CHILD and INFANT)
  //-----------------------------------------------------------------
  if (paxType.paxType() == requiredPaxType ||
      (requiredPaxType == ADULT && accPsgTypeInfo.childInd() != tse::YES &&
       accPsgTypeInfo.infantInd() != tse::YES /* isAdult */) ||
      (requiredPaxType == CHILD && accPsgTypeInfo.childInd() == tse::YES /* isChild */) ||
      (requiredPaxType == INFANT && accPsgTypeInfo.infantInd() == tse::YES /* isInfant */))
  {
    // Type matched
  }
  else
  {
    return false;
  }

  const int accPsgAge = paxType.age();

  if (accPsgAge > 0 ||
      paxType.paxTypeInfo().infantInd() == tse::YES) // infant age 0 needs to be checked
  {
    if ((minAge > 0 && accPsgAge < minAge) || (maxAge > 0 && accPsgAge > maxAge))
    {
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::displayAccTvlRuleToDiag()
//
// displayAccTvlRuleToDiag()   details rule data in AccompaniedTravelInfo
//                             to diagnostic collector
//
//  @param AccompaniedTravelInfo&    - Rule Item Info
//  @param DiagCollector*            - Diagnostic Collector
//
//  @return void
//
// </PRE>
//-------------------------------------------------------------------
void
AccompaniedTravel::displayAccTvlRuleToDiag(const AccompaniedTravelInfo& accTvlInfo,
                                           DiagCollector& diag)
{
  //----------------------------------------------------------------------
  //   CATEGORY 13 RULE DATA
  //----------------------------------------------------------------------
  diag << "CATEGORY 13 RULE DATA:" << std::endl;

  //--------------------------
  // Unavailable Tag
  //--------------------------
  if (accTvlInfo.unavailTag() != notApply)
  {
    if (accTvlInfo.unavailTag() == dataUnavailable)
    {
      diag << " UNAVAILABLE DATA" << std::endl;
    }
    else
    {
      diag << " TEXT DATA ONLY" << std::endl;
    }
  }

  //--------------------------------------
  // Geographic Specification Table 995
  //--------------------------------------
  diag << " GEOGRAPHIC SPECIFICATION TABLE 995 VIA1: " << accTvlInfo.geoTblItemNoVia1()
       << std::endl;
  diag << " GEOGRAPHIC SPECIFICATION TABLE 995 VIA2: " << accTvlInfo.geoTblItemNoVia2()
       << std::endl;

  //-------------------------------------------
  // Same Sector/Compartment/Rule Requirement
  //-------------------------------------------
  if (accTvlInfo.accTvlAllSectors() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR ALL SECTORS" << std::endl;
  }
  if (accTvlInfo.accTvlOneSector() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR AT LEAST ONE SECTOR" << std::endl;
  }
  if (accTvlInfo.accTvlSameCpmt() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR SAME COMPARTMENT" << std::endl;
  }
  if (accTvlInfo.accTvlSameRule() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR SAME TARIFF/CARRIER RULE" << std::endl;
  }

  //---------------------------------------
  // Accompanying Passenger Restrictions
  //---------------------------------------
  if (accTvlInfo.accPsgAppl() == notApply)
  {
    diag << " ACCOMPANYING PSG MUST BE AS" << std::endl;
  }
  else
  {
    diag << " ACCOMPANYING PSG MUST NOT BE AS" << std::endl;
  }

  diag << "   PSG TYPE - " << accTvlInfo.accPsgType() << std::endl;

  if (accTvlInfo.accPsgId() == notApply)
  {
    diag << "   NEED NOT IDENTIFY WHEN TICKETING" << std::endl;
  }
  else
  {
    diag << "   MUST IDENTIFY WHEN TICKETING" << std::endl;
  }

  // Fare Class Code or Booking Code
  if (accTvlInfo.fareClassBkgCdInd() == fareClass)
  {
    diag << "   FARE CLASS  -  ";
  }
  else
  {
    // bookingCode
    diag << "   BOOKING CODE -  ";
  }
  std::vector<FareClassCode>::const_iterator fareClassBkgCdsI;
  fareClassBkgCdsI = accTvlInfo.fareClassBkgCds().begin();
  for (; fareClassBkgCdsI != accTvlInfo.fareClassBkgCds().end(); fareClassBkgCdsI++)
  {
    diag << (*fareClassBkgCdsI) << "  ";
  }
  diag << std::endl;

  // age
  if (accTvlInfo.minAge() > 0)
  {
    diag << "   MIM AGE - " << accTvlInfo.minAge() << std::endl;
  }
  if (accTvlInfo.maxAge() > 0)
  {
    diag << "   MAX AGE - " << accTvlInfo.maxAge() << std::endl;
  }

  //---------------------------------------------------------
  // Accompanyied/Accompanying passenger number restriction
  //---------------------------------------------------------
  if (accTvlInfo.maxNoPsg() > 0)
  {
    diag << " MAXIMUM NO OF PSG ACCOMPANIED PER ACCOMPANYING PSG - ";
    diag << accTvlInfo.maxNoPsg() << std::endl;
  }

  if (accTvlInfo.minNoPsg() > 0)
  {
    diag << " MINIMUM NO OF ACCOMPANYING PSG PER ACCOMPANIED PSG - ";
    diag << accTvlInfo.minNoPsg() << std::endl;
  }

  //----------------------------------------------------------------------
  //    End of Display
  //----------------------------------------------------------------------
  diag << "***************************************************************" << std::endl;

  diag.flushMsg();
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::displayDiscountRuleToDiag()
//
// displayDiscountRuleToDiag()     details accompanied travel restriction
//                                 in DiscountInfo to diagnostic collector
//
//  @param DiscountInfo&        - Discount Rule Item Info
//  @param DiagCollector*       - Diagnostic Collector
//
//  @return void
//
// </PRE>
//-------------------------------------------------------------------
void
AccompaniedTravel::displayDiscountRuleToDiag(const DiscountInfo& discInfo, DiagCollector& diag)
{
  //-------------------------------------------
  // Accompanied Travel in (Cat19-22) Discount Info
  //-------------------------------------------
  diag << "    " << std::endl << "    " << std::endl;
  diag << "DISCOUNT FARE ACCOMPANIED TRAVEL RULE DATA" << std::endl;

  //-------------------------------------------
  // Same Sector/Compartment/Rule Requirement
  //-------------------------------------------
  if (discInfo.accTvlAllSectors() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR ALL SECTORS" << std::endl;
  }
  if (discInfo.accTvlOneSector() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR AT LEAST ONE SECTOR" << std::endl;
  }
  if (discInfo.accTvlSameCpmt() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR SAME COMPARTMENT" << std::endl;
  }
  if (discInfo.accTvlSameRule() != notApply)
  {
    diag << " ACCOMPANYING TRAVEL MUST BE FOR SAME TARIFF/CARRIER RULE" << std::endl;
  }

  //---------------------------------------
  // Accompanying Passenger Restrictions
  //---------------------------------------
  if (discInfo.accPsgAppl() == notApply)
  {
    diag << " DISCOUNT APPLY WHEN ONE OF THE FOLLOWING CONDITION MATCH" << std::endl;
  }
  else
  {
    diag << " DISCOUNT APPLY WHEN ONE OF THE FOLLOWING CONDITION NOT MATCH" << std::endl;
  }

  std::vector<DiscountSegInfo*>::const_iterator discSegI;
  discSegI = discInfo.segs().begin();
  uint16_t segIndex = 0;
  for (; discSegI != discInfo.segs().end(); discSegI++, segIndex++)
  {
    const DiscountSegInfo& discSegInfo = *(*discSegI);
    diag << "** CONDITION " << segIndex << " **" << std::endl;
    diag << " MIN NO PSG " << discSegInfo.minNoPsg() << std::endl;
    diag << " MAX NO PSG " << discSegInfo.maxNoPsg() << std::endl;
    diag << " MIN AGE1 " << discSegInfo.minAge1() << std::endl;
    diag << " MAX AGE1 " << discSegInfo.maxAge1() << std::endl;
    diag << " MIN AGE2 " << discSegInfo.minAge2() << std::endl;
    diag << " MAX AGE2 " << discSegInfo.maxAge2() << std::endl;
    diag << " MIN AGE3 " << discSegInfo.minAge3() << std::endl;
    diag << " MAX AGE3 " << discSegInfo.maxAge3() << std::endl;
    diag << " ACC PSG TYPE 1 " << discSegInfo.accPsgType1() << std::endl;
    diag << " ACC PSG TYPE 2 " << discSegInfo.accPsgType2() << std::endl;
    diag << " ACC PSG TYPE 3 " << discSegInfo.accPsgType3() << std::endl;
    if (discInfo.fareClassBkgCodeInd() == fareClass)
    {
      diag << " FARE CLASS CODE " << discSegInfo.fareClass() << std::endl;
    }
    else if (discInfo.fareClassBkgCodeInd() == bookingCode)
    {
      diag << " FARE BOOKING CODE " << discSegInfo.bookingCode() << std::endl;
    }
  }
  diag << "    " << std::endl << std::endl;
  diag.flushMsg();
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::needCombinabiltyCheck()
//
// needCombinabiltyCheck()    if need to set flag for combinability validation
//
//  @param AccomapniedTravelInfo&    - Accompanied Travel Rule Item Info
//
//  @return void
//
// </PRE>
//-------------------------------------------------------------------
bool
AccompaniedTravel::needCombinabiltyCheck(const AccompaniedTravelInfo& accTvlInfo) const
{
  if (accTvlInfo.fareClassBkgCdInd() != notApply ||
      /* no need to check, as coming from same PNR, Aug.2005
            accTvlInfo.accTvlAllSectors() != notApply
                ||
            accTvlInfo.accTvlOut() != notApply
                ||
            accTvlInfo.accTvlOneSector() != notApply
                ||
      */
      accTvlInfo.accTvlSameCpmt() != notApply ||
      accTvlInfo.accTvlSameRule() != notApply)
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::needCombinabiltyCheck()
//
// needCombinabiltyCheck()   if need to set flag for combinability validation
//
//  @param DiscountInfo&    - Discount Rule Item Info
//
//  @return void
//
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::needCombinabiltyCheck(const DiscountInfo& discountInfo) const
{ // lint !e578
  if (discountInfo.fareClassBkgCodeInd() != notApply ||
      /* no need to check, as coming from same PNR, Jan.2006
            discountInfo.accTvlAllSectors() != notApply
                ||
            discountInfo.accTvlOut() != notApply
                ||
            discountInfo.accTvlOneSector() != notApply
                ||
                */
      discountInfo.accTvlSameCpmt() != notApply ||
      discountInfo.accTvlSameRule() != notApply)
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validateInfFares()
//
// Description: For infant travel with 0 amount discount, it must use
//            fare basis of first accompanying passenger
//            Since we allow INF pricing seperately with ADT now, we do not
//            require an adult here
//
//   @param PricingTrx                - Pricing transaction
//   @param FarePath&                 - FarePath of infant passenger(s)
//   @param std::vector<FarePath*>&   - FarePath of all passengers
//
//   @return bool - possible values are:
//           true   Validation succeeded
//           false  Validatoin failed
//
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::validateInfFares(const PricingTrx& trx,
                                    const FarePath& infFP,
                                    const std::vector<FarePath*>& farePathGrp)
{
  if (UNLIKELY(farePathGrp.size() <= 1))
  {
    return true;
  }

  return validateInfFares(trx, infFP, *farePathGrp.front());
}

//-----------------------------------------------------------------------
bool
AccompaniedTravel::validateInfFares(const PricingTrx& trx,
                                    const FarePath& infFP,
                                    const FarePath& accFarePath)
{
  // IS transactions don't have to make sure that infants with 0 nuc amount
  // have the same fare class as their accompanying adults, so just
  // return true here
  if (UNLIKELY(trx.getTrxType() == PricingTrx::IS_TRX))
  {
    return true;
  }

  std::vector<FareUsage*> infZeroAmountFUs;
  getZeroAmountFareUsages(infZeroAmountFUs, infFP);

  if (infZeroAmountFUs.size() == 0)
    return true;

  std::vector<FareUsage*>::const_iterator fuIt = infZeroAmountFUs.begin();
  const std::vector<FareUsage*>::const_iterator fuItEnd = infZeroAmountFUs.end();

  FUMap fuMap;
  getFUMap(fuMap, accFarePath);

  const FUMapCI fuMapIterEnd = fuMap.end();

  for (; fuIt != fuItEnd; fuIt++)
  {
    const FUMapCI fuMapIter = fuMap.find((*fuIt)->paxTypeFare()->fareMarket());

    if (UNLIKELY(fuMapIter == fuMapIterEnd))
      return false;

    if ((fuMapIter->second)->paxTypeFare()->fareClass() != (*fuIt)->paxTypeFare()->fareClass())
      return false;
  }

  return true;
}

void
AccompaniedTravel::getZeroAmountFareUsages(std::vector<FareUsage*>& fuVec, const FarePath& farePath)
{
  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  for (; puIt != puItEnd; puIt++)
  {
    std::vector<FareUsage*>::const_iterator fuIt = (*puIt)->fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuItEnd = (*puIt)->fareUsage().end();

    for (; fuIt != fuItEnd; fuIt++)
    {
      if ((*fuIt)->paxTypeFare()->nucFareAmount() == 0)
      {
        fuVec.push_back(*fuIt);
      }
    }
  }
}

void
AccompaniedTravel::getFUMap(FUMap& fuMap, const FarePath& farePath)
{
  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  for (; puIt != puItEnd; puIt++)
  {
    std::vector<FareUsage*>::const_iterator fuIt = (*puIt)->fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuItEnd = (*puIt)->fareUsage().end();

    for (; fuIt != fuItEnd; fuIt++)
    {
      fuMap[(*fuIt)->paxTypeFare()->fareMarket()] = *fuIt;
    }
  }
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validateAccTvl()
//
// Description: Validate accompanying travel restriction for WTFR
//              from restored AccTvlFarePath
//
//   @param PricingTrx                          - Pricing transaction
//   @param std::vector<const AccTvlFarePath*>& - FarePath of all passengers
//   @param std::vector<bool>&             - validation results of all FarePath
//
//   @return bool - possible values are:
//           true   Validation fully succeeded
//           false  At least part of validation failed
//
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::validateAccTvl(PricingTrx& trx,
                                  const std::vector<const AccTvlFarePath*>& accFarePathVec,
                                  std::vector<bool>& resultVec) const
{
  bool fullyPassed = true;

  // for each fare path, we validate PaxTypeFare that needs accompanied
  // restriction validation
  std::vector<const AccTvlFarePath*>::const_iterator farePathI = accFarePathVec.begin();
  const std::vector<const AccTvlFarePath*>::const_iterator farePathIEnd = accFarePathVec.end();

  resultVec.clear();

  for (; farePathI != farePathIEnd; farePathI++)
  {
    const AccTvlFarePath& currentFarePath = **farePathI;
    bool validationRtn = true;

    std::vector<SimplePaxTypeFare*>::const_iterator ptfIter =
        currentFarePath.paxTypeFares().begin();
    std::vector<SimplePaxTypeFare*>::const_iterator ptfIterEnd =
        currentFarePath.paxTypeFares().end();

    for (; ptfIter != ptfIterEnd; ptfIter++)
    {
      const SimplePaxTypeFare& paxTypeFare = **ptfIter;
      if (!paxTypeFare.simpleAccTvlRule().empty())
      {
        std::vector<const SimplePaxTypeFare*> accPaxTypeFares;
        getAccTvlPaxTypeFares(paxTypeFare, accPaxTypeFares, &currentFarePath, accFarePathVec);

        if (!validateAccTvl(trx, paxTypeFare, accPaxTypeFares))
        {
          validationRtn = false;
          fullyPassed = false;
          break;
        }
      }
    }

    resultVec.push_back(validationRtn);
  }
  return fullyPassed;
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::getAccTvlPaxTypeFares()
//
// Description: Get same fare break SimplePaxTypeFare for
//              accompanying travel restriction for WTFR
//
//   @param PricingTrx          - Pricing transaction
//   @param SimplePaxTypeFare   - one being validated
//   @param std::vector<const SimplePaxTypeFare*>&
//                              - (output)SimplePaxTypeFare of all other
//                                passengers with same fare break
//   @param AccTvlFarePath*     - FarePath contains the fare being validated
//   @param std::vector<const AccTvlFarePath*>& accFarePathVec )
//                              - All FarePath
//
//   @return
//
// </PRE>
//-----------------------------------------------------------------------
void
AccompaniedTravel::getAccTvlPaxTypeFares(const SimplePaxTypeFare& currPaxTypeFare,
                                         std::vector<const SimplePaxTypeFare*>& accPaxTypeFares,
                                         const AccTvlFarePath* currentFarePath,
                                         const std::vector<const AccTvlFarePath*>& accFarePathVec)
    const
{
  std::vector<const AccTvlFarePath*>::const_iterator farePathI = accFarePathVec.begin();
  std::vector<const AccTvlFarePath*>::const_iterator farePathIEnd = accFarePathVec.end();

  const uint16_t origSegNo = currPaxTypeFare.origSegNo();
  const uint16_t destSegNo = currPaxTypeFare.destSegNo();

  for (; farePathI != farePathIEnd; farePathI++)
  {
    const AccTvlFarePath* farePath = *farePathI;

    if (farePath == currentFarePath)
      continue;

    std::vector<SimplePaxTypeFare*>::const_iterator ptfIter = farePath->paxTypeFares().begin();
    std::vector<SimplePaxTypeFare*>::const_iterator ptfIterEnd = farePath->paxTypeFares().end();

    for (; ptfIter != ptfIterEnd; ptfIter++)
    {
      const SimplePaxTypeFare& paxTypeFare = **ptfIter;
      if (paxTypeFare.origSegNo() > origSegNo)
        break;

      if (paxTypeFare.origSegNo() != origSegNo)
        continue;

      if (paxTypeFare.destSegNo() != destSegNo)
        continue;

      accPaxTypeFares.push_back(&paxTypeFare);
    }
  }
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validateAccTvl()
//
// Description: validate accompanying travel restriction for WTFR, use
//              SimplePaxTypeFare of a FarePath with SimplePaxTypeFares
//              of same fare break from all FarePaths
//
//   @param PricingTrx          - Pricing transaction
//   @param SimplePaxTypeFare   - one being validated
//   @param std::vector<const SimplePaxTypeFare*>&
//                              - SimplePaxTypeFare of all other
//                                passengers with same fare break
//
//   @return
//
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::validateAccTvl(PricingTrx& trx,
                                  const SimplePaxTypeFare& paxTypeFare,
                                  const std::vector<const SimplePaxTypeFare*>& accPaxTypeFares)
    const
{
  DiagManager diag(trx, Diagnostic860);

  if (UNLIKELY(diag.isActive()))
  {
    diagnosis(paxTypeFare, accPaxTypeFares, diag.collector());
  }

  std::vector<SimpleAccTvlRule*>::const_iterator ruleIter = paxTypeFare.simpleAccTvlRule().begin();

  std::vector<SimpleAccTvlRule*>::const_iterator ruleIterEnd = paxTypeFare.simpleAccTvlRule().end();

  for (; ruleIter != ruleIterEnd; ruleIter++)
  {
    const SimpleAccTvlRule& rule = **ruleIter;

    if (UNLIKELY(diag.isActive()))
    {
      // diagnosis rule
      diag << "TRIES RULE...\n";

      diagnosis(rule, diag.collector());
    }

    if (rule.isTktGuaranteed())
    {
      if (UNLIKELY(diag.isActive()))
        diag << "RULE PASSED - TKT GUARANTEED\n";
      return true;
    }

    if (foundMatchedPsg(rule, paxTypeFare, accPaxTypeFares))
    {
      if (UNLIKELY(diag.isActive()))
        diag << "RULE PASSED\n";
      return true;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
        diag << "RULE FAILED\n";
    }
  }

  if (UNLIKELY(diag.isActive()))
    diag << "FAILED ACCOMPANIED TRAVEL RESTRICTION\n";

  return false;
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::foundMatchedPsg()
//
// Description: validate to see if we have enough qualified
//              accompanying passenger for a SimplePaxTypeFare
//
//   @param SimpleAccTvlRule&    - rule being validated
//   @param SimplePaxTypeFare&   - SimplePaxTypeFare being validated
//   @param std::vector<const SimplePaxTypeFare*>&
//                              - SimplePaxTypeFare of all other
//                                passengers with same fare break
//
//   @return
//              true      -   Enough qualified accompanying passenger
//              false     -   No enough qualified accompanying passenger
//
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::foundMatchedPsg(const SimpleAccTvlRule& accTvlRule,
                                   const SimplePaxTypeFare& myPTFare,
                                   const std::vector<const SimplePaxTypeFare*>& accPaxTypeFares)
    const
{
  uint16_t numAccPsg = 0;
  const uint16_t numAccPsgRequired = (accTvlRule.numOfAccPsg == 0) ? 1 : accTvlRule.numOfAccPsg;

  std::vector<const SimplePaxTypeFare*>::const_iterator accPTFareIter = accPaxTypeFares.begin();
  const std::vector<const SimplePaxTypeFare*>::const_iterator accPTFareIterEnd =
      accPaxTypeFares.end();

  for (; accPTFareIter != accPTFareIterEnd; accPTFareIter++)
  {
    const SimplePaxTypeFare& accPTFare = **accPTFareIter;

    if (matchAccPsg(accTvlRule, myPTFare, accPTFare))
    {
      numAccPsg += accPTFare.paxNumber();
    }

    if (numAccPsg >= numAccPsgRequired)
      return true;
  }

  // no enough number of accompany passenger
  // need to support pricing cat13 that travel in group
  if (myPTFare.paxNumber() > 1)
  {
    if (matchAccPsg(accTvlRule, myPTFare, myPTFare))
    {
      numAccPsg += myPTFare.paxNumber();
    }

    if (numAccPsg >= numAccPsgRequired)
      return true;
  }

  return false;
}

bool
AccompaniedTravel::matchAccPsg(const SimpleAccTvlRule& accTvlRule,
                               const SimplePaxTypeFare& myPTFare,
                               const SimplePaxTypeFare& accPTFare) const
{
  //------------------------------------------------------------
  // Same Compartment requirment
  // Compartment is same for all sectors for a PaxTypeFare
  // We check here instead of within each air sectors.
  // APTCO requirement is ignored
  //------------------------------------------------------------
  if (accTvlRule.reqSameCpmt())
  {
    if (myPTFare.cabin() != accPTFare.cabin())
    {
      return false;
    }
  }

  if (accTvlRule.reqSameRule())
  {
    if ((myPTFare.ruleNumber() != accPTFare.ruleNumber()) ||
        (myPTFare.tcrRuleTariff() != accPTFare.tcrRuleTariff()))
    {
      return false;
    }
  }

  // for PaxType, FareClass/BookingCode request
  bool optPassed = false;
  std::vector<const SimpleAccTvlRule::AccTvlOpt*>::const_iterator accTvlOptIter =
      accTvlRule._accTvlOptions.begin();
  const std::vector<const SimpleAccTvlRule::AccTvlOpt*>::const_iterator accTvlOptIterEnd =
      accTvlRule._accTvlOptions.end();

  for (; accTvlOptIter != accTvlOptIterEnd; accTvlOptIter++)
  {
    const SimpleAccTvlRule::AccTvlOpt& accTvlOpt = **accTvlOptIter;

    // PaxType
    const bool needPaxTypeChk = !accTvlOpt._paxTypes.empty();
    bool paxTypeMatch = false;

    if (needPaxTypeChk)
    {
      std::vector<const PaxTypeCode*>::const_iterator optPaxTypeI = accTvlOpt._paxTypes.begin();
      const std::vector<const PaxTypeCode*>::const_iterator optPaxTypeIEnd =
          accTvlOpt._paxTypes.end();

      for (; optPaxTypeI != optPaxTypeIEnd; optPaxTypeI++)
      {
        if (**optPaxTypeI == accPTFare.paxTypeCode())
        {
          paxTypeMatch = true;
          break;
        }
      }
    }

    // FareClass/BookingCode
    const bool needFcBkgCdChk = !accTvlOpt._fareClassBkgCds.empty();
    bool fcBkgCdMatch = false;

    if (needFcBkgCdChk)
    {
      std::vector<const std::string*>::const_iterator optFcBkgCdI =
          accTvlOpt._fareClassBkgCds.begin();
      const std::vector<const std::string*>::const_iterator optFcBkgCdIEnd =
          accTvlOpt._fareClassBkgCds.end();

      if (accTvlRule.reqBkgCds())
      {
        std::string accBkgCd = accPTFare.bookingCode();

        for (; optFcBkgCdI != optFcBkgCdIEnd; optFcBkgCdI++)
        {
          if (**optFcBkgCdI == accBkgCd)
          {
            fcBkgCdMatch = true;
            break;
          }
        }
      }
      else
      {
        FareClassCode accFc = accPTFare.fareClass();

        for (; optFcBkgCdI != optFcBkgCdIEnd; optFcBkgCdI++)
        {
          if (RuleUtil::matchFareClass((*optFcBkgCdI)->c_str(), accFc.c_str()))
          {
            fcBkgCdMatch = true;
            break;
          }
        }
      }
    }

    const bool paxMatch = (!needPaxTypeChk || paxTypeMatch) && (!needFcBkgCdChk || fcBkgCdMatch);

    if (accTvlRule.negAppl())
    {
      if (paxMatch)
      {
        optPassed = false;
        break;
      }
      optPassed = true;
      continue; // must pass all opt
    }
    else
    {
      // not NegAppl
      if (paxMatch)
      {
        optPassed = true;
        break;
      }
      continue; // try next opt
    }
  }
  return optPassed;
}

void
AccompaniedTravel::diagnosis(const SimplePaxTypeFare& paxTypeFare, DiagCollector& diag) const
{
  diag << paxTypeFare.paxNumber() << paxTypeFare.requestedPaxTypeCode();

  diag << " TVL SEG " << paxTypeFare.origSegNo();
  if (paxTypeFare.origSegNo() != paxTypeFare.destSegNo())
  {
    diag << " TO " << paxTypeFare.destSegNo();
  }
  diag << " AT CABIN " << paxTypeFare.cabin() << " BOOKED BY " << paxTypeFare.bookingCode()
       << " USING FARE " << paxTypeFare.fareClass() << " RULE " << paxTypeFare.ruleNumber() << " "
       << paxTypeFare.tcrRuleTariff() << "\n";
}

void
AccompaniedTravel::diagnosis(const SimpleAccTvlRule& accTvlRule, DiagCollector& diag) const
{
  if (accTvlRule.numOfAccPsg != 0)
  {
    diag << "  REQUIRES " << accTvlRule.numOfAccPsg << " ACC PSG\n";
  }

  if (accTvlRule.reqSameCpmt())
  {
    diag << "  USING SAME COMPARTMENT\n";
  }

  if (accTvlRule.reqSameRule())
  {
    diag << "  USING SAME FARE RULE\n";
  }

  std::vector<const SimpleAccTvlRule::AccTvlOpt*>::const_iterator optIter =
      accTvlRule._accTvlOptions.begin();
  const std::vector<const SimpleAccTvlRule::AccTvlOpt*>::const_iterator optIterEnd =
      accTvlRule._accTvlOptions.end();

  for (; optIter != optIterEnd; optIter++)
  {
    const SimpleAccTvlRule::AccTvlOpt& ruleOption = **optIter;
    if (accTvlRule.negAppl())
    {
      diag << "  ACCOMPANYING PSG MUST NOT BE\n";
    }
    else
    {
      diag << "  ACCOMPANYING PSG MUST BE\n";
    }

    if (!ruleOption._fareClassBkgCds.empty())
    {
      if (accTvlRule.reqBkgCds())
      {
        diag << "   BOOKING CODES:";
      }
      else
      {
        diag << "   FARE CLASS:";
      }

      std::vector<const std::string*>::const_iterator fareClassBkgCdI =
          ruleOption._fareClassBkgCds.begin();
      const std::vector<const std::string*>::const_iterator fareClassBkgCdIEnd =
          ruleOption._fareClassBkgCds.end();

      for (; fareClassBkgCdI != fareClassBkgCdIEnd; fareClassBkgCdI++)
      {
        diag << **fareClassBkgCdI << " ";
      }
      diag << "\n";
    }
    if (!ruleOption._paxTypes.empty())
    {
      diag << "   PAXTYPE:";
      std::vector<const PaxTypeCode*>::const_iterator paxTypeI = ruleOption._paxTypes.begin();
      const std::vector<const PaxTypeCode*>::const_iterator paxTypeIEnd =
          ruleOption._paxTypes.end();
      for (; paxTypeI != paxTypeIEnd; paxTypeI++)
      {
        diag << **paxTypeI << " ";
      }
      diag << "\n";
    }
  }
}

void
AccompaniedTravel::diagnosis(const SimplePaxTypeFare& paxTypeFare,
                             const std::vector<const SimplePaxTypeFare*>& accPaxTypeFares,
                             DiagCollector& diag) const
{
  diag << "\nACCOMPANY TRAVEL RESTRICTED\n";
  diagnosis(paxTypeFare, diag);

  diag << "ACCOMPANIED BY\n";
  std::vector<const SimplePaxTypeFare*>::const_iterator paxTypeFareIter = accPaxTypeFares.begin();
  const std::vector<const SimplePaxTypeFare*>::const_iterator paxTypeFareIEnd =
      accPaxTypeFares.end();

  for (; paxTypeFareIter != paxTypeFareIEnd; paxTypeFareIter++)
  {
    diagnosis(**paxTypeFareIter, diag);
  }
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::collectTrailerMsg()
//
// Description: Print accompanying travel restriction in simple message
//              for WPA trailer message
//
//   @param ostringstream&          - Output object
//   @param PaxTypeFare&            - PaxTypeFare object
//   @param AccompaniedTravelInfo&  - Cat13 rule
//
//   @return
//
// </PRE>
//-----------------------------------------------------------------------
void
AccompaniedTravel::collectTrailerMsg(std::ostringstream& outputStr,
                                     const AccompaniedTravelInfo& accTvlInfo)
{
  if (accTvlInfo.accTvlSameRule() != notApply)
  {
    outputStr << " SAME RULE";
  }
  if (accTvlInfo.accTvlSameCpmt() != notApply)
  {
    outputStr << " SAME CABIN";
  }

  if (accTvlInfo.accPsgAppl() != applyMust)
  {
    outputStr << " NOT";
  }

  if (!accTvlInfo.accPsgType().empty())
  {
    outputStr << " " << accTvlInfo.accPsgType();
  }
  outputStr << " PASSENGER";

  if (accTvlInfo.fareClassBkgCdInd() != notApply)
  {
    outputStr << " ON";

    std::vector<FareClassCode>::const_iterator fareClassBkgCdI =
        accTvlInfo.fareClassBkgCds().begin();
    const std::vector<FareClassCode>::const_iterator fareClassBkgCdIEnd =
        accTvlInfo.fareClassBkgCds().end();

    for (; fareClassBkgCdI != fareClassBkgCdIEnd; fareClassBkgCdI++)
    {
      outputStr << " " << *fareClassBkgCdI;
    }
    if (accTvlInfo.fareClassBkgCdInd() == fareClass)
      outputStr << " FARE";
    else
      outputStr << " CLASS FARE";
  }
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::collectTrailerMsg()
//
// Description: Print cat19 accompanying travel restriction in simple message
//              for WPA trailer message
//
//   @param ostringstream&          - Output object
//   @param PaxTypeFare&            - PaxTypeFare object
//   @param DiscountInfo&           - Cat19 rule
//
//   @return
//
// </PRE>
//-----------------------------------------------------------------------
void
AccompaniedTravel::collectTrailerMsg(std::ostringstream& outputStr, const DiscountInfo& discInfo)
{
  if (discInfo.accTvlSameRule() != notApply)
  {
    outputStr << " SAME RULE";
  }
  if (discInfo.accTvlSameCpmt() != notApply)
  {
    outputStr << " SAME CABIN";
  }

  if (discInfo.accPsgAppl() != applyMust)
  {
    outputStr << " NOT";
  }

  std::vector<DiscountSegInfo*>::const_iterator discSegI = discInfo.segs().begin();
  const std::vector<DiscountSegInfo*>::const_iterator discSegIEnd = discInfo.segs().end();

  bool isFirstDiscSeg = true;

  for (; discSegI != discSegIEnd; discSegI++)
  {
    if (!isFirstDiscSeg)
    {
      outputStr << "  OR  ";
    }
    else
    {
      isFirstDiscSeg = false;
    }

    const DiscountSegInfo& discSegInfo = *(*discSegI);

    if (!discSegInfo.accPsgType1().empty())
    {
      outputStr << " " << discSegInfo.accPsgType1();
    }
    if (!discSegInfo.accPsgType2().empty())
    {
      outputStr << " " << discSegInfo.accPsgType2();
    }
    if (!discSegInfo.accPsgType3().empty())
    {
      outputStr << " " << discSegInfo.accPsgType2();
    }

    if (discInfo.fareClassBkgCodeInd() != notApply)
    {
      outputStr << " ON";

      if (discInfo.fareClassBkgCodeInd() == fareClass)
      {
        outputStr << " " << discSegInfo.fareClass() << " FARE";
      }
      else
      {
        outputStr << " " << discSegInfo.bookingCode() << " CLASS FARE";
      }
    }
  }
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validateAccTvl()
//
// Description: Interface function called by AltPricingTrx at WTFR entry
//              for accompanied travel restriction validation
//
//   @param PricingTrx&          - PricingTrx object (indead a AltPricingTrx)
//   @param AltPricingTrx::AccompRestrictionVec&  - Contain AccTvl restriction
//                      data and vector for restoring validation result
//
//   @return  boolean
//                 true  - everything passed
//                 false - part or all failed
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::validateAccTvl(PricingTrx& trx, AltPricingTrx::AccompRestrictionVec& accTvlData)
{
  std::vector<const std::string*> accTvlStrs;

  AltPricingTrx::AccompRestrictionVec::iterator accTvlDataIter = accTvlData.begin();
  const AltPricingTrx::AccompRestrictionVec::iterator accTvlDataIterEnd = accTvlData.end();

  for (; accTvlDataIter != accTvlDataIterEnd; accTvlDataIter++)
  {
    accTvlStrs.push_back(&((*accTvlDataIter).validationStr()));
  }

  std::vector<bool> resultVec;

  bool fullyPass = validateAccTvl(trx, accTvlStrs, resultVec);

  accTvlDataIter = accTvlData.begin();
  std::vector<bool>::const_iterator resultIter = resultVec.begin();
  const std::vector<bool>::const_iterator resultIterEnd = resultVec.end();

  for (; accTvlDataIter != accTvlDataIterEnd; ++accTvlDataIter)
  {
    if (resultIter == resultIterEnd)
    {
      (*accTvlDataIter).guaranteed() = false;
      // Reach end only when data went wrong; if so, no guaranteed
    }
    else
    {
      (*accTvlDataIter).guaranteed() = *resultIter;
      ++resultIter;
    }
  }
  return fullyPass;
}

//-----------------------------------------------------------------------
// <PRE>
//
// @MethodName    AccompaniedTravel::validateAccTvl()
//
// Description: function to validate vector of accompanied travel data in String
//              each data is for a WPA priced FarePath
//
//   @param PricingTrx&          - PricingTrx object (indead a AltPricingTrx)
//   @param const std::vector<const std::string*>&
//                               - AccTvl restriction data in vector of string
//   @param std::vector<bool>&   - boolean vector passed in to store the
//                                 validation results
//
//   @return  boolean
//                 true  - everything passed
//                 false - part or all failed
// </PRE>
//-----------------------------------------------------------------------
bool
AccompaniedTravel::validateAccTvl(PricingTrx& trx,
                                  const std::vector<const std::string*>& accTvlData,
                                  std::vector<bool>& resultVec)
{
  DiagManager diag(trx, Diagnostic860);
  if (UNLIKELY(diag.isActive()))
  {
    std::vector<const std::string*>::const_iterator accTvlDataIter = accTvlData.begin();
    const std::vector<const std::string*>::const_iterator accTvlDataIterEnd = accTvlData.end();
    for (; accTvlDataIter != accTvlDataIterEnd; accTvlDataIter++)
    {
      diag << **accTvlDataIter << "\n";
    }
    diag.collector().flushMsg();
  }
  AccTvlDetailIn detailIn;
  std::vector<const AccTvlFarePath*> farePaths;

  if (!detailIn.restoreAccTvlDetail(trx.dataHandle(), accTvlData, farePaths))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "FAILED TO RESTORE ACC TVL DATA - VALIDATION FAILED\n \n";
    }
    return false;
  }

  if (validateAccTvl(trx, farePaths, resultVec))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "VALIDATION PASSED\n \n";
    }
    return true;
  }
  else
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "VALIDATION FAILED\n \n";
    }
    return false;
  }
}

void
AccompaniedTravel::diagFareUsage(DiagCollector& diag, const FareUsage& fareUsage) const
{
  diag << "R2:FARERULE    :  " << fareUsage.paxTypeFare()->vendor() << " "
       << fareUsage.paxTypeFare()->tcrRuleTariff() << " " << fareUsage.paxTypeFare()->carrier()
       << " " << fareUsage.paxTypeFare()->ruleNumber() << std::endl;
  diag << "FARE USAGE     :  ";
  if (fareUsage.paxTypeFare()->actualPaxType())
    diag << fareUsage.paxTypeFare()->actualPaxType()->paxType();
  else
    diag << "NO ACTUAL PAXTYPE";

  diag << " " << fareUsage.paxTypeFare()->fareClass() << " "
       << Money(fareUsage.paxTypeFare()->fareAmount(), fareUsage.paxTypeFare()->currency())
       << (fareUsage.isInbound() ? " I" : " O") << std::endl;
}

ACCTVL_VALID_RESULT
AccompaniedTravel::useSameRule(const PaxTypeFare& myPaxTypeFare, const PaxTypeFare& accPaxTypeFare)
{
  if (UNLIKELY(myPaxTypeFare.ruleNumber() != accPaxTypeFare.ruleNumber()))
    return failReasonSameRuleNum;

  if (UNLIKELY(myPaxTypeFare.tcrRuleTariff() != accPaxTypeFare.tcrRuleTariff()))
    return failReasonSameRuleTariff;

  if (UNLIKELY(myPaxTypeFare.carrier() != accPaxTypeFare.carrier()))
    return failReasonSameRuleCarr;

  return accTvlPASS;
}
}
