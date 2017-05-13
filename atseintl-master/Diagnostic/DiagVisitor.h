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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/Cat33TaxReissue.h"
#include "Taxes/LegacyFacades/ItinSelector.h"

#include <vector>

namespace tse
{
class DiagCollector;
class Diag817Collector;
class Diag825Collector;
class TaxResponse;


class DiagVisitor
{
public:
  virtual void visit(DiagCollector&) = 0;
  virtual void visit(Diag825Collector&) = 0;
  virtual void visit(Diag817Collector&) = 0;
};

class PrintHeader : public DiagVisitor
{
  PricingTrx& _trx;
public:
  explicit PrintHeader(PricingTrx& trx) : _trx(trx) {}
  void visit(DiagCollector&) override;
  void visit(Diag817Collector&) override {}
  void visit(Diag825Collector&) override;
};

class PrintTaxReissueInfo : public DiagVisitor
{
  int _taxResissueSeq;
  TaxCode _taxCode;
  CarrierCode _carrierCode;
  Indicator _refundInd;
public:
  explicit PrintTaxReissueInfo(const Cat33TaxReissue& cat33TaxReissue,
      const TaxCode& taxCode, const CarrierCode& carrierCode);
  void visit(DiagCollector&) override;
  void visit(Diag817Collector&) override {}
  void visit(Diag825Collector&) override;
};

class PrintTaxesOnChangeFee : public DiagVisitor
{
  bool _isNewItinAndEmpty = false;
  std::vector<Itin*> _excItins;
public:
  PrintTaxesOnChangeFee(ItinSelector& itinSelector);
  void visit(DiagCollector&) override {}
  void visit(Diag817Collector& diag) override;
  void visit(Diag825Collector&) override {}
private:
  void printTaxResponse(Diag817Collector& diag, TaxResponse* taxResponse);
};

} // end of tse namespace
