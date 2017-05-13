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
#ifndef TAX_KH1_H
#define TAX_KH1_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;
class FareUsage;
class PaxTypeFare;

//---------------------------------------------------------------------------
// Tax special process 21 for Argentina
//---------------------------------------------------------------------------

class TaxKH1 : public Tax
{

public:
  TaxKH1();
  virtual ~TaxKH1();

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

  const FareUsage* fareUsage() const { return _fareUsage; }

private:
  static log4cxx::LoggerPtr _logger;

  //-----------------------------------------------------------------------------
  // Find and Set the Fare Usage pointer
  //-----------------------------------------------------------------------------

  bool locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg);
  bool isValidForValidatingCarrier(const PricingTrx& trx,
                                   const PaxTypeFare& ptf,
                                   const CarrierCode& vcxr) const;

  FareUsage* _fareUsage;

  TaxKH1(const TaxKH1& map);
  TaxKH1& operator=(const TaxKH1& map);
};

} /* end tse namespace */

#endif /* TAX_KH1_H */
