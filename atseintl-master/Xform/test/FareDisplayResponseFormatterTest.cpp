#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBCategoryRuleRecord.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/NegFareRest.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Xform/FareDisplayResponseFormatter.h"
#include "Xform/FareDisplayResponseXMLTags.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
namespace
{
class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare()
  {
    fareClassAppInfo() = &_fareClassAppInfo;
    fareClassAppSegInfo() = &_fareClassAppSegInfo;
    actualPaxType() = &_paxType;
    fareMarket() = &_fareMarket;
    fareDisplayInfo() = &_fareDisplayInfo;
    setFare(&_fare);

    _fare.setFareInfo(&_fareInfo);
    _fare.setTariffCrossRefInfo(&_tariffCrossRefInfo);
    _fare.status().set(Fare::FS_ScopeIsDefined);

    _bookingCode = "Y";
    _fareInfo.fareTariff() = 111;
    _fareInfo.linkNumber() = 112;
    _fareInfo.sequenceNumber() = 113;
    _fareInfo.fareClass() = "FCX";
    _fareInfo.vendor() = "AAA";
  }

  std::string createFareBasisCodeFD(FareDisplayTrx& trx) const { return std::string("FBC"); }

  // DataMembers
  Fare _fare;
  TariffCrossRefInfo _tariffCrossRefInfo;
  FareInfo _fareInfo;
  FareClassAppInfo _fareClassAppInfo;
  FareClassAppSegInfo _fareClassAppSegInfo;
  PaxType _paxType;
  FareMarket _fareMarket;
  FareDisplayInfo _fareDisplayInfo;
};

class MockCat35PaxTypeFare : public MockPaxTypeFare
{
public:
  MockCat35PaxTypeFare(PaxTypeFare* base)
  {
    _status.set(PaxTypeFare::PTF_Negotiated);

    _allRuleData.chkedRuleData = false;
    _allRuleData.chkedGfrData = false;
    _allRuleData.fareRuleData = &_ruleData;
    _allRuleData.gfrRuleData = 0;

    _ruleData.ruleItemInfo() = &_negFareRest;
    ;
    _ruleData.baseFare() = base;

    (*_paxTypeFareRuleDataMap)[35] = &_allRuleData;

    _fareInfo.fareTariff() = 351;
    _fareInfo.linkNumber() = 352;
    _fareInfo.sequenceNumber() = 353;
    _fareInfo.fareClass() = "F35";
    _fareInfo.vendor() = "BBB";
  }

  PaxTypeFareAllRuleData _allRuleData;
  PaxTypeFareRuleData _ruleData;
  NegFareRest _negFareRest;
};

class MockFDResponseXMLTags : public FareDisplayResponseXMLTags
{
public:
  MockFDResponseXMLTags() {}
  std::string originalFareAmount(const Fare*& fare, const DateTime& ticketingDate)
  {
    return std::string("5USD");
  }
};
}

class FareDisplayResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayResponseFormatterTest);
  CPPUNIT_TEST(testAddFqdType_Empty);

  CPPUNIT_TEST(testAddFqdType_Tariff);
  CPPUNIT_TEST(testAddFqdType_FareClass);
  CPPUNIT_TEST(testAddFqdType_Link);
  CPPUNIT_TEST(testAddFqdType_Sequence);
  CPPUNIT_TEST(testAddFqdType_Vendor);

  CPPUNIT_TEST(testAddFqdType_TariffCat35);
  CPPUNIT_TEST(testAddFqdType_FareClassCat35);
  CPPUNIT_TEST(testAddFqdType_LinkCat35);
  CPPUNIT_TEST(testAddFqdType_SequenceCat35);
  CPPUNIT_TEST(testAddFqdType_VendorCat35);
  CPPUNIT_TEST(testAddFqdTypeWithoutBrandProgram);
  CPPUNIT_TEST(testAddFqdTypeWithBrandProgram);
  CPPUNIT_TEST(testMoreFqdData);

  CPPUNIT_TEST(testAddCATTypeForFB_IC);
  CPPUNIT_TEST(testAddCATTypeForFB_RC);
  CPPUNIT_TEST(testAddCATTypeForFB_Cat1);

  CPPUNIT_TEST(testAddRULType_IC);
  CPPUNIT_TEST(testAddRULType_RC);
  CPPUNIT_TEST(testAddRULType_Cat1);

  CPPUNIT_TEST_SUITE_END();

