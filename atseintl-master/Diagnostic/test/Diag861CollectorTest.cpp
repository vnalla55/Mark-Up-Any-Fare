#include "Diagnostic/Diag861Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include <time.h>
#include <iostream>
#include "DataModel/Itin.h"
#include "Diagnostic/Diagnostic.h"
#include "Common/TseEnums.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/NvbNvaInfo.h"
#include <unistd.h>
#include "FareCalc/CalcTotals.h"
#include "Common/TseConsts.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{
class Diag861CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag861CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testlogNvb_NVB_1ST_SECTOR);
  CPPUNIT_TEST(testlogNvb_NVB_1ST_INTL_SECTOR);
  CPPUNIT_TEST(testlogNvb_NVB_ENTIRE_OUTBOUND);
  CPPUNIT_TEST(testlogNvb_NVB_ENTIRE_JOURNEY);
  CPPUNIT_TEST(testlogNvb_NVB_EMPTY);
  CPPUNIT_TEST(testlogNvb_UNKNOWN);
  CPPUNIT_TEST(testLogTravelSeg);
  CPPUNIT_TEST(testLogTravelSegMany);
  CPPUNIT_TEST(testLogTravelSegNotFound);
  CPPUNIT_TEST(testNegFareRestExtSeq);
  CPPUNIT_TEST(testlogNva_NVA_EMPTY);
  CPPUNIT_TEST(testlogNva_NVA_1ST_SECTOR_EARLIEST);
  CPPUNIT_TEST(testlogNva_NVA_1ST_INTL_SECTOR_EARLIEST);
  CPPUNIT_TEST(testlogNva_NVA_ENTIRE_OUTBOUND_EARLIEST);
  CPPUNIT_TEST(testNvbNvaInfo);
  CPPUNIT_TEST(testNvbNvaInfo_RuleAny);
  CPPUNIT_TEST(teseNvbNvaSeg);
  CPPUNIT_TEST(teseNvbNvaSegNotApplicable);
  CPPUNIT_TEST(teseNvbNvaSegNotApplicable2);
  CPPUNIT_TEST(testLogPricingUnit);
  CPPUNIT_TEST(testOperator);
  CPPUNIT_TEST(testOperatorDiagOff);
  CPPUNIT_TEST(testPrintSuppressionHeader);
  CPPUNIT_TEST(testClearLogIfDataEmpty);
  CPPUNIT_TEST(testPrintNvbNvaTableHeader);
  CPPUNIT_TEST(testPrintNvbNvaTableFooter);
  CPPUNIT_TEST(testPrintSuppressionFooterNotApplicable);
  CPPUNIT_TEST_SUITE_END();

