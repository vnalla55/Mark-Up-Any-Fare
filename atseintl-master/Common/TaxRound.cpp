// ----------------------------------------------------------------------------
//
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
// ----------------------------------------------------------------------------

#include "Common/TaxRound.h"

#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

#include <cmath>

namespace tse
{

static Logger
logger("atseintl.Taxes.TaxRound");

TaxRound::TaxRound() {}

// ----------------------------------------------------------------------------
// Description:  retrieveNationRoundingSpecifications
// ----------------------------------------------------------------------------

void
TaxRound::retrieveNationRoundingSpecifications(PricingTrx& trx,
                                               RoundingFactor& roundingUnit,
                                               CurrencyNoDec& roundingNoDec,
                                               RoundingRule& roundingRule) const
{
  const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);
  if (UNLIKELY(!pointOfSaleLocation))
  {
    LOG4CXX_ERROR(logger, "Cannot determine point of sale");
    return;
  }

  NationCode nationCode = pointOfSaleLocation->nation();

  if (LIKELY(trx.getOptions() &&
      !trx.getOptions()->currencyOverride().empty() &&
      trx.getOptions()->currencyOverride() != NUC))
  {
    const Currency* currency = nullptr;
    currency = trx.dataHandle().getCurrency( trx.getOptions()->currencyOverride() );

    if (UNLIKELY(!currency))
    {
      LOG4CXX_ERROR(logger,
                    "No Currency Table For Currency " << trx.getOptions()->currencyOverride());
      return;
    }

    const std::vector<Nation*> allNationList =
        trx.dataHandle().getAllNation(trx.getRequest()->ticketingDT());

    std::vector<Nation*>::const_iterator nationsListIter = allNationList.begin();
    std::vector<Nation*>::const_iterator nationsListEnd = allNationList.end();

    if (UNLIKELY(allNationList.empty()))
    {
      LOG4CXX_ERROR(logger, "No Nation Table");
      return;
    }

    for (; nationsListIter != nationsListEnd; nationsListIter++)
    {
      if (currency->controllingEntityDesc() == (*nationsListIter)->description())
      {
        nationCode = (*nationsListIter)->nation();
        break;
      }
    }

    const TaxNation* taxNation =
        trx.dataHandle().getTaxNation(nationCode, trx.getRequest()->ticketingDT());

    if (UNLIKELY(taxNation == nullptr))
    {
      LOG4CXX_WARN(logger, "No TaxNation Table Found For Nation: " << nationCode);
      return;
    }

    std::vector<NationCode>::const_iterator taxNationRoundI = taxNation->taxNationRound().begin();

    for (; taxNationRoundI != taxNation->taxNationRound().end(); taxNationRoundI++)
    {
      if (pointOfSaleLocation->nation() == (*taxNationRoundI))
        return;
    }

    if (currency->taxOverrideRoundingUnit() > 0)
    {
      roundingUnit = currency->taxOverrideRoundingUnit();
      roundingNoDec = currency->taxOverrideRoundingUnitNoDec();
      roundingRule = currency->taxOverrideRoundingRule();
      return;
    }
  }
  retrieveNationRoundingSpecifications(trx, nationCode, roundingUnit, roundingNoDec, roundingRule);
}

// ----------------------------------------------------------------------------
// Description:  retrieveNationRoundingSpecifications
// ----------------------------------------------------------------------------

void
TaxRound::retrieveNationRoundingSpecifications(PricingTrx& trx,
                                               const NationCode& nation,
                                               RoundingFactor& roundingUnit,
                                               CurrencyNoDec& roundingNoDec,
                                               RoundingRule& roundingRule) const
{
  const TaxNation* taxNation =
      trx.dataHandle().getTaxNation(nation, trx.getRequest()->ticketingDT());

  if (UNLIKELY(taxNation == nullptr))
  {
    LOG4CXX_WARN(logger, "No TaxNation Table Found For Nation: " << nation);
    return;
  }

  const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);
  if(pointOfSaleLocation)
  {
    std::vector<NationCode>::const_iterator taxNationRoundI = taxNation->taxNationRound().begin();

    for (; taxNationRoundI != taxNation->taxNationRound().end(); taxNationRoundI++)
    {
      if (pointOfSaleLocation->nation() == (*taxNationRoundI))
        return;
    }
  }

  roundingUnit = taxNation->roundingUnit();
  roundingNoDec = taxNation->roundingUnitNodec();
  roundingRule = taxNation->roundingRule();
}

// ----------------------------------------------------------------------------
// Description:  doSpecialTaxRound
// ----------------------------------------------------------------------------

MoneyAmount
TaxRound::doSpecialTaxRound(PricingTrx& trx,
                            const MoneyAmount& fareAmount,
                            const MoneyAmount& taxAmount,
                            uint_least64_t centNumber /*= 50*/) const
{
  //precision factor, meaning: 1=1cent, 10=0.1cent, 100=0.01cent, etc...
  uint_least16_t resFactor = 1;

  const uint_least16_t modAdjAmountBorder = static_cast<uint_least16_t>(2.5 * resFactor);

  MoneyAmount finalFareAmount = fareAmount;
  // round fare amount to 2 decimal places
  finalFareAmount = roundl(finalFareAmount * 100.0) / 100.0;

  if (finalFareAmount <= 0.07)
    return taxAmount;

  //rollback of RAT-350 - P_101352, 150384-WN-PBE
  //due to WN request, it's simpleset way to block the new functionality
  centNumber = 50;

  centNumber *= resFactor;

  MoneyAmount totalFare = finalFareAmount + taxAmount;

  // Need to be very careful with the double precision arithmetic.
  // This fails on a borderline case of a tax amount of 1 penny.
  MoneyAmount scaledAmt = totalFare * resFactor * 100.0;

  uint_least64_t intAmount = static_cast<uint_least64_t>(scaledAmt + EPSILON);
  uint_least64_t modAdjAmount = intAmount % centNumber;

  // Dealing with just to the penny.
  uint_least64_t scaledTaxAmount = static_cast<uint_least64_t>(taxAmount * resFactor * 100.0);
  uint_least64_t difference = scaledTaxAmount - modAdjAmount;

  MoneyAmount diff = static_cast<MoneyAmount>(difference) / (100 * resFactor);

  if (modAdjAmount <= modAdjAmountBorder)
    return diff;

  uint_least64_t finalAdjAmount = centNumber - modAdjAmount;
  uint_least64_t total = scaledTaxAmount + finalAdjAmount;
  MoneyAmount sum = static_cast<MoneyAmount>(total) / (100 * resFactor);

  if (modAdjAmount > modAdjAmountBorder && finalAdjAmount <= modAdjAmountBorder)
    return sum;

  return 0.0;
}

// ----------------------------------------------------------------------------
// Description:  applyTaxRound
// ----------------------------------------------------------------------------

MoneyAmount
TaxRound::applyTaxRound(const MoneyAmount& taxAmount,
                        const CurrencyCode& paymentCurrency,
                        const RoundingFactor& roundingUnit,
                        RoundingRule roundingRule) const
{
  CurrencyConverter curConverter;
  Money targetMoney(taxAmount, paymentCurrency);

  if (!curConverter.roundByRule(targetMoney, roundingUnit, roundingRule))
  {
    LOG4CXX_WARN(logger, "Currency Converter Failure For Currency: " << paymentCurrency);
    return taxAmount;
  }

  return targetMoney.value();
}
} //tse
