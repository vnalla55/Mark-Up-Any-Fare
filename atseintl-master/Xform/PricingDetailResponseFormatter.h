//----------------------------------------------------------------------------
//
//      File: PricingDetailResponseFormatter.h
//      Description: Class to format WPDF responses back to PSS
//      Created: May 18, 2005
//      Authors: Andrea Yang
//
//  Copyright Sabre 2005
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"


namespace tse
{

class CurrencyDetail;
class DateTime;
class ErrorResponseException;
class FareCalcConfig;
class FareCalcDetail;
class PricingDetailTrx;
class PaxDetail;
class SegmentDetail;
class TaxDetail;

class PricingDetailResponseFormatter
{
  friend class PricingDetailResponseFormatterTest;

public:
  virtual ~PricingDetailResponseFormatter() = default;

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::formatResponse
  //
  // Description: Prepare a WP response tagged suitably for PSS
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @return std::string - response string
  //--------------------------------------------------------------------------
  std::string
  formatResponse(PricingDetailTrx& pricingDetailTrx, FareCalcConfig* fcConfigArg = nullptr);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::formatResponse
  //
  // Description: Prepare an error response
  //
  // @return std::string - response string
  //--------------------------------------------------------------------------
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

protected:
  // Separation from database
  virtual tse::CurrencyNoDec
  noCurrencyDec(const tse::CurrencyCode& code, const DateTime& ticketingDate) const;

private:
  static constexpr char HORIZONTAL_FARECALC1 = '1';
  static constexpr char VERTICAL_FARECALC2 = '2';
  static constexpr char MIX_FARECALC3 = '3';

  static constexpr Indicator FC_ONE = '1';
  static constexpr Indicator FC_TWO = '2';
  static constexpr Indicator FC_THREE = '3';
  static constexpr Indicator BLANK = ' ';
  static constexpr Indicator PARENTH = '(';
  static constexpr Indicator DISPLAY = '*';
  static constexpr Indicator BRACKET = '<';

