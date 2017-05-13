//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#ifndef FC_UNIT_TEST_BASE
#define FC_UNIT_TEST_BASE

#include <algorithm>
#include <iterator>
#include <list>
#include <locale>
#include <sstream>
#include <string>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <log4cxx/basicconfigurator.h>

//#define DATA_HANDLE_H
// #define DataHandle DataHandle__unit_testing_FC
#include "Currency/FCException.h"
#include "Currency/test/FCDisplayTestMocks.h"
#include "DataModel/CurrencyTrx.h"

//#define private public
#define protected public

#include "Currency/FCDisplay.h"

//#undef private
#undef protected

#include "DataModel/Billing.h"

namespace tse
{

class FCUnitTestBase
{
protected:
  std::string& trim_string(std::string& str)
  {
    std::string::size_type pos1 = str.find_first_not_of(' ');
    std::string::size_type pos2 = str.find_last_not_of(' ');
    if (pos1 == std::string::npos && pos2 == std::string::npos)
    {
      str = "";
    }
    else
    {
      str = str.substr(pos1 == std::string::npos ? 0 : pos1,
                       pos2 == std::string::npos ? str.length() - 1 : pos2 - pos1 + 1);
    }
    return str;
  }

  void assert_response_contains_headers(std::vector<std::string>& lines)
  {
    CPPUNIT_ASSERT(lines.size() >= 4);
    CPPUNIT_ASSERT(lines[0].find("NUC - NEUTRAL UNIT OF CONSTRUCTION RATES LISTED") !=
                   std::string::npos);
    CPPUNIT_ASSERT(lines[1].find("ENTER FCHELP FOR ADDITIONAL FC NUC FORMATS") !=
                   std::string::npos);
    assert_column_names_line(lines[3]);
    //      TODO : column names test !
  }

  void assert_column_names_line(const std::string& colname_line)
  {
    /*CPPUNIT_ASSERT(colname_line.find("CURRENCY") == 0 ) ;
     CPPUNIT_ASSERT(colname_line.find("NUC RATE") == CURRENCY_COLUMN_WIDTH + CARRIER_COLUMN_WIDTH )
     ;
     CPPUNIT_ASSERT(colname_line.find("EFF SALE") == CURRENCY_COLUMN_WIDTH + CARRIER_COLUMN_WIDTH +
     NUCRATE_COLUMN_WIDTH ) ;
     CPPUNIT_ASSERT(colname_line.find("LAST SALE-TKT") == CURRENCY_COLUMN_WIDTH +
     CARRIER_COLUMN_WIDTH + NUCRATE_COLUMN_WIDTH + EFFSALE_COLUMN_WIDTH) ;*/
    // let's not be paranoid
    CPPUNIT_ASSERT(colname_line.find("NUC RATE") != std::string::npos);
    CPPUNIT_ASSERT(colname_line.find("EFF SALE") != std::string::npos);
    CPPUNIT_ASSERT(colname_line.find("LAST SALE-TKT") != std::string::npos);
  }

  void assert_valid_nuc_info_list_for_currency(const std::vector<NUCInfo*>& nuc_list,
                                               const CurrencyCode& curr)
  {
    for (std::vector<NUCInfo*>::const_iterator i = nuc_list.begin(); i != nuc_list.end(); ++i)
    {
      CPPUNIT_ASSERT((*i) != NULL);
      CPPUNIT_ASSERT((*i)->_cur == curr);
      CPPUNIT_ASSERT(nuc_initialized(**i));
    }
    bool sorted = (nuc_list_sorted(nuc_list));
    CPPUNIT_ASSERT_MESSAGE(std::string("vector<NUCInfor*> not sorted"), sorted);
  }

