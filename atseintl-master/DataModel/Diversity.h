/*
 * File:   Diversity.h
 * Author: Viktor Ostashevskyi
 *
 * Created on September 26, 2011, 3:57 AM
 */

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/DiversityModelType.h"
#include "DataModel/ItinIndex.h"

#include <boost/logic/tribool.hpp>
#include <boost/noncopyable.hpp>

#include <limits>
#include <utility>
#include <vector>

namespace tse
{
class DiagCollector;
class FareMarket;
class ShoppingTrx;

namespace shpq
{
class DiversityTestUtil;
}

class Diversity : private boost::noncopyable
{
public:
  friend class shpq::DiversityTestUtil;

  enum BucketType
  {
    GOLD,
    UGLY,
    LUXURY,
    JUNK,
    BUCKET_COUNT
  };
  enum NSBucketType
  {
    NSONLINE,
    NSINTERLINE,
    NSBUCKET_COUNT
  };

  struct ScheduleMarketData
  {
    std::vector<int> sopPerTOD;
    int32_t minFlightDuration = std::numeric_limits<int32_t>::max();
    int32_t maxFlightDuration = 0;
    uint16_t nonStopSOPs = 0;
    ScheduleMarketData(size_t todCount) : sopPerTOD(todCount, 0) {}
  };
  using ScheduleMarketDataStorage =
      std::map<const FareMarket*, std::map<CarrierCode, ScheduleMarketData>>;

  struct ScheduleCarrierData
  {
    uint16_t nonStopFlights = 0;
    uint16_t uniqueCnxnGoverningFlights = 0;
    uint16_t allGoverningFlights = 0; // don't have to be unique
    uint16_t onlineFlights = 0;
  };

  using ScheduleCarrierDataStorage = std::vector<std::map<CarrierCode, ScheduleCarrierData>>;

  struct AltDatesParam
  {
    uint16_t _fareLevelNumber = 3;
    std::size_t _optionsPerCarrier = 3;
    float _fareCutoffCoef = 4.;
    float _fareCutoffCoefDatePair = 1.5;

    AltDatesParam();
  };

  const static CarrierCode INTERLINE_CARRIER;

public:
  virtual ~Diversity() = default;

  void initialize(DiagCollector* dc,
                  ShoppingTrx& trx,
                  const std::map<ItinIndex::Key, CarrierCode>& carrierMap);
  void initializeV2(ShoppingTrx& trx);

  void setBucketDistribution(BucketType type, float value);
  float getBucketDistribution(BucketType type) const;

  void setEnabled() { _isEnabled = true; }
  bool isEnabled() const { return _isEnabled; }

  void setOCO() { _optionsCarrierOnly = true; }
  bool isOCO() const { return _optionsCarrierOnly; }

  bool hasDCL() const { return !_diversityCarrierList.empty(); }
  void setModel(DiversityModelType::Enum dmt) { _model = dmt; }
  DiversityModelType::Enum getModel() const { return _model; }

  bool isExchangeForAirlines() const { return _model == DiversityModelType::EFA; }
  bool isV2DiversityModel() const  { return _model == DiversityModelType::V2; }

