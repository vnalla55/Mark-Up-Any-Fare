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

class ResponseConverter
{
  friend class ResponseConverterTest;
  typedef std::function<std::vector<std::string>(const OutputTaxDetails&)> TaxDependenciesGetter;

public:

  ResponseConverter(tse::PricingTrx& trx,
      tax::AtpcoTaxesActivationStatus activationStatus) :
      _trx(trx), _activationStatus(activationStatus)
  {
  }

  struct TseData
  {
    const tse::DateTime& ticketingDate;
    const tse::FarePath* farePath;

    TseData(const tse::DateTime&, const tse::FarePath*);
  };

  void convertDiagnosticResponseToTaxResponse(
      const DiagnosticResponse& diagnosticResponse);

  void updateV2Trx(const tax::OutputResponse& outResponse,
      const V2TrxMappingDetails& v2TrxMappingDetails, std::ostream* gsaDiag,
      const TaxDependenciesGetter& getTaxDependencies);

private:
  void removeResponsesForEliminatedValCxrs(
      const ItinFarePathMapping& solutionMap);

  template<typename MapFarePath>
  void updateV2TrxFromMappings(const boost::optional<OutputItins>& solutions,
      const ItinFarePathMapping& solutionMap,
      const V2TrxMappingDetails& v2TrxMappingDetails, MapFarePath mapFarePath,
      const TaxDependenciesGetter& getTaxDependencies);

  void
  addOcLikeTaxItems(const boost::optional<type::Index>& groupRefId,
      const OutputItins& itins, const tse::Itin* const tseItin,
      const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
      tse::TaxResponse* atseV2Response, tse::FarePath* farePath,
      const TaxDependenciesGetter& getTaxDependencies);

  void initializeTaxItem(tse::TaxResponse& taxResponse,
      const boost::optional<IoTaxName>& taxName,
      const type::CurrencyCode& paymentCurrency,
      const type::CurDecimals& paymentCurrencyNoDec,
      std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
      const TaxDependenciesGetter& getTaxDependencies);

  void buildTaxItem(tse::TaxItem* pTaxItem,
      const boost::optional<IoTaxName>& taxName,
      const type::CurrencyCode& paymentCurrency,
      const type::CurDecimals& paymentCurrencyNoDec,
      std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
      const TseData& tseData, const TaxDependenciesGetter& getTaxDependencies);

  void
  initializeOCFeesTaxItem(const boost::optional<IoTaxName>& taxName,
      const type::CurrencyCode& paymentCurrency,
      const type::CurDecimals& paymentCurrencyNoDec,
      const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
      std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
      tse::FarePath* farePath);

  void buildOCFeesTaxItem(tse::OCFees::TaxItem* pTaxItem,
      const boost::optional<IoTaxName>& taxName,
      const type::CurrencyCode& paymentCurrency,
      const type::CurDecimals& paymentCurrencyNoDec,
      std::shared_ptr<OutputTaxDetails>& outputTaxDetails);

  tse::OCFees::TaxItem*
  buildOCFeesTaxItem(const boost::optional<IoTaxName>& taxName,
      const type::CurrencyCode& paymentCurrency,
      const type::CurDecimals& paymentCurrencyNoDec,
      std::shared_ptr<OutputTaxDetails>& outputTaxDetails);

  void initializeChangeFeeTaxItem(tse::TaxResponse& taxResponse,
      const boost::optional<IoTaxName>& taxName,
      const type::CurrencyCode& paymentCurrency,
      const type::CurDecimals& paymentCurrencyNoDec,
      std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
      const TaxDependenciesGetter& getTaxDependencies);

  void
  convertToV2Response(const OutputItins& solutions,
      const tax::ItinFarePathKey& mapping, tse::TaxResponse* atseV2Response,
      const TaxDependenciesGetter& getTaxDependencies);

  void
  convertTaxGroup(const boost::optional<type::Index> taxGroupId,
      const OutputItins& solutions,
      const TaxDependenciesGetter& getTaxDependencies,
      tse::TaxResponse* atseV2Response);

  void
  convertChangeFeeGroup(const boost::optional<type::Index> taxGroupId,
      const OutputItins& solutions,
      const TaxDependenciesGetter& getTaxDependencies,
      tse::TaxResponse* atseV2Response);

  void
  prepareResponseForMarkups(const tax::ItinFarePathKey& mapping,
      const tse::TaxResponse* taxResponse);

  void
  convertOcToV2Response(const OutputItins& solutions,
      const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
      const tax::ItinFarePathKey& mapping, tse::TaxResponse* atseV2Response,
      const TaxDependenciesGetter& getTaxDependencies);

  void
  buildTicketLineForTaxResponse(tse::TaxResponse& taxResponse);

  static tse::Logger _logger;

  tse::PricingTrx& _trx;
  tax::AtpcoTaxesActivationStatus _activationStatus;
};
} // end of tse namespace
