// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Pricing/PaxTypeFareBitmapValidator.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/PQ/ASOCandidateChecker.h"
#include "Pricing/Shopping/PQ/ScopedFMGlobalDirSetter.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"

#include <map>
#include <vector>

#include <stdint.h>

namespace tse
{

class BitmapOpOrderer;
class Diag941Collector;
class DiversityModel;
class FarePath;
class GroupFarePath;
class InterlineTicketCarrier;
class Itin;
class ItinStatistic;
class Logger;
class PaxTypeFare;
class ScopedFMGlobalDirSetter;
class ShoppingTrx;
class SOPInfo;
class Hasher;

namespace shpq
{
class SoloGroupFarePath;
class SoloPQ;
class SoloPQItem;
class SoloTrxData;
class SOPCombinationBuilder;
}

class SoloItinGenerator
{

  struct FamilyGroupingInfo
  {
    typedef std::pair<bool, bool> Statuses;
    typedef std::pair<FarePath*, Statuses> SolutionInfo;
    typedef std::map<FarePath*, SolutionInfo> BaseFarePathMap;
    typedef std::map<FarePath*, bool> OnlyThruFlagMap;

    FarePath* getBaseFarePath(FarePath* fp)
    {
      if (LIKELY(_baseFarePathMap.count(fp)))
        return _baseFarePathMap[fp].first;
      return nullptr;
    }

    bool getAsoStatus(FarePath* fp)
    {
      if (LIKELY(_baseFarePathMap.count(fp)))
        return _baseFarePathMap[fp].second.first;
      return false;
    }

    bool getHintStatus(FarePath* fp)
    {
      if (LIKELY(_baseFarePathMap.count(fp)))
        return _baseFarePathMap[fp].second.second;
      return false;
    }

    void addBaseFarePath(FarePath* baseFarePath,
                         FarePath* duplicatedFarePath,
                         bool asoCandidate = false,
                         bool hint = false)
    {
      _baseFarePathMap[duplicatedFarePath] =
          std::make_pair(baseFarePath, std::make_pair(asoCandidate, hint));
    }

    void setOnlyThruFlag(FarePath* baseFarePath, bool thruOnly)
    {
      _onlyThruFlagMap[baseFarePath] = thruOnly;
    }

    bool isOnlyThruFlag(FarePath* baseFarePath)
    {
      if (LIKELY(_onlyThruFlagMap.count(baseFarePath)))
        return _onlyThruFlagMap[baseFarePath];
      return false;
    }

  private:
    BaseFarePathMap _baseFarePathMap;
    OnlyThruFlagMap _onlyThruFlagMap;
  };

public:
  typedef std::vector<uint32_t> CarrierKeyVec;
  typedef std::vector<shpq::SopIdxVec> VectorOfSopVec;
  typedef std::pair<shpq::SopIdxVec, GroupFarePath*> SolutionItem;
  typedef std::vector<SolutionItem> SolutionVector;
  typedef std::map<FarePath*, SolutionVector> SimilarSolutionsMap;

  SoloItinGenerator(shpq::SoloTrxData& soloTrx,
                    shpq::SoloPQ& pq,
                    DiversityModel* dm,
                    const ItinStatistic& stats,
                    BitmapOpOrderer& bitmapOpOrderer);
  ~SoloItinGenerator();
  void generateSolutions(const shpq::SoloPQItemPtr& pqItem, shpq::SoloTrxData& trxData);
  void generateEstimatedSolutions();
  void flushDiag();

private:
  class ValidateCarrierRestrictionsPtfVisitor;
  bool checkFaresValidForCxr(const FarePath& fp, const LegId legId, const uint32_t cxrKey) const;
  void generateSolutionsImpl(const shpq::SoloPQItemPtr& pqItem,
                             const shpq::CxrKeyPerLeg& cxrKey,
                             shpq::SoloTrxData& soloTrxData);
  void
  initializePricingUnitsSegments(FarePath* farePath,
                                 const SoloItinGenerator::CarrierKeyVec& carrierKeys,
                                 shpq::SopIdxVecArg sopVec,
                                 shpq::SopIdxVecArg oSopVec,
                                 std::vector<ScopedFMGlobalDirSetterPtr>& globalDirSetters) const;

  bool validateCarrierRestrictions(std::vector<FareUsage*>& fareUsages,
                                   shpq::SopIdxVecArg oSopVec,
                                   const Itin* itin) const;

  bool checkAvailibility(const std::vector<TravelSeg*>& tSegs,
                         const std::vector<BookingCode>& bookingCodes,
                         const uint16_t segIndex) const;

