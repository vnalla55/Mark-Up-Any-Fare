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
#ifndef TAX_DL_01_H
#define TAX_DL_01_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"


namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;
class FareUsage;
class Logger;

//---------------------------------------------------------------------------
// Tax special process 21 for Argentina
//---------------------------------------------------------------------------

class TaxDL01 : public Tax
{

public:
  TaxDL01();
  virtual ~TaxDL01();

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  const FareUsage* fareUsage() const { return _fareUsage; }

private:
  static Logger _logger;

  const LocCode USH;
  const LocCode RGA;

  //-----------------------------------------------------------------------------
  // Find and Set the Fare Usage pointer
  //-----------------------------------------------------------------------------

  bool locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg);
  MoneyAmount convertAmoutToPaymentCurrency(PricingTrx& trx, MoneyAmount moneyAmount,
                                            CurrencyCode currencyCode);

  FareUsage* _fareUsage;
  std::map<uint32_t, MoneyAmount> _tvlSegToFareAmount;
  bool _fdRequest;

  TaxDL01(const TaxDL01& map);
  TaxDL01& operator=(const TaxDL01& map);
};

} /* end tse namespace */

#endif /* TAX_DL_H */
