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
#ifndef TAX_US2_H
#define TAX_US2_H

#include <vector>
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;
class Loc;

class TaxUS2 : public Tax
{

public:
  static const char* TAX_CODE_US1;

  TaxUS2();
  virtual ~TaxUS2();

  void applyUS2(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  static MoneyAmount calculateHalfTaxAmount(PricingTrx& trx,
                                            TaxResponse& taxResponse,
                                            CurrencyCode& paymentCurrency,
                                            TaxCodeReg& taxCodeReg);

private:
  bool validUS2(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  void processUS2(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  std::vector<TravelSeg*>::const_iterator
  applyIntToInt(PricingTrx& trx,
                TaxResponse& taxResponse,
                TaxCodeReg& taxCodeReg,
                std::vector<TravelSeg*>::const_iterator travelSegI);

  std::vector<TravelSeg*>::const_iterator
  applyIntToUS(PricingTrx& trx,
               TaxResponse& taxResponse,
               TaxCodeReg& taxCodeReg,
               std::vector<TravelSeg*>::const_iterator travelSegI);

  void applyUSToInt(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    std::vector<TravelSeg*>::const_iterator travelSegI);

  std::vector<TravelSeg*>::const_iterator
  applyUSToUS(PricingTrx& trx,
              TaxResponse& taxResponse,
              TaxCodeReg& taxCodeReg,
              std::vector<TravelSeg*>::const_iterator travelSegI,
              bool originUS,
              bool boarUS);

  void internationalFromUS(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  void internationalToUS(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  void createUS2(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 TravelSeg& travelSegFrom,
                 TravelSeg& travelSegTo);

  bool isMostDistantUS(PricingTrx& trx, TaxResponse& taxResponse);

  bool _bufferZone;
  bool _stopOverUS;
  bool _furthestPointUS;
  bool _tripOriginUS;
  bool _terminateUS;
  bool _validUSPoint;
  bool _validInternationalPoint;
  bool _validCanadaPoint;
  bool _validMexicoPoint;
  bool _validAKHIPoint;
  bool _mostDistantUSInitialized;
  bool _mostDistantUS;
  const Loc* _pointOfSaleLocation;

  TaxUS2(const TaxUS2& tax);
  TaxUS2& operator=(const TaxUS2& tax);

  static Logger _logger;
};

} /* end tse namespace */

#endif /* TAX_US2_H */
