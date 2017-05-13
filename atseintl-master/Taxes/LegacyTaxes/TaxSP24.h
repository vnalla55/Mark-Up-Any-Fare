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
#ifndef TAX_SP24_H
#define TAX_SP24_H

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

//---------------------------------------------------------------------------
// Tax special process 21 for Argentina
//---------------------------------------------------------------------------

class TaxSP24 : public Tax
{

public:
  TaxSP24();
  virtual ~TaxSP24();

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

  const FareUsage* fareUsage() const { return _fareUsage; }

private:
  static log4cxx::LoggerPtr _logger;

  //-----------------------------------------------------------------------------
  // Find and Set the Fare Usage pointer
  //-----------------------------------------------------------------------------

  bool locateFareUsage(TaxResponse& taxResponse, TravelSeg& travelSeg);

  FareUsage* _fareUsage;

  TaxSP24(const TaxSP24& map);
  TaxSP24& operator=(const TaxSP24& map);
};

} /* end tse namespace */

#endif /* TAX_SP24_H */