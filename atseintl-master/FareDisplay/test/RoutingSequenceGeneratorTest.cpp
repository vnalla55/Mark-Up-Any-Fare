#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/RoutingSequenceGenerator.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayResponse.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"

#include "FareDisplay/GroupingStrategy.h"
#include "FareDisplay/StrategyBuilder.h"

#include "DataModel/Itin.h"
#include "Common/DateTime.h"
#include "test/include/MockGlobal.h"

using namespace std;

namespace tse
{

class RoutingSequenceGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RoutingSequenceGeneratorTest);
  CPPUNIT_TEST(test_generate);
  CPPUNIT_TEST(test_generate_2);
  CPPUNIT_TEST(test_generate_3);
  CPPUNIT_TEST(test_generate_4);
  CPPUNIT_TEST(test_getRoutingOrder);
  CPPUNIT_TEST(test_getRTGSeq);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { setupTestData(); }
  void setupTestData()
  {
    _cxr = "AA";
    _n = "5873";
    _gd = GlobalDirection::AT;

    createPaxTypeFare(&_info1, _n, _cxr, _gd);

    _cxr = "BA";
    _n = "5870";
    _gd = GlobalDirection::AT;
    createPaxTypeFare(&_info2, _n, _cxr, _gd);

    _cxr = "BA";
    _n = "5871";
    _gd = GlobalDirection::PA;
    createPaxTypeFare(&_info3, _n, _cxr, _gd);

    _cxr = "BA";
    _n = "5870";
    _gd = GlobalDirection::SA;
    createPaxTypeFare(&_info6, _n, _cxr, _gd);

    _cxr = "YY";
    _n = "5873";
    _gd = GlobalDirection::AT;
    createPaxTypeFare(&_info5, _n, _cxr, _gd);

    _cxr = "YY";
    _n = "5872";
    _gd = GlobalDirection::SA;
    createPaxTypeFare(&_info4, _n, _cxr, _gd);

    _itin.geoTravelType() = GeoTravelType::International;

    _fare1.setFareInfo(&_info1);
    _fare2.setFareInfo(&_info2);
    _fare3.setFareInfo(&_info3);
    _fare4.setFareInfo(&_info4);
    _fare5.setFareInfo(&_info5);
    _fare6.setFareInfo(&_info6);

    _p1.initialize(&_fare1, &_p, &_fm);
    _p2.initialize(&_fare2, &_p, &_fm);
    _p3.initialize(&_fare3, &_p, &_fm);
    _p4.initialize(&_fare4, &_p, &_fm);
    _p5.initialize(&_fare5, &_p, &_fm);
    _p6.initialize(&_fare6, &_p, &_fm);

    _p1.fareDisplayInfo() = &_fdi1;
    _p2.fareDisplayInfo() = &_fdi2;
    _p3.fareDisplayInfo() = &_fdi3;
    _p4.fareDisplayInfo() = &_fdi4;
    _p5.fareDisplayInfo() = &_fdi5;
    _p6.fareDisplayInfo() = &_fdi6;

    _fm.allPaxTypeFare().push_back(&_p1);
    _fm.allPaxTypeFare().push_back(&_p2);
    _fm.allPaxTypeFare().push_back(&_p3);
    _fm.allPaxTypeFare().push_back(&_p4);
    _fm.allPaxTypeFare().push_back(&_p5);
    _fm.allPaxTypeFare().push_back(&_p6);
    _itin.fareMarket().push_back(&_fm);
    _fTrx.itin().push_back(&_itin);

    _aofi1.globalDir() = GlobalDirection::AT;
    _aofi1.routing() = "5873";
    _aofi1.carrier() = "AA";

    _aofi2.globalDir() = GlobalDirection::AT;
    _aofi2.routing() = "5874";
    _aofi2.carrier() = "AA";

    _aofi3.globalDir() = GlobalDirection::PA;
    _aofi3.routing() = "5875";
    _aofi3.carrier() = "AA";

    _aofi4.globalDir() = GlobalDirection::ZZ;
    _aofi4.routing() = "5876";
    _aofi4.carrier() = "AA";

    _fTrx.fdResponse() = &_fdResponse;
  }

  void test_generate()
  {
    std::list<PaxTypeFare*> fares;
    fares.push_back(&_p1);
    fares.push_back(&_p2);
    fares.push_back(&_p3);
    fares.push_back(&_p4);
    fares.push_back(&_p5);
    fares.push_back(&_p6);

    std::map<RoutingSequenceGenerator::RoutingSequence, std::string> generatedSequences;
    std::map<GlobalDirection, uint16_t> processedGlobals;

    _sequenceGen.generate(fares, generatedSequences, processedGlobals);

    // assert that generated "AT03" sequence string ( there are 3 AT GIs )
    // assert that generated "PA01" sequence string ( there is 1 PA GI
    CPPUNIT_ASSERT(std::find_if(generatedSequences.begin(),
                                generatedSequences.end(),
                                equalsTmpRtg("AT03")) != generatedSequences.end());

    CPPUNIT_ASSERT(std::find_if(generatedSequences.begin(),
                                generatedSequences.end(),
                                equalsTmpRtg("AT04")) == generatedSequences.end());

    CPPUNIT_ASSERT(std::find_if(generatedSequences.begin(),
                                generatedSequences.end(),
                                equalsTmpRtg("PA01")) != generatedSequences.end());

    CPPUNIT_ASSERT(std::find_if(generatedSequences.begin(),
                                generatedSequences.end(),
                                equalsTmpRtg("PA02")) == generatedSequences.end());

    CPPUNIT_ASSERT_EQUAL(uint16_t(3), processedGlobals[GlobalDirection::AT]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), processedGlobals[GlobalDirection::PA]);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), processedGlobals[GlobalDirection::SA]);
  }

  void test_generate_2()
  {
    std::list<PaxTypeFare*> fares;
    fares.push_back(&_p1);
    fares.push_back(&_p1);
    fares.push_back(&_p1);
    std::map<RoutingSequenceGenerator::RoutingSequence, std::string> generatedSequences;
    std::map<GlobalDirection, uint16_t> processedGlobals;

    // assert that only one sequence was created
    _sequenceGen.generate(fares, generatedSequences, processedGlobals);

    CPPUNIT_ASSERT_EQUAL(size_t(1), generatedSequences.size());
  }

  void test_generate_3()
  {
    std::vector<FDAddOnFareInfo*> addOnFareInfoList;
    addOnFareInfoList.push_back(&_aofi1);
    addOnFareInfoList.push_back(&_aofi2);
    addOnFareInfoList.push_back(&_aofi3);
    addOnFareInfoList.push_back(&_aofi4);

    // for DOMESTIC - addOnRoutingSeq is just copied from routing no.
    _sequenceGen.generate(addOnFareInfoList, DOMESTIC);

    // assert that size wasn't changed
    CPPUNIT_ASSERT_EQUAL(size_t(4), addOnFareInfoList.size());

    for (std::vector<FDAddOnFareInfo*>::iterator it = addOnFareInfoList.begin();
         it != addOnFareInfoList.end();
         ++it)
      CPPUNIT_ASSERT((*it) && (*it)->routing() == (*it)->addOnRoutingSeq());
  }

  void test_generate_4()
  {
    std::vector<FDAddOnFareInfo*> addOnFareInfoList;
    addOnFareInfoList.push_back(&_aofi1);
    addOnFareInfoList.push_back(&_aofi2);
    addOnFareInfoList.push_back(&_aofi3);
    addOnFareInfoList.push_back(&_aofi4);

    // for INTERNATIONAL - create AT01, AT02, PA01 and 5876 (for fare with GI == ZZ)
    _sequenceGen.generate(addOnFareInfoList, INTERNATIONAL);

    // assert that size wasn't changed
    CPPUNIT_ASSERT_EQUAL(size_t(4), addOnFareInfoList.size());

    for (std::vector<FDAddOnFareInfo*>::iterator it = addOnFareInfoList.begin();
         it != addOnFareInfoList.end();
         ++it)
      CPPUNIT_ASSERT((*it)->addOnRoutingSeq() == "AT01" || (*it)->addOnRoutingSeq() == "AT02" ||
                     (*it)->addOnRoutingSeq() == "PA01" || (*it)->addOnRoutingSeq() == "5876");
  }

  void test_getRoutingOrder()
  {
    std::string order;
    _sequenceGen.getRoutingOrder(order, 1);

    CPPUNIT_ASSERT_EQUAL(std::string("01"), order);

    _sequenceGen.getRoutingOrder(order, 10);
    CPPUNIT_ASSERT_EQUAL(std::string("10"), order);

    _sequenceGen.getRoutingOrder(order, 500);
    CPPUNIT_ASSERT_EQUAL(std::string("500"), order);
  }

  void test_getRTGSeq()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("AT12"), _sequenceGen.getRTGSeq(GlobalDirection::AT, 12));
    CPPUNIT_ASSERT_EQUAL(std::string("PA01"), _sequenceGen.getRTGSeq(GlobalDirection::PA, 1));
  }

  void createPaxTypeFare(FareInfo* info, RoutingNumber number, CarrierCode cxr, GlobalDirection gd)
  {
    info->routingNumber() = number;
    info->globalDirection() = gd;
    info->carrier() = cxr;
  }

