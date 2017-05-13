#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "FareCalc/CalcTotals.h"

#include "Xform/ERDSectionFormatter.h"

namespace tse
{

class ERDSectionFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ERDSectionFormatterTest);
  CPPUNIT_TEST(testPrepareERD_WithoutYY);
  CPPUNIT_TEST(testPrepareERD_WithYY);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memH;
  PricingTrx* _pricingTrx;
  CalcTotals* _calcTotals;
  FareUsage* _fareUsage;
  CalcTotals::FareCompInfo* _fareCompInfo;
  XMLConstruct* _construct;
  ERDSectionFormatter* _erd;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _pricingTrx = _memH.create<PricingTrx>();
    PricingRequest* request = _memH.create<PricingRequest>();
    request->ticketingAgent() = _memH.create<Agent>();
    _pricingTrx->setRequest(request);
    _pricingTrx->setOptions(_memH.create<PricingOptions>());
    _calcTotals = _memH.create<CalcTotals>();
    _fareUsage = _memH.create<FareUsage>();
    _fareUsage->paxTypeFare() = createPtf("AA");
    _fareUsage->travelSeg().push_back(_memH.create<AirSeg>());
    _fareCompInfo = _memH.create<CalcTotals::FareCompInfo>();
    _construct = _memH.create<XMLConstruct>();
    _erd = _memH.insert(new ERDSectionFormatter(
        *_pricingTrx, *_calcTotals, *_fareUsage, *_fareCompInfo, *_construct));
    _construct->openElement("TEST");
  }

  PaxTypeFare* createPtf(const CarrierCode& carrier)
  {
    PaxTypeFare* paxTypeFare = _memH.create<PaxTypeFare>();
    Fare* fare = _memH.create<Fare>();
    FareInfo* fareInfo = _memH.create<FareInfo>();
    fareInfo->carrier() = carrier;
    fareInfo->fareClass() = "FC";
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);
    paxTypeFare->actualPaxType() = _memH.create<PaxType>();
    paxTypeFare->fareMarket() = _memH.create<FareMarket>();
    ;

    return paxTypeFare;
  }

  void tearDown() { _memH.clear(); }

  void testPrepareERD_WithoutYY()
  {
    _erd->prepareERD();
    const std::string expectedXML = "B50=\"FC\"";
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _construct->getXMLData().find(expectedXML));
  }

  void testPrepareERD_WithYY()
  {
    _fareUsage->paxTypeFare() = createPtf("YY");
    _erd->prepareERD();
    const std::string expectedXML = "B50=\"FC\"";
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find(expectedXML));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ERDSectionFormatterTest);
}
