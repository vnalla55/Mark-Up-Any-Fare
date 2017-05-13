#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PricingTrx.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestItinFactory.h"
#include "test/testdata/TestTravelSegFactory.h"
#include "DataModel/AirSeg.h"
#include "Common/WnSnapUtil.h"
#include "Server/TseServer.h"
#include "test/testdata/TestPricingTrxFactory.h"

using namespace std;

namespace tse
{

class TseServerMock : public TseServer
{
public:
  static tse::ConfigMan* getConfig() { return Global::_configMan; }
  static void setConfig(tse::ConfigMan* configMan) { Global::_configMan = configMan; }
};

class WnSnapUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(WnSnapUtilTest);

  CPPUNIT_TEST(testSplitItinsByDirection1);
  CPPUNIT_TEST(testSplitItinsByDirection2);
  CPPUNIT_TEST(testSplitTaxByLegs1);
  CPPUNIT_TEST(testSplitTaxByLegs2);
  CPPUNIT_TEST(testSplitTaxByLegs3);
  CPPUNIT_TEST(testSplitTaxByLegs4);
  CPPUNIT_TEST(testSplitTaxByLegs5);

  CPPUNIT_TEST_SUITE_END();

public:
  PricingTrx* _pricingTrx;
  Itin* _itin;
  std::vector<Itin*> _itinFirstCxr;
  std::vector<Itin*> _itinSecondCxr;
  TestMemHandle _memH;
  tse::ConfigMan _config;

  void setUpSnapItinerary1()
  {
    _pricingTrx->itin().push_back(
        TestItinFactory::create("/vobs/atseintl/test/testdata/data/ItinSFO_GDL_SFO.xml"));
    std::vector<Itin*>& baseItin = _pricingTrx->itin();
    baseItin.at(0)->travelSeg().at(0)->legId() = 0;
    baseItin.at(0)->travelSeg().at(1)->legId() = 0;
    baseItin.at(0)->travelSeg().at(2)->legId() = 1;
    baseItin.at(0)->travelSeg().at(3)->legId() = 1;

    std::vector<std::pair<int, int> > legID;
    legID.push_back(std::pair<int, int>(0, 0));
    baseItin.at(0)->legID() = legID;

    _itinFirstCxr.clear();
    _itinSecondCxr.clear();

    Itin* itin1 = _memH.insert(new Itin);
    Itin* itin2 = _memH.insert(new Itin);

    itin1->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSFO_LAX.xml"));
    itin1->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_GDL.xml"));
    itin2->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegGDL_LAX.xml"));
    itin2->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_SFO.xml"));

    _itinFirstCxr.push_back(itin1);
    _itinSecondCxr.push_back(itin2);
  }