  bool nuc_list_sorted(const std::vector<NUCInfo*>& nuclist)
  {
    if (nuclist.size() <= 1)
      return true;

    std::vector<NUCInfo*>::const_iterator old_item = nuclist.begin();
    std::vector<NUCInfo*>::const_iterator curr_item = nuclist.begin();
    ++curr_item;

    for (; curr_item != nuclist.end(); ++curr_item, ++old_item)
    {
      if (((*curr_item)->_cur) < (*old_item)->_cur)
      {
        return false;
      }

      if ((*curr_item)->_cur == (*old_item)->_cur)
      {
        if ((*curr_item)->_carrier != "")
        {
          if (((*curr_item)->_carrier < (*old_item)->_carrier))
          {
            return false;
          }
          if ((*old_item)->_carrier == "")
          {
            return false;
          }
        }
        if ((*curr_item)->_carrier == (*old_item)->_carrier)
        {
          if ((*curr_item)->effDate() < (*old_item)->effDate())
          {
            return false;
          }
        }
      }
    }
    return true;
  }

  bool nuc_initialized(NUCInfo& nuc)
  {
    return nuc.expireDate() >= nuc.effDate() && nuc.createDate() <= nuc.discDate() &&
           nuc._cur.size() == 3 &&
           (nuc._carrier.size() == 2 || nuc._carrier.size() == 3 || nuc._carrier == "") &&
           nuc._nucFactor >= 0 && nuc._roundingFactor >= 0 && nuc._nucFactorNodec >= 0 &&
           nuc._roundingFactorNodec >= 0;
  }

  bool output_contains(FCCommand& comm, std::string str)
  {
    std::string output = comm._trx.response().rdbuf()->str();
    return output.find(str) != std::string::npos;
  }

  bool line_is_well_formatted_nuc_rate(const std::string& output_line, const CurrencyCode& curr)
  {
    std::vector<std::string> tokens = tokenize(output_line);

    unsigned idx = 0;
    CPPUNIT_ASSERT(curr == tokens[idx++]);

    if (tokens[idx].size() == 2 || tokens[idx].size() == 3)
    {
      ++idx; // not validating carrier string, only it's length
    }
    // assert correct currency string
    CPPUNIT_ASSERT(valid_currency_string(tokens[idx++], 0));
    // assert dates
    if (!assert_valid_date_string(tokens[idx++], 0))
      return false;

    if (tokens.size() > idx) // last date may be empty - +infinity
      if (!assert_valid_date_string(tokens[idx++], 0))
        return false;

    return true;
  }

  bool output_contains_well_formatted_nuc_rate(FCCommand& comm, const CurrencyCode& curr)
  {
    return line_is_well_formatted_nuc_rate(comm._trx.response().rdbuf()->str(), curr);
  }

  bool valid_currency_string(std::string& buff, size_t start_pos)
  {
    // cout << "is valid currency:" << buff << "?\n" ;
    // TODO can the string start with '.', or must there be '0.' ?
    int i = start_pos;
    if (buff[i] == '0') // case of '0 < nuc < 1' - leading 0
    {
      i++;
    }
    else
    {
      if (buff[i] < '1' || buff[i] > '9')
        return false;
      while (buff[i] >= '0' && buff[i] <= '9')
        i++;
      if (i - start_pos > 8)
        return false;
    }
    if (buff[i] != '.')
      return false;
    i++;
    int j = i;
    for (; j < i + 6; j++)
      if (!isnumber(buff[j]))
        return false;
    //      return buff[j] == ' ';
    return true;
  }

  bool isnumber(char c) { return c >= '0' && c <= '9'; }

  bool assert_valid_date_string(std::string& buff, size_t start_pos, unsigned year_dec = 4)
  {
    if (!isnumber(buff[start_pos]) && isnumber(buff[start_pos + 1]))
      return false;
    int day = 10 * (buff[start_pos] - '0') + buff[start_pos + 1] - '0';
    if (day <= 0 || day > 31)
      return false;
    if (!(valid_month_string(buff, start_pos + 2)))
      return false;
    for (unsigned i = start_pos + 5; i < start_pos + 5 + year_dec; i++)
      if (!(isnumber(buff[i])))
      {
        // cout << "NAN: " << buff[i] << '\n' ;
        return false;
      }
    return true;
  }

