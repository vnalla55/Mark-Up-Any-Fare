//-------------------------------------------------------------------
//
//  File:        FBDisplay.cpp
//  Created:     Apr 04, 2005
//  Design:      Partha Kumar Chakraborti
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
//-------------------------------------------------------------------

#ifndef FB_DISPLAY_CPP
#define FB_DISPLAY_CPP

#include "DataModel/FBDisplay.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FBCategoryRuleRecord.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"

using namespace tse;

static Logger
logger("atseintl.FareDisplay.FBDisplay");

namespace
{
static const char FB_CAT_AVAILABILITY_FARE_RULE = '*';
static const char FB_CAT_AVAILABILITY_GENERAL_RULE = '$';
static const char FB_CAT_AVAILABILITY_FOOTNOTE = '-';
static const char FB_CAT_AVAILABILITY_2_OR_MORE = '/';
static const CatNumber COMBINABILITY_RULE = 10;
static const CatNumber FARE_BY_RULE = 25;
}
// -----------------------------------------------------------
// @MethodName  FBDisplay::getCategoryApplicability()
//
// Description: .
//
// -----------------------------------------------------------

const char
FBDisplay::getCategoryApplicability(const bool isDutyCode7Or8, const int16_t cat) const
{
  // Search for the CAT in existing Map. If not found then create new.
  FBCategoryRuleRecord* fbCRR = getRuleRecordData(cat);
  if (!fbCRR)
    return NO_PARAM;

  bool isFareRule = fbCRR->hasFareRuleRecord2Info();
  bool isFootNote = fbCRR->hasFootNoteRecord2Info();
  bool isGeneral = fbCRR->hasGeneralRuleRecord2Info();

  // fbr (and cat10?) always treated like fare rule
  // TODO: cat10 fare/gen rule issue needs attention
  isFareRule |= (cat == FARE_BY_RULE && fbCRR->hasFareByRuleRecord2Info());
  isFareRule |= (cat == COMBINABILITY_RULE && fbCRR->hasCombinabilityRecord2Info());

  /* Remove this check (SPR 85423)
  if ( !isDutyCode7Or8 )
  {
      if ( isFareRule || isFootNote || isGeneral )
          return ALL_TYPE ; // return *

      return NO_PARAM ; // return ' '
  }*/

  // Fare = empty, Footnote = empty, General = empty ->Return ' '
  if (!isFareRule && !isFootNote && !isGeneral)
    return NO_PARAM;

  // Fare = yes, Footnote=empty, General=empty -> Return '*'
  if (isFareRule && !isFootNote && !isGeneral)
    return FB_CAT_AVAILABILITY_FARE_RULE;

  // Fare = empty, Footnote = empty, General = yes -> Return '$'
  if (!isFareRule && !isFootNote && isGeneral)
    return FB_CAT_AVAILABILITY_GENERAL_RULE;

  // Fare = empty, Footnote = yes, General = empty ->Return '-'
  if (!isFareRule && isFootNote && !isGeneral)
    return FB_CAT_AVAILABILITY_FOOTNOTE;

  // 2 or more has data -> Return '/'
  return FB_CAT_AVAILABILITY_2_OR_MORE;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::setRuleData()
//
// Description: Sets data based on category.
//
// -----------------------------------------------------------
void
FBDisplay::setRuleRecordData(CatNumber cat, FBCategoryRuleRecord* rd, bool useBaseFareRule)
{
  if (useBaseFareRule)
    _baseFarefbCategoryRuleRecordMap[cat] = rd;
  else
    _fbCategoryRuleRecordMap[cat] = rd;
}
// -----------------------------------------------------------
// @MethodName  FBDisplay::setRuleRecordData()
//
// Description: Sets data based on category.
//
// -----------------------------------------------------------
void
FBDisplay::setRuleRecordData(CatNumber cat, FBCategoryRuleRecord* rd)
{
  _fbCategoryRuleRecordMap[cat] = rd;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::setRuleRecordData()
//
// Description: Copies CategorRuleRecord of one paxTypeFare ot another
// We use this method whn Record2 info used and validated for base fare but it was actually
// filed for a different fare (Cat25 override). In such a situaltion cat25 fare need to point
// to same rules as of base fare.

// -----------------------------------------------------------
void
FBDisplay::setRuleRecordData(CatNumber cat,
                             PricingTrx& trx,
                             const PaxTypeFare& srcPaxTypeFare,
                             const PaxTypeFare& destPaxTypeFare,
                             bool useBaseFareRule)
{
  FareDisplayTrx* fdTrx(nullptr);
  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fdTrx))
    return;

  const FareDisplayInfo* srcFareDisplayInfo = srcPaxTypeFare.fareDisplayInfo();
  if (!srcFareDisplayInfo)
    return;
  FBCategoryRuleRecord* srcCRR = srcFareDisplayInfo->fbDisplay().getRuleRecordData(cat);
  if (!srcCRR)
    return;

  FareDisplayInfo* destFareDisplayInfo =
      const_cast<FareDisplayInfo*>(destPaxTypeFare.fareDisplayInfo());
  if (!destFareDisplayInfo)
    return;

  destFareDisplayInfo->fbDisplay().setRuleRecordData(cat, srcCRR, useBaseFareRule);
  return;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::getRuleData()
//
// Description: Get data based on category.
//
// -----------------------------------------------------------

FBCategoryRuleRecord*
FBDisplay::getRuleRecordData(CatNumber cat) const
{
  std::map<CatNumber, FBCategoryRuleRecord*>::const_iterator i = _fbCategoryRuleRecordMap.find(cat);

  if (i == _fbCategoryRuleRecordMap.end())
    return nullptr;
  else
    return i->second;
}
// -----------------------------------------------------------
// @MethodName  FBDisplay::getBaseFareRuleRecordData()
//
// Description: Get data based on category.
//
// -----------------------------------------------------------

FBCategoryRuleRecord*
FBDisplay::getBaseFareRuleRecordData(CatNumber cat) const
{
  std::map<CatNumber, FBCategoryRuleRecord*>::const_iterator i =
      _baseFarefbCategoryRuleRecordMap.find(cat);

  if (i == _baseFarefbCategoryRuleRecordMap.end())
    return nullptr;
  else
    return i->second;
}
// -----------------------------------------------------------
// @MethodName  FBDisplay::setLocationSwapped()
//
// Description: Sets isLocationSwapped indicators of Fare Rule Record 2
//                           FootNote Record 2
//                           General Rule Record 2
//              info based on category
//
// -----------------------------------------------------------
void
FBDisplay::setLocationSwapped(CatNumber cat, bool isLocationSwapped, DataHandle& dataHandle)
{

  // Search for the CAT in existing Map. If not found then create new.
  FBCategoryRuleRecord* fbCategoryRuleRecord = getRuleRecordData(cat);
  if (!fbCategoryRuleRecord)
    dataHandle.get(fbCategoryRuleRecord);

  fbCategoryRuleRecord->isFareRuleRecord2InfoLocationSwapped() = isLocationSwapped;
  fbCategoryRuleRecord->isFootNoteRecord2InfoLocationSwapped() = isLocationSwapped;
  fbCategoryRuleRecord->isGeneralRuleRecord2InfoLocationSwapped() = isLocationSwapped;

  // Set new value to the Map
  setRuleRecordData(cat, fbCategoryRuleRecord);
  return;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::setData()
//
// Description: Sets data of Fare Rule Record 2
//                           FootNote Record 2
//                           General Rule Record 2
//              info based on category
//
// -----------------------------------------------------------

void
FBDisplay::setData(CatNumber cat,
                   const FareRuleRecord2Info* fRR2,
                   const FootNoteRecord2Info* fNR2,
                   const GeneralRuleRecord2Info* gRR2,
                   const FareClassCode& fareBasis,
                   DataHandle& dataHandle)
{

  // Return back if cat is 10 or 25. We have seperate func for this.
  if (cat == COMBINABILITY_RULE || cat == FARE_BY_RULE)
    return;

  // Search for the CAT in existing Map. If not found then create new.
  FBCategoryRuleRecord* fbCategoryRuleRecord = getRuleRecordData(cat);
  if (!fbCategoryRuleRecord)
    dataHandle.get(fbCategoryRuleRecord);

  if (fRR2 != nullptr && !fRR2->categoryRuleItemInfoSet().empty())
    fbCategoryRuleRecord->fareRuleRecord2Info() = fRR2;
  if (fNR2 != nullptr && !fNR2->categoryRuleItemInfoSet().empty())
    fbCategoryRuleRecord->footNoteRecord2Info() = fNR2;
  if (gRR2 != nullptr && !gRR2->categoryRuleItemInfoSet().empty())
    fbCategoryRuleRecord->generalRuleRecord2Info() = gRR2;

  fbCategoryRuleRecord->fareBasis() = fareBasis;

  // Set new value to the Map
  setRuleRecordData(cat, fbCategoryRuleRecord);

  return;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::setData()

//
// Description: Sets data of Cobinability ( CAT10 )Record 2
//              info based on category
//
// -----------------------------------------------------------

void
FBDisplay::setData(const CombinabilityRuleInfo* cRR2,
                   const FareClassCode& fareBasis,
                   DataHandle& dataHandle,
                   bool isDomestic)

{
  if (!cRR2)
    return;

  // Search for the CAT in existing Map.
  // If not found then create new.

  FBCategoryRuleRecord* fbCategoryRuleRecord = getRuleRecordData(COMBINABILITY_RULE);

  if (!fbCategoryRuleRecord)
  {
    dataHandle.get(fbCategoryRuleRecord);
    fbCategoryRuleRecord->isRecordScopeDomestic() = isDomestic;
  }

  if (cRR2)
    fbCategoryRuleRecord->combinabilityRecord2Info() = cRR2;

  fbCategoryRuleRecord->fareBasis() = fareBasis;

  // Set new value to the Map
  setRuleRecordData(COMBINABILITY_RULE, fbCategoryRuleRecord);

  return;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::setData()
//
// Description: Sets data of Fare By Rule ( CAT25 )Record 2
//              info based on category
//
// -----------------------------------------------------------

void
FBDisplay::setData(const FareByRuleCtrlInfo* cFBRR2,
                   const FareClassCode& fareBasis,
                   DataHandle& dataHandle)
{
  if (!cFBRR2)
    return;

  // Search for the CAT in existing Map.
  // If not found then create new.

  FBCategoryRuleRecord* fbCategoryRuleRecord = getRuleRecordData(FARE_BY_RULE);

  if (!fbCategoryRuleRecord)
    dataHandle.get(fbCategoryRuleRecord);

  if (cFBRR2 != nullptr && !cFBRR2->categoryRuleItemInfoSet().empty())
    fbCategoryRuleRecord->fareByRuleRecord2Info() = cFBRR2;

  fbCategoryRuleRecord->fareBasis() = fareBasis;

  // Set new value to the Map
  setRuleRecordData(FARE_BY_RULE, fbCategoryRuleRecord);

  return;
}

// -----------------------------------------------------------
// @MethodName  FBDisplay::initialize()
//
// Description: Sets data for General Rule Record 2 based on category.
//
// -----------------------------------------------------------

void
FBDisplay::initialize(FareDisplayTrx& trx)
{

  const std::vector<CatNumber>& ruleCategories = trx.getOptions()->ruleCategories();

  DataHandle& dataHandle = trx.dataHandle();

  std::vector<CatNumber>::const_iterator iB = ruleCategories.begin();
  std::vector<CatNumber>::const_iterator iE = ruleCategories.end();

  while (iB != iE)
  {

    FBCategoryRuleRecord* fbCRR = getRuleRecordData(*iB);
    if (!fbCRR)
    {
      dataHandle.get(fbCRR);
      fbCRR->isRecordScopeDomestic() = trx.isRecordScopeDomestic(); // lint !e413
      setRuleRecordData(*iB, fbCRR);
    }
    iB++;
  }

  return;
}

#endif