  void setTODRanges(const std::vector<std::pair<uint16_t, uint16_t> >& value)
  {
    _todRanges = value;
  }
  const std::vector<std::pair<uint16_t, uint16_t> >& getTODRanges() const { return _todRanges; }
  void setFareCutoffCoef(float value) { _fareCutoffCoef = value; }
  float getFareCutoffCoef() const { return _fareCutoffCoef; }
  bool getFareCutoffCoefDefined() const { return !(_fareCutoffCoef < 1.); }
  void setNonStopOptionsPercentage(float value)
  {
    _nonStopOptionsPercentage = value;
  }
  float getNonStopOptionsPercentage() const { return _nonStopOptionsPercentage; }
  void setNonStopOptionsCount(std::size_t nso) { _nonStopOptionsCount = nso; }
  std::size_t getNonStopOptionsCount() const { return _nonStopOptionsCount; }
  void setInboundOutboundPairing(uint16_t value) { _inboundOutboundPairing = value; }
  uint16_t getInboundOutboundPairing() const { return _inboundOutboundPairing; }
  void setFareLevelNumber(uint16_t value) { _fareLevelNumber = value; }
  uint16_t getFareLevelNumber() const { return _fareLevelNumber; }
  void setFareAmountSeparator(MoneyAmount value) { _fareAmountSeparator = value; }
  MoneyAmount getFareAmountSeparator() const { return _fareAmountSeparator; }
  void setFareAmountSeparatorCoef(float value) { _fareAmountSeparatorCoef = value; }
  float getFareAmountSeparatorCoef() const { return _fareAmountSeparatorCoef; }
  void setTravelTimeSeparator(int32_t value) { _travelTimeSeparator = value; }
  int32_t getTravelTimeSeparator() const { return _travelTimeSeparator; }
  void setTravelTimeSeparatorCoef(float value) { _travelTimeSeparatorCoef = value; }
  float getTravelTimeSeparatorCoef() const { return _travelTimeSeparatorCoef; }
  bool getTravelTimeSeparatorCoefDefined() const { return !(_travelTimeSeparatorCoef < 1.0); }
  void setTODDistribution(const std::vector<float>& value) { _todDistribution = value; }
  const std::vector<float>& getTODDistribution() const { return _todDistribution; }
  MoneyAmount getFareCutoffAmount() const { return _fareCutoffAmount; }
  void setFareCutoffAmount(MoneyAmount value) { _fareCutoffAmount = value; }

  MoneyAmount getFareCutoffAmountExpandPreferredCarrier() const { return _fareCutoffAmountEPC; }
  void setFareCutoffAmountExpandPreferredCarrier(MoneyAmount value) { _fareCutoffAmountEPC = value; }

  bool isOptionsPerCarrierInPercents() const { return _optionsPerCarrierDefault < 0.0; }
  void setOptionsPerCarrierDefault(float value) { _optionsPerCarrierDefault = value; }
  float getOptionsPerCarrierDefault() const { return _optionsPerCarrierDefault; }
  void setFlightRepeatLimit(uint16_t value) { _flightRepeatLimit = value; }
  uint16_t getFlightRepeatLimit() const { return _flightRepeatLimit; }
  void setPreferredCarriers(const std::vector<CarrierCode>& value) { _preferredCarriers = value; }
  const std::vector<CarrierCode>& getPreferredCarriers() const { return _preferredCarriers; }
  void setOptionsPerCarrier(const CarrierCode& cxr, float value)
  {
    _optionsPerCarrier[cxr] = value;
  }
  float getOptionsPerCarrier(const CarrierCode& cxr) const
  {
    std::map<CarrierCode, float>::const_iterator valueIt = _optionsPerCarrier.find(cxr);
    if (valueIt != _optionsPerCarrier.end())
      return valueIt->second;
    return 0.0;
  }
  bool getNonStopOptionsPerCarrierEnabled() const { return _isNonStopOptionPerCarrierEnabled; }
  const std::map<CarrierCode, float>& getOptionsPerCarrierMap() const { return _optionsPerCarrier; }
  const std::map<CarrierCode, size_t>& getDCLMap() const { return _diversityCarrierList; }
  void setCarrierMap(const std::map<ItinIndex::Key, CarrierCode>& value) { _carrierMap = value; }
  const std::map<ItinIndex::Key, CarrierCode>& getCarrierMap() const { return _carrierMap; }

  void setHighDensityMarket(bool highDensityMarket) { _highDensityMarket = highDensityMarket; }

  boost::tribool isHighDensityMarket() const { return _highDensityMarket; }

  const ScheduleMarketDataStorage& getScheduleMarketDataStorage() const { return _smdStorage; }

  size_t getNumberOfOptionsToGenerate() const { return _numberOfOptionsToGenerate; }

  const AltDatesParam& getAltDates() const { return _altDatesParam; }

  void printScheduleInfo(DiagCollector* dc, const FareMarket* fm, const CarrierCode& govCxr) const;

