//----------------------------------------------------------------------------
//  File: BrandedFaresDataRetriever.h
//
//  Author: Michal Mlynek
//
//  Created:      07/01/2013
//
//  Description: For each fare market computes a ODC ( origin/destination/carrier)
//               tuple and creates a map : ODC -> vactor<FareMarket*>.
//               Then builds, using this map entries, requests to Pricing
//
//  Copyright Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "BrandedFares/BrandingRequestResponseHandler.h"
#include "Common/IAIbfUtils.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/IbfDiag901Collector.h"

namespace tse
{

class Diag892Collector;
class Diag894Collector;

typedef IAIbfUtils::OdcTuple OdcTuple;
typedef IAIbfUtils::ODPair ODPair;
typedef IAIbfUtils::OdDataForFareMarket OdDataForFareMarket;
typedef IAIbfUtils::FMsForBranding FMsForBranding;
typedef IAIbfUtils::OdcsForBranding OdcsForBranding;
typedef IAIbfUtils::FMsForBrandingPair FMsForBrandingPair;

class BrandedFaresDataRetriever
{

public:
  BrandedFaresDataRetriever(PricingTrx& trx, BrandRetrievalMode mode,
    Diag892Collector* diag892 = nullptr, Diag894Collector* diag894 = nullptr);
  ~BrandedFaresDataRetriever();

  bool process();
  const BrandingRequestResponseHandler& getRequestResponseHandler() const
  {
    return _requestResponseHandler;
  }

private:
  void flushDiagnostics();

  // For each fare market computes an ODC tuples this fare market applies to ( according to the IBF
  // brands matching legs logic )
  // Builds a map ODCTuple -> std::vector<FareMarket*>
  // returns true if at least one element has been inserted into the map
  bool buildODCWithFareMarketsMap();
  void buildODCWithFareMarketsMapIS();
  void buildODCWithFareMarketsMapMIP();

  void insertOdcTupleVecIntoMap(const std::vector<OdcTuple>& odcTupleVec, FareMarket* fm);
  // using ODC->FMs map sends requests to Pricing in order to obtain brands applicable for each ODC
  // tuple
  // returns status of getBrands() function
  bool retrieveBrandedFares();

  void displayFMsAndODCMatching();
  void flushBrandedDiagnostics(DiagCollector* diag);
  void fillDummyFMWithBrands();


  PricingTrx& _trx;
  bool _isMip;
  bool _isCatchAllBucket;
  bool _shouldYYFMsBeIgnored;
  const BrandRetrievalMode _brandRetrievalMode;
  FMsForBranding _fMsForBranding;
  std::vector<PaxTypeCode> _paxTypes;
  IAIbfUtils::TripType _tripType;
  Diag892Collector* _diag892;
  Diag894Collector* _diag894;
  IbfDiag901Collector _diag901;
  BrandingRequestResponseHandler _requestResponseHandler;
  static Logger _logger;
  static std::string _ignoredYYFareMarketsMsg;

  friend class BrandedFaresDataRetrieverTest;
};

} // End namespace tse
