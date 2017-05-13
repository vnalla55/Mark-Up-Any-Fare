//----------------------------------------------------------------------------
//
//        File: CurrencyService.cpp
// Description: Currency service class
//     Created: 06/07/2004
//     Authors: Mark Kasprowicz
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Currency/CurrencyService.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Currency/FCCommand.h"
#include "Currency/LocalCurrencyDisplay.h"
#include "DataModel/Billing.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/Trx.h"
#include "Server/TseServer.h"

namespace tse
{
static Logger
logger("atseintl.Currency.CurrencyService");

using namespace std;

static LoadableModuleRegister<Service, CurrencyService>
_("libCurrencyService.so");

bool
CurrencyService::process(CurrencyTrx& trx)
{
  // check request type to determine what command to process
  LOG4CXX_DEBUG(logger, "Entering CurrencyTrx::process");

  bool returnCode = true; // default is OK

  if (trx.requestType() == 'F') // FC command
  {
    LOG4CXX_DEBUG(logger, "Processing FC command");

    FCCommand fcCommand(trx);
    fcCommand.process();
  }
  else if (trx.requestType() == 'D') // DC command
  {
    LOG4CXX_DEBUG(logger, "Processing DC command");
    LocalCurrencyDisplay currencyDisplay(trx);

    // check action code to determine whether to display or convert

    if (trx.commandType() == 'D')
    {
      const DateTime effectiveDate = !trx.baseDT().isInfinity() ? trx.baseDT() : trx.pssLocalDate();
      if ( TrxUtil::isIcerActivated( trx, effectiveDate ) )
      {
        if (trx.sourceCurrency().equalToConst("CUR"))
          currencyDisplay.displayAllRates( effectiveDate );
        else
          currencyDisplay.displayRate( effectiveDate );
      }
      else
      {
        if (trx.sourceCurrency().equalToConst("CUR"))
          currencyDisplay.displayAllRates();
        else
        {
          const CurrencyCode& primaryCurrency = trx.sourceCurrency();
          currencyDisplay.displayRate(primaryCurrency);
        }
      }
    }
    else if (trx.commandType() == 'C')
    {
      currencyDisplay.convert();
    }
  }
  else
  {
    // invalid/unknown request type
    LOG4CXX_ERROR(logger,
                  "Unknown/Invalid requestType: '" << trx.requestType() << "' for CurrencyService");
    trx.response() << "UNKNOWN REQUEST" << endl;
    returnCode = false;
  }

  LOG4CXX_DEBUG(logger, "Leaving CurrencyTrx::process");

  return returnCode;
}
} // tse namespace
