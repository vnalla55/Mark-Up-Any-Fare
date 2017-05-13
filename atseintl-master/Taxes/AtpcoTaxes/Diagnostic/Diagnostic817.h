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

#include <boost/format.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "DataModel/Common/Types.h"
#include "Diagnostic/AtpcoDiagnostic.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "AtpcoTaxes/Common/AtpcoTaxesActivationStatus.h"

namespace tax
{
class ItinPayments;
class ItinsPayments;
class TaxName;
class OptionalService;
class PaymentDetail;

class Diagnostic817 : public AtpcoDiagnostic
{
  friend class Diagnostic817Test;

public:
  static const uint32_t NUMBER;

  Diagnostic817(const Request& request,
                const ItinsPayments& itinsPayments,
                const AtpcoTaxesActivationStatus& activationStatus);
  ~Diagnostic817();

private:
  struct Filter
  {
    type::PassengerCode passengerCode {};
  };

  virtual void printHeader() override;
  void printFooter() override;
  virtual void applyParameters() override;
  virtual void runAll() override;
  virtual void printHelp();

  void printDiagnostic817();
  void printYqyrs(const Itin& itin, uint32_t& number);
  void printItins(const ItinPayments& itinPayments, uint32_t& number);
  void printTaxesOnChangeFee(const ItinPayments& itinPayments, uint32_t& number);
  void print(type::ProcessingGroup group, const std::string& headerName);
  void printOC();
  void printBG();

  void printItinHeader(const std::string& headerName,
                       const std::string& itinNumber,
                       const type::PassengerCode& passengerCode,
                       const type::CarrierCode& validatingCarrier);

  void printTaxLine(uint32_t number,
                    const type::SabreTaxCode& sabreTaxCode,
                    const TaxName& taxName,
                    const PaymentDetail& detail,
                    const type::AirportCode& taxPointBegin,
                    const type::AirportCode& taxPointEnd);

  void printTaxLine(uint32_t number,
                    const type::SabreTaxCode& sabreTaxCode,
                    const TaxName& taxName,
                    const PaymentDetail& detail,
                    type::MoneyAmount taxAmount,
                    const type::AirportCode& taxPointBegin,
                    const type::AirportCode& taxPointEnd);

  void printOcTaxLine(uint32_t number,
                    const type::SabreTaxCode& sabreTaxCode,
                    const TaxName& taxName,
                    const PaymentDetail& detail,
                    const OptionalService&,
                    const type::AirportCode& taxPointBegin,
                    const type::AirportCode& taxPointEnd);

  void printYqyrTaxLine(uint32_t number,
                    const YqYr& yqYr,
                    const type::AirportCode& taxPointBegin,
                    const type::AirportCode& taxPointEnd);

  void addTaxCodeMapping(const TaxName& taxName, const type::SabreTaxCode& sabreTaxCode);

  const ItinsPayments& _itinsPayments;
  const boost::ptr_vector<Parameter>& _parameters;
  const std::vector<Itin>& _itins;
  const std::vector<GeoPath>& _geoPaths;
  const std::vector<GeoPathMapping>& _geoPathMappings;
  const std::vector<YqYr>& _yqYrs;
  const std::vector<YqYrPath>& _yqYrPaths;
  Filter _filter;
  boost::format _formatter;
  std::string _columnNames;
  const AtpcoTaxesActivationStatus& _activationStatus;

  static const std::string PASSENGER_SYMBOL;
  static const std::string ITIN_SYMBOL;
  static const std::string TAX_HEADER_SYMBOL;
  static const std::string ANCILLARY_HEADER_SYMBOL;
  static const std::string BAGGAGE_HEADER_SYMBOL;
  static const std::string VALIDATING_CARRIER_SYMBOL;
  static const std::string SUBCODE_SYMBOL;
  static const std::string TAXCODE_MAPPING_HEADER;
  static const std::string TAXCODE_MAPPING_COLUMNS;

  Printer<Diagnostic817, &Diagnostic817::printDiagnostic817> _diagnosticPrinter;
  Printer<Diagnostic817, &Diagnostic817::printOC> _ocPrinter;
  Printer<Diagnostic817, &Diagnostic817::printBG> _bgPrinter;
  Printer<Diagnostic817, &Diagnostic817::printHelp> _helpPrinter;

  typedef std::map<std::pair<type::TaxCode, type::TaxType>, std::string> TaxCodeMapping;
  TaxCodeMapping _taxCodeMapping;

public:
  Printer<Diagnostic817, &Diagnostic817::printDiagnostic817>& diagnosticPrinter()
  {
    return _diagnosticPrinter;
  }

  Printer<Diagnostic817, &Diagnostic817::printOC>& ocPrinter() { return _ocPrinter; }
  Printer<Diagnostic817, &Diagnostic817::printBG>& bgPrinter() { return _bgPrinter; }
};

}
