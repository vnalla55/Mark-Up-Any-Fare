//---------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#ifndef TAX_RC_H
#define TAX_RC_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 27 for Canadian Harmonized Sales Tax
//---------------------------------------------------------------------------

class TaxRC : public Tax
{

public:
  static const std::string TAX_CODE_CA1;
  static const std::string TAX_CODE_SQ2;

  TaxRC();
  virtual ~TaxRC();

  const bool& canadianPt() const { return _canadianPt; }
  const bool& origCAMaritime() const { return _origCAMaritime; }
  const bool& origCANotMaritime() const { return _origCANotMaritime; }
  const bool& usPoint() const { return _usPoint; }
  const bool& otherIntl() const { return _otherIntl; }
  const bool& zeroesBaseFare() const { return _zeroesBaseFare; }

  //---------------------------------------------------------------------------
  // Override of Tax Method as RC can applies as listed airports in PNR
  //---------------------------------------------------------------------------

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

  //---------------------------------------------------------------------------
  // Override of Tax Method to handle Special Check for RC Taxes
  //---------------------------------------------------------------------------

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  //---------------------------------------------------------------------------
  // Override of Tax Method to ignore General Processing - exception tax-on-tax
  //---------------------------------------------------------------------------

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

protected:
  bool _canadianPt;
  bool _origCAMaritime;
  bool _origCANotMaritime;
  bool _usPoint;
  bool _otherIntl;
  bool _zeroesBaseFare;

private:

  TaxRC(const TaxRC& tax);
  TaxRC& operator=(const TaxRC& tax);
};

} /* end tse namespace */

#endif /* TAX_RC_H */
