#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFareFactory.h"

#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diagnostic.h"

#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class Diag200CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag200CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorFare_Blank);
  CPPUNIT_TEST(testStreamingOperatorFare_Populated);
  CPPUNIT_TEST(testStreamingOperatorFareMarket_Empty);
  CPPUNIT_TEST(testStreamingOperatorValidatingCarrier);
  CPPUNIT_TEST_SUITE_END();

public:
  void testStreamingOperatorFare_Blank()
  {
    std::string expected = "P EH A 2431 A2UP1        4   R O  1627.50 USD FX  ***  2900.60\n";

    (*_diag) << *createPtf(false);
    CPPUNIT_ASSERT_EQUAL(expected, getDiagString());
  }

  void testStreamingOperatorFare_Populated()
  {
    std::string expected = "P AT A 1    AAAA         4   R I   200.00 USD FX  ***   300.00\n";

    (*_diag) << *createPtf(true);
    CPPUNIT_ASSERT_EQUAL(expected, getDiagString());
  }

  void testStreamingOperatorFareMarket_Empty()
  {
    std::string expected = "";

    FareMarket fm;

    (*_diag) << fm;
    CPPUNIT_ASSERT_EQUAL(expected, getDiagString());
  }
  void testStreamingOperatorValidatingCarrier()
  {
    std::string expected = "CONVERTED CURRENCY : USD\nTICKETING CARRIER : AA  UA  \n";
    FareMarket fm;
    fm.validatingCarriers().push_back("AA");
    fm.validatingCarriers().push_back("UA");
    Itin itin;
    itin.ticketingCarrier() = "AA";
    itin.calculationCurrency() = "USD";

    _trx->setValidatingCxrGsaApplicable(true);
    _diag->_trx = _trx;
    _diag->print(itin, fm);
    std::cout << getDiagString();
    CPPUNIT_ASSERT_EQUAL(expected, getDiagString());
  }
  void setUp()
  {
    _trx = _memHandle.insert(new PricingTrx());
    _diag = _memHandle.insert(new Diag200Collector());
    _diag->activate();
  }

  void tearDown() { _memHandle.clear(); }

protected:
  PaxTypeFare* createPtf(bool populated)
  {
    PaxTypeFare* mf = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/Fares/test/data/FareCurrencySelection/IntlPrime/trx_PaxTypeFare0.xml");
    mf->status().setNull();
    FareInfo* fi = (FareInfo*)mf->fare()->fareInfo();
    fi->_currency = "USD";

    if (populated)
    {
      mf->nucFareAmount() = 300;

      fi->_carrier = "AA";
      fi->_globalDirection = GlobalDirection::AT;
      fi->_vendor = ATPCO_VENDOR_CODE;
      fi->_ruleNumber = '1';
      fi->_fareClass = "AAAA";
      fi->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
      fi->_directionality = FROM;
      fi->_fareAmount = 200;
      fi->_currency = "USD";

      mf->setFare(mf->fare());
    }

    return mf;
  }

  std::string getDiagString()
  {
    _diag->flushMsg();
    return _diag->str();
  }

  Diag200Collector* _diag;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag200CollectorTest);
}
