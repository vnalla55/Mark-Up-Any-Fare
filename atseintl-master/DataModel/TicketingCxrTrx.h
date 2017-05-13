//-------------------------------------------------------------------
//  Authors:
//
//  Copyright Sabre 2013
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
#include "Common/ValidatingCxrConst.h"
#include "DataModel/Billing.h"
#include "DataModel/TicketingCxrRequest.h"
#include "DataModel/Trx.h"
#include "Service/Service.h"

#include <string>
#include <vector>

namespace tse
{
class CountrySettlementPlanInfo;

class TicketingCxrTrx : public Trx
{
public:
  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  void setRequest(TicketingCxrRequest* request) { _request = request; }
  const TicketingCxrRequest* getRequest() const { return _request; }
  TicketingCxrRequest* getRequest() { return _request; }

  Billing*& billing() { return _billing; }
  const Billing* billing() const override { return _billing; }

  DateTime& inputDT() { return _inputDT; }
  const DateTime& inputDT() const { return _inputDT; }

  vcx::TicketType getTicketType() const { return _tktType; }
  void setTicketType(vcx::TicketType t) { _tktType = t; }

  vcx::ValidationResult getValidationResult() const { return _valRes; }
  void setValidationResult(vcx::ValidationResult s) { _valRes = s; }

  vcx::ValidationStatus getValidationStatus() const { return _valStatus; }
  void setValidationStatus(vcx::ValidationStatus s) { _valStatus = s; }

  bool isGsaSwap() const { return _isGsaSwap; }
  bool isMultipleGSASwap() const { return (_isGsaSwap && _valCxrs.size() > 1); }
  void setGsaSwap(bool b) { _isGsaSwap = b; }
  void setGTCCarrier(bool b) { _isGTCCarrier = b; }

  std::vector<CarrierCode>& validatingCxrs() { return _valCxrs; }
  const std::vector<CarrierCode>& validatingCxrs() const { return _valCxrs; }

  bool isResultValid() const;
  vcx::ValidationResult getResultType(vcx::ValidationStatus status) const;

  CountrySettlementPlanInfo*& ctrySettlementInfo() {return  _spInfo; }
  const CountrySettlementPlanInfo* ctrySettlementInfo() const {return  _spInfo; }

  vcx::ValidatingCxrData& vcxrData() { return _valCxrData; }
  const vcx::ValidatingCxrData& vcxrData() const { return _valCxrData; }

  void buildMessageText(std::string& text) const;

  NationCode getCountry() const {
    if (_request)
      return (!_request->getSpecifiedCountry().empty() ?
          _request->getSpecifiedCountry() :
          _request->getPosCountry());
    return "";
  }

  bool isSkipInterlineCheck() const {
    return (_request &&
        (_valCxrData.participatingCxrs.empty() ||
         (_valCxrData.participatingCxrs.size()==1 &&
          _request->getValidatingCxr()==_valCxrData.participatingCxrs.front().cxrName)));
  }

private:
  CountrySettlementPlanInfo* _spInfo = nullptr;
  vcx::ValidatingCxrData _valCxrData;
  TicketingCxrRequest* _request = nullptr;
  Billing* _billing = nullptr;
  DateTime _inputDT{DateTime::localTime()}; // IDT
  vcx::ValidationResult _valRes =
      vcx::ValidationResult::NO_RESULT; // valid,not-valid,singleGSA, multipleGSA etc
  vcx::ValidationStatus _valStatus =
      vcx::ValidationStatus::NO_MSG; // use to generate text message for response
  vcx::TicketType _tktType = vcx::TicketType::NO_TKT_TYPE; // E or P
  bool _isGsaSwap = false;
  bool _isGTCCarrier = false;
  std::vector<CarrierCode> _valCxrs;
};

inline bool
TicketingCxrTrx::isResultValid() const
{
  return (_valCxrs.size() >= 1 &&
      (_valRes == vcx::VALID ||
       _valRes == vcx::VALID_SINGLE_GSA_SWAP ||
       _valRes == vcx::VALID_MULTIPLE_GSA_SWAP));
}

inline vcx::ValidationResult
TicketingCxrTrx::getResultType(vcx::ValidationStatus vstatus)const
{
  if (vcx::VALID_MSG==vstatus || vcx::VALID_OVERRIDE==vstatus)
  {
    if (_isGsaSwap)
    {
      return (isMultipleGSASwap() ?
          vcx::VALID_MULTIPLE_GSA_SWAP :
          vcx::VALID_SINGLE_GSA_SWAP);
    }
    return vcx::VALID;
  }
  return vcx::NOT_VALID;
}
} // tse namespace
