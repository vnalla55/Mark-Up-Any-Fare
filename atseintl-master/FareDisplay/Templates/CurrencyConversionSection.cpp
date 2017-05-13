//-------------------------------------------------------------------
//
//  File:     CurrencyConversionSection.cpp
//  Author:   LeAnn Perez
//  Date:     Jan 23, 2006
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/CurrencyConversionSection.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/NUCCollectionResults.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/Section.h"

namespace tse
{
FALLBACK_DECL(fallbackICERAPO44318Fix);

static Logger
logger("atseintl.FareDisplay.Templates.CurrencyConversionSection");

bool
CurrencyConversionSection::getConversionInfo(CurrencyCode& sourceCurrency,
                                             CurrencyCode& targetCurrency,
                                             CurrencyCode& intermediateCurrency,
                                             ExchRate& exchangeRate1,
                                             ExchRate& exchangeRate2,
                                             CurrencyNoDec& exchangRate1NoDec)
{
  LOG4CXX_DEBUG(logger, "Entering CurrencyConversionSection::getConversionInfo");

  // Get Source Currency
  const PaxTypeFare& paxTypeFare = *(_trx.allPaxTypeFare().front());
  const Money sourceMoney(paxTypeFare.originalFareAmount(), paxTypeFare.currency());

  // Get Target Currency for Display
  // CurrencyCode displayCurrency;
  // FareDisplayUtil::getDisplayCurrency(*_trx, displayCurrency);
  // Money targetMoney(displayCurrency);
  Itin* itin = _trx.itin().front();
  CurrencyCode displayCurrency = itin->calculationCurrency();
  Money targetMoney(displayCurrency);

  if (paxTypeFare.currency() == displayCurrency)
    return false;

  // ----------------------------------
  // NUC Conversion
  // ----------------------------------
  if (displayCurrency == NUC)
  {
    NUCCurrencyConverter nucConverter;
    Money amountNUC(0, NUC);

    Money amount(paxTypeFare.originalFareAmount(), paxTypeFare.currency());

    NUCCollectionResults nucResults;
    nucResults.collect() = true;

    CurrencyConversionRequest curConvReq(amountNUC,
                                         amount,
                                         _trx.getRequest()->ticketingDT(),
                                         *(_trx.getRequest()),
                                         _trx.dataHandle(),
                                         paxTypeFare.isInternational(),
                                         CurrencyConversionRequest::FAREDISPLAY);

    bool convertRC = nucConverter.convert(curConvReq, &nucResults);
    if (!convertRC)
    {
      return false;
    }
    else
    {
      exchangeRate1 = nucResults.exchangeRate();
      sourceCurrency = paxTypeFare.currency();
      targetCurrency = displayCurrency;
    }

    LOG4CXX_DEBUG(logger, "ROE Exchange Rate " << nucResults.exchangeRate());
    return true;
  }

  CurrencyConversionRequest request(targetMoney,
                                    sourceMoney,
                                    _trx.getRequest()->ticketingDT(),
                                    *(_trx.getRequest()),
                                    _trx.dataHandle(),
                                    false,
                                    CurrencyConversionRequest::FAREDISPLAY,
                                    false,
                                    _trx.getOptions());

  BSRCurrencyConverter bsrConverter;
  BSRCollectionResults bsrResults;
  bsrResults.collect() = true;

  try
  {
    bool rc = bsrConverter.convert(request, &bsrResults);
    if (!rc)
    {
      LOG4CXX_ERROR(logger,
                    "BSR Rate:" << paxTypeFare.currency() << " AND " << displayCurrency
                                << " WAS NOT AVAILABLE");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
    }
  }
  catch (tse::ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(logger, "BSR Converter exception: " << ex.what());
    throw NonFatalErrorResponseException(ex.code(), ex.what());
  }

  sourceCurrency = bsrResults.sourceCurrency();
  targetCurrency = bsrResults.targetCurrency();
  intermediateCurrency = bsrResults.intermediateCurrency();

  if (!fallback::fallbackICERAPO44318Fix(&_trx))
  {
    if (TrxUtil::isIcerActivated(_trx))
    {
      exchangeRate1 = bsrResults.exchangeRate1();
      exchangRate1NoDec = bsrResults.exchangeRate1NoDec();
    }
    else
    {
      if (_trx.getOptions()->bsrReciprocal())
      {
        exchangeRate1 = bsrResults.taxReciprocalRate1();
        exchangeRate2 = bsrResults.taxReciprocalRate2();
      }
      else
      {
        exchangeRate1 = bsrResults.exchangeRate1();
        exchangeRate2 = bsrResults.exchangeRate2();
      }
    }
  }
  else
  {
    if (_trx.getOptions()->bsrReciprocal())
    {
      exchangeRate1 = bsrResults.taxReciprocalRate1();
      exchangeRate2 = bsrResults.taxReciprocalRate2();
    }
    else
    {
      exchangeRate1 = bsrResults.exchangeRate1();
      exchangeRate2 = bsrResults.exchangeRate2();
    }

    if (TrxUtil::isIcerActivated(_trx))
    {
      exchangRate1NoDec = bsrResults.exchangeRate1NoDec();
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving CurrencyConversionSection::getConversionInfo");

  return true;
}
} // tse namespace
