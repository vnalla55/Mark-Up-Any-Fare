//-------------------------------------------------------------------
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

#include "Common/TseConsts.h"

#include <stdint.h>

namespace tse
{
class BaggagePolicy
{
public:
  enum DisclosurePolicy : uint8_t
  { ALL_TRAVELS,
    SELECTED_TRAVELS,
    UNFLOWN_TRAVELS };

  BaggagePolicy() = default;
  BaggagePolicy(const BaggagePolicy&) = delete;
  BaggagePolicy& operator=(const BaggagePolicy&) = delete;

  void setupPolicy(DisclosurePolicy disclosurePolicy, bool changeTktDateForAncillaryCharges)
  {
    _disclosurePolicy = disclosurePolicy;
    _changeTktDateForAncillaryCharges = changeTktDateForAncillaryCharges;
  }

  DisclosurePolicy getDisclosurePolicy() const { return _disclosurePolicy; }
  bool changeTktDateForAncillaryCharges() const { return _changeTktDateForAncillaryCharges; }

  void setRequestedBagPieces(uint32_t rbp) { _requestedBagPieces = std::min(rbp, MAX_BAG_PIECES); }
  uint32_t getRequestedBagPieces() const { return _requestedBagPieces; }

private:
  uint32_t _requestedBagPieces = 2;
  DisclosurePolicy _disclosurePolicy = ALL_TRAVELS;
  bool _changeTktDateForAncillaryCharges = false;
};

} // tse
