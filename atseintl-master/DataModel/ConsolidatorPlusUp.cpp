//-------------------------------------------------------------------
//
//  File:        ConsolidatorPlusUp.cpp
//  Created:     July 9, 2007
//  Design:      Marco Cartolano / Adrian Tovar
//  Authors:
//
//  Description: Consolidator plus up methods for Plus Up Pricing
//
//  Updates:
//
//  Copyright Sabre 2005
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

#include "DataModel/ConsolidatorPlusUp.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/NUCCollectionResults.h"
#include "Common/TruePaxType.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
static Logger
logger("atseintl.DataModel.ConsolidatorPlusUp");

const char*
ConsolidatorPlusUp::TAX_CODE_RC("RC");
const char*
ConsolidatorPlusUp::TAX_CODE_XG("XG");
const char*
ConsolidatorPlusUp::TAX_CODE_XQ("XQ");

void
ConsolidatorPlusUp::initialize(PricingTrx& trx,
                               const MoneyAmount& amount,
                               const CurrencyCode& currencyCode,
                               const TktDesignator& tktDesignator)
{
  _amount = amount;
  _currencyCode = currencyCode;
  _tktDesignator = tktDesignator;
  _fareCalcAmount = amount;

  const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);
  if (pointOfSaleLocation && LocUtil::isCanada(*pointOfSaleLocation))
    _canadianPointOfSale = true;
}

void
ConsolidatorPlusUp::addPlusUpToFarePath(PricingTrx& trx, FarePath*& farePath, DiagCollector* diag)
{
  if (!_canadianPointOfSale)
    return;

  const CurrencyCode& calculationCurrency = farePath->calculationCurrency();
  MoneyAmount convertedPlusUpAmt = _amount;
  ExchRate exchangeRate = 0.0;

  if (diag)
  {
    if (!_diagTaxesAdded && !_taxCodes.empty())
    {
      (*diag) << "PU COULD AFFECT THE FOLLOWING TAXES - SEE DIAGNOSTIC 804:" << std::endl;

      diag->setf(std::ios::left, std::ios::adjustfield);

      std::set<TaxCode>::const_iterator itr = _taxCodes.begin();
      std::set<TaxCode>::const_iterator itrEnd = _taxCodes.end();

      for (; itr != itrEnd; itr++)
      {
        (*diag) << std::setw(3) << *itr << " TAX" << std::endl;
      }
      diag->printLine();

      _diagTaxesAdded = true;
    }

    diag->setf(std::ios::left, std::ios::adjustfield);

    TruePaxType tpt(trx, *farePath);

    (*diag) << tpt.paxType();
    (*diag) << "-POS PU AMOUNT: " << std::setw(8) << Money(_amount, _currencyCode) << std::endl;
    (*diag) << "    * FARE CALC CURRENCY  : " << calculationCurrency;
  }

  if (calculationCurrency != _currencyCode)
  {
    // Need to convert the Plus Up amount to the Calculation currency
    convertedPlusUpAmt =
        convertAmount(trx, _amount, _currencyCode, calculationCurrency, exchangeRate);

    if (diag)
    {
      if (exchangeRate == 0.0)
      {
        (*diag) << " *** ROE TO " << calculationCurrency << " NOT AVAILABLE" << std::endl;
      }
      else
      {
        char strExchangeRate[15];
        sprintf(strExchangeRate, "%lf", exchangeRate);
        (*diag) << " *** ROE TO " << calculationCurrency << ": " << strExchangeRate << std::endl;
      }

      (*diag) << "    F/CALC PU AMT: " << std::setw(8)
              << Money(convertedPlusUpAmt, calculationCurrency) << std::endl;
    }
  }
  else
  {
    if (diag)
    {
      (*diag) << " *** NO CONVERSION REQUIRED" << std::endl;
    }
  }

  // Save fare calc amount for later use
  _fareCalcAmount = convertedPlusUpAmt;

  if (diag)
  {
    (*diag) << "    TOTAL AMOUNT : " << std::setw(8)
            << Money(farePath->getTotalNUCAmount(), calculationCurrency) << std::endl;
  }

  farePath->increaseTotalNUCAmount(convertedPlusUpAmt);

  if (diag)
  {
    (*diag) << "    NEW TOTAL AMT: " << std::setw(8)
            << Money(farePath->getTotalNUCAmount(), calculationCurrency) << std::endl;
    diag->printLine();
  }
}