  void getBookingCodes(const std::vector<int>& oSopVec,
                       const std::vector<TravelSeg*>& tSegs,
                       const PaxTypeFare* ptf,
                       const uint16_t legIdx,
                       std::vector<BookingCode>& bookingCodes) const;

  uint16_t flowLength(const std::vector<TravelSeg*> tSegs,
                      uint16_t startId,
                      const std::vector<BookingCode>& bookingCodes) const;

  bool isLocalJourneyCarrier(const std::vector<TravelSeg*>& tSegs) const;
  bool validateJourneyRestrictions(const std::vector<FareUsage*>& fareUsages,
                                   const Itin* itin,
                                   const std::vector<int>& oSopVec) const;

  bool validateMaximumPenalty(FarePath& farePath) const;

  /**
   * validateItin() can adjust FarePath totalNUCAmount,
   * so then it can happen that Diversity will reject solution for this FP as not needed
   */
  FarePath* validateItin(const FarePath* fp,
                         const CarrierKeyVec& carrierKeys,
                         shpq::SopIdxVecArg sopVec,
                         shpq::SopIdxVecArg oSopVec) const;
  bool addSolution(const DiversityModel::SOPCombination& sopComb,
                   FarePath* origFP,
                   FarePath* newFP,
                   const shpq::SoloPQItemPtr& pqItem,
                   std::size_t fpKey);

  shpq::SoloGroupFarePath* buildFakeGroupFarePath(FarePath* fp,
                                                  const shpq::SoloPQItemPtr& origPQItem,
                                                  size_t fpKey,
                                                  const DatePair* = nullptr) const;
  void setProcessThruOnlyHint(shpq::SopIdxVecArg sopVec, shpq::SoloGroupFarePath* gfp);
  bool validateInterlineTicketCarrierAgreement(shpq::SopIdxVecArg sopVec, const Itin& itin) const;
  ScopedFMGlobalDirSetter*
  createGlobalDirSetter(PaxTypeFare* fare, uint32_t legIndex, int sopIndex) const;
  void groupFPSolutions(SimilarSolutionsMap& solutionsMap, uint32_t familyGroupingOption);
  bool validateDelayedFlightStatuses(
      const CarrierKeyVec& carrierKeys,
      shpq::SopIdxVecArg bitVec,
      const std::vector<std::vector<SOPInfo> >& sopInfos,
      const DiversityModel::SOPCombination& combination,
      size_t& failedLegIdx,
      PaxTypeFareBitmapValidator::SkippedBitValidator* skippedBitValidator);
  bool isHintAllowed();

  shpq::SOPCombinationBuilder* createSOPCombinationBuilder(DataHandle& dh,
                                                           const shpq::SoloPQItemPtr& pqItem,
                                                           const shpq::CxrKeyPerLeg& cxrKey);

  void initializeDiagnostic();
  void initializeAcmsVars();

  void initializeDiagHashFunc();
  size_t hash_value(const FarePath& fp) const;
  void hash_carriers(const PaxTypeFare& ptf, Hasher& hasher) const;

private:
  enum FPHashFuncVersion
  {
    FP_HASH_FUNC_VER_1 = 1, // to investigate discrepancies vs previous releases
    FP_HASH_FUNC_VER_2 = 2
  };

  ShoppingTrx& _trx;
  shpq::SoloPQ& _pq;
  DiversityModel* _dm;
  const ItinStatistic& _stats;
  BitmapOpOrderer& _bitmapOpOrderer;
  Diag941Collector* _dc;
  FPHashFuncVersion _fpHashFuncVer;
  InterlineTicketCarrier* _interlineTicketCarrierData;
  FamilyGroupingInfo _familyGroupingInfo;
  double _thruFarePopulationRateLimit;
  bool _changeFamilyGroupingForThruOnly;
  shpq::ASOCandidateChecker _asoCandidateChecker;
  uint32_t _hintingStartsAt;
  uint32_t _initialSolutionsCount; // number of solution in flight matrix before processing current
                                   // FP
  uint32_t _maxFailedCombPerFP;
  uint32_t _maxFailedComb;
  uint32_t _failedCombCount;
  uint32_t _maxFailedDelayedBitValidationCountPerFP;
  static Logger _logger;

  // This is needed for isAlreadyGenerated check, will work okay as far as we are not multithreaded
  mutable std::vector<int> _sopIdxStdVecAdapter;
};

} /* namespace tse */
