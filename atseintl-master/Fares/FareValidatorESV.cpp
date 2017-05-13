//----------------------------------------------------------------------------
//  File:        FareValidatorESV.cpp
//  Created:     2008-04-16
//
//  Description: Class used to validate fares on specified fare market
//
//  Updates:
//
//  Copyright Sabre 2008
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
#include "Fares/FareValidatorESV.h"

#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/QualifyFltAppRuleData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "Fares/RoutingControllerESV.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.Fares.FareValidatorESV");

void
FareValidatorESV::findFirstValidFares(ValidatedFares& validatedFares)
{
  TSELatencyData metrics((*_trx), "FVO FIND FIRST VALID FARES");

  LOG4CXX_DEBUG(logger, "FareValidatorESV::findFirstValidFares(ValidatedFares&)");

  // Ensure that only one governing carrier thread work on specified fare
  // market at same time
  try
  {
    boost::unique_lock<boost::timed_mutex> g(_fareMarket->_esvMutex, boost::posix_time::milliseconds(2000));

    // Set correct travel segment vector in fare market
    if (false == ShoppingUtil::setupFareMarketTravelSegESV(_fareMarket, _itin))
    {
      LOG4CXX_ERROR(logger, "FareValidatorESV::findFirstValidFare - Fare market travel segments "
                             "are not set correctly.");

      return;
    }

    // Get availability list
    std::vector<ClassOfServiceList> availabilityList;

    if (_fareMarket->isFlowJourneyFare())
    {
      getFlowAvailability(availabilityList);
    }
    else
    {
      getCustomAvailability(availabilityList);
    }

    // Get vector of fares for specified passenger type
    const std::vector<PaxTypeFare*>& paxTypeFareVec = getFaresForPaxType();

    PaxTypeFare* paxTypeFare = nullptr;

    // Go thorough all fares
    std::vector<PaxTypeFare*>::const_iterator paxTypeFareIter(paxTypeFareVec.begin()),
        paxTypeFareIterEnd(paxTypeFareVec.end());

    for (; paxTypeFareIter != paxTypeFareIterEnd; ++paxTypeFareIter)
    {
      paxTypeFare = (*paxTypeFareIter);

      // Check if fare object is not null
      if (nullptr == paxTypeFare)
      {
        LOG4CXX_ERROR(logger,
                      "FareValidatorESV::findFirstValidFare - PaxTypeFare object is NULL.");
        continue;
      }

      if (paxTypeFare->fare()->fareInfo()->getPaxType() == "JCB")
      {
        _fareMarket->setJcb(true);
        continue;
      }

      // Check if fare directionality and fare market direction are matched
      // correctly if not skip validation of this fare
      if (((_fareMarket->direction() == FMDirection::OUTBOUND) &&
           (paxTypeFare->directionality() == TO)) ||
          ((_fareMarket->direction() == FMDirection::INBOUND) &&
           (paxTypeFare->directionality() == FROM)))
      {
        continue;
      }

      // If we already got valid one way fare (without cat 10 restrictions)
      // skip validation of one way fares
      if (paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
      {
        if (nullptr != validatedFares[VDF_TAG_1_3_104])
        {
          continue;
        }
      }

      // If we already got one valid round trip and open jaw fare (without
      // cat 10 restrictions) skip validation of round trip fares
      if (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      {
        if (((_fareMarketsCount > 2) || (nullptr != validatedFares[VDF_TAG_1_2_103])) && // CT
            (nullptr != validatedFares[VDF_TAG_1_2_102_104]) && // RT
            ((_fareMarketsCount > 2) || (nullptr != validatedFares[VDF_TAG_2_104]))) // OJ
        {
          continue;
        }
      }

      // If shopping component validation was already performed for this fare
      // and fare is not valid skip processing for this fare
      if (true == paxTypeFare->shoppingComponentValidationFailed())
      {
        continue;
      }

      if (true == skipFareIfCat10NotValid(paxTypeFare, validatedFares))
      {
        continue;
      }

      // Validate booking code
      BookingCode validatingBookingCode;
      if (!validateBookingCode(paxTypeFare, availabilityList, validatingBookingCode))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::BOOKINGCODE_FAIL);
        continue;
      }

      // Validate routing
      RoutingControllerESV shoppingRoutingController(*_trx);

      if (!shoppingRoutingController.validateRouting(_itin, paxTypeFare))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::ROUTING_FAIL);
        continue;
      }

      // Validate rule categories
      if (false == doRulesValidation(paxTypeFare))
      {
        continue;
      }

      if (false == outputResultsWithBFCat10Validation(paxTypeFare, validatedFares))
      {
        break;
      }

      // Break the loop if we find first one way and round trip fare which
      // pass both validations
      if ((nullptr != validatedFares[VDF_TAG_1_3_104]) &&
          (nullptr != validatedFares[VDF_TAG_1_2_102_104]) &&
          ((_fareMarketsCount > 2) || (nullptr != validatedFares[VDF_TAG_1_2_103])) &&
          ((_fareMarketsCount > 2) || (nullptr != validatedFares[VDF_TAG_2_104])))
      {
        break;
      }
    }
  }
  catch (...)
  {
    LOG4CXX_ERROR(
        logger,
        "FareValidatorESV::findFirstValidFares - Timeout reached for processing fare market.");
  }

  return;
}

