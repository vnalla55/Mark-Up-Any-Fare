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
#ifndef CABINVALIDATOR_H
#define CABINVALIDATOR_H

#include "Common/TseCodeTypes.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;
class TaxCodeCabin;
class AirSeg;

class CabinValidator
{

public:
  CabinValidator();
  virtual ~CabinValidator();

  bool validateCabinRestriction(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                const TaxCodeReg& taxCodeReg,
                                TravelSeg* travelSeg);

private:
  const TaxCodeCabin* getMatchingTaxCodeCabin(const AirSeg& airSeg,
                                              const TaxCodeReg& taxCodeReg,
                                              const PricingTrx& trx,
                                              const TaxResponse& taxResponse);

  bool validateFlightandDirection(const PricingTrx& trx,
                                  const TaxResponse& taxResponse,
                                  const TaxCodeReg& taxCodeReg,
                                  const AirSeg& airSeg,
                                  const TaxCodeCabin& taxCodeCabin) const;

  BookingCode _bkgCodeRebook;

  CabinValidator(const CabinValidator& map) = delete;
  CabinValidator& operator=(const CabinValidator& map) = delete;
};

} /* end tse namespace */

#endif /* CABINVALIDATOR_H */
