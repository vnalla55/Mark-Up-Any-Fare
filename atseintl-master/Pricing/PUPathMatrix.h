//-------------------------------------------------------------------
// File:    PUPQ.h
// Created: April 2004
// Authors: Mohammad Hossan
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

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseThreadingConst.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FlexFares/Types.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"

#include <boost/pool/object_pool.hpp>

#include <set>
#include <vector>

namespace tse
{
class FareMarket;
class FareMarketPath;
class FareMarketPathMatrix;
class Itin;
class PricingTrx;
class PricingUnitFactoryBucket;
class ConfigMan;

class PUPathMatrix
{

  friend class PUPathMatrixTest;
  friend class PUPathMatrixUseReducedConstructionsTest;

public:
  PUPathMatrix() = default;

  virtual ~PUPathMatrix() = default;

  struct PUPathBuildTask : public TseCallableTrxTask
  {
    PUPathBuildTask() { desc("PO BUILD PUPATH"); }

    void performTask() override;
    void combineSTPUpath(
        std::vector<PUPath*>& mainPathMatrix,
        std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > >& pathPUPathCombVect,
        std::vector<PUPath*>& combinedPUPaths);

    PUPathMatrix* _puPathMatrix = nullptr;
    FareMarketPath* _fareMarketPath = nullptr;
  };

  PU* constructPU()
  {
    PU* pu = nullptr;

    if (LIKELY(_avoidStaticObjectPool))
    {
      boost::lock_guard<boost::mutex> g(_puPoolMutex);
      pu = _puPool.construct();
    }
    else
    {
      pu = _dataHandle.create<PU>();
    }

    pu->setFlexFaresGroupId(getFlexFaresGroupId());

    return pu;
  }

  PUPath* constructPUPath()
  {
    PUPath* puPath = nullptr;

    if (LIKELY(_avoidStaticObjectPool))
    {
      boost::lock_guard<boost::mutex> g(_puPathPoolMutex);
      puPath = _puPathPool.construct();
    }
    else
      puPath = _dataHandle.create<PUPath>();

    puPath->itin() = _itin;
    puPath->setFlexFaresGroupId(getFlexFaresGroupId());

    if(_trx->getRequest()->isBrandedFaresRequest() && (_trx->getTrxType() == PricingTrx::MIP_TRX))
    {
      TSE_ASSERT(_brandCode != nullptr);
      puPath->setBrandCode(*_brandCode);
    }

    return puPath;
  }

  enum ItinBoundary
  {
    UNKNOWN,
    ONE_SUB_IATA,
    ONE_IATA,
    TWO_IATA,
    ALL_IATA
  };

