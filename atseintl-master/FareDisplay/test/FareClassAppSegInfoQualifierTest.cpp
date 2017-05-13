#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/FareClassAppSegInfoQualifier.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FareClassAppSegInfoQualifierTest : public CppUnit::TestFixture
{

  class FareClassAppSegInfoQualifierFD : public FareClassAppSegInfoQualifier
  {
  public:
    FareClassAppSegInfoQualifierFD() {}
    virtual ~FareClassAppSegInfoQualifierFD() {}

    const std::vector<CarrierApplicationInfo*>&
    getCarrierApplList(DataHandle& dh, const VendorCode& vendor, int itemNo) const
    {
      return _cai;
    }

    std::vector<CarrierApplicationInfo*> _cai;
  };

  CPPUNIT_TEST_SUITE(FareClassAppSegInfoQualifierTest);
  CPPUNIT_TEST(testQualifyFareClassAppSegInfo);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void getLoc(FareDisplayTrx& trx, Loc& loc, LocCode& code)
  {
    DateTime date = DateTime::localTime();
    const Loc* lc = trx.dataHandle().getLoc(code, date);
    loc = *(const_cast<Loc*>(lc));
  }

  void testQualifyFareClassAppSegInfo()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareClassAppSegInfo fcasi;
    FareClassAppSegInfoQualifierFD fq;
    AirSeg seg1;

    DateTime dt1(2005, 9, 12, 12, 0, 0);

    fdTrx.setOptions(&options);

    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    ptFare.fareClassAppSegInfo() = &fcasi;
    ptFare.fareMarket() = &fm;

    seg1.departureDT() = dt1;
    fdTrx.travelSeg().push_back(&seg1);

    Loc origin;
    Loc destination;
    LocCode orig = "DFW";
    LocCode dest = "LON";

    getLoc(fdTrx, origin, orig);
    getLoc(fdTrx, destination, dest);

    Itin itin;

    fm.origin() = &origin;
    fm.destination() = &destination;
    fm.governingCarrier() = "BA";

    fm.boardMultiCity() = orig;
    fm.offMultiCity() = dest;

    itin.fareMarket().push_back(&fm);
    fdTrx.itin().push_back(&itin);
    fdTrx.fareMarket().push_back(&fm);

    {
      fcasi._carrierApplTblItemNo = 0;
      fareInfo.carrier() = "BA";
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, ret);
    }
    {
      fcasi._carrierApplTblItemNo = 1;
      fareInfo.carrier() = "BA";
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, ret);
    }
    {
      fcasi._carrierApplTblItemNo = 1;
      fareInfo.carrier() = "AA";
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, ret);
    }
    {
      fcasi._carrierApplTblItemNo = 1;
      fareInfo.carrier() = "AA";
      CarrierApplicationInfo c1;
      c1.carrier() = "AA";
      CarrierApplicationInfo c2;
      c2.carrier() = "BA";
      fq._cai.push_back(&c1);
      fq._cai.push_back(&c2);
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, ret);
    }
    {
      fq._cai.clear();
      fcasi._carrierApplTblItemNo = 1;
      fareInfo.carrier() = "AA";
      CarrierApplicationInfo c1;
      c1.carrier() = "AA";
      CarrierApplicationInfo c2;
      c2.carrier() = "QA";
      fq._cai.push_back(&c1);
      fq._cai.push_back(&c2);
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_FareClassApp_Unavailable, ret);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareClassAppSegInfoQualifierTest);
}
