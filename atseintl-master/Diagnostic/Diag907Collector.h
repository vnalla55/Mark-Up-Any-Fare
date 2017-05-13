//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag903Collector.h"

namespace tse
{
class Diagnostic;
class PaxTypeFare;
class FareMarket;

class Diag907Collector : public Diag903Collector
{
public:
  explicit Diag907Collector(Diagnostic& root)
    : Diag903Collector(root), _curItinRowKey(nullptr), _curLeg(nullptr), _curLegCarrierIndex(nullptr)
  {
  }
  Diag907Collector() : _curItinRowKey(nullptr), _curLeg(nullptr), _curLegCarrierIndex(nullptr) {}

  virtual Diag907Collector& operator<<(const FareMarket& fareMarket) override;
  virtual Diag907Collector& operator<<(const ItinIndex& itinGroup) override;
  virtual Diag907Collector& operator<<(const ShoppingTrx& shoppingTrx) override;

private:
  void outputFareBitDesc(const PaxTypeFare& paxFare,
                         double point = 0.0,
                         int numberOfClassesToAdd = 0,
                         int requestedNumberOfSeats = 0);

private:
  const ItinIndex::Key* _curItinRowKey;
  const ShoppingTrx::Leg* _curLeg;
  const ItinIndex* _curLegCarrierIndex;
};

} // namespace tse

