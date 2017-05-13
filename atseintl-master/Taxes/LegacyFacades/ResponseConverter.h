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

#include <boost/optional.hpp>

#include "Common/Logger.h"
#include "ServiceFees/OCFees.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputTaxDetails.h"
#include "Taxes/Common/AtpcoTypes.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"

#include <functional>
#include <iosfwd>
#include <memory>

namespace tse
{
class DateTime;
class FarePath;
class PricingTrx;
class TaxItem;
class TaxResponse;
class OCFees;
}

namespace tax
{
class DiagnosticResponse;
class OutputResponse;
class IoTaxName;
class OutputItins;
class OutputTaxDetails;

class ResponseConverter_DEPRECATED
{
  typedef std::function<std::vector<std::string>(const OutputTaxDetails&)> TaxDependenciesGetter;

public:
  friend class ResponseConverterTest;

  struct TseData
  {
    const tse::DateTime& ticketingDate;
    const tse::FarePath* farePath;

    TseData(const tse::DateTime&, const tse::FarePath*);
  };

  static void convertDiagnosticResponseToTaxResponse(tse::PricingTrx& trx,
                                                     const DiagnosticResponse& diagnosticResponse);

  static void updateV2Trx(tse::PricingTrx& trx,
                          const tax::OutputResponse& outResponse,
                          const V2TrxMappingDetails& v2TrxMappingDetails,
                          std::ostream* gsaDiag,
                          const TaxDependenciesGetter& getTaxDependencies);

private:
  class Impl;

  static void initializeTaxItem(tse::PricingTrx& trx,
                                tse::TaxResponse& taxResponse,
                                const boost::optional<IoTaxName>& taxName,
                                const type::CurrencyCode& paymentCurrency,
                                const type::CurDecimals& paymentCurrencyNoDec,
                                std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                                const TaxDependenciesGetter& getTaxDependencies);

  static void buildTaxItem(tse::PricingTrx& trx,
                           tse::TaxItem* pTaxItem,
                           const boost::optional<IoTaxName>& taxName,
                           const type::CurrencyCode& paymentCurrency,
                           const type::CurDecimals& paymentCurrencyNoDec,
                           std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                           const TseData& tseData,
                           const TaxDependenciesGetter& getTaxDependencies);

  static void
  initializeOCFeesTaxItem(tse::PricingTrx& trx,
                          const boost::optional<IoTaxName>& taxName,
                          const type::CurrencyCode& paymentCurrency,
                          const type::CurDecimals& paymentCurrencyNoDec,
                          const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
                          std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                          tse::FarePath* farePath);

  static void buildOCFeesTaxItem(tse::OCFees::TaxItem* pTaxItem,
                                 const boost::optional<IoTaxName>& taxName,
                                 const type::CurrencyCode& paymentCurrency,
                                 const type::CurDecimals& paymentCurrencyNoDec,
                                 std::shared_ptr<OutputTaxDetails>& outputTaxDetails);

  static tse::OCFees::TaxItem*
  buildOCFeesTaxItem(tse::PricingTrx& trx,
                     const boost::optional<IoTaxName>& taxName,
                     const type::CurrencyCode& paymentCurrency,
                     const type::CurDecimals& paymentCurrencyNoDec,
                     std::shared_ptr<OutputTaxDetails>& outputTaxDetails);

  static void initializeChangeFeeTaxItem(tse::PricingTrx& trx,
                                         tse::TaxResponse& taxResponse,
                                         const boost::optional<IoTaxName>& taxName,
                                         const type::CurrencyCode& paymentCurrency,
                                         const type::CurDecimals& paymentCurrencyNoDec,
                                         std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                                         const TaxDependenciesGetter& getTaxDependencies);

  static void
  convertToV2Response(tse::PricingTrx& trx,
                      const OutputItins& solutions,
                      const tax::ItinFarePathKey& mapping,
                      tse::TaxResponse* atseV2Response,
                      const TaxDependenciesGetter& getTaxDependencies);

  static void
  convertTaxGroup(
      tse::PricingTrx& trx,
      const boost::optional<type::Index> taxGroupId,
      const OutputItins& solutions,
      const TaxDependenciesGetter& getTaxDependencies,
      tse::TaxResponse* atseV2Response);

  static void
  convertChangeFeeGroup(
      tse::PricingTrx& trx,
      const boost::optional<type::Index> taxGroupId,
      const OutputItins& solutions,
      const TaxDependenciesGetter& getTaxDependencies,
      tse::TaxResponse* atseV2Response);

  static void
  prepareResponseForMarkups(
      tse::PricingTrx& trx,
      const tax::ItinFarePathKey& mapping,
      const tse::TaxResponse* taxResponse);

  static void
  convertOcToV2Response(tse::PricingTrx& trx,
                        const OutputItins& solutions,
                        const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
                        const tax::ItinFarePathKey& mapping,
                        tse::TaxResponse* atseV2Response,
                        const TaxDependenciesGetter& getTaxDependencies);

  static void
  buildTicketLineForTaxResponse(tse::PricingTrx& trx, tse::TaxResponse& taxResponse);

  static tse::Logger _logger;
};
}