void
ConsolidatorPlusUp::addPlusUpToBaseFare(PricingTrx& trx,
                                        const FarePath& farePath,
                                        const CurrencyCode& baseFareCurrencyCode,
                                        MoneyAmount& baseFare,
                                        DiagCollector* diag)
{
  if (_canadianPointOfSale)
    return;

  const CurrencyCode& calculationCurrency = farePath.calculationCurrency();
  MoneyAmount convertedPlusUpAmt = _amount;
  ExchRate exchangeRate = 0.0;

  if (diag)
  {
    diag->setf(std::ios::left, std::ios::adjustfield);

    TruePaxType tpt(trx, farePath);

    (*diag) << tpt.paxType();
    (*diag) << "-POS PU AMOUNT: " << std::setw(8) << Money(_amount, _currencyCode) << std::endl;
    (*diag) << "    * FARE CALC CURRENCY  : " << calculationCurrency;
  }

  // 1st step: Check PU currency against Calculation currency
  if (calculationCurrency != _currencyCode)
  {
    // Need to convert the Plus Up amount to the Calculation currency
    convertedPlusUpAmt =
        convertAmount(trx, _amount, _currencyCode, calculationCurrency, exchangeRate);

    if (diag)
    {
      if (exchangeRate == 0.0)
      {
        (*diag) << " *** ROE TO " << calculationCurrency << " NOT AVAILABLE" << std::endl;
      }
      else
      {
        char strExchangeRate[15];
        sprintf(strExchangeRate, "%lf", exchangeRate);
        (*diag) << " *** ROE TO " << calculationCurrency << ": " << strExchangeRate << std::endl;
      }

      (*diag) << "    F/CALC PU AMT: " << std::setw(8)
              << Money(convertedPlusUpAmt, calculationCurrency) << std::endl;
    }
  }
  else
  {
    if (diag)
    {
      (*diag) << " *** NO CONVERSION REQUIRED" << std::endl;
    }
  }

  // Save fare calc amount for later use
  _fareCalcAmount = convertedPlusUpAmt;

  if (diag)
  {
    (*diag) << "    * BASE FARE CURRENCY  : " << baseFareCurrencyCode;
  }

  // 2nd step: Check Base Fare currency against Calculation currency
  if (baseFareCurrencyCode == farePath.calculationCurrency())
  {
    if (diag)
    {
      (*diag) << " *** NO CONVERSION REQUIRED" << std::endl;
      (*diag) << "    BASE FARE    : " << std::setw(8) << Money(baseFare, baseFareCurrencyCode)
              << std::endl;
    }

    baseFare += convertedPlusUpAmt;

    if (diag)
    {
      (*diag) << "    NEW BASE FARE: " << std::setw(8) << Money(baseFare, baseFareCurrencyCode)
              << std::endl;
      diag->printLine();
    }

    return;
  }

  // Need to convert the Plus Up amount in the Calculation currency
  //  to the Base Fare currency
  exchangeRate = 0.0;

  convertedPlusUpAmt = convertAmount(
      trx, convertedPlusUpAmt, calculationCurrency, baseFareCurrencyCode, exchangeRate);

  if (diag)
  {
    if (exchangeRate == 0.0)
    {
      (*diag) << " *** ROE TO " << baseFareCurrencyCode << " NOT AVAILABLE" << std::endl;
    }
    else
    {
      char strExchangeRate[15];
      sprintf(strExchangeRate, "%lf", exchangeRate);
      (*diag) << " *** ROE TO " << baseFareCurrencyCode << ": " << strExchangeRate << std::endl;
    }

    (*diag) << "    CONV PU AMT  : " << std::setw(8)
            << Money(convertedPlusUpAmt, baseFareCurrencyCode) << std::endl;
    (*diag) << "    BASE FARE    : " << std::setw(8) << Money(baseFare, baseFareCurrencyCode)
            << std::endl;
  }

  baseFare += convertedPlusUpAmt;

  if (diag)
  {
    (*diag) << "    NEW BASE FARE: " << std::setw(8) << Money(baseFare, baseFareCurrencyCode)
            << std::endl;
    diag->printLine();
  }
}

