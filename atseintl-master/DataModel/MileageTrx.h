//-------------------------------------------------------------------
//
//  Description: Mileage Transaction object
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Trx.h"
#include "Service/Service.h"

namespace tse
{

class MileageTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  struct MileageItem
  {
    uint16_t itemNumber = 0; // WNO
    const Loc* cityLoc = nullptr; // CIT
    CarrierCode carrierCode = INDUSTRY_CARRIER; // CXR
    std::string tpmGlobalDirection; // TGD
    bool isHidden = false; // FTH
    bool isSurface = false; // ARK
    StopType stopType = StopType::Connection; // STP
  };

  void setRequest(PricingRequest* request) { _request = request; }
  const PricingRequest* getRequest() const { return _request; }
  PricingRequest* getRequest() { return _request; }

  Billing*& billing() { return _billing; }
  const Billing* billing() const override { return _billing; }

  DateTime& inputDT() { return _inputDT; }
  const DateTime& inputDT() const { return _inputDT; }

  DateTime& travelDT() { return _travelDT; }
  const DateTime& travelDT() const { return _travelDT; }

  std::vector<MileageItem*>& items() { return _items; }
  const std::vector<MileageItem*>& items() const { return _items; }

  std::ostringstream& response() { return _response; }

  bool isFromItin() const { return _isFromItin; }
  bool& isFromItin() { return _isFromItin; }

private:
  PricingRequest* _request = nullptr;
  Billing* _billing = nullptr;

  bool _isFromItin = false;

  DateTime _inputDT{DateTime::localTime()}; // IDT
  DateTime _travelDT{_inputDT};

  std::vector<MileageItem*> _items;

  std::ostringstream _response;

}; // class MileageTrx
} // tse namespace
