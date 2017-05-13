//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/CurrencyConversionRequest.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleItem.h"


#include <vector>

namespace tse
{
namespace
{
using ConversionType = CurrencyConversionRequest::ApplicationType;
}

class PaxTypeFare;
class PricingTrx;
class Fare;
class FareMarket;
class FarePath;
class PricingTrx;
class PricingUnit;

class Penalties : public RuleApplicationBase
{
public:
  Penalties();

  virtual ~Penalties() {};

  /**
  *   @method validate
  *
  *   Description: Performs rule validations on a FareMarket.
  *
  *   @param PricingTrx           - Pricing transaction
  *   @param Itin                 - itinerary
  *   @param PaxTypeFare          - reference to Pax Type Fare
  *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
  *   @param FareMarket          -  Fare Market
  *
  *   @return Record3ReturnTypes - possible values are:
  *                                NOT_PROCESSED = 1
  *                                FAIL          = 2
  *                                PASS          = 3
  *                                SKIP          = 4
  *                                STOP          = 5
  *
  */
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& paxTypeFare,
                                      const RuleItemInfo* rule,
                                      const FareMarket& fareMarket) override;

  /**
  *   @method validate
  *
  *   Description: Performs rule validations on a PricingUnit.
  *
  *   @param PricingTrx           - Pricing transaction
  *   @param RuleItemInfo         - Record 2 Rule Item Segment Info
  *   @param FarePath             - Fare Path
  *   @param PricingUnit          - Pricing unit
  *   @param FareUsage            - Fare Usage
  *
  *   @return Record3ReturnTypes  - possible values are:
  *                                 NOT_PROCESSED = 1
  *                                 FAIL          = 2
  *                                 PASS          = 3
  *                                 SKIP          = 4
  *                                 STOP          = 5
  */
  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      const RuleItemInfo* rule,
                                      const FarePath& farePath,
                                      const PricingUnit& pricingUnit,
                                      const FareUsage& fareUsage) override;

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      const RuleItemInfo* rule,
                                      const PricingUnit& pricingUnit,
                                      const FareUsage& fareUsage);

  /**
  *   @method isThereChangePenalty
  *
  *   Description: check Penalty Rule to see if there is change penalty,
  *                which will effect NVA/NVB dates
  *
  *   @return bool                - true, there is change penalty
  *                                 else false
  */
  bool isThereChangePenalty(const PenaltyInfo& penaltyInfo) const;

  virtual Record3ReturnTypes
  validateOptions(const PenaltyInfo* penaltiesRule, DiagManager& diag) const;

  static Money getPenaltyAmount(PricingTrx& trx,
                                const PaxTypeFare& ptf,
                                const PenaltyInfo& penaltiesRule,
                                const CurrencyCode& paymentCurrency,
                                const FareUsage* fareUsage = nullptr,
                                const ConversionType type = ConversionType::OTHER);

  static constexpr Indicator RESERVATIONS_CANNOT_BE_CHANGED = 'N';
  static constexpr Indicator TICKET_NON_REFUNDABLE = 'X';
  static constexpr Indicator TICKETNRF_AND_RESNOCHANGE = 'B';
  static constexpr Indicator BLANK = ' ';
  static constexpr Indicator APPLIES = 'X';

private:
  bool nonFlexFareValidationNeeded(PricingTrx& trx) const;
  static MoneyAmount getPercentagePenaltyFromFU(const FareUsage& fu, Percent percent,
                                                const CurrencyCode& paymentCurrency,
                                                PricingTrx& trx,
                                                const ConversionType conversionType);
};

} // namespace tse