MoneyAmount
ConsolidatorPlusUp::calcTaxablePlusUpAmount(PricingTrx& trx,
                                            TaxCode& taxCode,
                                            const FarePath* farePath,
                                            Percent taxableAmtPercent)
{
  if (!_canadianPointOfSale)
    return 0.0;

  if ((strncmp(taxCode.c_str(), TAX_CODE_RC, 2) == 0) ||
      (strncmp(taxCode.c_str(), TAX_CODE_XG, 2) == 0) ||
      (strncmp(taxCode.c_str(), TAX_CODE_XQ, 2) == 0))
  {
    _taxCodes.insert(taxCode);

    ExchRate exchangeRate;

    // Need to convert the Plus Up amount to the Calculation currency
    MoneyAmount plusUpAmt =
        convertAmount(trx, _amount, _currencyCode, farePath->calculationCurrency(), exchangeRate);

    return (plusUpAmt * taxableAmtPercent);
  }
  else
    return 0.0;
}

void
ConsolidatorPlusUp::qualifyFarePath(PricingTrx& trx, FarePath& farePath, DiagCollector* diag)
{
  if (diag && !_diagHeaderAdded)
  {
    diag->printHeader();
    diag->printLine();
    (*diag) << *(trx.getRequest()->ticketingAgent());
    diag->printLine();

    _diagHeaderAdded = true;
  }

  auto& pus = farePath.pricingUnit();

  for (const auto pu : pus)
  {
    auto& fus = pu->fareUsage();
    for (const auto fu : fus)
    {
      const auto ptFare = fu->paxTypeFare();
      // Cat 35 fares T or C types should not price with PU qualifier
      if (ptFare->isNegotiated() &&
          (ptFare->fcaDisplayCatType() == ConsolidatorPlusUp::NET_SUBMIT_FARE ||
           ptFare->fcaDisplayCatType() == ConsolidatorPlusUp::NET_SUBMIT_FARE_UPD))
      {
        LOG4CXX_DEBUG(logger, "PU not allowed with cat 35 net fares");

        if (diag)
        {
          (*diag) << "NEGOTIATED NET FARE CANNOT BE PRICED WITH PU* QUALIFIER:" << std::endl << " "
                  << std::endl;
          (*diag) << *ptFare << std::endl;

          diag->printLine();
        }

        throw NonFatalErrorResponseException(
            ErrorResponseException::PU_NOT_ALLOWED_WITH_CAT35_NET_FARES);
      }
    } // for each fare usage
  } // for each pricing unit
}

MoneyAmount
ConsolidatorPlusUp::convertAmount(PricingTrx& trx,
                                  MoneyAmount& amount,
                                  const CurrencyCode& sourceCurrency,
                                  const CurrencyCode& targetCurrency,
                                  ExchRate& exchangeRate)
{
  if (sourceCurrency == targetCurrency)
  {
    exchangeRate = 1.0;
    return amount;
  }

  Money source(amount, sourceCurrency);
  Money target(targetCurrency);

  //-----------------------------------
  // NUC Conversion
  //-----------------------------------
  if (sourceCurrency == NUC || targetCurrency == NUC)
  {
    CurrencyConversionRequest currencyConvReq(target,
                                              source,
                                              trx.getRequest()->ticketingDT(),
                                              *(trx.getRequest()),
                                              trx.dataHandle(),
                                              true,
                                              CurrencyConversionRequest::DC_CONVERT);

    NUCCurrencyConverter nucConverter;
    NUCCollectionResults nucResults;
    nucResults.collect() = true;

    if (!nucConverter.convert(currencyConvReq, &nucResults))
    {
      LOG4CXX_ERROR(logger,
                    "Currency conversion failed From currency " << source.code() << " To currency "
                                                                << target.code());
      return 0.0;
    }

    exchangeRate = nucResults.exchangeRate();
  }

  //-----------------------------------
  // BSR Conversion
  //-----------------------------------
  else
  {
    CurrencyConversionFacade ccFacade;
    BSRCollectionResults bsrResults;

    if (!ccFacade.convert(
            target, source, trx, false, CurrencyConversionRequest::DC_CONVERT, false, &bsrResults))
    {
      LOG4CXX_ERROR(logger,
                    "Currency conversion failed From currency " << source.code() << " To currency "
                                                                << target.code());
      return 0.0;
    }

    exchangeRate = bsrResults.exchangeRate1();
  }

  return target.value();
}
} // tse namespace
