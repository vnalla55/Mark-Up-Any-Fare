//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#ifndef FC_CONVERT_TEST
#define FC_CONVERT_TEST

#include <cmath>
#include <float.h>

#include "Currency/test/FCUnitTestBase.h"
// it's functions
#include "DataModel/CurrencyTrx.h"

#define private public
#define protected public
#include "Currency/FCConvert.h"
#undef private
#undef protected

#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{

class FCConvertTest : public FCConvert, public CppUnit::TestFixture, public FCUnitTestBase
{
  CPPUNIT_TEST_SUITE(FCConvertTest);
  CPPUNIT_TEST(test_creation);

  CPPUNIT_TEST(test_conversion_to_NUC);
  CPPUNIT_TEST(test_conversion_to_currency);
  CPPUNIT_TEST(test_format_header);
  CPPUNIT_TEST(test_perform_conversion);
  CPPUNIT_TEST(test_rounding);
  CPPUNIT_TEST(test_format_rounding);
  CPPUNIT_TEST(test_format_currency_display);

  CPPUNIT_TEST(test_create_response);

  CPPUNIT_TEST(test_get_truncated_for_rounding);
  CPPUNIT_TEST(test_truncate);

  CPPUNIT_TEST(test_validate_data);

  CPPUNIT_TEST(test_validate_historical_date);

  CPPUNIT_TEST(test_the_test);
  CPPUNIT_TEST_SUITE_END();

public:
  FCConvertTest() : FCConvert(*new CurrencyTrx), FCUnitTestBase()
  {
    /*      log4cxx::Logger::getRootLogger()->removeAllAppenders();
     log4cxx::BasicConfigurator::configure();   */
    init_test_data();
    _trx.transactionStartTime() = current_date;
  }

  void test_creation()
  {
    CPPUNIT_ASSERT(true);
    // if this test suite was created - FCConver was created as base class
  }

  void test_validate_historical_date()
  {
    _trx.transactionStartTime() = DateTime(2007, 10, 10);
    DateTime valid_dates[] = { DateTime(2008, 5, 5), DateTime(2006, 1, 15),
                               DateTime(2005, 10, 10) };

    DateTime invalid_dates[] = { DateTime(2005, 10, 9) };

    for (unsigned i = 0; i < sizeof(valid_dates) / sizeof(valid_dates[0]); i++)
      CPPUNIT_ASSERT_NO_THROW(validate_historical_date(valid_dates[i]));

    for (unsigned i = 0; i < sizeof(invalid_dates) / sizeof(invalid_dates[0]); i++)
      CPPUNIT_ASSERT_THROW(validate_historical_date(invalid_dates[i]),
                           fc_exceptions::WrongDateEntered);

    // test the leap year
    _trx.transactionStartTime() = DateTime(2004, 2, 29);
    CPPUNIT_ASSERT_NO_THROW(validate_historical_date(DateTime(2003, 1, 1)));
  }

  void setup_valid_local_data()
  {
    currency = "PLN";
    currency_nodec = 2;
    sourceVal = 1000;
    nucRate = 2.5;
    currency_nuc = new NUCInfo;
    currency_nuc->_cur = "PLN";
    currency_nuc->_nucFactor = 2.5;
    currency_nuc->_roundingFactor = 0.1;
    currency_nuc->_roundingRule = UP;

    conversionType = CURRENCY_TO_NUC;
    trxDate = DateTime::localTime();

    itlRoundingRule = UP;
    domRoundingRule = UP;
    taxRoundingRule = DOWN;

    itlRoundingFactor = 0.1;
    domRoundingFactor = 0.1;
    taxRoundingFactor = 10;
  }