  void setUpSnapItinerary2()
  {
    _pricingTrx->itin().clear();
    _pricingTrx->subItinVecOutbound().clear();
    _pricingTrx->itin().push_back(
        TestItinFactory::create("/vobs/atseintl/test/testdata/data/ItinSFO_GDL_SFO_WN.xml"));
    std::vector<Itin*>& baseItin = _pricingTrx->itin();
    baseItin.at(0)->travelSeg().at(0)->legId() = 0;
    baseItin.at(0)->travelSeg().at(1)->legId() = 0;
    baseItin.at(0)->travelSeg().at(2)->legId() = 0;
    baseItin.at(0)->travelSeg().at(3)->legId() = 0;

    std::vector<std::pair<int, int> > legID;
    legID.push_back(std::pair<int, int>(0, 0));
    baseItin.at(0)->legID() = legID;
    _itinFirstCxr.clear();
    _itinSecondCxr.clear();

    Itin* itin1 = _memH.insert(new Itin);

    itin1->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSFO_LAX.xml"));
    itin1->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_GDL_WN.xml"));
    itin1->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegGDL_LAX_WN.xml"));
    itin1->travelSeg().push_back(
        TestTravelSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_SFO.xml"));

    _itinFirstCxr.push_back(itin1);
  }

  void setUp()
  {
    TseServerMock::setConfig(&_config);
    _pricingTrx = TestPricingTrxFactory::create("/vobs/atseintl/test/testdata/data/pricingTrx.xml");
    _itin = _memH.insert(new Itin);
  }

  void tearDown() { _memH.clear(); }

  void compareItin(std::vector<Itin*>& firstItinVec, std::vector<Itin*>& secondItinVec)
  {
    std::vector<Itin*>::iterator itinVecFirstIt = firstItinVec.begin();
    std::vector<Itin*>::iterator itinVecFirstItEnd = firstItinVec.end();

    std::vector<Itin*>::iterator itinVecSecondIt = secondItinVec.begin();
    std::vector<Itin*>::iterator itinVecSecondItEnd = secondItinVec.end();

    for (; itinVecFirstIt != itinVecFirstItEnd && itinVecSecondIt != itinVecSecondItEnd;
         ++itinVecFirstIt, ++itinVecSecondIt)
    {
      std::vector<TravelSeg*>::iterator travelSegIter = (*itinVecFirstIt)->travelSeg().begin();
      std::vector<TravelSeg*>::iterator travelSegIterEnd = (*itinVecFirstIt)->travelSeg().end();

      std::vector<TravelSeg*>::iterator compareTravelSegIter =
          (*itinVecSecondIt)->travelSeg().begin();
      std::vector<TravelSeg*>::iterator compareTravelSegIterEnd =
          (*itinVecSecondIt)->travelSeg().end();

      for (; travelSegIter != travelSegIterEnd || compareTravelSegIter != compareTravelSegIterEnd;
           ++travelSegIter, ++compareTravelSegIter)
      {
        TravelSeg* travelSeg = (*travelSegIter);
        AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
        TravelSeg* travelSegComp = (*compareTravelSegIter);
        AirSeg* airSegComp = dynamic_cast<AirSeg*>(travelSegComp);

        if (airSeg != NULL && airSegComp != NULL)
        {
          CPPUNIT_ASSERT_EQUAL(airSegComp->carrier(), airSeg->carrier());
          CPPUNIT_ASSERT_EQUAL(airSegComp->flightNumber(), airSeg->flightNumber());
        }
      }
    }
  }

  void testSplitItinsByDirection1()
  {
    setUpSnapItinerary1();
    WnSnapUtil::splitItinsByDirection(*_pricingTrx);

    compareItin(_pricingTrx->subItinVecOutbound(), _itinFirstCxr);
    compareItin(_pricingTrx->subItinVecInbound(), _itinSecondCxr);
  }

  void testSplitItinsByDirection2()
  {
    setUpSnapItinerary2();
    WnSnapUtil::splitItinsByDirection(*_pricingTrx);

    compareItin(_pricingTrx->subItinVecOutbound(), _itinFirstCxr);
  }

  void testSplitTaxByLegs1()
  {
    TaxItem* taxItem = _memH.insert(new TaxItem);
    taxItem->taxAmount() = 5.0;
    taxItem->setPaymentCurrencyNoDec(0);

    MoneyAmount expected = 3.0;

    CPPUNIT_ASSERT(WnSnapUtil::divideAmountWithRounding(taxItem) == expected);
  }

  void testSplitTaxByLegs2()
  {
    TaxItem* taxItem = _memH.insert(new TaxItem);
    taxItem->taxAmount() = 6.0;
    taxItem->setPaymentCurrencyNoDec(0);

    MoneyAmount expected = 3.0;

    CPPUNIT_ASSERT(WnSnapUtil::divideAmountWithRounding(taxItem) == expected);
  }

  void testSplitTaxByLegs3()
  {
    TaxItem* taxItem = _memH.insert(new TaxItem);
    taxItem->taxAmount() = 5.9;
    taxItem->setPaymentCurrencyNoDec(1);

    MoneyAmount expected = 3.0;

    CPPUNIT_ASSERT(WnSnapUtil::divideAmountWithRounding(taxItem) == expected);
  }

  void testSplitTaxByLegs4()
  {
    TaxItem* taxItem = _memH.insert(new TaxItem);
    taxItem->taxAmount() = 5.99;
    taxItem->setPaymentCurrencyNoDec(2);

    MoneyAmount expected = 3.0;

    CPPUNIT_ASSERT(WnSnapUtil::divideAmountWithRounding(taxItem) == expected);
  }

  void testSplitTaxByLegs5()
  {
    TaxItem* taxItem = _memH.insert(new TaxItem);
    taxItem->taxAmount() = 6.4;
    taxItem->setPaymentCurrencyNoDec(2);

    MoneyAmount expected = 3.2;

    CPPUNIT_ASSERT(WnSnapUtil::divideAmountWithRounding(taxItem) == expected);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WnSnapUtilTest);

} // end of tse
