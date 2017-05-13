#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"

#include <vector>

using std::vector;

namespace tse
{

class PricingUnit;
class FarePath;
class FareUsage;
class Diag527Collector;
class PricingTrx;
class PaxTypeFare;

class ToursApplication
{
public:
  ToursApplication(PricingTrx* trx);
  virtual ~ToursApplication();

  Record3ReturnTypes validate(const PricingUnit* pricingUnit);
  Record3ReturnTypes validate(const FarePath* farePath);

protected:
  virtual Record3ReturnTypes validate(const vector<FareUsage*>& fareUsages);
  CarrierCode getCarrier(const PaxTypeFare* paxTypeFare) const;
  bool isNegotiatedFareWithTourCode(const PaxTypeFare* paxTypeFare) const;

  PricingTrx* _trx;
  Diag527Collector* _diag;

  friend class ToursApplicationTest;
};
}

