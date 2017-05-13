//-------------------------------------------------------------------
//
//  File:     FTCurrencyConversionSection.cpp
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

#include "FareDisplay/Templates/FTCurrencyConversionSection.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
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

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Templates.FTCurrencyConversionSection");

void
FTCurrencyConversionSection::buildDisplay()
{
  LOG4CXX_DEBUG(logger, "Entering buildDisplay");

  if (_trx.allPaxTypeFare().empty())
  {
    LOG4CXX_DEBUG(logger, "buildDisplay:: allPaxTypeFare is empty. Going back");
    return;
  }

  // Format the Display
  std::ostringstream oss[2];
  initializeLine(&oss[0], 0);
  initializeLine(&oss[1], 0);
  oss[0].seekp(0, std::ios_base::beg);
  oss[1].seekp(0, std::ios_base::beg);

  // Format Fare Basis Code
  const PaxTypeFare& paxTypeFare = *(_trx.allPaxTypeFare().front());
  std::string fareBasis((paxTypeFare.createFareBasisCodeFD(_trx)).c_str());
  oss[0] << "FARE BASIS - ";
  oss[0] << std::setfill(' ') << std::setw(15);
  oss[0].setf(std::ios::left, std::ios::adjustfield);
  oss[0] << fareBasis;

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
    _trx.response() << oss[0].str();
    _trx.response() << " " << std::endl;
    return;
  }

  // Get Display Currency
  // CurrencyCode displayCurrency;
  // FareDisplayUtil::getDisplayCurrency(*_trx, displayCurrency);
  Itin* itin = _trx.itin().front();
  CurrencyCode displayCurrency = itin->calculationCurrency();

  oss[0].seekp(41, std::ios_base::beg);
  oss[1].seekp(41, std::ios_base::beg);

  // Display NUC Currency Header Message
  if (displayCurrency == NUC)
  {
    oss[0] << "NUC CONV- " << paxTypeFare.currency() << " TO " << displayCurrency;
    _trx.response() << oss[0].str();
    _trx.response() << " " << std::endl;
    return;
  }

  // Display BSR Currency Conversion Header - Single Conversion
  if (intermediateCurrency.empty())
  {
    if (sourceCurrency != targetCurrency)
    {

      oss[0] << "BSR CONV- " << sourceCurrency << " TO " << targetCurrency;
    }

    if (!oss[0].str().empty())
    {
      _trx.response() << oss[0].str();
      _trx.response() << " " << std::endl;
    }

    return;
  }

  // ----------------------------------------------------------
  // Display BSR Currency Conversion Header - Double Conversion
  // If IntermediateCurrency Present
  // ----------------------------------------------------------

  // SourceCur to intermediateCur using bsr 1 intermediateCur - sourceCur
  // IntermediateCur to TargetCur using bsr 1 IntermediateCur - TargetCur

  if (sourceCurrency != intermediateCurrency)
  {
    oss[0] << "BSR CONV- " << sourceCurrency << " TO " << intermediateCurrency;
  }

  if (intermediateCurrency != targetCurrency)
  {
    // --------------------------
    // Generate Conversion Line2
    // --------------------------
    if (sourceCurrency != intermediateCurrency)
    {
      oss[1] << "BSR CONV- " << intermediateCurrency << " TO " << targetCurrency;
    }
  }

  _trx.response() << oss[0].str();
  if (!oss[1].str().empty())
  {
    _trx.response() << oss[1].str();
    _trx.response() << " " << std::endl;
  }

  LOG4CXX_DEBUG(logger, "Leaving FTCurrencyConversionSection");
}
} // tse namespace
