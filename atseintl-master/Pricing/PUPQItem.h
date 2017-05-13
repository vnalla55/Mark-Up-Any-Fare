//-------------------------------------------------------------------
//
//  File:        PUPQItem.h
//  Created:     JUL 1, 2004
//  Authors:     Mohammad Hossan
//
//  Description:
//
//  Updates:
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

#include "Common/ArrayVector.h"
#include "Common/TseObjectPool.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Pricing/PricingEnums.h"
#include "Pricing/PriorityStatus.h"
#include "Pricing/PU.h"

namespace tse
{

typedef TseObjectPool<FareUsage> FareUsagePool;

class PUPQItem
{

public:
  friend class PUPQItemComparatorsTest;

  enum class PUValidationStatus : uint8_t
  { UNKNOWN,
    PASSED,
    FAILED };

  PUPQItem() = default;

  PUPQItem(const PUPQItem& item) = delete;
  PUPQItem& operator=(const PUPQItem& rhs) = delete;

  void clearReusedFareUsage() { _pricingUnit->clearReusedFareUsage(); }

  PricingUnit*& pricingUnit() { return _pricingUnit; }
  const PricingUnit* pricingUnit() const { return _pricingUnit; }

  const PU* pu() const { return _pu; }
  PU*& pu() { return _pu; }

  const ArrayVector<uint16_t>& fareIndices() const { return _fareIndices; }
  ArrayVector<uint16_t>& fareIndices() { return _fareIndices; }

  const uint16_t& xPoint() const { return _xPoint; }
  uint16_t& xPoint() { return _xPoint; }

  PriorityStatus& mutablePriorityStatus() { return _priorityStatus; }
  const PriorityStatus& priorityStatus() const { return _priorityStatus; }

  bool& isValid() { return _isValid; }
  const bool& isValid() const { return _isValid; }

  int32_t& cxrFareComboIdx() { return _cxrFareComboIdx; }
  const int32_t& cxrFareComboIdx() const { return _cxrFareComboIdx; }

  PUValidationStatus& cat10Status() { return _cat10Status; }
  const PUValidationStatus& cat10Status() const { return _cat10Status; }

  PUValidationStatus& ruleReValStatus() { return _ruleReValStatus; }
  const PUValidationStatus& ruleReValStatus() const { return _ruleReValStatus; }

  bool& paused() { return _paused; }
  const bool& paused() const { return _paused; }

  bool& rebookClassesExists() { return _rebookClassesExists; }
  const bool& rebookClassesExists() const { return _rebookClassesExists; }

  // Must be called to have the mutuable FareUsage before
  // PUPQItem is put into PricingUnitFactory::_validPUPQItem
  //
  bool cloneFareUsage(PricingTrx& trx, FareUsagePool& fuPool);

  class GreaterFare
  {
  public:
    bool operator()(const PUPQItem* lhs, const PUPQItem* rhs) const;

  private:
    PaxTypeFare::CabinComparator _cabinComparator;
  };

  class LowerFare
  {
  public:
    bool operator()(const PUPQItem* lhs, const PUPQItem* rhs) const;

  private:
    PaxTypeFare::CabinComparator _cabinComparator;
  };

  template <typename T>
  using Greater = PriorityStatus::Greater<PUPQItem, T>;

  template <typename T>
  using GreaterLowToHigh = PriorityStatus::GreaterLowToHigh<PUPQItem, T>;

  const CabinType& getCabinPriority() const { return _cabinPriority; }
  void updateCabinPriority(const CabinType& newCabin);

private:
  PricingUnit* _pricingUnit = nullptr;

  PriorityStatus _priorityStatus;

  // Needed for PricingUnit Generate-Validate loop
  //
  PU* _pu = nullptr;
  ArrayVector<uint16_t> _fareIndices;
  uint16_t _xPoint = 0;
  bool _isValid = true;
  int32_t _cxrFareComboIdx = -1;
  PUValidationStatus _cat10Status = PUPQItem::PUValidationStatus::UNKNOWN;
  PUValidationStatus _ruleReValStatus = PUPQItem::PUValidationStatus::UNKNOWN;
  bool _paused = false;
  bool _rebookClassesExists = false;

  CabinType _cabinPriority;
};

} // tse namespace
