//-------------------------------------------------------------------
//
//  File:        RuleCategoryProcessor.h
//  Created:     June 21, 2005
//  Authors:     Doug Batchelor
//
//  Description: This class performs required processing
//               on rule categories when the requested
//               display is RD (long or short).
//
//  Updates:
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

#pragma once

#include "DBAccess/RuleCatAlphaCode.h"
#include "Routing/RoutingInfo.h"
#include "Rules/FareMarketRuleController.h"

namespace tse
{

class FareDisplayOptions;
class FareDisplayTrx;

class RuleCategoryProcessor
{
public:
  RuleCategoryProcessor();
  virtual ~RuleCategoryProcessor();

  void decipherRuleDisplayCodes(FareDisplayTrx& trx, FareMarketRuleController& fmrc);

  static void adjustCategoryLists(FareDisplayTrx& trx, std::vector<uint16_t>& categorySequence);

protected:
  bool translateAlphaCode(FareDisplayOptions& fdo, AlphaCode& inputCode, FareDisplayTrx& trx);

  void addToRuleCategories(std::vector<CatNumber>& outputCategories,
                           const std::vector<RuleCatAlphaCode*>& catsToAdd);
  void addToRuleCategories(std::vector<CatNumber>& outputCategories,
                           const std::vector<CatNumber>& catsToAdd);

  // These methods do the following processing on a vector of
  // alpha codes or Category Numbers:
  // 1. eliminates any codes that are not valid.
  // 2. saves any invalid codes in the badCodes vector.
  // 3. eliminates any duplicate codes.
  // The inputCodes vector may be changed so the caller must be aware
  // of this and adjust accordingly.
  uint16_t elimBadCodes(std::vector<AlphaCode>& inputCodes,
                        std::vector<AlphaCode>& invalidCodes,
                        FareDisplayTrx& trx);
  uint16_t elimBadCodes(std::vector<CatNumber>& inputCodes,
                        std::vector<CatNumber>& invalidCodes,
                        const std::vector<uint16_t>& validCodes);

private:
  RuleCategoryProcessor(const RuleCategoryProcessor& rhs);
  RuleCategoryProcessor& operator=(const RuleCategoryProcessor& rhs);
};

} // End namespace tse

