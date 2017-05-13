#include <iostream>
#include <functional>
#include <vector>

#include "FareCalc/test/FcStreamTest.h"
#include "FareCalc/FcStream.h"
#include "FareCalc/FcUtil.h"

#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"

namespace tse
{

void
FcStreamTest::setUp()
{
}

void
FcStreamTest::tearDown()
{
}

void
FcStreamTest::testCollect()
{
}

void
FcStreamTest::testForEachFareUsage()
{
}

void
FcStreamTest::testSplit()
{
  std::string output("31MAY DEPARTURE DATE-----LAST DAY TO PURCHASE 03MAY\n"
                     "       BASE FARE      EQUIV AMT      TAXES             TOTAL\n"
                     " 1-    EUR460.00      USD624.00     125.80XT       USD749.80ADT\n"
                     "    XT     30.20US       5.50YC       7.00XY       5.00XA\n"
                     "            2.50AY       5.70DE      10.70RD      16.00RA\n"
                     "           38.70FR       4.50XF\n"
                     "          460.00         624.00     125.80            749.80TTL\n"
                     "ADT-01  VLAPDE4\n"
                     " BRE LH X/FRA Q85.34AF X/PAR AF ATL M216.65VLAPDE4 AF X/PAR\n"
                     " Q85.34AF BRE M216.64VLAPDE4 NUC603.97END ROE0.7616 XFATL4.5\n"
                     "NON/REF SAPEX/CHG OF RES RESTRICTED\n"
                     "  \n"
                     "**\n"
                     "PRICED USING RULE OVERRIDE-FOLLOWING FARE RULES NOT MET\n"
                     "  FLIGHT APP RESTRICTIONS\n"
                     "  INCORRECT BOOKING CLASS\n"
                     "FARE NOT GUARANTEED IF TICKETED\n"
                     "**\n\n");

  FareCalc::FcStream fc;
  fc << output;

  // for (int k=0; k<100000; ++k)
  //{
  std::vector<std::string> lines;
  fc.split(lines);

  CPPUNIT_ASSERT_EQUAL(lines[12], std::string("**"));
  CPPUNIT_ASSERT_EQUAL(lines[13],
                       std::string("PRICED USING RULE OVERRIDE-FOLLOWING FARE RULES NOT MET"));
  CPPUNIT_ASSERT_EQUAL(lines[14], std::string("  FLIGHT APP RESTRICTIONS"));
  CPPUNIT_ASSERT_EQUAL(lines[15], std::string("  INCORRECT BOOKING CLASS"));
  CPPUNIT_ASSERT_EQUAL(lines[16], std::string("FARE NOT GUARANTEED IF TICKETED"));
  CPPUNIT_ASSERT_EQUAL(lines[17], std::string("**"));
  //}std::
  // std::cout << "\ntestSplit completed" << endl;
}

} // tse
