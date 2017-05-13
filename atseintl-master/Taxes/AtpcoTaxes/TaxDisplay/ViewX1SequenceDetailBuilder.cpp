// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/ViewX1SequenceDetailBuilder.h"

#include "DataModel/Services/RulesRecord.h"
#include "ServiceInterfaces/CarrierApplicationService.h"
#include "ServiceInterfaces/CarrierFlightService.h"
#include "ServiceInterfaces/PassengerTypesService.h"
#include "ServiceInterfaces/ServiceFeeSecurityService.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "ServiceInterfaces/SectorDetailService.h"
#include "ServiceInterfaces/Services.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"

namespace tax
{
namespace display
{

std::unique_ptr<ViewX1SequenceDetail>
ViewX1SequenceDetailBuilder::build(const ViewX1TaxDetail::DataSet& taxDetailData,
                                   ResponseFormatter& formatter,
                                   DetailEntryNo entryNo)
{
  auto rulesRecord = taxDetailData.begin();
  std::advance(rulesRecord, entryNo - 1);
  return build(**rulesRecord, formatter);
}

std::unique_ptr<ViewX1SequenceDetail>
ViewX1SequenceDetailBuilder::build(const RulesRecord& rulesRecord,
                                   ResponseFormatter& formatter)
{
  auto view = std::make_unique<ViewX1SequenceDetail>(rulesRecord, // iterator points at shared_ptr
                                                     _services.locService(),
                                                     _services.taxRoundingInfoService(),
                                                     _request,
                                                     formatter);

  setCategories(rulesRecord, *view);

  return view;
}

void ViewX1SequenceDetailBuilder::setCategories(const RulesRecord& rulesRecord, ViewX1SequenceDetail& view)
{
  if (_request.x1categories[static_cast<size_t>(X1Category::TRAVEL_CARRIER_QUALIFYING_TAGS)])
  {
    const std::shared_ptr<const CarrierApplication> carrierApplication =
        _services.carrierApplicationService().getCarrierApplication(rulesRecord.vendor,
                                                                    rulesRecord.carrierApplicationItem);

    const std::shared_ptr<const CarrierFlight> carrierFlightBefore =
        _services.carrierFlightService().getCarrierFlight(rulesRecord.vendor,
                                                          rulesRecord.carrierFlightItemBefore);

    const std::shared_ptr<const CarrierFlight> carrierFlightAfter =
        _services.carrierFlightService().getCarrierFlight(rulesRecord.vendor,
                                                          rulesRecord.carrierFlightItemAfter);

    const std::shared_ptr<const PassengerTypeCodeItems> passengerTypeCodeItems =
        _services.passengerTypesService().getPassengerTypeCode(rulesRecord.vendor,
                                                               rulesRecord.passengerTypeCodeItem);

    view.setTravelCarrierQualifyingTagsData(carrierApplication,
                                            carrierFlightBefore,
                                            carrierFlightAfter,
                                            passengerTypeCodeItems);
  }

  if (_request.x1categories[static_cast<size_t>(X1Category::SERVICE_BAGGAGE_TAX)])
  {
    const std::shared_ptr<const ServiceFeeSecurityItems> serviceFeeSecurityItems =
        _services.serviceFeeSecurityService().getServiceFeeSecurity(rulesRecord.vendor,
                                                                    rulesRecord.svcFeesSecurityItemNo);

    const std::shared_ptr<const ServiceBaggage> serviceBaggage =
        _services.serviceBaggageService().getServiceBaggage(rulesRecord.vendor,
                                                            rulesRecord.serviceBaggageItemNo);

    view.setServiceBaggageData(serviceFeeSecurityItems, serviceBaggage);
  }

  if (_request.x1categories[static_cast<size_t>(X1Category::TRAVEL_SECTOR_DETAILS)])
  {
    const std::shared_ptr<const SectorDetail> sectorDetail =
        _services.sectorDetailService().getSectorDetail(rulesRecord.vendor,
                                                        rulesRecord.sectorDetailItemNo);

    view.setTravelSectorData(sectorDetail);
  }
}

} // display
} // tax
