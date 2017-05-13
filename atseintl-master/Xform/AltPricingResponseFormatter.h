//----------------------------------------------------------------------------
//
//      File: AltPricingResponseFormatter.h
//      Description: Class to format WPA Pricing responses back to sending client
//      Created: February 8, 2006
//      Authors: Mike Carroll
//
//  Copyright Sabre 2006
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
#include "Common/TNBrands/ItinBranding.h"
#include "Xform/BrandsOptionsFilterForDisplay.h"
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

class AltPricingResponseFormatter : public PricingResponseFormatter
{
  friend class AltPricingResponseFormatterTest;
public:

  //--------------------------------------------------------------------------
  // @function AltPricingResponseFormatter::formatResponse
  //
  // Description: Prepare an AltPricingRequest
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
  // @function AltPricingResponseFormatter::formatResponse
  //
  // Description: Prepare an error response
  //
  // @param ere - error
  // @param response - formatted response
  //--------------------------------------------------------------------------
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

  void addAdditionalPaxInfo(PricingTrx& pricingTrx,
                            CalcTotals& calcTotals,
                            uint16_t paxNumber,
                            XMLConstruct& construct) override;

protected:
  void addAdditionalPaxInfoFareCalc(XMLConstruct& construct,
                                    PricingTrx& pricingTrx,
                                    const CalcTotals& calcTotals,
                                    const bool isUsDot,
                                    const bool isSchemaValid,
                                    const bool isDisclosureActivate) const;
  bool validSchemaVersion(const PricingTrx& pricingTrx) const override;

  void openTopLevelElement(XMLConstruct& construct);

  void preparePassengers(PricingTrx& pricingTrx,
                         FareCalcCollector& fareCalcCollector,
                         XMLConstruct& construct) override;

  void preparePassengersWithBrands(PricingTrx& pricingTrx,
                                   FareCalcCollector& fareCalcCollector,
                                   XMLConstruct& construct,
                                   ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger = nullptr);

  void addSoldOutInformation(XMLConstruct& construct,
              PricingTrx& pricingTrx,
              Itin& itin,
              BrandsOptionsFilterForDisplay::BrandingOptionSpaceIndex spaceIndex,
              const BrandsOptionsFilterForDisplay::SoldoutStatus& soldout,
              const PaxType* paxType) const;


  void scanTotalsItin(CalcTotals& calcTotals,
                      bool& fpFound,
                      bool& infantMessage,
                      char& nonRefundable,
                      MoneyAmount& moneyAmountAbsorbtion) override;

  void getPsgOptions(PricingTrx& pricingTrx,
                     FareCalcCollector* fareCalcCollector,
                     XMLConstruct& altConstruct);

  size_t numValidCalcTotals(FareCalcCollector& fareCalcCollector);

  void prepareFarePath(PricingTrx& pricingTrx,
                       CalcTotals& calcTotals,
                       const CurrencyNoDec& noDec1,
                       const CurrencyNoDec& noDec2,
                       uint16_t paxNumber,
                       bool stopoverFlag,
                       const FuFcIdMap& fuFcIdCol,
                       XMLConstruct& construct) override;

  void addCalcTotalsToResponse(PricingTrx& pricingTrx,
                              FareCalcCollector& fareCalcCollector,
                              CalcTotals& calcTotals,
                              const Itin& itin,
                              const FareCalcConfig& fcConfig,
                              int paxNumber,
                              XMLConstruct& construct,
                              ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger = nullptr);

  void addOCFeesReferences(const std::map<std::string, std::set<int>>& sfgToOCFeesMapping,
                           XMLConstruct& construct);

  void prepareOBFeesInfo(const PricingTrx& pricingTrx, XMLConstruct& construct) const;
  void prepareUncompressedResponseText(const PricingTrx& pricingTrx, std::string& str);
  std::string removeObFromResponse(const std::string& responseString) const;
  std::string removeObTagsFromResponse(const std::string& responseString) const;
  void prepareNoBrandsInfo(const PricingTrx& pricingTrx,
                           CalcTotals& calcTotals,
                           XMLConstruct& construct) const;

}; // End class AltPricingResponseFormatter

} // End namespace tse