namespace
{
bool
hasType(
    const std::set<std::pair<BookingCode, FareValidatorESV::ValidatedFareType> >& validatedTypes,
    const BookingCode& bkc,
    FareValidatorESV::ValidatedFareType vt)
{
  return (validatedTypes.find(std::make_pair(bkc, vt)) != validatedTypes.end());
}
}

void
FareValidatorESV::findFirstValidFares(LocalJourneyValidatedFares& validatedFares)
{
  TSELatencyData metrics((*_trx), "FVO FIND FIRST VALID FARES");

  LOG4CXX_DEBUG(logger,
                "FareValidatorESV::findFirstValidFaresLocalJourneyCxr(ValidatedPaxTypeFares&)");

  // Ensure that only one governing carrier thread work on specified fare
  // market at same time
  try
  {
    boost::unique_lock<boost::timed_mutex> g(_fareMarket->_esvMutex, boost::posix_time::milliseconds(2000));
    // Set correct travel segment vector in fare market
    if (!ShoppingUtil::setupFareMarketTravelSegESV(_fareMarket, _itin))
    {
      LOG4CXX_ERROR(logger, "FareValidatorESV::findFirstValidFare - Fare market travel segments "
                             "are not set correctly.");

      return;
    }

    // Get availability list
    std::vector<ClassOfServiceList> flowAvailabilityList;
    std::vector<ClassOfServiceList> localAvailabilityList;

    getFlowAvailability(flowAvailabilityList);
    getLocalAvailability(localAvailabilityList);

    // Get vector of fares for specified passenger type
    const std::vector<PaxTypeFare*>& paxTypeFareVec = getFaresForPaxType();

    PaxTypeFare* paxTypeFare = nullptr;

    ValidatedTypes validatedTypesLocal;
    ValidatedTypes validatedTypesFlow;

    // Go thorough all fares
    std::vector<PaxTypeFare*>::const_iterator paxTypeFareIter(paxTypeFareVec.begin()),
        paxTypeFareIterEnd(paxTypeFareVec.end());

    for (; paxTypeFareIter != paxTypeFareIterEnd; ++paxTypeFareIter)
    {
      paxTypeFare = (*paxTypeFareIter);

      // Check if fare object is not null
      if (nullptr == paxTypeFare)
      {
        LOG4CXX_ERROR(logger,
                      "FareValidatorESV::findFirstValidFare - PaxTypeFare object is NULL.");
        continue;
      }

      if (paxTypeFare->fare()->fareInfo()->getPaxType() == "JCB")
      {
        _fareMarket->setJcb(true);
        continue;
      }

      // Check if fare directionality and fare market direction are matched
      // correctly if not skip validation of this fare
      if (((_fareMarket->direction() == FMDirection::OUTBOUND) &&
           (paxTypeFare->directionality() == TO)) ||
          ((_fareMarket->direction() == FMDirection::INBOUND) &&
           (paxTypeFare->directionality() == FROM)))
      {
        continue;
      }

      // If shopping component validation was already performed for this fare
      // and fare is not valid skip processing for this fare
      if (paxTypeFare->shoppingComponentValidationFailed())
      {
        continue;
      }

      // Validate booking code
      BookingCode localVbkc, flowVbkc; // validating booking code
      bool localRes = validateBookingCode(paxTypeFare, localAvailabilityList, localVbkc);
      bool flowRes = validateBookingCode(paxTypeFare, flowAvailabilityList, flowVbkc);
      if (!localRes && !flowRes)
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::BOOKINGCODE_FAIL);
        continue;
      }

      // Validate routing
      RoutingControllerESV shoppingRoutingController(*_trx);

      if (!shoppingRoutingController.validateRouting(_itin, paxTypeFare))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::ROUTING_FAIL);
        continue;
      }

      // Validate rule categories
      if (!doRulesValidation(paxTypeFare))
      {
        continue;
      }

      if (localRes)
        outputResultsWithBFCat10Validation(
            paxTypeFare, localVbkc, validatedTypesLocal, validatedFares, false);
      if (flowRes)
        outputResultsWithBFCat10Validation(
            paxTypeFare, flowVbkc, validatedTypesFlow, validatedFares, true);
    }
  }
  catch (...)
  {
    LOG4CXX_ERROR(
        logger,
        "FareValidatorESV::findFirstValidFares - Timeout reached for processing fare market.");
  }

  return;
}

