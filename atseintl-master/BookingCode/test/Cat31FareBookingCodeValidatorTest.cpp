
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Cabin.h"
#include <boost/ptr_container/ptr_set.hpp>

namespace
{
std::ostream& operator<<(std::ostream& s, tse::Cat31FareBookingCodeValidator::Result result)
{
  switch (result)
  {
  case tse::Cat31FareBookingCodeValidator::FAILED:
    s << "FAILED";
    break;
  case tse::Cat31FareBookingCodeValidator::PASSED:
    s << "PASSED";
    break;
  case tse::Cat31FareBookingCodeValidator::SKIPPED:
    s << "SKIPPED";
    break;
  default:
    s << "POSTPONED_TO_PHASE2";
    break;
  }
  return s;
}
} // namespace

namespace tse
{
namespace
{
CabinType
ToCabinType(const BookingCode& bookingCode)
{
  CabinType cabinType;
  if (bookingCode == "F")
    cabinType.setPremiumFirstClass();
  else if (bookingCode == "D")
    cabinType.setFirstClass();
  else if (bookingCode == "C")
    cabinType.setPremiumBusinessClass();
  else if (bookingCode == "A")
    cabinType.setBusinessClass();
  else if (bookingCode == "Y")
    cabinType.setPremiumEconomyClass();
  else
    cabinType.setEconomyClass();
  return cabinType;
}

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& /*date*/)
  {
    Cabin* cabin = _memHandle.create<Cabin>();
    cabin->carrier() = carrier;
    cabin->classOfService() = classOfService;
    cabin->cabin() = ToCabinType(classOfService);
    return cabin;
  }
};

class MockAirSeg : public AirSeg
{
public:
  MockAirSeg(const LocCode& board,
             const LocCode& off,
             const CarrierCode& carrierCode,
             const BookingCode& bookingCode,
             bool flown,
             const CabinType& cabinType)
  {
    origAirport() = board;
    destAirport() = off;
    boardMultiCity() = board;
    offMultiCity() = off;
    segmentType() = Air;
    carrier() = carrierCode;
    bookedCabin() = cabinType;

    const FlightNumber flight = 70;
    flightNumber() = flight;
    setBookingCode(bookingCode);
    unflown() = !flown;
  }

  MockAirSeg(const LocCode& board,
             const LocCode& off,
             const CarrierCode& carrierCode,
             const BookingCode& bookingCode,
             bool flown)
    : MockAirSeg(board, off, carrierCode, bookingCode, flown, ToCabinType(bookingCode))
  {
  }
};

class MockFareMarket : public FareMarket
{
public:
  MockFareMarket(const LocCode& boardCity, const LocCode& offCity, const CarrierCode& carrierCode)
  {
    boardMultiCity() = boardCity;
    offMultiCity() = offCity;
    governingCarrier() = carrierCode;
  }
};

class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(FareMarket& fm, MoneyAmount amount, const BookingCode& primeBookingCode)
  {
    setFare(&_fare);
    _fare.setFareInfo(&_fareInfo);
    fareMarket() = &fm;
    fareClassAppInfo() = &_fareClassAppInfo;
    fareClassAppSegInfo() = &_fareClassAppSegInfo;
    _fareClassAppSegInfo._bookingCode[0] = primeBookingCode;
    nucFareAmount() = amount;
    setIsShoppingFare(); // make sure isValidNoBookingCode() returns true
  }

  MockPaxTypeFare(FareMarket& fm,
                  const Indicator& normalOrSpecial,
                  const BookingCode& primeBookingCode,
                  MoneyAmount amount = MoneyAmount())
    : MockPaxTypeFare(fm, amount, primeBookingCode)
  {
    _fareClassAppInfo._pricingCatType = normalOrSpecial;
  }

  bool operator<(const PaxTypeFare& rhs) const { return nucFareAmount() < rhs.nucFareAmount(); }

private:
  Fare _fare;
  FareInfo _fareInfo;
  FareClassAppInfo _fareClassAppInfo;
  FareClassAppSegInfo _fareClassAppSegInfo;
};

class MockExcItin : public ExcItin
{
public:
  MockExcItin()
  {
    _farePath.pricingUnit().push_back(&_pricingUnit);
    farePath().push_back(&_farePath);
  }

  void addPaxTypeFare(PaxTypeFare* paxTypeFare)
  {
    const std::vector<TravelSeg*>& travelSegs = paxTypeFare->fareMarket()->travelSeg();
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = paxTypeFare;
    fareUsage->travelSeg() = travelSegs;
    _pricingUnit.fareUsage().push_back(fareUsage);
    std::copy(travelSegs.begin(), travelSegs.end(), std::back_inserter(travelSeg()));
  }

private:
  TestMemHandle _memHandle;
  PricingUnit _pricingUnit;
  FarePath _farePath;
};

