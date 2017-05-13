#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PaxTypeFare.h"
#include "DataModel/CxrPrecalculatedTaxes.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/PricingTrx.h"

namespace tse
{

class CxrPrecalculatedTaxesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CxrPrecalculatedTaxesTest);
  CPPUNIT_TEST(testGetLowerBoundTotalTaxAmount);
  CPPUNIT_TEST(testGetTotalTaxAmountForCarrier);
  CPPUNIT_TEST_SUITE_END();

public:

  PrecalculatedTaxes getPrecalculatedTaxes(MoneyAmount m)
  {
    PrecalculatedTaxes p;

    p.setDefaultAmount(PrecalculatedTaxesAmount::YQYR, m);
    p.setDefaultAmount(PrecalculatedTaxesAmount::CAT12, m);
    p.setDefaultAmount(PrecalculatedTaxesAmount::XF, m);

    return p;
  }

  PrecalculatedTaxesAmount getPrecalculatedTaxesAmount(MoneyAmount m)
  {
    PrecalculatedTaxesAmount p;

    p.setAmount(PrecalculatedTaxesAmount::YQYR, m);
    p.setAmount(PrecalculatedTaxesAmount::CAT12, m);
    p.setAmount(PrecalculatedTaxesAmount::XF, m);

    return p;
  }

  void testGetLowerBoundTotalTaxAmount()
  {
    CxrPrecalculatedTaxes c;
    PaxTypeFare ptf1, ptf2;

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 0);

    PrecalculatedTaxes pt = getPrecalculatedTaxes(5);

    PrecalculatedTaxesAmount pta = getPrecalculatedTaxesAmount(10);
    pt.setAmounts(ptf1, pta);

    c["AA"] = pt;

    c.processLowerBoundAmounts();

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 30);
    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf2) == 15); // default amount

    pta = getPrecalculatedTaxesAmount(20);
    pt.setAmounts(ptf1, pta);

    ptf1.validatingCarriers().push_back("AA");
    ptf1.validatingCarriers().push_back("BB");
    ptf2.validatingCarriers().push_back("AA");

    c["BB"] = pt;

    c.processLowerBoundAmounts();

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 30);
    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf2) == 15); // default amount

    pta = getPrecalculatedTaxesAmount(1);
    pt.setAmounts(ptf1, pta);
    c["CC"] = pt;
    ptf1.validatingCarriers().push_back("CC");

    c.processLowerBoundAmounts();

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 3);
    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf2) == 15); // default amount

    pta = getPrecalculatedTaxesAmount(33);
    pt.setAmounts(ptf2, pta);
    ptf2.validatingCarriers().push_back("BB");

    c["BB"] = pt;

    c.processLowerBoundAmounts();

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 3);
    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf2) == 99);

    pta = getPrecalculatedTaxesAmount(16);
    pt.setAmounts(ptf2, pta);
    ptf2.validatingCarriers().push_back("CC");

    c["CC"] = pt;

    c.processLowerBoundAmounts();

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 3);
    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf2) == 48);

    pta = getPrecalculatedTaxesAmount(60);
    pt.setAmounts(ptf2, pta);
    ptf2.validatingCarriers().push_back("AA");

    c["AA"] = pt;

    c.processLowerBoundAmounts();

    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf1) == 3);
    CPPUNIT_ASSERT(c.getLowerBoundTotalTaxAmount(ptf2) == 48);
  }

  void testGetTotalTaxAmountForCarrier()
  {
    CxrPrecalculatedTaxes c;
    PaxTypeFare ptf;

    CPPUNIT_ASSERT(c.getTotalTaxAmountForCarrier(ptf, "AA") == 0);

    PrecalculatedTaxes pt = getPrecalculatedTaxes(5);

    PrecalculatedTaxesAmount pta = getPrecalculatedTaxesAmount(10);
    pt.setAmounts(ptf, pta);

    c["AA"] = pt;

    CPPUNIT_ASSERT(c.getTotalTaxAmountForCarrier(ptf, "AA") == 30);
    CPPUNIT_ASSERT(c.getTotalTaxAmountForCarrier(ptf, "BB") == 0);
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(CxrPrecalculatedTaxesTest);

} // tse
