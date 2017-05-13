//
// Copyright Sabre 2011-12-19
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockTseServer.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "DataModel/AirSeg.h"
#include "DataModel/Diversity.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "ItinAnalyzer/ItinAnalyzerServiceWrapper.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace tse
{
class ItinAnalyzerServiceWrapperTest : public CppUnit::TestFixture
{

  class MyDataHandle : public DataHandleMock
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

    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if ( locCode == "DFW" )
        return "DFW";
      else if ( locCode == "LAX" )
        return "LAX";
      else if ( locCode == "LON" )
        return "LON";

      return DataHandleMock::getMultiTransportCityCode( locCode, carrierCode, tvlType, tvlDate );
    }

  };

  CPPUNIT_TEST_SUITE(ItinAnalyzerServiceWrapperTest);
  CPPUNIT_TEST(testReuseOfFareMarkets);
  CPPUNIT_TEST(testNoReuseOfFareMarketsForCarnivalSumOfLocal);
  CPPUNIT_TEST(testParseAcrossStopoverExcludedCarriersCitie);
  CPPUNIT_TEST(testisAcrossStopoverDomesticUSFareMarket);
  CPPUNIT_TEST(testAtaeRedundantMarketsCallForRtw);
  CPPUNIT_TEST(testCollectSegmentsForRtw);
  CPPUNIT_TEST(testCollectSegmentsForRtw_AddedArunk);
  CPPUNIT_TEST(testSelectGovCxrs_Within_USCA);
  CPPUNIT_TEST(testSelectGovCxrs_Tpm);
  CPPUNIT_TEST(testSelectGovCxrs_Vctr_Match);
  CPPUNIT_TEST(testSelectGovCxrs_Vctr_NoMatch);
  CPPUNIT_TEST(testIsVctrCarrier_No_FM);
  CPPUNIT_TEST(testIsVctrCarrier_No_FCI);
  CPPUNIT_TEST(testIsVctrCarrier_No_VCTR);
  CPPUNIT_TEST(testIsVctrCarrier_VCTR_Match);
  CPPUNIT_TEST(testIsVctrCarrier_VCTR_NoMatch);
  CPPUNIT_TEST_SUITE_END();

  MyDataHandle* _myDataHandle;
  TestMemHandle _memHandle;
  MockTseServer* _tseServer;
  ItinAnalyzerService* _itinAnalyzer;
  ItinAnalyzerServiceWrapper* _itinAnalyzerWrapper;

  Itin* itin;
  ShoppingTrx* shoppingTrx;
  PricingOptions* _options;

  FareMarket* fm1;
  FareMarket* fm2;

