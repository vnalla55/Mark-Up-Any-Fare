#include "Currency/FCDisplay.h"

#include "Currency/FCException.h"
#include "DataModel/Billing.h"
#include "DataModel/CurrencyTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"

#include <stdexcept>

using namespace std;
using namespace fc_exceptions;

namespace tse
{
static Logger
logger("atseintl.Currency.FCDisplay");

void
FCDisplay::append_output_line(NUCInfo& nuc)
{
  // CURRENCY..cxr.......NUC RATE.......EFF SALE......LAST SALE-TKT
  _trx.response().setf(ios::left, ios::adjustfield);
  _trx.response() << setw(CURRENCY_COLUMN_WIDTH) << nuc._cur << setw(CARRIER_COLUMN_WIDTH)
                  << nuc._carrier << setw(NUCRATE_COLUMN_WIDTH) << format_nuc_rate(nuc, false)
                  << setw(EFFSALE_COLUMN_WIDTH) << format_date(nuc.effDate())
                  << setw(LASTSALE_COLUMN_WIDTH) << format_date(nuc.expireDate());
}

void
FCDisplay::show_column_names()
{
  _trx.response().setf(ios::left, ios::adjustfield);
  _trx.response() << setw(CURRENCY_COLUMN_WIDTH) << COLUMN_NAME_CURRENCY
                  << setw(CARRIER_COLUMN_WIDTH) << COLUMN_NAME_CARRIER << setw(NUCRATE_COLUMN_WIDTH)
                  << COLUMN_NAME_NUCRATE << setw(EFFSALE_COLUMN_WIDTH) << COLUMN_NAME_EFFSALE
                  << setw(LASTSALE_COLUMN_WIDTH) << COLUMN_NAME_LASTSALE << endl;
}

void
FCDisplay::show_response(const vector<NUCInfo*>& nucs)
{
  if (nucs.empty())
  {
    // couldn't find data for given currency
    throw NoNucRecord(DATA_NOT_ON_FILE_STRING, _trx.sourceCurrency(), logger);
  }
  else
  {
    displayStandardHeader();
    show_column_names();
    show_nuc_list(nucs);
  }
}

void
FCDisplay::show_nuc_list(const vector<NUCInfo*>& nucs)
{
  for (auto & nuc : nucs)
  {
    append_output_line(*nuc);
    _trx.response() << endl;
  }
}

void
FCDisplay::determine_ranges(DateRange& date_range, CurrenciesRange& currencies_range)
{
  currencies_range = UNSPECIFIED_CURRENCIES_RANGE;
  date_range = UNSPECIFIED_DATE_RANGE;

  // check is historical date is valid
  validate_historical_date(_trx.baseDT());

  const string& action_code = _trx.billing()->actionCode();
  if (action_code.find("FC**") != string::npos) // always check 'fc**' befor e 'fc*'!
  {
    if (_trx.baseDT().isPosInfinity())
    {
      date_range = CURRENT_TO_INFINITY;
    }
    else
    {
      date_range = INVALID_DATE_RANGE;
      _trx.setErrorResponse();
      throw FormatError(logger);
    }
  }
  else if (action_code.find("FC*") != string::npos)
  {
    if (_trx.baseDT().isPosInfinity())
    {
      date_range = CURRENT;
    }
    else if (_trx.baseDT().isValid())
    {
      date_range = FROM_DATE_TO_INFINITY;
    }
    else
    {
      // we got some invalid value
      date_range = INVALID_DATE_RANGE;
      throw runtime_error("invalid date range");
    }
  }
  else
  {
    // wrong action code
    // date / currencies - unspecified
    throw runtime_error("unknown display action code");
  }

  // show single or all currencies ?
  if (_trx.sourceCurrency() == EMPTY_CURRENCY)
  {
    currencies_range = DISPLAY_ALL;
  }
  else
  {
    currencies_range = DISPLAY_SINGLE;
  }

  if (date_range == UNSPECIFIED_DATE_RANGE || currencies_range == UNSPECIFIED_CURRENCIES_RANGE)
    throw runtime_error("unspecified date/currency ranges");
}

const vector<NUCInfo*>&
FCDisplay::get_nuc_info(const DateTime& start_date,
                        const DateTime& end_date,
                        const CurrencyCode& currency,
                        CurrenciesRange curr_range)
{
  const vector<NUCInfo*>* to_ret = nullptr;
  if (curr_range == DISPLAY_SINGLE)
  {
    to_ret = &get_nuc_info_for_single_currency(start_date, end_date, currency);
  }
  else if (curr_range == DISPLAY_ALL)
  {
    to_ret = &get_nuc_info_for_multiple_currencies(start_date, end_date);
  }
  else
  {
    // processing error - shouldn't get here
    throw runtime_error("Processing error in get_nuc_info()");
  }
  if (to_ret)
  {
    if (to_ret->empty())
      throw NoNucRecord(DATA_NOT_ON_FILE_STRING, currency, logger);
    // check if start date is covered - if not, NO DATA ON FILE
    for (vector<NUCInfo*>::const_iterator it = to_ret->begin(); it != to_ret->end(); ++it)
    {
      if ((*it)->effDate() <= start_date && (*it)->expireDate() >= start_date)
        return *to_ret; // found current record
    }
    // currency is OK - we found some records - but not the current one
    throw NoNucRecord(DATA_NOT_ON_FILE_STRING, currency, logger);
  }
  else
  {
    // shouldn't get here
    throw new runtime_error("processing error #2 in get_nuc_info");
  }
}

const vector<NUCInfo*>&
FCDisplay::get_nuc_info_for_multiple_currencies(const DateTime& start_date,
                                                const DateTime& end_date)
{
  try
  {
    const vector<CurrencyCode>& all_currencies = _trx.dataHandle().getAllCurrencies();

    for (const auto currency : all_currencies)
    {
      const vector<NUCInfo*>& nxt_curr =
          get_nuc_info_for_single_currency(start_date, end_date, currency);
      copy(nxt_curr.begin(), nxt_curr.end(), back_inserter(nuc_vector));
    }
  }
  catch (const exception& x)
  {
    throw runtime_error("error obtaining nuc info for single currency");
  }

  if (nuc_vector.empty())
    throw OtherUserError(DATA_NOT_ON_FILE_STRING, logger);
  return nuc_vector;
}

void
FCDisplay::process()
{
  // FC* -> all, current only
  // FC*DDMMMYY -> all, from date on entry to +infinity
  // FC** -> all, from current to +infinity
  // FC*xxx -> specific currency, current only
  // FC*xxxDDMMMYY -> specific currency, from date on entry to +infinity
  // FC**xxx -> specific currency, from current to +infinity
  static const DateTime positive_infinity = DateTime(boost::date_time::pos_infin);
  DateTime current = _trx.transactionStartTime();
  const vector<NUCInfo*>* nucs = nullptr;

  try
  {
    DateRange date_range;
    CurrenciesRange currency_range;
    determine_ranges(date_range, currency_range);

    switch (date_range)
    {
    case CURRENT:
      nucs = &get_nuc_info(current, current, _trx.sourceCurrency(), currency_range);
      break;

    case CURRENT_TO_INFINITY:
      nucs = &get_nuc_info(current, positive_infinity, _trx.sourceCurrency(), currency_range);
      break;

    case FROM_DATE_TO_INFINITY:
      nucs = &get_nuc_info(_trx.baseDT(), positive_infinity, _trx.sourceCurrency(), currency_range);
      break;

    default:
      throw runtime_error("wrong/unknown request data");
    }

    if (nucs && nucs->size() > 0)
    {
      sort_nuc_list(*nucs);
      show_response(*nucs);
    }
    else
      throw runtime_error("empty nuc list returned");
  }
  catch (const NoNucRecord& err)
  {
    string msg = "";
    if (currency_code_exists(err.currency))
    {
      msg = DATA_NOT_ON_FILE_STRING;
      _trx.setErrorResponse();
    }
    else
    {
      msg = INVALID_CURRENCY_STRING;
      _trx.setErrorResponse();
      if (should_display_fchelp_info())
        msg = msg + REFER_TO_FCHELP;
    }
    _trx.response() << msg;
    LOG4CXX_ERROR(logger, msg);
  }
  catch (const WrongDateEntered& err)
  {
    // don't append 'REFER TO ...' message
    _trx.response() << err.what();
    LOG4CXX_ERROR(logger, err.what());
  }
  catch (const OtherUserError& err)
  {
    // error messages added to response
    _trx.response() << err.what();
    if (should_display_fchelp_info())
      _trx.response() << REFER_TO_FCHELP;
    LOG4CXX_ERROR(logger, err.what());
  }
  catch (const runtime_error& err)
  {
    _trx.response() << PROCESSING_ERROR_STRING;
    _trx.setErrorResponse();
    LOG4CXX_ERROR(logger, "runtime error: " << err.what());
  }
  catch (...)
  {
    _trx.response() << UNKNOWN_ERROR_STRING;
    _trx.setErrorResponse();
    LOG4CXX_ERROR(logger, "unknown error");
  }
}

// data is sorted by:
// (1) currency code,
// (2) carrier code - but blank ("") carrier should always go last
// (3) effective date
static bool
compare_nuc(const NUCInfo* n1, const NUCInfo* n2)
{
  if (n1->_cur != n2->_cur)
    return n1->_cur < n2->_cur;
  else if (n1->_carrier != n2->_carrier)
  {
    if (n1->_carrier.empty())
      return false;
    return n2->_carrier.empty() || (n1->_carrier < n2->_carrier);
  }
  else
    return (n1->effDate() < n2->effDate());
}

void
FCDisplay::sort_nuc_list(const vector<NUCInfo*>& vec_)
{
  vector<NUCInfo*>& vec = const_cast<vector<NUCInfo*>&>(vec_);
  sort(vec.begin(), vec.end(), ptr_fun(compare_nuc));
}
const vector<NUCInfo*>&
FCDisplay::get_nuc_info_for_single_currency(const DateTime& start_date,
                                            const DateTime& end_date,
                                            const CurrencyCode& currency)
{
  try
  {
    const vector<NUCInfo*>& to_ret =
        (_trx.dataHandle().getNUCAllCarriers(currency, start_date, end_date));

    return to_ret;
  }
  catch (exception& x)
  {
    LOG4CXX_WARN(logger, "unknown exception get_nuc_info_for_single_currency()");
    throw runtime_error("Error in get_nuc_info_for_single_currency()");
  }
}
} // tse namespace
