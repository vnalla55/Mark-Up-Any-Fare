//-------------------------------------------------------------------
////
////  Copyright Sabre 2008
////
////          The copyright to the computer program(s) herein
////          is the property of Sabre.
////          The program(s) may be used and/or copied only with
////          the written permission of Sabre or in accordance
////          with the terms and conditions stipulated in the
////          agreement/contract under which the program(s)
////          have been supplied.
////
////-------------------------------------------------------------------

#include "DataModel/PricingUnit.h"
#include "DBAccess/ReissueSequence.h"

#pragma once

namespace tse
{

class FareUsage;
class AdvResOverride;

class AdvResOverride
{
private:
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  DateTime _fromDate;
  DateTime _toDate;
  bool _ignoreTktAfterResRestriction;
  bool _ignoreTktDeforeDeptRestriction;
  ReissueSequenceW* _reissueSequence;
  bool _diag305OFF;

public:
  AdvResOverride()
    : _pricingUnit(nullptr),
      _fareUsage(nullptr),
      _fromDate(DateTime::emptyDate()),
      _toDate(DateTime::emptyDate()),
      _ignoreTktAfterResRestriction(false),
      _ignoreTktDeforeDeptRestriction(false),
      _reissueSequence(nullptr),
      _diag305OFF(false)
  {
  }

  explicit AdvResOverride(PricingUnit& pu)
    : _pricingUnit(&pu),
      _fareUsage(nullptr),
      _fromDate(DateTime::emptyDate()),
      _toDate(DateTime::emptyDate()),
      _ignoreTktAfterResRestriction(false),
      _ignoreTktDeforeDeptRestriction(false),
      _reissueSequence(nullptr),
      _diag305OFF(false)
  {
    _pricingUnit->volChangesAdvResOverride() = this;
  }

  ~AdvResOverride()
  {
    if (_pricingUnit)
      _pricingUnit->volChangesAdvResOverride() = nullptr;
  }

  inline void setDiag305OFF(bool status = true) { _diag305OFF = status; }
  inline bool diag305OFF() const { return _diag305OFF; }

  PricingUnit*& pricingUnit() { return _pricingUnit; }
  const PricingUnit* pricingUnit() const { return _pricingUnit; }

  FareUsage*& fareUsage() { return _fareUsage; }
  const FareUsage* fareUsage() const { return _fareUsage; }

  DateTime& fromDate() { return _fromDate; }
  const DateTime& fromDate() const { return _fromDate; }

  DateTime& toDate() { return _toDate; }
  const DateTime& toDate() const { return _toDate; }

  bool& ignoreTktAfterResRestriction() { return _ignoreTktAfterResRestriction; }
  bool ignoreTktAfterResRestriction() const { return _ignoreTktAfterResRestriction; }

  bool& ignoreTktDeforeDeptRestriction() { return _ignoreTktDeforeDeptRestriction; }
  bool ignoreTktDeforeDeptRestriction() const { return _ignoreTktDeforeDeptRestriction; }

  ReissueSequenceW*& reissueSequence() { return _reissueSequence; }
  const ReissueSequenceW* reissueSequence() const { return _reissueSequence; }

  bool operator()(const AdvResOverride& p1, const AdvResOverride& p2) const
  {
    if (p1._pricingUnit == p2._pricingUnit)
    {
      if (p1._fareUsage == p2._fareUsage)
      {
        const ReissueSequence* rs1 =
            p1._reissueSequence ? p1._reissueSequence->overridingWhenExists() : nullptr;
        const ReissueSequence* rs2 =
            p2._reissueSequence ? p2._reissueSequence->overridingWhenExists() : nullptr;

        return rs1 < rs2;
      }
      else if (p1._fareUsage < p2._fareUsage)
        return true;
    }
    else if (p1._pricingUnit < p2._pricingUnit)
      return true;
    return false;
  }
};

} // namespace tse

