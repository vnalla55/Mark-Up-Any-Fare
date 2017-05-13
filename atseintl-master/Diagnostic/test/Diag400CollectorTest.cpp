#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag400Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/CppUnitHelperMacros.h"

#include <string>

namespace tse
{
class Diag400CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag400CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorFinal);
  CPPUNIT_TEST(testStreamingOperatorPaxTypeFare);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag400Collector diag(diagroot);

      std::string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(std::string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorFinal()
  //---------------------------------------------------------------------
  void testStreamingOperatorFinal()
  {

    Diagnostic diagroot(Diagnostic400);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag400Collector diag(diagroot);

    diag.enable(Diagnostic400);
    CPPUNIT_ASSERT(diag.isActive());

    PaxTypeFare mPaxTypeFare;
    PaxTypeFare PaxTypeFare;
    FareMarket mfareMarket;
    //  MockFareMarket        mmfareMarket;
    Fare mFare;
    AirSeg mockAirSeg;
    PricingTrx trx;
    PricingRequest req;
    PaxType mPaxType;

    mPaxTypeFare.initialize(&mFare, &mPaxType, &mfareMarket);

    //  pFare->set_paxTypeCode(paxT);
    mPaxTypeFare.setFare(&mFare);

    PaxTypeCode pType = "CNN";

    mPaxType.paxType() = pType;

    CabinType cabinFC;
    cabinFC.setFirstClass();
    mPaxTypeFare.cabin() = cabinFC;

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "DEN";

    mockAirSeg.boardMultiCity() = "DFW";
    mockAirSeg.origAirport() = "DFW";
    mockAirSeg.origin() = &origin;

    mockAirSeg.offMultiCity() = "DEN";
    mockAirSeg.destAirport() = "DEN";
    mockAirSeg.destination() = &dest;

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.
    mockAirSeg.carrier() = bcCarrier;
    mockAirSeg.setBookingCode("Y ");

    mfareMarket.travelSeg().push_back(&mockAirSeg);

    mfareMarket.origin() = &origin;
    mfareMarket.destination() = &dest;

    mfareMarket.governingCarrier() = bcCarrier;

    mfareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

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

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, mfareMarket, &tcri);

    fcasi._bookingCode[0] = "F ";
    fcasi._bookingCode[1] = "Y ";

    mPaxTypeFare.fareClassAppInfo() = &fcai;

    mPaxTypeFare.fareClassAppSegInfo() = &fcasi;

    std::vector<TravelSeg*>::iterator iterTvl = mfareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = mfareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
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

      mPaxTypeFare.segmentStatus().push_back(segStat);
    }
    FareClassCode fareClass = "Y";

    trx.fareMarket().push_back(&mfareMarket);

    req.fareClassCode() = fareClass.c_str();
    trx.setRequest(&req);

    ClassOfService cos;
    CabinType cabin;
    cabin.setEconomyClass();
    cos.bookingCode() = "Y ";
    cos.numSeats() = 10;
    cos.cabin() = cabin;

    ClassOfService cos1;

    cos1.bookingCode() = "F ";
    cos1.numSeats() = 1;
    cos1.cabin() = cabin;

    ClassOfService cos2;

    cos2.bookingCode() = "A ";
    cos2.numSeats() = 0;
    cos2.cabin() = cabin;

    std::vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    mfareMarket.classOfServiceVec().push_back(&_cos);

    CPPUNIT_ASSERT(diag.finalDiagTest(trx, mfareMarket));

    std::string expected;
    std::string actual = diag.str();

    expected += "\nDFW-AA-DEN   US/CA   \n\n";
    expected += "    DFW  Y    AVL BKG CODES: Y F \n";
    expected += "    DEN\n\n";
    expected += "DFW-AA-DEN\n";
    expected += "  FARE BASIS   CXR V RULE TAR O O     FARE  CUR PAX BK C STAT\n";
    expected += "       CLASS              NUM R I    AMOUNT     TYP    B\n";
    expected += "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
    expected += " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    expected += "--------------------------------------------------------\n";
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorPaxTypeFare()
  //---------------------------------------------------------------------
  void testStreamingOperatorPaxTypeFare()
  {

    Diagnostic diagroot(Diagnostic400);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag400Collector diag(diagroot);
    diag.enable(Diagnostic400);

    CPPUNIT_ASSERT(diag.isActive());

    PaxTypeFare mPaxTypeFare;
    PaxTypeFare PaxTypeFare;
    FareMarket mfareMarket;
    Fare mFare;
    AirSeg mockAirSeg;

    PaxType mPaxType;

    mPaxTypeFare.initialize(&mFare, &mPaxType, &mfareMarket);

    //   GeoTravelType        geoTravelType = Domestic;

    //  pFare->set_paxTypeCode(paxT);
    mPaxTypeFare.setFare(&mFare);

    PaxTypeCode pType = "CNN";

    mPaxType.paxType() = pType;

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "ORD";

    mockAirSeg.origin() = &origin;
    mockAirSeg.destination() = &dest;
    mockAirSeg.setBookingCode("Y ");

    mfareMarket.travelSeg().push_back(&mockAirSeg);

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.

    mfareMarket.origin() = &origin;

    mfareMarket.destination() = &dest;

    mfareMarket.governingCarrier() = bcCarrier;

    mfareMarket.travelBoundary() = FMTravelBoundary::TravelWithinOneIATA; // International

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

    FareClassAppInfo fcai;
    FareClassAppSegInfo fcasi;

    mFare.initialize(Fare::FS_Domestic, &fi, mfareMarket, &tcri);

    fcasi._bookingCode[0] = "F ";
    fcasi._bookingCode[1] = "Y ";

    mPaxTypeFare.fareClassAppInfo() = &fcai;

    mPaxTypeFare.fareClassAppSegInfo() = &fcasi;
    mPaxTypeFare.setFare(&mFare);

    //  mPaxTypeFare.set_bkgCodes();

    std::vector<TravelSeg*>::iterator iterTvl = mfareMarket.travelSeg().begin();
    std::vector<TravelSeg*>::iterator iterTvlEnd = mfareMarket.travelSeg().end();

    PaxTypeFare::SegmentStatus segStat;

    for (; iterTvl != iterTvlEnd; iterTvl++) // Initialize
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

      mPaxTypeFare.segmentStatus().push_back(segStat);
    }

    diag << mPaxTypeFare;

    std::string expected;
    std::string actual = diag.str();

    expected += "P Y        AA A 0       5 O I F        NOT YET PROCESSED \n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag400CollectorTest);
}