public:
  void setUp()
  {
    _tseServer = _memHandle.create<MockTseServer>();
    _myDataHandle = _memHandle.create<MyDataHandle>();

    _itinAnalyzer =
        _memHandle.insert<ItinAnalyzerService>(new ItinAnalyzerService("ITIN_SVC", *_tseServer));
    _itinAnalyzerWrapper = _memHandle.insert<ItinAnalyzerServiceWrapper>(
        new ItinAnalyzerServiceWrapper(*_itinAnalyzer));

    itin = _memHandle.create<Itin>();
    shoppingTrx = _memHandle.create<ShoppingTrx>();
    _options = _memHandle.create<PricingOptions>();
    shoppingTrx->setOptions(_options);

    fm1 = TestFareMarketFactory::create(TSE_VOB_DIR "/test/sampledata/DFW_LAX_LON_FareMarket.xml");
    fm2 = TestFareMarketFactory::create(TSE_VOB_DIR "/test/sampledata/DFW_LAX_LON_FareMarket.xml");

    shoppingTrx->setRequest(_memHandle.create<PricingRequest>());
}

  void tearDown()
  {
    _memHandle.clear();
  }

  void testReuseOfFareMarkets()
  {
    //AirSeg seg;
    //fm1->travelSeg().push_back(&seg);
    //fm2->travelSeg().push_back(&seg);

    //PricingRequest req;
    //shoppingTrx->setRequest(&req);

    _itinAnalyzerWrapper->storeFareMarket(*shoppingTrx, fm1, *itin, true);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), shoppingTrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(fm1, itin->fareMarket().at(0));

    _itinAnalyzerWrapper->storeFareMarket(*shoppingTrx, fm2, *itin, true);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), shoppingTrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(fm2, itin->fareMarket().at(1));

    _itinAnalyzerWrapper->storeFareMarket(*shoppingTrx, fm1, *itin, true);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), shoppingTrx->fareMarket().size());
  }

  void testNoReuseOfFareMarketsForCarnivalSumOfLocal()
  {
    shoppingTrx->getOptions()->setCarnivalSumOfLocal(true);

    _itinAnalyzerWrapper->storeFareMarket(*shoppingTrx, fm1, *itin, true);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), shoppingTrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(fm1, itin->fareMarket().at(0));

    _itinAnalyzerWrapper->storeFareMarket(*shoppingTrx, fm2, *itin, true);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), shoppingTrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(fm2, itin->fareMarket().at(1));

    _itinAnalyzerWrapper->storeFareMarket(*shoppingTrx, fm1, *itin, true);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), shoppingTrx->fareMarket().size());
  }

  void testParseAcrossStopoverExcludedCarriersCitie()
  {

    std::string acrossStopoverEnabledCarriersConfig("SY^*|F9^TST^TES|US^DEN");
    _itinAnalyzerWrapper->parseAcrossStopoverExcludedCarriersCities(
        acrossStopoverEnabledCarriersConfig);

    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.size() == 3);

    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.find("SY") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.end());
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["SY"].size() == 1);
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["SY"].find("*") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["SY"].end());

    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.find("F9") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.end());
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["F9"].size() == 2);
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["F9"].find("TST") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["F9"].end());
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["F9"].find("TES") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["F9"].end());

    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.find("US") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities.end());
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["US"].size() == 1);
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["US"].find("DEN") !=
                   _itinAnalyzerWrapper->_acrossStopoverEnabledCarriersCities["US"].end());
  }

  void testisAcrossStopoverDomesticUSFareMarket()
  {
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    AirSeg* a4 = _memHandle.create<AirSeg>();
    Loc* loc1 = _memHandle.create<Loc>();
    Loc* loc2 = _memHandle.create<Loc>();
    Loc* loc3 = _memHandle.create<Loc>();
    Loc* loc4 = _memHandle.create<Loc>();
    Loc* loc5 = _memHandle.create<Loc>();
    Loc* loc6 = _memHandle.create<Loc>();
    Loc* loc7 = _memHandle.create<Loc>();
    Loc* loc8 = _memHandle.create<Loc>();
    a1->origin() = loc1;
    a1->destination() = loc2;
    a2->origin() = loc3;
    a2->destination() = loc4;
    a3->origin() = loc5;
    a3->destination() = loc6;
    a4->origin() = loc7;
    a4->destination() = loc8;

    a2->stopOver() = true;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3, (TravelSeg*)a4;

    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3, (TravelSeg*)a4;
    fm->geoTravelType() = GeoTravelType::Domestic;
    fm->setOrigDestByTvlSegs();

    itin->fareMarket() += fm;

    std::string acrossStopoverEnabledCarriersConfig("SY^*|F9^*|US^DEN");
    _itinAnalyzerWrapper->parseAcrossStopoverExcludedCarriersCities(
        acrossStopoverEnabledCarriersConfig);

    CPPUNIT_ASSERT(_itinAnalyzerWrapper->isAcrossStopoverDomesticUSFareMarket(*fm));

    fm->governingCarrier() = "SY";
    CPPUNIT_ASSERT(!_itinAnalyzerWrapper->isAcrossStopoverDomesticUSFareMarket(*fm));

    fm->governingCarrier() = "US";
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->isAcrossStopoverDomesticUSFareMarket(*fm));

    CPPUNIT_ASSERT(loc8 == fm->travelSeg().back()->destination());
    loc8->city() = "DEN";
    CPPUNIT_ASSERT(!_itinAnalyzerWrapper->isAcrossStopoverDomesticUSFareMarket(*fm));
  }

  class IASmock : public ItinAnalyzerService
  {
  public:
    IASmock(const std::string& name, TseServer& srv) : ItinAnalyzerService(name, srv) {}

  private:
    virtual void setATAEContent(PricingTrx& trx)
    {
      return;
    };
  };

  AirSeg* createSeg(const LocCode& org, const LocCode& dst)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origAirport() = org;
    seg->destAirport() = dst;
    seg->boardMultiCity() = org;
    seg->offMultiCity() = dst;
    Loc* orgLoc = _memHandle.create<Loc>();
    orgLoc->loc() = org;
    seg->origin() = orgLoc;
    Loc* dstLoc = _memHandle.create<Loc>();
    dstLoc->loc() = dst;
    seg->destination() = dstLoc;
    return seg;
  }

  void testAtaeRedundantMarketsCallForRtw()
  {
    PricingTrx trx;
    _options->setRtw(true);
    trx.setOptions(_options);
    trx.itin().push_back(itin);
    itin->travelSeg() += createSeg("JFK", "LON"), createSeg("LON", "JFK");

    for (size_t i = 0; i < 3; ++i)
      itin->fareMarket().push_back(_memHandle(new FareMarket));
    itin->fareMarket()[0]->travelSeg() += itin->travelSeg()[0];
    itin->fareMarket()[1]->travelSeg() += itin->travelSeg()[1];
    itin->fareMarket()[2]->travelSeg() = itin->travelSeg();

    trx.fareMarket() = itin->fareMarket();
    FareMarket* last = trx.fareMarket().back();

    IASmock iasm("ITIN_SVC", *_tseServer);
    ItinAnalyzerServiceWrapper iasw(iasm);
    iasw.setATAEContent(trx);

    CPPUNIT_ASSERT_EQUAL(size_t(1), trx.fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(last, trx.fareMarket().back());
    CPPUNIT_ASSERT_EQUAL(last, itin->fareMarket().back());
  }

  void testCollectSegmentsForRtw()
  {
    PricingTrx& trx = *shoppingTrx;
    Itin itin;
    itin.travelSeg() += createSeg("NYC", "PAR"), createSeg("PAR", "NYC");

    std::vector<TravelSeg*> segs;
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->collectSegmentsForRtw(trx, itin, segs));
    CPPUNIT_ASSERT_EQUAL(size_t(2), itin.travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), segs.size());
  }

  void testCollectSegmentsForRtw_AddedArunk()
  {
    PricingTrx& trx = *shoppingTrx;
    Itin itin;
    itin.travelSeg() += createSeg("NYC", "PAR"), createSeg("PAR", "WAW");

    std::vector<TravelSeg*> segs;
    CPPUNIT_ASSERT(_itinAnalyzerWrapper->collectSegmentsForRtw(trx, itin, segs));
    CPPUNIT_ASSERT_EQUAL(size_t(3), itin.travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), segs.size());

    TravelSeg& arunk = *segs.back();
    LocCode waw = "WAW";
    LocCode nyc = "NYC";

    CPPUNIT_ASSERT_EQUAL(Surface, arunk.segmentType());
    CPPUNIT_ASSERT_EQUAL(ARUNK_PNR_SEGMENT_ORDER, arunk.pnrSegment());
    CPPUNIT_ASSERT_EQUAL(waw, arunk.origAirport());
    CPPUNIT_ASSERT_EQUAL(waw, arunk.origin()->loc());
    CPPUNIT_ASSERT_EQUAL(waw, arunk.boardMultiCity());
    CPPUNIT_ASSERT_EQUAL(nyc, arunk.destAirport());
    CPPUNIT_ASSERT_EQUAL(nyc, arunk.destination()->loc());
    CPPUNIT_ASSERT_EQUAL(nyc, arunk.offMultiCity());
  }

  void testSelectGovCxrs_Within_USCA()
  {
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest( &request );
    trx.setOptions( _options );
    trx.setIataFareSelectionApplicable( true );
    Itin itin;
    bool checkIfMarketAlreadyAdded = true;
    bool fareMarketAdded = false;
    bool fmBreakSet = fm1->breakIndicator();
    std::vector<CarrierCode> govCxrOverrides;
    std::set<CarrierCode> participatingCarriers;
    fm1->primarySector() = fm1->travelSeg().front();
    fm1->travelBoundary() = FMTravelBoundary::TravelWithinUSCA;

    CPPUNIT_ASSERT( "AA" == fm1->governingCarrier() );
    CPPUNIT_ASSERT( trx.fareMarket().empty() );
    CPPUNIT_ASSERT( itin.fareMarket().empty() );

    _itinAnalyzerWrapper->selectGovCxrs(
        trx, itin, fm1,
        checkIfMarketAlreadyAdded, fareMarketAdded, fmBreakSet,
        govCxrOverrides, participatingCarriers, 0 );

    CPPUNIT_ASSERT( fareMarketAdded );
    CPPUNIT_ASSERT( 1 == trx.fareMarket().size() );
    CPPUNIT_ASSERT( 1 == itin.fareMarket().size() );
    CPPUNIT_ASSERT( "AA" == trx.fareMarket()[0]->governingCarrier() );
    CPPUNIT_ASSERT( !trx.fareMarket()[0]->isHighTPMGoverningCxr() );
  }

  void testSelectGovCxrs_Tpm()
  {
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest( &request );
    trx.setOptions( _options );
    trx.setIataFareSelectionApplicable( true );
    Itin itin;
    bool checkIfMarketAlreadyAdded = true;
    bool fareMarketAdded = false;
    bool fmBreakSet = fm1->breakIndicator();
    std::vector<CarrierCode> govCxrOverrides;
    std::set<CarrierCode> participatingCarriers;
    fm1->primarySector() = fm1->travelSeg().front();
    fm1->travelBoundary() = FMTravelBoundary::TravelWithinOneIATA;

    CPPUNIT_ASSERT( "AA" == fm1->governingCarrier() );
    CPPUNIT_ASSERT( trx.fareMarket().empty() );
    CPPUNIT_ASSERT( itin.fareMarket().empty() );

    _itinAnalyzerWrapper->selectGovCxrs(
        trx, itin, fm1,
        checkIfMarketAlreadyAdded, fareMarketAdded, fmBreakSet,
        govCxrOverrides, participatingCarriers,0 );

    CPPUNIT_ASSERT( fareMarketAdded );
    CPPUNIT_ASSERT( 2 == trx.fareMarket().size() );
    CPPUNIT_ASSERT( 2 == itin.fareMarket().size() );
    CPPUNIT_ASSERT( "AA" == trx.fareMarket()[0]->governingCarrier() );
    CPPUNIT_ASSERT( !trx.fareMarket()[0]->isHighTPMGoverningCxr() );
    CPPUNIT_ASSERT( "BA" == trx.fareMarket()[1]->governingCarrier() );
    CPPUNIT_ASSERT( trx.fareMarket()[1]->isHighTPMGoverningCxr() );
  }

  // A cloned fare market is not created, if the original fare market governing carrier
  // matches the given VCTR carrier. This would only apply to exchange processing.
  void testSelectGovCxrs_Vctr_Match()
  {
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest( &request );
    trx.setOptions( _options );
    trx.setIataFareSelectionApplicable( true );
    Itin itin;
    bool checkIfMarketAlreadyAdded = true;
    bool fareMarketAdded = false;
    bool fmBreakSet = fm1->breakIndicator();
    std::vector<CarrierCode> govCxrOverrides;
    std::set<CarrierCode> participatingCarriers;
    fm1->primarySector() = fm1->travelSeg().front();
    fm1->travelBoundary() = FMTravelBoundary::TravelWithinOneIATA;

    FareCompInfo fci;
    fci.hasVCTR() = true;
    fci.VCTR().carrier() = fm1->governingCarrier();
    fm1->fareCompInfo() = &fci;

    CPPUNIT_ASSERT( "AA" == fm1->governingCarrier() );
    CPPUNIT_ASSERT( trx.fareMarket().empty() );
    CPPUNIT_ASSERT( itin.fareMarket().empty() );

    _itinAnalyzerWrapper->selectGovCxrs(
        trx, itin, fm1,
        checkIfMarketAlreadyAdded, fareMarketAdded, fmBreakSet,
        govCxrOverrides, participatingCarriers,0 );

    CPPUNIT_ASSERT( fareMarketAdded );
    CPPUNIT_ASSERT( 1 == trx.fareMarket().size() );
    CPPUNIT_ASSERT( 1 == itin.fareMarket().size() );
    CPPUNIT_ASSERT( "AA" == trx.fareMarket()[0]->governingCarrier() );
    CPPUNIT_ASSERT( !trx.fareMarket()[0]->isHighTPMGoverningCxr() );
  }

  // If the given VCTR carrier doesn't match the fare market governing carrier, then the
  // fare market is cloned if a TPM carrier exists. This would only apply to exchange processing.
  void testSelectGovCxrs_Vctr_NoMatch()
  {
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest( &request );
    trx.setOptions( _options );
    trx.setIataFareSelectionApplicable( true );
    Itin itin;
    bool checkIfMarketAlreadyAdded = true;
    bool fareMarketAdded = false;
    bool fmBreakSet = fm1->breakIndicator();
    std::vector<CarrierCode> govCxrOverrides;
    std::set<CarrierCode> participatingCarriers;
    fm1->primarySector() = fm1->travelSeg().front();
    fm1->travelBoundary() = FMTravelBoundary::TravelWithinOneIATA;

    FareCompInfo fci;
    fci.hasVCTR() = true;
    fci.VCTR().carrier() = "GC";
    fm1->fareCompInfo() = &fci;

    CPPUNIT_ASSERT( "AA" == fm1->governingCarrier() );
    CPPUNIT_ASSERT( trx.fareMarket().empty() );
    CPPUNIT_ASSERT( itin.fareMarket().empty() );

    _itinAnalyzerWrapper->selectGovCxrs(
        trx, itin, fm1,
        checkIfMarketAlreadyAdded, fareMarketAdded, fmBreakSet,
        govCxrOverrides, participatingCarriers, 0 );

    CPPUNIT_ASSERT( fareMarketAdded );
    CPPUNIT_ASSERT( 2 == trx.fareMarket().size() );
    CPPUNIT_ASSERT( 2 == itin.fareMarket().size() );
    CPPUNIT_ASSERT( "AA" == trx.fareMarket()[0]->governingCarrier() );
    CPPUNIT_ASSERT( !trx.fareMarket()[0]->isHighTPMGoverningCxr() );
    CPPUNIT_ASSERT( "BA" == trx.fareMarket()[1]->governingCarrier() );
    CPPUNIT_ASSERT( trx.fareMarket()[1]->isHighTPMGoverningCxr() );
  }

  void testIsVctrCarrier_No_FM()
  {
    FareMarket* nullFareMarket = 0;
    CPPUNIT_ASSERT( !_itinAnalyzerWrapper->isVctrCarrier( nullFareMarket ) );
  }

  void testIsVctrCarrier_No_FCI()
  {
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "GC";
    CPPUNIT_ASSERT( !_itinAnalyzerWrapper->isVctrCarrier( &fareMarket ) );
  }

  void testIsVctrCarrier_No_VCTR()
  {
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "GC";
    FareCompInfo fci;
    fci.fareMarket() = &fareMarket;
    fci.hasVCTR() = false;
    fareMarket.fareCompInfo() = &fci;
    CPPUNIT_ASSERT( !_itinAnalyzerWrapper->isVctrCarrier( &fareMarket ) );
  }

  void testIsVctrCarrier_VCTR_Match()
  {
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "GC";
    FareCompInfo fci;
    fci.fareMarket() = &fareMarket;
    fci.VCTR() = VCTR("ATP", "GC", 11, "13", 0);
    fci.hasVCTR() = true;
    fareMarket.fareCompInfo() = &fci;
    CPPUNIT_ASSERT( _itinAnalyzerWrapper->isVctrCarrier( &fareMarket ) );
  }

  void testIsVctrCarrier_VCTR_NoMatch()
  {
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "G1";
    FareCompInfo fci;
    fci.fareMarket() = &fareMarket;
    fci.VCTR() = VCTR("ATP", "G2", 11, "13", 0);
    fci.hasVCTR() = true;
    fareMarket.fareCompInfo() = &fci;
    CPPUNIT_ASSERT( !_itinAnalyzerWrapper->isVctrCarrier( &fareMarket ) );
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinAnalyzerServiceWrapperTest);
}
