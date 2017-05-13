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

#include "Diagnostic/AtpcoDiagnostic.h"

namespace tax
{
class ItinsPayments;
class ItinPayments;
class OptionalService;
class TaxName;
class PaymentDetail;

class PositiveDiagnostic : public AtpcoDiagnostic
{
  friend class PositiveDiagnosticTest;

  static const std::string TAX_INCLUDED;

public:
  static const uint32_t NUMBER;

  PositiveDiagnostic(const ItinsPayments& itinsPayments,
                     const boost::ptr_vector<Parameter>& parameters);
  ~PositiveDiagnostic();

private:
  struct Filter
  {
    Filter() : itemNumber(), passengerCode(), isGPPrinter(), isDCPrinter() {};

    bool isValidPassenger(const type::PassengerCode& requestedPassengerCode)
    {
      return (passengerCode.empty() || (requestedPassengerCode == passengerCode));
    }

    bool noPrinterEnabled() const { return !(isGPPrinter || isDCPrinter); }

    type::Index itemNumber;
    type::PassengerCode passengerCode;
    bool isGPPrinter;
    bool isDCPrinter;
  };

  virtual void printHeader() override;
  virtual void applyParameters() override;
  virtual void runAll() override;
  virtual void printHelp();

  void printValidTaxes();
  void printValidTaxesGroup(const ItinPayments& itinPayments,
                            const type::ProcessingGroup& processingGroup,
                            const std::string& header,
                            uint32_t& number);
  void printValidTaxesGroupOC(const ItinPayments& itin,
                              const std::string& header,
                              const type::ProcessingGroup&,
                              uint32_t& number);

  void printTaxes();
  void
  printTaxesGroup(const ItinPayments& itinPayments, const type::ProcessingGroup& processingGroup);
  void printTaxesGroupOCType(const ItinPayments& itinPayments,
                             const type::ProcessingGroup& processingGroup);

  void printGeoProperties();
  void printGeoPropertiesGroup(const ItinPayments& itinPayments,
                               const type::ProcessingGroup& processingGroup);
  void printGeoPropertiesGroupOCType(const ItinPayments& itinPayments,
                                     const type::ProcessingGroup& group);
  void printFilterTax();
  void printFilterTaxGroup(const ItinPayments& itinPayments,
                           const type::ProcessingGroup& processingGroup,
                           const std::string& header,
                           uint32_t& number);

  void printFilterOS(const ItinPayments& itin,
                     const std::string& header,
                     const type::ProcessingGroup&,
                     uint32_t& number);

  void printTaxLine(uint32_t number,
                    const TaxName& taxName,
                    const PaymentDetail& detail,
                    const type::AirportCode& taxPointBegin,
                    const type::AirportCode& taxPointEnd,
                    const type::MoneyAmount& total,
                    char isTaxInclInd);

  void printValidTaxHeader(const ItinPayments& itinPayments, const std::string& header);

  void printTaxHeader(const TaxName& taxName,
                      const type::SeqNo& seqNo,
                      const type::PassengerCode& passengerCode);

  void printOCHeader(const OptionalService& oc);

  const ItinsPayments& _itinsPayments;
  const boost::ptr_vector<Parameter>& _parameters;
  Filter _filter;
  std::string _columnNames;

  Printer<PositiveDiagnostic, &PositiveDiagnostic::printHelp> _helpPrinter;
  Printer<PositiveDiagnostic, &PositiveDiagnostic::printValidTaxes> _validTaxesPrinter;
  Printer<PositiveDiagnostic, &PositiveDiagnostic::printTaxes> _taxPrinter;
  Printer<PositiveDiagnostic, &PositiveDiagnostic::printGeoProperties> _geoPropertiesPrinter;
  Printer<PositiveDiagnostic, &PositiveDiagnostic::printFilterTax> _filterPrinter;

public:
  Printer<PositiveDiagnostic, &PositiveDiagnostic::printValidTaxes>& validTaxesPrinter()
  {
    return _validTaxesPrinter;
  }

  Printer<PositiveDiagnostic, &PositiveDiagnostic::printTaxes>& validPrinter()
  {
    return _taxPrinter;
  }

  Printer<PositiveDiagnostic, &PositiveDiagnostic::printFilterTax>& filterPrinter()
  {
    return _filterPrinter;
  }
};
}