  bool isOnlineNonStopCapable() const { return (_maxOnlineNonStopCount > 0); }
  bool isInterlineNonStopCapable() const { return _maxInterlineNonStopCount > 0; }
  bool isDirectCarriersCapable() const { return !_directOptionsCarriers.empty(); }

  bool isAdditionalNonStopsEnabled() const
  {
    return _nonStopOptionsCount > 0 || _nonStopOptionsPercentage > EPSILON;
  }

  size_t getMaxOnlineNonStopCount() const { return _maxOnlineNonStopCount; }
  size_t getMaxRCOnlineOptionsCount() const { return _maxRCOnlineOptionsCount; }
  size_t getMaxInterlineNonStopCount() const { return _maxInterlineNonStopCount; }
  size_t getMaxNonStopCount() const { return _maxOnlineNonStopCount + _maxInterlineNonStopCount; }
  size_t getMaxNonStopCountFor(CarrierCode cxr) const;
  const std::map<CarrierCode, size_t>& getMaxNonStopCountPerCarrier() const
  {
    return _maxNonStopCountPerCarrier;
  }

  int16_t getNumOfCustomSolutions() const { return _numOfCustomSolutions; }
  void setNumOfCustomSolutions(int16_t cus) { _numOfCustomSolutions = cus; }

  int16_t getMinimalNumCustomSolutions() const { return _minimalNumCustomSolutions; }

  uint32_t getMaxLongConnectionSolutions() const { return _maxLongConnectionSolutions; }
  void setMaxLongConnectionSolutions(uint32_t num) { _maxLongConnectionSolutions = num; }

  const std::set<CarrierCode>& getDirectOptionsCarriers() const { return _directOptionsCarriers; }

  bool shouldConsiderCarrierForNonStop(CarrierCode cxr) const;
  bool shouldUseInterlineNonStops() const { return _shouldUseInterlineNonStops; }
  bool shouldGenerateAllNonStopFlightOnlySolutions() const
  {
    return _shouldGenerateAllNonStopFlightOnlySolutions;
  }

  CarrierCode detectCarrier(ItinIndex::Key cxrKey) const;

  /**
   * Update max number of nonstops with booking code validation results
   */
  bool updateNSBookingCodeInfo(const ShoppingTrx& trx);
  void setDiversityCarrierList(const CarrierCode& cxr, size_t value)
  {
    _diversityCarrierList[cxr] = value;
  }

protected:
  /**
   * Keep this index to update max number of nonstops
   * with booking code validation results.
   * The only need of this interface is to stub it in unit tests.
   */
  struct NonStopIndex
  {
    virtual void
    addNS(LegId legIdx, CarrierCode carrier, int origSopId, const ShoppingTrx& trx) = 0;

    virtual void addNS(const ShoppingTrx& trx) = 0;

    virtual bool updateBookingCodeInfo(const ShoppingTrx& trx) = 0;

    virtual void calcStatPutResultsTo(size_t& outMaxOnlineNSCount,
                                      size_t& outMaxInterlineNSCount,
                                      std::map<CarrierCode, size_t>& outMaxNSPerCarrier) const = 0;

    virtual ~NonStopIndex() = default;
  };

  virtual NonStopIndex* createNonStopIndex(const ShoppingTrx& trx);

private:
  class NonStopIndexImpl;

