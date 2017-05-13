//-----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ArunkSeg.h"
#include "Diagnostic/Diag188Collector.h"


using namespace std;

namespace tse
{

namespace
{

class MyDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& destination,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    static Mileage ret;
    ret.orig()        = origin;
    ret.dest()        = destination;
    ret.mileageType() = mileageType;
    ret.globaldir()   = globalDir;
    ret.vendor()      = "IATA";

    if ( "COR" == origin && "AEP" == destination )
    {
      ret.mileage()     = 6724;
    }
    else if ( "AEP" == origin && "USH" == destination )
    {
      ret.mileage()     = 1470;
    }
    else if ( "DFW" == origin && "LAX" == destination )
    {
      ret.mileage()     = 1238;
    }
    else if ( "LAX" == origin && "LON" == destination )
    {
      ret.mileage()     = 5454;
    }
    else
    {
      ret.mileage()     = 0;
    }

    return &ret;
  }

};
} // anon NS

class Diag188CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag188CollectorTest);
  CPPUNIT_TEST(testPrintArunk_Null);
  CPPUNIT_TEST(testPrintArunk);
  CPPUNIT_TEST(testPrintFareMarket_NoTpm);
  CPPUNIT_TEST(testPrintFareMarket_WithTpm);
  CPPUNIT_TEST(testPrintFareMarkets_None);
  CPPUNIT_TEST(testPrintFareMarkets);
  CPPUNIT_TEST(testPrintFooter);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintSideTrips_None);
  CPPUNIT_TEST(testPrintSideTrips_Single);
  CPPUNIT_TEST(testPrintSideTrips_Multi);
  CPPUNIT_TEST(testPrintTpmSeg_Null);
  CPPUNIT_TEST(testPrintTpmMileage_Skip);
  CPPUNIT_TEST(testPrintTpmMileage_NoTravelSegs);
  CPPUNIT_TEST(testPrintTpmMileage);
  CPPUNIT_TEST(testPrintTpmSeg_NoTotal);
  CPPUNIT_TEST(testPrintTpmSeg_WithTotal);
  CPPUNIT_TEST(testPrintTpmSegs_Unable);
  CPPUNIT_TEST(testPrintTpmSegs_SingleSegment);
  CPPUNIT_TEST(testPrintTpmSegs_SameCarrier);
  CPPUNIT_TEST(testPrintTpmSegs_MultiCarrier);
  CPPUNIT_TEST(testPrintTravelSeg_Null);
  CPPUNIT_TEST(testPrintTravelSeg_Arunk);
  CPPUNIT_TEST(testPrintTravelSeg);
  CPPUNIT_TEST(testPrintTravelSegments);
  CPPUNIT_TEST(testPricingTrx);
  CPPUNIT_TEST(testExchangePricingTrx);
  CPPUNIT_TEST(testRefundPricingTrx);
  CPPUNIT_TEST(testRexPricingTrx);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandleMock>();
    _memHandle.create<TestConfigInitializer>();
    _collector = _memHandle.create<Diag188Collector>();
    _collector->activate();
    _trx = _memHandle.create<PricingTrx>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testPrintArunk_Null()
  {
    std::stringstream expectedDiag;
    expectedDiag << "              ARUNK" << std::endl;

    ArunkSeg* arunkSeg = 0;
    _collector->printArunk( arunkSeg );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintArunk()
  {
    std::stringstream expectedDiag;
    expectedDiag << "DFW-MIA       ARUNK" << std::endl;

    ArunkSeg arunkSeg;
    arunkSeg.origAirport() = "DFW";
    arunkSeg.destAirport() = "MIA";
    _collector->printArunk( &arunkSeg );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFareMarket_NoTpm()
  {
    std::stringstream expectedDiag;
    expectedDiag << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: SKIP" << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "   PRIMARY SECTOR IS NULL" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinSameCountryExceptUSCA; //Contrived to skip TPM

    _collector->printFareMarket( *fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFareMarket_WithTpm()
  {
    std::stringstream expectedDiag;
    expectedDiag << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // Non-domestic for TPM
    fm->primarySector() = fm->travelSeg().front();

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    _collector->_trx = &prcTrx;

    _collector->printFareMarket( *fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFareMarketHeader()
  {
    std::stringstream expectedDiag;
    expectedDiag << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 3 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl;

    const uint16_t fmNumber = 3;
    _collector->printFareMarketHeader( fmNumber );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFareMarkets_None()
  {
    std::stringstream expectedDiag;
    expectedDiag << "--------------------------------------------------------------" << std::endl
                 << "NUMBER OF FARE MARKETS IN TRX: 0" << std::endl
                 << "  " << std::endl;

    PricingTrx prcTrx;
    _collector->printFareMarkets( prcTrx );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFareMarkets()
  {
    std::stringstream expectedDiag;
    expectedDiag << "--------------------------------------------------------------" << std::endl
                 << "NUMBER OF FARE MARKETS IN TRX: 2" << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 1 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 2 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // Non-domestic for TPM
    fm->primarySector() = fm->travelSeg().front();

    FareMarket clonedFareMarket;
    fm->clone( clonedFareMarket );

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    prcTrx.travelSeg() = fm->travelSeg();
    prcTrx.fareMarket().push_back( fm );
    prcTrx.fareMarket().push_back( &clonedFareMarket );

    _collector->_trx = &prcTrx;
    _collector->printFareMarkets( prcTrx );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintFooter()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** END   DIAG 188 ***********************" << std::endl;
    _collector->printFooter();
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintHeader()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** START DIAG 188 ***********************" << std::endl;
    _collector->printHeader();
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmMileage_Skip()
  {
    std::stringstream expectedDiag;
    expectedDiag << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: SKIP" << std::endl;

    FareMarket fm;
    fm.travelBoundary() = FMTravelBoundary::TravelWithinSameCountryExceptUSCA;
    _collector->printTpmMileage( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );

    fm.travelBoundary() = FMTravelBoundary::TravelWithinUSCA;
    _collector->str( "" );
    _collector->printTpmMileage( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmMileage_NoTravelSegs()
  {
    std::stringstream expectedDiag;
    expectedDiag << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL" << std::endl;

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    _collector->_trx = &prcTrx;

    FareMarket fm;
    fm.travelBoundary() = FMTravelBoundary::TravelWithinOneIATA;
    _collector->printTpmMileage( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmMileage()
  {
    std::stringstream expectedDiag;
    expectedDiag << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      6724  " << std::endl;

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    _collector->_trx = &prcTrx;

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    airSeg->carrier() = "JH";

    FareMarket fm;
    fm.travelBoundary() = FMTravelBoundary::TravelWithinOneIATA;
    fm.travelSeg().push_back( airSeg );

    _collector->printTpmMileage( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSeg_Null()
  {
    std::stringstream expectedDiag;
    expectedDiag << std::endl;

    const AirSeg* nullAirSeg = 0;
    _collector->printTpmSeg( nullAirSeg, 1234, 34567 );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSeg_NoTotal()
  {
    std::stringstream expectedDiag;
    expectedDiag << "DFW-AA-MIA    1234      " << std::endl;

    AirSeg airSeg;
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "MIA";
    airSeg.carrier() = "AA";
    _collector->printTpmSeg( &airSeg, 1234, 0 );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSeg_WithTotal()
  {
    std::stringstream expectedDiag;
    expectedDiag << "DFW-AA-MIA    1234      34567 " << std::endl;

    AirSeg airSeg;
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "MIA";
    airSeg.carrier() = "AA";
    _collector->printTpmSeg( &airSeg, 1234, 34567 );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSegs_Unable()
  {
    std::stringstream expectedDiag;
    expectedDiag << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "UNABLE TO DETERMINE TPM. UNSUPPORTED TRANSACTION TYPE." << std::endl;

    AirSeg airSeg;
    airSeg.pnrSegment() = 1;
    airSeg.carrier() = "JH";
    airSeg.flightNumber() = 2424;
    airSeg.setBookingCode( "Y" );
    airSeg.departureDT() = DateTime( 2010, 3, 1 );
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "MIA";
    airSeg.geoTravelType() = GeoTravelType::Domestic;

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back( &airSeg );

    _collector->printTpmSegs( travelSegs );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSegs_SingleSegment()
  {
    std::stringstream expectedDiag;
    expectedDiag << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      6724  " << std::endl;

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    _collector->_trx = &prcTrx;

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    airSeg->carrier() = "JH";

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back( airSeg );

    _collector->printTpmSegs( travelSegs );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSegs_SameCarrier()
  {
    std::stringstream expectedDiag;
    expectedDiag << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      "       << std::endl
                 << "AEP-JH-USH    1470      8194  " << std::endl;

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    _collector->_trx = &prcTrx;

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    airSeg->carrier() = "JH";
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    airSeg2->carrier() = "JH";

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back( airSeg );
    travelSegs.push_back( airSeg2 );

    _collector->printTpmSegs( travelSegs );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTpmSegs_MultiCarrier()
  {
    std::stringstream expectedDiag;
    expectedDiag << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      6724  " << std::endl
                 << "AEP-MA-USH    1470      1470  " << std::endl;

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    _collector->_trx = &prcTrx;

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    airSeg->carrier() = "JH";
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    airSeg2->carrier() = "MA";

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back( airSeg );
    travelSegs.push_back( airSeg2 );

    _collector->printTpmSegs( travelSegs );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTravelSeg_Null()
  {
    std::stringstream expectedDiag;
    expectedDiag << std::endl;

    const AirSeg* nullAirSeg = 0;
    _collector->printTravelSegment( nullAirSeg );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTravelSeg_Arunk()
  {
    std::stringstream expectedDiag;
    expectedDiag << "                 DFWMIA" << std::endl;

    ArunkSeg arunkSeg;
    arunkSeg.origAirport() = "DFW";
    arunkSeg.destAirport() = "MIA";
    _collector->printTravelSegment( &arunkSeg );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTravelSeg()
  {
    std::stringstream expectedDiag;
    expectedDiag << " 2 JH2424Y  01MAR DFWMIA DOM" << std::endl;

    AirSeg airSeg;
    airSeg.pnrSegment() = 2;
    airSeg.carrier() = "JH";
    airSeg.flightNumber() = 2424;
    airSeg.setBookingCode( "Y" );
    airSeg.departureDT() = DateTime( 2010, 3, 1 );
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "MIA";
    airSeg.geoTravelType() = GeoTravelType::Domestic;
    _collector->printTravelSegment( &airSeg );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintTravelSegments()
  {
    std::stringstream expectedDiag;
    expectedDiag << " 1 JH2424Y  01MAR DFWMIA DOM" << std::endl
                 << " 2 MA8888Y  01MAR MIANCE INT" << std::endl;

    AirSeg airSeg;
    airSeg.pnrSegment() = 1;
    airSeg.carrier() = "JH";
    airSeg.flightNumber() = 2424;
    airSeg.setBookingCode( "Y" );
    airSeg.departureDT() = DateTime( 2010, 3, 1 );
    airSeg.origAirport() = "DFW";
    airSeg.destAirport() = "MIA";
    airSeg.geoTravelType() = GeoTravelType::Domestic;

    AirSeg airSeg2;
    airSeg2.pnrSegment() = 2;
    airSeg2.carrier() = "MA";
    airSeg2.flightNumber() = 8888;
    airSeg2.setBookingCode( "Y" );
    airSeg2.departureDT() = DateTime( 2010, 3, 1 );
    airSeg2.origAirport() = "MIA";
    airSeg2.destAirport() = "NCE";
    airSeg2.geoTravelType() = GeoTravelType::International;

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back( &airSeg );
    travelSegs.push_back( &airSeg2 );
    _collector->printTravelSegments( travelSegs );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintSideTrips_None()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl;

    FareMarket fm;
    _collector->printSideTrips( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintSideTrips_Single()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 1" << std::endl
                 << "SIDE TRIP NUMBER: 1" << std::endl
                 << " 1 JH111 Y  23JUN CORAEP DOM" << std::endl
                 << " 2 MA222 Y  23JUN AEPUSH DOM" << std::endl;

    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    airSeg1->carrier()       = "JH";
    airSeg1->flightNumber()  = 111;
    airSeg2->carrier()      = "MA";
    airSeg2->flightNumber() = 222;

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back( airSeg1 );
    travelSegs.push_back( airSeg2 );
    FareMarket fm;
    fm.sideTripTravelSeg().push_back( travelSegs );

    _collector->printSideTrips( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintSideTrips_Multi()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 2" << std::endl
                 << "SIDE TRIP NUMBER: 1" << std::endl
                 << " 2 JH111 Y  23JUN CORAEP DOM" << std::endl
                 << " 3 JH222 Y  23JUN AEPUSH DOM" << std::endl
                 << "SIDE TRIP NUMBER: 2" << std::endl
                 << " 4 MA2424Y  01MAY CUNMEX DOM" << std::endl;

    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    airSeg1->pnrSegment()   = 2;
    airSeg1->carrier()      = "JH";
    airSeg1->flightNumber() = 111;
    airSeg2->pnrSegment()   = 3;
    airSeg2->carrier()      = "JH";
    airSeg2->flightNumber() = 222;

    AirSeg* airSeg3 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCUN_MEX.xml");
    airSeg3->pnrSegment()    = 4;
    airSeg3->carrier()       = "MA";
    airSeg3->flightNumber()  = 2424;

    std::vector<TravelSeg*> sideTrip1;
    sideTrip1.push_back( airSeg1 );
    sideTrip1.push_back( airSeg2 );
    std::vector<TravelSeg*> sideTrip2;
    sideTrip2.push_back( airSeg3 );

    FareMarket fm;
    fm.sideTripTravelSeg().push_back( sideTrip1 );
    fm.sideTripTravelSeg().push_back( sideTrip2 );

    _collector->printSideTrips( fm );
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPricingTrx()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** START DIAG 188 ***********************" << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***"      << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "NUMBER OF FARE MARKETS IN TRX: 1" << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 1 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl
                 << "  " << std::endl
                 << "*********************** END   DIAG 188 ***********************" << std::endl;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // Non-domestic for TPM
    fm->primarySector() = fm->travelSeg().front();

    PricingTrx prcTrx;
    PricingOptions prcOptions;
    prcTrx.setOptions( &prcOptions );
    PricingRequest request;
    prcTrx.setRequest( &request );
    prcTrx.fareMarket().push_back( fm );
    prcTrx.travelSeg() = fm->travelSeg();
    _collector->_trx = &prcTrx;

    *_collector << prcTrx;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testExchangePricingTrx()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** START DIAG 188 ***********************" << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***"      << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** EXCHANGE ITIN ***"      << std::endl
                 << " 1 JH111 Y  23JUN CORAEP DOM" << std::endl
                 << " 2 JH222 Y  23JUN AEPUSH DOM" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      "       << std::endl
                 << "AEP-JH-USH    1470      8194  " << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "NUMBER OF FARE MARKETS IN TRX: 1" << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 1 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl
                 << "  " << std::endl
                 << "*********************** END   DIAG 188 ***********************" << std::endl;

    AirSeg* excSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    AirSeg* excSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    excSeg1->pnrSegment()   = 1;
    excSeg1->carrier()      = "JH";
    excSeg1->flightNumber() = 111;
    excSeg2->pnrSegment()   = 2;
    excSeg2->carrier()      = "JH";
    excSeg2->flightNumber() = 222;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // Non-domestic for TPM
    fm->primarySector() = fm->travelSeg().front();

    ExchangePricingTrx excTrx;
    PricingOptions prcOptions;
    excTrx.setOptions( &prcOptions );
    PricingRequest request;
    excTrx.setRequest( &request );
    excTrx.fareMarket().push_back( fm );
    excTrx.travelSeg() = fm->travelSeg();

    ExcItin excItin;
    excItin.travelSeg().push_back( excSeg1 );
    excItin.travelSeg().push_back( excSeg2 );
    excTrx.exchangeItin().push_back( &excItin );

    _collector->_trx = &excTrx;

    *_collector << excTrx;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testRefundPricingTrx()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** START DIAG 188 ***********************" << std::endl
                 << "  " << std::endl
                 << "*** FLOWN ITIN ***"      << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TICKETED ITIN ***"      << std::endl
                 << " 1 JH111 Y  23JUN CORAEP DOM" << std::endl
                 << " 2 JH222 Y  23JUN AEPUSH DOM" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      "       << std::endl
                 << "AEP-JH-USH    1470      8194  " << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "NUMBER OF FARE MARKETS IN TRX: 1" << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 1 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl
                 << "  " << std::endl
                 << "*********************** END   DIAG 188 ***********************" << std::endl;

    AirSeg* excSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    AirSeg* excSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    excSeg1->pnrSegment()   = 1;
    excSeg1->carrier()      = "JH";
    excSeg1->flightNumber() = 111;
    excSeg2->pnrSegment()   = 2;
    excSeg2->carrier()      = "JH";
    excSeg2->flightNumber() = 222;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // Non-domestic for TPM
    fm->primarySector() = fm->travelSeg().front();

    RefundPricingTrx refTrx;
    PricingOptions prcOptions;
    refTrx.setOptions( &prcOptions );
    PricingRequest request;
    refTrx.setRequest( &request );
    refTrx.fareMarket().push_back( fm );
    refTrx.travelSeg() = fm->travelSeg();

    Itin itin;
    itin.travelSeg() = fm->travelSeg();
    refTrx.newItin().push_back( &itin );

    ExcItin excItin;
    excItin.travelSeg().push_back( excSeg1 );
    excItin.travelSeg().push_back( excSeg2 );
    refTrx.exchangeItin().push_back( &excItin );

    _collector->_trx = &refTrx;

    *_collector << refTrx;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testRexPricingTrx()
  {
    std::stringstream expectedDiag;
    expectedDiag << "*********************** START DIAG 188 ***********************" << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***"      << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** EXCHANGE ITIN ***"      << std::endl
                 << " 1 JH111 Y  23JUN CORAEP DOM" << std::endl
                 << " 2 JH222 Y  23JUN AEPUSH DOM" << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "COR-JH-AEP    6724      "       << std::endl
                 << "AEP-JH-USH    1470      8194  " << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "NUMBER OF FARE MARKETS IN TRX: 1" << std::endl
                 << "  " << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "*** FARE MARKET 1 ***" << std::endl
                 << "--------------------------------------------------------------" << std::endl
                 << "ORIGIN-DESTINATION : DFW-LON" << std::endl
                 << "GOVERNING CARRIER  : AA" << std::endl
                 << "GLOBAL DIRECTION   : AT   -VIA ATLANTIC" << std::endl
                 << "GEO TRAVEL TYPE    : INTERNATIONAL" << std::endl
                 << "DIRECTION          : OUTBOUND" << std::endl
                 << "TRAVEL DATE        : 2010-03-01" << std::endl
                 << "  " << std::endl
                 << "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: " << std::endl
                 << "  " << std::endl
                 << "SEGMENTS      TPM       TOTAL"  << std::endl
                 << "DFW-AA-LAX    1238      1238  " << std::endl
                 << "LAX-BA-LON    5454      5454  " << std::endl
                 << "  " << std::endl
                 << "*** TRAVEL SEGMENTS ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << " 1 BA0   Y  23JUN LAXLON INT" << std::endl
                 << "*** PRIMARY SECTOR ***" << std::endl
                 << " 1 AA0   Y  23JUN DFWLAX INT" << std::endl
                 << "*** SIDE TRIPS ***" << std::endl
                 << "NUMBER OF SIDE TRIPS: 0" << std::endl
                 << "  " << std::endl
                 << "  " << std::endl
                 << "*********************** END   DIAG 188 ***********************" << std::endl;

    AirSeg* excSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    AirSeg* excSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");
    excSeg1->pnrSegment()   = 1;
    excSeg1->carrier()      = "JH";
    excSeg1->flightNumber() = 111;
    excSeg2->pnrSegment()   = 2;
    excSeg2->carrier()      = "JH";
    excSeg2->flightNumber() = 222;

    FareMarket* fm =
        TestFareMarketFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm->travelDate() = DateTime( 2010, 3, 1 );
    fm->travelBoundary() = FMTravelBoundary::TravelWithinTwoIATA; // Non-domestic for TPM
    fm->primarySector() = fm->travelSeg().front();

    RexPricingTrx rexTrx;
    PricingOptions prcOptions;
    rexTrx.setOptions( &prcOptions );
    PricingRequest request;
    rexTrx.setRequest( &request );
    rexTrx.fareMarket().push_back( fm );
    rexTrx.travelSeg() = fm->travelSeg();

    ExcItin excItin;
    excItin.travelSeg().push_back( excSeg1 );
    excItin.travelSeg().push_back( excSeg2 );
    rexTrx.exchangeItin().push_back( &excItin );

    _collector->_trx = &rexTrx;

    *_collector << rexTrx;
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

private:
  Diag188Collector* _collector;
  TestMemHandle _memHandle;
  PricingTrx* _trx;

};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag188CollectorTest);
} // NS tse