private:
  struct equalsTmpRtg
  {
    equalsTmpRtg(const std::string& str) : tmpRtg(str) {}

    bool operator()(const std::pair<RoutingSequenceGenerator::RoutingSequence, std::string>& pair)
    {
      return pair.second == tmpRtg;
    }

  private:
    std::string tmpRtg;
  };

  class MockRoutingSequenceGenerator : public RoutingSequenceGenerator
  {
    friend class tse::RoutingSequenceGeneratorTest;
    friend struct equalsTmpRtg;
  };

  MockRoutingSequenceGenerator _sequenceGen;
  FareDisplayTrx _fTrx;
  Itin _itin;
  FareMarket _fm;
  PaxType _p;
  PaxTypeFare _p1, _p2, _p3, _p4, _p5, _p6;
  Fare _fare1, _fare2, _fare3, _fare4, _fare5, _fare6;
  FareInfo _info1, _info2, _info3, _info4, _info5, _info6;
  FareDisplayInfo _fdi1, _fdi2, _fdi3, _fdi4, _fdi5, _fdi6;
  CarrierCode _cxr;
  RoutingNumber _n;
  GlobalDirection _gd;
  FareDisplayResponse _fdResponse;
  FDAddOnFareInfo _aofi1, _aofi2, _aofi3, _aofi4;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RoutingSequenceGeneratorTest);

} // tse
