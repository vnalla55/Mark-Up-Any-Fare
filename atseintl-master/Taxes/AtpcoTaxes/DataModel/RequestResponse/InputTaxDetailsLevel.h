// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

namespace tax
{

class InputTaxDetailsLevel
{
  bool _allDetails;
  bool _geoDetails;
  bool _calcDetails;
  bool _taxOnTaxDetails;
  bool _taxOnFaresDetails;
  bool _taxOnYQYRDetails;
  bool _taxOnOptionalServiceDetails;
  bool _taxOnExchangeReissueDetails;
  bool _exchangeReissueDetails;

public:
  InputTaxDetailsLevel()
    : _allDetails()
    , _geoDetails()
    , _calcDetails()
    , _taxOnTaxDetails()
    , _taxOnFaresDetails()
    , _taxOnYQYRDetails()
    , _taxOnOptionalServiceDetails()
    , _taxOnExchangeReissueDetails()
    , _exchangeReissueDetails()
  {
  }

  bool  allDetails() const { return _allDetails; }
  bool& allDetails() { return _allDetails; }

  bool  geoDetails() const { return _geoDetails; }
  bool& geoDetails() { return _geoDetails; }

  bool  calcDetails() const { return _calcDetails; }
  bool& calcDetails() { return _calcDetails; }

  bool  taxOnTaxDetails() const { return _taxOnTaxDetails; }
  bool& taxOnTaxDetails() { return _taxOnTaxDetails; }

  bool  taxOnFaresDetails() const { return _taxOnFaresDetails; }
  bool& taxOnFaresDetails() { return _taxOnFaresDetails; }

  bool  taxOnYQYRDetails() const { return _taxOnYQYRDetails; }
  bool& taxOnYQYRDetails() { return _taxOnYQYRDetails; }

  bool  taxOnOptionalServiceDetails() const { return _taxOnOptionalServiceDetails; }
  bool& taxOnOptionalServiceDetails() { return _taxOnOptionalServiceDetails; }

  bool  taxOnExchangeReissueDetails() const { return _taxOnExchangeReissueDetails; }
  bool& taxOnExchangeReissueDetails() { return _taxOnExchangeReissueDetails; }

  bool  exchangeReissueDetails() const { return _exchangeReissueDetails; }
  bool& exchangeReissueDetails() { return _exchangeReissueDetails; }

};

} // namespace tax

