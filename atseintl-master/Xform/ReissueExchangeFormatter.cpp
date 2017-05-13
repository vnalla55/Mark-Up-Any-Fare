// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TrxUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RefundPenalty.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Currency.h"
#include "FareCalc/CalcTotals.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/ReissueExchangeFormatter.h"
#include "Xform/ReissueExchangeModel.h"
#include "Xform/XformUtil.h"
#include "Xform/ComparablePenaltyFee.h"
#include "Xform/PreviousTaxInformationFormatter.h"

#include <boost/foreach.hpp>

namespace tse
{
using PricingUnits = std::vector<PricingUnit*>;

namespace
{
Logger
logger("atseintl.Xform.PricingResponseFormatter");
}

void
ReissueExchangeFormatter::formatReissueExchange(CalcTotals& calcTotals)
{
  RexPricingTrx* rex = dynamic_cast<RexPricingTrx*>(&_pricingTrx);
  if (rex)
  {
    _construct.openElement(xml2::ReissueExchange);

    Indicator resPenIndicator = calcTotals.farePath->residualPenaltyIndicator(*rex);
    if (resPenIndicator != ' ')
      _construct.addAttributeChar(xml2::ResidualIndicator, resPenIndicator);

    const ProcessTagPermutation& winnerPerm = *calcTotals.farePath->lowestFee31Perm();

    _construct.addAttributeChar(xml2::FormOfRefundIndicator, winnerPerm.formOfRefundInd());

    const char isTag7Permutation = winnerPerm.hasTag7only() ? 'T' : 'F';
    _construct.addAttributeChar(xml2::Tag7PricingSolution, isTag7Permutation);

    electronicTicketInd(winnerPerm.electronicTicket());

    prepareChangeFee(*rex, calcTotals);

    // Add <TBD.../><TBD .../> taxes on change fee(s)
    prepareTaxesOnChangeFee(calcTotals);

    // close <REX>
    _construct.closeElement();
  }
  else if (_pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX)
  {
    const RefundPricingTrx& trx = static_cast<RefundPricingTrx&>(_pricingTrx);
    formatReissueExchange(*ReissueExchangeModel::create(trx, calcTotals).get());
  }
}

void
ReissueExchangeFormatter::formatReissueExchange(const ReissueExchangeModel& model)
{
  _construct.openElement(xml2::ReissueExchange);
  _construct.addAttributeChar(xml2::FormOfRefundIndicator, model.getFormOfRefund());
  _construct.addAttributeBoolean(xml2::ReissueTaxRefundable, model.getReissueTaxRefundable());
  _construct.addAttributeBoolean(xml2::NonRefundable, !model.isRefundable());

  if (model.isRefundable())
  {
    addChangeFeesForRefund(model.getWinnerPermutation());
    formatTaxesOnChangeFee(model);
  }

  _construct.closeElement();
}

void
ReissueExchangeFormatter::formatTaxesOnChangeFee(const ReissueExchangeModel& model)
{
  PreviousTaxInformationFormatter taxFormatter(_construct);

  for(const auto& taxOnChangeFee : model.getTaxesOnChangeFee())
    taxFormatter.formatTBD(*taxOnChangeFee);
}

void
ReissueExchangeFormatter::prepareTaxesOnChangeFee(CalcTotals& calcTotals)
{
  if (!_pricingTrx.getRequest())
    return;

  for (const TaxItem* taxItemPtr : calcTotals.changeFeeTaxItems())
  {
    const TaxItem& taxItem = *taxItemPtr;

    if (taxItem.failCode())
      continue;

    MoneyAmount amountPublished = taxItem.taxAmt();

    Money target(taxItem.taxCur());
    CurrencyNoDec amountPublishedNoDec = target.noDec(_pricingTrx.ticketingDate());
    if (taxItem.taxType() == Tax::PERCENTAGE)
    {
      Money source(taxItem.taxAmount(), taxItem.paymentCurrency());
      TaxUtil::convertTaxCurrency(_pricingTrx, source, target);

      amountPublished = target.value();
    }

    _construct.openElement(xml2::TaxBreakdown);
    _construct.addAttribute(xml2::ATaxCode, taxItem.taxCode());
    _construct.addAttributeDouble(
        xml2::TaxAmount, taxItem.taxAmount(), taxItem.paymentCurrencyNoDec());
    _construct.addAttribute(xml2::PublishedCurrency, taxItem.taxCur());
    _construct.addAttributeDouble(xml2::AmountPublished, amountPublished, amountPublishedNoDec);
    _construct.addAttribute(xml2::StationCode, taxItem.taxLocalBoard());
    _construct.addAttribute(xml2::TaxCurrencyCode, taxItem.paymentCurrency());
    _construct.addAttributeChar(xml2::TbdTaxType, taxItem.taxType());
    _construct.addAttribute(xml2::TaxCountryCode, taxItem.nation());
    _construct.addAttribute(xml2::ATaxDescription, taxItem.taxDescription());
    _construct.addAttribute(xml2::MinMaxTaxCurrency, taxItem.minMaxTaxCurrency());
    _construct.addAttributeDouble(xml2::TaxRateUsed, taxItem.taxAmt(), taxItem.taxNodec());

    _construct.closeElement();
  }

  for (const TaxItem* taxItem : calcTotals.changeFeeTaxItems())
  {
    if ((taxItem->failCode() != TaxItem::EXEMPT_ALL_TAXES) &&
        (taxItem->failCode() != TaxItem::EXEMPT_SPECIFIED_TAXES))
      continue;

    _construct.openElement(xml2::TaxExempt);
    _construct.addAttribute(xml2::ATaxCode, taxItem->taxCode());
    _construct.addAttributeDouble(
        xml2::TaxAmount, taxItem->taxExemptAmount(), taxItem->paymentCurrencyNoDec());
    _construct.addAttribute(xml2::TaxAirlineCode,
                            MCPCarrierUtil::swapToPseudo(&_pricingTrx, taxItem->carrierCode()));
    _construct.addAttribute(xml2::StationCode, taxItem->taxLocalBoard());
    _construct.addAttribute(xml2::TaxCurrencyCode, taxItem->paymentCurrency());
    _construct.addAttributeChar(xml2::TbdTaxType, taxItem->taxType());
    _construct.addAttribute(xml2::MinMaxTaxCurrency, taxItem->minMaxTaxCurrency());
    _construct.addAttributeDouble(xml2::TaxRateUsed, taxItem->taxAmt(), taxItem->taxNodec());

    _construct.closeElement();
  }
}

void
ReissueExchangeFormatter::electronicTicketInd(const Indicator& electronicTicketIndicator)
{
  if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_BLANK)
  {
    _construct.addAttributeChar(xml2::ElectronicTicketRequired, 'F');
    _construct.addAttributeChar(xml2::ElectronicTicketNotAllowed, 'F');
  }
  else if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED)
  {
    _construct.addAttributeChar(xml2::ElectronicTicketRequired, 'T');
    _construct.addAttributeChar(xml2::ElectronicTicketNotAllowed, 'F');
  }
  else if (electronicTicketIndicator == ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED)
  {
    _construct.addAttributeChar(xml2::ElectronicTicketRequired, 'F');
    _construct.addAttributeChar(xml2::ElectronicTicketNotAllowed, 'T');
  }
}

