//--------------------------------------------------------------------
//
//  File:        RuleCategoryProcessor.cpp
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
//--------------------------------------------------------------------

#include "Fares/RuleCategoryProcessor.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/RuleCatAlphaCode.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/FareMarketRuleController.h"
#include "Server/TseServer.h"

#include <iostream>

namespace tse
{

namespace
{
std::vector<CatNumber>
NC_CATEGORIES_VEC(NC_CATEGORIES, NC_CATEGORIES + NUM_NC_CATEGORIES);
}

//----------------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------------
RuleCategoryProcessor::RuleCategoryProcessor() {}

//----------------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------------
RuleCategoryProcessor::~RuleCategoryProcessor() {}

//----------------------------------------------------------------------------
// This method inspects any rule categories/alpha codes that may have
// been added to the query, validates & performs conversions and sends
// back the list of rule categories to be displayed along with possible
// error info.
//----------------------------------------------------------------------------
void
RuleCategoryProcessor::decipherRuleDisplayCodes(FareDisplayTrx& trx, FareMarketRuleController& fmrc)
{
  FareDisplayResponse& fdr = *trx.fdResponse();
  FareDisplayOptions& fdo = *trx.getOptions();

  int16_t numAlpha = fdo.alphaCodes().size(); // Combinable + Non-combinable
  int16_t numRules = fdo.ruleCategories().size();

  bool hasCommandLineCategories = false;

  bool showMX = fdo.isCombScoreboardDisplay();
  if (showMX && (numAlpha == 0) && (numRules == 0))
  {
    // Special case of /MX being the only thing on command line
    //...
    std::vector<CatNumber> v;
    fdo.ruleCategories().swap(v);
    return;
  }

  // Check for any rule categories.
  if (numRules > 0)
  {
    hasCommandLineCategories = true;
    // Eliminate any duplicates and bad codes
    numRules =
        elimBadCodes(fdo.ruleCategories(), fdr.badCategoryNumbers(), fmrc.categorySequence());
  }

  // Process any alpha codes.
  if (numAlpha > 0)
  {
    hasCommandLineCategories = true;
    // Eliminate any duplicates and bad codes
    numAlpha = elimBadCodes(fdo.alphaCodes(), fdr.badAlphaCodes(), trx);
    // if it's still bigger than 0 we need to translate all of the
    // codes into rule categories and add them to the
    // _ruleCategories list to be processed.
    if (numAlpha > 0)
    {
      bool mustAddCat10 = false;
      for (int i = 0; i < numAlpha; i++)
      {
        if (translateAlphaCode(fdo, fdo.alphaCodes()[i], trx))
          mustAddCat10 = true;
      }
      if (mustAddCat10)
      {
        addToRuleCategories(fdo.ruleCategories(), NC_CATEGORIES_VEC);
      }
    }
  }

  if (TrxUtil::isFqFareRetailerEnabled(trx))
  {
    // Check for fare mark-up on command line
    if (fdo.retailerDisplay() == IND_YES)
      addToRuleCategories(fdo.ruleCategories(),
                          trx.dataHandle().getRuleCatAlphaCode(RETAILER_CODE));
  }

  // Check for international Construction on command line
  if (fdo.IntlConstructionDisplay() == IND_YES)
    addToRuleCategories(fdo.ruleCategories(),
                        trx.dataHandle().getRuleCatAlphaCode(INTL_CONST_CODE));

  // Cat 50 shall be first
  std::vector<CatNumber>::iterator cat50 =
      std::find(fdo.ruleCategories().begin(), fdo.ruleCategories().end(), 50);

  if (cat50 != fdo.ruleCategories().end())
  {
    *cat50 = 0;
  }

  // Sort the resulting collection of rule categories in ascending order.
  std::sort(fdo.ruleCategories().begin(), fdo.ruleCategories().end());

  // Change Cat00 for Cat50
  if (numRules > 0 || numAlpha > 0)
  {
    std::vector<CatNumber>::iterator firstCat = fdo.ruleCategories().begin();
    if (*firstCat == 0)
      *firstCat = 50;
  }

  // Special case check for command line categories that are
  // all invalid.  If all command line categories are invalid,
  // we skip the category list adjustment.
  if (!hasCommandLineCategories || (numAlpha > 0) || (numRules > 0))
  {
    adjustCategoryLists(trx, fmrc.categorySequence());
  }
}

// This method does the following processing on a vector of alpha codes:
// 1. eliminates any codes that are not valid.
// 2. saves any invalid codes in the badCodes vector.
// 3. eliminates any duplicate codes.
// The inputCodes vector may be changed so the caller must be aware
// of this and adjust accordingly.
uint16_t
RuleCategoryProcessor::elimBadCodes(std::vector<AlphaCode>& inputCodes,
                                    std::vector<AlphaCode>& invalidCodes,
                                    FareDisplayTrx& trx)
{
  std::vector<AlphaCode>::iterator i = inputCodes.begin();
  std::vector<AlphaCode>::iterator iEnd = inputCodes.end();

  for (;;)
  {
    bool codeErased = false;

    i = inputCodes.begin();
    iEnd = inputCodes.end();

    for (; i != iEnd; i++)
    {
      // Query DB to determine if it's valid
      const std::vector<RuleCatAlphaCode*>& rcd = trx.dataHandle().getRuleCatAlphaCode(*i);
      if (!rcd.empty())
      {
        continue;
      }

      // it's a bad code, save it in vec of invalid codes
      invalidCodes.push_back(*i);
      // eliminate from original input vec.
      inputCodes.erase(i);
      codeErased = true;
      break;
    }
    if (!codeErased)
      break;
  }
  return inputCodes.size();
}

// This method does the following processing on a vector of rule categories:
// 1. eliminates any codes that are not valid.
// 2. saves any invalid codes in the badCodes vector.
// 3. eliminates any duplicate codes.
// The inputCodes vector may be changed so the caller must be aware
// of this and adjust accordingly.
uint16_t
RuleCategoryProcessor::elimBadCodes(std::vector<CatNumber>& inputCodes,
                                    std::vector<CatNumber>& invalidCodes,
                                    const std::vector<uint16_t>& validCodes)
{
  std::vector<CatNumber>::iterator i = inputCodes.begin();
  std::vector<CatNumber>::iterator iEnd = inputCodes.end();

  std::vector<uint16_t>::const_iterator j = validCodes.begin();
  std::vector<uint16_t>::const_iterator jEnd = validCodes.end();

  for (;;)
  {
    bool codeErased = false;

    if (inputCodes.empty())
      break;

    i = inputCodes.begin();
    iEnd = inputCodes.end();

    // Change Cat0 to real Cat50
    if (*i == 0)
      *i = 50;

    for (; i != iEnd; i++)
    {
      // Go through all the valid codes
      if (find(j, jEnd, *i) != jEnd)
        continue;

      // it's a bad code, save it in vec of invalid codes
      invalidCodes.push_back(*i);
      // eliminate from original input vec.
      inputCodes.erase(i);
      codeErased = true;
      break;
    }
    // Change Cat50 back to Cat0 for sorting
    std::vector<CatNumber>::iterator iter = inputCodes.begin();
    if (*iter == 50)
      *iter = 0;

    if (!codeErased)
      break;
  }
  return inputCodes.size();
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  RuleCategoryProcessor::translateAlphaCode()
//
// This method translates combinability and non-combinability alpha codes
// into category or sub-category numbers using the table, SPTRCA,
// and adds them to the vector of rule categories or sub-categories..
//
//
// @param   fdo        - a reference to a FareDisplayOptions object.
// @param   inputCode  - the alpha code to be translated.
// @param   trx        - a reference to the FareDisplayTrx object.
//
// @return  bool.      - true if a cat 10 sub-category was added
//                     - false if a cat 10 sub-category was NOT added
//
// </PRE>
// -------------------------------------------------------------------------
bool
RuleCategoryProcessor::translateAlphaCode(FareDisplayOptions& fdo,
                                          AlphaCode& inputCode,
                                          FareDisplayTrx& trx)
{
  const std::vector<RuleCatAlphaCode*>& rcd = trx.dataHandle().getRuleCatAlphaCode(inputCode);

  if ((inputCode == COMBINABILITY_CODE_OW) || (inputCode == COMBINABILITY_CODE_OJ) ||
      (inputCode == COMBINABILITY_CODE_RT) || (inputCode == COMBINABILITY_CODE_CT) ||
      (inputCode == COMBINABILITY_CODE_EE) || (inputCode == COMBINABILITY_CODE_IA))
  {
    // Is Rule category 10 already in the output list?
    if (find(fdo.ruleCategories().begin(),
             fdo.ruleCategories().end(),
             COMBINABILITY_RULE_CATEGORY) == fdo.ruleCategories().end())
    {
      // No it's not, so add the sub-cat
      FareDisplayResponse& fdr = *trx.fdResponse();
      addToRuleCategories(fdr.subCategoryNumbers(), rcd);
      return true;
    }
  }
  else
  {
    addToRuleCategories(fdo.ruleCategories(), rcd);
  }
  return false;
}

// This method adds a set of rule categories contained in catsToAdd
// to the outputCategories vector.
// If the category to be added is already in outputCategories, it
// is simply not used.  This assures that the resulting vector of
// rule categories does not contain duplicates.
//
void
RuleCategoryProcessor::addToRuleCategories(std::vector<CatNumber>& outputCategories,
                                           const std::vector<RuleCatAlphaCode*>& catsToAdd)
{
  std::vector<RuleCatAlphaCode*>::const_iterator i = catsToAdd.begin();
  std::vector<RuleCatAlphaCode*>::const_iterator iEnd = catsToAdd.end();

  for (; i != iEnd; i++)
  {
    if (find(outputCategories.begin(), outputCategories.end(), (*i)->displayCategory()) !=
        outputCategories.end())
      continue;
    // add this rule category to the list
    outputCategories.push_back((*i)->displayCategory());
  }
}

void
RuleCategoryProcessor::addToRuleCategories(std::vector<CatNumber>& outputCategories,
                                           const std::vector<CatNumber>& catsToAdd)
{
  std::vector<CatNumber>::const_iterator i = catsToAdd.begin();
  std::vector<CatNumber>::const_iterator iEnd = catsToAdd.end();

  for (; i != iEnd; i++)
  {
    if (find(outputCategories.begin(), outputCategories.end(), *i) != outputCategories.end())
      continue;
    // add this rule category to the list
    outputCategories.push_back(*i);
  }
}

// We must make sure that the RuleController's _categorySequence contains the
// corrrect set of rule categories.  If any were part of the RD command, they will be
// in the vector _ruleCategories in /DataModel/FareDisplayOptions.h.  These are the
// categories to use.  If not, we should use the default list read from
// TseServer.cfg.

void
RuleCategoryProcessor::adjustCategoryLists(FareDisplayTrx& trx, std::vector<uint16_t>& categorySequence)
{
  FareDisplayOptions& fdo = *trx.getOptions();

  if (fdo.ruleCategories().empty())
  {
    // In this case we want to use the default list, so transfer it to _ruleCategories.
    std::vector<uint16_t>::const_iterator i = categorySequence.begin();
    std::vector<uint16_t>::const_iterator j = categorySequence.end();
    fdo.ruleCategories().insert(fdo.ruleCategories().begin(), i, j);
    // if the list contains IC category, set option to show it.
    if (find(i, j, IC_RULE_CATEGORY) != j)
      fdo.IntlConstructionDisplay() = IND_YES;
    if (find(i, j, RETAILER_CATEGORY) != j)
    {
      if (FareDisplayUtil::isFrrCustomer(trx))
        fdo.retailerDisplay() = IND_YES;
    }
  }
}
}
