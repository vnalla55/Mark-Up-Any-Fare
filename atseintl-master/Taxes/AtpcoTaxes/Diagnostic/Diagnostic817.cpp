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
#include "Common/MoneyUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "Diagnostic/Diagnostic817.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/ItinsPayments.h"
#include "DomainDataObjects/OptionalService.h"
#include "ServiceInterfaces/Services.h"

#include <iomanip>
#include <map>
#include <sstream>
#include <tuple>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{
class Trx;
}

namespace tax
{
namespace
{
std::string
getOCInfoText(bool isTaxInclInd)
{
  static const std::string TAX_INCLUDED = "TAX INCLUDED";
  return isTaxInclInd ? TAX_INCLUDED : " ";
}
}

const uint32_t Diagnostic817::NUMBER = 817;
const std::string Diagnostic817::PASSENGER_SYMBOL = "PSGR";
const std::string Diagnostic817::ITIN_SYMBOL = "ITIN";
const std::string Diagnostic817::TAX_HEADER_SYMBOL = "ATPCO TAX OUT VECTOR";
const std::string Diagnostic817::ANCILLARY_HEADER_SYMBOL = "ATPCO ANCILLARY TAX OUT VECTORS";
const std::string Diagnostic817::BAGGAGE_HEADER_SYMBOL = "ATPCO BAGGAGE TAX OUT VECTORS";
const std::string Diagnostic817::VALIDATING_CARRIER_SYMBOL = "VALIDATING CARRIER";
const std::string Diagnostic817::SUBCODE_SYMBOL = "SUBCODE";
const std::string Diagnostic817::TAXCODE_MAPPING_HEADER = "ATP/SABR TAXCODE/TXTYPE MAPPING";
const std::string Diagnostic817::TAXCODE_MAPPING_COLUMNS = "ATP TXCODE/TXTYPE    SABR TXCODE";

Diagnostic817::Diagnostic817(const Request& request,
                             const ItinsPayments& itinsPayments,
                             const AtpcoTaxesActivationStatus& activationStatus)
  : _itinsPayments(itinsPayments),
    _parameters(request.diagnostic().parameters()),
    _itins(request.allItins()),
    _geoPaths(request.geoPaths()),
    _geoPathMappings(request.geoPathMappings()),
    _yqYrs(request.yqYrs()),
    _yqYrPaths(request.yqYrPaths()),
    _filter(),
    _formatter(),
    _columnNames(),
    _activationStatus(activationStatus),
    _taxCodeMapping()
{
  _formatter =
      boost::format("%|2|%|=4| %|-3|%|7.2f|%|3|%|7.2f| %|7.2f| %|=5| %|=3| %|=7|%|7|%|3|\n");

  _columnNames = str(_formatter % "" % "CODE" % "TYP" % "TXAMT" % "" % "TXTTL" % "TXFARE" %
                     "BOARD" % "OFF" % "CARRIER" % "SEQNO " % "SPN");
}

Diagnostic817::~Diagnostic817(void)
{
}

void
Diagnostic817::runAll()
{
  diagnosticPrinter().print(*this);
  ocPrinter().print(*this);
  bgPrinter().print(*this);
  _helpPrinter.print(*this);
  printLine('-');
}

void
Diagnostic817::applyParameters()
{
  bool diagEnabled = false;

  for (Parameter const& parameter: _parameters)
  {
    if (parameter.name() == "HE" || parameter.name() == "HELP")
    {
      _helpPrinter.enable();
      diagEnabled = true;
    }
    else if (parameter.name() == "OC")
    {
      ocPrinter().enable();
      diagEnabled = true;
    }
    else if (parameter.name() == "BC")
    {
      bgPrinter().enable();
      diagEnabled = true;
    }
    else if (parameter.name() == "PA")
    {
      type::PassengerCode paxCode (UninitializedCode);
      codeFromString(parameter.value(), paxCode);
      _filter.passengerCode = paxCode;
    }
  }

  if (!diagEnabled)
  {
    diagnosticPrinter().enable();
  }
}

void
Diagnostic817::printDiagnostic817()
{
  for (const ItinPayments& itinPayments: _itinsPayments._itinPayments)
  {
    uint32_t number = 0;

    const Itin& itin = _itins[itinPayments.itinId()];
    printItinHeader(TAX_HEADER_SYMBOL,
                    itin.label(),
                    itinPayments.requestedPassengerCode(),
                    itinPayments.validatingCarrier());

    _result << _columnNames;

    if (_activationStatus.isTaxOnItinYqYrTaxOnTax())
    {
      printYqyrs(itin, number);
    }

    if (_activationStatus.isTaxOnItinYqYrTaxOnTax())
    {
      printItins(itinPayments, number);
    }

    if (_activationStatus.isTaxOnChangeFee())
    {
      printTaxesOnChangeFee(itinPayments, number);
    }
  }

  printFooter();
}

void
Diagnostic817::printYqyrs(const Itin& itin, uint32_t& number)
{
  CompactOptional<type::Index> yqyrGeoIndex = itin.yqYrPathGeoPathMappingRefId();

  if (itin.yqYrPath() && yqyrGeoIndex.has_value())
  {
    assert(yqyrGeoIndex.value() < _geoPathMappings.size());

    const GeoPath& geoPath = _geoPaths[itin.geoPathRefId()];
    const YqYrPath& yqYrPath = *itin.yqYrPath();
    const GeoPathMapping& yqYrPathMapping = _geoPathMappings[yqyrGeoIndex.value()];
    for (size_t usageIdx = 0; usageIdx < yqYrPath.yqYrUsages().size(); ++usageIdx)
    {
      const Mapping& mapping = yqYrPathMapping.mappings()[usageIdx];
      const YqYrUsage& yqYrUsage = yqYrPath.yqYrUsages()[usageIdx];
      const YqYr& yqYr = _yqYrs[yqYrUsage.index()];

      const Geo* taxPointBoard = &geoPath.geos()[mapping.maps().front().index()];
      const Geo* taxPointOff = &geoPath.geos()[mapping.maps().back().index()];

      printYqyrTaxLine(++number, yqYr, taxPointBoard->locCode(), taxPointOff->locCode());
    }
  }
}

void
Diagnostic817::printItins(const ItinPayments& itinPayments, uint32_t& number)
{
  for (Payment const& payment : itinPayments.payments(type::ProcessingGroup::Itinerary))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      if (!detail->isValidForItinerary() && detail->getYqYrDetails().areAllFailed())
        continue;

      if (!_filter.passengerCode.empty() &&
          (itinPayments.requestedPassengerCode() != _filter.passengerCode))
      {
        continue;
      }

      const Geo* taxPointBoard = &detail->getTaxPointBegin(),
                 *taxPointOff = &detail->getTaxPointEnd();
      if (taxName.taxPointTag() == type::TaxPointTag::Sale &&
          itinPayments.itinId() < _itins.size())
      {
        const std::vector<Geo>& geos = _itins[itinPayments.itinId()].geoPath()->geos();
        taxPointBoard = &geos.front();
        taxPointOff = &geos.back();
      }
      else if (taxName.taxPointTag() == type::TaxPointTag::Arrival)
      {
        taxPointBoard = &detail->getTaxPointEnd();
        taxPointOff = &detail->getTaxPointBegin();
      }

      addTaxCodeMapping(taxName, detail->sabreTaxCode());
      printTaxLine(++number,
                   detail->sabreTaxCode(),
                   taxName,
                   *detail,
                   taxPointBoard->locCode(),
                   taxPointOff->locCode());

      _result << "\n";
    }
  }
}

