//-------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollectorGuard.h"
#include "Pricing/Combinations.h"

namespace tse
{

class CombinationsSubCat
{

public:
  CombinationsSubCat(PricingTrx& trx,
                     DiagCollector& diag,
                     const VendorCode& vendor,
                     const uint32_t itemNo,
                     const PricingUnit& prU,
                     const FareUsage& fu,
                     Combinations::ValidationFareComponents& components,
                     bool& negativeApplication,
                     char subCatNumber,
                     DiagnosticTypes diagnostic)
    : _trx(trx),
      _diag(diag),
      _vendor(vendor),
      _itemNo(itemNo),
      _prU(prU),
      _fu(fu),
      _components(components),
      _negativeApplication(negativeApplication),
      _subCatNumber(subCatNumber),
      _diagnostic(diagnostic)
  {
  }

  virtual ~CombinationsSubCat() = default;

protected:
  void displayDiagError()
  {
    DiagCollectorGuard dcg(_diag);

    _diag.enable(_diagnostic);
    if (_diag.isActive())
      _diag << " NOT FOUND " << _subCatNumber << " - ITEM NO " << _itemNo << " - "
            << _fu.paxTypeFare()->createFareBasis(&_trx, false) << " FARE" << std::endl;
  }

  void displayDiagItem()
  {
    _diag << std::endl << "  VENDOR " << _vendor << " - " << _subCatNumber << " ITEM NO " << _itemNo
          << " - FARE " << _fu.paxTypeFare()->createFareBasis(&_trx, false) << std::endl;
  }

  static log4cxx::LoggerPtr _logger;

  PricingTrx& _trx;
  DiagCollector& _diag;
  const VendorCode& _vendor;
  const uint32_t _itemNo;
  const PricingUnit& _prU;
  const FareUsage& _fu;
  Combinations::ValidationFareComponents& _components;
  bool& _negativeApplication;
  char _subCatNumber;
  DiagnosticTypes _diagnostic;
};
}

