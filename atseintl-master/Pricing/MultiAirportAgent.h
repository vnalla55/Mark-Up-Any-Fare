#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/FPPQItem.h"

namespace tse
{
class TravelSeg;

class MultiAirportAgent
{
public:
  void perform();
  bool isAirportSolution(const SopIdVec& sopVec) const;
  void init(ShoppingTrx* trx, const LocCode& city, const LocCode& airport, const bool& atOrigin);

private:
  bool existsAirportSolution(ShoppingPQ& pq) const;
  bool possibleAirportSolution(const CarrierCode* const carrier) const;
  bool possibleAirportSolution(const CarrierCode* const carrier,
                               const ShoppingTrx::Leg& leg,
                               const bool& oppositePoint) const;
  void addAirportsolution(ShoppingPQ& pq) const;
  bool isAirportSolution(const ShoppingTrx::SchedulingOption& sop, const bool& oppositePoint) const;
  bool isAirportSolution(const Loc* const loc) const;
  void findMinMaxFamily(const ShoppingPQ& pq,
                        ShoppingTrx::FlightMatrix::const_iterator& minFam,
                        ShoppingTrx::FlightMatrix::const_iterator& maxFam,
                        bool& minFound,
                        bool& maxFound) const;
  ShoppingTrx::SchedulingOption& getSop(const SopIdVec& sopVec, const int i) const;
  const std::vector<TravelSeg*>& getTSvec(const ShoppingTrx::SchedulingOption& sop) const;
  bool isFrontAirport(const std::vector<TravelSeg*>& tsVec) const;
  bool isBackAirport(const std::vector<TravelSeg*>& tsVec) const;
  void logDebugInfo(const ShoppingPQ& pq, const int& fms, const int& efms) const;
  void logDebugInfoMatrix(const ShoppingPQ& pq) const;
  void logDebugInfoFlightMatrix(const ShoppingTrx::FlightMatrix& matrix) const;
  void logDebugInfoEstimateMatrix(const ShoppingTrx::EstimateMatrix& matrix) const;
  void logDebugInfoSolution(const SopIdVec& sopVec,
                            GroupFarePath* const gfp,
                            const SopIdVec* const parentSopVec = nullptr) const;
  std::string getGFPinfo(GroupFarePath* const gfp) const;
  std::string getSopDebugInfo(const ShoppingTrx::SchedulingOption& sop) const;
  std::string getFPinfo(FPPQItem* const item) const;
  std::string getFPinfo(PricingUnit* const pu) const;
  std::string getSopVecInfo(const SopIdVec& sopVec) const;
  int
  getFamilySize(const ShoppingPQ& pq, const ShoppingTrx::FlightMatrix::const_iterator fit) const;
  void removeFamily(ShoppingPQ& pq, const ShoppingTrx::FlightMatrix::value_type& v) const;

  ShoppingTrx* _trx;
  LocCode _city;
  LocCode _airport;
  bool _atOrigin;
};
}
