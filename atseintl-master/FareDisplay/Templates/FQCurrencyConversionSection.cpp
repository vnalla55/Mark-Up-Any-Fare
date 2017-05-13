//-------------------------------------------------------------------
//
//  File:     FQCurrencyConversionSection.cpp
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
#include "FareDisplay/Templates/FQCurrencyConversionSection.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
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

static Logger
logger("atseintl.FareDisplay.Templates.FQCurrencyConversionSection");

void
FQCurrencyConversionSection::buildDisplay()
{
  LOG4CXX_DEBUG(logger, "Entering buildDisplay");

  if (_trx.allPaxTypeFare().empty())
  {
    LOG4CXX_DEBUG(logger, "buildDisplay:: allPaxTypeFare is empty. Going back");
    return;
  }

  // Get Display Currency
  // CurrencyCode displayCurrency;
  // FareDisplayUtil::getDisplayCurrency(*_trx, displayCurrency);
  Itin* itin = _trx.itin().front();
  CurrencyCode displayCurrency = itin->calculationCurrency();

  const PaxTypeFare& paxTypeFare = *(_trx.allPaxTypeFare().front());

  std::ostringstream oss[2];
  initializeLine(&oss[0], 1);
  initializeLine(&oss[1], 1);
  oss[0].seekp(0, std::ios_base::beg);
  oss[1].seekp(0, std::ios_base::beg);

  // Display Multiple Currency Header
  if (_trx.multipleCurrencies())
  {
    if (displayCurrency == NUC)
    {
      oss[0] << "MULTIPLE SELLING CURRENCIES CONVERTED TO " << displayCurrency
             << " USING CURRENT ROE";
    }
    else
    {
      oss[0] << "MULTIPLE SELLING CURRENCIES CONVERTED TO " << displayCurrency
             << " USING CURRENT BSR";
    }

    _trx.response() << oss[0].str();
    return;
  }

  // Get Currency Converion Info
  CurrencyCode sourceCurrency;
  CurrencyCode targetCurrency;
  CurrencyCode intermediateCurrency;
  ExchRate exchangeRate1;
  ExchRate exchangeRate2;
  CurrencyNoDec exchangRate1NoDec;

  if (!getConversionInfo(
          sourceCurrency, targetCurrency, intermediateCurrency, exchangeRate1, exchangeRate2,
          exchangRate1NoDec))
  {
    return;
  }

  // Display NUC Conversion Header
  if (displayCurrency == NUC)
  {
    oss[0] << paxTypeFare.currency() << " CONVERTED TO " << displayCurrency << " USING ROE ";
    oss[0].setf(std::ios::fixed, std::ios::floatfield);
    oss[0].setf(std::ios::right, std::ios::adjustfield);
    oss[0] << std::setw(8) << exchangeRate1 << " - 1 NUC";

    _trx.response() << oss[0].str();
    return;
  }

  if (TrxUtil::isIcerActivated(_trx))
  {
    if (sourceCurrency != targetCurrency)
    {
      oss[0] << sourceCurrency << " CONVERTED TO " << targetCurrency;
      std::string formattedRate = FareCalcUtil::formatExchangeRate(exchangeRate1, exchangRate1NoDec);
      oss[0] << " USING BSR 1 " << sourceCurrency << " - " << formattedRate << " "
             << targetCurrency;
      _trx.response() << oss[0].str();
    }
    LOG4CXX_DEBUG(logger, "Leaving FQCurrencyConversionSection");
    return;
  }

  // Display BSR Currency Conversion Header - Single Currency Conversion
  if (intermediateCurrency.empty())
  {
    if (sourceCurrency != targetCurrency)
    {
      oss[0] << sourceCurrency << " CONVERTED TO " << targetCurrency;
    }

    CurrencyCode aaaCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    LocCode aaaLoc = _trx.getRequest()->ticketingAgent()->agentCity();
    if (!(_trx.getRequest()->salePointOverride().empty()))
    {
      aaaLoc = _trx.getRequest()->salePointOverride();
    }

    const Loc* loc = _trx.dataHandle().getLoc(aaaLoc, _trx.travelDate());
    if (loc)
    {
      NationCode nation = loc->nation();
      CurrencyUtil::getNationCurrency(nation, aaaCurrency, _trx.ticketingDate());
    }

    if (targetCurrency == aaaCurrency)
    {
      if (_trx.getOptions()->bsrReciprocal() && _trx.getRequest()->displayCurrency().empty())

      {
        oss[0] << " USING BSR " << exchangeRate1 << " " << targetCurrency << " - 1 "
               << sourceCurrency;
      }
      else
      {
        oss[0] << " USING BSR 1 " << targetCurrency << " - " << exchangeRate1 << " "
               << sourceCurrency;
      }
    }
    else
    {
      oss[0] << " USING BSR 1 " << sourceCurrency << " - " << exchangeRate1 << " "
             << targetCurrency;
    }

    if (!oss[0].str().empty())
    {
      _trx.response() << oss[0].str();
    }

    return;
  }

  // -------------------------------------------------------------------
  // Display BSR Currency Conversion Header - Double Currency Conversion
  // If IntermediateCurrency Present
  // -------------------------------------------------------------------

  // SourceCur to intermediateCur using bsr 1 intermediateCur - sourceCur
  // IntermediateCur to TargetCur using bsr 1 IntermediateCur - TargetCur

  if (sourceCurrency != intermediateCurrency)
  {
    if (_trx.getOptions()->bsrReciprocal())
    {
      oss[0] << sourceCurrency << " CONVERTED TO " << intermediateCurrency << " USING BSR ";
      oss[0].setf(std::ios::fixed, std::ios::floatfield);
      oss[0].setf(std::ios::right, std::ios::adjustfield);
      oss[0] << std::setw(8) << exchangeRate1;
      oss[0] << " " << intermediateCurrency << " - 1 " << sourceCurrency;
    }
    else
    {
      oss[0] << sourceCurrency << " CONVERTED TO " << intermediateCurrency << " USING BSR 1 "
             << intermediateCurrency << " - ";
      oss[0].setf(std::ios::fixed, std::ios::floatfield);
      oss[0].setf(std::ios::right, std::ios::adjustfield);
      oss[0] << std::setw(8) << exchangeRate1 << " " << sourceCurrency;
    }
  }

  if (intermediateCurrency != targetCurrency)
  {
    // --------------------------
    // Generate Conversion Line2
    // --------------------------
    if (sourceCurrency != intermediateCurrency)
    {
      if (_trx.getOptions()->bsrReciprocal())
      {
        oss[1] << "THEN " << intermediateCurrency << " CONVERTED TO " << targetCurrency
               << " USING BSR ";
        oss[1].setf(std::ios::fixed, std::ios::floatfield);
        oss[1].setf(std::ios::right, std::ios::adjustfield);
        oss[1] << std::setw(8) << exchangeRate2;
        oss[1] << " " << intermediateCurrency << " - 1 " << targetCurrency;
      }
      else
      {
        oss[1] << "THEN " << intermediateCurrency << " CONVERTED TO " << targetCurrency
               << " USING BSR 1 " << intermediateCurrency << " - ";
        oss[1].setf(std::ios::fixed, std::ios::floatfield);
        oss[1].setf(std::ios::right, std::ios::adjustfield);
        oss[1] << std::setw(8) << exchangeRate2;
        oss[1] << " " << targetCurrency;
      }
    }
    else
    {
      oss[0] << intermediateCurrency << " CONVERTED TO " << targetCurrency << " USING BSR 1 "
             << intermediateCurrency << " - " << exchangeRate2 << " " << targetCurrency;
    }
  }

  _trx.response() << oss[0].str();

  if (!oss[1].str().empty())
  {
    _trx.response() << oss[1].str();
  }

  LOG4CXX_DEBUG(logger, "Leaving FQCurrencyConversionSection");
}
} // namespace tse
