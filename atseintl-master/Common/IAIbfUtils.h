//-------------------------------------------------------------------
//
//  Authors:    Michal Mlynek
//
//  Copyright Sabre 2013
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

#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"

namespace tse
{
class IAIbfUtils
{
public:
  typedef std::pair<LocCode, LocCode> ODPair;

  enum TripType
  { ROUNDTRIP,
    OPENJAW,
    ONEWAY };

  struct OdcTuple
  {
    OdcTuple(const CarrierCode& carrierCode, const LocCode& origin,
             const LocCode& destination);

    OdcTuple(const CarrierCode& carrierCode, const ODPair odPair);

    LocCode origin;
    LocCode destination;
    CarrierCode governingCarrier;

    bool operator<(const OdcTuple& b) const;
  };

  typedef std::map<OdcTuple, std::vector<FareMarket*> > FMsForBranding;
  typedef std::map<FareMarket*, std::set<OdcTuple> > OdcsForBranding;
  typedef std::pair<OdcTuple, std::vector<FareMarket*> > FMsForBrandingPair;

  struct OdDataForFareMarket
  {
    ODPair fmOD;
    ODPair legOD;
    ODPair tripOD;
    ODPair significantLegOD;
    bool isWithinLeg;
  };

  struct ItinHasBrands : public std::unary_function<const Itin*, bool>
  {
    ItinHasBrands() {}
    bool operator()(const Itin* itin) const { return !(itin->brandCodes().empty()); }
  };

  struct ItinHasBrandsOnAllSegments : public std::unary_function<const Itin*, bool>
  {
    ItinHasBrandsOnAllSegments() {}
    bool operator()(const Itin* itin) const;
  };

  // Checks that trip is Open Jaw according to IBF criteria:
  // Arunkc cannot spread across countriesi and in case of OOJ the trip has to be international
  static bool isTripOpenJaw(const Itin* itin, DataHandle& dataHandle);

  static TripType calculateTripType(const PricingTrx& trx);

  // According to IBF US/VI.PR is same country, Scandinavia is same country and RU/XU is same
  // country
  // Function doesn't check if was is sent to it is indeed an arunk. Pass only arunk to it.
  static bool hasArunkThatSpreadsAcrossCountries(const std::vector<TravelSeg*>& arunks);

  // For the purpose of calculating trip type we need to have an itin comprising travel segments of
  // a whole trip
  // In MIP every itin will do, in IS we need a journey itin
  // because all the rest contain only segments of one leg
  static const Itin* getRepresentativeItin(const PricingTrx& trx);

  // Returns origin and destination from the Itin data structure
  // in MIP it will equal to the trip O&D
  // in IS is equals Leg's O&D
  static ODPair getODPairFromItin(const Itin& itin);

  /* Function that creates ODC Tuples according to the following requirements:

  - If BrandRetrievalMode is PER_O_AND_D:

  FareMarket          OneWay                              RT,OJ,CT
  ----------          ------                              ---------
  Matches Leg         Trip O&D + Leg O&D                  Leg O&D
  Matcesh Multi Legs  Trip O&D + O 1st,D Last             O 1st, D Last
  Extends Leg         Trip O&D + Significant Leg O&D      Significant O&D
  Within Leg          Trip O&D + Leg O&D                  Leg O&D

  Refer to IBF SRS for more info

  - If BrandRetrievalMode is PER_FARE_COMPONENT
  then origin and destination from a given fare market is used
  */
  static void fillOdcTupleVec(std::vector<OdcTuple>& odcTupleVec,
                              const OdDataForFareMarket& odData,
                              const CarrierCode& govCarrier,
                              const TripType tripType,
                              const BrandRetrievalMode mode);

  static const TravelSeg* getPrimarySector(const FareMarket* fm);

  // This function should be used only in IS
  static void fillOdDataForFareMarketShopping(OdDataForFareMarket& odDataForFm,
                                              const Itin& currentItin,
                                              const Itin& journeyItin);

  // This function should be used only in MIP and Pricing
  static void fillOdDataForFareMarketPricing(const Itin* itin,
                                         FareMarket* fm,
                                         const IAIbfUtils::TripType tripType,
                                         const BrandRetrievalMode mode,
                                         std::vector<OdcTuple>& odcTupleVec,
                                         PricingTrx& trx);

  // This function should be used only in MIP and Pricing
  static void findAllOdDataForMarketPricing(const Itin* itin,
                                            FareMarket* fm,
                                            const IAIbfUtils::TripType tripType,
                                            const BrandRetrievalMode mode,
                                            PricingTrx& trx,
                                            std::function<void (OdcTuple&)> function);

  // Helper function for fillOdDataForFareMarketPricing
  static std::set<ODPair> calculateODPairsForFareMarket(const FareMarket& fareMarket,
                                                        const Itin& itin,
                                                        const IAIbfUtils::TripType tripType);

  static void getSegmentMap(const Itin* itin, std::map<const TravelSeg*, uint16_t>& segmentMap);

  static void invertOdcToFmMap(const FMsForBranding& inputMap, OdcsForBranding& result);

  static std::string legInfoToString(const ItinLegs& itinLegs);

}; // End IAIbfUtils

// Works on a reference to a vector of items.
// Splits the vector provided into ranges.
// returns those ranges iteratively in a form of indexes of first and last elements in range
// returns (-1,-1) if no more ranges left to return
// Does not modify the range provided in any way.
template <typename T>
class VectorIntoRangeSplitter
{
public:
  typedef std::pair<int, int> IndexRange;
  typedef typename std::function<bool(T, T)> RangeEndPredicate;

  VectorIntoRangeSplitter(const std::vector<T>& input, bool collectDebugMessages)
     : _originalVector(input), _debug(collectDebugMessages),_currentIndex(0)
     {
       if (_debug)
       {
         std::ostringstream ss;
         ss << "SPLITTER INITIALIZED WITH " << _originalVector.size() << " ITEMS\n";
         _debugMsg.push_back(ss.str());
       }
     }
  void reset() { _currentIndex = 0; if (_debug){_debugMsg.push_back("RESET\n");}}

  void addPredicate(RangeEndPredicate predicate) { _predicates.push_back(predicate);}

  IndexRange getNextRange()
  {
    if (!hasMore())
      return logOnReturn(std::make_pair(-1, -1));

    const auto rangeStart = _currentIndex;
    while (++_currentIndex, hasMore())
    {
      for (RangeEndPredicate& predicate : _predicates)
        if (predicate(_originalVector[_currentIndex-1], _originalVector[_currentIndex]))
          return logOnReturn(std::make_pair(rangeStart, _currentIndex-1));
    }
    return logOnReturn(std::make_pair(rangeStart, _currentIndex-1));
  }

  bool hasMore() const { return _currentIndex < _originalVector.size(); }

  IndexRange logOnReturn(IndexRange value)
  {
    if (!_debug) { return value; }
    std::ostringstream ss;
    ss << "NEXT RANGE: " << value.first << "-" << value.second << std::endl;
    _debugMsg.push_back(ss.str());
    return value;
  }

  const std::vector<std::string>& getDebugMessages() const { return _debugMsg; }

private:
  std::vector<RangeEndPredicate> _predicates;
  const std::vector<T>& _originalVector;
  bool _debug;
  size_t _currentIndex;
  std::vector<std::string> _debugMsg;
};  // End VectorIntoRangeSplitter

std::ostream& operator<<(std::ostream& stream, const IAIbfUtils::TripType tripType);
std::ostream& operator<<(std::ostream& stream, const IAIbfUtils::OdcTuple& odcTuple);

} // namespace tse

