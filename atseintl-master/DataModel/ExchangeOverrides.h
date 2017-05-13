//-------------------------------------------------------------------
//  Copyright Sabre 2007
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
#include "DataModel/FareUsage.h"
#include "DataModel/MinFarePlusUp.h"

namespace tse
{
class BaseExchangeTrx;
class FarePath;
class MileageTypeData;
class SurchargeData;
class TravelSeg;

class SurchargeOverride final
{
public:
  SurchargeOverride() = default;
  SurchargeOverride(const SurchargeOverride&) = delete;
  SurchargeOverride& operator=(const SurchargeOverride&) = delete;

  void initialize(SurchargeData& surchargeData) const;

  Indicator& type() { return _type; }
  const Indicator& type() const { return _type; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  CurrencyCode& currency() { return _currency; }
  const CurrencyCode& currency() const { return _currency; }

  TravelSeg*& travelSeg() { return _travelSeg; }
  const TravelSeg* travelSeg() const { return _travelSeg; }

  bool& fromExchange() { return _fromExchange; }
  const bool fromExchange() const { return _fromExchange; }

  bool& removed() { return _removed; }
  const bool removed() const { return _removed; }

  bool& singleSector() { return _singleSector; }
  const bool& singleSector() const { return _singleSector; }

  LocCode& fcBrdCity() { return _fcBrdCity; }
  const LocCode& fcBrdCity() const { return _fcBrdCity; }

  LocCode& fcOffCity() { return _fcOffCity; }
  const LocCode& fcOffCity() const { return _fcOffCity; }

  bool& fcFpLevel() { return _fcFpLevel; }
  const bool& fcFpLevel() const { return _fcFpLevel; }

private:
  MoneyAmount _amount = 0;
  TravelSeg* _travelSeg = nullptr;
  LocCode _fcBrdCity;
  LocCode _fcOffCity;
  CurrencyCode _currency;
  bool _fromExchange = false;
  bool _removed = false;
  bool _singleSector = true;
  bool _fcFpLevel = false;
  Indicator _type = ' ';
};

class DifferentialOverride final
{
public:
  DifferentialOverride() = default;
  DifferentialOverride(const DifferentialOverride&) = delete;
  DifferentialOverride& operator=(const DifferentialOverride&) = delete;

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  LocCode& lowHipOrigin() { return _lowHipOrigin; }
  const LocCode& lowHipOrigin() const { return _lowHipOrigin; }

  LocCode& lowHipDestination() { return _lowHipDestination; }
  const LocCode& lowHipDestination() const { return _lowHipDestination; }

  LocCode& highHipOrigin() { return _highHipOrigin; }
  const LocCode& highHipOrigin() const { return _highHipOrigin; }

  LocCode& highHipDestination() { return _highHipDestination; }
  const LocCode& highHipDestination() const { return _highHipDestination; }

  LocCode& highDiffFareOrigin() { return _highDiffFareOrigin; }
  const LocCode& highDiffFareOrigin() const { return _highDiffFareOrigin; }

  LocCode& highDiffFareDestination() { return _highDiffFareDestination; }
  const LocCode& highDiffFareDestination() const { return _highDiffFareDestination; }

  LocCode& hipConstructedCity() { return _hipConstructedCity; }
  const LocCode& hipConstructedCity() const { return _hipConstructedCity; }

  FareClassCode& lowFareClass() { return _lowFareClass; }
  const FareClassCode& lowFareClass() const { return _lowFareClass; }

  FareClassCode& highFareClass() { return _highFareClass; }
  const FareClassCode& highFareClass() const { return _highFareClass; }

  Indicator& lowFareCabin() { return _lowFareCabin; }
  const Indicator& lowFareCabin() const { return _lowFareCabin; }

  Indicator& highFareCabin() { return _highFareCabin; }
  const Indicator& highFareCabin() const { return _highFareCabin; }

  bool& fromExchange() { return _fromExchange; }
  const bool fromExchange() const { return _fromExchange; }

  Code<2>& mileageOnDiff() { return _mileageOnDiff; }
  const Code<2>& mileageOnDiff() const { return _mileageOnDiff; }

  void usedForRecreate(bool used) { _usedForRecreate = used; }
  bool usedForRecreate() const { return _usedForRecreate; }

  void initialize(DifferentialData& differentialData, const FareMarket& fareMarket) const;

private:
  MoneyAmount _amount = 0;
  LocCode _lowHipOrigin; // A01
  LocCode _lowHipDestination; // A02
  LocCode _highHipOrigin; // A03
  LocCode _highHipDestination; // A04
  LocCode _highDiffFareOrigin; // A13
  LocCode _highDiffFareDestination; // A14
  LocCode _hipConstructedCity; // A18
  Code<2> _mileageOnDiff; // Q48
  FareClassCode _lowFareClass; // B30
  FareClassCode _highFareClass; // BJ0
  Indicator _lowFareCabin = ' '; // N00
  Indicator _highFareCabin = ' '; // N04
  bool _fromExchange = false;
  bool _usedForRecreate = false;
};

class StopoverOverride final
{
public:
  void initialize(FareUsage::StopoverSurcharge& stopoverSurcharges,
                  const std::pair<CurrencyCode, CurrencyNoDec>& formattedCurrency) const;

  uint16_t& count() { return _count; }
  const uint16_t count() const { return _count; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  TravelSeg*& travelSeg() { return _travelSeg; }
  const TravelSeg* travelSeg() const { return _travelSeg; }

  bool& fromExchange() { return _fromExchange; }
  const bool fromExchange() const { return _fromExchange; }

private:
  MoneyAmount _amount = 0;
  TravelSeg* _travelSeg = nullptr;
  uint16_t _count = 0;
  bool _fromExchange = false;
};

class MinimumFareOverride : public MinFarePlusUpItem
{
public:
  // This portion is for BHCPlusUpItem
  LocCode fareBoardPoint;
  LocCode fareOffPoint;

