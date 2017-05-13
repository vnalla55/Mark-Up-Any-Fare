//-------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "ServiceFees/OCFees.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AncillaryOptions/AncillaryPriceModifier.h"

#include "boost/optional/optional.hpp"

#include <vector>

namespace tse
{
class AncillaryPricingTrx;
class Itin;
class ServiceFeesGroup;

class AncillaryPriceModifierProcessor
{
public:
  AncillaryPriceModifierProcessor(AncillaryPricingTrx& trx, Itin& itin, OCFees::AmountRounder& amountRounder);

  void processGroups(const std::vector<ServiceFeesGroup*>& serviceFeesGroups);

private:
  void processOcFees(OCFees& ocFees);
  void addModifiedSegment(OCFees& ocFees, const AncillaryIdentifier aid, const AncillaryPriceModifier& ancPriceModifier);
  void applyQuantity(OCFees::OCFeesSeg* ocFeesSeg);
  boost::optional<Money> applyPercentageModification(const Money& feePrice,
                                                     AncillaryPriceModifier::Type type,
                                                     boost::optional<unsigned int> percentage);
  boost::optional<Money> applyMonetaryModification(const Money& feePrice,
                                                   AncillaryPriceModifier::Type type,
                                                   boost::optional<Money> money);
  MoneyAmount roundFeeAmount(const MoneyAmount& feeAmount, const CurrencyCode& feeCurrency);
  MoneyAmount inTargetCurrency(Money money, CurrencyCode targetCurrency);

  AncillaryPricingTrx& _trx;
  Itin& _itin;
  OCFees::AmountRounder& _amountRounder;
};



}