  void test_validate_data()
  {
    /* part is not testable without DB connection ? */
    setup_valid_local_data();
    sourceVal = -1;
    CPPUNIT_ASSERT_THROW(validate_data(), runtime_error);

    // what if converted value output is bigger than 13 chars ?
    setup_valid_local_data();
    conversionType = NUC_TO_CURRENCY;
    convertedVal = 1234568901234.123;
    CPPUNIT_ASSERT_THROW(create_response(), fc_exceptions::WrongConvertedValue);
    convertedVal = 123456789012.12;
    CPPUNIT_ASSERT_THROW(create_response(), fc_exceptions::WrongConvertedValue);

    convertedVal = 123.33;
    CPPUNIT_ASSERT_NO_THROW(create_response());

    // what if rounding type is (magically?) different than U/D/O/N ?
    // runtime_error
    setup_valid_local_data();
    itlRoundingRule = (RoundingRule)123;
    CPPUNIT_ASSERT_THROW(validate_data(), runtime_error);

    // what if rounding failed (for some unknown reason)
    // runtime_error during response creation;
    setup_valid_local_data();
    conversionType = NUC_TO_CURRENCY;
    itlRoundingFactor = -1.234;
    CPPUNIT_ASSERT_THROW(validate_data(), runtime_error);

    // what if couldn't determine conversion type -> wrong data
    // runtime_error
    // setup_valid_data(); -> not testable without DB connection

    // what if some date is wrong
    // WrongDateEntered : runtime_error
    setup_valid_local_data();
    trxDate = DateTime(DateTime::localTime().year() - 3, 1, 1);
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDateEntered);