  // This portion is for RSCPlusUpItem
  LocCode constructPoint2;
};

class PlusUpOverride final
{
public:
  MinimumFareModule& moduleName() { return _moduleName; }
  const MinimumFareModule& moduleName() const { return _moduleName; }

  MinFarePlusUpItem*& plusUpItem() { return _plusUpItem; }
  const MinFarePlusUpItem* plusUpItem() const { return _plusUpItem; }

  TravelSeg*& travelSeg() { return _travelSeg; }
  const TravelSeg* travelSeg() const { return _travelSeg; }

  bool& fromExchange() { return _fromExchange; }
  const bool fromExchange() const { return _fromExchange; }

private:
  MinFarePlusUpItem* _plusUpItem = nullptr;
  TravelSeg* _travelSeg = nullptr;
  MinimumFareModule _moduleName = MinimumFareModule::MAX_FARE_MODULE_IND;
  bool _fromExchange = false;
};

class ExchangeOverrides final
{
  friend class ExchangeOverridesTest;

  class TripChunkOrder
  {
  public:
    bool operator()(const FareUsage* fu1, const FareUsage* fu2) const
    {
      return CompareTravelSeg()(fu1->paxTypeFare()->fareMarket()->travelSeg().front(),
                                fu2->paxTypeFare()->fareMarket()->travelSeg().front());
    }
  };

  typedef std::set<FareUsage*, TripChunkOrder> OrderedTripChunks;

public:
  ExchangeOverrides(BaseExchangeTrx& trx) : _trx(trx) {}
  ExchangeOverrides(const ExchangeOverrides&) = delete;
  ExchangeOverrides& operator=(const ExchangeOverrides&) = delete;

  void fill(FarePath& farePath, bool copySurcharges = true) const;
  OrderedTripChunks createOrderedTripChunks(FarePath& fPath) const;

  std::vector<DifferentialOverride*>& differentialOverride() { return _differentialOverride; }
  const std::vector<DifferentialOverride*>& differentialOverride() const
  {
    return _differentialOverride;
  }

  std::map<const TravelSeg*, LocCode>& dummyFareMileTktCity() { return _dummyFareMileTktCity; }
  const std::map<const TravelSeg*, LocCode>& dummyFareMileTktCity() const
  {
    return _dummyFareMileTktCity;
  }

  std::vector<SurchargeOverride*>& surchargeOverride() { return _surchargeOverride; }
  const std::vector<SurchargeOverride*>& surchargeOverride() const { return _surchargeOverride; }

  std::vector<StopoverOverride*>& stopoverOverride() { return _stopoverOverride; }
  const std::vector<StopoverOverride*>& stopoverOverride() const { return _stopoverOverride; }

  uint16_t& journeySTOOverrideCnt() { return _journeySTOOverrideCnt; }
  const uint16_t journeySTOOverrideCnt() const { return _journeySTOOverrideCnt; }

  std::vector<PlusUpOverride*>& plusUpOverride() { return _plusUpOverride; }
  const std::vector<PlusUpOverride*>& plusUpOverride() const { return _plusUpOverride; }

  std::vector<MileageTypeData*>& mileageTypeData() { return _mileageTypeData; }
  const std::vector<MileageTypeData*>& mileageTypeData() const { return _mileageTypeData; }

  std::map<const TravelSeg*, uint16_t>& dummyFCSegs() { return _dummyFCSegs; }
  const std::map<const TravelSeg*, uint16_t>& dummyFCSegs() const { return _dummyFCSegs; }

  std::map<const TravelSeg*, int16_t>& dummyFareMiles() { return _dummyFareMiles; }
  const std::map<const TravelSeg*, int16_t>& dummyFareMiles() const { return _dummyFareMiles; }

  std::map<const TravelSeg*, LocCode>& dummyFareMileCity() { return _dummyFareMileCity; }
  const std::map<const TravelSeg*, LocCode>& dummyFareMileCity() const
  {
    return _dummyFareMileCity;
  }

  std::map<const TravelSeg*, char>& forcedSideTrip() { return _forcedSideTrip; }
  const std::map<const TravelSeg*, char>& forcedSideTrip() const { return _forcedSideTrip; }

private:
  void fillSurcharges(FareUsage& fu) const;
  void fillStopovers(FareUsage& fu, FarePath& farePath) const;
  void fillDifferentials(FareUsage& fu, FarePath& farePath) const;

  std::pair<CurrencyCode, CurrencyNoDec> getFormattedCurrency(const FarePath& farePath) const;

  BaseExchangeTrx& _trx;

  std::vector<DifferentialOverride*> _differentialOverride;
  std::map<const TravelSeg*, LocCode> _dummyFareMileTktCity;
  std::vector<SurchargeOverride*> _surchargeOverride;
  std::vector<StopoverOverride*> _stopoverOverride;
  uint16_t _journeySTOOverrideCnt = 0;
  std::vector<PlusUpOverride*> _plusUpOverride;
  std::vector<MileageTypeData*> _mileageTypeData;

  std::map<const TravelSeg*, uint16_t> _dummyFCSegs;
  std::map<const TravelSeg*, int16_t> _dummyFareMiles;
  std::map<const TravelSeg*, LocCode> _dummyFareMileCity;
  std::map<const TravelSeg*, char> _forcedSideTrip;
};
} // tse namespace
