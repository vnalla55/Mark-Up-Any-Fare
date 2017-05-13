/*----------------------------------------------------------------------------
*  Copyright Sabre 2015
*
*     The copyright to the computer program(s) herein
*     is the property of Sabre.
*     The program(s) may be used and/or copied only with
*     the written permission of Sabre or in accordance
*     with the terms and conditions stipulated in the
*     agreement/contract under which the program(s)
*     have been supplied.
*-------------------------------------------------------------------------*/
#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseFormatter.h"
#include <string>

namespace tse
{
class PricingTrx;
class CalcTotals;
class FareCalcCollector;
class FareComponentDataFormatter;

class StructuredRulesResponseFormatter : public PricingResponseFormatter
{
public:
  StructuredRulesResponseFormatter() = default;

  static void formatResponse(const ErrorResponseException& ere, std::string& response);
  static void
  formatResponse(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  std::string formatResponse(const std::string& responseString,
                             bool displayOnly,
                             PricingTrx& pricingTrx,
                             FareCalcCollector* fareCalcCollector,
                             ErrorResponseException::ErrorResponseCode errCode =
                                 ErrorResponseException::NO_ERROR) override;

private:
  static void formatFareComponentLevelData(const std::vector<FareCompInfo*>& fcContainer,
                                           const FarePath* farePath,
                                           FareComponentDataFormatter& formater);
};
}
