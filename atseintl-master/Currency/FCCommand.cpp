//----------------------------------------------------------------------------
//
//        File: FCCommand.cpp
// Description: FC command class will display NUC rates
//     Created: 10/11/07
//     Authors: Svetlana Tsarkova, Tomasz Karczewski
//
//  Copyright Sabre 2003
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
#include "Currency/FCCommand.h"

#include "Common/Logger.h"
#include "Currency/FCConvert.h"
#include "Currency/FCDisplay.h"
#include "Currency/FCException.h"
#include "Currency/FCHelp.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"

#include <cfloat> // for DBL_EPSILON
#include <stdexcept>

namespace tse
{
static Logger
logger("atseintl.Currency.FCCommand");

using namespace std;
using namespace fc_exceptions;

void
FCCommand::process()
{
  FCCommand* executor = nullptr;
  if (_trx.commandType() == 'D')
  {
    if (_trx.billing()->actionCode().find("FCHEL") == 0)
    {
      executor = new FCHelp(_trx);
    }
    else
    {
      executor = new FCDisplay(_trx);
    }
  }
  else if (_trx.commandType() == 'C')
  {
    executor = new FCConvert(_trx);
  }

  if (executor)
  {
    executor->process();
    delete executor;
  }
  else
  {
    LOG4CXX_ERROR(logger, "Unknown command type - " << _trx.commandType());
    _trx.response() << UNKNOWN_REQUEST_STRING << '\n';
    _trx.setErrorResponse();
  }
}

//-----------------------------------------------------------------------------
//   @method  displayStandardHeader();
//
//   Description: Displays standard output header message
//
//   @return
//-----------------------------------------------------------------------------
void
FCCommand::displayStandardHeader()
{
  _trx.response() << "NUC - NEUTRAL UNIT OF CONSTRUCTION RATES LISTED -" << endl;
  if (should_display_fchelp_info())
    _trx.response() << "ENTER FCHELP FOR ADDITIONAL FC NUC FORMATS" << endl;
  _trx.response() << " " << endl;
}

bool
FCCommand::should_display_fchelp_info()
{
  return !(_trx.getRequest() && _trx.getRequest()->ticketingAgent() &&
           _trx.getRequest()->ticketingAgent()->axessUser());
}

string
FCCommand::format_date(const DateTime& date)
{
  static DateTime infinity = DateTime(9999, 12, 30);
  return (date >= infinity ? "         " : date.dateToString(DDMMMYYYY, nullptr));
}

string
FCCommand::format_nuc_rate(const NUCInfo& nuc, bool append_left)
{
  if (nuc._nucFactor >= 100000000)
    return NUC_TOO_HIGH_VALUE;

  char buff[17];
  // have to truncate, as snprintf may round the value otherwise
  double to_show = truncate(nuc._nucFactor, 6);

  if (append_left)
    snprintf(buff, 16, "%-15.6f", to_show);
  else
    snprintf(buff, 16, "%15.6f", to_show);

  // if appending left - remove spaces on the right
  if (append_left)
    for (char character : buff)
      if (character == ' ')
        character = '\0';

  return static_cast<string>(buff);
}
// slightly modified copy of CurrencyUtil::truncateNUCAmount()
// - that function didn't allow different no_dec values (were always set to 2)
double
FCCommand::truncate(const double& value, unsigned no_dec)
{
  LOG4CXX_DEBUG(logger, "truncate() val=" << value << ",no_dec=" << no_dec);

  unsigned decimal_factor = static_cast<unsigned>(::round(::pow(10, no_dec)));
  int e;
  double mantissa = frexp(value, &e);
  mantissa += DBL_EPSILON;
  double to_ret = trunc(decimal_factor * (ldexp(mantissa, e))) / decimal_factor;
  LOG4CXX_DEBUG(logger, "truncated value: " << to_ret);
  return to_ret;
}

bool
FCCommand::currency_code_exists(const CurrencyCode& curr)
{
  bool to_ret = false;
  try
  {
    const std::vector<CurrencyCode>& allCurrencies = _trx.dataHandle().getAllCurrencies();
    to_ret = (find(allCurrencies.begin(), allCurrencies.end(), curr) != allCurrencies.end());
  }
  catch (...) {}
  return to_ret;
}

void
FCCommand::validate_historical_date(const DateTime& date)
{
  // only check non-infinity dates
  if (date.isValid())
  {
    // check date
    int y = _trx.transactionStartTime().year() - 2;
    int m = _trx.transactionStartTime().month();
    int d = _trx.transactionStartTime().day();
    if (m == 2 && d == 29)
    {
      // it is leap year's 29FEB !
      m = 3;
      d = 1;
    }

    DateTime oldestAllowed = DateTime(y, m, d);
    if (date < oldestAllowed)
    {
      _trx.setErrorResponse();
      throw WrongDateEntered(BEYOND_MAX_HIST_DATE + oldestAllowed.dateToString(DDMMMYY, nullptr),
                             logger);
    }
  }
}
} // tse namespace
