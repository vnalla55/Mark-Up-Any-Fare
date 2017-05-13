//-------------------------------------------------------------------
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

#include "Common/TseStringTypes.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/FareMarketPath.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class Itin;
class PricingTrx;
class FareMarket;
class MergedFareMarket;
class TravelSeg;

class FareMarketPathMatrix
{

public:
  typedef std::vector<FareMarketPath*> FareMarketPathVect;

  FareMarketPathMatrix(PricingTrx& trx,
                       Itin& itin,
                       std::vector<MergedFareMarket*>& mergedFareMarketVect,
                       BrandCode brandCode = "")
    : _trx(trx), _itin(itin), _brandCode(brandCode), _mergedFareMarketVect(mergedFareMarketVect)
  {
  }

  bool buildAllFareMarketPath();

  FareMarketPath* constructFareMarketPath();

  bool buildFareMarketPath(std::vector<TravelSeg*>& travelSegs,
                           std::vector<FareMarketPath*>& targetMatrix,
                           const bool processSideTrip);
  const BrandCode& getBrandCode()
  {
    return _brandCode;
  }

  void buildPath(std::vector<TravelSeg*>& travelSegs,
                 std::vector<MergedFareMarket*>& fareMarketVect,
                 const uint16_t curSegNum,
                 const uint16_t pathStartSN,
                 const uint16_t pathEndSN,
                 FareMarketPath& fareMarketpath,
                 std::vector<FareMarketPath*>& targetMatrix,
                 const bool processSideTrip);

  bool buildSideTrip();

  bool genMarketSideTripCombination(FareMarketPathVect& pathVect,
                                    uint16_t stIdx,
                                    uint16_t stCount,
                                    std::vector<FareMarketPathVect>& mktSideTripVect,
                                    std::vector<FareMarketPathVect>& mktSideTripsCombVect);

  bool genPathSideTripCombination(
      std::vector<MergedFareMarket*>& fareMarketPath,
      std::map<MergedFareMarket*, FareMarketPathVect>& mktPathMap,
      uint16_t mktIdx,
      uint16_t mktCount,
      std::vector<std::vector<FareMarketPathVect> >& pathSideTripVect,
      std::vector<std::map<MergedFareMarket*, FareMarketPathVect> >& pathSideTripCombVect);

  FareMarketPath* copyFareMarketPath(const FareMarketPath& oldPath);

  bool copyFareMarketPathVect(const FareMarketPathVect& oldVect, FareMarketPathVect& newVect);

  bool copyFareMarketPathMap(const std::map<MergedFareMarket*, FareMarketPathVect>& oldMktPathMap,
                             std::map<MergedFareMarket*, FareMarketPathVect>& newMktPathMap);

  void markUsedMergedFareMarket();
  bool mergedFareMarketUsedInPath(const MergedFareMarket* fareMarket,
                                  const std::vector<FareMarketPath*>& fmpVect);

  void collectDiagnostic(Itin* itin, std::vector<FareMarketPath*>* fmps = nullptr);

  const Itin& itin() const { return _itin; }
  Itin& itin() { return _itin; }

  const PricingTrx& trx() const { return _trx; }
  PricingTrx& trx() { return _trx; }

  const std::vector<FareMarketPath*>& fareMarketPathMatrix() const { return _fareMarketPathMatrix; }
  std::vector<FareMarketPath*>& fareMarketPathMatrix() { return _fareMarketPathMatrix; }

  const std::vector<MergedFareMarket*>& mergedFareMarketVect() const
  {
    return _mergedFareMarketVect;
  }
  std::vector<MergedFareMarket*>& mergedFareMarketVect() { return _mergedFareMarketVect; }

  typedef std::vector<FareMarketPathMatrix*> FareMarketPathMatrixVector;

protected:
  DataHandle _dataHandle;
  PricingTrx& _trx;
  Itin& _itin;
  BrandCode  _brandCode;

  // _mergedFareMarketVect contains all the MergedFareMarket for all the
  // Origin-Destination of the _itin in consideration.
  //
  // Each element has a vector<FareMarket*> for a particular Origin-Destination,
  // where each FareMarket is a reasult of merging of Multiple Gov-Cxr
  // FareMarket for that O&D of same priority. this vector is sorted
  // according to the  FareMarket priority
  //
  std::vector<MergedFareMarket*>& _mergedFareMarketVect;

  std::vector<FareMarketPath*> _fareMarketPathMatrix;

  void removeInvalidFareMarketPaths();

private:
  FareMarketPathMatrix(const FareMarketPathMatrix& fm);
  FareMarketPathMatrix& operator=(const FareMarketPathMatrix& rhs);

  static log4cxx::LoggerPtr _logger;
};

} // tse namespace

