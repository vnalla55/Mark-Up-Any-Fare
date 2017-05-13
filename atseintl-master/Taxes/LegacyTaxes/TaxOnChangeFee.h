// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#ifndef TAX_ON_CHANGE_FEE_H
#define TAX_ON_CHANGE_FEE_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse

{
class TaxResponse;
class PricingTrx;
class TaxCodeReg;
class TravelSeg;
class Money;
class CurrencyConversionFacade;
class Itin;

class TaxOnChangeFee : public Tax
{

public:
  TaxOnChangeFee() = default;
  virtual ~TaxOnChangeFee() = default;

  bool validateSequence(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& travelSegStartIndex,
                        uint16_t& travelSegEndIndex,
                        bool checkSpn = false) override;

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  bool validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

  void preparePortionOfTravelIndexes(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg) override;

  void doTaxRange(PricingTrx& trx,
                  TaxResponse& taxResponse,
                  uint16_t& startIndex,
                  uint16_t& endIndex,
                  TaxCodeReg& taxCodeReg) override;

  bool validateRange(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t& startIndex,
                     uint16_t& endIndex) override;

  bool validateGeoSpecLoc1(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex) override;

  bool validateTransferTypeLoc1(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t startIndex,
                               uint16_t endIndex) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  bool validateCarrierExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex) override;

  bool validateTaxOnChangeFees(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg) override;

  bool validateBaseTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  bool validateEquipmentExemption(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex) override;

  bool validateFareClass(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex) override;


  bool validateCabin(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex) override;

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  MoneyAmount fareAmountInNUCForCurrentSegment(PricingTrx& trx,
                                               const FarePath* farePath,
                                               const TaxCodeReg& taxCodeReg) const override;

  bool shouldCheckTravelDate() const override;

  TaxOnChangeFee(const TaxOnChangeFee&);
  TaxOnChangeFee& operator=(const TaxOnChangeFee&);

  const std::vector<TravelSeg*>& getTravelSeg(const TaxResponse& taxResponse) const override;
  const Itin* getOldItin() const {return _oldItin;};

private:
  static log4cxx::LoggerPtr _loggerTaxOnChangeFee;

  const Itin* _oldItin = nullptr;
};
}
#endif // TAX_ON_CHANGE_FEE_H
