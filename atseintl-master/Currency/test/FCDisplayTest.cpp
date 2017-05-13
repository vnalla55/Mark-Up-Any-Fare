//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Currency/test/FCUnitTestBase.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

using namespace std;

namespace tse
{
class FCDisplayTest : public CppUnit::TestFixture, public FCDisplay, public FCUnitTestBase
{
  CPPUNIT_TEST_SUITE(FCDisplayTest);

  CPPUNIT_TEST(test_the_test);

  CPPUNIT_TEST(test_creation);
  CPPUNIT_TEST(test_single_line);

  CPPUNIT_TEST(test_format_nuc_rate);
  CPPUNIT_TEST(test_format_date);
  CPPUNIT_TEST(test_show_column_names);
  CPPUNIT_TEST(test_sorting);
  CPPUNIT_TEST(test_determine_ranges);

  CPPUNIT_SKIP_TEST(test_process);

  CPPUNIT_TEST_SUITE_END();

public:
  FCDisplayTest() : FCDisplay(*_localTrx) { init_test_data(); }

  void test_creation()
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    get_fcdisplay(trx.get());
  }

  void test_sorting()
  {
    vector<NUCInfo*>* to_sort = NUCTestData::get_all_nuc_info();

    CPPUNIT_ASSERT(!nuc_list_sorted(*to_sort));
    sort_nuc_list(*to_sort);
    /*for (int i=0;i<to_sort->size();i++)
     {
     cout << (*to_sort)[i]->_cur << "," << (*to_sort)[i]->_carrier << "," <<
     (*to_sort)[i]->effDate() << "!\n";
     }	*/
    CPPUNIT_ASSERT(nuc_list_sorted(*to_sort));
  }

  void test_determine_ranges()
  {
    CurrencyCode valid = "PLN";
    CurrencyCode empty = "";

    string fcstar = "FC*";
    string fcstarstar = "FC**";
    string invalid_action_code = "INVALID!";

    CPPUNIT_ASSERT(test_determine_ranges(pos_inf, valid, fcstar, CURRENT, DISPLAY_SINGLE));
    CPPUNIT_ASSERT(test_determine_ranges(pos_inf, empty, fcstar, CURRENT, DISPLAY_ALL));
    CPPUNIT_ASSERT(test_determine_ranges(
        neg_inf, valid, fcstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        neg_inf, empty, fcstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(
        test_determine_ranges(current_date, valid, fcstar, FROM_DATE_TO_INFINITY, DISPLAY_SINGLE));
    CPPUNIT_ASSERT(
        test_determine_ranges(current_date, empty, fcstar, FROM_DATE_TO_INFINITY, DISPLAY_ALL));
    CPPUNIT_ASSERT(
        test_determine_ranges(past_date, valid, fcstar, FROM_DATE_TO_INFINITY, DISPLAY_SINGLE));
    CPPUNIT_ASSERT(
        test_determine_ranges(past_date, empty, fcstar, FROM_DATE_TO_INFINITY, DISPLAY_ALL));
    CPPUNIT_ASSERT(
        test_determine_ranges(future_date, valid, fcstar, FROM_DATE_TO_INFINITY, DISPLAY_SINGLE));
    CPPUNIT_ASSERT(
        test_determine_ranges(future_date, empty, fcstar, FROM_DATE_TO_INFINITY, DISPLAY_ALL));

    CPPUNIT_ASSERT(
        test_determine_ranges(pos_inf, valid, fcstarstar, CURRENT_TO_INFINITY, DISPLAY_SINGLE));
    CPPUNIT_ASSERT(
        test_determine_ranges(pos_inf, empty, fcstarstar, CURRENT_TO_INFINITY, DISPLAY_ALL));
    CPPUNIT_ASSERT(test_determine_ranges(
        neg_inf, valid, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        neg_inf, empty, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        current_date, valid, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        current_date, empty, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        past_date, valid, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        past_date, empty, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        future_date, valid, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(
        future_date, empty, fcstarstar, INVALID_DATE_RANGE, UNSPECIFIED_CURRENCIES_RANGE));
    CPPUNIT_ASSERT(test_determine_ranges(future_date,
                                         empty,
                                         invalid_action_code,
                                         UNSPECIFIED_DATE_RANGE,
                                         UNSPECIFIED_CURRENCIES_RANGE));
  }

  bool test_determine_ranges(DateTime dt,
                             CurrencyCode& curr,
                             string action_code,
                             DateRange exp_range,
                             CurrenciesRange exp_curr)
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);

    trx->baseDT() = dt;
    trx->sourceCurrency() = curr;
    std::unique_ptr<Billing> bil(new Billing);
    trx->billing() = bil.get();
    trx->billing()->actionCode().insert(0, action_code);

    FCDisplay displ = get_fcdisplay(trx.get());

    DateRange date_range;
    CurrenciesRange curr_range;

    if (exp_range == INVALID_DATE_RANGE || exp_curr == INVALID_CURRENCIES_RANGE ||
        exp_range == UNSPECIFIED_DATE_RANGE || exp_curr == UNSPECIFIED_CURRENCIES_RANGE)
    {
      bool thrown = false;
      try { displ.determine_ranges(date_range, curr_range); }
      catch (...) { thrown = true; }
      return thrown;
    }
    else
    {
      displ.determine_ranges(date_range, curr_range);
      CPPUNIT_ASSERT_EQUAL(exp_range, date_range);
      CPPUNIT_ASSERT_EQUAL(exp_curr, curr_range);
      return true;
    }
  }

  void test_show_column_names()
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    FCDisplay& fc_display = get_fcdisplay(trx.get());
    fc_display.show_column_names();
    assert_column_names_line(fc_display._trx.response().str());
  }