bool
FareValidatorESV::outputResultsWithBFCat10Validation(PaxTypeFare* paxTypeFare,
                                                     ValidatedFares& validatedFares)
{
  TSELatencyData metrics((*_trx), "FVO OUTPUT RESULTS WITH BF CAT 10 VALIDATION");

  LOG4CXX_DEBUG(
      logger,
      "FareValidatorESV::outputResultsWithBFCat10Validation(PaxTypeFare*, ValidatedFares&)");

  // Check if we have valid one way fares
  if ((nullptr == validatedFares[VDF_TAG_1_3_104]) &&
      ((paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) ||
       (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED)))
  {
    if (((1 == (*_trx).legs().size()) && (1 == _fareMarketsCount)) ||
        (false == paxTypeFare->fare()->fareInfo()->sameCarrier104()))
    {
      validatedFares[VDF_TAG_1_3_104] = paxTypeFare;
      paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
    }
    else if (true == paxTypeFare->isFlightValidESV(_flightId))
    {
      paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
    }

    if (nullptr == validatedFares[VDF_TAG_1_3])
    {
      validatedFares[VDF_TAG_1_3] = paxTypeFare;
      paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
    }
  }

  // If we have only one leg it means that it's one way trip and we
  // don't need to look for round trip fares
  if ((*_trx).legs().size() == 1)
  {
    // Break the loop if we find first one way fare which pass both
    // validations
    if (nullptr != validatedFares[VDF_TAG_1_3_104])
    {
      return false;
    }
  }
  else
  {
    // Check if we have valid round trip fare
    if ((nullptr == validatedFares[VDF_TAG_1_2_102_104]) &&
        ((paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
         (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)))
    {
      bool validRT = false;

      if (_fareMarketsCount > 1)
      {
        if (false == paxTypeFare->fare()->fareInfo()->sameCarrier104())
        {
          validRT = true;
        }
      }
      else
      {
        if (false == paxTypeFare->fare()->fareInfo()->sameCarrier102())
        {
          validRT = true;
        }
      }

      if (true == validRT)
      {
        validatedFares[VDF_TAG_1_2_102_104] = paxTypeFare;
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
      else if (true == paxTypeFare->isFlightValidESV(_flightId))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
      }

      if (nullptr == validatedFares[VDF_TAG_1_2])
      {
        validatedFares[VDF_TAG_1_2] = paxTypeFare;
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
    }

    // Check if we have valid circle trip fare
    if ((_fareMarketsCount < 3) && (nullptr == validatedFares[VDF_TAG_1_2_103]) &&
        ((paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
         (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)))
    {
      if (false == paxTypeFare->fare()->fareInfo()->sameCarrier103())
      {
        validatedFares[VDF_TAG_1_2_103] = paxTypeFare;
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
      else if (true == paxTypeFare->isFlightValidESV(_flightId))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
      }

      if (nullptr == validatedFares[VDF_TAG_1_2])
      {
        validatedFares[VDF_TAG_1_2] = paxTypeFare;
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
    }

    // Check if we have valid open jaw fare
    if ((_fareMarketsCount < 3) && (nullptr == validatedFares[VDF_TAG_2_104]) &&
        (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED))
    {
      if (false == paxTypeFare->fare()->fareInfo()->sameCarrier104())
      {
        validatedFares[VDF_TAG_2_104] = paxTypeFare;
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
      else if (true == paxTypeFare->isFlightValidESV(_flightId))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
      }

      if (nullptr == validatedFares[VDF_TAG_2])
      {
        validatedFares[VDF_TAG_2] = paxTypeFare;
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
    }
  }

  return true;
}

bool
FareValidatorESV::outputResultsWithBFCat10Validation(PaxTypeFare* paxTypeFare,
                                                     const BookingCode& bookingCode,
                                                     ValidatedTypes& validatedTypes,
                                                     LocalJourneyValidatedFares& validatedFares,
                                                     bool isFlow)
{
  TSELatencyData metrics((*_trx), "FVO OUTPUT RESULTS WITH BF CAT 10 VALIDATION");

  // LOG4CXX_DEBUG(logger, "FareValidatorESV::outputResultsWithBFCat10Validation(PaxTypeFare*,
  // ValidatedTypes&, ValidatedPaxTypeFares&)");

  // Check if we have valid one way fares
  if (!hasType(validatedTypes, bookingCode, VDF_TAG_1_3_104) &&
      ((paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) ||
       (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED)))
  {
    if (((1 == (*_trx).legs().size()) && (1 == _fareMarketsCount)) ||
        (!paxTypeFare->fare()->fareInfo()->sameCarrier104()))
    {
      validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_1_3_104));
      validatedFares[VDF_TAG_1_3_104]->push_back(
          ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
      paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
    }
    else if (paxTypeFare->isFlightValidESV(_flightId))
    {
      paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
    }

    if (!hasType(validatedTypes, bookingCode, VDF_TAG_1_3))
    {
      validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_1_3));
      validatedFares[VDF_TAG_1_3]->push_back(
          ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
      paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
    }
  }

  // If we have only one leg it means that it's one way trip and we
  // don't need to look for round trip fares
  if ((*_trx).legs().size() == 1)
  {
    // Break the loop if we find first one way fare which pass both
    // validations
    if (hasType(validatedTypes, bookingCode, VDF_TAG_1_3_104))
    {
      return false;
    }
  }
  else
  {
    // Check if we have valid round trip fare
    if (!hasType(validatedTypes, bookingCode, VDF_TAG_1_2_102_104) &&
        ((paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
         (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)))
    {
      bool validRT = false;

      if (_fareMarketsCount > 1)
      {
        if (!paxTypeFare->fare()->fareInfo()->sameCarrier104())
        {
          validRT = true;
        }
      }
      else
      {
        if (!paxTypeFare->fare()->fareInfo()->sameCarrier102())
        {
          validRT = true;
        }
      }

      if (validRT)
      {
        validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_1_2_102_104));
        validatedFares[VDF_TAG_1_2_102_104]->push_back(
            ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
      else if (paxTypeFare->isFlightValidESV(_flightId))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
      }

      if (!hasType(validatedTypes, bookingCode, VDF_TAG_1_2))
      {
        validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_1_2));
        validatedFares[VDF_TAG_1_2]->push_back(
            ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
    }

    // Check if we have valid circle trip fare
    if ((_fareMarketsCount < 3) && !hasType(validatedTypes, bookingCode, VDF_TAG_1_2_103) &&
        ((paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
         (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)))
    {
      if (!paxTypeFare->fare()->fareInfo()->sameCarrier103())
      {
        validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_1_2_103));
        validatedFares[VDF_TAG_1_2_103]->push_back(
            ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
      else if (paxTypeFare->isFlightValidESV(_flightId))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
      }

      if (!hasType(validatedTypes, bookingCode, VDF_TAG_1_2))
      {
        validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_1_2));
        validatedFares[VDF_TAG_1_2]->push_back(
            ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
    }

    // Check if we have valid open jaw fare
    if ((_fareMarketsCount < 3) && !hasType(validatedTypes, bookingCode, VDF_TAG_2_104) &&
        (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED))
    {
      if (!paxTypeFare->fare()->fareInfo()->sameCarrier104())
      {
        validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_2_104));
        validatedFares[VDF_TAG_2_104]->push_back(
            ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
      else if (paxTypeFare->isFlightValidESV(_flightId))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT_10_RESTRICTIONS);
      }

      if (!hasType(validatedTypes, bookingCode, VDF_TAG_2))
      {
        validatedTypes.insert(std::make_pair(bookingCode, VDF_TAG_2));
        validatedFares[VDF_TAG_2]->push_back(
            ValidatedPTF::create(_trx->dataHandle(), bookingCode, paxTypeFare, isFlow));
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::PASSED);
      }
    }
  }

  return true;
}

