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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;

//---------------------------------------------------------------------------
// Tax special process 37 (Low Premium) & 38 (High Premium) for Great Britian
//---------------------------------------------------------------------------

class TaxSP36 : public Tax
{
  friend class TaxSP36Test;

public:
  static constexpr char TAX_EXCLUDE = 'Y';
  static constexpr char YES = 'Y';
  static constexpr char BLANK = ' ';
  static const std::string EVERY_CLASSES;

  bool validateCabin(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  bool reverseCabinCnxCheck(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TravelSeg*>::const_iterator travelSegI);

  bool forwardCabinCnxCheck(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TravelSeg*>::const_iterator travelSegI);

  bool taxChargeOncePerDirection(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t segmentOrder);

private:
  bool validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex);

  uint16_t findTaxStopOverIndex(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t startIndex);

  bool stopOverTaxNation(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex);

  BookingCode _bkgCodeRebook;
};
} /* end tse namespace */
