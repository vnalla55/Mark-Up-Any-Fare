//-------------------------------------------------------------------
//  Authors:
//
//  Copyright Sabre 2014
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

#include "Common/TseCodeTypes.h"
#include "DataModel/Billing.h"
#include "DataModel/TicketingCxrDisplayRequest.h"
#include "DataModel/TicketingCxrDisplayResponse.h"
#include "DataModel/Trx.h"
#include "Service/Service.h"

#include <string>
#include <vector>

namespace tse
{

class TicketingCxrDisplayRequest;

class TicketingCxrDisplayTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  Billing*& billing() { return _billing; };
  const Billing* billing() const override { return _billing; };

  TicketingCxrDisplayRequest*& getRequest() { return _request; }
  const TicketingCxrDisplayRequest* getRequest() const { return _request; }

  TicketingCxrDisplayResponse& getResponse() { return _response; }
  const TicketingCxrDisplayResponse& getResponse() const { return _response; }

  const CrsCode getPrimeHost() const {
    if (_request) {
      return !_request->specifiedPrimeHost().empty() ?
        _request->specifiedPrimeHost() :
        _request->pointOfSale().primeHost();
    }
    return "";
  }

  const NationCode getCountry() const {
    if (_request) {
      return !_request->specifiedCountry().empty() ?
        _request->specifiedCountry() :
        _request->pointOfSale().country();
    }
    return "";
  }

  const CarrierCode getValidatingCxr() const {
    if (_request)
      return _request->validatingCxr();
    return "";
  }

  vcx::ValidationStatus getValidationStatus() const { return _valStatus; }
  void setValidationStatus(vcx::ValidationStatus s) { _valStatus = s; }

private:
  TicketingCxrDisplayRequest* _request = nullptr;
  TicketingCxrDisplayResponse _response;
  Billing* _billing = nullptr;
  vcx::ValidationStatus _valStatus = vcx::ValidationStatus::NO_MSG;
};
} // tse namespace
