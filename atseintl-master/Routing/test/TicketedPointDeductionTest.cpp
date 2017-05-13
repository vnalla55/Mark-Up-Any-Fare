#include "Routing/TicketedPointDeduction.h"
#include "Routing/MileageRouteItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TpdPsr.h"
#include "test/testdata/TestTpdPsrViaGeoLocFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTpdPsrViaCxrLocFactory.h"
#include "test/testdata/TestTpdPsrViaExceptFactory.h"
#include "Routing/FareCalcVisitor.h"
#include "test/include/CppUnitHelperMacros.h"
#include <iosfwd>
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
std::ostream& operator<<(std::ostream& os, const TpdPsrViaGeoLoc* l)
{
  os << "setNo " << l->setNo() << " orderNo " << l->orderNo() << " loc " << l->loc().loc()
     << " rel " << l->relationalInd() << " noStop " << l->stopoverNotAllowed() << std::endl;
  return os;
}
std::ostream& operator<<(std::ostream& os, const TpdPsrViaCxrLoc* l)
{
  os << "loc1 " << l->loc1().loc() << " loc2 " << l->loc2().loc() << " cxr " << l->viaCarrier()
     << std::endl;
  return os;
}
std::ostream& operator<<(std::ostream& os, const TpdPsrViaExcept* l)
{
  os << "loc1 " << l->loc1().loc() << " loc2 " << l->loc2().loc() << " cxr " << l->viaCarrier()
     << std::endl;
  return os;
}
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  TpdPsr* getTPD(LocCode loc1,
                 LocCode loc2,
                 int tpmDeduction = 100,
                 GlobalDirection gd = GlobalDirection::AT)
  {
    TpdPsr* ret = _memHandle.create<TpdPsr>();
    ret->loc1().locType() = 'C';
    ret->loc1().loc() = loc1;
    ret->loc2().locType() = 'C';
    ret->loc2().loc() = loc2;
    ret->tpmDeduction() = tpmDeduction;
    ret->globalDir() = gd;
    return ret;
  }

public:
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "LAX")
      return "LAX";
    else if (locCode == "CHI")
      return "CHI";
    else if (locCode == "NYC")
      return "NYC";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                        const CarrierCode& carrier,
                                        Indicator area1,
                                        Indicator area2,
                                        const DateTime& date)
  {
    std::vector<TpdPsr*>& ret = *_memHandle.create<std::vector<TpdPsr*> >();
    if (carrier == "EM")
      return ret;
    else if (carrier == "NO")
    {
      ret.push_back(getTPD("DEN", "SEA"));
      return ret;
    }
    else if (carrier == "MA")
    {
      ret.push_back(getTPD("BOS", "LAX"));
      return ret;
    }
    else if (carrier == "2M")
    {
      ret.push_back(getTPD("BOS", "LAX"));
      ret.push_back(getTPD("BOS", "LAX", 1000, GlobalDirection::PA));
      return ret;
    }
    else if (carrier == "BG")
    {
      ret.push_back(getTPD("BOS", "LAX", 100, GlobalDirection::PA));
      return ret;
    }
    return DataHandleMock::getTpdPsr(applInd, carrier, area1, area2, date);
  }
};
}

static const std::string routingTestDataPath = "/vobs/atseintl/Routing/test/data/";
static const std::string globalTestDataPath = "/vobs/atseintl/test/testdata/data/";

class TicketedPointDeductionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketedPointDeductionTest);
  CPPUNIT_SKIP_TEST(testProcessViaGeoLocs);
  CPPUNIT_TEST(testProcessThruMktCxrs);
  CPPUNIT_TEST(testProcessThruMktCxrs_withSurface);
  CPPUNIT_TEST(testProcessViaCxrLocs);
  CPPUNIT_TEST(testProcessViaExcepts);
  CPPUNIT_TEST(testProcessSubRoute);
  CPPUNIT_TEST(testApply);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<MyDataHandle>(); }
  void tearDown() { _memHandle.clear(); }

  void testProcessViaGeoLocs()
  {
    MileageRouteItems items;
    Loc* bos(TestLocFactory::create(globalTestDataPath + "LocBOS.xml", true));
    CPPUNIT_ASSERT(bos != 0);
    Loc* nyc(TestLocFactory::create(globalTestDataPath + "LocJFK.xml", true));
    CPPUNIT_ASSERT(nyc != 0);
    nyc->loc() = "NYC";
    Loc* chi(TestLocFactory::create(globalTestDataPath + "LocORD.xml", true));
    CPPUNIT_ASSERT(chi != 0);
    chi->loc() = "CHI";
    Loc* lax(TestLocFactory::create(globalTestDataPath + "LocLAX.xml", true));
    CPPUNIT_ASSERT(lax != 0);
    MileageRouteItem bosnyc, nycchi, chilax;
    bosnyc.city1() = bos;
    bosnyc.city2() = nyc;
    bosnyc.isStopover() = false;
    nycchi.city1() = nyc;
    nycchi.city2() = chi;
    chilax.city1() = chi;
    chilax.city2() = lax;
    items.push_back(bosnyc);
    items.push_back(nycchi);
    items.push_back(chilax);
    // SPR 107547
    /* BACKED OUT BECAUSE OF NEGATIVE IMPACT!!
       TpdPsrViaGeoLoc *l11NYCNN = TestTpdPsrViaGeoLocFactory::create(routingTestDataPath +
       "TpdPsrViaGeoLoc_11NYCNN.xml");
       CPPUNIT_ASSERT(l11NYCNN != 0);
       TpdPsrViaGeoLoc *l12CHIAON= TestTpdPsrViaGeoLocFactory::create(routingTestDataPath +
       "TpdPsrViaGeoLoc_12CHIAON.xml");
       CPPUNIT_ASSERT(l12CHIAON != 0);
       TpdPsrViaGeoLoc *l12ATLAON= TestTpdPsrViaGeoLocFactory::create(routingTestDataPath +
       "TpdPsrViaGeoLoc_12ATLAON.xml");
       CPPUNIT_ASSERT(l12ATLAON != 0);
       TpdPsrViaGeoLoc *l11ATLNN = TestTpdPsrViaGeoLocFactory::create(routingTestDataPath +
       "TpdPsrViaGeoLoc_11ATLNN.xml");
       CPPUNIT_ASSERT(l11ATLNN != 0);

    */
    TpdPsrViaGeoLoc* l12NYCAN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_12NYCAN.xml", true);
    CPPUNIT_ASSERT(l12NYCAN != 0);
    TpdPsrViaGeoLoc* l11CHINN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_11CHINN.xml", true);
    CPPUNIT_ASSERT(l11CHINN != 0);
    TpdPsrViaGeoLoc* l11BOSNN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_11BOSNN.xml", true);
    CPPUNIT_ASSERT(l11BOSNN != 0);
    TpdPsrViaGeoLoc* l12LAXAN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_12LAXAN.xml", true);
    CPPUNIT_ASSERT(l12LAXAN != 0);
    TpdPsrViaGeoLoc* l12DENON = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_12DENON.xml", true);
    CPPUNIT_ASSERT(l12DENON != 0);
    TpdPsrViaGeoLoc* l11DENNN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_11DENNN.xml", true);
    CPPUNIT_ASSERT(l11DENNN != 0);
    TpdPsrViaGeoLoc* l12NYCON = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_12NYCON.xml", true);
    CPPUNIT_ASSERT(l12NYCON != 0);
    TpdPsrViaGeoLoc* l12DENAON = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_12DENAON.xml", true);
    CPPUNIT_ASSERT(l12DENAON != 0);
    TpdPsrViaGeoLoc* l12NYCAON = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_12NYCAON.xml", true);
    CPPUNIT_ASSERT(l12NYCAON != 0);
    TpdPsrViaGeoLoc* l21DENNN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_21DENNN.xml", true);
    CPPUNIT_ASSERT(l21DENNN != 0);
    TpdPsrViaGeoLoc* l21CHINN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_21CHINN.xml", true);
    CPPUNIT_ASSERT(l21CHINN != 0);
    TpdPsr tpdPsr;
    tpdPsr.viaGeoLocs().push_back(l12NYCAN);
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    DataHandle dataHandle;
    TicketedPointDeduction tpd;
    FareCalcVisitor visitor;
    l11CHINN->stopoverNotAllowed() = STPOVRNOTALWD_YES;
    CPPUNIT_ASSERT(!tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    l11CHINN->stopoverNotAllowed() = ' ';
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11BOSNN);
    tpdPsr.viaGeoLocs().push_back(l12NYCAN);
    CPPUNIT_ASSERT(!tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l12LAXAN);
    CPPUNIT_ASSERT(!tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l12DENON);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11DENNN);
    tpdPsr.viaGeoLocs().push_back(l12NYCON);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11BOSNN);
    tpdPsr.viaGeoLocs().push_back(l12DENON);
    CPPUNIT_ASSERT(!tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l12NYCON);
    tpdPsr.tpdThruViaMktOnlyInd() = THRUVIAMKTONLY_NO;
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11BOSNN);
    tpdPsr.viaGeoLocs().push_back(l12DENAON);
    CPPUNIT_ASSERT(!tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11DENNN);
    tpdPsr.viaGeoLocs().push_back(l12NYCAON);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l12DENAON);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l12NYCAON);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l21CHINN);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11CHINN);
    tpdPsr.viaGeoLocs().push_back(l21DENNN);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11DENNN);
    tpdPsr.viaGeoLocs().push_back(l21CHINN);
    CPPUNIT_ASSERT(tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.viaGeoLocs().push_back(l11DENNN);
    tpdPsr.viaGeoLocs().push_back(l21DENNN);
    CPPUNIT_ASSERT(!tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor, true));

    //---------------  SPR 107547 ----------------------------------------
    /* BACKED OUT BECAUSE OF NEGATIVE IMPACT!!


     bool responseArray[] = {
    //  Has stops     T-T    T-F     F-T   F-F

    false, false, false,  true,
    false, false, false,  true,
    true,  true,  true,   true,
    true,  true,  true,   true,
    false, false, true,   true,
    false, false, true,   true };

    std::string message, msgout;
    std::stringstream msg2;
    bool ret;
    tpdPsr.viaGeoLocs().clear();
    l11NYCNN->stopoverNotAllowed() = STPOVRNOTALWD_YES;
    l12CHIAON->stopoverNotAllowed() = STPOVRNOTALWD_YES;

    tpdPsr.viaGeoLocs().push_back(l11NYCNN);
    tpdPsr.viaGeoLocs().push_back(l12CHIAON);
    message = "\nNYC w StopNAllowed  and/or  CHI with StopNAllowed\n";

    int testIndex = 0;
    int count = 1;
    while (count <=6)

    {
      items[0].isStopover() = true;    // There is a Stop at NYC
      items[1].isStopover() = true;   //  There is a Stop at CHI
      ret = tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor);
      std::stringstream msg2;
  //    msg2.seekp(0);
      msg2 <<  testIndex << "NYC Stop  CHI Stop   Return =     " << ret << std::endl;
      msgout = message +  msg2.str();
      CPPUNIT_ASSERT_MESSAGE(msgout, responseArray[testIndex] == ret);

      testIndex++;


      items[0].isStopover() = true;    // There is a Stop at NYC
      items[1].isStopover() = false;   // tHERE IS not a Stop at CHI
      ret = tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor);
  //    msg2.seekp(0);
      msg2 <<  testIndex << "NYC Stop  CHI NonStop   Return =     " << ret << std::endl;
      msgout = message +  msg2.str();
      CPPUNIT_ASSERT_MESSAGE(msgout, responseArray[testIndex] == ret);
      testIndex++;


      items[0].isStopover() = false;    // There is a Not Stop at NYC
      items[1].isStopover() = true;   //  There is a Stop at CHI
      ret = tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor);
  //    msg2.seekp(0);
      msg2  << testIndex << "NYC NonStop  CHI Stop   Return =     " << ret << std::endl;
      msgout = message +  msg2.str();
      CPPUNIT_ASSERT_MESSAGE(msgout, responseArray[testIndex] == ret);
      testIndex++;


      items[0].isStopover() = false;    // There is a Not Stop at NYC
      items[1].isStopover() = false;   //  There is a Not Stop at CHI
      ret = tpd.processViaGeoLocs(items, tpdPsr, dataHandle, &visitor);
 //     msg2.seekp(0);
      msg2  << testIndex << "NYC NonStop  CHI NonStop   Return =     " << ret << std::endl;
      msgout = message +  msg2.str();
      CPPUNIT_ASSERT_MESSAGE(msgout, responseArray[testIndex] == ret);
      testIndex++;


      count++;

      if(count == 2)
      {
        tpdPsr.viaGeoLocs().clear();
        l11CHINN->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        l12NYCAON->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        tpdPsr.viaGeoLocs().push_back(l11CHINN);
        tpdPsr.viaGeoLocs().push_back(l12NYCAON);
      //  message.empty();
        message = "\nCHI w StopNAllowed  and/or  NYC with StopNAllowed \n";

      }

      else if(count == 3)
      {
        tpdPsr.viaGeoLocs().clear();
        l11NYCNN->stopoverNotAllowed() = ' ';
        l12CHIAON->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        tpdPsr.viaGeoLocs().push_back(l11NYCNN);
        tpdPsr.viaGeoLocs().push_back(l12CHIAON);
  //      message.seekp(0);
        message =  "\nNYC w Allowed  and/or  CHI with StopNAllowed\n";
      }
      else if(count == 4)
      {
        tpdPsr.viaGeoLocs().clear();
        l11NYCNN->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        l12CHIAON->stopoverNotAllowed() = ' ';
        tpdPsr.viaGeoLocs().push_back(l11NYCNN);
        tpdPsr.viaGeoLocs().push_back(l12CHIAON);
  //      message.seekp(0);
        message =  "\nNYC w StopNAllowed  and/or  CHI with Allowed\n";
      }

      else if(count == 5)
      {
        tpdPsr.viaGeoLocs().clear();
        l11NYCNN->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        l12ATLAON->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        tpdPsr.viaGeoLocs().push_back(l11NYCNN);
        tpdPsr.viaGeoLocs().push_back(l12ATLAON);
   //     message.seekp(0);
        message = "\nNYC w StopNAllowed  and/or  ATL with StopNAllowed\n";

      }
      else if(count == 6)
      {
        tpdPsr.viaGeoLocs().clear();
        l11ATLNN->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        l12NYCAON->stopoverNotAllowed() = STPOVRNOTALWD_YES;
        tpdPsr.viaGeoLocs().push_back(l11ATLNN);
        tpdPsr.viaGeoLocs().push_back(l12NYCAON);
   //     message.seekp(0);
        message = "\nATL w StopNAllowed  and/or  NYC with StopNAllowed\n";


      }

    }      // end of while

 */
  }

  void testProcessThruMktCxrs()
  {
    MileageRouteItems items;
    TpdPsr tpdPsr;
    CarrierCode governingCarrier("BA");
    TicketedPointDeduction tpd;
    CPPUNIT_ASSERT(tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    tpdPsr.carrier() = "BA";
    tpdPsr.thruMktCxrs().push_back("AF");
    tpdPsr.thruViaMktSameCxr() = THRUVIAMKTSAMECXR_NO;
    CPPUNIT_ASSERT(tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    tpdPsr.carrier() = "";
    tpdPsr.thruMktCarrierExcept() = THRUMKTCXREXCEPT_YES;
    CPPUNIT_ASSERT(tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    tpdPsr.thruMktCarrierExcept() = THRUMKTCXREXCEPT_NO;
    CPPUNIT_ASSERT(!tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    tpdPsr.thruMktCxrs().clear();
    tpdPsr.thruMktCxrs().push_back("BA");
    CPPUNIT_ASSERT(tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    tpdPsr.thruMktCarrierExcept() = THRUMKTCXREXCEPT_YES;
    CPPUNIT_ASSERT(!tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    tpdPsr.thruViaMktSameCxr() = THRUVIAMKTSAMECXR_YES;
    tpdPsr.thruMktCarrierExcept() = THRUMKTCXREXCEPT_NO;
    MileageRouteItem item1, item2;
    item1.segmentCarrier() = "AF";
    item2.segmentCarrier() = "BA";
    items.push_back(item1);
    items.push_back(item2);
    CPPUNIT_ASSERT(!tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    items.clear();
    items.push_back(item2);
    items.push_back(item1);
    CPPUNIT_ASSERT(!tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    items.clear();
    items.push_back(item2);
    items.push_back(item2);
    CPPUNIT_ASSERT(tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
  }

  void testProcessThruMktCxrs_withSurface()
  {
    MileageRouteItems items;
    TpdPsr tpdPsr;
    CarrierCode governingCarrier("AA");
    TicketedPointDeduction tpd;
    tpdPsr.carrier() = "AA";
    tpdPsr.thruMktCxrs().push_back("AF");
    tpdPsr.thruViaMktSameCxr() = THRUVIAMKTSAMECXR_NO;
    tpdPsr.thisCarrierRestr() = 'Y';
    MileageRouteItem item1, item2, item3;
    item1.segmentCarrier() = "AA";
    item2.segmentCarrier() = "AA";
    item3.segmentCarrier() = INDUSTRY_CARRIER;
    item3.isSurface() = true;
    items.push_back(item1);
    items.push_back(item2);
    items.push_back(item3);
    CPPUNIT_ASSERT(tpd.processThruMktCxrs(items, tpdPsr, governingCarrier));
    items.clear();
    tpdPsr.thruMktCxrs().clear();
    tpdPsr.carrier() = "";
  }

  void testProcessViaCxrLocs()
  {
    MileageRouteItems items;
    Loc* bos(TestLocFactory::create(globalTestDataPath + "LocBOS.xml"));
    CPPUNIT_ASSERT(bos != 0);
    Loc* nyc(TestLocFactory::create(globalTestDataPath + "LocJFK.xml"));
    CPPUNIT_ASSERT(nyc != 0);
    Loc* chi(TestLocFactory::create(globalTestDataPath + "LocORD.xml"));
    CPPUNIT_ASSERT(chi != 0);
    Loc* lax(TestLocFactory::create(globalTestDataPath + "LocLAX.xml"));
    CPPUNIT_ASSERT(lax != 0);
    MileageRouteItem bosnyc, nycchi, chilax;
    bosnyc.city1() = bos;
    bosnyc.city2() = nyc;
    bosnyc.segmentCarrier() = "UA";
    nycchi.city1() = nyc;
    nycchi.city2() = chi;
    nycchi.segmentCarrier() = "AA";
    chilax.city1() = chi;
    chilax.city2() = lax;
    chilax.segmentCarrier() = "QF";
    items.push_back(bosnyc);
    items.push_back(nycchi);
    items.push_back(chilax);
    TpdPsr tpdPsr;
    TicketedPointDeduction tpd;
    TpdPsrViaCxrLoc* boslaxaa =
        TestTpdPsrViaCxrLocFactory::create(routingTestDataPath + "TpdPsrViaCxrLoc_BOSLAXAA.xml");
    CPPUNIT_ASSERT(boslaxaa != 0);
    TpdPsrViaCxrLoc* denlaxaa =
        TestTpdPsrViaCxrLocFactory::create(routingTestDataPath + "TpdPsrViaCxrLoc_DENLAXAA.xml");
    CPPUNIT_ASSERT(denlaxaa != 0);
    TpdPsrViaCxrLoc* chiseaaa =
        TestTpdPsrViaCxrLocFactory::create(routingTestDataPath + "TpdPsrViaCxrLoc_CHISEAAA.xml");
    CPPUNIT_ASSERT(chiseaaa != 0);
    TpdPsrViaCxrLoc* densfoaa =
        TestTpdPsrViaCxrLocFactory::create(routingTestDataPath + "TpdPsrViaCxrLoc_DENSFOAA.xml");
    CPPUNIT_ASSERT(densfoaa != 0);
    TpdPsrViaCxrLoc* bosnycua =
        TestTpdPsrViaCxrLocFactory::create(routingTestDataPath + "TpdPsrViaCxrLoc_BOSNYCUA.xml");
    CPPUNIT_ASSERT(bosnycua != 0);
    TpdPsrViaCxrLoc* chilaxqf =
        TestTpdPsrViaCxrLocFactory::create(routingTestDataPath + "TpdPsrViaCxrLoc_CHILAXQF.xml");
    CPPUNIT_ASSERT(chilaxqf != 0);
    CPPUNIT_ASSERT(tpd.processViaCxrLocs(items, tpdPsr, "QF"));
    tpdPsr.viaCxrLocs().push_back(densfoaa);
    CPPUNIT_ASSERT(tpd.processViaCxrLocs(items, tpdPsr, "AA"));
    tpdPsr.viaCxrLocs().clear();
    tpdPsr.viaCxrLocs().push_back(denlaxaa);
    CPPUNIT_ASSERT(tpd.processViaCxrLocs(items, tpdPsr, "AA"));
    tpdPsr.viaCxrLocs().clear();
    tpdPsr.viaCxrLocs().push_back(chiseaaa);
    CPPUNIT_ASSERT(tpd.processViaCxrLocs(items, tpdPsr, "AA"));
    tpdPsr.viaCxrLocs().clear();
    tpdPsr.viaCxrLocs().push_back(boslaxaa);
    CPPUNIT_ASSERT(!tpd.processViaCxrLocs(items, tpdPsr, "AA"));
    tpdPsr.viaCxrLocs().clear();
    tpdPsr.viaCxrLocs().push_back(bosnycua);
    tpdPsr.viaCxrLocs().push_back(chilaxqf);
    CPPUNIT_ASSERT(tpd.processViaCxrLocs(items, tpdPsr, "UA"));
    // tpdPsr.viaCxrLocs().push_back(boslaxaa);
    // CPPUNIT_ASSERT(!tpd.processViaCxrLocs(items, tpdPsr, "AA"));
  }

  void testProcessViaExcepts()
  {
    MileageRouteItems items;
    Loc* bos(TestLocFactory::create(globalTestDataPath + "LocBOS.xml"));
    CPPUNIT_ASSERT(bos != 0);
    Loc* nyc(TestLocFactory::create(globalTestDataPath + "LocJFK.xml"));
    CPPUNIT_ASSERT(nyc != 0);
    Loc* chi(TestLocFactory::create(globalTestDataPath + "LocORD.xml"));
    CPPUNIT_ASSERT(chi != 0);
    Loc* lax(TestLocFactory::create(globalTestDataPath + "LocLAX.xml"));
    CPPUNIT_ASSERT(lax != 0);
    MileageRouteItem bosnyc, nycchi, chilax;
    bosnyc.city1() = bos;
    bosnyc.city2() = nyc;
    bosnyc.segmentCarrier() = "UA";
    nycchi.city1() = nyc;
    nycchi.city2() = chi;
    nycchi.segmentCarrier() = "AA";
    chilax.city1() = chi;
    chilax.city2() = lax;
    chilax.segmentCarrier() = "QF";
    items.push_back(bosnyc);
    items.push_back(nycchi);
    items.push_back(chilax);
    TpdPsr tpdPsr;
    TicketedPointDeduction tpd;
    TpdPsrViaExcept* boslaxaa =
        TestTpdPsrViaExceptFactory::create(routingTestDataPath + "TpdPsrViaExcept_BOSLAXAA.xml");
    CPPUNIT_ASSERT(boslaxaa != 0);
    TpdPsrViaExcept* denlaxaa =
        TestTpdPsrViaExceptFactory::create(routingTestDataPath + "TpdPsrViaExcept_DENLAXAA.xml");
    CPPUNIT_ASSERT(denlaxaa != 0);
    TpdPsrViaExcept* chiseaaa =
        TestTpdPsrViaExceptFactory::create(routingTestDataPath + "TpdPsrViaExcept_CHISEAAA.xml");
    CPPUNIT_ASSERT(chiseaaa != 0);
    TpdPsrViaExcept* densfoaa =
        TestTpdPsrViaExceptFactory::create(routingTestDataPath + "TpdPsrViaExcept_DENSFOAA.xml");
    CPPUNIT_ASSERT(densfoaa != 0);
    TpdPsrViaExcept* bosnycqf =
        TestTpdPsrViaExceptFactory::create(routingTestDataPath + "TpdPsrViaExcept_BOSNYCQF.xml");
    CPPUNIT_ASSERT(bosnycqf != 0);
    TpdPsrViaExcept* chilaxua =
        TestTpdPsrViaExceptFactory::create(routingTestDataPath + "TpdPsrViaExcept_CHILAXUA.xml");
    CPPUNIT_ASSERT(chilaxua != 0);
    CPPUNIT_ASSERT(tpd.processViaExcepts(items, tpdPsr, "UA"));
    tpdPsr.viaExcepts().push_back(densfoaa);
    CPPUNIT_ASSERT(tpd.processViaExcepts(items, tpdPsr, "AA"));
    tpdPsr.viaExcepts().clear();
    tpdPsr.viaExcepts().push_back(denlaxaa);
    CPPUNIT_ASSERT(tpd.processViaExcepts(items, tpdPsr, "AA"));
    tpdPsr.viaExcepts().clear();
    tpdPsr.viaExcepts().push_back(chiseaaa);
    CPPUNIT_ASSERT(tpd.processViaExcepts(items, tpdPsr, "AA"));
    tpdPsr.viaExcepts().clear();
    tpdPsr.viaExcepts().push_back(boslaxaa);
    CPPUNIT_ASSERT(!tpd.processViaExcepts(items, tpdPsr, "AA"));
    tpdPsr.viaExcepts().clear();
    tpdPsr.viaExcepts().push_back(bosnycqf);
    tpdPsr.viaExcepts().push_back(chilaxua);
    CPPUNIT_ASSERT(tpd.processViaExcepts(items, tpdPsr, "QF"));
    tpdPsr.viaExcepts().push_back(boslaxaa);
    CPPUNIT_ASSERT(!tpd.processViaExcepts(items, tpdPsr, "AA"));
  }

  void testProcessSubRoute()
  {
    MileageRouteItems items;
    Loc* bos(TestLocFactory::create(globalTestDataPath + "LocBOS.xml", true));
    CPPUNIT_ASSERT(bos != 0);
    Loc* nyc(TestLocFactory::create(globalTestDataPath + "LocJFK.xml", true));
    CPPUNIT_ASSERT(nyc != 0);
    Loc* chi(TestLocFactory::create(globalTestDataPath + "LocORD.xml", true));
    CPPUNIT_ASSERT(chi != 0);
    Loc* lax(TestLocFactory::create(globalTestDataPath + "LocLAX.xml", true));
    CPPUNIT_ASSERT(lax != 0);
    MileageRouteItem bosnyc, nycchi, chilax;
    bosnyc.city1() = bos;
    bosnyc.city2() = nyc;
    bosnyc.segmentCarrier() = "UA";
    nycchi.city1() = nyc;
    nycchi.city2() = chi;
    nycchi.segmentCarrier() = "AA";
    chilax.city1() = chi;
    chilax.city2() = lax;
    chilax.segmentCarrier() = "QF";
    items.push_back(bosnyc);
    items.push_back(nycchi);
    items.push_back(chilax);
    std::vector<TpdPsr*> tpdList;
    TicketedPointDeduction tpd;
    CarrierCode governingCarrier("AA");
    DataHandle dataHandle;
    FareCalcVisitor visitor;
    CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    TpdPsr tpdPsr;
    tpdPsr.loc1().locType() = tpdPsr.loc2().locType() = 'C';
    tpdPsr.loc1().loc() = "DEN";
    tpdPsr.loc2().loc() = "SFO";
    tpdList.push_back(&tpdPsr);
    CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    tpdPsr.loc1().loc() = "BOS";
    CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    tpdPsr.loc2().loc() = "LAX";
    tpdPsr.tpmDeduction() = 100;
    CPPUNIT_ASSERT(tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), items.back().tpd());
    TpdPsrViaGeoLoc* l11DENNN = TestTpdPsrViaGeoLocFactory::create(
        routingTestDataPath + "TpdPsrViaGeoLoc_11DENNN.xml", true);
    CPPUNIT_ASSERT(l11DENNN != 0);
    tpdPsr.viaGeoLocs().push_back(l11DENNN);
    items.back().tpd() = 0;
    CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    tpdPsr.viaGeoLocs().clear();
    tpdPsr.thruViaMktSameCxr() = THRUVIAMKTSAMECXR_YES;
    CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    tpdPsr.thruViaMktSameCxr() = THRUVIAMKTSAMECXR_NO;
    TpdPsrViaCxrLoc* failViaCxr = _memHandle.create<TpdPsrViaCxrLoc>();
    TpdPsrViaCxrLoc* viaCxrLoc = TestTpdPsrViaCxrLocFactory::create(
        routingTestDataPath + "TpdPsrViaCxrLoc_BOSLAXAA_ps.xml", true);
    CPPUNIT_ASSERT(viaCxrLoc != 0);
    *failViaCxr = *viaCxrLoc;
    tpdPsr.viaCxrLocs().push_back(failViaCxr);
    // CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    tpdPsr.viaCxrLocs().clear();
    TpdPsrViaExcept* failViaExc = _memHandle.create<TpdPsrViaExcept>();
    TpdPsrViaExcept* viaExcept = TestTpdPsrViaExceptFactory::create(
        routingTestDataPath + "TpdPsrViaExcept_BOSLAXAA_ps.xml", true);
    CPPUNIT_ASSERT(viaExcept != 0);
    *failViaExc = *viaExcept;
    tpdPsr.viaExcepts().push_back(failViaExc);
    CPPUNIT_ASSERT(!tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), items.back().tpd());
    tpdPsr.viaExcepts().clear();
    TpdPsr greaterTpd;
    greaterTpd.loc1().loc() = "BOS";
    greaterTpd.loc2().loc() = "LAX";
    greaterTpd.tpmDeduction() = 1000;
    greaterTpd.thruViaMktSameCxr() = THRUVIAMKTSAMECXR_NO;
    tpdList.push_back(&greaterTpd);
    CPPUNIT_ASSERT(tpd.processSubRoute(items, tpdList, governingCarrier, dataHandle, &visitor));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1000), items.back().tpd());
  }

  void testApply()
  {
    MileageRoute route;
    route.tpd() = 0;
    TicketedPointDeduction tpd;
    CPPUNIT_ASSERT(!tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), route.tpd());
    Loc* bos(TestLocFactory::create(globalTestDataPath + "LocBOS.xml"));
    CPPUNIT_ASSERT(bos != 0);
    Loc* nyc(TestLocFactory::create(globalTestDataPath + "LocJFK.xml"));
    CPPUNIT_ASSERT(nyc != 0);
    Loc* chi(TestLocFactory::create(globalTestDataPath + "LocORD.xml"));
    CPPUNIT_ASSERT(chi != 0);
    Loc* lax(TestLocFactory::create(globalTestDataPath + "LocLAX.xml"));
    CPPUNIT_ASSERT(lax != 0);
    MileageRouteItem bosnyc, nycchi, chilax;
    bosnyc.city1() = bos;
    bosnyc.city2() = nyc;
    bosnyc.segmentCarrier() = "UA";
    nycchi.city1() = nyc;
    nycchi.city2() = chi;
    nycchi.segmentCarrier() = "AA";
    chilax.city1() = chi;
    chilax.city2() = lax;
    chilax.segmentCarrier() = "QF";
    route.mileageRouteItems().push_back(bosnyc);
    route.mileageRouteItems().push_back(nycchi);
    route.mileageRouteItems().push_back(chilax);
    route.governingCarrier() = "EM";
    route.globalDirection() = GlobalDirection::ZZ;
    CPPUNIT_ASSERT(!tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), route.tpd());
    route.globalDirection() = GlobalDirection::AT;
    CPPUNIT_ASSERT(!tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), route.tpd());
    route.governingCarrier() = "NO";
    route.globalDirection() = GlobalDirection::ZZ;
    CPPUNIT_ASSERT(!tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), route.tpd());
    route.governingCarrier() = "MA";
    CPPUNIT_ASSERT(tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), route.tpd());
    route.governingCarrier() = "2M";
    CPPUNIT_ASSERT(tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1000), route.tpd());
    route.globalDirection() = GlobalDirection::AT;
    CPPUNIT_ASSERT(tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), route.tpd());
    route.governingCarrier() = "BG";
    route.tpd() = 0;
    CPPUNIT_ASSERT(!tpd.apply(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), route.tpd());
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TicketedPointDeductionTest);
}
