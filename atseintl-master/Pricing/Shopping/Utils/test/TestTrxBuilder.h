//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#include "Common/Assert.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "test/include/TestMemHandle.h"

#include <vector>
#include <set>

namespace tse
{

typedef std::pair<unsigned int, uint32_t> LegSopTuple;

class TestTrxBuilder
{
public:
  TestTrxBuilder(TestMemHandle& handle) : _handle(handle) {}

  ShoppingTrx::SchedulingOption* buildSOP(CarrierCode cxrCode, bool direct)
  {
    Itin* itin = _handle.create<Itin>();
    uint32_t tmp = 0;

    ShoppingTrx::SchedulingOption* aeSop = _handle.create<ShoppingTrx::SchedulingOption>(itin, tmp);
    aeSop->governingCarrier() = cxrCode;

    if (direct)
    {
      aeSop->itin()->travelSeg().push_back(getTravelSeg());
    }
    else
    {
      aeSop->itin()->travelSeg().push_back(getTravelSeg());
      aeSop->itin()->travelSeg().push_back(getTravelSeg());
    }
    return aeSop;
  }

  ShoppingTrx::Leg* buildLeg(const std::vector<ShoppingTrx::SchedulingOption*>& sops)
  {
    ShoppingTrx::Leg* leg = _handle.create<ShoppingTrx::Leg>();
    for (unsigned int i = 0; i < sops.size(); ++i)
    {
      leg->addSop(*sops[i]);
    }
    leg->requestSops() = sops.size();
    return leg;
  }

  ShoppingTrx* getTrx(const std::vector<ShoppingTrx::Leg*>& legs)
  {
    ShoppingTrx* trx = _handle.create<ShoppingTrx>();
    for (unsigned int i = 0; i < legs.size(); ++i)
    {
      trx->legs().push_back(*legs[i]);
    }
    return trx;
  }

private:
  TravelSeg* getTravelSeg()
  {
    TravelSeg* seg = _handle.create<AirSeg>();
    seg->origin() = _handle.create<Loc>();
    seg->destination() = _handle.create<Loc>();
    return seg;
  }

  TestMemHandle& _handle;
};

class SimpleTransactionBuilder
{
public:
  SimpleTransactionBuilder(TestMemHandle& handle) : _handle(handle) {}

  // Trx:
  // Leg    0        1
  // --------------------
  // Sop    0 AA d   0 BB
  //        1 BB     1 CC d
  //        2 BB d   2 AA d
  //        3 CC
  ShoppingTrx* buildSimpleTrx()
  {
    TestTrxBuilder* bld = _handle.create<TestTrxBuilder>(_handle);

    std::vector<ShoppingTrx::SchedulingOption*> sops;
    std::vector<ShoppingTrx::Leg*> legs;

    sops.push_back(bld->buildSOP("AA", true));
    sops.push_back(bld->buildSOP("BB", false));
    sops.push_back(bld->buildSOP("BB", true));
    sops.push_back(bld->buildSOP("CC", false));

    legs.push_back(bld->buildLeg(sops));
    sops.clear();

    sops.push_back(bld->buildSOP("BB", false));
    sops.push_back(bld->buildSOP("CC", true));
    sops.push_back(bld->buildSOP("AA", true));

    legs.push_back(bld->buildLeg(sops));

    return bld->getTrx(legs);
  }

  const std::set<LegSopTuple> getAll() const
  {
    std::set<LegSopTuple> tmp;
    tmp.insert(std::make_pair(0, 0));
    tmp.insert(std::make_pair(0, 1));
    tmp.insert(std::make_pair(0, 2));
    tmp.insert(std::make_pair(0, 3));

    tmp.insert(std::make_pair(1, 0));
    tmp.insert(std::make_pair(1, 1));
    tmp.insert(std::make_pair(1, 2));
    return tmp;
  }

  const std::set<LegSopTuple> getBB() const
  {
    std::set<LegSopTuple> tmp;
    tmp.insert(std::make_pair(0, 1));
    tmp.insert(std::make_pair(0, 2));
    tmp.insert(std::make_pair(1, 0));
    return tmp;
  }

  const std::set<LegSopTuple> getDirects() const
  {
    std::set<LegSopTuple> tmp;
    tmp.insert(std::make_pair(0, 0));
    tmp.insert(std::make_pair(0, 2));
    tmp.insert(std::make_pair(1, 1));
    tmp.insert(std::make_pair(1, 2));
    return tmp;
  }

private:
  TestMemHandle& _handle;
};
} // namespace tse
