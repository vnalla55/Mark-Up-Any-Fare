// ---------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "Common/TseConsts.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

namespace tse
{
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TaxRestrictionLocationInfo;

// ----------------------------------------------------------------------------
// TaxApply will call all the TaxValidator functions for all special taxes
// ----------------------------------------------------------------------------

class TaxCodeValidator
{
  friend class TaxCodeValidatorTest;
  friend class TaxRestrictionLocationTest;

public:
  static constexpr char YES = 'Y';
  static constexpr char NO = 'N';
  static constexpr char BLANK = ' ';

  static constexpr char DOMESTIC = 'D';
  static constexpr char INTERNATIONAL = 'I';

  static constexpr char CIRCLE_TRIP = 'R';
  static constexpr char ONEWAY_TRIP = 'O';

  static constexpr char APPLIED = 'C';
  static constexpr char BOOKLET = 'B';

  static constexpr char PERCENTAGE = 'P';
  static constexpr char FARE = 'F';

  static constexpr char ORIGIN = 'O';
  static constexpr char TERMINATION = 'T';
  static constexpr char WHOLLY_WITHIN = 'W';

  static constexpr char ALL = 'A';
  static constexpr char CASH = 'C';
  static constexpr char CHECK = 'K';
  static constexpr char CARD = 'R';

  static const std::string TAX_CODE_AY;
  static const std::string TAX_CODE_US2;
  static const std::string TAX_CODE_ZP;
  static const std::string PAX_TYPE_CODE_CMP;

  TaxCodeValidator() = default;
  virtual ~TaxCodeValidator() = default;
  TaxCodeValidator(const TaxCodeValidator&) = delete;
  TaxCodeValidator& operator=(const TaxCodeValidator&) = delete;

  bool validateTaxCode(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       const VendorCode& vendorCode = Vendor::SABRE,
                       const ZoneType& zoneType = MANUAL,
                       const LocUtil::ApplicationType& applicationType = LocUtil::TAXES);

  bool validatePartitionId(PricingTrx& trx, TaxCodeReg& taxCodeReg) const;

  bool validateOriginalTicket(PricingTrx& trx, TaxCodeReg& taxCodeReg) const;

  bool validatePointOfSale(PricingTrx& trx,
                           TaxCodeReg& taxCodeReg,
                           const VendorCode& vendorCode,
                           const ZoneType& zoneType,
                           const LocUtil::ApplicationType& applicationType);

  bool validatePointOfIssue(PricingTrx& trx,
                            TaxCodeReg& taxCodeReg,
                            const VendorCode& vendorCode,
                            const ZoneType& zoneType,
                            const LocUtil::ApplicationType& applicationType);

  bool virtual
  validatePassengerRestrictions(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                const PaxType* paxType);

  void setTravelSeg(const std::vector<TravelSeg*>* seg) { _travelSeg=seg; };

protected:
  bool virtual isTaxOnChangeFee(PricingTrx& trx, TaxCodeReg& taxCodeReg) const;
  bool virtual validateEffectiveDate(PricingTrx& trx, TaxCodeReg& taxCodeReg);

  bool virtual validateOriginDate(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg);


  bool validateFreeTktExemption(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateSellCurrencyRestrictions(PricingTrx& trx, TaxCodeReg& taxCodeReg);

  bool
  validateCarrierRestrictions(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  virtual bool validateTravelType(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateTripOwRt(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateOrigin(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      const VendorCode& vendorCode,
                      const ZoneType& zoneType,
                      const LocUtil::ApplicationType& applicationType);

  bool validateFormOfPayment(PricingTrx& trx, TaxCodeReg& taxCodeReg);

  bool validateApplyTaxOnce(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateSegmentFee(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateRestrictionLocation(PricingTrx& trx,
                                   TaxCodeReg& taxCodeReg,
                                   const VendorCode& vendorCode,
                                   const ZoneType& zoneType,
                                   const LocUtil::ApplicationType& applicationType);

  bool checkRestrictionLocation(const TaxRestrictionLocationInfo* location,
                                PricingTrx& trx,
                                const VendorCode& vendorCode,
                                const ZoneType& zoneType,
                                const LocUtil::ApplicationType& applicationType);

  bool
  validateAnciliaryServiceCode(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);

  bool validateJourneyType(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           utc::OverrideJourneyType& journeyType);

  const std::vector<TravelSeg*>& getTravelSeg(const TaxResponse& taxResponse) const;

  const std::vector<TravelSeg*>* _travelSeg = nullptr;
};
}
