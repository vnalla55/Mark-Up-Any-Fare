//-------------------------------------------------------------------
//
//  File:        FareDisplayController.h
//  Created:     February 1, 2005
//  Authors:     LeAnn Perez
//
//  Updates:
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

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <set>

namespace tse
{
class Fare;
class FareDisplayTrx;
class FareDisplayResponse;
class FBDisplay;
class PaxTypeFare;
class FareDisplayOptions;

/**
*   @class FareDisplayController
*
*   Description:
*   FareDisplay is responsible for displaying the FQ response.
*
*/
class FareDisplayController final
{
  friend class FareDisplayControllerTest;

public:
  void display(FareDisplayTrx& trx);

private:
  static const uint16_t MAX_COUNT = 5;
  static const uint16_t LAST_CXR = 4;
  static const uint16_t DEFAULT_SHOPPER_TEMPLATE = 40;

  static constexpr char SLASH = '/';

  bool displayFareInfo(FareDisplayTrx& trx);
  bool isRuleDisplay(FareDisplayTrx& trx);
  int16_t validFares(FareDisplayTrx& trx);
  void getRuleText(FareDisplayTrx& trx);
  void callForRuleText(FareDisplayTrx& trx,
                       const FareDisplayResponse& response,
                       FBDisplay& fbDisplay,
                       const int32_t objId,
                       PaxTypeFare& ptf);
  bool isRTGNeeded(FareDisplayTrx& trx);
  bool hasCat16fromBaseFare(const PaxTypeFare& ptf) const;

  bool isRuleCategoryRequested(FareDisplayOptions& fdo, CatNumber cat);
};
} // namespace tse
