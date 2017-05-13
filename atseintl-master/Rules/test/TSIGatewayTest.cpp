//-------------------------------------------------------------------
////
////  Copyright Sabre 2009
////
////          The copyright to the computer program(s) herein
////          is the property of Sabre.
////          The program(s) may be used and/or copied only with
////          the written permission of Sabre or in accordance
////          with the terms and conditions stipulated in the
////          agreement/contract under which the program(s)
////          have been supplied.
////
////-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "Rules/TSIGateway.h"
#include <iostream>
#include "DBAccess/RuleItemInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/ATPResNationZones.h"

using namespace tse;
using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  ATPResNationZones* getResZone(NationCode nation, std::string zone1, std::string zone2 = "")
  {
    ATPResNationZones* ret = _memHandle.create<ATPResNationZones>();
    ret->nation() = nation;
    ret->zones().push_back(zone1);
    if (!zone2.empty())
      ret->zones().push_back(zone2);
    return ret;
  }

public:
  const std::vector<ATPResNationZones*>& getATPResNationZones(const NationCode& key)
  {
    std::vector<ATPResNationZones*>& ret = *_memHandle.create<std::vector<ATPResNationZones*> >();
    if (key == "AG")
    {
      ret.push_back(getResZone(key, "0000140"));
      return ret;
    }
    else if (key == "BR")
    {
      ret.push_back(getResZone(key, "0000170", "0000171"));
      return ret;
    }
    else if (key == "CA")
    {
      ret.push_back(getResZone(key, "0000000", "0000002"));
      return ret;
    }
    else if (key == "MX")
    {
      ret.push_back(getResZone(key, "0000000", "0000009"));
      return ret;
    }
    else if (key == "US")
    {
      ret.push_back(getResZone(key, "0000000", "0000001"));
      return ret;
    }
    return DataHandleMock::getATPResNationZones(key);
  }
};
}

class TSIGatewayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TSIGatewayTest);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is1st_WhenTravelArea1To2To2To3);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is2nd_WhenTravelArea1To1To2To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is1st_WhenTravelArea1To2To2To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is3rd_WhenTravelArea1To1To1To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is1st_WhenTravelNation1To2To2To3);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is2nd_WhenTravelNation1To1To2To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is1st_WhenTravelNation1To2To2To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is3rd_WhenTravelNation1To1To1To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is1st_WhenTravelZone1To2To2To3);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is2nd_WhenTravelZone1To1To2To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is1st_WhenTravelZone1To2To2To2);
  CPPUNIT_TEST(testtsDepartFromOrigGW_is3rd_WhenTravelZone1To1To1To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is3rd_WhenTravelArea1To2To2To3);
  CPPUNIT_TEST(testtsArriveOnDestGW_is2nd_WhenTravelArea1To1To2To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is1st_WhenTravelArea1To2To2To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is3rd_WhenTravelArea1To1To1To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is3rd_WhenTravelNation1To2To2To3);
  CPPUNIT_TEST(testtsArriveOnDestGW_is2nd_WhenTravelNation1To1To2To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is1st_WhenTravelNation1To2To2To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is3rd_WhenTravelNation1To1To1To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is3rd_WhenTravelZone1To2To2To3);
  CPPUNIT_TEST(testtsArriveOnDestGW_is2nd_WhenTravelZone1To1To2To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is1st_WhenTravelZone1To2To2To2);
  CPPUNIT_TEST(testtsArriveOnDestGW_is3rd_WhenTravelZone1To1To1To2);
  CPPUNIT_TEST(testisDepartureFromGW_1st2nd3rd_WhenTravelArea1To2To2To3);
  CPPUNIT_TEST(testisDepartureFromGW_2nd3rd_WhenTravelArea1To1To2To2);
  CPPUNIT_TEST(testisDepartureFromGW_1st2nd_WhenTravelArea1To2To2To2);
  CPPUNIT_TEST(testisDepartureFromGW_3rd_WhenTravelArea1To1To1To2);
  CPPUNIT_TEST(testisDepartureFromGW_1st2nd3rd_WhenTravelNation1To2To2To3);
  CPPUNIT_TEST(testisDepartureFromGW_2nd3rd_WhenTravelNation1To1To2To2);
  CPPUNIT_TEST(testisDepartureFromGW_1st2nd_WhenTravelNation1To2To2To2);
  CPPUNIT_TEST(testisDepartureFromGW_3rd_WhenTravelNation1To1To1To2);
  CPPUNIT_TEST(testisDepartureFromGW_1st2nd3rd_WhenTravelZone1To2To2To3);
  CPPUNIT_TEST(testisDepartureFromGW_2nd3rd_WhenTravelZone1To1To2To2);
  CPPUNIT_TEST(testisDepartureFromGW_1st2nd_WhenTravelZone1To2To2To2);
  CPPUNIT_TEST(testisDepartureFromGW_3rd_WhenTravelZone1To1To1To2);
  CPPUNIT_TEST(testisArrivalOnGW_1st2nd3rd_WhenTravelArea1To2To2To3);
  CPPUNIT_TEST(testisArrivalOnGW_1st2nd_WhenTravelArea1To1To2To2);
  CPPUNIT_TEST(testisArrivalOnGW_1st_WhenTravelArea1To2To2To2);
  CPPUNIT_TEST(testisArrivalOnGW_2nd3rd_WhenTravelArea1To1To1To2);
  CPPUNIT_TEST(testisArrivalOnGW_1st2nd3rd_WhenTravelNation1To2To2To3);
  CPPUNIT_TEST(testisArrivalOnGW_1st2nd_WhenTravelNation1To1To2To2);
  CPPUNIT_TEST(testisArrivalOnGW_1st_WhenTravelNation1To2To2To2);
  CPPUNIT_TEST(testisArrivalOnGW_2nd3rd_WhenTravelNation1To1To1To2);
  CPPUNIT_TEST(testisArrivalOnGW_1st2nd3rd_WhenTravelZone1To2To2To3);
  CPPUNIT_TEST(testisArrivalOnGW_1st2nd_WhenTravelZone1To1To2To2);
  CPPUNIT_TEST(testisArrivalOnGW_1st_WhenTravelZone1To2To2To2);
  CPPUNIT_TEST(testisArrivalOnGW_2nd3rd_WhenTravelZone1To1To1To2);
  CPPUNIT_TEST(testCombineInboundGW_OutZone1To2To2To3_InZone3To1);
  CPPUNIT_TEST(testCombineInboundGW_OutZone2To1_InZone1To1To2To2);
  CPPUNIT_TEST(testCombineInboundGW_OutZone1To1_InZone1To1To2To2);
  CPPUNIT_TEST(testRtw_A1_A2_A2_A2);
  CPPUNIT_TEST(testRtw_A1_A2_A2_A3);
  CPPUNIT_TEST(testRtw_A1_A2_A2_A1);
  CPPUNIT_TEST(testRtw_Z1_Z1_Z2_Z2);
  CPPUNIT_TEST(testRtw_Z1_Z2_Z1_Z2);
  CPPUNIT_TEST(testRtw_N1_N1_N2_N2);
  CPPUNIT_TEST(testRtw_N1_N1_N1_N1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    buildTravelSegs();
    buildTravel1234();
    _tsiGW = _memHandle.create<TSIGateway>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _travel.clear(); }

  void buildTravelSegs()
  {
    _tvlSeg1 = _memHandle.create<AirSeg>();
    _tvlSeg1->origin() = &_loc1;
    _tvlSeg1->destination() = &_loc2;

    _tvlSeg2 = _memHandle.create<AirSeg>();
    _tvlSeg2->origin() = &_loc2;
    _tvlSeg2->destination() = &_loc3;

    _tvlSeg3 = _memHandle.create<AirSeg>();
    _tvlSeg3->origin() = &_loc3;
    _tvlSeg3->destination() = &_loc4;
  }

  void buildTravel1234()
  {
    _travel.push_back(_tvlSeg1);
    _travel.push_back(_tvlSeg2);
    _travel.push_back(_tvlSeg3);
  }

  void setTravelAreas(const IATAAreaCode& a1,
                      const IATAAreaCode& a2,
                      const IATAAreaCode& a3,
                      const IATAAreaCode& a4)
  {
    _loc1.area() = a1;
    _loc2.area() = a2;
    _loc3.area() = a3;
    _loc4.area() = a4;
  }

  void setTravelNations(const NationCode& a1,
                        const NationCode& a2,
                        const NationCode& a3,
                        const NationCode& a4)
  {
    setTravelAreas("1", "1", "1", "1");
    _loc1.nation() = a1;
    _loc2.nation() = a2;
    _loc3.nation() = a3;
    _loc4.nation() = a4;
  }

  void setTravelOneZoneNation1To2To2To3() { setTravelNations("CA", "US", "US", "MX"); }
  void setTravelOneZoneNation1To1To2To2() { setTravelNations("CA", "CA", "US", "US"); }
  void setTravelOneZoneNation1To2To2To2() { setTravelNations("CA", "US", "US", "US"); }
  void setTravelOneZoneNation1To1To1To2() { setTravelNations("CA", "CA", "CA", "US"); }
  void setTravelZone1To2To2To3() { setTravelNations("US", "BR", "BR", "AG"); }
  void setTravelZone1To1To2To2() { setTravelNations("US", "US", "BR", "BR"); }
  void setTravelZone1To2To2To2() { setTravelNations("US", "BR", "BR", "BR"); }
  void setTravelZone1To1To1To2() { setTravelNations("US", "US", "US", "BR"); }

  void testtsDepartFromOrigGW_is1st_WhenTravelArea1To2To2To3()
  {
    setTravelAreas("1", "2", "2", "3");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is2nd_WhenTravelArea1To1To2To2()
  {
    setTravelAreas("1", "1", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is1st_WhenTravelArea1To2To2To2()
  {
    setTravelAreas("1", "2", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is3rd_WhenTravelArea1To1To1To2()
  {
    setTravelAreas("1", "1", "1", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsArriveOnDestGW_is3rd_WhenTravelArea1To2To2To3()
  {
    setTravelAreas("1", "2", "2", "3");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is2nd_WhenTravelArea1To1To2To2()
  {
    setTravelAreas("1", "1", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is1st_WhenTravelArea1To2To2To2()
  {
    setTravelAreas("1", "2", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is3rd_WhenTravelArea1To1To1To2()
  {
    setTravelAreas("1", "1", "1", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsArriveOnDestGW());
  }

  void testisDepartureFromGW_1st2nd3rd_WhenTravelArea1To2To2To3()
  {
    setTravelAreas("1", "2", "2", "3");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_2nd3rd_WhenTravelArea1To1To2To2()
  {
    setTravelAreas("1", "1", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_1st2nd_WhenTravelArea1To2To2To2()
  {
    setTravelAreas("1", "2", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_3rd_WhenTravelArea1To1To1To2()
  {
    setTravelAreas("1", "1", "1", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st2nd3rd_WhenTravelArea1To2To2To3()
  {
    setTravelAreas("1", "2", "2", "3");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st2nd_WhenTravelArea1To1To2To2()
  {
    setTravelAreas("1", "1", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st_WhenTravelArea1To2To2To2()
  {
    setTravelAreas("1", "2", "2", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_2nd3rd_WhenTravelArea1To1To1To2()
  {
    setTravelAreas("1", "1", "1", "2");
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testtsDepartFromOrigGW_is1st_WhenTravelNation1To2To2To3()
  {
    setTravelOneZoneNation1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is2nd_WhenTravelNation1To1To2To2()
  {
    setTravelOneZoneNation1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is1st_WhenTravelNation1To2To2To2()
  {
    setTravelOneZoneNation1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is3rd_WhenTravelNation1To1To1To2()
  {
    setTravelOneZoneNation1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsArriveOnDestGW_is3rd_WhenTravelNation1To2To2To3()
  {
    setTravelOneZoneNation1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is2nd_WhenTravelNation1To1To2To2()
  {
    setTravelOneZoneNation1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is1st_WhenTravelNation1To2To2To2()
  {
    setTravelOneZoneNation1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is3rd_WhenTravelNation1To1To1To2()
  {
    setTravelOneZoneNation1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsArriveOnDestGW());
  }

  void testisDepartureFromGW_1st2nd3rd_WhenTravelNation1To2To2To3()
  {
    setTravelOneZoneNation1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_2nd3rd_WhenTravelNation1To1To2To2()
  {
    setTravelOneZoneNation1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_1st2nd_WhenTravelNation1To2To2To2()
  {
    setTravelOneZoneNation1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_3rd_WhenTravelNation1To1To1To2()
  {
    setTravelOneZoneNation1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st2nd3rd_WhenTravelNation1To2To2To3()
  {
    setTravelOneZoneNation1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st2nd_WhenTravelNation1To1To2To2()
  {
    setTravelOneZoneNation1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st_WhenTravelNation1To2To2To2()
  {
    setTravelOneZoneNation1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_2nd3rd_WhenTravelNation1To1To1To2()
  {
    setTravelOneZoneNation1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testtsDepartFromOrigGW_is1st_WhenTravelZone1To2To2To3()
  {
    setTravelZone1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is2nd_WhenTravelZone1To1To2To2()
  {
    setTravelZone1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is1st_WhenTravelZone1To2To2To2()
  {
    setTravelZone1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsDepartFromOrigGW_is3rd_WhenTravelZone1To1To1To2()
  {
    setTravelZone1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsDepartFromOrigGW());
  }

  void testtsArriveOnDestGW_is3rd_WhenTravelZone1To2To2To3()
  {
    setTravelZone1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is2nd_WhenTravelZone1To1To2To2()
  {
    setTravelZone1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is1st_WhenTravelZone1To2To2To2()
  {
    setTravelZone1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsArriveOnDestGW());
  }

  void testtsArriveOnDestGW_is3rd_WhenTravelZone1To1To1To2()
  {
    setTravelZone1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT_EQUAL(_tvlSeg3, _tsiGW->tsArriveOnDestGW());
  }

  void testisDepartureFromGW_1st2nd3rd_WhenTravelZone1To2To2To3()
  {
    setTravelZone1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_2nd3rd_WhenTravelZone1To1To2To2()
  {
    setTravelZone1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_1st2nd_WhenTravelZone1To2To2To2()
  {
    setTravelZone1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisDepartureFromGW_3rd_WhenTravelZone1To1To1To2()
  {
    setTravelZone1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg1));
    CPPUNIT_ASSERT(!_tsiGW->isDepartureFromGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st2nd3rd_WhenTravelZone1To2To2To3()
  {
    setTravelZone1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st2nd_WhenTravelZone1To1To2To2()
  {
    setTravelZone1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_1st_WhenTravelZone1To2To2To2()
  {
    setTravelZone1To2To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testisArrivalOnGW_2nd3rd_WhenTravelZone1To1To1To2()
  {
    setTravelZone1To1To1To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);
    CPPUNIT_ASSERT(!_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));
  }

  void testCombineInboundGW_OutZone1To2To2To3_InZone3To1()
  {
    setTravelZone1To2To2To3();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);

    std::vector<TravelSeg*> inboundTvl;
    AirSeg tvlSeg4;
    tvlSeg4.origin() = &_loc4;
    tvlSeg4.destination() = &_loc1;
    inboundTvl.push_back(&tvlSeg4);
    TSIGateway tsiGWIn;
    CPPUNIT_ASSERT(tsiGWIn.markGW(TSIGateway::MARK_ALL_GATEWAY, inboundTvl));
    _tsiGW->combineInboundGW(tsiGWIn);

    CPPUNIT_ASSERT(_tsiGW->isDepartureFromGW(&tvlSeg4));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(&tvlSeg4));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(_tsiGW->isArrivalOnGW(_tvlSeg3));

    CPPUNIT_ASSERT_EQUAL(_tvlSeg1, _tsiGW->tsDepartFromOrigGW());
    CPPUNIT_ASSERT_EQUAL((TravelSeg*)&tvlSeg4, _tsiGW->tsArriveOnDestGW());
  }

  void testCombineInboundGW_OutZone2To1_InZone1To1To2To2()
  {
    setTravelZone1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);

    std::vector<TravelSeg*> outboundTvl;
    AirSeg tvlSeg4;
    tvlSeg4.origin() = &_loc4;
    tvlSeg4.destination() = &_loc1;
    outboundTvl.push_back(&tvlSeg4);

    TSIGateway tsiGWOut;
    CPPUNIT_ASSERT(tsiGWOut.markGW(TSIGateway::MARK_ALL_GATEWAY, outboundTvl));

    tsiGWOut.combineInboundGW(*_tsiGW);

    CPPUNIT_ASSERT(tsiGWOut.isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(tsiGWOut.isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!tsiGWOut.isArrivalOnGW(_tvlSeg3));
    CPPUNIT_ASSERT_EQUAL((TravelSeg*)&tvlSeg4, tsiGWOut.tsDepartFromOrigGW());
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, tsiGWOut.tsArriveOnDestGW());
  }

  void testCombineInboundGW_OutZone1To1_InZone1To1To2To2()
  {
    setTravelZone1To1To2To2();
    _tsiGW->markGW(TSIGateway::MARK_ALL_GATEWAY, _travel);

    std::vector<TravelSeg*> outboundTvl;
    AirSeg tvlSeg4;
    Loc loc5;
    loc5.area() = _loc1.area();
    loc5.nation() = _loc1.nation();

    tvlSeg4.origin() = &loc5;
    tvlSeg4.destination() = &_loc1;
    outboundTvl.push_back(&tvlSeg4);

    TSIGateway tsiGWOut;
    CPPUNIT_ASSERT(!tsiGWOut.markGW(TSIGateway::MARK_ALL_GATEWAY,
                                    outboundTvl)); // should have no gateway at outbound

    tsiGWOut.combineInboundGW(*_tsiGW);

    CPPUNIT_ASSERT(tsiGWOut.isArrivalOnGW(_tvlSeg1));
    CPPUNIT_ASSERT(tsiGWOut.isArrivalOnGW(_tvlSeg2));
    CPPUNIT_ASSERT(!tsiGWOut.isArrivalOnGW(_tvlSeg3));
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, tsiGWOut.tsDepartFromOrigGW());
    CPPUNIT_ASSERT_EQUAL(_tvlSeg2, tsiGWOut.tsArriveOnDestGW());
  }

  void validateDepartureGw(bool isGw1, bool isGw2, bool isGw3)
  {
    CPPUNIT_ASSERT_EQUAL(isGw1, _tsiGW->isDepartureFromGW(_travel[0]));
    CPPUNIT_ASSERT_EQUAL(isGw2, _tsiGW->isDepartureFromGW(_travel[1]));
    CPPUNIT_ASSERT_EQUAL(isGw3, _tsiGW->isDepartureFromGW(_travel[2]));
  }

  void validateArrivalGw(bool isGw1, bool isGw2, bool isGw3)
  {
    CPPUNIT_ASSERT_EQUAL(isGw1, _tsiGW->isArrivalOnGW(_travel[0]));
    CPPUNIT_ASSERT_EQUAL(isGw2, _tsiGW->isArrivalOnGW(_travel[1]));
    CPPUNIT_ASSERT_EQUAL(isGw3, _tsiGW->isArrivalOnGW(_travel[2]));
  }

  void testRtw_A1_A2_A2_A2()
  {
    setTravelAreas("1", "2", "2", "2");
    CPPUNIT_ASSERT(_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
    validateDepartureGw(true, true, false);
    validateArrivalGw(true, false, false);
  }

  void testRtw_A1_A2_A2_A3()
  {
    setTravelAreas("1", "2", "2", "3");
    CPPUNIT_ASSERT(_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
    validateDepartureGw(true, true, true);
    validateArrivalGw(true, true, true);
  }

  void testRtw_A1_A2_A2_A1()
  {
    setTravelAreas("1", "2", "2", "1");
    CPPUNIT_ASSERT(_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
    validateDepartureGw(true, true, true);
    validateArrivalGw(true, true, true);
  }

  void testRtw_Z1_Z1_Z2_Z2()
  {
    setTravelNations("US", "CA", "BR", "BR");
    CPPUNIT_ASSERT(_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
    validateDepartureGw(false, true, true);
    validateArrivalGw(true, true, false);
  }

  void testRtw_Z1_Z2_Z1_Z2()
  {
    setTravelNations("US", "BR", "CA", "BR");
    CPPUNIT_ASSERT(_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
    validateDepartureGw(true, true, true);
    validateArrivalGw(true, true, true);
  }

  void testRtw_N1_N1_N2_N2()
  {
    setTravelNations("US", "US", "CA", "CA");
    CPPUNIT_ASSERT(_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
    validateDepartureGw(false, true, true);
    validateArrivalGw(true, true, false);
  }

  void testRtw_N1_N1_N1_N1()
  {
    setTravelNations("US", "US", "US", "US");
    CPPUNIT_ASSERT(!_tsiGW->markGwRtw(TSIGateway::MARK_ALL_GATEWAY, _travel));
  }

private:
  Loc _loc1, _loc2, _loc3, _loc4;
  TravelSeg* _tvlSeg1, *_tvlSeg2, *_tvlSeg3;
  std::vector<TravelSeg*> _travel;
  TSIGateway* _tsiGW;
  TestMemHandle _memHandle;
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(TSIGatewayTest);
