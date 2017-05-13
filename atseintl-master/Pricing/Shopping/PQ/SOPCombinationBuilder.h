// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <boost/utility.hpp>

#include <map>
#include <memory>
#include <vector>

namespace tse
{
class Diag941Collector;
class FarePath;
class SOPInfo;

namespace shpq
{

class SOPCombinationBuilder : private boost::noncopyable
{
public:
  static constexpr char INVALID_STATUS = 'N';

  typedef std::vector<uint32_t> CarrierKeyVec;
  typedef std::map<DateTime, std::set<int> > SopsByDate;
  typedef std::vector<SopsByDate> SopsByDateByLeg;
  typedef std::vector<std::vector<SOPInfo> > SOPInfos;

  SOPCombinationBuilder(const SoloPQItemPtr& pqItem,
                        const shpq::CxrKeyPerLeg& cxrKey,
                        bool processDirectFlightsOnly,
                        DiversityModel& dm,
                        Diag941Collector* dc,
                        ShoppingTrx& trx);

  ~SOPCombinationBuilder() {}

  DiversityModel::SOPCombinationList& getOrCreateSOPCombinationList();

  // The next data can be accessed once getOrCreateSOPCombinationList() has been called
  const CarrierKeyVec& getCxrKeys() const { return _carrierKeys; }
  const SOPInfos& getSOPInfos() { return _sopInfos; }
  SopIdxVec getFlbIdxVec(SopIdxVecArg intSopVec) const;
  //

  SopsByDateByLeg& getSopsByDateByLeg() { return _sopsByDateByLeg; }

protected:
  void detectCarriers();
  void collectSOPInfo(); // implies detectCarriers()
  void collectApplicableCombinations();
  void collectSOPInfoZones();

  // those checks are applied to SOPCombination which is being created
  bool isAlreadyGenerated(SopIdxVecArg oSopVec) const;
  bool isInterlineSolution(SopIdxVecArg sopVec) const;
  bool checkMinConnectionTime(SopIdxVecArg sopVec) const;

  void addCombination(const DiversityModel::SOPCombination& comb, const shpq::SopIdxVec& sopVec);

  const SOPInfos& getSOPInfosForCombinations() const;

  // input:
  SoloPQItemPtr _pqItem;
  FarePath& _farePath;
  const bool _processDirectFlightsOnly;
  DiversityModel& _dm;
  Diag941Collector* const _dc;
  ShoppingTrx& _trx;

  // output:
  CarrierKeyVec _carrierKeys;
  SOPInfos _sopInfos;
  std::unique_ptr<SOPInfos> _sopInfosFiltered;
  std::map<SopIdxVec, SopIdxVec> _oSopVecToSopVec;
  bool _isCombListCreated;

  std::vector<SopsByDate> _sopsByDateByLeg;

  DiversityModel::SOPCombinationList& getCombinations() { return _combinations; }

private:
  std::size_t detectEndIdxForNonStop(const ShoppingTrx& trx,
                                     uint16_t legIndex,
                                     const SOPUsages& sopUsages) const;
  bool
  validateSOPCombForAltDates(DiversityModel::SOPCombination& sopComb, const SOPInfo& sopInfo) const;

  void printThrownCombinations(const std::vector<SOPInfo>& inboundSops,
                               const std::vector<SOPInfo>* outboundSops);

  void collectApplicableCombinationsImpl(const std::vector<std::vector<SOPInfo>*>& sopInfos);
  void checkAddApplicableCombination(const std::vector<SOPInfo>& combination,
                                     shpq::SopIdxVec& sopVec,
                                     bool isRoundTrip);

  bool isCombinationCabinValid(const std::vector<SOPInfo>& combination, bool isRoundTrip) const;
  bool isCombinationValid(DiversityModel::SOPCombination& combination) const;

  typedef std::vector<SOPInfo> SOPInfoVec;
  typedef std::map<uint16_t, SOPInfoVec> SOPZoneMap;

  class SOPInfoZone
  {
  public:
    uint16_t queueRank() const { return _queueRank; }
    uint16_t& queueRank() { return _queueRank; }
    std::vector<SOPInfo> sopInfos() const { return _sopInfos; }
    std::vector<SOPInfo>& sopInfos() { return _sopInfos; }
  private:
    uint16_t _queueRank;
    std::vector<SOPInfo> _sopInfos;
  };

  void flush(SOPZoneMap& sopZoneMapIn, std::vector<SOPInfoZone>& sopInfoZonesLeg);

  std::vector<std::vector<SOPInfoZone> > _sopInfoZones;

  DiversityModel::SOPCombinationList _combinations;

  // This is needed for isAlreadyGenerated check, will work okay as far as we are not multithreaded
  mutable std::vector<int> _sopIdxStdVecAdapter;
};

} /* ns shpq */
} /* ns tse */