  bool valid_month_string(std::string& buff, size_t start_pos)
  {
    std::string month = buff.substr(start_pos, 3);
    // cout << "MONTH: " << month << '\n' ;
    return (month == "JAN" || month == "FEB" || month == "MAR" || month == "APR" ||
            month == "MAY" || month == "JUN" || month == "JUL" || month == "AUG" ||
            month == "SEP" || month == "OCT" || month == "NOV" || month == "DEC");
  }

  // used to compress string - replace every sequence of whitespaces into 1 space char
  struct should_not_copy
  {
    should_not_copy(const std::string& seq_) : seq(seq_), cnt(0) {}
    const std::string& seq;
    int cnt;
    bool iswhitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }
    bool operator()(char c)
    {
      return !((!(cnt++)) || (!iswhitespace(c)) || (!iswhitespace(seq[cnt - 2])));
    }
  };

  static bool stringempty(std::string& s) { return s.size() == 0; }

  std::vector<std::string> tokenize(const std::string& s)
  {
    std::vector<std::string> to_ret;
    std::string compressed = compress_string(s);
    split_string(compressed, ' ', to_ret);
    std::vector<std::string>::iterator remIt =
        remove_if(to_ret.begin(), to_ret.end(), ptr_fun(stringempty));

    to_ret.erase(remIt, to_ret.end());

    return to_ret;
  }

  std::string compress_string(const std::string& s)
  {
    std::ostringstream compressed;
    std::ostreambuf_iterator<char> comp_writer(compressed.rdbuf());
    should_not_copy pred(s);
    remove_copy_if(s.begin(), s.end(), comp_writer, pred);
    std::string to_ret = compressed.str();
    for (unsigned i = 0; i < to_ret.size(); i++)
      if (pred.iswhitespace(to_ret[i]))
        to_ret[i] = ' ';
    return to_ret;
  }

  bool output_contains_sequence_compress_spaces(FCCommand& comm, std::string sequence)
  {
    std::string compressed = compress_string(comm._trx.response().rdbuf()->str());
    // cout << "\n'" << comm._trx.response().rdbuf()->str() <<  '\'' << "-->" << '\'' << compressed
    // << "'\n";
    return compressed.find(sequence) != std::string::npos;
  }

  void split_string(const std::string& str, const char delim, std::vector<std::string>& output)
  {
    int index = 0, last_index = -1;
    while (index != (int)std::string::npos)
    {
      index = str.find(delim, last_index + 1);
      if (index == (int)std::string::npos)
        output.push_back(str.substr(last_index + 1, std::string::npos));
      else
        output.push_back(str.substr(last_index + 1, index - last_index - 1));
      last_index = index;
    }
  }

  void init_test_data()
  {
    PLN._cur = "PLN";
    PLN._carrier = "";
    PLN._expireDate = DateTime(2009, 10, 10);
    PLN._effDate = DateTime(2007, 10, 10);
    PLN._nucFactor = 2.79310000;
    PLN._roundingFactor = 1;
    PLN._nucFactorNodec = 8;
    PLN._roundingFactorNodec = 0;
    PLN._roundingRule = UP;

    USD._cur = "USD";
    USD._carrier = "";
    USD._expireDate = DateTime(9999, 12, 31);
    USD._effDate = DateTime(2001, 9, 20);
    USD._nucFactor = 1.00000000;
    USD._roundingFactor = 1;
    USD._nucFactorNodec = 8;
    USD._roundingFactorNodec = 0;
    USD._roundingRule = NONE;

    current_date = DateTime::localTime(); ///(2007,10,16) ;

    pos_inf = DateTime(boost::date_time::pos_infin);
    neg_inf = DateTime(boost::date_time::neg_infin);
    past_date = DateTime(current_date.year() - 1, 1, 1);
    future_date = DateTime(current_date.year() + 1, 1, 1);
  }

  NUCInfo PLN;
  NUCInfo USD;
  NUCInfo NUC;

  DateTime current_date;
  DateTime pos_inf;
  DateTime neg_inf;
  DateTime past_date;
  DateTime future_date;
};

} // namespace tse
#endif
