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
#pragma once

#include "ServiceInterfaces/DefaultServices.h"
#include "Taxes/AtpcoTaxes/Diagnostic/LoggingService.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/AtpcoTaxesDriver.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/OptionalService.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/Common/AtpcoTypes.h"
#include "Taxes/LegacyFacades/ServicesFeesMap.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"

#include <memory>
#include <string>
#include <vector>

namespace tax
{
class BusinessRulesProcessor;
class InputRequest;
class OptionalService;
}

namespace tse
{
class PricingTrx;

class AtpcoTaxesDriverV2 : public tax::AtpcoTaxesDriver
{
  friend class AtpcoTaxesDriverTest;

public:
  class AtpcoCoreProcess;
  friend class AtpcoCoreProcess;

  typedef tax::V2TrxMappingDetails::OptionalServicesRefs OptionalServicesRefs;

  AtpcoTaxesDriverV2(PricingTrx& _trx, const tax::ServicesFeesMap& servicesFees);
  ~AtpcoTaxesDriverV2();

  // AtpcoTaxesDriver I
  virtual bool buildRequest() override;
  virtual void setServices() override;
  virtual bool processTaxes() override;
  virtual bool convertResponse() override;

  void setGsaDiagnostic(std::ostream& diag) { _gsaDiagnostic = &diag; }

private:
  class Impl;
  template <typename LogSrvc, typename ISrvc>
    std::unique_ptr<typename LogSrvc::BaseService>
    install(ISrvc* base);

  MoneyAmount getFeeAmount(tax::OptionalService& oc, const CurrencyCode& feeCurrency) const;
  void handleOcWithTaxInclInd(tax::Request& request);
  void processFullRequest(tax::Request& request);
  void processShortPath(const std::string& shortPathData);

  tax::DefaultServices _services;
  tse::PricingTrx& _trx;
  const tax::ServicesFeesMap& _servicesFees;
  std::unique_ptr<tax::BusinessRulesProcessor> _processor;
  tax::V2TrxMappingDetails _v2TrxMappingDetails;
  std::string _diagnostic;
  std::ostream* _gsaDiagnostic;
  std::unique_ptr<tax::Request> _request;
  std::string _xmlRequest;
  std::vector<tax::LoggingService*> _serviceLoggers;
  bool _doServiceLogging {false};
  tax::InputRequest* _inputRequest = nullptr;
};

} // namespace tse
