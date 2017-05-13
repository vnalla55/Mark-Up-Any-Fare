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
#ifndef TAXGB_H
#define TAXGB_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxGB : public Tax
{
public:
  static const std::string GREAT_BRITAIN_CODE;
  static constexpr char TAX_EXCLUDE = 'Y';

  TaxGB();
  virtual ~TaxGB();

  static bool isSpecialEquipment(const EquipmentType& equipment)
  {
    return !equipment.empty() && _specialEquipmentTypes.find(equipment) != std::string::npos;
  }

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

protected:
  bool reverseCabinCnxCheck(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TravelSeg*>::const_iterator travelSegI,
                            bool& isTaxApplicable,
                            bool isTravelWithinUK);

  bool forwardCabinCnxCheck(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            std::vector<TravelSeg*>::const_iterator travelSegI,
                            bool& isTaxApplicable,
                            bool isTravelWithinUK);

  bool validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex);

  void getRebookBkg(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegIndex);

  virtual bool
  isStopOver(const TravelSeg* pTvlSeg, const TravelSeg* pTvlSegPrev, bool isTravelWithinUK);

  virtual bool isForcedStopOver(const TravelSeg*, const TravelSeg* pTvlSegPrev);

  BookingCode _bkgCodeRebook;

  uint16_t _nLowCabinTaxCodeSPN;
  uint16_t _nHighCabinTaxCodeSPN;

  static const std::string _specialEquipmentTypes;

private:
  TaxGB(const TaxGB& map);
  TaxGB& operator=(const TaxGB& map);
};

} /* end tse namespace */

#endif /* TAXGB_H */
