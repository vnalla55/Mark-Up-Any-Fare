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
#ifndef TAX_XG_H
#define TAX_XG_H

#include "Common/TseCodeTypes.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 27 QST on AIF In Quebec Province
//---------------------------------------------------------------------------

class TaxXG : public Tax
{

public:
  static const std::string TAX_CODE_XG;
  static const std::string TAX_CODE_XG1;
  static const std::string TAX_CODE_XG3;
  static const std::string TAX_CODE_XG4;
  static const std::string TAX_CODE_CA1;
  static const std::string TAX_CODE_SQ;
  static const std::string TAX_CODE_SQ1;
  static const std::string TAX_CODE_SQ3;

  TaxXG();
  virtual ~TaxXG();

  const bool& canadianPt() const { return _canadianPt; }
  const bool& usPoint() const { return _usPoint; }
  const bool& mexicanPt() const { return _mexicanPt; }
  const bool& otherIntl() const { return _otherIntl; }
  const bool& zeroesBaseFare() const { return _zeroesBaseFare; }

  const Loc* pointOfSaleLocation() const { return _pointOfSaleLocation; }

  //---------------------------------------------------------------------------
  // Override of Tax Method to handle Special Check for XG Taxes
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
  //---------------------------------------------------------------------------
  // Override of Tax Method to handle Special Check for XG Taxes
  //---------------------------------------------------------------------------

  virtual bool validateFromTo(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex);

  //---------------------------------------------------------------------------
  // Override of Tax Method to handle Special Check for XG/XG3/XG4 Taxes
  //---------------------------------------------------------------------------

  virtual bool validateXG(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  //---------------------------------------------------------------------------
  // Override of Tax Method to handle Special Check for XG1 Tax
  //---------------------------------------------------------------------------

  virtual bool validateXG1(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool _canadianPt;
  bool _whollyCA;
  bool _origCAMaritime;
  bool _origCANotMaritime;
  bool _origUS;
  bool _usPoint;
  bool _hawaiianPt;
  bool _mexicanPt;
  bool _otherIntl;
  bool _zeroesBaseFare;

  const Loc* _pointOfSaleLocation;

  TaxXG(const TaxXG& tax);
  TaxXG& operator=(const TaxXG& tax);
};

} /* end tse namespace */

#endif /* TAX_XG_H */
