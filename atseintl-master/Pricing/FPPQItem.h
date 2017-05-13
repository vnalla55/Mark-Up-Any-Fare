//-------------------------------------------------------------------
// File:    FPPQItem.h
// Created: July 2004
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

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "Pricing/PricingEnums.h"
#include "Pricing/PriorityStatus.h"

namespace tse
{
class PUPath;
class FarePath;
class FarePathFactory;
class FarePathFactoryStorage;
class PUPQItem;
struct FarePathSettings;

class FPPQItem
{

public:
  friend class FPPQItemComparatorsTest;
  enum class EOEValidationStatus : uint8_t
  { EOE_UNKNOWN,
    EOE_PASSED,
    EOE_FAILED };

  void reuseSurchargeData() const;

  FarePath*& farePath() { return _farePath; }
  const FarePath* farePath() const { return _farePath; }

  PUPath*& puPath() { return _puPath; }
  const PUPath* puPath() const { return _puPath; }

  FarePathFactory*& farePathFactory() { return _farePathFactory; }
  const FarePathFactory* farePathFactory() const { return _farePathFactory; }

  std::vector<uint32_t>& puIndices() { return _puIndices; }
  const std::vector<uint32_t>& puIndices() const { return _puIndices; }

  std::vector<PUPQItem*>& pupqItemVect() { return _pupqItemVect; }
  const std::vector<PUPQItem*>& pupqItemVect() const { return _pupqItemVect; }

  uint16_t& xPoint() { return _xPoint; }
  const uint16_t& xPoint() const { return _xPoint; }

  PriorityStatus& mutablePriorityStatus() { return _priorityStatus; }
  const PriorityStatus& priorityStatus() const { return _priorityStatus; }

  PriorityStatus& mutablePausedPriorityStatus() { return _pausedPriorityStatus; }
  const PriorityStatus& pausedPriorityStatus() const { return _pausedPriorityStatus; }

  bool& isValid() { return _isValid; }
  const bool& isValid() const { return _isValid; }

  bool& paused() { return _paused; }
  const bool& paused() const { return _paused; }

  MoneyAmount& xPointPuNUCAmount() { return _xPointPuNUCAmount; }
  const MoneyAmount& xPointPuNUCAmount() const { return _xPointPuNUCAmount; }

  bool& hasCopiedPricingUnit() { return _hasCopiedPricingUnit; }
  const bool& hasCopiedPricingUnit() const { return _hasCopiedPricingUnit; }

  bool& ignorePUIndices() { return _ignorePUIndices; }
  const bool& ignorePUIndices() const { return _ignorePUIndices; }

  EOEValidationStatus& eoeValidationStatus() { return _eoeValidationStatus; }
  const EOEValidationStatus& eoeValidationStatus() const { return _eoeValidationStatus; }

  const uint16_t& getFlexFaresGroupId() const;

  bool needRecalculateCat12() const;

  bool isEqualAmountComponents(const FPPQItem& rhs) const;

  class GreaterFare
  {
  public:
    bool operator()(const FPPQItem* lhs, const FPPQItem* rhs) const;

  private:
    PaxTypeFare::CabinComparator _cabinComparator;
  };

  class LowerFare
  {
  public:
    bool operator()(const FPPQItem* lhs, const FPPQItem* rhs) const;

  private:
    PaxTypeFare::CabinComparator _cabinComparator;
  };

  FPPQItem* clone(FarePathFactoryStorage& storage);

  template <typename T>
  using Greater = PriorityStatus::Greater<FPPQItem, T>;

  template <typename T>
  using GreaterLowToHigh = PriorityStatus::GreaterLowToHigh<FPPQItem, T>;

  FPPQItem* createDuplicate(PricingTrx&) const;

  void clearPriority()
  {
    _priorityStatus.clear();
    _pausedPriorityStatus.clear();
  }

  const CabinType& getCabinPriority() const { return _cabinPriority; }
  void updateCabinPriority(const CabinType& newCabin);

  unsigned getNumberOfStopsPriority() const { return _numberOfStopsPriority; }
  void setNumberOfStopsPriority(unsigned value) { _numberOfStopsPriority = value; }

  void setFarePathSettings(FarePathSettings* farePathSettings)
  {
    _farePathSettings = farePathSettings;
  }
  const FarePathSettings* getFarePathSettings() { return _farePathSettings; }

private:
  FarePathSettings* _farePathSettings = nullptr;
  FarePath* _farePath = nullptr;
  PriorityStatus _priorityStatus, _pausedPriorityStatus;

  // Needed for farepath Generate-Validate loop
  //
  PUPath* _puPath = nullptr;

  std::vector<uint32_t> _puIndices;
  std::vector<PUPQItem*> _pupqItemVect;

  FarePathFactory* _farePathFactory = nullptr; // added for IS optimization

  MoneyAmount _xPointPuNUCAmount = 0;

  unsigned _numberOfStopsPriority = 0;

  uint16_t _xPoint = 0;

  bool _isValid = true;
  bool _paused = false;
  bool _hasCopiedPricingUnit = false;
  bool _ignorePUIndices = false; // out of order Cxr-Fare combination
  CabinType _cabinPriority;

  EOEValidationStatus _eoeValidationStatus = EOEValidationStatus::EOE_UNKNOWN;
};

} // tse namespace