  void collectScheduleMarketData(const ShoppingTrx& trx);
  void collectScheduleCarrierData(const ShoppingTrx& trx, ScheduleCarrierDataStorage& scdStorage);
  void collectOnlineCarriers(DiagCollector* dc,
                             const ShoppingTrx& trx,
                             const ScheduleCarrierDataStorage& scdStorage,
                             std::set<CarrierCode>& onlineCarriers);
  void calculateMaxRCOnlineOptions(DiagCollector* dc,
                                   const ShoppingTrx& trx,
                                   const ScheduleCarrierDataStorage& scdStorage);
  void calculateMarketSize(DiagCollector* dc,
                           const ShoppingTrx& trx,
                           const ScheduleCarrierDataStorage& scdStorage,
                           const std::set<CarrierCode>& onlineCarriers);
  void calculateMarketSizeFromOnlineCarriers(DiagCollector* dc,
                                             const ShoppingTrx& trx,
                                             const ScheduleCarrierDataStorage& scdStorage,
                                             const std::set<CarrierCode>& onlineCarriers);
  void setupNumberOfOptionsToGenerate(DiagCollector* dc, const ShoppingTrx& trx);
  void setupNumberOfOptionsPerCarrierDefault(DiagCollector* dc);
  void setupCarriersRequirements(const std::set<CarrierCode>& onlineCarriers,
      const ShoppingTrx& trx);
  void setupHDNSMarket(DiagCollector* dc,
                       const ShoppingTrx& trx,
                       const ScheduleCarrierDataStorage& scdStorage);
  void setupDirectCarriers(DiagCollector* dc,
                           const std::set<CarrierCode>& onlineCarriers,
                           const ScheduleCarrierDataStorage& scdStorage);
  void setupNonStopOptionsPerCarrier(DiagCollector* dc, const ShoppingTrx& trx);

  void setupNumOfCustomSolutions(DiagCollector* dc, const ShoppingTrx& trx);
  void setupCustomSolutionsRatio(DiagCollector* dc, const ShoppingTrx& trx);

  void setupMaxLongConnectionSolutions(DiagCollector* dc, const ShoppingTrx& trx);

  void setupNonStopOptionsCount(DiagCollector* dc, const ShoppingTrx& trx);
  void printSopsInfo(DiagCollector* dc, const ShoppingTrx& trx) const;
  void printDCL(DiagCollector* dc) const;

  void cleanupCarriersOnDCLMap(DiagCollector* dc);
protected:
  std::map<ItinIndex::Key, CarrierCode> _carrierMap;
  std::map<CarrierCode, float> _optionsPerCarrier;
  std::set<CarrierCode> _directOptionsCarriers;
  std::map<CarrierCode, size_t> _maxNonStopCountPerCarrier;
  std::map<CarrierCode, size_t> _diversityCarrierList;
  ScheduleMarketDataStorage _smdStorage;
  std::vector<std::pair<uint16_t, uint16_t> > _todRanges;
  std::vector<float> _todDistribution;
  std::vector<CarrierCode> _preferredCarriers;
  DiversityModelType::Enum _model = DiversityModelType::Enum::DEFAULT;
  MoneyAmount _fareAmountSeparator = 0.0;
  MoneyAmount _fareCutoffAmount = 0.0;
  MoneyAmount _fareCutoffAmountEPC = 0.0;
  float _fareCutoffCoef = 0.0;
  float _nonStopOptionsPercentage = 0.0f;
  size_t _nonStopOptionsCount = 0;
  float _fareAmountSeparatorCoef = 0.0;
  float _travelTimeSeparatorCoef = 0.0;
  float _bucketDistribution[BUCKET_COUNT] = {0.65, 0.15, 0.20, 0.0};
  float _optionsPerCarrierDefault = 1.0;
  boost::tribool _highDensityMarket = boost::indeterminate;
  size_t _maxOnlineNonStopCount = std::numeric_limits<size_t>::max();
  size_t _maxInterlineNonStopCount = std::numeric_limits<size_t>::max();
  size_t _maxRCOnlineOptionsCount = 0;
  bool _shouldUseInterlineNonStops = false;
  bool _shouldGenerateAllNonStopFlightOnlySolutions = false;
  int32_t _travelTimeSeparator = 0;
  uint32_t _numberOfOptionsToGenerate = 0;
  size_t _marketSize = 0;
  size_t _minOptionsCountForMarketSizeCalculation = 50;
  uint16_t _inboundOutboundPairing = 5;
  uint16_t _fareLevelNumber = 0;
  uint16_t _flightRepeatLimit = 0;
  const AltDatesParam _altDatesParam;
  bool _isNonStopOptionPerCarrierEnabled = false;
  bool _isEnabled = false;
  NonStopIndex* _nonstopIndex = nullptr;
  int16_t _numOfCustomSolutions = 0;
  int16_t _minimalNumCustomSolutions = 0;
  uint32_t _maxLongConnectionSolutions = 0;
  bool _optionsCarrierOnly = false;
};
}
