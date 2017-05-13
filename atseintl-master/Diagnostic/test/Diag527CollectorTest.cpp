#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag527Collector.h"
#include "DBAccess/Tours.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diagnostic.h"

namespace tse
{

class Diag527CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag527CollectorTest);
  CPPUNIT_TEST(testDiagActivated);
  CPPUNIT_TEST(testDiagDeactivated);
  CPPUNIT_TEST(testDisplayHeader);
  CPPUNIT_TEST(testDisplayPUHeader);
  CPPUNIT_TEST(testDisplayFPHeader);
  CPPUNIT_TEST(testDisplayStatusPass);
  CPPUNIT_TEST(testDisplayStatusFail);
  CPPUNIT_TEST(testDisplayStatusSoftPass);
  CPPUNIT_TEST(testDisplayStatusUnk);
  CPPUNIT_TEST(testDisplayPaxTypeFareEmptyTourCode);
  CPPUNIT_TEST(testDisplayPaxTypeFareTourCode);
  CPPUNIT_TEST(testDisplayPaxTypeFareRT);
  CPPUNIT_TEST(testDisplayPaxTypeFareCT);
  CPPUNIT_TEST(testDisplayPaxTypeFareOW);
  CPPUNIT_TEST(testDisplayPaxTypeFareOJ);
  CPPUNIT_TEST(testDisplayPaxTypeFareUnknown);
  CPPUNIT_TEST_SUITE_END();

private:
  class MockDiag527Collector : public Diag527Collector
  {
  protected:
    void getTourCode(const PaxTypeFare& paxTypeFare, string& tourCode) { tourCode = _tourCode; }
    void getMoney(const PaxTypeFare& paxTypeFare, string& money) { money = _money; }

  public:
    MockDiag527Collector(Diagnostic& root) : Diag527Collector(root) {}

    string _tourCode, _money;
  };

  Diag527Collector* _collector;
  MockDiag527Collector* _mock;
  Diagnostic* _diagRoot;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _diagRoot = _memHandle.insert(new Diagnostic(Diagnostic527));
    _diagRoot->activate();
    _collector = _memHandle.insert(new Diag527Collector(*_diagRoot));
    _collector->enable(Diagnostic527);
    _mock = _memHandle.insert(new MockDiag527Collector(*_diagRoot));
    _mock->enable(Diagnostic527);
  }

  void tearDown() { _memHandle.clear(); }

  void testDiagActivated()
  {
    _collector->activate();
    _collector->displayHeader();
    CPPUNIT_ASSERT(!_collector->str().empty());
  }

  void testDiagDeactivated()
  {
    _collector->deActivate();
    _collector->displayHeader();
    CPPUNIT_ASSERT(_collector->str().empty());
  }

  void testDisplayHeader()
  {
    _collector->displayHeader();
    string expectedResult("***************************************************************\n"
                          "*        ATSE CATEGORY 27 TOURS FARE PATH PROCESSING          *\n"
                          "***************************************************************\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayPUHeader()
  {
    _collector->displayPUHeader();
    string expectedResult("PRICING UNIT:\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayFPHeader()
  {
    _collector->displayFPHeader();
    string expectedResult("FARE PATH:\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayStatusPass()
  {
    _collector->displayStatus(PASS);
    string expectedResult("TOUR CODE COMBINATION: PASS\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayStatusFail()
  {
    _collector->displayStatus(FAIL);
    string expectedResult("TOUR CODE COMBINATION: FAIL\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayStatusSoftPass()
  {
    _collector->displayStatus(SOFTPASS);
    string expectedResult("TOUR CODE COMBINATION: SOFTPASS\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayStatusUnk()
  {
    _collector->displayStatus(SKIP);
    string expectedResult("TOUR CODE COMBINATION: UNK\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void preparePaxTypeFare(PaxTypeFare& paxTypeFare,
                          VendorCode vendor,
                          CarrierCode carrier,
                          TariffNumber tcrRuleTariff,
                          RuleNumber ruleNumber,
                          LocCode boardCty,
                          LocCode offCty)
  {
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->boardMultiCity() = boardCty;
    fareMarket->offMultiCity() = offCty;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = vendor;
    fareInfo->carrier() = carrier;
    fareInfo->ruleNumber() = ruleNumber;
    fareInfo->fareClass() = "Y";

    TariffCrossRefInfo* tariffCrossInfo = _memHandle.create<TariffCrossRefInfo>();
    tariffCrossInfo->ruleTariff() = tcrRuleTariff;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);
    fare->setTariffCrossRefInfo(tariffCrossInfo);

    PaxType* paxType = _memHandle.create<PaxType>();

    paxTypeFare.initialize(fare, paxType, fareMarket);
  }

  void prepareFirstPaxTypeFare(PaxTypeFare& paxTypeFare)
  {
    preparePaxTypeFare(paxTypeFare, "ATP", "BA", 389, "JP01", "MIA", "BFS");
  }

  void prepareMock()
  {
    _mock->_money = "USD578.00";
  }

  void testDisplayPaxTypeFareEmptyTourCode()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);
    prepareMock();

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::ROUNDTRIP);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01  RT\n"
                             "TOUR CODE: \n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }

  void testDisplayPaxTypeFareTourCode()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);

    prepareMock();
    _mock->_tourCode = "TOURCODE";

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::ROUNDTRIP);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01  RT\n"
                             "TOUR CODE: TOURCODE\n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }

  void testDisplayPaxTypeFareRT()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);
    prepareMock();

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::ROUNDTRIP);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01  RT\n"
                             "TOUR CODE: \n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }

  void testDisplayPaxTypeFareCT()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);
    prepareMock();

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::CIRCLETRIP);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01  CT\n"
                             "TOUR CODE: \n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }

  void testDisplayPaxTypeFareOW()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);
    prepareMock();

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::ONEWAY);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01  OW\n"
                             "TOUR CODE: \n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }

  void testDisplayPaxTypeFareOJ()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);
    prepareMock();

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::OPENJAW);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01  OJ\n"
                             "TOUR CODE: \n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }

  void testDisplayPaxTypeFareUnknown()
  {
    PaxTypeFare paxTypeFare;
    prepareFirstPaxTypeFare(paxTypeFare);
    prepareMock();

    _mock->displayPaxTypeFare(paxTypeFare, PricingUnit::Type::UNKNOWN);
    string expectedResult = ("MIA BFS             Y USD578.00  ATP   389  BA  JP01    \n"
                             "TOUR CODE: \n\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _mock->str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag527CollectorTest);
}
