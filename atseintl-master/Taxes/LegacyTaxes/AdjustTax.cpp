//----------------------------------------------------------------------------
//
//  File:         AdjustTax.cpp
//  Description:  Tax Validator Class for ATSE International Project
//  Created:      3/24/2004
//  Authors:      Sommapan Lathitham
//
//  Description: This routine will adjust the taxes for minimum tax,
//               maximum tax, any plusup amount and also ticket
//               booklet.
//
//  Updates:
//          3/25/2004 - SL - Create AdjustTax for ATSE International.
//
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

#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Money.h"
#include "Common/TaxRound.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

using namespace tse;


// ----------------------------------------------------------------------------
//
// void AdjustTax::applyAdjustTax
//
// Description:  We now are working with PAYMENT CURRENCY, we check for any plusup tax
//               amount apply, ticket booklet, minimum tax or maximum tax.
//
// ----------------------------------------------------------------------------

MoneyAmount
AdjustTax::applyAdjust(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       const MoneyAmount& taxAmount,
                       const CurrencyCode& paymentCurrency,
                       TaxCodeReg& taxCodeReg,
                       CalculationDetails& details)
{
  CurrencyConversionFacade ccFacade;
  bool rc;

  MoneyAmount adjustAmt = taxAmount; // This class will pass back the amount

  //----------- Check PlusUP,  --------------------------

  if (UNLIKELY((taxCodeReg.plusupAmt() > 0) && (adjustAmt != 0))) // Plus Up tax if tax not zero
  {
    if (paymentCurrency != taxCodeReg.taxCur())
    {
      Money sourceMoney(taxCodeReg.plusupAmt(), taxCodeReg.taxCur());
      Money targetMoney(0, paymentCurrency);

      rc = ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES);

      if (rc)
      {
        adjustAmt += targetMoney.value(); // Add the plus up amount
      }
      else
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic814);
      }
    }
    else
    {
      adjustAmt += taxCodeReg.plusupAmt(); // Add the plus up amount
    }
  } // end of Plus Up
  details.plusedUpAmount = adjustAmt;

  //-----------------  //Apply Once Per Ticket Booklet -----------------

  if (UNLIKELY((taxCodeReg.occurrence() == TICKET_BOOKLET) && (adjustAmt != 0)))
  {
    uint16_t numTvlSeg = taxResponse.farePath()->itin()->travelSeg().size();

    if (NUM_COUPON <= 0) // lint !e506
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::NUM_COUPON_BOOKLET, Diagnostic814);
    }
    else
    {
      uint16_t itinBooklet = (numTvlSeg / NUM_COUPON);

      if (numTvlSeg % NUM_COUPON != 0)
      {
        ++itinBooklet;
      }

      adjustAmt = (itinBooklet * adjustAmt);

      if (paymentCurrency != taxCodeReg.taxCur())
      {
        Money sourceMoney(taxCodeReg.taxAmt(), taxCodeReg.taxCur());
        Money targetMoney(0, paymentCurrency);

        rc = ccFacade.convert(
            targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES);

        if (rc)
        {
          adjustAmt = targetMoney.value(); // charge per booklet.
        }
        else
        {
          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic814);
        }
      }
    }
  }
  details.bookletAdjustedAmount = adjustAmt;

  //--------------------------------  //Minimum tax  -------------------
  if (taxCodeReg.minTax() != 0)
  {
    if (paymentCurrency != taxCodeReg.taxCur())
    {
      Money sourceMoney(taxCodeReg.minTax(), taxCodeReg.taxCur());
      Money targetMoney(0, paymentCurrency);

      rc = ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES);

      if (rc)
      {
        RoundingFactor roundingUnit = taxCodeReg.taxcdRoundUnit();
        RoundingRule roundingRule = NONE;

        TaxRound taxRound;

        MoneyAmount minTaxAmount = taxRound.applyTaxRound(
            targetMoney.value(), paymentCurrency, roundingUnit, roundingRule);

        if (minTaxAmount == 0)
        {
          minTaxAmount = targetMoney.value();
        }

        if (minTaxAmount > adjustAmt) // If min tax is greater than tax
        {
          adjustAmt = minTaxAmount; // charge the minumum.
        }
      }
      else
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic814);
      }
    }
    else
    {
      if (taxCodeReg.minTax() > adjustAmt)
      {
        adjustAmt = taxCodeReg.minTax();
      }
    }
  } // end of minTax
  details.minTaxAdjustedAmount = adjustAmt;

  //--------------------------------  //Maximum tax  -------------------
  if (taxCodeReg.maxTax() != 0 && taxCodeReg.taxType() == PERCENTAGE) // Maximum tax
  {
    if (paymentCurrency != taxCodeReg.taxCur())
    {
      Money sourceMoney(taxCodeReg.maxTax(), taxCodeReg.taxCur());
      Money targetMoney(0, paymentCurrency);

      rc = ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES);

      if (rc)
      {
        RoundingFactor roundingUnit = taxCodeReg.taxcdRoundUnit();
        RoundingRule roundingRule = NONE;

        TaxRound taxRound;

        MoneyAmount maxTaxAmount = taxRound.applyTaxRound(
            targetMoney.value(), paymentCurrency, roundingUnit, roundingRule);

        if (maxTaxAmount == 0)
        {
          maxTaxAmount = targetMoney.value();
        }

        if (maxTaxAmount < adjustAmt) // If max tax is less than tax,
        {
          adjustAmt = maxTaxAmount; // charge the maximum
        }
      }
      else
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic814);
      }
    }
    else
    {
      if (taxCodeReg.maxTax() < adjustAmt)
      {
        adjustAmt = taxCodeReg.maxTax();
      }
    }
  } // end of Max Tax.
  details.adjustedTaxAmount = adjustAmt;

  return adjustAmt; // return taxAmount to Tax.cpp (adjustTax.applyAdjust)
}
// AdjustTax::AdjustTax