bool
FareValidatorESV::doRulesValidation(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics((*_trx), "FVO DO RULES VALIDATION");

  LOG4CXX_DEBUG(logger, "FareValidatorESV::doRulesValidation(PaxTypeFare*)");

  bool skipSCV = false;

  // If shopping component validation was already performed for this fare
  // and fare is valid do not repeat shopping component validation
  if ((true == paxTypeFare->shoppingComponentValidationPerformed()) &&
      (false == paxTypeFare->shoppingComponentValidationFailed()))
  {
    skipSCV = true;
  }

  // Mark that shopping component validation was executed for this fare
  paxTypeFare->shoppingComponentValidationPerformed() = true;

  std::vector<uint32_t>::iterator ruleIter;

  for (ruleIter = _rule_validation_order->begin(); ruleIter != _rule_validation_order->end();
       ++ruleIter)
  {
    uint32_t ruleNumber = (*ruleIter);

    switch (ruleNumber)
    {
    case CAT_1:
      if ((false == skipSCV) && (false == validateCat1(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_2:
      if ((false == skipSCV) && (false == validateCat2(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_2FR:
      if (false == validateCat2FR(paxTypeFare))
      {
        return false;
      }
      break;

    case CAT_3:
      if ((false == skipSCV) && (false == validateCat3(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_4FR:
      if (false == validateCat4FR(paxTypeFare))
      {
        return false;
      }
      break;

    case CAT_4Q:
      if (false == validateCat4Q(paxTypeFare))
      {
        return false;
      }
      break;

    case CAT_5:
      if ((false == skipSCV) && (false == validateCat5(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_6:
      if ((false == skipSCV) && (false == validateCat6(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_7:
      if ((false == skipSCV) && (false == validateCat7(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_8FR:
      if (false == validateCat8FR(paxTypeFare))
      {
        return false;
      }
      break;

    case CAT_9FR:
      if (false == validateCat9FR(paxTypeFare))
      {
        return false;
      }
      break;

    case CAT_11:
      if ((false == skipSCV) && (false == validateCat11(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_14:
      if ((false == skipSCV) && (false == validateCat14(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_14FR:
      if (false == validateCat14FR(paxTypeFare))
      {
        return false;
      }
      break;

    case CAT_15S:
      if ((false == skipSCV) && (false == validateCat15S(paxTypeFare)))
      {
        return false;
      }
      break;

    case CAT_15:
      if ((false == skipSCV) && (false == validateCat15(paxTypeFare)))
      {
        return false;
      }
      break;

    default:
      LOG4CXX_ERROR(logger, "FareValidatorESV::doRulesValidation - Unsupported rule category");
      break;
    };
  }

  return true;
}

void
FareValidatorESV::getFaresForPaxType(std::vector<PaxTypeFare*>& paxTypeFareVecRet)
{
  TSELatencyData metrics((*_trx), "FVO GET FARES FOR PAX TYPE");

  LOG4CXX_DEBUG(logger, "FareValidatorESV::getFaresForPaxType(std::vector<PaxTypeFare*>&)");

  std::vector<PaxTypeBucket>::iterator paxTypeCortegeIter;

  for (paxTypeCortegeIter = _fareMarket->paxTypeCortege().begin();
       paxTypeCortegeIter != _fareMarket->paxTypeCortege().end();
       paxTypeCortegeIter++)
  {
    PaxTypeBucket& paxTypeCortege = *paxTypeCortegeIter;

    if (paxTypeCortege.requestedPaxType()->paxType() == _paxType->paxType())
    {
      paxTypeFareVecRet = paxTypeCortege.paxTypeFare();
      break;
    }
  }
}

const std::vector<PaxTypeFare*>&
FareValidatorESV::getFaresForPaxType() const
{
  static std::vector<PaxTypeFare*> empty;
  TSELatencyData metrics(*_trx, "FVO GET FARES FOR PAX TYPE");
  LOG4CXX_DEBUG(logger, "FareValidatorESV::getFaresForPaxType(std::vector<PaxTypeFare *> &)");
  const std::vector<PaxTypeBucket>& cortegeVector = _fareMarket->paxTypeCortege();
  std::vector<PaxTypeBucket>::const_iterator paxTypeCortegeIter(cortegeVector.begin()),
      paxTypeCortegeIterEnd(cortegeVector.end());
  for (; paxTypeCortegeIter != paxTypeCortegeIterEnd; ++paxTypeCortegeIter)
  {
    const PaxTypeBucket& paxTypeCortege = *paxTypeCortegeIter;
    if (paxTypeCortege.requestedPaxType()->paxType() == _paxType->paxType())
    {
      return paxTypeCortege.paxTypeFare();
    }
  }
  return empty;
}

bool
FareValidatorESV::validateBookingCode(
    PaxTypeFare* paxTypeFare,
    std::vector<ClassOfServiceList>& availabilityList,
    BookingCode& validatingBookingCode)
{
  TSELatencyData metrics((*_trx), "FVO VALIDATE BOOKING CODE");

  LOG4CXX_DEBUG(logger, "FareValidatorESV::validateBookingCode(PaxTypeFare*)");

  std::vector<BookingCode> bookingCodes;
  std::vector<BookingCode>* bookingCodesPtr = &bookingCodes;
  const std::vector<BookingCode>* bookingCodesBF =
      paxTypeFare->fare()->fareInfo()->getBookingCodes(*_trx);

  if ((nullptr == bookingCodesBF) || (bookingCodesBF->empty()))
  {
    const FareClassCode& fareClass = paxTypeFare->fare()->fareInfo()->fareClass();
    if (!fareClass.empty())
    {
      bookingCodesPtr->push_back(BookingCode(fareClass[0]));
    }
  }
  else
  {
    bookingCodesPtr = (std::vector<BookingCode>*)bookingCodesBF;
  }

  bool bValid = false;
  PaxTypeFare::SegmentStatusVec segmentStatusVec;

  if (availabilityList.size() == _fareMarket->travelSeg().size())
  {
    const std::vector<TravelSeg*>& travelSegVector = _fareMarket->travelSeg();
    segmentStatusVec.resize(travelSegVector.size());

    if (true == checkAvailability(segmentStatusVec,
                                  travelSegVector,
                                  availabilityList,
                                  bookingCodesPtr,
                                  validatingBookingCode))
    {
      bValid = true;
    }
  }
  else
  {
    LOG4CXX_ERROR(logger, "FareValidatorESV::validateBookingCode - Availability map size is not "
                           "equal to travel segments size.");
    bValid = false;
  }

  if (bValid)
  {
    TSELatencyData metrics((*_trx), "PASS FVO VALIDATE BOOKING CODE");
    paxTypeFare->flightBitmapESV()[_flightId]._segmentStatus.swap(segmentStatusVec);
  }
  else
  {
    TSELatencyData metrics((*_trx), "FAIL FVO VALIDATE BOOKING CODE");
  }

  return bValid;
}

bool
FareValidatorESV::checkAvailability(PaxTypeFare::SegmentStatusVec& segmentStatusVec,
                                    const std::vector<TravelSeg*>& travelSegVector,
                                    std::vector<ClassOfServiceList>& availabilityList,
                                    std::vector<BookingCode>* bookingCodesPtr,
                                    BookingCode& validatingBookingCode)
{
  bool bValid = true;

  std::vector<TravelSeg*>::const_iterator segIter(travelSegVector.begin()),
      segIterEnd(travelSegVector.end());
  int idx(0);
  for (; segIter != segIterEnd; ++segIter, ++idx)
  {
    bool bCodeFound = false;
    PaxTypeFare::SegmentStatus& segStat = segmentStatusVec[idx];

    std::vector<BookingCode>::const_iterator bkkIter(bookingCodesPtr->begin()),
        bkkIterEnd(bookingCodesPtr->end());

    for (; bkkIter != bkkIterEnd; ++bkkIter)
    {
      const BookingCode& bookingCode = (*bkkIter);

      int index = ((int)(bookingCode[0])) - 65;
      ClassOfService* cos = ((availabilityList[idx])[index]);

      if (nullptr != cos)
      {
        bCodeFound = true;
        validatingBookingCode = bookingCode;

        // Update segment status
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
        segStat._bkgCodeReBook = bookingCode;
        segStat._reBookCabin = cos->cabin();

        break;
      }
    }

    if (!bCodeFound)
    {
      bValid = false;
      break;
    }
  }

  return bValid;
}

void
FareValidatorESV::getFlowAvailability(
    std::vector<ClassOfServiceList>& availabilityList)
{
  TSELatencyData metrics((*_trx), "FVO GET FLOW AVAILABILITY");

  LOG4CXX_DEBUG(
      logger,
      "FareValidatorESV::getFlowAvailability(std::vector<ClassOfServiceList>&)");

  ShoppingTrx::ClassOfServiceKey cosKey;

  bool startFm = false;
  bool endFm = false;
  bool getAvailability = false;

  int segId = 0;
  int startId = 0;
  int endId = 0;

  std::vector<TravelSeg*>::iterator segIter;

  for (segIter = _itin->travelSeg().begin(); segIter != _itin->travelSeg().end(); ++segIter)
  {
    TravelSeg* travelSeg = (*segIter);
    const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*(travelSeg));

    if (airSegment.carrier() == _fareMarket->governingCarrier())
    {
      getAvailability = true;

      if (travelSeg->origin()->loc() == _fareMarket->origin()->loc())
      {
        startFm = true;
        startId = segId;
      }

      if (travelSeg->destination()->loc() == _fareMarket->destination()->loc())
      {
        endFm = true;
        endId = segId;
      }

      cosKey.push_back(travelSeg);
      ++segId;
    }
    else
    {
      if ((true == startFm) && (true == endFm))
      {
        break;
      }
      else if (true == getAvailability)
      {
        cosKey.clear();
        getAvailability = false;
        segId = 0;
      }
    }
  }

  AvailabilityMap::const_iterator availMapIter;

  availMapIter = _trx->availabilityMap().find(ShoppingUtil::buildAvlKey(cosKey));

  if (availMapIter != _trx->availabilityMap().end())
  {
    int cosListVecId = 0;

    std::vector<ClassOfServiceList>::const_iterator cosListVecIter;

    for (cosListVecIter = ((availMapIter->second)->begin());
         cosListVecIter != ((availMapIter->second)->end());
         ++cosListVecIter, ++cosListVecId)
    {
      if ((cosListVecId >= startId) && (cosListVecId <= endId))
      {
        availabilityList.push_back(*cosListVecIter);
      }
    }
  }
  else
  {
    LOG4CXX_ERROR(logger,
                  "FareValidatorESV::getFlowAvailability - Availability not found in map.");
  }
}

void
FareValidatorESV::getLocalAvailability(
    std::vector<ClassOfServiceList>& availabilityList)
{
  TSELatencyData metrics((*_trx), "FVO GET LOCAL AVAILABILITY");

  LOG4CXX_DEBUG(
      logger,
      "FareValidatorESV::getLocalAvailability(std::vector<ClassOfServiceList>&)");

  std::vector<TravelSeg*>::iterator segIter;

  for (segIter = _fareMarket->travelSeg().begin(); segIter != _fareMarket->travelSeg().end();
       ++segIter)
  {
    TravelSeg* travelSeg = (*segIter);

    ShoppingTrx::ClassOfServiceKey cosKey;
    cosKey.push_back(travelSeg);

    AvailabilityMap::const_iterator availMapIter;

    availMapIter = _trx->availabilityMap().find(ShoppingUtil::buildAvlKey(cosKey));

    if (availMapIter != _trx->availabilityMap().end())
    {
      availabilityList.push_back(((availMapIter->second)->at(0)));
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "FareValidatorESV::getLocalAvailability - Availability not found in map.");
    }
  }
}

bool
FareValidatorESV::skipFareIfCat10NotValid(PaxTypeFare* paxTypeFare, ValidatedFares& validatedFares)
{
  TSELatencyData metrics((*_trx), "FVO SKIP FARE IF CAT 10 NOT VALID");

  LOG4CXX_DEBUG(logger,
                "FareValidatorESV::skipFareIfCat10NotValid(PaxTypeFare*, ValidatedFares&)");

  if ((nullptr == validatedFares[VDF_TAG_1_3_104]) &&
      ((paxTypeFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) ||
       (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED)))
  {
    if (nullptr == validatedFares[VDF_TAG_1_3])
    {
      return false;
    }

    if (((1 == (*_trx).legs().size()) && (1 == _fareMarketsCount)) ||
        (false == paxTypeFare->fare()->fareInfo()->sameCarrier104()))
    {
      return false;
    }
  }

  if (2 == (*_trx).legs().size())
  {
    if ((nullptr == validatedFares[VDF_TAG_1_2_102_104]) &&
        ((paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
         (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)))
    {
      if (nullptr == validatedFares[VDF_TAG_1_2])
      {
        return false;
      }

      bool validRT = false;

      if (_fareMarketsCount > 1)
      {
        if (false == paxTypeFare->fare()->fareInfo()->sameCarrier104())
        {
          validRT = true;
        }
      }
      else
      {
        if (false == paxTypeFare->fare()->fareInfo()->sameCarrier102())
        {
          validRT = true;
        }
      }

      if (true == validRT)
      {
        return false;
      }
    }

    if ((_fareMarketsCount < 3) && (nullptr == validatedFares[VDF_TAG_1_2_103]) &&
        ((paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED) ||
         (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)))
    {
      if (nullptr == validatedFares[VDF_TAG_1_2])
      {
        return false;
      }

      if (false == paxTypeFare->fare()->fareInfo()->sameCarrier103())
      {
        return false;
      }
    }

    if ((_fareMarketsCount < 3) && (nullptr == validatedFares[VDF_TAG_2_104]) &&
        (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED))
    {
      if (nullptr == validatedFares[VDF_TAG_2])
      {
        return false;
      }

      if (false == paxTypeFare->fare()->fareInfo()->sameCarrier104())
      {
        return false;
      }
    }
  }

  return true;
}

void
FareValidatorESV::getCustomAvailability(
    std::vector<ClassOfServiceList>& availabilityList)
{
  TSELatencyData metrics((*_trx), "FVO GET CUSTOM AVAILABILITY");

  LOG4CXX_DEBUG(
      logger,
      "FareValidatorESV::getCustomAvailability(std::vector<ClassOfServiceList>&)");

  AvailabilityMap::const_iterator availMapIter;

  availMapIter = _trx->availabilityMap().find(ShoppingUtil::buildAvlKey(_fareMarket->travelSeg()));

  if (availMapIter != _trx->availabilityMap().end())
  {
    availabilityList = *(availMapIter->second);
  }
  else
  {
    LOG4CXX_ERROR(logger,
                  "FareValidatorESV::getCustomAvailability - Availability not found in map.");
  }
}

bool
FareValidatorESV::validateCat1(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 1");

  _ruleControllersESV->preValidationRC.categorySequence()[0] = 1;
  _ruleControllersESV->preValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 1");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 1");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat15S(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 15S");

  _ruleControllersESV->preValidationRC.categorySequence()[0] = 15;
  _ruleControllersESV->preValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid() && paxTypeFare->isCat15SecurityValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 15S");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 15S");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat2(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 2");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 2;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 2");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 2");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat3(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 3");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 3;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 3");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 3");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat5(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 5");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 5;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 5");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 5");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat6(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 6");

  bool pass = false;

  if ((_fareMarket->direction() != FMDirection::OUTBOUND) ||
      (paxTypeFare->owrt() != ROUND_TRIP_MAYNOT_BE_HALVED))
  {
    pass = true;
  }
  else
  {
    _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 6;
    _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

    pass = paxTypeFare->areAllCategoryValid();
  }

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 6");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 6");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat7(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 7");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 7;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 7");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 7");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat11(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 11");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 11;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 11");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 11");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat14(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 14");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 14;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 14");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 14");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat15(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 15");

  _ruleControllersESV->shoppingComponentValidationRC.categorySequence()[0] = 15;
  _ruleControllersESV->shoppingComponentValidationRC.validate(*_trx, *_itin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 15");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 15");
    paxTypeFare->shoppingComponentValidationFailed() = true;
  }

  return pass;
}

bool
FareValidatorESV::validateCat2FR(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 2FR");

  paxTypeFare->setCategoryProcessed(RuleConst::DAY_TIME_RULE, false);

  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.categorySequence()[0] = 2;
  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.validate(
      *_trx, *_journeyItin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 2FR");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 2FR");
    paxTypeFare->resetRuleStatusESV();
    paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT2_FAIL);
  }

  return pass;
}

bool
FareValidatorESV::validateCat4FR(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 4FR");

  paxTypeFare->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, false);

  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.categorySequence()[0] = 4;
  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.validate(
      *_trx, *_journeyItin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 4FR");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 4FR");
    paxTypeFare->resetRuleStatusESV();
    paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT4_FAIL);
  }

  return pass;
}

bool
FareValidatorESV::validateCat8FR(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 8FR");

  paxTypeFare->setCategoryProcessed(RuleConst::STOPOVER_RULE, false);

  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.categorySequence()[0] = 8;
  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.validate(
      *_trx, *_journeyItin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 8FR");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 8FR");
    paxTypeFare->resetRuleStatusESV();
    paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT8_FAIL);
  }

  return pass;
}

bool
FareValidatorESV::validateCat9FR(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 9FR");

  paxTypeFare->setCategoryProcessed(RuleConst::TRANSFER_RULE, false);

  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.categorySequence()[0] = 9;
  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.validate(
      *_trx, *_journeyItin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 9FR");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 9FR");
    paxTypeFare->resetRuleStatusESV();
    paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT9_FAIL);
  }

  return pass;
}

bool
FareValidatorESV::validateCat14FR(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 14FR");

  paxTypeFare->setCategoryProcessed(RuleConst::TRAVEL_RESTRICTIONS_RULE, false);

  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.categorySequence()[0] = 14;
  _ruleControllersESV->shoppingComponentWithFlightsValidationRC.validate(
      *_trx, *_journeyItin, *paxTypeFare);

  bool pass = paxTypeFare->areAllCategoryValid();

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 14FR");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 14FR");
    paxTypeFare->resetRuleStatusESV();
    paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::CAT14_FAIL);
  }

  return pass;
}

bool
FareValidatorESV::validateCat4Q(PaxTypeFare* paxTypeFare)
{
  TSELatencyData metrics(*_trx, "FVO VALIDATE CAT 4Q");

  typedef VecMap<uint32_t, QualifyFltAppRuleData*> QMap;

  QMap::const_iterator iter = paxTypeFare->qualifyFltAppRuleDataMap().begin();
  QMap::const_iterator iterEnd = paxTypeFare->qualifyFltAppRuleDataMap().end();

  if (iter != iterEnd)
  {
    while (iter != iterEnd)
    {
      paxTypeFare->setCategoryProcessed(iter->first, false);
      ++iter;
    }

    _ruleControllersESV->shoppingComponentValidateQualifiedCat4RC.categorySequence()[0] = 4;
    _ruleControllersESV->shoppingComponentValidateQualifiedCat4RC.validate(
        *_trx, *_journeyItin, *paxTypeFare);

    iter = paxTypeFare->qualifyFltAppRuleDataMap().begin();

    while (iter != iterEnd)
    {
      if (!paxTypeFare->isCategoryValid(iter->first))
      {
        paxTypeFare->setFlightInvalidESV(_flightId, RuleConst::QUALIFYCAT4_FAIL);
        paxTypeFare->setCategoryValid(iter->first);
      }

      iter++;
    }
  }

  bool pass = paxTypeFare->isFlightValidESV(_flightId);

  if (true == pass)
  {
    TSELatencyData metrics(*_trx, "PASS FVO VALIDATE CAT 4Q");
  }
  else
  {
    TSELatencyData metrics(*_trx, "FAIL FVO VALIDATE CAT 4Q");
  }

  return pass;
}

} // ns tse

