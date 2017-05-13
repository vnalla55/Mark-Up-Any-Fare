//----------------------------------------------------------------------------
//
//      File: NoPNRPricingResponseFormatter.h
//      Description: Class to format WQ Pricing responses back to sending client
//      Created: January 30, 2008
//      Authors: Marcin Augustyniak
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

#pragma once

#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseFormatter.h"

#include <log4cxx/helpers/objectptr.h>

#include <string>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PricingTrx;
class FareCalcCollector;

class NoPNRPricingResponseFormatter : public PricingResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function NoPNRPricingResponseFormatter::formatResponse
  //
  // Description: Prepare an NoPNRPricingRequest
  //
  // @param responseString - a valid string
  // @param displayOnly - display only
  //--------------------------------------------------------------------------
  virtual std::string formatResponse(
      const std::string& responseString,
      bool displayOnly,
      PricingTrx& pricingTrx,
      FareCalcCollector* fareCalcCollector,
      ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR) override;

  //--------------------------------------------------------------------------
  // @function NoPNRPricingResponseFormatter::formatResponse
  //
  // Description: Prepare an error response
  //
  // @param ere - error type
  // @param response - formatted response
  //--------------------------------------------------------------------------
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

protected:
  void openTopLevelElement(XMLConstruct& construct);

  void preparePassengers(PricingTrx& pricingTrx,
                         FareCalcCollector& fareCalcCollector,
                         XMLConstruct& construct) override;

  void addAdditionalPaxInfo(PricingTrx& pricingTrx,
                            CalcTotals& calcTotals,
                            uint16_t paxNumber,
                            XMLConstruct& construct) override;

  void scanTotalsItin(CalcTotals& calcTotals,
                      bool& fpFound,
                      bool& infantMessage,
                      char& nonRefundable,
                      MoneyAmount& moneyAmountAbsorbtion) override;

  size_t numValidCalcTotals(FareCalcCollector& fareCalcCollector);

  void prepareFarePath(PricingTrx& pricingTrx,
                       CalcTotals& calcTotals,
                       const CurrencyNoDec& noDec1,
                       const CurrencyNoDec& noDec2,
                       uint16_t paxNumber,
                       bool stopoverFlag,
                       const FuFcIdMap& fuFcIdCol,
                       XMLConstruct& construct) override;

private:
  static log4cxx::LoggerPtr _logger;
  static log4cxx::LoggerPtr _uncompressedLogger;

}; // End class NoPNRPricingResponseFormatter

} // End namespace tse