protected:
  FareDisplayResponseFormatter* _formatter;
  FareDisplayTrx* _fdTrx;
  XMLConstruct* _xml;
  TestMemHandle _memHandle;

  class MyDataHandle : public DataHandleMock
  {
  public:
    const RuleCategoryDescInfo* getRuleCategoryDesc(const CatNumber& key)
    {
      _rcdi.defaultMsg() = std::to_string(key);
      return &_rcdi;
    }
  private:
    RuleCategoryDescInfo _rcdi;
  };


public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _formatter = _memHandle.insert(
        new FareDisplayResponseFormatter(*_memHandle.insert(new MockFDResponseXMLTags)));
    _fdTrx = _memHandle.insert(new FareDisplayTrx);
    _fdTrx->setOptions(_memHandle.insert(new FareDisplayOptions));
    FareDisplayRequest* request = _memHandle.insert(new FareDisplayRequest);
    request->ticketingAgent() = _memHandle.insert(new Agent);
    _fdTrx->setRequest(request);
    _xml = _memHandle.insert(new XMLConstruct);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void createFare()
  {
    MockPaxTypeFare* ptf = _memHandle.insert(new MockPaxTypeFare);

    _fdTrx->allPaxTypeFare().push_back(ptf);
  }

  void createCat35Fare()
  {
    MockPaxTypeFare* base = _memHandle.insert(new MockPaxTypeFare);
    MockPaxTypeFare* cat35ptf = _memHandle.insert(new MockCat35PaxTypeFare(base));

    _fdTrx->allPaxTypeFare().push_back(cat35ptf);
  }

  // TESTS
  void testAddFqdType_Empty()
  {
    _formatter->addFQDType(*_fdTrx, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().empty());
  }

  void testAddFqdType_Tariff()
  {
    createFare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("Q3W=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("111"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_FareClass()
  {
    createFare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("BJ0=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("FCX"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_Link()
  {
    createFare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("Q46=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("112"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_Sequence()
  {
    createFare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("Q1K=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("113"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_Vendor()
  {
    createFare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("S37=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("AAA"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_TariffCat35()
  {
    createCat35Fare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("Q3W=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("351"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_FareClassCat35()
  {
    createCat35Fare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("BJ0=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("FCX"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_LinkCat35()
  {
    createCat35Fare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("Q46=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("112"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_SequenceCat35()
  {
    createCat35Fare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("Q1K=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("113"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdType_VendorCat35()
  {
    createCat35Fare();

    _formatter->addFQDType(*_fdTrx, *_xml);
    int pos;
    CPPUNIT_ASSERT((pos = _xml->getXMLData().find("S37=\"")) > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("AAA"), _xml->getXMLData().substr(pos + 5, 3));
  }

  void testAddFqdTypeWithoutBrandProgram()
  {
    createFare();
    _formatter->addFQDType(*_fdTrx, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SB2") == std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SB3") == std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SC0") == std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SC2") == std::string::npos);
  }

  void createBrandedFare()
  {
    MockPaxTypeFare* ptf = _memHandle.insert(new MockPaxTypeFare);
    QualifiedBrand programBrandPair;
    programBrandPair.first = _memHandle.create<BrandProgram>();
    programBrandPair.second = _memHandle.create<BrandInfo>();
    programBrandPair.first->programCode() = "BD";
    programBrandPair.first->programName() = "Diamond";
    programBrandPair.second->brandCode() = "CC";
    programBrandPair.second->brandName() = "Cricket Team Cape Cobras";
    ptf->fareDisplayInfo() = _memHandle.create<FareDisplayInfo>();
    ptf->fareDisplayInfo()->setBrandProgramPair(programBrandPair);
    _fdTrx->allPaxTypeFare().push_back(ptf);
  }

  void testAddFqdTypeWithBrandProgram()
  {
    createBrandedFare();
    _formatter->addFQDType(*_fdTrx, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SB2=\"CC\"") != std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SB3=\"CRICKET TEAM CAPE COBRAS\"") != std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SC0=\"BD\"") != std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("SC2=\"DIAMOND\"") != std::string::npos);
  }

  void testMoreFqdData()
  {
    createFare();
    std::map<FieldColumnElement, std::string> *fareDataMap = _memHandle.insert(new std::map<FieldColumnElement, std::string>);
    fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
          JOURNEY_IND, "R"));
    fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
          SAME_DAY_IND, " "));
    fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
          NET_FARE_IND, "N"));
    fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
          ADVANCE_PURCHASE, " -/$ "));
    fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
          MIN_MAX_STAY, " -/12M"));
    _fdTrx->addFareDataMapVec(fareDataMap);
    _formatter->addFQDType(*_fdTrx, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().find("P04") != std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("PAL") == std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("N1I=\"N\"") != std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("S50=\"-/$\"") != std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("S51") == std::string::npos);
    CPPUNIT_ASSERT(_xml->getXMLData().find("S52=\"12M\"") != std::string::npos);
  }

  MockPaxTypeFare& createPtfWithRule(CatNumber cat)
  {
    FBCategoryRuleRecord* rd =_memHandle.create<FBCategoryRuleRecord>();
    FareRuleRecord2Info* fRR = _memHandle.create<FareRuleRecord2Info>();
    rd->fareRuleRecord2Info() = fRR;
    createBrandedFare();
    MockPaxTypeFare& ptf = (MockPaxTypeFare&)*_fdTrx->allPaxTypeFare().back();
    _fdTrx->getOptions()->ruleCategories().push_back(cat);
    _fdTrx->itin().push_back(_memHandle.create<Itin>());
    ptf.fareDisplayInfo()->fbDisplay().setRuleRecordData(cat, rd, false);
    ptf.fareDisplayInfo()->fbDisplay().setData(cat, fRR, 0, 0, "FC", _fdTrx->dataHandle());

    return ptf;
  }

  void testAddCATTypeForFB_IC()
  {
    MockPaxTypeFare& ptf = createPtfWithRule(IC_RULE_CATEGORY);
    _formatter->addCATTypeForFB(*_fdTrx, ptf.fareDisplayInfo(), ptf, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().empty());
  }

  void testAddCATTypeForFB_RC()
  {
    MockPaxTypeFare& ptf = createPtfWithRule(RETAILER_CATEGORY);
    _formatter->addCATTypeForFB(*_fdTrx, ptf.fareDisplayInfo(), ptf, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().empty());
  }

  void testAddCATTypeForFB_Cat1()
  {
    MockPaxTypeFare& ptf = createPtfWithRule(ELIGIBILITY_RULE);
    _formatter->addCATTypeForFB(*_fdTrx, ptf.fareDisplayInfo(), ptf, *_xml);
    CPPUNIT_ASSERT(_xml->getXMLData().find("<CAT NUM=\"1\">") != std::string::npos);
  }

  void testAddRULType_IC()
  {
    MockPaxTypeFare& ptf = createPtfWithRule(IC_RULE_CATEGORY);
    _formatter->addRULType(*_fdTrx, ptf.fareDisplayInfo(), *_xml);
    CPPUNIT_ASSERT_EQUAL((std::string) "<RUL Q3V=\"IC\"/>", _xml->getXMLData());
  //  CPPUNIT_ASSERT(_xml->getXMLData().find("<CAT NUM=\"1\">") != std::string::npos);
  }

  void testAddRULType_RC()
  {
    MockPaxTypeFare& ptf = createPtfWithRule(RETAILER_CATEGORY);
    _formatter->addRULType(*_fdTrx, ptf.fareDisplayInfo(), *_xml);
    CPPUNIT_ASSERT_EQUAL((std::string) "<RUL Q3V=\"RR\"/>", _xml->getXMLData());
 //   CPPUNIT_ASSERT(_xml->getXMLData().find("<CAT NUM=\"1\">") != std::string::npos);
  }

  void testAddRULType_Cat1()
  {
    MockPaxTypeFare& ptf = createPtfWithRule(ELIGIBILITY_RULE);
    _formatter->addRULType(*_fdTrx, ptf.fareDisplayInfo(), *_xml);
    CPPUNIT_ASSERT_EQUAL((std::string) "<RUL Q3V=\"1\"/>", _xml->getXMLData());
  //  CPPUNIT_ASSERT(_xml->getXMLData().find("<CAT NUM=\"1\">") != std::string::npos);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayResponseFormatterTest);
}