void
ReissueExchangeFormatter::prepareChangeFee(RexPricingTrx& trx, CalcTotals& calcTotals)
{
  if (trx.newItin().size())
  {
    // Final Pricing Solution's FarePath
    Itin& itin(*trx.newItin()[0]);
    if (itin.farePath().size())
    {
      FarePath& farePath(*itin.farePath()[0]);

      if (farePath.lowestFee31Perm())
      {
        const ProcessTagPermutation* leastExpensivePermutation = farePath.lowestFee31Perm();

        RexPricingTrx::WaivedChangeRecord3Set& waived(trx.waivedChangeFeeRecord3());
        const RexPricingTrx::WaivedChangeRecord3Set::const_iterator NOT_WAIVED(waived.end());
        // Rated fees orders the fee for identifying the most expensive penalty.
        std::multiset<ComparablePenaltyFee> rated;

        std::vector<ProcessTagInfo*>::const_iterator pti =
            leastExpensivePermutation->processTags().begin();
        std::vector<ProcessTagInfo*>::const_iterator pte =
            leastExpensivePermutation->processTags().end();

        bool waiverCode(false);

        ReissueCharges* reissueCharges(farePath.reissueCharges());
        if (farePath.ignoreReissueCharges() || (reissueCharges && reissueCharges->changeFee == 0.0))
        {
          for (; pti != pte; pti++)
          {
            ProcessTagInfo* info(*pti);
            if (waived.find(info->record3()->orig()) != NOT_WAIVED)
            {
              waiverCode = true;
              break;
            }
          }

          PenaltyFee pf;
          pf.penaltyAmount = 0.0;
          pf.penaltyCurrency = (reissueCharges != nullptr ? reissueCharges->changeFeeCurrency
                                                          : farePath.baseFareCurrency());
          ComparablePenaltyFee fee(pf, trx);
          if (waiverCode)
            fee.waived = true;
          else
            fee.notApplicable = true;
          rated.insert(fee);
        }

        if (!farePath.ignoreReissueCharges() && reissueCharges)
        {
          if (reissueCharges->minAmtApplied)
          {
            PenaltyFee pf;
            pf.penaltyAmount = reissueCharges->changeFee;
            pf.penaltyCurrency = reissueCharges->changeFeeCurrency;
            ComparablePenaltyFee fee(pf, trx);
            rated.insert(fee);
          }
          else
          {
            std::map<const PaxTypeFare*, PenaltyFee*>& penaltyFees(reissueCharges->penaltyFees);
            std::map<const PaxTypeFare*, PenaltyFee*>::iterator pfi(penaltyFees.begin());
            std::map<const PaxTypeFare*, PenaltyFee*>::iterator pfe(penaltyFees.end());
            for (; pfi != pfe; pfi++)
            {
              ComparablePenaltyFee fee(*pfi->second, trx);
              rated.insert(fee);
            }
          }
        }

        std::multiset<ComparablePenaltyFee>::reverse_iterator highestChange(rated.rbegin());
        if (highestChange != rated.rend())
          highestChange->highest = true;

        BOOST_REVERSE_FOREACH (const ComparablePenaltyFee& fee, rated)
          formatChangeFee(ChangeFeeModel(fee, trx));
      }
    }
  }
}

