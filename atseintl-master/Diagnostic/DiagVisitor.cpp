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

#include "Common/RexPricingPhase.h"
#include "Diagnostic/Diag817Collector.h"
#include "Diagnostic/Diag825Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagVisitor.h"

#include <iostream>

namespace tse
{

void
PrintHeader::visit(DiagCollector& diag)
{
}

void
PrintHeader::visit(Diag825Collector& diag)
{
  diag.activate();

  if (RexPricingPhase(&_trx).isNewPhase())
  {
    diag << "- BEGIN NEW ITIN DIAGNOSTIC -\n";
  }
  else
  {
    diag << "- BEGIN EXC ITIN DIAGNOSTIC -\n";
  }

  diag << "***************************************************************\n"
      << "                REFUNDABLE INDICATOR\n"
      << "***************************************************************\n"
      << std::setw(8) << "CODE"
      << std::setw(9) << "CARRIER"
      << std::setw(11) << "REFUNDIND"
      << std::setw(10) << "SEQ\n";

}

PrintTaxReissueInfo::PrintTaxReissueInfo(const Cat33TaxReissue& cat33TaxReissue,
      const TaxCode& taxCode, const CarrierCode& carrierCode)
    : _taxResissueSeq(0),
      _taxCode(taxCode),
      _carrierCode(carrierCode),
      _refundInd(cat33TaxReissue.getRefundableTaxTag())
{
  if (cat33TaxReissue.getTaxReissue())
  {
    _taxResissueSeq = cat33TaxReissue.getTaxReissue()->seqNo();
  }
}

void
PrintTaxReissueInfo::visit(DiagCollector& diag)
{
}

void
PrintTaxReissueInfo::visit(Diag825Collector& diag)
{
  diag << std::setw(3) << diag.lineNumber()++
      << std::setw(5) << _taxCode
      << std::setw(9) << _carrierCode
      << std::setw(11) << _refundInd
      << std::setw(9) <<  _taxResissueSeq << "\n";

  diag.flushMsg();
}

PrintTaxesOnChangeFee::PrintTaxesOnChangeFee(ItinSelector& itinSelector)
{
  if (itinSelector.isNewItin())
  {
    if (itinSelector.getItin().empty())
    {
      _isNewItinAndEmpty = true;
      _excItins = itinSelector.get();
    }
  }
}

void
PrintTaxesOnChangeFee::printTaxResponse(Diag817Collector& diag, TaxResponse* taxResponse)
{
  diag.enable(Diagnostic817);
  diag << *taxResponse;
  diag.displayInfo();
}

void
PrintTaxesOnChangeFee::visit(Diag817Collector& diag)
{
  if (diag.rootDiag() && diag.rootDiag()->diagParamMapItem(Diagnostic::ITIN_TYPE) != "ALL")
    diag << "FOR EXC ITINERARY USE 817/ITEXC\n";

  if (!_isNewItinAndEmpty)
    return;

  for(Itin* itin : _excItins)
  {
    for (TaxResponse* taxResponse : itin->getTaxResponses())
    {
      if (taxResponse->changeFeeTaxItemVector().empty())
        continue;

      TaxResponse taxResponseCopy = *taxResponse;
      taxResponseCopy.taxItemVector().clear();
      printTaxResponse(diag, &taxResponseCopy);
    }
  }

  diag.flushMsg();
}

} // end of tse namespace
