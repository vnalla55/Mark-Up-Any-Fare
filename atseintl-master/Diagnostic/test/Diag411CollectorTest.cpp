#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/Diag411Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFareMarketFactory.h"

namespace tse
{

class Diag411CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag411CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorFareMarket);
  CPPUNIT_TEST(testStreamingOperatorPaxTypeFare);

  CPPUNIT_TEST(testPrintMessages);
  CPPUNIT_TEST(testDisplayHierarchy);
  CPPUNIT_TEST(testLineSkip);
  CPPUNIT_TEST(testStreamingOperatorBookingCodeStatus);
  CPPUNIT_TEST(testStreamingOperatorBookingCodeSegmentStatus);

  CPPUNIT_TEST_SUITE_END();

  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag411Collector diag(diagroot);

      CPPUNIT_ASSERT_EQUAL(std::string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorFareMarket()
  //---------------------------------------------------------------------
  void testStreamingOperatorFareMarket()
  {
    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    Diag411Collector diag(diagroot);
    diag.enable(Diagnostic411);

    CPPUNIT_ASSERT(diag.isActive());

    FareMarket* fMkt = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/FareCurrencySelection/IntlPrime/fareMarket.xml");

    ClassOfService cos;
    CabinType economy;
    economy.setEconomyClass();

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin() = economy;

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin() = economy;

    ClassOfService cos2;

    cos2.bookingCode() = "A";
    cos2.numSeats() = 0;
    cos2.cabin() = economy;

    std::vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    fMkt->classOfServiceVec().clear();
    fMkt->classOfServiceVec().push_back(&_cos);

    diag << *fMkt;

    std::string expected;

    expected += "\n";
    expected += "LHR-BA-BOM   INTERNATIONAL   \n";
    expected += "\n";
    expected += "     LHR  Y   AVL BKG CODES: YF\n";
    expected += "     BOM\n";
    expected += "\n";
    expected += "  FARE BASIS   CXR V RULE TAR O O  FARE AMT CUR  PAX C FARE \n";
    expected += "       CLASS              NUM R I                TPE B TYPE \n";

    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorPaxTypeFare()
  //---------------------------------------------------------------------
  void testStreamingOperatorPaxTypeFare()
  {

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag411Collector diag(diagroot);
    diag.enable(Diagnostic411);

    CPPUNIT_ASSERT(diag.isActive());

    PricingTrx trx;
    PricingRequest req;
    PricingOptions opt;
    PaxTypeFare mPaxTypeFare;
    FareMarket mFareMarket;
    Fare mFare;
    FareInfo fi;
    fi._ruleNumber = '1';
    fi._fareAmount = 200;
    fi._fareTariff = 5;
    fi._currency = "USD";
    fi._fareClass = "AAAA";
    fi._carrier = "AA";
    fi._globalDirection = GlobalDirection::AT;
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    mFare.initialize(Fare::FS_Domestic, &fi, mFareMarket, &tcri);

    FareClassAppInfo fai;
    FareClassAppSegInfo fsai;

    PaxType mPaxType;

    mPaxTypeFare.initialize(&mFare, &mPaxType, &mFareMarket);

    //  pFare->set_paxTypeCode(paxT);
    mPaxTypeFare.setFare(&mFare);
    mPaxTypeFare.fareClassAppSegInfo() = &fsai;
    mPaxTypeFare.fareClassAppInfo() = &fai;

    PaxTypeCode pType = "CNN";
    fsai._paxType = "CNN";

    mPaxType.paxType() = pType;

    trx.fareMarket().push_back(&mFareMarket);

    FareClassCode fareClass = "Y";
    req.fareClassCode() = fareClass.c_str();

    Agent agent;
    req.ticketingAgent() = &agent;

    trx.setRequest(&req);
    trx.setOptions(&opt);

    std::pair<PricingTrx*, PaxTypeFare*> output(&trx, &mPaxTypeFare);

    diag << output;

    std::string expected;

    expected += "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
    expected += "P AAAA          AA A 1      5 R I    200.00 USD  CNN   0    SP \n \n";

    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  //---------------------------------------------------------------------
  // testPrintMessages()
  //---------------------------------------------------------------------
  void testPrintMessages()
  {
    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    diag.printMessages(1);

    diag.printMessages(2);

    diag.printMessages(3);

    diag.printMessages(4);

    diag.printMessages(5);

    diag.printMessages(6);

    diag.printMessages(7);

    diag.printMessages(8);

    diag.printMessages(9);

    diag.printMessages(10);

    diag.printMessages(11);

    diag.printMessages(12);

    diag.printMessages(13);

    diag.printMessages(14, '1');

    diag.printMessages(15);

    diag.printMessages(16);

    diag.printMessages(17);

    diag.printMessages(18);

    diag.printMessages(19);

    diag.printMessages(20);

    diag.printMessages(21);

    diag.printMessages(22);

    diag.printMessages(23);

    diag.printMessages(24);

    diag.printMessages(25);

    diag.printMessages(26);

    diag.printMessages(27);

    diag.printMessages(28);

    diag.printMessages(29);

    diag.printMessages(30);
  }

  //---------------------------------------------------------------------
  // testDisplayHierarchy()
  //---------------------------------------------------------------------
  void testDisplayHierarchy()
  {

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    diag.displayHierarchy(1);

    diag.displayHierarchy(2);

    diag.displayHierarchy(3);

    diag.displayHierarchy(4);

    diag.displayHierarchy(5);

    diag.displayHierarchy(6);

    diag.displayHierarchy(7);

    diag.displayHierarchy(8);
  }

  //---------------------------------------------------------------------
  // testLineSkip()
  //---------------------------------------------------------------------
  void testLineSkip()
  {
    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    diag.lineSkip(0);

    diag.lineSkip(1);

    diag.lineSkip(2);

    diag.lineSkip(3);
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorBookingCodeStatus()
  //---------------------------------------------------------------------
  void testStreamingOperatorBookingCodeStatus()
  {

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag411Collector diag(diagroot);
    diag.enable(Diagnostic411);

    CPPUNIT_ASSERT(diag.isActive());

    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    Fare mFare;
    PaxType mPaxType;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';
    //   paxTfare._ruleStatus = PaxTypeFare::RS_Cat16;
    paxTfare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);

    diag << paxTfare.bookingCodeStatus();

    paxTfare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);

    diag << paxTfare.bookingCodeStatus();

    paxTfare.bookingCodeStatus().set(PaxTypeFare::BKS_MIXED);

    diag << paxTfare.bookingCodeStatus();

    paxTfare.bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED);

    diag << paxTfare.bookingCodeStatus();
    /*
    std::cout << "X" << std::endl;
    std::cout << diag.str() << std::endl;
    std::cout << "X" << std::endl;

    string expected;
    string actual = diag.str ();

    expected += "        TRAVEL SEGMENT: DFW-AA-ORD\n";
    expected += "                        FLIGHT - 777\n";
    expected += "                        BOOKED COS - Y\n";
    expected += "\n";

    std::cout << "length of Expected: " <<   expected.size();
    std::cout << "length of Actual  : " <<   actual.size();

    CPPUNIT_ASSERT_EQUAL( expected, diag.str() );
    */
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorBookingCodeSegmentStatus()
  //---------------------------------------------------------------------
  void testStreamingOperatorBookingCodeSegmentStatus()
  {
    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag411Collector diag(diagroot);
    diag.enable(Diagnostic411);

    CPPUNIT_ASSERT(diag.isActive());

    FareMarket fareMarket;
    PaxTypeFare paxTfare;
    Fare mFare;
    PaxType mPaxType;
    AirSeg airSeg;
    AirSeg airSeg1;

    paxTfare.initialize(&mFare, &mPaxType, &fareMarket);

    paxTfare.setFare(&mFare);

    paxTfare.fareTypeApplication() = 'N';
    //   paxTfare._ruleStatus = PaxTypeFare::RS_Cat16;
    paxTfare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    //  Gets statuses for each segment in the Fare Market

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    airSeg.carrier() = bcCarrier;
    airSeg.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg);

    Loc origin1;
    Loc dest1;

    origin1.loc() = "DEN";
    dest1.loc() = "LON";

    airSeg1.origin() = &origin1;
    airSeg1.destination() = &dest1;

    airSeg1.carrier() = bcCarrier;
    airSeg1.setBookingCode("Y");

    fareMarket.travelSeg().push_back(&airSeg1);

    TravelSegType segType = Air;
    airSeg.segmentType() = segType;
    airSeg1.segmentType() = segType;

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest1;

    fareMarket.governingCarrier() = bcCarrier;

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    FareInfo fi;

    fi._ruleNumber = "0";
    fi._fareAmount = 0.0;
    fi._fareTariff = 5;
    fi._currency = "USD";
    fi._fareClass = "Y";
    fi._carrier = "AA";
    fi._vendor = Vendor::ATPCO;
    fi._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi._directionality = TO;

    TariffCrossRefInfo tcri;
    tcri._ruleTariff = 0;

    mFare.initialize(Fare::FS_Domestic, &fi, fareMarket, &tcri);

    mFare.status() = Fare::FS_RoutingValid;

    typedef std::vector<TravelSeg*>::iterator TravelSegVecI;

    PaxTypeFare::SegmentStatus segStat;

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }
    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOMATCH, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }
    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_T999, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_REC1_T999, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_CONV1_T999, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_CONV2_T999, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_MIXEDCLASS, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_LOCALMARKET, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }

    for (TravelSegVecI iterTvl = fareMarket.travelSeg().begin();
         iterTvl != fareMarket.travelSeg().end();
         iterTvl++) // Initialize
    {
      segStat._bkgCodeSegStatus.setNull();

      if (dynamic_cast<AirSeg*>(*iterTvl) != 0)
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL, true);
      }
      else
      {
        segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);
      }

      diag << segStat;
      //      paxTfare.segmentStatus().push_back(segStat);
    }
  }

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag411CollectorTest);
}