void
ReissueExchangeFormatter::addChangeFeesForRefund(const RefundPermutation& winnerPerm)
{
  if (winnerPerm.minimumPenalty().value() > EPSILON)
  {
    formatChangeFee(NonZeroFeeChangeFeeModel(winnerPerm, true, false, _pricingTrx));
  }
  else if (winnerPerm.totalPenalty().value() < EPSILON)
  {
    formatChangeFee(ZeroFeeChangeFeeModel(winnerPerm, _pricingTrx));
  }
  else
  {
    for (const auto& pF : winnerPerm.penaltyFees())
      for (const auto& f : pF.second->fee())
        formatChangeFee(NonZeroFeeChangeFeeModel(f, _pricingTrx));
  }
}

void
ReissueExchangeFormatter::formatChangeFee(const AbstractChangeFeeModel& model)
{
  if (model.isSkipped())
    return;

  _construct.openElement(xml2::ChangeFee);
  _construct.addAttributeDouble(xml2::ChangeFeeAmount, model.getAmount(), model.getPrecision());

  if (model.isZeroFee())
  {
    _construct.addAttributeBoolean(xml2::ChangeFeeNotApplicable, model.isChangeFeeNotApplicable());
    _construct.addAttributeBoolean(xml2::ChangeFeeWaived, model.isChangeFeeWaived());
  }
  else if (model.isNonZeroFee())
  {
    _construct.addAttribute(xml2::ChangeFeeCurrency, model.getChangeFeeCurrency());
  }
  else
  {
    _construct.addAttributeBoolean(xml2::ChangeFeeNotApplicable, model.isChangeFeeNotApplicable());
    _construct.addAttributeBoolean(xml2::ChangeFeeWaived, model.isChangeFeeWaived());
    _construct.addAttribute(xml2::ChangeFeeCurrency, model.getChangeFeeCurrency());
  }

  _construct.addAttributeBoolean(xml2::HighestChangeFeeIndicator,
                                 model.isHighestChangeFeeIndicator());

  if (model.isZeroFee() || model.isNonZeroFee())
  {
    _construct.addAttributeBoolean(xml2::NonRefundableIndicator, !model.isRefundable());
  }

  if (_pricingTrx.excTrxType() == PricingTrx::AF_EXC_TRX &&
      TrxUtil::isAutomatedRefundCat33Enabled(_pricingTrx))
  {
    _construct.addAttributeDouble(xml2::ChangeFeeAmountInPaymentCurrency,
                                  model.getAmountInPaymentCurrency(),
                                  model.getPaymentPrecision());
    _construct.addAttribute(xml2::ChangeFeeAgentCurrency, model.getPaymentCurrency());
  }

  _construct.closeElement();
}

} // end of tse namespace