  bool buildAllPUPath(FareMarketPathMatrix& fmpMatrix,
                      std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);
  bool buildAllPUPath(std::vector<FareMarketPath*>& matrix,
                      std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  const Itin* itin() const { return _itin; }
  Itin*& itin() { return _itin; }

  const PricingTrx* trx() const { return _trx; }
  PricingTrx*& trx() { return _trx; }

  DataHandle& dataHandle() { return _dataHandle; }

  bool& useCxrPreference() { return _useCxrPreference; }
  bool useCxrPreference() const { return _useCxrPreference; }

  const std::vector<PUPath*>& puPathMatrix() const { return _puPathMatrix; }
  std::vector<PUPath*>& puPathMatrix() { return _puPathMatrix; }

  tse::ConfigMan*& config() { return _config; }
  tse::ConfigMan* config() const { return _config; }

  typedef std::vector<PUPathMatrix*> PUPathMatrixVector;
  typedef std::map<MergedFareMarket*, std::vector<PUPath*> >::value_type ST_VALUE_TYPE;
  typedef std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >::value_type
  STV_VALUE_TYPE;

  const bool avoidStaticObjectPool() const { return _avoidStaticObjectPool; }
  bool& avoidStaticObjectPool() { return _avoidStaticObjectPool; }

  uint16_t brandIndex() const { return _brandIndex; }
  uint16_t& brandIndex() { return _brandIndex; }

  BrandCode*& brandCode() { return _brandCode; }
  const BrandCode* brandCode() const { return _brandCode; }

  void setFlexFaresGroupId(const flexFares::GroupId& id) { _flexFaresGroupId = id; }
  const flexFares::GroupId& getFlexFaresGroupId() const { return _flexFaresGroupId; }

  struct PUPathPtrCmp
  {
    bool operator()(const PUPath* puPath1, const PUPath* puPath2) { return *puPath1 < *puPath2; }
  };

  bool checkOJMileageRestriction(const std::vector<MergedFareMarket*>& obLeg,
                                 const std::vector<MergedFareMarket*>& ibLeg,
                                 const GeoTravelType geoTvlType,
                                 const PricingUnit::PUSubType& ojType,
                                 PricingUnit::OJSurfaceStatus& ojSurfaceShortest,
                                 DateTime travelDate,
                                 const bool openJawBtwTwoAreas,
                                 std::vector<CarrierCode>& invalidCxrForTOJ,
                                 bool failOnMileage = false);

protected:
  typedef std::vector<MergedFareMarket*> MergedFMVector;

  PU* buildRTPU(MergedFareMarket*, MergedFareMarket*);
  PU* buildOJPU(const MergedFMVector& outboundFM,
                const MergedFMVector& inboundFM,
                const GeoTravelType geoTvlType,
                const bool ojPUCxrPref);
  PU* buildOWPU(MergedFareMarket*);

  bool buildPUPath(const FareMarketPath& fmp,
                   const uint16_t mktIdx,
                   const uint16_t totalMktCnt,
                   PUPath& puPath,
                   std::vector<MergedFareMarket*>& fmPath,
                   std::vector<PUPath*>& puPathVect,
                   bool& done,
                   bool onlyOwFares = false);

  bool buildRT(const FareMarketPath& fmp,
               const uint16_t mktIdx,
               const uint16_t totalMktCnt,
               PUPath& puPath,
               std::vector<MergedFareMarket*>& fmPath,
               std::vector<PUPath*>& puPathVect,
               bool& done);

  bool buildCT(const FareMarketPath& fmp,
               const uint16_t mktIdx,
               const uint16_t totalMktCnt,
               PUPath& puPath,
               std::vector<MergedFareMarket*>& fmPath,
               std::vector<PUPath*>& puPathVect,
               bool& done);

  bool buildOJ(const FareMarketPath& fmp,
               const uint16_t obCompCount,
               const uint16_t ibCompCount,
               const uint16_t mktIdx,
               const uint16_t ibMktIdx,
               const uint16_t totalMktCnt,
               PUPath& puPath,
               std::vector<MergedFareMarket*>& fmPath,
               std::vector<PUPath*>& puPathVect,
               bool& done);

  bool buildOW(const FareMarketPath& fmp,
               const uint16_t mktIdx,
               const uint16_t totalMktCnt,
               PUPath& puPath,
               std::vector<MergedFareMarket*>& fmPath,
               std::vector<PUPath*>& puPathVect,
               bool& done);

  void buildCTPU(PU& ctPU,
                 const uint16_t nextIdx,
                 const uint16_t totalMktCnt,
                 PUPath& puPath,
                 std::vector<MergedFareMarket*>& fmPath,
                 bool passedCTProvision,
                 bool& ctFound,
                 bool& done);

  PU& buildRW(MergedFareMarket& mfm);

  void updatePUFactoryBucket(PUPath& puPath,
                             std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                             std::set<PU*, PU::PUPtrCmp>& alreadySeen);
  void addPUToPUFactoryBucket(PU*& pu,
                              std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                              std::set<PU*, PU::PUPtrCmp>& alreadySeen);

  bool buildSideTripPUPath(
      FareMarketPath& fmp,
      // std::map< MergedFareMarket*, std::vector<FareMarketPath*> >& sideTrips,
      std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > >& pathPUPathCombVect);

  bool isPUPathValid(const FareMarketPath& fmp, PUPath& puPath, const uint16_t totalMktCnt);
  virtual bool isOWoutOfIntlRT(const FareMarketPath& fmp, PUPath& puPath);
  bool twoOWFormRT(const PU& owPU1, const PU& owPU2);
  bool owPUtoBlock(const PU& pu);

  bool isRoundTrip(const MergedFareMarket& fm1, const MergedFareMarket& fm2);

  bool isValidOpenJawTrip(const std::vector<MergedFareMarket*>& obLeg,
                          const std::vector<MergedFareMarket*>& ibLeg,
                          const GeoTravelType geoTvlType,
                          PricingUnit::PUSubType& ojType,
                          bool& sameNationOJ,
                          bool& sameNationOrigSurfaceOJ,
                          bool& allowNOJInZone210,
                          PricingUnit::OJSurfaceStatus& ojSurfaceShortest,
                          bool& openJawBtwTwoAreas,
                          std::vector<CarrierCode>& invalidCxrForTOJ,
                          bool& allowNormalFareForOOJ,
                          bool& specialEuropeanDoubleOJ,
                          bool& specialOJ);

  bool checkMileageForTOJAcross2Areas(const std::vector<MergedFareMarket*>& obLeg,
                                      const std::vector<MergedFareMarket*>& ibLeg,
                                      const uint32_t smaller_mileage,
                                      const uint32_t larger_mileage,
                                      const uint32_t surfaceTOJ_mileage,
                                      std::vector<CarrierCode>& invalidCxrForTOJ);

  bool checkMileageForTOJAcross2AreasPerLeg(const std::vector<MergedFareMarket*>& leg,
                                            const uint32_t smaller_mileage,
                                            const uint32_t larger_mileage,
                                            const uint32_t surfaceTOJ_mileage,
                                            std::vector<CarrierCode>& invalidCxrForTOJ);

  bool checkOJIataArea(const Loc& obOrig,
                       const Loc& obDest,
                       const GlobalDirection obGD,
                       const Loc& ibOrig,
                       const Loc& ibDest,
                       const GlobalDirection ibGD,
                       const PricingUnit::PUSubType& ojType,
                       bool& invalidOJ,
                       bool& openJawBtwTwoAreas,
                       bool& failOnMileage);

  bool fmReturnToOJLeg(const MergedFareMarket& fm, const std::vector<MergedFareMarket*>& ojLeg);

  void determineItinTravelBoundary();
  bool isTravelWithinScandinavia();

  bool isInLeg(const Loc& loc,
               const LocCode& city,
               const std::vector<MergedFareMarket*>& ibLeg,
               const bool endPoint);

  bool intersectOJLegs(const std::vector<MergedFareMarket*>& obLeg,
                       const std::vector<MergedFareMarket*>& ibLeg);

  bool isSamePointInFcc(bool& origMatch,
                        const MergedFareMarket& obOrigFM,
                        const MergedFareMarket& ibDestFM,
                        bool& destMatch,
                        const MergedFareMarket& obDestFM,
                        const MergedFareMarket& ibOrigFM);

  bool isValidFCforOJLeg(const MergedFareMarket& origFMkt, const MergedFareMarket& fm);

  bool isValidFCforCT(PU& ctPu,
                      std::vector<MergedFareMarket*>& fmktVect,
                      const MergedFareMarket& fm,
                      Directionality& dir,
                      bool& passedCTProvision,
                      bool& closed);
  bool isValidCT(PU& ctPu, const bool passedCTProvision);

  uint32_t getSurfaceMileage(const Loc& loc1, const Loc& loc2, DateTime travelDate);
  uint32_t getOJLegMileage(const std::vector<MergedFareMarket*>& leg,
                           const GeoTravelType geoTvlType,
                           DateTime travelDate);

  void setOWPUDirectionality(PUPath& puPath);
  void setIsIntlCTJourneyWithOWPU(const FareMarketPath& fmp, PUPath& puPath);
  void setPUGovCarrier(PU& pu);

  bool isInboundToNationZone210(const Loc& origin, const MergedFareMarket& fm);
  static bool isInboundToNetherlandAntilles(const Loc& origin, const MergedFareMarket& fm);

  bool isInboundToCountry(const Loc& puOrig, const MergedFareMarket& fm)
  {
    return isInboundToCountry(puOrig, *fm.origin(), *fm.destination());
  }
  bool isInboundToCountry(const Loc& puOrig, const Loc& orig, const Loc& dest);

  bool isOutBoundFromCountry(const Loc& puOrig, const MergedFareMarket& fm)
  {
    return isOutBoundFromCountry(puOrig, *fm.origin(), *fm.destination());
  }
  bool isOutBoundFromCountry(const Loc& puOrig, const Loc& orig, const Loc& dest);

  bool isWithinSameCountry(const Loc& loc1, const Loc& loc2);

  bool determineGeoTravelType(const MergedFareMarket& fm, GeoTravelType& geoTvlType);

  bool genMktPUPathCombination(std::vector<PUPath*>& puPathVect,
                               const uint16_t stIdx,
                               const uint16_t stCount,
                               std::vector<std::vector<PUPath*> >& mktPUPathVect,
                               std::vector<std::vector<PUPath*> >& mktPUPathCombVect);

  bool genPathPUPathCombination(
      std::map<MergedFareMarket*, std::vector<PUPath*> >& puPathComb,
      std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >::iterator it,
      std::map<MergedFareMarket*, std::vector<std::vector<PUPath*> > >& pathPUPathVect,
      std::vector<std::map<MergedFareMarket*, std::vector<PUPath*> > >& pathPUPathCombVect);

  void addPUPathsToMainMatrix(std::vector<PUPath*>& combinedPUPaths);

  bool isBuildPUPathComplete(const std::vector<PUPath*>& puPathVect);
  void limitFareMarketPath(std::vector<FareMarketPath*>& matrix);
  uint16_t fareBreakCount(FareMarketPath& fmp);

  void createMainTripSideTripLink(PUPath& puPath);

  bool checkTag2FareIndicator(const PU& pu);
  bool checkTag2FareIndicator(const std::vector<MergedFareMarket*>& fareMarketVect);

  void copyPUPath(PUPath& oldPath, PUPath& newPath);
  void copyPU(PU& oldPU, PU& newPU);

  void
  markIntlOWfromBrokenOJ(PU* oj, const MergedFareMarket* fm1, const MergedFareMarket* fm2) const;

  bool isSpecialEuropeanDoubleOJ(const Loc& obOrig,
                                 const Loc& obDest,
                                 const Loc& ibOrig,
                                 const Loc& ibDest);

  bool checkCarrierPrefForSPEuropeanDoubleOJ(const std::vector<MergedFareMarket*>& obLeg,
                                             const std::vector<MergedFareMarket*>& ibLeg,
                                             std::vector<CarrierCode>& invalidCxrForTOJ);

  bool checkCarrierPrefForSPEuropeanDoubleOJPerLeg(const std::vector<MergedFareMarket*>& leg,
                                                   std::vector<CarrierCode>& invalidCxrForTOJ);

  bool isSameAnyOneXPoint(const MergedFareMarket& obOrigFM,
                          const MergedFareMarket& ibDestFM,
                          const MergedFareMarket& obDestFM,
                          const MergedFareMarket& ibOrigFM);

  tse::ConfigMan* _config = nullptr;

  DataHandle _dataHandle;
  PricingTrx* _trx = nullptr;
  Itin* _itin = nullptr;
  ItinBoundary _itinBoundary = ItinBoundary::UNKNOWN;
  bool _useCxrPreference = true;
  bool _travelWithinScandinavia = false;

  std::vector<PUPath*> _puPathMatrix;

  // These mutexes are used only in PUPathMatrix.cpp during the
  // Multithreaded construction of PUPath
  boost::mutex _puPathMatrixMutex;

  static const uint16_t SIXTY = 60;
  static const uint16_t MAX_OJ_COMP = 9;
  static const uint16_t OPT_OJ_COMP = 3;
  static const uint16_t FMP_THRESHOLD = 1000;
  static const uint16_t FIVE_K = 5000;

  uint32_t _fareMarketPathCount = 0;
  uint32_t _maxPUPathPerFMP = 0;

  const FareCalcConfig* _fcConfig = nullptr;

  bool _avoidStaticObjectPool = true;
  boost::object_pool<PU> _puPool;
  boost::mutex _puPoolMutex;
  boost::object_pool<PUPath> _puPathPool;
  boost::mutex _puPathPoolMutex;
  uint16_t _brandIndex = INVALID_BRAND_INDEX;
  BrandCode* _brandCode = nullptr;
  flexFares::GroupId _flexFaresGroupId = 0;

private:
  TseThreadingConst::TaskId _taskId = TseThreadingConst::PUPATHMATRIX_TASK;

  typedef std::set<PU*, PU::PUPtrCmp> UniqueIntlSameNationOJ;
  UniqueIntlSameNationOJ findUniqueInternationalSameNationOJ() const;

  PUPathMatrix(const PUPathMatrix&) = delete;
  PUPathMatrix& operator=(const PUPathMatrix&) = delete;
  void setCTPUTurnAroundPoint(PU& ctPu);
};

typedef std::vector<PUPathMatrix*> PUPathMatrixVec;
typedef std::vector<PUPathMatrix*>::iterator PUPathMatrixVecI;
typedef std::vector<PUPathMatrix*>::const_iterator PUPathMatrixVecIC;
} // tse namespace