class MockRexPricingTrx : public RexPricingTrx
{
public:
  MockRexPricingTrx(ExcItin& excItin)
  {
    trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    exchangeItin().push_back(&excItin);
    setExcTrxType(PricingTrx::AF_EXC_TRX);
  }

  virtual bool matchFareRetrievalDate(const PaxTypeFare& /*paxTypeFare*/) { return true; }
};

} // namespace

class Cat31FareBookingCodeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Cat31FareBookingCodeValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testAtpcoExample1);
  CPPUNIT_TEST(testAtpcoExample2);
  CPPUNIT_TEST(testAtpcoExample3);
  CPPUNIT_TEST(testAtpcoExample4);
  CPPUNIT_TEST(testAtpcoExample5);
  CPPUNIT_TEST(testAtpcoExample6);
  CPPUNIT_TEST(testAtpcoExample7);
  CPPUNIT_TEST(testAtpcoExample8);
  CPPUNIT_TEST(testAtpcoExample9);
  CPPUNIT_TEST(testAtpcoExample10);
  CPPUNIT_TEST(testAtpcoExample11);
  CPPUNIT_TEST(testAtpcoExample12);
  CPPUNIT_TEST(testAtpcoExample13);
  CPPUNIT_TEST(testAtpcoExample14);
  CPPUNIT_TEST(testAtpcoExample15);
  CPPUNIT_TEST(testAtpcoExample16);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testConstructor()
  {
    RexPricingTrx trx;
    FareMarket fareMarket;
    Itin itin;
    CPPUNIT_ASSERT_NO_THROW(Cat31FareBookingCodeValidator(trx, fareMarket, itin));
  }

  void testAtpcoExample1()
  {
    MockAirSeg mcnAtlSeg("MCN", "ATL", "DL", "Y", true);
    MockAirSeg atlHnlSeg("ATL", "HNL", "DL", "Y", false);
    MockAirSeg atlOggSeg("ATL", "OGG", "DL", "Y", false);

    MockFareMarket mcnHnlMarket("MCN", "HNL", "DL");
    mcnHnlMarket.travelSeg().push_back(&mcnAtlSeg);
    mcnHnlMarket.travelSeg().push_back(&atlHnlSeg);

    MockFareMarket mcnOggMarket("MCN", "OGG", "DL");
    mcnOggMarket.travelSeg().push_back(&mcnAtlSeg);
    mcnOggMarket.travelSeg().push_back(&atlOggSeg);

    MockPaxTypeFare prevFare(mcnHnlMarket, 'N', "Y");
    MockPaxTypeFare replFare(mcnOggMarket, 'N', "Y", 1200.0);

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::Domestic;
    replItin.travelSeg() = mcnOggMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator mcnOggValidator(trx, mcnOggMarket, replItin);

    CPPUNIT_ASSERT(mcnOggValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::SKIPPED,
                         mcnOggValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(mcnOggValidator.validateCat33(replFare));
  }

  void testAtpcoExample2()
  {
    MockAirSeg mcnAtlSeg("MCN", "ATL", "DL", "B", true);
    MockAirSeg atlHnlSeg("ATL", "HNL", "DL", "B", false);
    MockAirSeg atlOggSeg("ATL", "OGG", "DL", "Y", false);

    MockFareMarket mcnHnlMarket("MCN", "HNL", "DL");
    mcnHnlMarket.travelSeg().push_back(&mcnAtlSeg);
    mcnHnlMarket.travelSeg().push_back(&atlHnlSeg);

    MockFareMarket mcnOggMarket("MCN", "OGG", "DL");
    mcnOggMarket.travelSeg().push_back(&mcnAtlSeg);
    mcnOggMarket.travelSeg().push_back(&atlOggSeg);

    MockPaxTypeFare prevFare(mcnHnlMarket, 'S', "B");
    MockPaxTypeFare replFare(mcnOggMarket, 'N', "Y", 1700.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 3200.0, "F"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 2700.0, "C"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 1700.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 800.0, "B"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 797.0, "K"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 750.0, "K"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 727.0, "K"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 711.0, "M"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 499.0, "W"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 460.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(mcnOggMarket, 379.0, "Z"));

    for (PaxTypeFare& fare : otherFares)
    {
      mcnOggMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::Domestic;
    replItin.travelSeg() = mcnOggMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator mcnOggValidator(trx, mcnOggMarket, replItin);

    CPPUNIT_ASSERT(mcnOggValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         mcnOggValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(!mcnOggValidator.validateCat33(replFare));
  }

  void testAtpcoExample3()
  {
    MockAirSeg chiDenSeg("CHI", "DEN", "UA", "Q", true);
    MockAirSeg denLaxSeg("DEN", "LAX", "UA", "Q", false);
    MockAirSeg denSfoSeg("DEN", "SFO", "UA", "M", false);

    MockFareMarket chiLaxMarket("CHI", "LAX", "UA");
    chiLaxMarket.travelSeg().push_back(&chiDenSeg);
    chiLaxMarket.travelSeg().push_back(&denLaxSeg);

    MockFareMarket chiSfoMarket("CHI", "SFO", "UA");
    chiSfoMarket.travelSeg().push_back(&chiDenSeg);
    chiSfoMarket.travelSeg().push_back(&denSfoSeg);

    MockPaxTypeFare prevFare(chiLaxMarket, 'S', "Q");
    MockPaxTypeFare replFare(chiSfoMarket, 'S', "M", 221.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 3200.0, "F"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 2700.0, "C"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 1700.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 800.0, "B"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 622.0, "H"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 500.0, "M"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 315.0, "H"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 221.0, "M"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 129.0, "W"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 127.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(chiSfoMarket, 98.0, "Z"));

    for (PaxTypeFare& fare : otherFares)
    {
      chiSfoMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::Domestic;
    replItin.travelSeg() = chiSfoMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator chiSfoValidator(trx, chiSfoMarket, replItin);

    CPPUNIT_ASSERT(chiSfoValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         chiSfoValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(chiSfoValidator.validateCat33(replFare));
  }

  void testAtpcoExample4()
  {
    MockAirSeg nycChiSeg("NYC", "CHI", "UA", "V", true);
    MockAirSeg chiDenSeg("CHI", "DEN", "UA", "W", true);
    MockAirSeg denSnaSeg("DEN", "SNA", "UA", "Q", false);
    MockAirSeg denSeaSeg("DEN", "SEA", "UA", "M", false);

    MockFareMarket nycChiMarket("NYC", "CHI", "UA");
    nycChiMarket.travelSeg().push_back(&nycChiSeg);

    MockFareMarket chiDenMarket("CHI", "DEN", "UA");
    chiDenMarket.travelSeg().push_back(&chiDenSeg);

    MockFareMarket denSnaMarket("DEN", "SNA", "UA");
    denSnaMarket.travelSeg().push_back(&denSnaSeg);

    MockFareMarket nycSeaMarket("NYC", "SEA", "UA");
    nycSeaMarket.travelSeg().push_back(&nycChiSeg);
    nycSeaMarket.travelSeg().push_back(&chiDenSeg);
    nycSeaMarket.travelSeg().push_back(&denSeaSeg);

    MockPaxTypeFare prevFare1(nycChiMarket, 'S', "V");
    MockPaxTypeFare prevFare2(nycChiMarket, 'S', "W");
    MockPaxTypeFare prevFare3(nycChiMarket, 'S', "Q");
    MockPaxTypeFare replFare(nycSeaMarket, 'S', "M", 330.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 2600.0, "F"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 2100.0, "C"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 1700.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 900.0, "B"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 762.0, "B"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 415.0, "H"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 360.0, "V"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 330.0, "M"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 315.0, "W"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 300.0, "S"));
    otherFares.insert(new MockPaxTypeFare(nycSeaMarket, 297.0, "Q"));

    for (PaxTypeFare& fare : otherFares)
    {
      nycSeaMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);
    prevItin.addPaxTypeFare(&prevFare3);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::Domestic;
    replItin.travelSeg() = nycSeaMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator nycSeaValidator(trx, nycSeaMarket, replItin);

    CPPUNIT_ASSERT(nycSeaValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::FAILED,
                         nycSeaValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(nycSeaValidator.validateCat33(replFare));
  }

  void testAtpcoExample5()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "UA", "Y", true);
    MockAirSeg nycLonSeg("NYC", "LON", "UA", "C", true);
    MockAirSeg lonFraSeg("LON", "FRA", "LH", "Y", false);
    MockAirSeg lonSinSeg("LON", "SIN", "LH", "Y", false);

    MockFareMarket wasFraMarket("WAS", "FRA", "UA");
    wasFraMarket.travelSeg().push_back(&wasNycSeg);
    wasFraMarket.travelSeg().push_back(&nycLonSeg);
    wasFraMarket.travelSeg().push_back(&lonFraSeg);

    MockFareMarket wasSinMarket("WAS", "SIN", "UA");
    wasSinMarket.travelSeg().push_back(&wasNycSeg);
    wasSinMarket.travelSeg().push_back(&nycLonSeg);
    wasSinMarket.travelSeg().push_back(&lonSinSeg);

    MockPaxTypeFare prevFare(wasFraMarket, 'N', "C");
    MockPaxTypeFare replFare(wasSinMarket, 'N', "Y", 2200.0);

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasSinMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasSinValidator(trx, wasSinMarket, replItin);

    CPPUNIT_ASSERT(wasSinValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::SKIPPED,
                         wasSinValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(wasSinValidator.validateCat33(replFare));
  }

  void testAtpcoExample6()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "UA", "F", true);
    MockAirSeg nycLonSeg("NYC", "LON", "UA", "C", true);
    MockAirSeg lonFraSeg("LON", "FRA", "LH", "C", false);
    MockAirSeg lonSinSeg("LON", "SIN", "LH", "Y", false);

    MockFareMarket wasFraMarket("WAS", "FRA", "UA");
    wasFraMarket.travelSeg().push_back(&wasNycSeg);
    wasFraMarket.travelSeg().push_back(&nycLonSeg);
    wasFraMarket.travelSeg().push_back(&lonFraSeg);

    MockFareMarket wasSinMarket("WAS", "SIN", "UA");
    wasSinMarket.travelSeg().push_back(&wasNycSeg);
    wasSinMarket.travelSeg().push_back(&nycLonSeg);
    wasSinMarket.travelSeg().push_back(&lonSinSeg);

    MockPaxTypeFare prevFare(wasFraMarket, 'S', "C");
    MockPaxTypeFare replFare(wasSinMarket, 'N', "Y", 2200.0);

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasSinMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasSinValidator(trx, wasSinMarket, replItin);

    CPPUNIT_ASSERT(wasSinValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::SKIPPED,
                         wasSinValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(!wasSinValidator.validateCat33(replFare));
  }

  void testAtpcoExample7()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "UA", "C", true);
    MockAirSeg nycLonSegPrev("NYC", "LON", "UA", "Y", false);
    MockAirSeg lonFraSegPrev("LON", "FRA", "LH", "Y", false);
    MockAirSeg nycLonSegRepl("NYC", "LON", "UA", "M", false);
    MockAirSeg lonParSegRepl("LON", "PAR", "LH", "M", false);

    MockFareMarket wasFraMarket("WAS", "FRA", "UA");
    wasFraMarket.travelSeg().push_back(&wasNycSeg);
    wasFraMarket.travelSeg().push_back(&nycLonSegPrev);
    wasFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasParMarket("WAS", "PAR", "UA");
    wasParMarket.travelSeg().push_back(&wasNycSeg);
    wasParMarket.travelSeg().push_back(&nycLonSegRepl);
    wasParMarket.travelSeg().push_back(&lonParSegRepl);

    MockPaxTypeFare prevFare(wasFraMarket, 'S', "Y");
    MockPaxTypeFare replFare(wasParMarket, 'S', "M", 333.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 7475.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 4800.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 1737.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 790.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 500.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 487.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 400.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 360.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 333.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 320.0, "W"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 300.0, "Z"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 279.0, "Z"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasParMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasParMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasParValidator(trx, wasParMarket, replItin);

    CPPUNIT_ASSERT(wasParValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::FAILED,
                         wasParValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(!wasParValidator.validateCat33(replFare));
  }

  void testAtpcoExample8()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "UA", "Q", true);
    MockAirSeg nycLonSegPrev("NYC", "LON", "UA", "Q", false);
    MockAirSeg lonFraSegPrev("LON", "FRA", "LH", "Q", false);
    MockAirSeg nycLonSegRepl("NYC", "LON", "UA", "M", false);
    MockAirSeg lonParSegRepl("LON", "PAR", "LH", "M", false);

    MockFareMarket wasFraMarket("WAS", "FRA", "UA");
    wasFraMarket.travelSeg().push_back(&wasNycSeg);
    wasFraMarket.travelSeg().push_back(&nycLonSegPrev);
    wasFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasParMarket("WAS", "PAR", "UA");
    wasParMarket.travelSeg().push_back(&wasNycSeg);
    wasParMarket.travelSeg().push_back(&nycLonSegRepl);
    wasParMarket.travelSeg().push_back(&lonParSegRepl);

    MockPaxTypeFare prevFare(wasFraMarket, 'S', "Q");
    MockPaxTypeFare replFare(wasParMarket, 'S', "M", 333.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 7475.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 4800.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 1737.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 790.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 500.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 487.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 400.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 360.0, "K"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 333.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 320.0, "W"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 300.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(wasParMarket, 279.0, "Z"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasParMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasParMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasParValidator(trx, wasParMarket, replItin);

    CPPUNIT_ASSERT(wasParValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         wasParValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(wasParValidator.validateCat33(replFare));
  }

  void testAtpcoExample9()
  {
    CabinType premiumEconomyClass;
    premiumEconomyClass.setPremiumEconomyClass();

    // force Premium Economy Class for Q booking code to match the original ATPCO example
    MockAirSeg wasNycSeg("WAS", "NYC", "AA", "Q", true, premiumEconomyClass);
    MockAirSeg nycLonSeg("NYC", "LON", "BA", "M", true);
    MockAirSeg lonFraSegPrev("LON", "FRA", "BA", "M", false);
    MockAirSeg lonSinSegRepl("LON", "SIN", "BA", "B", false);

    MockFareMarket wasFraMarket("WAS", "FRA", "BA");
    wasFraMarket.travelSeg().push_back(&wasNycSeg);
    wasFraMarket.travelSeg().push_back(&nycLonSeg);
    wasFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasSinMarket("WAS", "SIN", "BA");
    wasSinMarket.travelSeg().push_back(&wasNycSeg);
    wasSinMarket.travelSeg().push_back(&nycLonSeg);
    wasSinMarket.travelSeg().push_back(&lonSinSegRepl);

    MockPaxTypeFare prevFare(wasFraMarket, 'S', "M");
    MockPaxTypeFare replFare(wasSinMarket, 'S', "B", 762.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 9215.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 7216.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 2300.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 900.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 762.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 723.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 700.0, "V"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 650.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 647.0, "V"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 630.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 599.0, "Z"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasSinMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasSinMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasSinValidator(trx, wasSinMarket, replItin);

    CPPUNIT_ASSERT(wasSinValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2,
                         wasSinValidator.validateCat31(replFare));

    FareUsage replFareUsage;
    replFareUsage.paxTypeFare() = &replFare;
    replFareUsage.travelSeg() = replFare.fareMarket()->travelSeg();
    PricingUnit replPricingUnit;
    replPricingUnit.fareUsage().push_back(&replFareUsage);
    FarePath replFarePath;
    replFarePath.pricingUnit().push_back(&replPricingUnit);

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::FAILED,
                         wasSinValidator.validateCat31(replFare, &replFarePath));

    CPPUNIT_ASSERT(wasSinValidator.validateCat33(replFare));
  }

  void testAtpcoExample10()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "AA", "V", true);
    MockAirSeg nycLonSeg("NYC", "LON", "BA", "Q", true);
    MockAirSeg lonFraSegPrev("LON", "FRA", "BA", "Q", false);
    MockAirSeg lonSinSegRepl("LON", "SIN", "BA", "M", false);

    MockFareMarket wasFraMarket("WAS", "FRA", "BA");
    wasFraMarket.travelSeg().push_back(&wasNycSeg);
    wasFraMarket.travelSeg().push_back(&nycLonSeg);
    wasFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasSinMarket("WAS", "SIN", "BA");
    wasSinMarket.travelSeg().push_back(&wasNycSeg);
    wasSinMarket.travelSeg().push_back(&nycLonSeg);
    wasSinMarket.travelSeg().push_back(&lonSinSegRepl);

    MockPaxTypeFare prevFare(wasFraMarket, 'S', "Q");
    MockPaxTypeFare replFare(wasSinMarket, 'S', "M", 650.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 9215.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 7216.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 2300.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 900.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 762.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 723.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 700.0, "V"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 650.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 647.0, "V"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 630.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(wasSinMarket, 599.0, "Z"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasSinMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasSinMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasSinValidator(trx, wasSinMarket, replItin);

    CPPUNIT_ASSERT(wasSinValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2,
                         wasSinValidator.validateCat31(replFare));

    FareUsage replFareUsage;
    replFareUsage.paxTypeFare() = &replFare;
    replFareUsage.travelSeg() = replFare.fareMarket()->travelSeg();
    PricingUnit replPricingUnit;
    replPricingUnit.fareUsage().push_back(&replFareUsage);
    FarePath replFarePath;
    replFarePath.pricingUnit().push_back(&replPricingUnit);

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         wasSinValidator.validateCat31(replFare, &replFarePath));

    CPPUNIT_ASSERT(wasSinValidator.validateCat33(replFare));
  }

  void testAtpcoExample11()
  {
    MockAirSeg bomParSeg("BOM", "PAR", "AI", "Y", true);
    MockAirSeg parLonSeg("PAR", "LON", "BA", "C", true);
    MockAirSeg lonManSegPrev("LON", "MAN", "BA", "C", false);
    MockAirSeg lonManSegRepl("LON", "MAN", "BA", "Y", false);

    MockFareMarket bomParMarket("BOM", "PAR", "AI");
    bomParMarket.travelSeg().push_back(&bomParSeg);

    MockFareMarket parManMarket("PAR", "MAN", "BA");
    parManMarket.travelSeg().push_back(&parLonSeg);
    parManMarket.travelSeg().push_back(&lonManSegPrev);

    MockFareMarket bomManMarket("BOM", "MAN", "AI");
    bomManMarket.travelSeg().push_back(&bomParSeg);
    bomManMarket.travelSeg().push_back(&parLonSeg);
    bomManMarket.travelSeg().push_back(&lonManSegRepl);

    MockPaxTypeFare prevFare1(bomParMarket, 'N', "Y");
    MockPaxTypeFare prevFare2(parManMarket, 'N', "C");
    MockPaxTypeFare replFare(bomManMarket, 'N', "Y", 3400.0);

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = bomManMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator bomManValidator(trx, bomManMarket, replItin);

    CPPUNIT_ASSERT(bomManValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::SKIPPED,
                         bomManValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(bomManValidator.validateCat33(replFare));
  }

  void testAtpcoExample12()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "AA", "M", true);
    MockAirSeg nycLonSeg("NYC", "LON", "AA", "M", true);
    MockAirSeg lonFraSegPrev("LON", "FRA", "BA", "Y", false);
    MockAirSeg lonSinSegRepl("LON", "SIN", "BA", "Y", false);

    MockFareMarket wasLonMarket("WAS", "LON", "AA");
    wasLonMarket.travelSeg().push_back(&wasNycSeg);
    wasLonMarket.travelSeg().push_back(&nycLonSeg);

    MockFareMarket lonFraMarket("LON", "FRA", "BA");
    lonFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasSinMarket("WAS", "SIN", "BA");
    wasSinMarket.travelSeg().push_back(&wasNycSeg);
    wasSinMarket.travelSeg().push_back(&nycLonSeg);
    wasSinMarket.travelSeg().push_back(&lonSinSegRepl);

    MockPaxTypeFare prevFare1(wasLonMarket, 'S', "M");
    MockPaxTypeFare prevFare2(lonFraMarket, 'N', "Y");
    MockPaxTypeFare replFare(wasSinMarket, 'N', "Y", 2200.0);

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasSinMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasSinValidator(trx, wasSinMarket, replItin);

    CPPUNIT_ASSERT(wasSinValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         wasSinValidator.validateCat31(replFare));

    CPPUNIT_ASSERT(!wasSinValidator.validateCat33(replFare));
  }

  void testAtpcoExample13()
  {
    MockAirSeg wasNycSeg("WAS", "NYC", "UA", "V", true);
    MockAirSeg nycLonSeg("NYC", "LON", "LH", "Q", true);
    MockAirSeg lonFraSegPrev("LON", "FRA", "LH", "Q", false);
    MockAirSeg lonMucSegRepl("LON", "MUC", "LH", "H", false);

    MockFareMarket wasNycMarket("WAS", "NYC", "UA");
    wasNycMarket.travelSeg().push_back(&wasNycSeg);

    MockFareMarket nycFraMarket("NYC", "FRA", "LH");
    nycFraMarket.travelSeg().push_back(&nycLonSeg);
    nycFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasMucMarket("WAS", "MUC", "LH");
    wasMucMarket.travelSeg().push_back(&wasNycSeg);
    wasMucMarket.travelSeg().push_back(&nycLonSeg);
    wasMucMarket.travelSeg().push_back(&lonMucSegRepl);

    MockPaxTypeFare prevFare1(wasNycMarket, 'S', "V");
    MockPaxTypeFare prevFare2(nycFraMarket, 'S', "Q");
    MockPaxTypeFare replFare(wasMucMarket, 'S', "H", 330.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 9215.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 7216.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 2300.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 762.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 415.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 360.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 330.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 315.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 300.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 297.0, "Q"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasMucMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasMucMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasMucValidator(trx, wasMucMarket, replItin);

    CPPUNIT_ASSERT(wasMucValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2,
                         wasMucValidator.validateCat31(replFare));

    FareUsage replFareUsage;
    replFareUsage.paxTypeFare() = &replFare;
    replFareUsage.travelSeg() = replFare.fareMarket()->travelSeg();
    PricingUnit replPricingUnit;
    replPricingUnit.fareUsage().push_back(&replFareUsage);
    FarePath replFarePath;
    replFarePath.pricingUnit().push_back(&replPricingUnit);

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         wasMucValidator.validateCat31(replFare, &replFarePath));

    CPPUNIT_ASSERT(wasMucValidator.validateCat33(replFare));
  }

  void testAtpcoExample14()
  {
    MockAirSeg ricWasSeg("RIC", "WAS", "UA", "V", true);
    MockAirSeg wasNycSeg("WAS", "NYC", "UA", "W", true);
    MockAirSeg nycLonSeg("NYC", "LON", "UA", "W", true);
    MockAirSeg lonFraSegPrev("LON", "FRA", "LH", "Q", false);
    MockAirSeg lonMucSegRepl("LON", "MUC", "UA", "B", false);

    MockFareMarket ricWasMarket("RIC", "WAS", "UA");
    ricWasMarket.travelSeg().push_back(&ricWasSeg);

    MockFareMarket wasLonMarket("NYC", "FRA", "UA");
    wasLonMarket.travelSeg().push_back(&nycLonSeg);
    wasLonMarket.travelSeg().push_back(&wasNycSeg);

    MockFareMarket lonFraMarket("LON", "FRA", "LH");
    lonFraMarket.travelSeg().push_back(&lonFraSegPrev);

    MockFareMarket wasMucMarket("WAS", "MUC", "UA");
    wasMucMarket.travelSeg().push_back(&ricWasSeg);
    wasMucMarket.travelSeg().push_back(&wasNycSeg);
    wasMucMarket.travelSeg().push_back(&nycLonSeg);
    wasMucMarket.travelSeg().push_back(&lonMucSegRepl);

    MockPaxTypeFare prevFare1(ricWasMarket, 'S', "V");
    MockPaxTypeFare prevFare2(wasLonMarket, 'S', "W");
    MockPaxTypeFare prevFare3(lonFraMarket, 'S', "Q");
    MockPaxTypeFare replFare(wasMucMarket, 'S', "B", 762.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 9215.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 7216.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 2300.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 1100.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 900.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 762.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 415.0, "V"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 360.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 330.0, "V"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 315.0, "W"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 300.0, "S"));
    otherFares.insert(new MockPaxTypeFare(wasMucMarket, 297.0, "Q"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasMucMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);
    prevItin.addPaxTypeFare(&prevFare3);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasMucMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasMucValidator(trx, wasMucMarket, replItin);

    CPPUNIT_ASSERT(wasMucValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2,
                         wasMucValidator.validateCat31(replFare));

    FareUsage replFareUsage;
    replFareUsage.paxTypeFare() = &replFare;
    replFareUsage.travelSeg() = replFare.fareMarket()->travelSeg();
    PricingUnit replPricingUnit;
    replPricingUnit.fareUsage().push_back(&replFareUsage);
    FarePath replFarePath;
    replFarePath.pricingUnit().push_back(&replPricingUnit);

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         wasMucValidator.validateCat31(replFare, &replFarePath));

    CPPUNIT_ASSERT(wasMucValidator.validateCat33(replFare));
  }

  void testAtpcoExample15()
  {
    MockAirSeg bkkLaxSeg("BKK", "LAX", "TG", "C", true);
    MockAirSeg laxDenSeg("LAX", "DEN", "UA", "F", true);
    MockAirSeg denChiSegPrev("DEN", "CHI", "UA", "Q", false);
    MockAirSeg denNycSegRepl("DEN", "NYC", "UA", "F", false);

    MockFareMarket bkkDenMarket("BKK", "DEN", "TG");
    bkkDenMarket.travelSeg().push_back(&bkkLaxSeg);
    bkkDenMarket.travelSeg().push_back(&laxDenSeg);

    MockFareMarket denChiMarket("DEN", "CHI", "UA");
    denChiMarket.travelSeg().push_back(&denChiSegPrev);

    MockFareMarket wasNycMarket("BKK", "NYC", "TG");
    wasNycMarket.travelSeg().push_back(&bkkLaxSeg);
    wasNycMarket.travelSeg().push_back(&laxDenSeg);
    wasNycMarket.travelSeg().push_back(&denNycSegRepl);

    MockPaxTypeFare prevFare1(bkkDenMarket, 'S', "C");
    MockPaxTypeFare prevFare2(denChiMarket, 'S', "Q");
    MockPaxTypeFare replFare(wasNycMarket, 'S', "C", 1111.0);

    boost::ptr_set<MockPaxTypeFare> otherFares;
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 219215.0, "F"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 217216.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 214300.0, "Y"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 111100.0, "C"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 80900.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 70762.0, "B"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 60415.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 60160.0, "M"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 51030.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 51000.0, "H"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 43000.0, "Q"));
    otherFares.insert(new MockPaxTypeFare(wasNycMarket, 42900.0, "Q"));

    for (PaxTypeFare& fare : otherFares)
    {
      wasNycMarket.allPaxTypeFare().push_back(&fare);
    }

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg() = wasNycMarket.travelSeg();

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator wasNycValidator(trx, wasNycMarket, replItin);

    CPPUNIT_ASSERT(wasNycValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2,
                         wasNycValidator.validateCat31(replFare));

    FareUsage replFareUsage;
    replFareUsage.paxTypeFare() = &replFare;
    replFareUsage.travelSeg() = replFare.fareMarket()->travelSeg();
    PricingUnit replPricingUnit;
    replPricingUnit.fareUsage().push_back(&replFareUsage);
    FarePath replFarePath;
    replFarePath.pricingUnit().push_back(&replPricingUnit);

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::FAILED,
                         wasNycValidator.validateCat31(replFare, &replFarePath));

    CPPUNIT_ASSERT(wasNycValidator.validateCat33(replFare));
  }

  void testAtpcoExample16()
  {
    MockAirSeg bkkLaxSeg("BKK", "LAX", "TG", "C", true);
    MockAirSeg laxDenSeg("LAX", "DEN", "UA", "F", true);
    MockAirSeg denChiSegPrev("DEN", "CHI", "UA", "Q", false);
    MockAirSeg denNycSegRepl("DEN", "NYC", "UA", "F", false);

    MockFareMarket bkkDenMarket("BKK", "DEN", "TG");
    bkkDenMarket.travelSeg().push_back(&bkkLaxSeg);
    bkkDenMarket.travelSeg().push_back(&laxDenSeg);

    MockFareMarket denChiMarket("DEN", "CHI", "UA");
    denChiMarket.travelSeg().push_back(&denChiSegPrev);

    MockFareMarket bkkLaxMarket("BKK", "LAX", "TG");
    bkkLaxMarket.travelSeg().push_back(&bkkLaxSeg);
    bkkLaxMarket.changeStatus() = FL;

    MockFareMarket laxNycMarket("LAX", "NYC", "TG");
    laxNycMarket.travelSeg().push_back(&laxDenSeg);
    laxNycMarket.travelSeg().push_back(&denNycSegRepl);

    MockPaxTypeFare prevFare1(bkkDenMarket, 'S', "C");
    MockPaxTypeFare prevFare2(denChiMarket, 'S', "Q");
    MockPaxTypeFare replFare1(bkkLaxMarket, 'S', "F", 1111.0);
    MockPaxTypeFare replFare2(laxNycMarket, 'N', "F", 1111.0);

    MockExcItin prevItin;
    prevItin.addPaxTypeFare(&prevFare1);
    prevItin.addPaxTypeFare(&prevFare2);

    Itin replItin;
    replItin.geoTravelType() = GeoTravelType::International;
    replItin.travelSeg().push_back(&bkkLaxSeg);
    replItin.travelSeg().push_back(&laxDenSeg);
    replItin.travelSeg().push_back(&denNycSegRepl);

    FareUsage replFareUsage1;
    replFareUsage1.paxTypeFare() = &replFare1;
    replFareUsage1.travelSeg() = replFare1.fareMarket()->travelSeg();
    FareUsage replFareUsage2;
    replFareUsage2.paxTypeFare() = &replFare2;
    replFareUsage2.travelSeg() = replFare2.fareMarket()->travelSeg();
    PricingUnit replPricingUnit;
    replPricingUnit.fareUsage().push_back(&replFareUsage1);
    replPricingUnit.fareUsage().push_back(&replFareUsage2);
    FarePath replFarePath;
    replFarePath.pricingUnit().push_back(&replPricingUnit);

    MockRexPricingTrx trx(prevItin);

    Cat31FareBookingCodeValidator bkkLaxValidator(trx, bkkLaxMarket, replItin);

    CPPUNIT_ASSERT(bkkLaxValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::POSTPONED_TO_PHASE2,
                         bkkLaxValidator.validateCat31(replFare1));

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         bkkLaxValidator.validateCat31(replFare1, &replFarePath));

    CPPUNIT_ASSERT(!bkkLaxValidator.validateCat33(replFare1));

    Cat31FareBookingCodeValidator laxNycValidator(trx, laxNycMarket, replItin);

    CPPUNIT_ASSERT(laxNycValidator.isActive());

    CPPUNIT_ASSERT_EQUAL(Cat31FareBookingCodeValidator::PASSED,
                         laxNycValidator.validateCat31(replFare2));

    CPPUNIT_ASSERT(!laxNycValidator.validateCat33(replFare2));
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Cat31FareBookingCodeValidatorTest);

} // namespace tse
