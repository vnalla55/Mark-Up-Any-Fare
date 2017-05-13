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

#include "Common/AtpcoTaxesActivationStatus.h"
#include "Common/GroupedContainers.h"
#include "Common/TaxDetailsLevel.h"
#include "DomainDataObjects/ErrorMessage.h"
#include "DomainDataObjects/ExemptedRule.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Response.h"
#include "Processor/ExcItinTaxesSelector.h"
#include "Processor/TopologicalOrderer.h"
#include "Rules/GroupId.h"
#include "Rules/LimitGroup.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentWithRules.h"
#include "DataModel/Services/PreviousTicketTaxInfo.h"

#include <boost/ref.hpp>

#include <memory>
#include <vector>


namespace tax
{
struct GeoPathProperties;
struct CalculatorsGroups;
class TaxData;
class Request;
class Services;
class Itin;
class AtpcoDiagnostic;
class DiagnosticCommand;
class BusinessRulesContainer;

using TaxKey = std::pair<type::TaxCode, type::TaxType>;
using TaxValue = const TaxData*;
using ProcessingOrderer = TopologicalOrderer<TaxKey, TaxValue>;
using TaxValues = std::vector<TaxValue>;
using OrderedTaxes = std::vector<boost::reference_wrapper<const std::vector<TaxValue>>>;
using ItinsRawPayments = GroupedContainers<std::vector<RawPayments>>;

namespace BusinessRulesProcessorUtils
{
void
getOrderedTaxes(Request& request,
                Services& services,
                ProcessingOrderer& orderer,
                TaxValues& lastGroup,
                OrderedTaxes& orderedTaxes);
}

class BusinessRulesProcessor
{
  friend class BusinessRulesProcessorTest;
  friend class GeoPathPropertiesCalculatorTest;

public:
  BusinessRulesProcessor(Services& services);
  virtual ~BusinessRulesProcessor(void);

  void run(Request& request, const AtpcoTaxesActivationStatus& activationStatus);
  void runSingleItin(const Request& request,
                     type::Index itinIndex,
                     const OrderedTaxes& orderedTaxes,
                     ItinsRawPayments& itinsRawPayments,
                     ItinsPayments& itinsPayments);

  ErrorMessage& errorMessage() { return _errorMessage; }
  Response& response() { return _response; }
  const TaxDetailsLevel& detailsLevel() const { return _detailsLevel; }

  std::set<PreviousTicketTaxInfo> getComputedTaxesOnItin() const;
  bool matchesFilter(const TaxName& taxName, const BusinessRulesContainer& rulesContainer);

private:
  ErrorMessage _errorMessage;
  Services& _services;
  Response _response;
  TaxDetailsLevel _detailsLevel;
  type::Timestamp _ticketingDate;
  std::unique_ptr<ExcItinTaxesSelector> _taxesSelector;

  type::CarrierCode
  getMarketingCarrier(const Request& request, const Geo& taxPoint, const type::Index& itinId);
  void limitTax(const type::Index& itinId,
                const Request& request,
                RawPayments& itinRawPayments,
                std::vector<PaymentWithRules>& paymentsToCalculate,
                bool isAYTax = false);
  void calculateTax(const type::Index& itinId,
                    const Request& request,
                    RawPayments& itinRawPayments,
                    std::vector<PaymentWithRules>& paymentsToCalculate);
  Services& services() { return _services; }

  void createDiagnosticResponse(AtpcoDiagnostic& diagnostic);
  bool createDBDiagnosticResponse(const Request& request, Services& services);
  bool analyzeRequest(Request& request, Services& services);
  bool createRequestDiagnosticResponse(const DiagnosticCommand& diagnostic, const Request& request);

  void validateTax(const type::ProcessingGroup& processingGroup,
                   const TaxValue& tax,
                   const Geo& taxPoint,
                   const Geo& nextPrevTaxPoint,
                   const type::Index itinId,
                   const GeoPathProperties& geoPathProperties,
                   const Request& request,
                   RawPayments& itinRawPayments,
                   std::vector<PaymentWithRules>& paymentsToCalculate);

  void applyDepartureTax(const type::ProcessingGroup& processingGroup,
                         const TaxValue& tax,
                         const Itin& itin,
                         const GeoPathProperties& geoPathProperties,
                         const Request& request,
                         RawPayments& itinRawPayments);
  void applyArrivalTax(const type::ProcessingGroup& processingGroup,
                       const TaxValue& tax,
                       const Itin& itin,
                       const GeoPathProperties& geoPathProperties,
                       const Request& request,
                       RawPayments& itinRawPayments);
  void applySaleTax(const type::ProcessingGroup& processingGroup,
                    const TaxValue& tax,
                    const Itin& itin,
                    const GeoPathProperties& geoPathProperties,
                    const Request& request,
                    RawPayments& itinRawPayments);

  void calculateRawPayments(const type::ProcessingGroup& processingGroup,
                            const OrderedTaxes& orderedTaxes,
                            Request& request,
                            std::vector<RawPayments>& itinsRawPayments);
  ItinsRawPayments calculateRawPayments(Request& request, const OrderedTaxes& orderedTaxes);

  void calculateRawPaymentsSingleItin(const type::ProcessingGroup& processingGroup,
                                      const OrderedTaxes& orderedTaxes,
                                      const Request& request,
                                      std::vector<RawPayments>& itinsRawPayments,
                                      type::Index itinIndex);
  void calculateRawPaymentsSingleItin(const Request& request,
                                      const OrderedTaxes& orderedTaxes,
                                      type::Index itinIndex,
                                      ItinsRawPayments& itinsRawPayments);

  ItinsPayments
  filterPaymentsByDiagnostic(const std::vector<type::ProcessingGroup>& processingGroups,
                             const ItinsRawPayments& itinsRawPayments,
                             const std::vector<Itin*>& itins,
                             const DiagnosticCommand& diagnostic) const;

  void
  filterPaymentsSingleItin(const std::vector<type::ProcessingGroup>& processingGroups,
                           const ItinsRawPayments& itinsRawPayments,
                           const std::vector<Itin*>& itins,
                           type::Index itinIndex,
                           ItinsPayments& itinsPayments) const;

  void addBuildInfoDiagnosticToResponse(const DiagnosticCommand& diagnostic,
                                        const std::string& buildInfo);
  bool
  createRegularResponse(const DiagnosticCommand& diagnostic, ItinsPayments& itinsPayments);

  bool createPositiveDiagnosticResponse(const DiagnosticCommand& diagnostic,
                                        const ItinsPayments& itinsPayments);

  bool createNegativeDiagnosticResponse(const DiagnosticCommand& diagnostic,
                                        const ItinsPayments& itinsPayments);

  bool create817DiagnosticResponse(const Request& request,
                                   const ItinsPayments& itinsPayments,
                                   const AtpcoTaxesActivationStatus& activationStatus);

  bool createPostprocessResponse(const Request& request,
                                 ItinsPayments&& itinsPayments,
                                 const AtpcoTaxesActivationStatus& activationStatus);

  void finalRoundingForPercentageTaxes(RawPayments& itinRawPayments);

  struct InputFilter
  {
    type::Nation nation {UninitializedCode};
    type::TaxCode taxCode {UninitializedCode};
    type::TaxType taxType {UninitializedCode};
    type::SeqNo seq {};
    type::SeqNo seqLimit {};
    bool isSeqRange {false};
    type::Vendor vendor {UninitializedCode};
  };

  InputFilter _filter;

  void parseFilterParameters(const DiagnosticCommand& diagnostic);
};

} // namespace tax
