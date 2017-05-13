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
#pragma once

#include "DataModel/Services/CarrierApplication.h"
#include "DataModel/Services/CarrierFlight.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "DataModel/Services/SectorDetail.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "DataModel/Services/ServiceFeeSecurity.h"
#include "TaxDisplay/View.h"

namespace tax
{
class LocService;
class RulesRecord;
class TaxRoundingInfoService;

namespace display
{
class TaxDisplayRequest;

class ViewX1SequenceDetail : public View
{
  friend class ViewX1SequenceDetailTest;

public:
  ViewX1SequenceDetail(const RulesRecord& rulesRecord,
                       const LocService& locService,
                       const TaxRoundingInfoService& roundingService,
                       const TaxDisplayRequest& request,
                       ResponseFormatter& responseFormatter)
    : View(responseFormatter),
      _roundingService(roundingService),
      _locService(locService),
      _rulesRecord(rulesRecord),
      _request(request) {}

  bool header() override;
  bool body() override;
  bool footer() override;

  void setTravelCarrierQualifyingTagsData(
      const std::shared_ptr<const CarrierApplication>& carrierApplication,
      const std::shared_ptr<const CarrierFlight>& carrierFlightBefore,
      const std::shared_ptr<const CarrierFlight>& carrierFlightAfter,
      const std::shared_ptr<const PassengerTypeCodeItems>& passengerTypeCodeItems);

  void setServiceBaggageData(
      const std::shared_ptr<const ServiceFeeSecurityItems>& serviceFeeSecurityItems,
      const std::shared_ptr<const ServiceBaggage>& serviceBaggage);

  void setTravelSectorData(
      const std::shared_ptr<const SectorDetail>& sectorDetail);

private:
  void printCategories();
  void categoryTax();
  void categoryDateApplications();
  void categorySaleTicketDelivery();
  void categoryTaxDetails();
  void categoryTicketValue();
  void categoryTaxableUnitDetails();
  void categoryTravelApplication();
  void categoryTaxPointSpecification();
  void categoryTravelCarrierQualifyingTags();
  void categoryServiceBaggageTax();
  void categoryTravelSectorDetails();
  void categoryConnectionTags();
  void categoryProcessingApplicationDetail();

  std::list<Line> getCarrierApplicationDisplay(const CarrierApplication& carrierApplication);
  std::list<Line> getCarrierFlightDisplay(const CarrierFlight& carrierFlight);
  std::list<Line> getPassengerApplicationDisplay(const PassengerTypeCodeItems& items);
  std::list<Line> getSectorDetailDisplay(const SectorDetail& sectorDetail);
  std::list<Line> getServiceFeeSecurityItemsDisplay(const ServiceFeeSecurityItems& items);
  std::list<Line> getServiceBaggageDisplay(const ServiceBaggage& serviceBaggage);

  std::shared_ptr<const CarrierApplication> _carrierApplication;
  std::shared_ptr<const CarrierFlight> _carrierFlightBefore;
  std::shared_ptr<const CarrierFlight> _carrierFlightAfter;
  std::shared_ptr<const PassengerTypeCodeItems> _passengerTypeCodeItems;
  std::shared_ptr<const SectorDetail> _sectorDetail;
  std::shared_ptr<const ServiceFeeSecurityItems> _serviceFeeSecurityItems;
  std::shared_ptr<const ServiceBaggage> _serviceBaggage;

  const TaxRoundingInfoService& _roundingService;
  const LocService& _locService;
  const RulesRecord& _rulesRecord;
  const TaxDisplayRequest& _request;
};

} // namespace display
} // namespace tax
