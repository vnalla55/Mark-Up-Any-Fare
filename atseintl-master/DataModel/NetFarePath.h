//-------------------------------------------------------------------
//
//  File:        NetFarePath.h
//  Created:     June 5, 2012
//  Authors:
//
//  Description: Fare Path contains Net Fares for the CWT ticketing (NETSELL option).
//
//  Copyright Sabre 2006
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

#include "DataModel/FarePath.h"

namespace tse
{
class FareUsage;
class PricingTrx;
class DataHandle;

class NetFarePath : public FarePath
{
public:
  NetFarePath() : _trx(nullptr), _dataHandle(nullptr), _origFp(nullptr) {}
  virtual ~NetFarePath() {}

  void initialize(PricingTrx* _trx, FarePath* origFp);
  const FarePath* originalFarePath() const { return _origFp; }
  virtual NetFarePath* clone(DataHandle& dataHandle) const override;
  FareUsage* getNetFareUsage(FareUsage* origFu);
  const std::map<FareUsage*, FareUsage*>& netFareMap() const { return _netFareMap; }
  std::map<FareUsage*, FareUsage*>& netFareMap() { return _netFareMap; }

private:
  PricingTrx* _trx;
  DataHandle* _dataHandle;
  FarePath* _origFp;
  std::map<FareUsage*, FareUsage*> _netFareMap;
};

} // tse

