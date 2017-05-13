#include "Diagnostic/Diag690Collector.h"

#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/VoluntaryChangesInfo.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <algorithm>
#include <iostream>

namespace tse
{
class Diag690CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag690CollectorTest);

  CPPUNIT_TEST(testResidualPenaltyIndicator_Blank);
  CPPUNIT_TEST(testResidualPenaltyIndicator_TheSame);
  CPPUNIT_TEST(testPrintRefundPlusUp);
  CPPUNIT_TEST(testPrintFareUsage_refundable);
  CPPUNIT_TEST(testPrintFareUsage_nonRefundable);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diagCollector = _memHandle.create<Diag690Collector>();
    _diagCollector->_trx = _memHandle.create<PricingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { Diag690Collector diag; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testResidualPenaltyIndicator_Blank()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("RESIDUAL/PENALTY INDICATOR: BLANK/N\n"),
                         _diagCollector->residualPenaltyIndicator(' ', 'N'));
  }

  void testResidualPenaltyIndicator_TheSame()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("RESIDUAL/PENALTY INDICATOR: R\n"),
                         _diagCollector->residualPenaltyIndicator('R', 'R'));
  }
  void testPrintRefundPlusUp()
  {
    _diagCollector->_trx = _memHandle.create<RefundPricingTrx>();
    _diagCollector->activate();
    _diagCollector->printRefundPlusUp();

    CPPUNIT_ASSERT_EQUAL(std::string(" PLUS-UPS RECREATED: Y\n"), _diagCollector->str());
  }

  FareUsage& createFareUsage()
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    FareMarket* fm = _memHandle.create<FareMarket>();

    FareInfo* info1 = _memHandle.create<FareInfo>();
    info1->fareClass() = "ABC";

    Fare* fare1 = _memHandle.create<Fare>();
    fare1->setFareInfo(info1);

    fm->boardMultiCity() = "KRK";
    fm->offMultiCity() = "WAW";
    ptf1->fareMarket() = fm;
    ptf1->setFare(fare1);
    fu->paxTypeFare() = ptf1;

    return *fu;
  }

  void testPrintFareUsage_refundable()
  {
    FareUsage& fu = createFareUsage();

    fu.paxTypeFare()->nucFareAmount() = 100.00;
    fu.isNonRefundable() = false;
    fu.setNonRefundableAmount(Money(100, NUC));

    _diagCollector->activate();
    _diagCollector->printFareUsage(fu, NUC, false);

    std::string expected = " KRK-WAW\n"
                           "  NONREFUNDABLE AMOUNT: 100.00 NUC\n"
                           "  SURCHARGE        0  TRANSFER         0  STOPOVER         0\n"
                           "  DIFF             0  EMS              0\n"
                           "   ABC               0                     I    \n\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diagCollector->str());
  }

  void testPrintFareUsage_nonRefundable()
  {
    FareUsage& fu = createFareUsage();

    fu.paxTypeFare()->nucFareAmount() = 100.00;
    fu.isNonRefundable() = true;

    _diagCollector->activate();
    _diagCollector->printFareUsage(fu, NUC, false);

    std::string expected = " KRK-WAW\n"
                           "  NONREFUNDABLE: YES\n"
                           "  SURCHARGE        0  TRANSFER         0  STOPOVER         0\n"
                           "  DIFF             0  EMS              0\n"
                           "   ABC               0                     I    \n\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diagCollector->str());
  }

  TestMemHandle _memHandle;
  Diag690Collector* _diagCollector;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag690CollectorTest);

} // end of tse
