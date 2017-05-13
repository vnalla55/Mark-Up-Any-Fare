#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DataModel/PricingRequest.h"

namespace tse
{
class PointOfSale
{
public:
  PointOfSale() {}
  PointOfSale(const CrsCode& ph, const NationCode& nc, const PseudoCityCode& aaa)
    : _primeHost(ph), _country(nc), _pcc(aaa)
  {
  }

  const CrsCode& primeHost() const { return _primeHost; }
  CrsCode& primeHost() { return _primeHost; }

  const NationCode& country() const { return _country; }
  NationCode& country() { return _country; }

  const PseudoCityCode& pcc() const { return _pcc; }
  PseudoCityCode& pcc() { return _pcc; }

private:
  CrsCode _primeHost;
  NationCode _country;
  PseudoCityCode _pcc;
};

class TicketingCxrDisplayRequest : public PricingRequest
{
public:
  TicketingCxrDisplayRequest() : _reqType(vcx::NO_REQ) {}

  PointOfSale& pointOfSale() { return _pos; }
  const PointOfSale& pointOfSale() const { return _pos; }

  NationCode& specifiedCountry() { return _specifiedCountry; }
  const NationCode& specifiedCountry() const { return _specifiedCountry; }

  CrsCode& specifiedPrimeHost() { return _specifiedPrimeHost; }
  const CrsCode& specifiedPrimeHost() const { return _specifiedPrimeHost; }

  SettlementPlanType& settlementPlan() { return _settlementPlan; }
  const SettlementPlanType& settlementPlan() const { return _settlementPlan; }

  CarrierCode& validatingCxr() { return _validatingCxr; }
  const CarrierCode& validatingCxr() const { return _validatingCxr; }

  vcx::TCSRequestType getRequestType() const { return _reqType; }
  void setRequestType(vcx::TCSRequestType reqType) { _reqType = reqType; }

protected:
  TicketingCxrDisplayRequest(const TicketingCxrDisplayRequest&);
  TicketingCxrDisplayRequest& operator=(const TicketingCxrDisplayRequest&);

private:
  PointOfSale _pos;
  NationCode _specifiedCountry;
  CrsCode _specifiedPrimeHost;
  SettlementPlanType _settlementPlan;
  CarrierCode _validatingCxr;
  vcx::TCSRequestType _reqType;
};

} // tse namespace