void
Diagnostic817::printTaxesOnChangeFee(const ItinPayments& itinPayments, uint32_t& number)
{
  for (Payment const& payment : itinPayments.payments(type::ProcessingGroup::ChangeFee))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      if (!detail->isCalculated())
        continue;

      if (!_filter.passengerCode.empty() &&
          (itinPayments.requestedPassengerCode() != _filter.passengerCode))
      {
        continue;
      }

      const Geo* taxPointBoard = &detail->getTaxPointBegin(),
                 *taxPointOff = &detail->getTaxPointEnd();
      if (taxName.taxPointTag() == type::TaxPointTag::Sale &&
          itinPayments.itinId() < _itins.size())
      {
        const std::vector<Geo>& geos = _itins[itinPayments.itinId()].geoPath()->geos();
        taxPointBoard = &geos.front();
        taxPointOff = &geos.back();
      }
      else if (taxName.taxPointTag() == type::TaxPointTag::Arrival)
      {
        taxPointBoard = &detail->getTaxPointEnd();
        taxPointOff = &detail->getTaxPointBegin();
      }

      addTaxCodeMapping(taxName, detail->sabreTaxCode());

      printTaxLine(++number,
                   detail->sabreTaxCode(),
                   taxName,
                   *detail,
                   detail->taxOnChangeFeeAmount(),
                   taxPointBoard->locCode(),
                   taxPointOff->locCode());

      _result << "\n";
    }
  }
}

