// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/GeoPathProperties.h"
#include "Processor/ApplicatorFactory.h"
#include "Processor/ApplyRuleFunctor.h"
#include "Processor/BusinessRulesProcessor.h"
#include "Processor/RawSubjectsCollector.h"
#include "Processor/TaxPointValidationProgress.h"
#include "Processor/TaxValidator.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/TaxData.h"

namespace tax
{

class RawSubjects;
class RawSubjectsCollector;
class TaxPointValidationProgress;

class TaxValidator
{
public:
  TaxValidator(BusinessRulesProcessor& processor,
               Services& services,
               const TaxValue& tax,
               const Geo& taxPoint,
               const Geo& nextPrevTaxPoint,
               const type::Index itinId,
               const GeoPathProperties& geoPathProperties,
               const Request& request,
               RawPayments& itinRawPayments,
               std::vector<PaymentWithRules>& paymentsToCalculate,
               type::CarrierCode& marketingCarrier,
               const type::ProcessingGroup& processingGroup);

  PaymentDetail&
  addPaymentDetails(const BusinessRulesContainer& rulesContainer,
                    const RawSubjects& subjects);

  bool
  validate(const BusinessRulesContainer& rulesContainer);

private:
  bool
  markDuplicatedOptionalServices(PaymentDetail& paymentDetail);

  BusinessRulesProcessor& _processor;
  Services& _services;
  const TaxValue& _tax;
  const Geo& _taxPoint;
  const Geo& _nextPrevTaxPoint;
  const type::Index _itinId;
  const GeoPathProperties& _geoPathProperties;
  const Request& _request;
  RawPayments& _itinRawPayments;
  std::vector<PaymentWithRules>& _paymentsToCalculate;
  type::CarrierCode& _marketingCarrier;
  RawSubjectsCollector _subjectsCollector;
  const RawSubjects& _subjects;
  TaxPointValidationProgress _validationProgress;
};

} // end of tax namespace
