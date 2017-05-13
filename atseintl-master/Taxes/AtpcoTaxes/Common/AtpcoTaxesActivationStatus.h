// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
class AtpcoTaxesActivationStatus
{
public:
  AtpcoTaxesActivationStatus() = default;
  ~AtpcoTaxesActivationStatus() = default;

  void setTaxOnOC(bool isEnabled) { _taxOnOC = isEnabled; }
  bool isTaxOnOC() const { return _taxOnOC; }

  void setTaxOnBaggage(bool isEnabled) { _taxOnBaggage = isEnabled; }
  bool isTaxOnBaggage() const { return _taxOnBaggage; }

  bool isTaxOnOCBaggage() const { return _taxOnOC || _taxOnBaggage; }

  void setTaxOnChangeFee(bool isEnabled) { _taxOnChangeFee = isEnabled; }
  bool isTaxOnChangeFee() const { return _taxOnChangeFee; }

  void setTaxOnItinYqYrTaxOnTax(bool isEnabled) { _taxOnItinYqYrTaxOnTax = isEnabled; }
  bool isTaxOnItinYqYrTaxOnTax() const { return _taxOnItinYqYrTaxOnTax; }

  void setAllEnabled();
  bool isAllEnabled() const { return isTaxOnOCBaggage() && isTaxOnChangeFee() && isTaxOnItinYqYrTaxOnTax(); }

  bool isAnyEnabled() const { return isTaxOnOCBaggage() || isTaxOnChangeFee() || isTaxOnItinYqYrTaxOnTax(); }

  void setOldTaxesCalculated(bool isEnabled) { _isOldTaxesCalculated = isEnabled; }
  bool isOldTaxesCalculated() const { return _isOldTaxesCalculated; }

private:
  bool _taxOnOC = false;
  bool _taxOnBaggage = false;
  bool _taxOnChangeFee = false;
  bool _taxOnItinYqYrTaxOnTax = false;
  bool _isOldTaxesCalculated = false;
};
}