  static constexpr int32_t BASE_FARE_WIDTH = 9;
  static constexpr int32_t HORIZ_EQUIV_WIDTH = 11;
  static constexpr int32_t HORIZ_TAX_WIDTH = 10;
  static constexpr int32_t HORIZ_TOTAL_WIDTH = 12;
  static constexpr int32_t VERT_EQUIV_WIDTH = 9;
  static constexpr int32_t VERT_TAX_WIDTH = 9;
  static constexpr int32_t VERT_TOTAL_WIDTH = 9;
  static constexpr int32_t MIXED_EQUIV_WIDTH = 10;
  static constexpr int32_t MIXED_TAX_WIDTH = 10;
  static constexpr int32_t MIXED_TOTAL_WIDTH = 10;

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addPassengerTypeLine
  //
  // Description: Add the WPDF Passenger Type line
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addPassengerTypeLine(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addFareLine
  //
  // Description: Add the WPDF fare line
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  // @param fcConfig - output configuration information
  //--------------------------------------------------------------------------
  void addFareLine(PricingDetailTrx& pricingDetailTrx,
                   const PaxDetail& paxDetail,
                   const FareCalcConfig& fcConfig);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addTaxLine
  //
  // Description: Add the WPDF tax line
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  // @param fcConfig - output configuration information
  //--------------------------------------------------------------------------
  void addTaxLine(PricingDetailTrx& pricingDetailTrx,
                  const PaxDetail& paxDetail,
                  const FareCalcConfig& fcConfig);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addTotalLine
  //
  // Description: Add the WPDF total line
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  // @param fcConfig - output configuration information
  //--------------------------------------------------------------------------
  void addTotalLine(PricingDetailTrx& pricingDetailTrx,
                    const PaxDetail& paxDetail,
                    const FareCalcConfig& fcConfig);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addFareCalcLine
  //
  // Description: Add the WPDF Fare Calc line
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addFareCalcLine(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addXTLines
  //
  // Description: Add the WPDF XT line(s)
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  // @param fcConfig - output configuration information
  //--------------------------------------------------------------------------
  void addXTLines(PricingDetailTrx& pricingDetailTrx,
                  const PaxDetail& paxDetail,
                  const FareCalcConfig& fcConfig);

  void extractExchangeRates(const CurrencyDetail& curDetail, ExchRate& rateOne, ExchRate& rateTwo);

  size_t formatExchangeRate(const ExchRate& rate,
                            const CurrencyNoDec noDec,
                            const CurrencyCode& from,
                            const CurrencyCode& to,
                            char separator,
                            std::ostringstream& stream);

  size_t formatBSR(const ExchRate& rate,
                   const CurrencyNoDec noDec,
                   const CurrencyCode& from,
                   const CurrencyCode& to,
                   char separator,
                   std::ostringstream& stream);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addBSRLine
  //
  // Description: Add the WPDF BSR line
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addBSRLine(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addTrafficDocumentIssuedInLine
  //
  // Description: Add the WPDF Traffic Document Issued In
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  //--------------------------------------------------------------------------
  void addTrafficDocumentIssuedInLine(PricingDetailTrx& pricingDetailTrx);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addFareCalcHeaderLine
  //
  // Description: Add the header line for the Fare Calc info
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  // @param fcConfig - output configuration information
  //--------------------------------------------------------------------------
  void addFareCalcHeaderLine(PricingDetailTrx& pricingDetailTrx,
                             const PaxDetail& paxDetail,
                             const FareCalcConfig& fcConfig);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addSegmentDetails
  //
  // Description: Add detailed information for each segment
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addSegmentDetails(PricingDetailTrx& pricingDetailTrx,
                         const PaxDetail& paxDetail,
                         const FareCalcConfig& fcConfig);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addComponentPrefixInfo
  //
  // Description: Add detailed information for fare component prefixes
  //
  // @param segmentDetail - the segment to be processed
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  //--------------------------------------------------------------------------
  void addComponentPrefixInfo(const SegmentDetail& segmentDetail,
                              const std::string& calcLine,
                              PricingDetailTrx& pricingDetailTrx,
                              const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addSurchargeInfo
  //
  // Description: Add detailed information of all surcharges in a segment
  //
  // @param segmentDetail - the segment from which the surcharges will be taken
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  //--------------------------------------------------------------------------
  void addSurchargeInfo(const SegmentDetail& segmentDetail,
                        const PaxDetail& paxDetail,
                        PricingDetailTrx& pricingDetailTrx,
                        const FareCalcConfig& fcConfig);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addFareBreakPointInfo
  //
  // Description: Add detailed information of all fare break points
  //
  // @param fareCalcDetail - the segment from which the surcharges will be taken
  // @param pricingDetailTrx - the stream on which to output the surcharge information
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addFareBreakPointInfo(const FareCalcDetail& fareCalcDetail,
                             PricingDetailTrx& pricingDetailTrx,
                             const PaxDetail& paxDetail,
                             const FareCalcConfig& fcConfig,
                             const bool isSideTrip);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addPlusUps
  //
  // Description: Add detailed information for each plus up charge
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addPlusUps(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addDifferentials
  //
  // Description: Add detailed information for each differential
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addDifferentials(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addNonSpecificTotal
  //
  // Description: Add Non Specific Stopover/Transfer surcharges
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addNonSpecificTotal(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addConsolidatorPlusUp
  //
  // Description: Add detailed information for the consolidator plus up
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addConsolidatorPlusUp(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addBreakPointTotal
  //
  // Description: Add detailed information for each plus up charge
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addBreakPointTotal(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addTaxHeaderLine
  //
  // Description: Add the header for the detailed tax information
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addTaxHeaderLine(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addTaxDetails
  //
  // Description: Add the detailed tax information
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addTaxDetails(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addIATARateInfo
  //
  // Description: Add the currency conversion information for NUC fares
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addIATARateInfo(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addFareBSRInfo
  //
  // Description: Add the BSR currency conversion information for fares
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addFareBSRInfo(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addTaxBSRInfo
  //
  // Description: Add the BSR currency conversion information for taxes
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addTaxBSRInfo(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  void formatExchangeRateColumns(int& column, size_t fieldSize, std::ostringstream& response);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addBSR
  //
  // Description: Add the actual BSR currency conversion information
  //
  // @param response - stream to which to output the information
  // @param curDetail - currency conversion information
  //--------------------------------------------------------------------------
  void addBSR(PricingDetailTrx& pricingDetailTrx,
              std::ostringstream& response,
              const CurrencyDetail& curDetail,
              int& column);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::addPUTripType
  //
  // Description: Add the pricing unit trip type information
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @param paxDetail - passenger type information
  //--------------------------------------------------------------------------
  void addPUTripType(PricingDetailTrx& pricingDetailTrx, const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::taxExempt
  //
  // Description: Check to see if a given tax code is exempt for this transaction
  //
  // @param code - tax code to check
  // @return bool true if tax exempt, false otherwise
  //--------------------------------------------------------------------------
  bool taxExempt(PricingDetailTrx& pricingDetailTrx,
                 const std::string& code,
                 const PaxDetail& paxDetail);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::analyzeFCwithSideTrip
  //
  // Description: Analyze Fare Component with the side-trip in it.
  //              The FC could be split in two FareCalcDetail. One partial FCD
  //              before FareCalcDetail with the side-trip, and another FCD after
  //              side-trip. Need to combine them in one FCD for display purpose.
  //
  // @param fareCalcDetail - current Fare Calc Detail pointer
  //
  //--------------------------------------------------------------------------
  void analyzeFCwithSideTrip(FareCalcDetail* fcd);

  //--------------------------------------------------------------------------
  // @function PricingDetailResponseFormatter::isCanadianPointOfSale
  //
  // Description: Determine whether the agent is located in Canada or not
  //
  // @param pricingDetailTrx - a valid Pricing Detail Trx
  // @return bool true if agent is in Canada
  //
  //--------------------------------------------------------------------------
  bool isCanadianPointOfSale(PricingDetailTrx& pricingDetailTrx);

  void createFCMarketMap(const PaxDetail& paxDetail);

  std::string fareDescription(const FareCalcDetail& fareCalcDetail) const;

  FareCalcDetail* _fCDstored;
  std::vector<LocCode> _departureCity;

  std::map<const SegmentDetail*, std::pair<size_t, size_t> > _marketPosInFCLine;

}; // End class PricingDetailResponseFormatter

} // End namespace tse

