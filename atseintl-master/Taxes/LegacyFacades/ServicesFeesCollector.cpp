// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/ForEachTaxResponse.h"
#include "Taxes/LegacyFacades/ServicesFeesCollector.h"
#include "Taxes/LegacyTaxes/TaxDriver.h"

namespace tax
{
namespace
{

class ComputeServiceFees
{
  tse::PricingTrx& _trx;

public:
  explicit ComputeServiceFees(tse::PricingTrx& trx)
    : _trx(trx)
  {
  }

  void operator()(tse::TaxResponse& taxResponse, tse::Itin& itin, tse::FarePath& farePath) const
  {
    itin.validatingCarrier() = taxResponse.validatingCarrier();
    tse::TaxDriver::validateAndApplyServicesFees(_trx, taxResponse);
  }
};

class MapServiceFees
{
  ServicesFeesMap& _servicesFees;

public:
  explicit MapServiceFees(ServicesFeesMap& serviceFees)
    : _servicesFees(serviceFees)
  {
  }

  void operator()(tse::TaxResponse& taxResponse, tse::Itin& itin, tse::FarePath& farePath) const
  {
    // copy pointers
    ServicesFeesMapKey key = std::make_tuple(&itin, &farePath, toTaxCarrierCode(taxResponse.validatingCarrier()));
    _servicesFees[key] = taxResponse.taxItemVector();
  }
};

} // anonymous namespace

void
computeServiceFees(tse::PricingTrx& trx)
{
  forEachTaxResponse(trx, ComputeServiceFees(trx));
}

void
mapServiceFees(tse::PricingTrx& trx, ServicesFeesMap& servicesFees)
{
  forEachTaxResponse(trx, MapServiceFees(servicesFees));
}

} // namespace tax
