// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

#include "Pricing/PricingEnums.h"

namespace tse
{

class PriorityStatus
{
public:
  PriorityStatus()
    : _farePriority(uint8_t(DEFAULT_PRIORITY)), 
      _fareByRulePriority(uint8_t(FBR_PRIORITY_DEFAULT)),
      _paxTypeFarePriority(uint8_t(DEFAULT_PRIORITY)),
      _fareCxrTypePriority(uint8_t(DEFAULT_PRIORITY)),
      _negotiatedFarePriority(uint8_t(DEFAULT_PRIORITY)),
      _ptfRank(0)
  {
  }

  PRIORITY farePriority() const { return PRIORITY(_farePriority); }
  void setFarePriority(PRIORITY priority) { _farePriority = uint8_t(priority); }

  FBR_PRIORITY fareByRulePriority() const { return FBR_PRIORITY(_fareByRulePriority); }
  void setFareByRulePriority(FBR_PRIORITY priority) { _fareByRulePriority = uint8_t(priority); }

  PRIORITY paxTypeFarePriority() const { return PRIORITY(_paxTypeFarePriority); }
  void setPaxTypeFarePriority(PRIORITY priority) { _paxTypeFarePriority = uint8_t(priority); }

  PRIORITY fareCxrTypePriority() const { return PRIORITY(_fareCxrTypePriority); }
  void setFareCxrTypePriority(PRIORITY priority) { _fareCxrTypePriority = uint8_t(priority); }

  PRIORITY negotiatedFarePriority() const { return PRIORITY(_negotiatedFarePriority); }
  void setNegotiatedFarePriority(PRIORITY priority) { _negotiatedFarePriority = uint8_t(priority); }

  uint16_t ptfRank() const { return _ptfRank; }
  void setPtfRank(uint16_t rank) { _ptfRank = rank; }

  void clear()
  {
    setFarePriority(DEFAULT_PRIORITY);
    setFareByRulePriority(FBR_PRIORITY_DEFAULT);
    setPaxTypeFarePriority(DEFAULT_PRIORITY);
    setFareCxrTypePriority(DEFAULT_PRIORITY);
    setNegotiatedFarePriority(DEFAULT_PRIORITY);
    setPtfRank(0);
  }

  template <typename PQType, typename Base>
  class Greater : public Base
  {
  public:
    bool operator()(const PQType* lhs, const PQType* rhs) const
    {
      const PriorityStatus& lps = lhs->priorityStatus();
      const PriorityStatus& rps = rhs->priorityStatus();

      if (lps.farePriority() != rps.farePriority())
      {
        return lps.farePriority() > rps.farePriority();
      }

      return Base::operator()(lhs, rhs);
    }
  };

  template <typename PQType, typename Base>
  class GreaterLowToHigh : public Base
  {
  public:
    bool operator()(const PQType* lhs, const PQType* rhs) const
    {
      return Base::operator()(lhs, rhs);
    }
  };

  static FBR_PRIORITY mergeFbrPriorities(FBR_PRIORITY currentPrio, FBR_PRIORITY newPrio)
  {
    if (newPrio == currentPrio)
      return currentPrio;

    if (currentPrio == FBR_PRIORITY_FBR_ONLY)
    {
      if (LIKELY(newPrio == FBR_PRIORITY_MIXED ||
          newPrio == FBR_PRIORITY_PUB_OR_CARRIER_ONLY))
        return FBR_PRIORITY_MIXED;
    }
    else if (currentPrio == FBR_PRIORITY_PUB_OR_CARRIER_ONLY)
    {
      if (LIKELY(newPrio == FBR_PRIORITY_MIXED ||
          newPrio == FBR_PRIORITY_FBR_ONLY))
        return FBR_PRIORITY_MIXED;
    }
    else if (currentPrio == FBR_PRIORITY_DEFAULT)
    {
      return newPrio;
    }

    return currentPrio;
  }

private:
  // Cxr might force to use their fare instead of YY, etc fare,
  // will be indicated in Cxr-Preference Table. CXR fare may be higher than
  // YY fare
  //
  // higher the number lower the priority
  uint8_t _farePriority;

  // Fare By Rule fares are preferred over other fares with the same price.
  //
  // higher the number, lower the priority
  uint8_t _fareByRulePriority;

  // should be prefered over other
  // e.g. for CNN, use Child fare instead of
  // ADT fare when both have same price
  //
  // FarePath with req PaxType fare
  uint8_t _paxTypeFarePriority;

  // for equal amount fare
  // and cxr-pref-table is not forcing
  // to use cxr fare any way
  //
  // prefer cxr fare over YY fare
  uint8_t _fareCxrTypePriority;

  // higher the number, lower the priority
  uint8_t _negotiatedFarePriority;

  // Higher the number higher the priority
  uint16_t _ptfRank;
};

}

