// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"

#include "Taxes/Pfc/PfcDisplayBuilderPXC.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXE.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXM.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXA.h"
#include "Taxes/Pfc/PfcDisplayBuilderPXH.h"

#include "Taxes/Pfc/PfcDisplayErrorMsg.h"

#include "Common/DateTime.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace tse
{
class PfcDisplayFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PfcDisplayFormatterTest);
  CPPUNIT_TEST(newInstanceTest);
  CPPUNIT_TEST(stringAdjustmentTest);
  CPPUNIT_TEST(toStringTest);
  CPPUNIT_TEST(vectorToStringTest);
  CPPUNIT_TEST(noteCounterTest);
  CPPUNIT_TEST(pfcAmountTest);
  CPPUNIT_TEST(pxcLineTest);
  CPPUNIT_TEST(pxcWarningLineTest);
  CPPUNIT_TEST(pxcFlightTest);
  CPPUNIT_TEST(pxeLineTest);
  CPPUNIT_TEST(pxa5colLineTest);
  CPPUNIT_TEST(pxa4colLineTest);
  CPPUNIT_TEST(pxa3colLineTest);
  CPPUNIT_TEST(pxaFareTest);
  CPPUNIT_TEST(pxaTaxTest);
  CPPUNIT_TEST(pxaItineraryTest);
  CPPUNIT_TEST(pxhLineTest);
  CPPUNIT_TEST(pxhDateTimeTest);
  CPPUNIT_TEST(pxmLineTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void newInstanceTest()
  {
    std::unique_ptr<PfcDisplayFormatter> ptr(new PfcDisplayFormatter());
    CPPUNIT_ASSERT(ptr);
  }

  void stringAdjustmentTest()
  {
    std::string c1 = "Center";
    std::string c2 = "PASSENGER FACILITY CHARGES";
    std::string l1 = "Left";
    std::string l2 = "PASSENGER FACILITY CHARGES";
    std::string r1 = "Right";
    std::string r2 = "PASSENGER FACILITY CHARGES";

    PfcDisplayFormatter fmt;

    std::string s1c = fmt.center(c1);
    std::string s2c = fmt.center(c2);
    std::string s1l = fmt.left(l1);
    std::string s2l = fmt.left(l2);
    std::string s1r = fmt.right(r1);
    std::string s2r = fmt.right(r2);

    CPPUNIT_ASSERT(c1 == s1c.substr(28, c1.size()) && c2 == s2c.substr(18, c2.size()) &&
                   l1 == s1l.substr(0, l1.size()) && l2 == s2l.substr(0, l2.size()) &&
                   r1 == s1r.substr(58, r1.size()) && r2 == s2r.substr(37, r2.size()));
  }

  void toStringTest()
  {
    int32_t i = -9999;
    uint32_t ui = 10;
    double d = 10.12;

    std::string i2s = "-9999";
    std::string ui2s = "10";
    std::string d2s = "10.12";

    PfcDisplayFormatter fmt;

    CPPUNIT_ASSERT_EQUAL(fmt.toString(i), i2s);
    CPPUNIT_ASSERT_EQUAL(fmt.toString(ui), ui2s);
    CPPUNIT_ASSERT_EQUAL(fmt.toString(d), d2s);
  }

  void vectorToStringTest()
  {
    std::vector<std::string> v;
    v.push_back("#");
    v.push_back("$");
    v.push_back("*");

    PfcDisplayFormatter fmt;

    CPPUNIT_ASSERT(fmt.toString(v) == "#$*");
  }

  void noteCounterTest()
  {
    PfcDisplayFormatterPXC fmt;

    CPPUNIT_ASSERT(fmt.counter(1) == "001" && fmt.counter(12) == "012" &&
                   fmt.counter(123) == "123");
  }

  void pfcAmountTest()
  {
    PfcDisplayFormatterPXC fmt;

    CPPUNIT_ASSERT(fmt.pfcAmount(4.5) == "USD 4.50");
  }

  void pxcFlightTest()
  {
    PfcDisplayFormatterPXC fmt;

    CPPUNIT_ASSERT(fmt.flight(31) == "  31");
  }

  void pxcLineTest()
  {
    std::string header = PfcDisplayBuilderPXC::TABLE_HEADER;

    std::string arpt = "ABE$*/-#";
    std::string effDate = "24MAR07";
    std::string discDate = "01AUG18";
    std::string amount = "USD 4.50";
    std::string equivAmt = "10.30";
    std::string note = "001";

    std::string arpt2 = "YUM$";
    std::string effDate2 = "01NOV07";
    std::string discDate2 = "01JAN13";
    std::string amount2 = "USD 40.50";
    std::string equivAmt2 = "100.30";
    std::string note2 = "";

    PfcDisplayFormatterPXC fmt(header);

    std::string line = fmt.line(arpt, effDate, discDate, amount, equivAmt, note);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arpt) == std::string::npos || line.find(effDate) == std::string::npos ||
        line.find(discDate) == std::string::npos || line.find(amount) == std::string::npos ||
        line.find(equivAmt) == std::string::npos || line.find(note) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }

    arpt = "YUM$";
    effDate = "01NOV07";
    discDate = "01JAN13";
    amount = "USD 40.50";
    equivAmt = "100.30";
    note = "";

    line = fmt.line(arpt, effDate, discDate, amount, equivAmt, note);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arpt) == std::string::npos || line.find(effDate) == std::string::npos ||
        line.find(discDate) == std::string::npos || line.find(amount) == std::string::npos ||
        line.find(equivAmt) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxcWarningLineTest()
  {
    PfcDisplayFormatterPXC fmt;

    std::string arpt = "DFW";
    std::string warning = PfcDisplayErrorMsg::PFC_NOT_APPLICABLE;

    std::string line = fmt.warningLine(arpt, warning);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arpt) == std::string::npos || line.find(warning) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxeLineTest()
  {
    std::string header = PfcDisplayBuilderPXE::TABLE_HEADER;

    std::string from = "ABQ";
    std::string to = "ALM";
    std::string cxr = "ZV";
    std::string fltRange = "ALL";
    std::string effDate = "14SEP07";
    std::string discDate = "NONE";

    PfcDisplayFormatterPXE fmt(header);

    std::string line = fmt.line(from, to, cxr, fltRange, effDate, discDate);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(from) == std::string::npos || line.find(to) == std::string::npos ||
        line.find(cxr) == std::string::npos || line.find(fltRange) == std::string::npos ||
        line.find(effDate) == std::string::npos || line.find(discDate) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxa5colLineTest()
  {
    std::string header = PfcDisplayBuilderPXA_generalInfo::TABLE_HEADER;

    std::string nbr = "1";
    std::string arpt = "ABE";
    std::string effDate = "01NOV07";
    std::string discDate = "01AUG18";
    std::string carrier = "NW";

    PfcDisplayFormatterPXA fmt(header);

    std::string line = fmt.line(nbr, arpt, effDate, discDate, carrier);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(nbr) == std::string::npos || line.find(arpt) == std::string::npos ||
        line.find(effDate) == std::string::npos || line.find(discDate) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxa4colLineTest()
  {
    std::string header = PfcDisplayBuilderPXA_detailInfo::TABLE_HEADER2;

    std::string carrier = "NW";
    std::string arpt = "ABE";
    std::string effDate = "01NOV07";
    std::string discDate = "01AUG18";

    PfcDisplayFormatterPXA fmt(header);

    std::string line = fmt.line(carrier, arpt, effDate, discDate);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(carrier) == std::string::npos || line.find(arpt) == std::string::npos ||
        line.find(effDate) == std::string::npos || line.find(discDate) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxa3colLineTest()
  {
    std::string header = PfcDisplayBuilderPXA_detailInfo::SEQ_LINE1;

    std::string arptCites = "";
    std::string rule = "2640";
    std::string fareTariff = "191";

    PfcDisplayFormatterPXA fmt(header);

    std::string line = fmt.line(arptCites, rule, fareTariff);

    if (line.find(rule) == std::string::npos || line.find(fareTariff) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }

    arptCites = "LAS-MEM";
    rule = "5002";
    fareTariff = "117";

    line = fmt.line(arptCites, rule, fareTariff);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arptCites) == std::string::npos || line.find(rule) == std::string::npos ||
        line.find(fareTariff) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxaFareTest()
  {
    MoneyAmount amount1 = 4.13;

    PfcDisplayFormatterPXA fmt;

    std::string amountExpected1 = "4.13-";

    CPPUNIT_ASSERT(fmt.fare(amount1) == amountExpected1);
  }

  void pxaTaxTest()
  {
    MoneyAmount amount1 = 0.37;

    PfcDisplayFormatterPXA fmt;

    std::string amountExpected1 = "0.37-";

    CPPUNIT_ASSERT(fmt.fare(amount1) == amountExpected1);
  }

  void pxaItineraryTest()
  {
    std::string header = PfcDisplayBuilderPXA_itineraryInfo::TABLE_HEADER;

    std::string arpt = "ABE";
    std::string effDate = "01NOV07";
    std::string discDate = "01AUG18";
    std::string carrier = "NW";

    PfcDisplayFormatterPXA fmt(header);

    std::string line = fmt.line(arpt, effDate, discDate, carrier);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arpt) == std::string::npos || line.find(effDate) == std::string::npos ||
        line.find(discDate) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxhLineTest()
  {
    std::string header = PfcDisplayBuilderPXH::TABLE_HEADER;

    std::string arpt = "DFW";
    std::string effDate = "24MAR07";
    std::string expireDate = "01AUG18";
    std::string discDate = "01AUG18";
    std::string amount = "USD 4.50";
    std::string segCnt = "000";
    std::string createDT = "15DEC05/19:11";

    PfcDisplayFormatterPXH fmt(header);

    std::string line = fmt.line(arpt, effDate, expireDate, discDate, amount, segCnt, createDT);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arpt) == std::string::npos || line.find(effDate) == std::string::npos ||
        line.find(expireDate) == std::string::npos || line.find(discDate) == std::string::npos ||
        line.find(amount) == std::string::npos || line.find(segCnt) == std::string::npos ||
        line.find(createDT) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }

  void pxhDateTimeTest()
  {
    DateTime dt = boost::posix_time::time_from_string("2008-01-20 23:59:59.000");

    PfcDisplayFormatterPXH fmt;

    std::string dtExpected = "20JAN08/23:59";

    CPPUNIT_ASSERT(fmt.dateTime(dt) == dtExpected);
  }

  void pxmLineTest()
  {
    std::string header = PfcDisplayBuilderPXM::TABLE_HEADER;

    std::string arpt = "ACT";
    std::string type = "AIRPORT/CITY";
    std::string coterminal = "2";

    PfcDisplayFormatterPXM fmt(header);

    std::string line = fmt.line(arpt, type, coterminal);

    if (line.size() > PfcDisplayFormatter::GREEN_SCREEN_WIDTH ||
        line.find(arpt) == std::string::npos || line.find(type) == std::string::npos ||
        line.find(coterminal) == std::string::npos)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(true);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PfcDisplayFormatterTest);
}
