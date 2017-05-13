// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <map>
#include <vector>

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DataModel/PricingTrx.h"
#include "ServiceFees/OCFees.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/OptionalServicesForOtaTaxReqBuilder.h"
#include "DataModel/PricingUnit.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"

namespace tax
{

OptionalServicesForOtaTaxReqBuilder::OptionalServicesForOtaTaxReqBuilder(tse::PricingTrx& trx,
                                                                        InputRequest& inputRequest,
                                                                        tax::InputItin& itin,
                                                                        const tse::FarePath& tseFarePath)
  : _trx(trx),
    _inputRequest(inputRequest),
    _itin(itin),
    _tseFarePath(tseFarePath)
{
}

OptionalServicesForOtaTaxReqBuilder::~OptionalServicesForOtaTaxReqBuilder() {}

void
OptionalServicesForOtaTaxReqBuilder::addOptionalServices()
{
  addGeoPathMappings();
  for(tse::PricingUnit* tsePricingUnit : _tseFarePath.pricingUnit())
  {
    for(tse::FareUsage* tseFareUsage : tsePricingUnit->fareUsage())
    {
      addOCFees(tseFareUsage);
    }
  }
}

void
OptionalServicesForOtaTaxReqBuilder::addGeoPathMappings()
{
  _inputRequest.optionalServicePaths().push_back(new InputOptionalServicePath());
  tax::type::Index optionalServicePathId = _inputRequest.optionalServicePaths().size() - 1;
  _inputRequest.optionalServicePaths().back()._id = optionalServicePathId;
  _itin._optionalServicePathRefId = optionalServicePathId;

  _inputRequest.geoPathMappings().push_back(new InputGeoPathMapping());
  tax::type::Index getPathMappingId = _inputRequest.geoPathMappings().size() - 1;
  _inputRequest.geoPathMappings().back()._id = getPathMappingId;
  _itin._optionalServicePathGeoPathMappingRefId = getPathMappingId;
}

void
OptionalServicesForOtaTaxReqBuilder::addOCFees(const tse::FareUsage* tseFareUsage)
{
  std::unique_ptr<tax::InputOptionalService> optionalService(new tax::InputOptionalService());

  if (nullptr != tseFareUsage->paxTypeFare())
    optionalService->_amount = tax::doubleToAmount(tseFareUsage->paxTypeFare()->nucFareAmount());
  optionalService->_ownerCarrier = tse::toTaxCarrierCode(_trx.getRequest()->validatingCarrier());
  optionalService->_type = type::OptionalServiceTag::FlightRelated;
  //PrePaid not handled by OTA interface
  optionalService->_id = _inputRequest.optionalServices().size();

  std::unique_ptr<InputOptionalServiceUsage> optionalServiceUsage(new InputOptionalServiceUsage());
  optionalServiceUsage->_optionalServiceRefId = optionalService->_id;

  _inputRequest.optionalServices().push_back(optionalService.release());
  _inputRequest.optionalServicePaths().back()._optionalServiceUsages.push_back(
      optionalServiceUsage.release());

  buildMappings();

}

void
OptionalServicesForOtaTaxReqBuilder::buildMappings()
{
  std::unique_ptr<tax::InputMapping> mapping(new tax::InputMapping());

  InputMap* map1 = new InputMap();
  map1->_geoRefId = 0;
  mapping->maps().push_back(map1);

  InputMap* map2 = new InputMap();
  map2->_geoRefId = 1;
  mapping->maps().push_back(map2);

  _inputRequest.geoPathMappings().back()._mappings.push_back(mapping.release());
}

} // end of namespace tax