  void test_format_nuc(const NUCInfo& NUC, const string& expected)
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    FCDisplay& fc_display = get_fcdisplay(trx.get());
    string s = fc_display.format_nuc_rate(NUC, false);
    CPPUNIT_ASSERT_EQUAL(expected, trim_string(s));
  }

  void test_format_nuc(double nucFactor, unsigned nucFactorNodec, const string& expected)
  {
    NUCInfo tst_nuc;
    tst_nuc._nucFactor = nucFactor;
    tst_nuc._nucFactorNodec = nucFactorNodec;
    test_format_nuc(tst_nuc, expected);
  }

  void test_format_nuc_rate()
  {
    test_format_nuc(PLN, "2.793100");
    test_format_nuc(USD, "1.000000");

    test_format_nuc(1.00000001, 8, "1.000000");
    test_format_nuc(1, 0, "1.000000");
    test_format_nuc(0.0123456789, 10, "0.012345");
    test_format_nuc(0.100, 3, "0.100000");
    test_format_nuc(1234.5, 1, "1234.500000");
    test_format_nuc(100000000, 0, NUC_TOO_HIGH_VALUE);
  }

  void test_format_date(int y, int m, int d, const string& expected)
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    FCDisplay& fc_display = get_fcdisplay(trx.get());
    DateTime test_date = DateTime(y, m, d);
    string date_string = fc_display.format_date(test_date);

    if (expected.empty())
      for (unsigned i = 0; i < date_string.size(); i++)
        CPPUNIT_ASSERT(date_string[i] == ' ');
    else
      CPPUNIT_ASSERT_EQUAL(expected, date_string);
  }

  void test_format_date()
  {
    test_format_date(2004, 1, 1, "01JAN2004");
    test_format_date(2007, 10, 10, "10OCT2007");
    test_format_date(1990, 12, 31, "31DEC1990");
    test_format_date(9999, 12, 31, "");
  }

  void test_single_line()
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    FCDisplay& fc_display = get_fcdisplay(trx.get());
    fc_display.append_output_line(PLN);
    // cout << '\n' << fc_display._trx.response().str() << '\n' ;
    CPPUNIT_ASSERT(output_contains_well_formatted_nuc_rate(fc_display, "PLN"));
  }

  void test_process()
  {
    test_get_nuc_info_now_into_future();
    test_get_nuc_info_past_into_future();
    test_get_nuc_info_future_into_future();
    test_get_multiple_nuc_info_now_into_future();
    test_get_multiple_nuc_info_past_into_future();
    test_get_multiple_nuc_info_future_into_future();
    test_get_current_nuc();
    test_get_multiple_current_nuc();
  }

  void test_get_nuc_info_now_into_future() { process(current_date, "PLN", "FC**"); }

  void test_get_nuc_info_past_into_future() { process(past_date, "PLN", "FC**"); }

  void test_get_nuc_info_future_into_future() { process(future_date, "PLN", "FC**"); }

  void test_get_multiple_nuc_info_now_into_future() { process(current_date, "", "FC*"); }

  void test_get_multiple_nuc_info_past_into_future() { process(past_date, "", "FC*"); }

  void test_get_multiple_nuc_info_future_into_future() { process(future_date, "", "FC*"); }

  void test_get_current_nuc() { process(pos_inf, "PLN", "FC*"); }

  void test_get_multiple_current_nuc() { process(pos_inf, "PLN", "FC*"); }

  void process(const DateTime& dte, const CurrencyCode& curr, const string& action_code)
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    trx->baseDT() = dte;
    trx->sourceCurrency() = curr;
    std::unique_ptr<Billing> bil(new Billing);
    trx->billing() = bil.get();
    trx->billing()->actionCode().insert(0, action_code);

    FCDisplay displ = get_fcdisplay(trx.get());

    displ.process();
  }

  void test_output_contains_sequence_compress_spaces(const string& seq, const string& expected)
  {
    std::unique_ptr<CurrencyTrx> trx(new CurrencyTrx);
    FCDisplay& fc_display = get_fcdisplay(trx.get());
    fc_display._trx.response() << seq;
    CPPUNIT_ASSERT(output_contains_sequence_compress_spaces(fc_display, expected));
  }

  void test_the_test()
  {
    // added, to test some of the functions used in the unit test
    test_output_contains_sequence_compress_spaces("a   b c     d e  f     g  h", "a b c d e f g h");
    test_output_contains_sequence_compress_spaces("a   b c     d e  f     g \n h",
                                                  "a b c d e f g h");
    test_output_contains_sequence_compress_spaces("a   b c     d\t\te  f     g\n h",
                                                  "a b c d e f g h");
    test_output_contains_sequence_compress_spaces("a   b c     de  f     g\n h", "a b c de f g h");
    test_output_contains_sequence_compress_spaces("   \n  a   b c     de  f     g\n h",
                                                  " a b c de f g h");
  }

protected:
  FCDisplay& get_fcdisplay(CurrencyTrx* _trx)
  {
    _trx->transactionStartTime() = current_date;
    return _trx->dataHandle().safe_create<FCDisplay>(*_trx);
  }

private:
  std::unique_ptr<CurrencyTrx> _localTrx{new CurrencyTrx};
};
CPPUNIT_TEST_SUITE_REGISTRATION(FCDisplayTest);
} // namespace tse