private:
  Diag861Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    try
    {
      _memHandle.create<TestConfigInitializer>();
      _diag = _memHandle.create<Diag861Collector>();
      PricingTrx* trx = _memHandle.create<PricingTrx>();
      _diag->trx() = trx;
      _diag->enable(Diagnostic861);
      _diag->activate();
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    Diag861Collector diag;
    CPPUNIT_ASSERT_EQUAL(string(""), diag.str());
  }

  void testlogNvb_NVB_1ST_SECTOR()
  {
    std::string expect = std::string("PROCESS NVB: ") + NVB_1ST_SECTOR + " - 1ST SECTOR";

    _diag->logNvb(NVB_1ST_SECTOR);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNvb_NVB_1ST_INTL_SECTOR()
  {
    std::string expect = std::string("PROCESS NVB: ") + NVB_1ST_INTL_SECTOR + " - 1ST INTL SECTOR";

    _diag->logNvb(NVB_1ST_INTL_SECTOR);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNvb_NVB_ENTIRE_OUTBOUND()
  {
    std::string expect = std::string("PROCESS NVB: ") + NVB_ENTIRE_OUTBOUND + " - ENTIRE OUTBOUND";

    _diag->logNvb(NVB_ENTIRE_OUTBOUND);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNvb_NVB_ENTIRE_JOURNEY()
  {
    std::string expect = std::string("PROCESS NVB: ") + NVB_ENTIRE_JOURNEY + " - ENTIRE JOURNEY";

    _diag->logNvb(NVB_ENTIRE_JOURNEY);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNvb_NVB_EMPTY()
  {
    std::string expect = std::string("PROCESS NVB: ") + NVB_EMPTY + " - EMPTY";

    _diag->logNvb(NVB_EMPTY);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNvb_UNKNOWN()
  {
    std::string expect = "PROCESS NVB: Z - UNKNOWN";

    _diag->logNvb('Z');
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  AirSeg*
  makeAirSeg(const LocCode& orig, const LocCode& dest, int16_t segmentOrder, int16_t pnrSegment)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->segmentOrder() = segmentOrder;
    airSeg->pnrSegment() = pnrSegment;
    airSeg->origAirport() = orig;
    airSeg->boardMultiCity() = orig;
    airSeg->destAirport() = dest;
    airSeg->offMultiCity() = dest;

    return airSeg;
  }

  void testLogTravelSeg()
  {
    std::string expect = "PNR SEGMENT: 1    FROM: LON    TO: KRK\n";

    FarePath farePath;
    Itin* itin = _memHandle.create<Itin>();
    farePath.itin() = itin;
    farePath.itin()->travelSeg().push_back(makeAirSeg("LON", "KRK", 1, 12));

    _diag->logTravelSeg(1, farePath);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testLogTravelSegMany()
  {
    std::string expect = "PNR SEGMENT: 2    FROM: MUN    TO: KRK\n";

    FarePath farePath;
    Itin* itin = _memHandle.create<Itin>();
    farePath.itin() = itin;
    farePath.itin()->travelSeg().push_back(makeAirSeg("LON", "KRK", 1, 15));
    farePath.itin()->travelSeg().push_back(makeAirSeg("MUN", "KRK", 2, 13));
    farePath.itin()->travelSeg().push_back(makeAirSeg("NYC", "ORD", 3, 07));

    _diag->logTravelSeg(2, farePath);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testLogTravelSegNotFound()
  {
    std::string expect;
    FarePath farePath;
    Itin* itin = _memHandle.create<Itin>();
    farePath.itin() = itin;
    farePath.itin()->travelSeg().push_back(makeAirSeg("LON", "KRK", 8, 15));
    farePath.itin()->travelSeg().push_back(makeAirSeg("MUN", "KRK", 7, 13));
    farePath.itin()->travelSeg().push_back(makeAirSeg("NYC", "ORD", 9, 07));

    _diag->logTravelSeg(11, farePath);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testNegFareRestExtSeq()
  {
    std::string expect = "  TICKETED FARE DATA PER SEGMENT/COMPONENT :\n"
                         "  SEQNO  FRM TO  CXR VIA POINTS      FARE BASIS  UNFBC    SPNV\n"
                         "  0                                                       N\n";

    NegFareRestExtSeq* nfr = _memHandle.create<NegFareRestExtSeq>();
    (*_diag) << (*nfr);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNva_NVA_EMPTY()
  {
    std::string expect = std::string("    NVA: ") + NVA_EMPTY + " - EMPTY\n";

    _diag->logNva(NVA_EMPTY);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNva_NVA_1ST_SECTOR_EARLIEST()
  {
    std::string expect = std::string("    NVA: ") + NVA_1ST_SECTOR_EARLIEST + " - 1ST SECTOR\n";

    _diag->logNva(NVA_1ST_SECTOR_EARLIEST);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNva_NVA_1ST_INTL_SECTOR_EARLIEST()
  {
    std::string expect =
        std::string("    NVA: ") + NVA_1ST_INTL_SECTOR_EARLIEST + " - 1ST INTL SECTOR\n";

    _diag->logNva(NVA_1ST_INTL_SECTOR_EARLIEST);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testlogNva_NVA_ENTIRE_OUTBOUND_EARLIEST()
  {
    std::string expect =
        std::string("    NVA: ") + NVA_ENTIRE_OUTBOUND_EARLIEST + " - ENTIRE OUTBOUND\n";

    _diag->logNva(NVA_ENTIRE_OUTBOUND_EARLIEST);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testNvbNvaInfo()
  {
    std::string strCreateDate("2004-11-07 23:13:44.000");
    DateTime createDate(strCreateDate);

    std::string strExpireDate("2008-11-07 23:59:59.9999");
    DateTime expireDate(strExpireDate);

    NvbNvaInfo info;
    info.vendor() = "5KAD";
    info.carrier() = CarrierCode("AA");
    info.ruleTariff() = 1256;
    info.rule() = "123";
    info.createDate() = createDate;
    info.expireDate() = expireDate;

    (*_diag) << info;
    std::string expect = std::string("----------------NVB NVA INFO------------- \n") +
                         "  VENDOR   CARRIER   RULE TARIFF   RULE   \n" +
                         "   5KAD       AA        1256        123\n" +
                         "CREATE DATE: 2004-NOV-07 23:13:44\n" +
                         "EXPIRE DATE: 2008-NOV-07 23:59:59\n";

    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void teseNvbNvaSeg()
  {
    NvbNvaSeg seg;
    seg.sequenceNumber() = 25;
    seg.fareBasis() = "AA123";
    seg.nvb() = NVB_1ST_INTL_SECTOR;
    seg.nva() = NVA_ENTIRE_OUTBOUND_EARLIEST;

    std::string expect = std::string("SEQNO: 25    FARE BASIS: AA123\n") +
                         "    NVB: 1ST INTL SECTOR    NVA: ENTIRE OUTBOUND\n" + "MATCHED\n \n";

    _diag->logNvbNvaSeg(seg);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void teseNvbNvaSegNotApplicable()
  {
    NvbNvaSeg seg;
    seg.sequenceNumber() = 25;
    seg.fareBasis() = "AA123";
    seg.nvb() = NVB_1ST_INTL_SECTOR;
    seg.nva() = NVA_ENTIRE_OUTBOUND_EARLIEST;

    std::string expect = "SEQNO: 25    FARE BASIS: AA123           - NOT MATCHED\n";
    _diag->logNvbNvaSeg(seg, false);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }
  void teseNvbNvaSegNotApplicable2()
  {
    NvbNvaSeg seg;
    seg.sequenceNumber() = 25;
    seg.fareBasis() = "AA12345678";
    seg.nvb() = NVB_1ST_INTL_SECTOR;
    seg.nva() = NVA_ENTIRE_OUTBOUND_EARLIEST;

    std::string expect = "SEQNO: 25    FARE BASIS: AA12345678      - NOT MATCHED\n";
    _diag->logNvbNvaSeg(seg, false);
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testNvbNvaInfo_RuleAny()
  {
    std::string strCreateDate("2004-11-07 23:13:44.000");
    DateTime createDate(strCreateDate);

    std::string strExpireDate("2008-11-07 23:59:59.9999");
    DateTime expireDate(strExpireDate);

    NvbNvaInfo info;
    info.vendor() = "5KAD";
    info.carrier() = CarrierCode("AA");
    info.ruleTariff() = 1256;
    info.rule() = ANY_RULE;
    info.createDate() = createDate;
    info.expireDate() = expireDate;

    (*_diag) << info;
    std::string expect = std::string("----------------NVB NVA INFO------------- \n") +
                         "  VENDOR   CARRIER   RULE TARIFF   RULE   \n" +
                         "   5KAD       AA        1256        ANY\n" +
                         "CREATE DATE: 2004-NOV-07 23:13:44\n" +
                         "EXPIRE DATE: 2008-NOV-07 23:59:59\n";

    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testLogPricingUnit()
  {
    NvbNvaInfo info;
    PricingUnit pu;
    CalcTotals calcTotals;

    calcTotals.tvlSegNVB[1] = DateTime(2002, Jan, 10);
    calcTotals.tvlSegNVB[2] = DateTime(2003, Jan, 10);
    calcTotals.tvlSegNVB[3] = DateTime(2004, Jan, 10);

    calcTotals.tvlSegNVA[1] = DateTime(2012, Jan, 10);
    calcTotals.tvlSegNVA[2] = DateTime(2013, Jan, 10);
    calcTotals.tvlSegNVA[3] = DateTime(2014, Jan, 10);

    FareUsage* fr = _memHandle.create<FareUsage>();
    fr->travelSeg().push_back(makeAirSeg("KRK", "LON", 1, 0));
    fr->travelSeg().push_back(makeAirSeg("KRK", "NYC", 2, 0));

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(makeAirSeg("KRK", "LON", 1, 0));
    itin->travelSeg().push_back(makeAirSeg("KRK", "NYC", 2, 0));

    pu.fareUsage().push_back(fr);
    _diag->logPricingUnit(pu.fareUsage(), itin, calcTotals);
    std::string expect;

    expect = std::string("**************************************************************\n") +
             "            PRICING UNIT STATE BEFORE PROCESSING      \n" +
             "  FROM     TO      TVL DATE      NVB DATE      NVA DATE   \n" +
             "   KRK    LON             N/A   2002-JAN-10   2012-JAN-10\n" +
             "   KRK    NYC             N/A   2003-JAN-10   2013-JAN-10\n";

    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testOperator()
  {
    NvbNvaInfo info;
    std::string expect = "TEST STRING";
    (*_diag) << "TEST STRING";

    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testOperatorDiagOff()
  {
    NvbNvaInfo info;
    _diag->deActivate();
    std::string expect;
    (*_diag) << "TEST STRING";

    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testPrintSuppressionHeader()
  {
    NvbNvaInfo info;
    _diag->activate();

    std::string expect;
    expect = std::string(" \n") +
             "**************************************************************\n" +
             "*****************NVB NVA SUPPRESSION SECTION******************\n" +
             "**************************************************************\n";

    _diag->printSuppressionHeader();
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testClearLogIfDataEmpty()
  {
    NvbNvaInfo info;
    std::string expect;
    expect = "*****************NVB NVA SUPPRESSION SECTION******************\n";
    expect += "**************************************************************\n";
    expect += "*********  RESULTS FOR SMF NVB NVA UNIQUE TABLE   ************\n";
    expect += "*********       NON QUALIFYING ITINERARY           ***********\n";
    expect += "            SMF TABLE NOT APPLICABLE \n";
    expect += "**************************************************************\n";

    _diag->clearLogIfDataEmpty();
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testPrintSuppressionFooterNotApplicable()
  {
    NvbNvaInfo info;
    std::string expect;
    _diag->_isEmptyNvbNvaTable = false;
    expect = std::string("                        NOT APPLICABLE\n") +
             "**************END OF NVB NVA SUPPRESSION SECTION**************\n";

    _diag->printSuppressionFooter();
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
    _diag->_isEmptyNvbNvaTable = true;
  }

  void testPrintNvbNvaTableHeader()
  {
    NvbNvaInfo info;
    std::string expect;
    expect = std::string(" \n") +
             "**************************************************************\n" +
             "***************NVB NVA TABLE PROCESSING SECTION***************\n";

    _diag->printNvbNvaTableHeader();
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }

  void testPrintNvbNvaTableFooter()
  {
    NvbNvaInfo info;
    std::string expect;
    expect = "************END OF NVB NVA TABLE PROCESSING SECTION***********\n";

    _diag->printNvbNvaTableFooter();
    CPPUNIT_ASSERT_EQUAL(expect, _diag->str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag861CollectorTest);
} // tse