void
Diagnostic817::print(type::ProcessingGroup group, const std::string& headerName)
{
  typedef std::tuple<const OptionalService*, const TaxName*, const PaymentDetail*> DataRefs;
  typedef std::map<tax::type::OcSubCode, std::vector<DataRefs> > OcMap;
  typedef std::map<tax::type::OcSubCode, bool> OcTaxInclIndMap;

  for (const ItinPayments& itinPayments : _itinsPayments._itinPayments)
  {
    if (!_filter.passengerCode.empty() &&
        (itinPayments.requestedPassengerCode() != _filter.passengerCode))
    {
      continue;
    }

    // collecting itin/passenger taxes
    OcMap ocMap;
    OcTaxInclIndMap ocTaxInclIndMap;
    for (Payment const& payment : itinPayments.payments(group))
    {
      const TaxName& taxName = payment.taxName();
      for (const PaymentDetail* detail : payment.paymentDetail())
      {
        for (OptionalService const& oc : detail->optionalServiceItems())
        {
          if (oc.isFailed())
          {
            continue;
          }

          ocTaxInclIndMap[oc.subCode()] = oc.taxInclInd();
          ocMap[oc.subCode()].push_back(std::make_tuple(&oc, &taxName, detail));
        }
      }
    }

    printItinHeader(headerName,
                    _itins[itinPayments.itinId()].label(),
                    itinPayments.requestedPassengerCode(),
                    itinPayments.validatingCarrier());

    _result << _columnNames << "\n";

    // print data for one itin/passenger
    uint32_t number = 0;
    for (const OcMap::value_type& pair : ocMap)
    {
      const tax::type::OcSubCode& subCode = pair.first;
      printHeaderShort(SUBCODE_SYMBOL + " " + subCode.asString() + " " +
                       getOCInfoText(ocTaxInclIndMap[subCode]));
      for (const DataRefs& data : pair.second)
      {
        const OptionalService& os = *std::get<0>(data);
        const TaxName& taxName = *std::get<1>(data);
        const PaymentDetail& detail = *std::get<2>(data);

        addTaxCodeMapping(taxName, os.sabreTaxCode());
        printOcTaxLine(++number,
                     os.sabreTaxCode(),
                     taxName,
                     detail,
                     os,
                     os.getTaxPointBegin().locCode(),
                     os.getTaxPointEnd().locCode());
      }
    }
  }
  printFooter();
}


void
Diagnostic817::printOC()
{
  return print(type::ProcessingGroup::OC, ANCILLARY_HEADER_SYMBOL);
}

void
Diagnostic817::printBG()
{
  return print(type::ProcessingGroup::Baggage, BAGGAGE_HEADER_SYMBOL);
}

void
Diagnostic817::printOcTaxLine(uint32_t number,
                            const type::SabreTaxCode& sabreTaxCode,
                            const TaxName& taxName,
                            const PaymentDetail& detail,
                            const OptionalService& oc,
                            const type::AirportCode& taxPointBegin,
                            const type::AirportCode& taxPointEnd)
{
  type::MoneyAmount taxAmt = detail.taxAmt();

  if (taxName.percentFlatTag() == type::PercentFlatTag::Percent)
  {
    taxAmt *= 100;
  }

  type::MoneyAmount taxAmount = detail.isCommandExempt() ? 0 : oc.getTaxEquivalentAmount();

  _result << _formatter % number % sabreTaxCode % taxName.percentFlatTag() %
                 amountToDouble(taxAmt) % detail.taxCurrency() % amountToDouble(taxAmount) %
                 amountToDouble(oc.amount()) % taxPointBegin % taxPointEnd %
                 detail.marketingCarrier() % detail.seqNo() % detail.spn();
}

