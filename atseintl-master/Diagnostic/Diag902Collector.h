//----------------------------------------------------------------------------
//  File:        Diag902Collector.h
//  Created:     2004-08-17
//
//  Description: Diagnostic 902 formatter
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
//----------------------------------------------------------------------------
#pragma once

#include "Diagnostic/Diag200Collector.h"

namespace tse
{
class TravelSeg;

class Diag902Collector : public Diag200Collector
{
public:
  explicit Diag902Collector(Diagnostic& root)
    : Diag200Collector(root),
      _acrossStopOverLeg(false),
      _primarySector(nullptr),
      _legIndex(0),
      _shoppingTrx(nullptr)
  {
  }
  Diag902Collector() : _acrossStopOverLeg(false), _primarySector(nullptr), _legIndex(0), _shoppingTrx(nullptr)
  {
  }

  virtual Diag902Collector& operator<<(const Itin& itin) override;
  virtual Diag902Collector& operator<<(const ItinIndex& itinGroup) override;
  virtual Diag902Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

private:
  void displayBookingCodeAvail(const Itin& itin);
  void displaySOPPenalties(
      const Itin& itin, const int&, const int&, const int&, const DateTime& reqDepDateTime);

  bool _acrossStopOverLeg;
  const TravelSeg* _primarySector;

protected:
  uint32_t _legIndex;
  const ShoppingTrx* _shoppingTrx;
};

} // namespace tse