    // what if currency decimals in entered value is wrong/different that allowed for Currency?
    setup_valid_local_data();
    conversionType = NUC_TO_CURRENCY;
    // here, default 2 decimals may be used at most
    sourceVal = 1.12;
    _trx.amountStr() = "1.12";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 1.123;
    _trx.amountStr() = "1.123";
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDecimalsForCurrency);

    setup_valid_local_data();
    currency_nodec = 2;
    sourceVal = 10.23;
    _trx.amountStr() = "10.23";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 5.3;
    _trx.amountStr() = "5.3";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 750;
    _trx.amountStr() = "750";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 760.001;
    _trx.amountStr() = "760.001";
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDecimalsForCurrency);

    currency_nodec = 4;

    sourceVal = 10.2312;
    _trx.amountStr() = "10.2312";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 5.3;
    _trx.amountStr() = "5.3";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 75;
    _trx.amountStr() = "75";
    CPPUNIT_ASSERT_NO_THROW(validate_data());

    sourceVal = 750.00001;
    _trx.amountStr() = "750.00001";
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDecimalsForCurrency);

    sourceVal = 0.000009;
    _trx.amountStr() = "0.000009";
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDecimalsForCurrency);

    _trx.amountStr() = "1234567891123.01";
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDecimalsForCurrency);

    _trx.amountStr() = "1234567891123";
    CPPUNIT_ASSERT_THROW(validate_data(), fc_exceptions::WrongDecimalsForCurrency);
  }

  void test_create_response()
  {
    // response for current cur->nuc
    FCConvert fc_conv(*new CurrencyTrx);

    fc_conv.sourceVal = 1.0;
    fc_conv.convertedVal = 1.0;
    fc_conv.nucRate = 1.0;
    fc_conv.conversionType = CURRENCY_TO_NUC;
    fc_conv.currency = "XYX";
    fc_conv.trxDate = current_date;

    fc_conv.currency_nuc = new NUCInfo;
    fc_conv.currency_nuc->_cur = "XYX";
    fc_conv.currency_nuc->_nucFactor = 1.0;

    string res = fc_conv.create_response();

    size_t first_line_end = res.find('\n');

    CPPUNIT_ASSERT(first_line_end != string::npos);

    string header = res.substr(0, first_line_end);
    string the_rest = res.substr(first_line_end + 1);

    test_format_header(header, "XYX", fc_conv.trxDate);

    test_curr_to_nuc_resp_line(the_rest, fc_conv.convertedVal);
  }

  void test_perform_conversion()
  {
    currency = "PLN";
    sourceVal = 1000;
    nucRate = 2.0;
    conversionType = CURRENCY_TO_NUC;

    perform_conversion();
    CPPUNIT_ASSERT(is_equal(convertedVal, 500.0));

    nucRate = 0.5;
    conversionType = CURRENCY_TO_NUC; // should behave just like for current
    perform_conversion();
    CPPUNIT_ASSERT(is_equal(convertedVal, 2000.0));

    nucRate = 4.0;
    sourceVal = 1000;
    conversionType = NUC_TO_CURRENCY; // should behave just like for current date
    perform_conversion();
    CPPUNIT_ASSERT(is_equal(convertedVal, 4000.0));

    nucRate = 0.25;
    conversionType = NUC_TO_CURRENCY;
    perform_conversion();
    CPPUNIT_ASSERT(is_equal(convertedVal, 250));
  }

  void test_format_currency_display()
  {
    double a = 10.1123;

    CPPUNIT_ASSERT_EQUAL(std::string("10.112"), format_currency_display(a, 3));
    CPPUNIT_ASSERT_EQUAL(std::string("10.1123000"), format_currency_display(a, 7));
    CPPUNIT_ASSERT_EQUAL(std::string("10.1123"), format_currency_display(a, 4));

    double b = 0.00525;

    CPPUNIT_ASSERT(format_currency_display(b, 2) != ".00");
    CPPUNIT_ASSERT(format_currency_display(b, 2) == "0.00");
    CPPUNIT_ASSERT(format_currency_display(b, 5) == "0.00525");
    CPPUNIT_ASSERT(format_currency_display(b, 7) == "0.0052500");
  }

  ///////////////////// lowest level - test conversion itself
  void test_conversion_to_NUC()
  {
    double test_rate_1 = 0.001;
    double test_rate_2 = -10.5;
    double test_rate_3 = 15.5;

    double test_quantity_1 = 10.5;
    double test_quantity_2 = -22.34534;
    double test_quantity_3 = 0.0023;

    CPPUNIT_ASSERT_THROW(convert_to_NUC(test_rate_2, test_quantity_1), runtime_error);
    CPPUNIT_ASSERT_THROW(convert_to_NUC(test_rate_1, test_quantity_2), runtime_error);

    CPPUNIT_ASSERT(
        is_equal(convert_to_NUC(test_rate_1, test_quantity_1), test_quantity_1 / test_rate_1));
    CPPUNIT_ASSERT(
        is_equal(convert_to_NUC(test_rate_1, test_quantity_3), test_quantity_3 / test_rate_1));
    CPPUNIT_ASSERT(
        is_equal(convert_to_NUC(test_rate_3, test_quantity_1), test_quantity_1 / test_rate_3));
    CPPUNIT_ASSERT(
        is_equal(convert_to_NUC(test_rate_3, test_quantity_3), test_quantity_3 / test_rate_3));
  }

  void test_conversion_to_currency()
  {
    double test_rate_1 = 0.00456;
    double test_rate_2 = 134.3453;

    double test_nuc_quantity_1 = 0.045;
    double test_nuc_quantity_2 = 126.347;

    double test_nuc_quant_bad = -0.123;
    double test_nuc_rate_bad = -1.34;

    CPPUNIT_ASSERT_THROW(convert_to_currency(test_rate_1, test_nuc_quant_bad), runtime_error);
    CPPUNIT_ASSERT_THROW(convert_to_currency(test_nuc_rate_bad, test_nuc_quantity_2),
                         runtime_error);

    CPPUNIT_ASSERT(is_equal(convert_to_currency(test_rate_1, test_nuc_quantity_1),
                            test_nuc_quantity_1 * test_rate_1));
    CPPUNIT_ASSERT(is_equal(convert_to_currency(test_rate_1, test_nuc_quantity_2),
                            test_nuc_quantity_2 * test_rate_1));
    CPPUNIT_ASSERT(is_equal(convert_to_currency(test_rate_2, test_nuc_quantity_1),
                            test_nuc_quantity_1 * test_rate_2));
    CPPUNIT_ASSERT(is_equal(convert_to_currency(test_rate_2, test_nuc_quantity_2),
                            test_nuc_quantity_2 * test_rate_2));
  }

  void test_truncate()
  {
    double test_set[][3] = { //  value   no_dec    expected_result
                             { 1.234, 2.1, 1.23 },
                             { 1234, 0.1, 1234 },
                             { 1.234, 3.1, 1.234 },
                             { 1234.5, 2.1, 1234.5 } };

    for (unsigned i = 0; i < sizeof(test_set) / sizeof(test_set[0]); i++)
    {
      CPPUNIT_ASSERT_MESSAGE(
          string("test_truncate: failed for val=") + boost::lexical_cast<string>(test_set[i][0]) +
              ",no_dec=" + boost::lexical_cast<string>(test_set[i][1]) + "; expected=" +
              boost::lexical_cast<string>(test_set[i][2]) + ",actual=" +
              boost::lexical_cast<string>(truncate(test_set[i][0], (unsigned)test_set[i][1])),
          is_equal(truncate(test_set[i][0], (unsigned)(test_set[i][1])), test_set[i][2]));
    }
  }

  /////////////////// test format of response header
  void test_format_header()
  {
    // test the test itself
    test_format_header("RATE 1NUC -  40.704000 PLN / 23JUL07", "PLN", DateTime(2007, 7, 23));

    // test the function output
    test_format_header(format_header(PLN, current_date), "PLN", current_date);
    test_format_header(format_header(USD, past_date), "USD", past_date);
    test_format_header(format_header(USD, pos_inf), "USD", DateTime::localTime());

    CPPUNIT_ASSERT_THROW(format_header(USD, neg_inf), runtime_error);
  }

  void test_get_truncated_for_rounding()
  {
    double test_set[][3] = { //  value   rounding_factor expected_result  currency_nodec
                             { 1.234, 0.1, 1.23 },
                             { 1234, 100, 1234 },
                             { 1.234, 0.01, 1.234 },
                             { 1.234, 0.0001, 1.234 },
                             { 1234.5, 100, 1234.5 } };

    unsigned trf_nodec;
    for (unsigned i = 0; i < sizeof(test_set) / sizeof(test_set[0]); i++)
    {
      CPPUNIT_ASSERT_MESSAGE(
          string("test_get_truncated_for_rounding: failed for val=") +
              boost::lexical_cast<string>(test_set[i][0]) + ",fact=" +
              boost::lexical_cast<string>(test_set[i][1]) + "; expected=" +
              boost::lexical_cast<string>(test_set[i][2]) + ",actual=" +
              boost::lexical_cast<string>(get_truncated_for_rounding(
                  test_set[i][0], (unsigned)(test_set[i][1]), trf_nodec)),
          is_equal(get_truncated_for_rounding(test_set[i][0], test_set[i][1], trf_nodec),
                   test_set[i][2]));
    }
  }

  /////////////////// test the rounding routine
  void test_rounding()
  {
    const double RUP = -0.200;
    const double RDOWN = -2.200;
    const double RNEAREST = -22.200;

    // removing 'NONE' roundings from test set - as
    // they coredump trying to access DB data via DataHandle
    double test_set[][4] = {
      // value      rounding_factor	  rounding_rule	    expected_output
      { 0.59, 0.1, RUP, 0.6 },
      { 0.59, 0.1, RDOWN, 0.5 },
      { 0.59, 0.1, RNEAREST, 0.6 },
      { 150.592, 0.01, RUP, 150.6 },
      { 0.4229, 0.001, RDOWN, 0.422 },
      { 0.12, 1, RNEAREST, 0 },
      { 0.72, 1, RNEAREST, 1 },
      { 740, 50, RUP, 750 },
      { 99.9, 0.5, RDOWN, 99.5 },
      { 999, 100, RUP, 1000 },
      //	{   900.001,	100,		    RUP,	      1000 },	----> error in CurrencyConverter
      { 120.32, 0.1, RDOWN, 120.3 },
      { 0.009, 0.1, RNEAREST, 0 }
    };

    ostringstream oss;
    for (unsigned i = 0; i < sizeof(test_set) / sizeof(test_set[0]); i++)
    {
      RoundingRule rr =
          test_set[i][2] == RUP ? UP : test_set[i][2] == RDOWN ? DOWN : test_set[i][2] == RNEAREST
                                                                            ? NEAREST
                                                                            : NONE;
      // in this test - for rounding NONE - always apply nodec = 1 !
      double converted = apply_rounding(test_set[i][0], test_set[i][1], rr, 1);

      oss << "test set #" << i;
      CPPUNIT_ASSERT_MESSAGE(oss.str(), is_equal(converted, test_set[i][3]));
      oss.str("");
    }
  }

  ////////// test the rounded value's format
  void test_format_rounding()
  {
    // 200PLN = 79.483042292926 ;
    double converted_value = 79.483042292926;

    double rounding_factor_100 = 100;
    double rounding_factor_5 = 5;
    double rounding_factor_1 = 1;
    double rounding_factor_005 = 0.05;
    // double rounding_factor_0001 = 0.001 ;

    string res;

    res = format_rounding("PLN", converted_value, rounding_factor_100, UP, 2, RT_INTERNATIONAL);
    rounding_fine(res, rounding_factor_100, UP, 2, RT_INTERNATIONAL);

    res = format_rounding("PLN", converted_value, rounding_factor_5, UP, 2, RT_INTERNATIONAL);
    rounding_fine(res, rounding_factor_5, UP, 2, RT_INTERNATIONAL);

    res = format_rounding("PLN", converted_value, rounding_factor_1, DOWN, 2, RT_DOMESTIC);
    rounding_fine(res, rounding_factor_1, DOWN, 2, RT_DOMESTIC);

    res = format_rounding("PLN", converted_value, rounding_factor_005, NEAREST, 2, RT_OTHER_TAXES);
    rounding_fine(res, rounding_factor_005, NEAREST, 2, RT_OTHER_TAXES);

    /*      res = format_rounding( "PLN" , converted_value , 0 , NONE , 2 , RT_INTERNATIONAL );
     cout << ">>>>>>>>>>>-------> " << res << "\n" ;
     CPPUNIT_ASSERT(rounding_fine(res,0,NONE,2,RT_INTERNATIONAL )) ; TODO !!! */
  }

  void test_process()
  {
    test_convert_from_nuc(false);
    test_convert_from_nuc(true);
    test_convert_from_curr(false);
    test_convert_from_curr(true);
  }

  void test_the_test()
  {
    CPPUNIT_ASSERT(correct_truncated_nuc_format("10.5", 1.0));
    CPPUNIT_ASSERT(correct_truncated_nuc_format("40.0", 5.0));
    CPPUNIT_ASSERT(correct_truncated_nuc_format("0.008", 0.01));
    CPPUNIT_ASSERT(correct_truncated_nuc_format("1030.0", 100.0));
    CPPUNIT_ASSERT(correct_truncated_nuc_format("10300.0", 1000.0));
    CPPUNIT_ASSERT(correct_truncated_nuc_format("12.0", 5.0));
    CPPUNIT_ASSERT(correct_truncated_nuc_format("103.0", 10.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("10", 1.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("40.79", 5.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("0.0008", 0.01));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("103", 100.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("103", 10.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("10330", 1000.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("10303", 1000.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("10300.05", 1000.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("12.02", 5.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("1", 5.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("103.15", 10.0));
    CPPUNIT_ASSERT(!correct_truncated_nuc_format("103.0001", 10.0));
    CPPUNIT_ASSERT(correct_rounded_value_format("10.5", 0.1));
    CPPUNIT_ASSERT(correct_rounded_value_format("40.0", 0.5));
    CPPUNIT_ASSERT(correct_rounded_value_format("0.008", 0.001));
    CPPUNIT_ASSERT(correct_rounded_value_format("1030", 10.0));
    CPPUNIT_ASSERT(correct_rounded_value_format("10300", 100.0));
    CPPUNIT_ASSERT(correct_rounded_value_format("12.0", 0.5));
    CPPUNIT_ASSERT(correct_rounded_value_format("103", 1.0));
    CPPUNIT_ASSERT(!correct_rounded_value_format("10", 0.1));
    CPPUNIT_ASSERT(!correct_rounded_value_format("40.79", 0.5));
    CPPUNIT_ASSERT(!correct_rounded_value_format("0.0008", 0.001));
    CPPUNIT_ASSERT(!correct_rounded_value_format("103", 10.0));
    CPPUNIT_ASSERT(!correct_rounded_value_format("10330", 100.0));
    CPPUNIT_ASSERT(!correct_rounded_value_format("10303", 100.0));
    CPPUNIT_ASSERT(!correct_rounded_value_format("10300.0", 100.0));
    CPPUNIT_ASSERT(!correct_rounded_value_format("12.02", 0.5));
    CPPUNIT_ASSERT(!correct_rounded_value_format("1", 0.5));
    CPPUNIT_ASSERT(!correct_rounded_value_format("103.1", 1.0));
    CPPUNIT_ASSERT(!correct_rounded_value_format("103.0001", 1.0));
  }

protected:
  void test_curr_to_nuc_resp_line(const string& line, double expected_val)
  {
    vector<string> tokens = tokenize(line);

    CPPUNIT_ASSERT_MESSAGE("wrong token count", tokens.size() >= 3);
    CPPUNIT_ASSERT_EQUAL(std::string("NUC"), tokens[0]);

    double val = strtod(tokens[1].c_str(), NULL);
    CPPUNIT_ASSERT(!(val <= 0 || !is_equal(expected_val, val)));

    CPPUNIT_ASSERT_EQUAL(std::string("TRUNCATED"), tokens[2]);
  }

  void test_format_header(const string& header, const string& currency, const DateTime& dte)
  {
    //             RATE 1NUC -  40.712345 AAA / 18SEP07
    vector<string> tokens = tokenize(header);
    int idx = 0;

    CPPUNIT_ASSERT_EQUAL(std::string("RATE"), tokens[idx++]);
    CPPUNIT_ASSERT_EQUAL(std::string("1NUC"), tokens[idx++]);
    CPPUNIT_ASSERT_EQUAL(std::string("-"), tokens[idx++]);

    double val = strtod(tokens[idx++].c_str(), NULL);
    CPPUNIT_ASSERT(val > 0);

    CPPUNIT_ASSERT_EQUAL(currency, tokens[idx++]);
    CPPUNIT_ASSERT_EQUAL(std::string("/"), tokens[idx++]);

    if (!dte.isPosInfinity())
    {
      CPPUNIT_ASSERT_MESSAGE(std::string("invalid date string:" + tokens[idx]),
                             assert_valid_date_string(const_cast<string&>(tokens[idx]), 0, 2));
      CPPUNIT_ASSERT_EQUAL(dte.dateToString(DDMMMYY, NULL), tokens[idx]);
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(DateTime::localTime().dateToString(DDMMMYY, NULL), tokens[idx]);
    }
  }

  void
  rounding_fine(string& res, double factor, RoundingRule rule, unsigned curr_dec, RoundingType type)
  {
    string last_part =
        get_rounding_rule_string(rule, factor) + string(" - ") + get_rounding_type_string(type);
    test_format_single_rounding(res, factor, last_part, curr_dec);
  }

  void test_format_single_rounding(const string& output,
                                   double rounding_factor,
                                   const string& last_part,
                                   unsigned curr_decimals)
  {
    vector<string> parts = tokenize(output);
    // INR        8140.8     TRUNCATED
    // INR        8145       ROUNDED UP TO NEXT  5 - OTHER/TAXES

    int idx = 0;
    // currency code
    CPPUNIT_ASSERT_EQUAL(size_t(3), parts[idx++].size());

    // truncated value, checking only decimals count
    CPPUNIT_ASSERT(correct_truncated_nuc_format(parts[idx++], rounding_factor));
    CPPUNIT_ASSERT_EQUAL(TRUNCATED_INFO_STRING, parts[idx++]);
    // currency code again, must be the same
    CPPUNIT_ASSERT_EQUAL(parts[0], parts[idx++]);
    // rounded value, checking only decimals count
    CPPUNIT_ASSERT(correct_rounded_currency_format(parts[idx++], curr_decimals));
    // last part - from "ROUNDED ..." - just passed as string
    vector<string> last_part_parts = tokenize(last_part);
    for (unsigned i = 0; i < last_part_parts.size(); i++)
      CPPUNIT_ASSERT(!((parts.size() < i + idx - 1) || last_part_parts[i] != parts[i + idx]));
  }

  void test_convert_from_nuc(bool past_date)
  {
    // no idea, how to do that without MockDataHandle and creating mocks by re#define's
    process();
  }

  void test_convert_from_curr(bool past_date)
  {
    // no idea, how to do that without MockDataHandle and creating mocks by re#define's
    process();
  }

  bool valid_double(const string& s)
  {
    for (unsigned i = 0; i < s.size(); i++)
      if (!::isdigit(s[i]) && s[i] != '.')
        return false;
    return true;
  }

  bool correct_rounded_currency_format(const string& val, unsigned nodec)
  {
    if (!valid_double(val))
      return false;
    if (nodec == 0)
    {
      return (val.find('.') == string::npos);
    }
    if (val.find('.') == string::npos)
      return false;
    return (val.size() - val.find('.') - 1 == nodec);
  }

  bool correct_rounded_value_format(const string& val, double factor)
  {
    return correct_modified_curr_format(val, factor, false);
  }

  bool correct_truncated_nuc_format(const string& nuc, double factor)
  {
    return correct_modified_curr_format(nuc, factor, true);
  }

  bool correct_modified_curr_format(const string& nuc, double factor, bool nuc_truncation)
  {
    double log = log10(factor);
    double significant = (log >= -0.000001 ? ceil(log + 0.999) : ::floor(log));

    if (nuc_truncation)
    {

      if (significant >= ::ceil(1.0))
      {
        // for NUC truncation, always leave at least 1 digit after the decimal point
        significant = -1.0;
      }
      else
      {
        significant = ::floor(significant - 0.9);
      }
    }
    else
    {
    }

    if (significant < 0)
    {
      // assert, that there are enough decimals (and '.')
      string::size_type dot = nuc.find('.');
      if (dot == string::npos)
        return false;
      int expected_decimals = int(-significant);
      string::size_type i = 0;
      for (i = dot + 1; i < dot + 1 + expected_decimals; i++)
        if (i >= nuc.size() || !::isdigit(nuc[i]))
          return false;
      return (i == nuc.size());
    }
    else if (significant == 0)
    {
      throw new runtime_error("significant must not be 0 - error in unit test");
    }
    else
    {
      if (nuc_truncation)
      {
        // for NUC truncation, always leave 1 digit after the decimal point
        // even for rounding > 1
        // this should be determined earlier, shouldn't get here
        throw new runtime_error("shouldn't get here!");
      }
      else
      {
        // assert, that there is no '.', and enough zeroes
        if (nuc.find('.') != string::npos)
          return false;
        int expected_zeroes = int(significant - 1);
        int i;
        for (i = 0; i < expected_zeroes; i++)
          if (nuc[nuc.size() - 1 - i] != '0')
            return false;
        return true;
      }
    }
  }

  static const double sigma;
  bool is_equal(double a, double b)
  {
    if (a < b)
      return ((b - a) < sigma);
    else
      return (a - b) < sigma;
  }
};

const double FCConvertTest::sigma = 0.000001;

CPPUNIT_TEST_SUITE_REGISTRATION(FCConvertTest);

} // namespace tse
#endif
