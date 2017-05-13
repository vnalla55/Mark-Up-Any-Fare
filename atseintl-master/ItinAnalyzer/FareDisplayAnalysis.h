//----------------------------------------------------------------------------
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#pragma once


#include <string>

namespace tse
{
class DataHandle;
class FareDisplayRequest;
class FareDisplayOptions;
class FareDisplayTrx;
class FareMarket;

class FareDisplayAnalysis final
{
  friend class FareDisplayAnalysisTest;

public:
  FareDisplayAnalysis(FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayAnalysis::selectTravelBoundary
  //
  // Description: Scans the origin and destination of each travel segment
  //              to select the travel Boundary of Itin and FareMarket.
  //
  // @param tvlSegs - valid reference to a vector of pointers to TravelSegs
  // @return Boundary - a boundary
  //--------------------------------------------------------------------------

  void process();
  void setFareMarketCat19Flags(FareDisplayTrx& trx, FareMarket* fareMarket);
  bool matchChInAppend(std::string& partialFareBasis, bool isChild);
  bool endsWith(const std::string& str, const std::string& end);

private:
  FareDisplayTrx& _trx;

  bool isQualifierPresent(const FareDisplayRequest& request, const FareDisplayOptions& options);
  void setFamilyPlanDefaults(const FareDisplayRequest&, FareDisplayOptions&);
  void populatePassengerTypes();
  void initializePaxType();

  bool setInclusionCode(FareDisplayRequest&, FareDisplayOptions&);

  void checkWebInclCd();

  FareDisplayRequest& _request;
  FareDisplayOptions& _options;
  DataHandle& _dataHandle;
};
}

