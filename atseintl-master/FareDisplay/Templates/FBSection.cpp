//-------------------------------------------------------------------
//
//  File:        FBSection.cpp
//  Authors:     Doug Batchelor
//  Created:     May 4, 2005
//  Description: This class abstracts a section.  It maintains
//               all the data and methods necessary to describe
//               and realize an FB response.
//
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

#include "FareDisplay/Templates/FBSection.h"

#include "Common/FallbackUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/RuleCategoryDescInfo.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

#include <sstream>
#include <string>

namespace tse
{

static Logger
logger("atseintl.FareDisplay.FBSection");

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  FBSection::buildDisplay()
//
// This is the main processing method of the FareCategoryTemplate class.
// It requires a reference to the Fare Display Transaction. When
// called, it iterates through all Rule categories and
// displays the Category number and description.
//
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------
void
FBSection::buildDisplay()
{
  // Make sure we have some fares
  if (_trx.allPaxTypeFare().empty())
    return;

  PaxTypeFare& paxTypeFare = *_trx.allPaxTypeFare().front();

  FareDisplayInfo* fdPtr = paxTypeFare.fareDisplayInfo();
  if (fdPtr == nullptr)
  {
    LOG4CXX_WARN(logger, "No FareDisplayInfo!");
    return;
  }

  // Check for FB data
  // We need to iterate through every entry in
  // Options::RuleCategories.  Get FBCategoryRuleRecord pointers
  // from FBDisplay and display the data associated with it.
  const FBDisplay& fb = fdPtr->fbDisplay();

  // Iterate through all Rule categories and display the rule
  // number and description.
  for (const auto& catNumber : _trx.getOptions()->ruleCategories())
  {
    _catRuleRecord = fb.getRuleRecordData(catNumber);

    if (!_catRuleRecord)
    {
      LOG4CXX_ERROR(logger, "No category rule record for: " << catNumber);
      continue;
    }

    if (!FareDisplayUtil::isFrrCustomer(_trx))
    {
      if (catNumber== RETAILER_CATEGORY)
        continue;
    }

    fdPtr->catNum() = catNumber;

    addCategoryLine(catNumber);

    FareDisplayResponseUtil fdru;
    fdru.buildFBDisplay(catNumber, _trx, fdPtr, paxTypeFare);
  } // end FOR loop
}

void
FBSection::addCategoryLine(const CatNumber& catNumber)
{
  const RuleCategoryDescInfo* ruleDescInfo = _trx.dataHandle().getRuleCategoryDesc(catNumber);
  if (ruleDescInfo == nullptr)
  {
    LOG4CXX_WARN(logger, "Unable to get cat desc for: " << catNumber);
    return;
  }

  // Category first
  _trx.response().setf(std::ios::right, std::ios::adjustfield);

  switch (catNumber)
  {
  case IC_RULE_CATEGORY:
    _trx.response() << INTL_CONST_CODE << BLANK;
    break;
  case RETAILER_CATEGORY:
    _trx.response() << RETAILER_CODE << BLANK;
    break;
  default:
    _trx.response() << std::setfill('0') << std::setw(2) << catNumber << BLANK;
    break;
  }
  _trx.response() << ruleDescInfo->shortDescription() << SPACE << DASH << std::endl;
}
} // tse namespace
