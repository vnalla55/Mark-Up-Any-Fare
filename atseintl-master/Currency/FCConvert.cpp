//----------------------------------------------------------------------------
//
//        File: FCConvert.h
// Description: FCConvert class converts currencies
//		to NUC and NUC to currencies,
//		and adds formatted output to
//		the response
//		provides processing for entries:
//			FC$
//
//     Created: 10/11/07
//     Authors: Tomasz Karczewski
//
//  Copyright Sabre 2007
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "Currency/FCConvert.h"

#include "Common/CurrencyConverter.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Currency/FCException.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"

#include <boost/lexical_cast.hpp>

#include <cmath>
#include <sstream>

using namespace std;
using namespace fc_exceptions;

namespace tse
{

static Logger
logger("atseintl.Currency.FCConvert");

double
FCConvert::convert_to_currency(double nuc_rate, double quantity)
{
  LOG4CXX_DEBUG(logger, "convert_to_currency()");
  if (nuc_rate <= 0 || quantity < 0)
    throw runtime_error(string("convert_to_currency() - wrong data: ") +
                        (nuc_rate <= 0 ? "nuc rate" : "quantity"));
  return quantity * nuc_rate;
}

double
FCConvert::convert_to_NUC(double nuc_rate, double quantity)
{
  LOG4CXX_DEBUG(logger, "convert_to_NUC()");
  if (nuc_rate <= 0 || quantity < 0)
    throw runtime_error(string("convert_to_NUC() - wrong data: ") +
                        (nuc_rate <= 0 ? "nuc rate" : "quantity"));
  return quantity / nuc_rate;
}

double
FCConvert::apply_rounding(double value, double rounding_factor, RoundingRule rule, unsigned no_dec)
{
  LOG4CXX_DEBUG(logger, "apply_rounding()");
  CurrencyConverter curr_conv;
  if (curr_conv.isZeroAmount(value))
  {
    LOG4CXX_DEBUG(logger, "rounding 0 - always results in 0");
    return 0;
  }

  Money amount(value, currency);
  if (value <= 0)
    throw runtime_error(string("wrong value to convert: ") + boost::lexical_cast<string>(value) +
                        " - should've been detected earlier");

  bool res = false;

  LOG4CXX_DEBUG(logger,
                string("ROUNDING: amount=") + boost::lexical_cast<string>(amount.value()) +
                    "factor_no: " + boost::lexical_cast<string>(rounding_factor));

  switch (rule)
  {
  case UP:
    LOG4CXX_DEBUG(logger, "rounding::UP");
    res = curr_conv.roundUp(amount, rounding_factor);
    break;

  case DOWN:
    LOG4CXX_DEBUG(logger, "rounding::DOWN");
    res = curr_conv.roundDown(amount, rounding_factor);
    break;

  case NEAREST:
    LOG4CXX_DEBUG(logger, "rounding::NEAREST");
    res = curr_conv.roundNearest(amount, rounding_factor);
    break;

  case NONE:
    LOG4CXX_DEBUG(logger, "rounding::NONE");
    res = curr_conv.roundNone(amount, rounding_factor);
    break;

  default:
    throw runtime_error("unknown rounding rule");
  };

  if (!res)
    throw runtime_error("rounding failed");

  LOG4CXX_ERROR(logger, "rounded value: " << amount.value());
  return amount.value();
}

string
FCConvert::format_rounding(const CurrencyCode& currency,
                           double value,
                           double rounding_factor,
                           RoundingRule rrule,
                           unsigned curr_decimals,
                           RoundingType type)
{
  LOG4CXX_DEBUG(logger, "format_rounding()");
  // INR        8140.8     TRUNCATED
  // INR        8145       ROUNDED UP TO NEXT  5 - OTHER/TAXES

  ostringstream oss;
  string truncated_line = format_truncated_line(currency, value, rounding_factor);

  oss.setf(ios::left, ios::adjustfield);
  oss << truncated_line << endl << setw(FCC_CURRENCY_CLMN_WIDTH) << currency
      << setw(FCC_VALUE_CLMN_WIDTH)
      << format_currency_display(
             apply_rounding(value, rounding_factor, rrule, curr_decimals), curr_decimals, 13)
      << get_rounding_rule_string(rrule, rounding_factor) << " - " << get_rounding_type_string(type)
      << endl;

  return oss.str();
}

string
FCConvert::format_truncated_line(const CurrencyCode& shown_code, double val, double rounding_factor)
{
  LOG4CXX_DEBUG(logger, "format_truncated_line()");

  unsigned tfr_nodec;

  double truncated_for_rounding_val = get_truncated_for_rounding(val, rounding_factor, tfr_nodec);
  stringstream oss;
  oss.setf(ios::left, ios::adjustfield);
  oss.precision(tfr_nodec);

  oss << setw(FCC_CURRENCY_CLMN_WIDTH) << shown_code << setw(FCC_VALUE_CLMN_WIDTH) << fixed
      << truncated_for_rounding_val << TRUNCATED_INFO_STRING;

  return oss.str();
}

string
FCConvert::format_currency_display(double val, unsigned curr_no_dec, int max_len)
{
  LOG4CXX_DEBUG(logger, "format_currency_display()");
  // truncate the value
  double value = truncate(val, curr_no_dec);
  ostringstream ss;
  ss.precision(curr_no_dec);
  ss.setf(ios::fixed);
  ss << value;
  string to_return = ss.str();
  // should leading '0' be appended ?
  if (to_return.size() > 0 && to_return[0] == '.')
    to_return.insert(to_return.begin(), '.');

  if (max_len > 0)
    if (to_return.size() > (unsigned)max_len)
      throw WrongConvertedValue(string("CURRENCY AMOUNT DISPLAY ") + to_return + " TOO LONG",
                                logger);

  return to_return;
}

const string&
FCConvert::get_rounding_type_string(RoundingType type)
{
  LOG4CXX_DEBUG(logger, "get_rounding_type_string()");
  return ROUNDING_TYPE_STRINGS[(unsigned)type];
}

string
FCConvert::get_rounding_rule_string(RoundingRule rule, double rounding_factor)
{
  LOG4CXX_DEBUG(logger, "get_rounding_rule_string()");
  string to_ret = (rule == UP) ? ROUNDING_STRINGS[0] : (rule == DOWN) ? ROUNDING_STRINGS[2]
                                                                      : (rule == NEAREST)
                                                                            ? ROUNDING_STRINGS[1]
                                                                            : ROUNDING_STRINGS[3];

  if (rule != NONE)
  {
    to_ret += " ";
    to_ret += boost::lexical_cast<string>(rounding_factor);
  }
  else
  {
    to_ret += " 0 ";
  }
  return to_ret;
}

const string
FCConvert::format_header(const NUCInfo& nuc_info, const DateTime& date)
{
  LOG4CXX_DEBUG(logger, "format_header()");
  // RATE 1NUC -  40.712345 XXX / 18SEP07
  ostringstream oss;

  LOG4CXX_DEBUG(logger, "format_header: date - " << date);
  LOG4CXX_DEBUG(logger, "local time: " << DateTime::localTime());

  DateTime tmp_date = (date.isPosInfinity() ? _trx.transactionStartTime() : date);

  LOG4CXX_DEBUG(logger, "date to show: " << date);

  if (!tmp_date.isValid())
    throw runtime_error("invalid date passed to fc convert header");

  LOG4CXX_DEBUG(logger, "date is valid");

  oss << "RATE 1NUC -  " << format_nuc_rate(nuc_info, true) << ' ' << nuc_info._cur << " / "
      << tmp_date.dateToString(DDMMMYY, nullptr);

  return oss.str();
}

// here: the processing sequence get_conversion_data->perform_conversion->show_response
// must be kept
void
FCConvert::process()
{
  LOG4CXX_DEBUG(logger, "process()");

  try
  {
    // get the currency data needed & save in member variables
    // throws: WrongDateEntered, NoNucRecord, NoCurrencyRecord, runtime_error
    get_conversion_data();

    // validate that all needed data is ok
    // throws: runtime_error,WrongDateEntered
    validate_data();

    // convert
    // throws : runtime_error
    perform_conversion();

    // show conversion
    // throws: runtime_error, WrongConvertedValue
    show_response();
  }
  catch (const RecordDoesntExist& x)
  {
    // 1) validate currency code
    // 2) if valid - no data on file, otherwise - invalid curr code
    string msg;
    if (currency_code_exists(x.currency))
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
    LOG4CXX_ERROR(logger, x.what());
  }
  catch (const WrongDateEntered& x)
  {
    // do not append 'REFER NO FCHELP'
    _trx.response() << x.what();
    LOG4CXX_ERROR(logger, x.what());
  }
  catch (const OtherUserError& x)
  {
    // added to response, message already formatted
    _trx.response() << x.what();
    if (should_display_fchelp_info())
      _trx.response() << REFER_TO_FCHELP;
    LOG4CXX_ERROR(logger, x.what());
  }
  catch (const runtime_error& x)
  {
    // logical errors, only "PROCESSING ERROR" to response
    _trx.response() << PROCESSING_ERROR_STRING;
    _trx.setErrorResponse();
    LOG4CXX_ERROR(logger, "runtime error - " << x.what());
  }
  catch (...)
  {
    // unknow errors, only "UNKNOWN ERROR" to response
    _trx.response() << UNKNOWN_ERROR_STRING;
    _trx.setErrorResponse();
    LOG4CXX_ERROR(logger, "other exception");
  }
}

double
FCConvert::get_truncated_for_rounding(double value, double rounding_factor, unsigned& tfr_nodec)
{
  LOG4CXX_DEBUG(logger, "get_truncated_for_rounding()");
  // one digit right of decimal on whole unit rounding factor
  // for others - 1 place right to factor
  unsigned no_dec = 1;
  if (rounding_factor < 1)
    no_dec += (unsigned)(-::floor(::log10(rounding_factor)));

  tfr_nodec = no_dec;
  return truncate(value, no_dec);
}

// sets type, currency, date; needs ConversionType set
void
FCConvert::get_conversion_data()
{
  LOG4CXX_DEBUG(logger, "get_conversion_data()");

  // determine conversion type
  conversionType = INVALID;
  LOG4CXX_DEBUG(logger,
                string("source(): ") + _trx.sourceCurrency() + ", target():" + _trx.targetCurrency()
                    << "\n");

  if (_trx.sourceCurrency() != NUC_CODE && _trx.targetCurrency() != NUC_CODE)
  {
    _trx.setErrorResponse();
    throw FormatError(logger);
  }

  if (_trx.sourceCurrency() == NUC_CODE)
  {
    conversionType = NUC_TO_CURRENCY;
  }
  else if (_trx.targetCurrency() == NUC_CODE)
  {
    conversionType = CURRENCY_TO_NUC;
  }

  if (conversionType == INVALID)
  {
    LOG4CXX_ERROR(logger, "invalid conversion data");
    throw runtime_error("unknown conversion type - should've been detected earlier");
  }

  CurrencyCode source = _trx.sourceCurrency();
  CurrencyCode dest = _trx.targetCurrency();
  const DateTime& date = _trx.baseDT();

  sourceVal = boost::lexical_cast<double>(_trx.amountStr());

  trxDate = (date.isPosInfinity() ? _trx.transactionStartTime() : date);

  validate_historical_date(trxDate);

  if (conversionType == NUC_TO_CURRENCY)
  {
    currency = dest;
    get_db_data(true);
  }
  else
  {
    currency = source;
    // not getting the roundings, as they won't be needed here
    get_db_data(false);
  }
}

// currency, trxDate have to be determined; sets other data
void
FCConvert::get_db_data(bool get_roundings)
{
  LOG4CXX_DEBUG(logger, "get_db_data()");
  get_nuc_rate();
  // also: get the roundings
  get_nodec_and_roundings_for_currency(get_roundings);
}

// currency, trxDate have to be determined
void
FCConvert::get_nuc_rate()
{
  LOG4CXX_DEBUG(logger, "get_nuc_rate()");
  // get historical rate
  NUCInfo* nuc = nullptr;
  //  nuc = _trx.dataHandle().getNUCFirst(currency,"",trxDate);// -> getNUCFirst() doesn't return
  // any data when NUCH table has invalid entries with EXPIREDATE before EFFDATE
  const vector<NUCInfo*> nucs = _trx.dataHandle().getNUCAllCarriers(currency, trxDate, trxDate);
  for (const auto NUCInfo : nucs)
  {
    if (NUCInfo->_carrier.empty() && NUCInfo->effDate() <= trxDate &&
        NUCInfo->expireDate() >= trxDate)
    {
      nuc = NUCInfo;
      break;
    }
  }

  if (nuc != nullptr)
  {
    currency_nuc = nuc;
    nucRate = nuc->_nucFactor;
    itlRoundingRule = nuc->_roundingRule;
    itlRoundingFactor = nuc->_roundingFactor; // nodec already applied in DBAccess layer(using
                                              // SQLQuery::adjustDecimal())
  }
  else
    throw NoNucRecord(string("NO NUC RATE FOR ") + currency + " WITH DATE  " +
                          trxDate.dateToString(DDMMMYY, nullptr),
                      currency,
                      logger);
}

// 'currency',trxDate have to be already determined
void
FCConvert::get_nodec_and_roundings_for_currency(bool get_roundings)
{
  LOG4CXX_DEBUG(logger, "get_nodec_and_roundings_for_currency()");
  const Currency* curr = nullptr;
  curr = _trx.dataHandle().getCurrency( currency );

  if (curr == nullptr)
    throw NoCurrencyRecord(string("NO CURRENCY RECORD FOR ") + currency + " WITH DATE " +
                               trxDate.dateToString(DDMMMYY, nullptr),
                           currency,
                           logger);

  currency_nodec = curr->noDec();

  if (get_roundings)
  {
    domRoundingFactor = curr->domRoundingFactor(); // nodec already applied
    if (domRoundingFactor == -1) // use international rounding ?
      domRoundingFactor = itlRoundingFactor;
    domRoundingRule = (domRoundingFactor != 0) ? itlRoundingRule : NONE;

    taxRoundingFactor = curr->taxOverrideRoundingUnit(); // nodec already applied
    if (taxRoundingFactor == -1) // use international factor ?
      taxRoundingFactor = itlRoundingFactor;

    taxRoundingRule = (curr->taxOverrideRoundingRule() == NONE) ? itlRoundingRule
                                                                : curr->taxOverrideRoundingRule();
  }
}

string
FCConvert::get_additional_header_message()
{
  if (conversionType == NUC_TO_CURRENCY && currency.equalToConst("USD"))
    return "DOMESTIC ROUNDING RULES APPLY ONLY IN USA/US TERRITORIES.\n"
           "USE INTERNATIONAL ROUNDING RULES FOR DOMESTIC ITINERARIES\n"
           "CALCULATED IN USD OUTSIDE USA/US TERRITORIES.";
  else
    return "";
}

// conversionType, convertedValue, roundings etc. - have to be already determined
void
FCConvert::show_response()
{
  LOG4CXX_DEBUG(logger, "show_response()");
  string res = create_response();
  displayStandardHeader();
  string additionalHeaderText = get_additional_header_message();
  if (additionalHeaderText.size() != 0)
  {
    _trx.response() << additionalHeaderText << endl << " " << endl;
  }

  _trx.response().setf(ios::left, ios::adjustfield);
  _trx.response() << flush << res << endl << ends;
}

// conversionType, convertedValue, roundings etc. - have to be already determined
string
FCConvert::create_response()
{
  LOG4CXX_DEBUG(logger, "create_response()");
  if (currency_nuc == nullptr)
    throw new runtime_error("nuc info not obtained - should've been detected earlier");

  ostringstream res;
  res << format_header(*currency_nuc, trxDate) << endl << " " << endl;
  if (conversionType == NUC_TO_CURRENCY)
  {
    res << format_rounding(currency,
                           convertedVal,
                           itlRoundingFactor,
                           itlRoundingRule,
                           currency_nodec,
                           RT_INTERNATIONAL) << endl << " " << endl;
    res << format_rounding(currency,
                           convertedVal,
                           domRoundingFactor,
                           domRoundingRule,
                           currency_nodec,
                           RT_DOMESTIC) << endl << " " << endl;
    res << format_rounding(currency,
                           convertedVal,
                           taxRoundingFactor,
                           taxRoundingRule,
                           currency_nodec,
                           RT_OTHER_TAXES) << endl << " " << endl;
  }

  else if (conversionType == CURRENCY_TO_NUC)
  {
    res << format_truncated_line(NUC_CODE, convertedVal, NUC_PSEUDO_ROUNDING_FACTOR) << endl;
  }
  else
  {
    throw runtime_error("unknow conversion type - should've been detected earlier");
  }
  return res.str();
}

// conversionType & convertedVal must already be already determined
void
FCConvert::perform_conversion()
{
  LOG4CXX_DEBUG(logger, "perform_conversion()");
  if (conversionType == NUC_TO_CURRENCY)
  {
    convertedVal = convert_to_currency(nucRate, sourceVal);
  }
  else
  {
    convertedVal = convert_to_NUC(nucRate, sourceVal);
  }
}

void
FCConvert::validate_data()
{
  if (sourceVal < 0)
    throw runtime_error("uninitialized source value");

  if (conversionType == NUC_TO_CURRENCY &&
      (itlRoundingFactor < 0 || domRoundingFactor < 0 || taxRoundingFactor < 0))
    throw runtime_error("one of the rounding factors not initialized for NUC->CURR conversion");

  if (currency_nuc == nullptr)
    throw runtime_error("NUC info not obtained");

  if (conversionType == INVALID)
    throw OtherUserError(UNKNOWN_REQUEST_STRING, logger);

  if (itlRoundingRule != UP && itlRoundingRule != DOWN && itlRoundingRule != NONE &&
      itlRoundingRule != NEAREST)
    throw runtime_error("incorrect/unknown rounding type for international");
  if (domRoundingRule != UP && domRoundingRule != DOWN && domRoundingRule != NONE &&
      domRoundingRule != NEAREST)
    throw runtime_error("incorrect/unknown rounding type for domestic");
  if (taxRoundingRule != UP && taxRoundingRule != DOWN && taxRoundingRule != NONE &&
      taxRoundingRule != NEAREST)
    throw runtime_error("incorrect/unknown rounding type for tax");

  if (currency.empty())
    throw runtime_error("currency not initialized - currency code blank");

  unsigned max_input_nodec = (conversionType == NUC_TO_CURRENCY) ? 2 : currency_nodec;
  // check the currency value nodec
  string to_check = _trx.amountStr();
  // count the decimals after '.'
  string::size_type decim = to_check.find('.');
  if (decim != string::npos)
    if (to_check.size() - decim - 1 > max_input_nodec)
    {
      _trx.setErrorResponse();
      throw WrongDecimalsForCurrency(INVALID_DECIMALS_STRING, logger);
    }

  // check, if decimal part is shorter than 13 digits
  int dec_cnt = 0;
  if (decim == string::npos)
  {
    dec_cnt = to_check.size();
  }
  else
  {
    dec_cnt = decim;
  }

  if (dec_cnt >= 13)
  {
    _trx.setErrorResponse();
    throw WrongDecimalsForCurrency(FIELD_ERROR_STRING, logger);
  }

  validate_historical_date(trxDate);
}
} // tse namespace
