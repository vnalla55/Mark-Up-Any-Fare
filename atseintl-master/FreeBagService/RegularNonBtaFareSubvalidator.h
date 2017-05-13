//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/OcTypes.h"
#include "FreeBagService/BaggageS7SubvalidatorInterfaces.h"

#include <set>

#include <boost/container/flat_set.hpp>

namespace tse
{
class BaggageTravel;
class FarePath;
class PaxTypeFare;

class RegularNonBtaFareSubvalidator : public INonBtaFareSubvalidator
{
public:
  RegularNonBtaFareSubvalidator(const BaggageTravel& bt,
                                const Ts2ss& ts2ss,
                                const FarePath* fp)
    : _bt(bt), _ts2ss(ts2ss), _fp(fp)
  {}

  void onFaresUpdated(const FarePath* fp);
  StatusS7Validation validate(const OptionalServicesInfo& s7, OCFees& ocFees) override;
  StatusS7Validation revalidate(const OptionalServicesInfo& s7);

private:
  void prepareFareInfo();

  bool checkRuleTariffInd(const OptionalServicesInfo& s7) const;
  bool checkTourCode(const OptionalServicesInfo& s7) const;

  const BaggageTravel& _bt;
  const Ts2ss& _ts2ss;
  const FarePath* _fp;
  boost::container::flat_set<PaxTypeFare*> _fares;
  bool _allFaresOnBtKnown = true;
};
}
