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

#include "DataModel/Common/Types.h"
#include "Diagnostic/AtpcoDiagnostic.h"

namespace tax
{
class Services;
class ItinsPayments;
class TaxName;
class PaymentDetail;
class OptionalService;

class NegativeDiagnostic : public AtpcoDiagnostic
{

  friend class NegativeDiagnosticTest;

public:
  static const uint32_t NUMBER;

  NegativeDiagnostic(const ItinsPayments& itinsPayments,
                     const boost::ptr_vector<Parameter>& parameters,
                     Services& services);
  ~NegativeDiagnostic();

private:
  struct Filter
  {
    Filter()
      : nation(),
        taxCode(),
        taxType(),
        seq(),
        seqLimit(),
        isSeqRange(false),
        subCode(),
        group(),
        subGroup(),
        carrier(),
        type(type::OptionalServiceTag::Blank),
        showGP(false) {};

    type::Nation nation;
    type::TaxCode taxCode;
    type::TaxType taxType;
    type::SeqNo seq;
    type::SeqNo seqLimit;
    bool isSeqRange;
    type::OcSubCode subCode;
    type::ServiceGroupCode group;
    type::ServiceGroupCode subGroup;
    type::CarrierCode carrier;
    type::OptionalServiceTag type;
    bool showGP;
  };

  virtual void printHeader() override;
  virtual void applyParameters() override;
  virtual void runAll() override;

  bool filter(const TaxName& taxName, type::SeqNo seqNo) const;

  void printHelp();
  void printInvalidTaxes();

  void printPaymentDetail(const TaxName& taxName, const PaymentDetail& detail);
  void printOptionalService(const TaxName& taxName,
                            const PaymentDetail& detail,
                            const OptionalService& optionalService);
  bool printPaymentDetailFiltered(const TaxName& taxName, const PaymentDetail& detail);
  bool printOptionalServicesFiltered(const TaxName& taxName, const PaymentDetail& detail);

  Services& _services;
  const ItinsPayments& _itinsPayments;
  const boost::ptr_vector<Parameter>& _parameters;

  Printer<NegativeDiagnostic, &NegativeDiagnostic::printHelp> _helpPrinter;
  Printer<NegativeDiagnostic, &NegativeDiagnostic::printInvalidTaxes> _invalidTaxesPrinter;

  Filter _filter;

public:
  Printer<NegativeDiagnostic, &NegativeDiagnostic::printInvalidTaxes>& invalidTaxesPrinter()
  {
    return _invalidTaxesPrinter;
  }
};
}
