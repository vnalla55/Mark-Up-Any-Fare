//-------------------------------------------------------------------
//
//  File:        PrivateIndicator.cpp
//  Created:     April 7, 2005
//  Authors:     Tony Lam
//  Description: This class will process private indicator for all
//               private fares.  There are 4 private indicators
//               - PILLOW
//               - SLASH (/)
//               - X
//               - STAR (*)
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "DataModel/PrivateIndicator.h"

#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"

using namespace std;

namespace tse
{
// this value defines the mode date not in last 24 hours Indicator
const string PrivateIndicator::BLANK_INDICATOR = " ";
// this value defines General Private Fare filed without Account Code
const string PrivateIndicator::PILLOW = "@";
// this value defines General Private Fare filed via Cat 35
const string PrivateIndicator::SLASH = "/";
// this value defines General Private Fare filed via Cat 35 ticketing ineligible
const string PrivateIndicator::TICKETING_INELIGIBLE = "X";
// this value defines General Private Fare filed with Account Code
const string PrivateIndicator::STAR = "*";

const Indicator PrivateIndicator::_indicators[NUM_IND_TYPE] = {
  IND_NOTPRIVATE, IND_CORPID, IND_CAT35, IND_XTKTC35, IND_PRIVATE
};
const std::string PrivateIndicator::_indStr[NUM_IND_TYPE] = {
  "", "CORPID  *", "CAT35   /", "XTKTC35 X", "PRIVATE @"
};

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  PrivateIndicator::privateIndicator()
//
// The main processing method.  Called to determine the privateIndicator
//
// @param   ptFare              - the object that contains all of the
//                                applicable data to be formatted.
//
// @param   string              - output, tells caller how many lines
//                                were formatted.
//
// @param   setToBlank          - default value for Private Indctaor
//
// @return  none
//
// </PRE>
// -------------------------------------------------------------------------

void
PrivateIndicator::privateIndicatorOld(const PaxTypeFare& ptFare,
                                   std::string& privateInd,
                                   bool setToBlank,
                                   bool isFQ)
{
  uint16_t privateIndSeq = getPrivateFareIndicatorOld(ptFare, isFQ);
  if (privateIndSeq == NotPrivate)
  {
    if (UNLIKELY(setToBlank))
      privateInd = BLANK_INDICATOR;
  }
  else
    privateInd = privateFareIndicator(privateIndSeq);
}

void
PrivateIndicator::privateIndicator(const PaxTypeFare& ptFare,
                                   std::string& privateInd,
                                   bool setToBlank,
                                   bool isFQ)
{
  uint16_t privateIndSeq = getPrivateFareIndicator(ptFare, isFQ);
  if (privateIndSeq == NotPrivate)
  {
    if (UNLIKELY(setToBlank))
      privateInd = BLANK_INDICATOR;
  }
  else
    privateInd = privateFareIndicator(privateIndSeq);
}

uint16_t
PrivateIndicator::getPrivateFareIndicatorOld(const PaxTypeFare& ptFare, bool isFQ)
{
  if (ptFare.tcrTariffCat() != 1 /*RuleConst::PRIVATE_TARIFF*/)
    return NotPrivate;

  // General Private Fare filed without Account Code or Corporate ID
  FBRPaxTypeFareRuleData* fbrData = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  const NegPaxTypeFareRuleData* negPTFareRuleData = ptFare.getNegRuleData();
  // Check Ticketing ineligible CAT 35
  if (negPTFareRuleData && negPTFareRuleData->tktIndicator() == 'N')
  {
    return XTktC35;
  }

  // Any private fare filed with a Corporate ID or an Account Code
  // Can be a Cat 15/25/35
  if ((ptFare.matchedCorpID() ||
       (fbrData != nullptr && !fbrData->fbrApp()->accountCode().empty())) &&
      (ptFare.isCat15SecurityValid() || ptFare.isCategoryValid(RuleConst::FARE_BY_RULE) ||
       ptFare.isCategoryValid(RuleConst::NEGOTIATED_RULE)))
  {
    return CorpIDSeq;
  }

  // Check Cat35 fare - if not updatable (==L or ==T), always pillow
  if (ptFare.fcaDisplayCatType() != 'C')
  {
    return Private;
  }
  if (!ptFare.matchedCorpID() || (fbrData != nullptr && fbrData->fbrApp()->accountCode().empty()))
  {
    if ((!isFQ && ptFare.fcaDisplayCatType() == 'C') ||
        (isFQ && ptFare.fareDisplayCat35Type() == 'M'))
      return Cat35Seq;
    else
      return Private;
  }
  return Private;
}

uint16_t
PrivateIndicator::getPrivateFareIndicator(const PaxTypeFare& ptFare, bool isFQ)
{
  if (ptFare.tcrTariffCat() != 1 /*RuleConst::PRIVATE_TARIFF*/)
    return NotPrivate;

  // General Private Fare filed without Account Code or Corporate ID
  FBRPaxTypeFareRuleData* fbrData = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  const NegPaxTypeFareRuleData* negPTFareRuleData = ptFare.getNegRuleData();
  if (negPTFareRuleData && negPTFareRuleData->tktIndicator() == 'N')
  {
    return XTktC35;
  }

  if (ptFare.fcaDisplayCatType() == 'C' || ptFare.fareDisplayCat35Type() == 'M')
    return Cat35Seq;

  // Any private fare filed with a Corporate ID or an Account Code
  // Can be a Cat 15/25/35
  if ((ptFare.matchedCorpID() ||
       (fbrData != nullptr && !fbrData->fbrApp()->accountCode().empty())) &&
      (ptFare.isCat15SecurityValid() || ptFare.isCategoryValid(RuleConst::FARE_BY_RULE) ||
       ptFare.isCategoryValid(RuleConst::NEGOTIATED_RULE)))
  {
    return CorpIDSeq;
  }

  // Check Cat35 fare - if not updatable (==L or ==T), always pillow
  if (ptFare.fcaDisplayCatType() != 'C')
  {
    return Private;
  }

  return Private;
}

void
PrivateIndicator::resolvePrivateFareIndOld(uint16_t& targetIndSeq, uint16_t nextIndSeq)
{
  if (nextIndSeq == NotPrivate || targetIndSeq == nextIndSeq)
    return;

  if (targetIndSeq == NotPrivate)
  {
    targetIndSeq = nextIndSeq;
    return;
  }

  targetIndSeq = Private;
}

void
PrivateIndicator::resolvePrivateFareInd(uint16_t& targetIndSeq, uint16_t nextIndSeq)
{
  if (targetIndSeq == NotPrivate)
  {
    targetIndSeq = nextIndSeq;
    return;
  }
  else if (nextIndSeq == NotPrivate)
    return;
  else if (nextIndSeq == CorpIDSeq)
    return;
  else if (nextIndSeq == Cat35Seq)
  {
    if (targetIndSeq != XTktC35)
    {
      targetIndSeq = Cat35Seq;
      return;
    }
    else
      return;
  }
  else if (nextIndSeq == XTktC35)
  {
    targetIndSeq = XTktC35;
    return;
  }
  else if (nextIndSeq == Private)
  {
    if (targetIndSeq == CorpIDSeq)
    {
      targetIndSeq = Private;
      return;
    }
    else
      return;
  }

  targetIndSeq = Private;
}
} // tse namespace