void
Diagnostic817::printTaxLine(uint32_t number,
                            const type::SabreTaxCode& sabreTaxCode,
                            const TaxName& taxName,
                            const PaymentDetail& detail,
                            const type::AirportCode& taxPointBegin,
                            const type::AirportCode& taxPointEnd)
{
  type::MoneyAmount taxAmt = detail.taxAmt();

  if (taxName.percentFlatTag() == type::PercentFlatTag::Percent)
  {
    taxAmt *= 100;
  }

  type::MoneyAmount taxAmount = detail.isCommandExempt() ? 0 : detail.taxEquivalentAmount();

  _result << _formatter % number % sabreTaxCode % taxName.percentFlatTag() %
                 amountToDouble(taxAmt) % detail.taxCurrency() % amountToDouble(taxAmount) %
                 amountToDouble(detail.totalTaxableAmount()) % taxPointBegin % taxPointEnd %
                 detail.marketingCarrier() % detail.seqNo() % detail.spn();
}

void
Diagnostic817::printTaxLine(uint32_t number,
                            const type::SabreTaxCode& sabreTaxCode,
                            const TaxName& taxName,
                            const PaymentDetail& detail,
                            type::MoneyAmount taxAmount,
                            const type::AirportCode& taxPointBegin,
                            const type::AirportCode& taxPointEnd)
{
  type::MoneyAmount taxAmt = detail.taxAmt();

  if (taxName.percentFlatTag() == type::PercentFlatTag::Percent)
  {
    taxAmt *= 100;
  }

  _result << _formatter % number % sabreTaxCode % taxName.percentFlatTag() %
                 amountToDouble(taxAmt) % detail.taxCurrency() % amountToDouble(taxAmount) %
                 amountToDouble(detail.totalTaxableAmount()) % taxPointBegin % taxPointEnd %
                 detail.marketingCarrier() % detail.seqNo() % detail.spn();
}

void
Diagnostic817::printYqyrTaxLine(uint32_t number,
                            const YqYr& yqYr,
                            const type::AirportCode& taxPointBegin,
                            const type::AirportCode& taxPointEnd)
{
  std::string yqYrCode = yqYr.code().asString() + yqYr.type();

  // TODO: add handling of percent YQ/YRs, currently assuming all are flat
  _result << _formatter % number % yqYrCode % 'F' % amountToDouble(yqYr.originalAmount()) %
                 yqYr.originalCurrency() % amountToDouble(yqYr.amount()) % amountToDouble(0) %
                 taxPointBegin % taxPointEnd % yqYr.carrierCode() % yqYr.seqNo() % 0;
}

void
Diagnostic817::printHelp()
{
  printHeaderLong("HELP");

  _result << "NO PARAM  - DIAGNOSTIC 817\n"
          << "OC        - TAXES ON OC\n"
          << "BC        - TAXES ON BAGGAGE CHARGES\n"
          << "PAXXX     - ONLY FOR PASSENGER XXX\n"
          << "HELP      - HELP INFO\n"
          << "\n";

  printInputFilterHelp();
}

void
Diagnostic817::printHeader()
{
}

void
Diagnostic817::printFooter()
{
  if (_taxCodeMapping.empty())
    return;

  printLine('*');
  _result << TAXCODE_MAPPING_HEADER << '\n';
  _result << TAXCODE_MAPPING_COLUMNS << '\n';

  boost::format mappingFormat = boost::format("%s %-17s %-3s\n");

  for (const TaxCodeMapping::value_type& each: _taxCodeMapping)
  {
    _result << mappingFormat % each.first.first % each.first.second % each.second;
  }
}

void
Diagnostic817::printItinHeader(const std::string& headerName,
                               const std::string& itinNumber,
                               const type::PassengerCode& passengerCode,
                               const type::CarrierCode& validatingCarrier)
{
  std::string itinLabel;
  if (!itinNumber.empty())
  {
    itinLabel = ITIN_SYMBOL + " " + itinNumber + " - ";
  }

  printLine('*');
  printHeaderShort(headerName + " - " + itinLabel + PASSENGER_SYMBOL + " " + passengerCode.asString());
  printLine('*');
  _result << VALIDATING_CARRIER_SYMBOL + ": " + validatingCarrier.asString() << "\n";
  printLine('*');
}

void
Diagnostic817::addTaxCodeMapping(const TaxName& taxName, const type::SabreTaxCode& sabreTaxCode)
{
  _taxCodeMapping[std::make_pair(taxName.taxCode(), taxName.taxType())] = sabreTaxCode;
}

}
