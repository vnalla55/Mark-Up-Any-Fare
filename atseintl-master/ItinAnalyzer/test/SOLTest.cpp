#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/ClassOfService.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DST.h"
#include "DBAccess/PaxTypeInfo.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TseServerStub.h"

using namespace boost::assign;

namespace tse
{
FALLBACKVALUE_DECL(fallbackExcludeCodeSharingFix);

namespace
{

class DiversityMock : public Diversity
{
protected:
  struct NonStopIndexStub : public NonStopIndex
  {
    void addNS(LegId legIdx, CarrierCode carrier, int origSopId, const ShoppingTrx& trx) {}
    void addNS(const ShoppingTrx& trx) {}
    bool updateBookingCodeInfo(const ShoppingTrx& trx) { return false; }
    void calcStatPutResultsTo(size_t& outMaxOnlineNSCount,
                              size_t& outMaxInterlineNSCount,
                              std::map<CarrierCode, size_t>& outMaxNSPerCarrier) const
    {
    }
  };

  // override
  NonStopIndex* createNonStopIndex(const ShoppingTrx& trx)
  {
    return &trx.dataHandle().safe_create<NonStopIndexStub>();
  }
};

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  MultiAirportCity* getMT(LocCode city, LocCode airport)
  {
    MultiAirportCity* ret = _memHandle.create<MultiAirportCity>();
    ret->airportCode() = airport;
    ret->city() = city;
    return ret;
  }

public:
  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date)
  {
    if (carrier == "**" && classOfService == "Y")
    {
      Cabin* ret = _memHandle.create<Cabin>();
      ret->cabin().setEconomyClass();
      return ret;
    }
    return DataHandleMock::getCabin(carrier, classOfService, date);
  }
  const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
  {
    std::vector<MultiAirportCity*>* ret = _memHandle.create<std::vector<MultiAirportCity*> >();
    if (city == "CCS")
    {
      return *ret;
    }
    else if (city == "MIA")
    {
      ret->push_back(getMT(city, "MIA"));
      ret->push_back(getMT(city, "MPB"));
      return *ret;
    }
    else if (city == "SCL")
    {
      ret->push_back(getMT(city, "SCL"));
      ret->push_back(getMT(city, "ULC"));
      return *ret;
    }
    else if (city == "SDQ")
    {
      ret->push_back(getMT(city, "SDQ"));
      ret->push_back(getMT(city, "HEX"));
      return *ret;
    }
    else if (city == "LAX")
    {
      ret->push_back(getMT(city, "LAX"));
      return *ret;
    }
    else if (city == "JFK")
    {
      ret->push_back(getMT(city, "JFK"));
      return *ret;
    }
    else if (city == "DFW")
    {
      ret->push_back(getMT(city, "DFW"));
      return *ret;
    }
    return DataHandleMock::getMultiAirportCity(city);
  }

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "CCS")
      return "CCS";
    else if (locCode == "SCL")
      return "SCL";
    else if (locCode == "SDQ")
      return "SDQ";
    else if (locCode == "DFW")
      return "DFW";
    else if (locCode == "JFK")
      return "JFK";
    else if (locCode == "LAX")
      return "LAX";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }

  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
  {
    CarrierPreference* cxrPref(0);
    _memHandle.get(cxrPref);
    if ("AA" == carrier || "DL" == carrier)
    {
      cxrPref->flowMktJourneyType() = 'N';
      cxrPref->localMktJourneyType() = 'Y';
    }
    else if ("LH" == carrier)
    {
      cxrPref->flowMktJourneyType() = 'Y';
      cxrPref->localMktJourneyType() = 'N';
    }
    else
    {
      cxrPref->flowMktJourneyType() = 'N';
      cxrPref->localMktJourneyType() = 'N';
    }
    return cxrPref;
  }

  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    std::vector<FareCalcConfig*>* ptr(0);
    _memHandle.get(ptr);
    return *ptr;
  }

  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    if (origin == "SCL" && dest == "CCS")
    {
      ret->mileage() = 3050;
      return ret;
    }
    else if (origin == "SCL" && dest == "MIA")
    {
      ret->mileage() = 4138;
      return ret;
    }
    else if ((origin == "JFK" && dest == "DFW") || (origin == "DFW" && dest == "JFK"))
    {
      ret->mileage() = 2138;
      return ret;
    }
    else if ((origin == "DFW" && dest == "LAX") || (origin == "LAX" && dest == "DFW"))
    {
      ret->mileage() = 3138;
      return ret;
    }
    else if ((origin == "JFK" && dest == "LAX") || (origin == "LAX" && dest == "JFK"))
    {
      ret->mileage() = 4138;
      return ret;
    }
    else if (origin == "SCL" && dest == "SDQ")
    {
      return 0;
    }
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};

} // anon ns

class SOLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SOLTest);
  CPPUNIT_TEST(testCodeShareInternational);
  CPPUNIT_TEST(testCodeShareDomestic);
  CPPUNIT_TEST(testCodeShareInternationalThreeSegments);
  CPPUNIT_TEST(testCodeShareDomesticThreeSegments);
  CPPUNIT_TEST(testSOLDisabledInRequest);
  CPPUNIT_TEST(testSOLEnabledInRequest);
  CPPUNIT_TEST(testSuppressLocalFM);
  CPPUNIT_TEST(testMultiDestination);
  CPPUNIT_TEST(testNumberPaxTypes);
  CPPUNIT_TEST(testAssignAvailability);
  CPPUNIT_TEST(testScheduleMarketData);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _diversity = _memHandle.create<DiversityMock>();
  }

  void tearDown() { _memHandle.clear(); }

  void setupTrx(ShoppingTrx& trx)
  {
    PricingOptions* option(0);
    trx.dataHandle().get(option);
    option->jpsEntered() = ' ';
    trx.setOptions(option);
    Agent* agent(0);
    trx.dataHandle().get(agent);
    PricingRequest* request(0);
    trx.dataHandle().get(request);
    request->ticketingAgent() = agent;
    request->lowFareRequested() = YES;
    agent->agentFunctions() = "YFH";
    agent->agentCity() = "HDQ";
    agent->agentDuty() = "8";
    agent->airlineDept() = "HDQ";
    agent->cxrCode() = "1S";
    agent->currencyCodeAgent() = "USD";
    Loc* loc(0);
    trx.dataHandle().get(loc);
    agent->agentLocation() = loc;
    trx.setRequest(request);

    Billing* billing(0);
    trx.dataHandle().get(billing);
    billing->userPseudoCityCode() = "HDQ";
    billing->userStation() = "925";
    billing->userBranch() = "3470";
    billing->partitionID() = "AA";
    billing->userSetAddress() = "02BD09";
    billing->aaaCity() = "HDQ";
    billing->aaaSine() = "YFH";
    billing->serviceName() = "ITPRICE1";
    billing->actionCode() = "WPBET";
    trx.billing() = billing;

    addPaxType(trx, "ADT", 1);

    std::vector<std::pair<uint16_t, uint16_t> > todRanges;
    todRanges.push_back(std::make_pair(0, 359));
    todRanges.push_back(std::make_pair(360, 719));
    todRanges.push_back(std::make_pair(720, 1079));
    todRanges.push_back(std::make_pair(1080, 1439));
    trx.diversity().setTODRanges(todRanges);
  }

  void testSOLDisabledInRequest()
  {
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setup1(trx);
    trx.setSimpleTrip(true);
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(!isSumOfLocalsEnabled);
  }

  void testSOLEnabledInRequest()
  {
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setup1(trx);
    trx.diversity().setEnabled();
    trx.setSimpleTrip(true);
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(isSumOfLocalsEnabled);
  }

  void testSuppressLocalFM()
  {
    TseServerStub server;
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setup1(trx);
    trx.diversity().setEnabled();
    trx.getRequest()->setSuppressSumOfLocals(true);
    trx.setSimpleTrip(true);
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(isSumOfLocalsEnabled);
    itinAnalyzer.process(trx);
    CPPUNIT_ASSERT(1 == trx.fareMarket().size());
  }

  void testCodeShareInternational()
  {
    TseServerStub server;
    TestConfigInitializer::setValue("EXCLUDE_CODE_SHARE_SOP", "Y", "SHOPPING_DIVERSITY", true);
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setupCodeShare(trx, true);
    trx.setSimpleTrip(true);
    trx.diversity().setEnabled();
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(isSumOfLocalsEnabled);
    itinAnalyzer.process(trx);
    CPPUNIT_ASSERT_EQUAL(size_t(3), trx.fareMarket().size());
  }

  void testCodeShareDomestic()
  {
    TseServerStub server;
    TestConfigInitializer::setValue("EXCLUDE_CODE_SHARE_SOP", "Y", "SHOPPING_DIVERSITY", true);
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setupCodeShare(trx, false);
    trx.setSimpleTrip(true);
    trx.diversity().setEnabled();
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(isSumOfLocalsEnabled);
    itinAnalyzer.process(trx);
    CPPUNIT_ASSERT_EQUAL(size_t(6), trx.fareMarket().size());
  }

  void testfallbackExcludeCodeSharingFix(bool flag, int count)
  {
    fallback::value::fallbackExcludeCodeSharingFix.set(flag);
    TseServerStub server;
    TestConfigInitializer::setValue("EXCLUDE_CODE_SHARE_SOP", "Y", "SHOPPING_DIVERSITY", true);
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setupCodeShareThreeSegments(trx, true);
    trx.setSimpleTrip(true);
    trx.diversity().setEnabled();
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(isSumOfLocalsEnabled);
    itinAnalyzer.process(trx);
    CPPUNIT_ASSERT_EQUAL(size_t(count), trx.fareMarket().size());
  }

  void testCodeShareInternationalThreeSegments()
  {
    testfallbackExcludeCodeSharingFix(false, 5);
    testfallbackExcludeCodeSharingFix(true, 3);
  }

  void testCodeShareDomesticThreeSegments()
  {
    TseServerStub server;
    TestConfigInitializer::setValue("EXCLUDE_CODE_SHARE_SOP", "Y", "SHOPPING_DIVERSITY", true);
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setupCodeShareThreeSegments(trx, false);
    trx.setSimpleTrip(true);
    trx.diversity().setEnabled();
    bool isSumOfLocalsEnabled(trx.isSumOfLocalsProcessingEnabled());
    CPPUNIT_ASSERT(isSumOfLocalsEnabled);
    itinAnalyzer.process(trx);
    CPPUNIT_ASSERT_EQUAL(size_t(10), trx.fareMarket().size());
  }

  void addPaxType(ShoppingTrx& trx, const PaxTypeCode& type, int number)
  {
    PaxType* paxType(0);
    trx.dataHandle().get(paxType);
    paxType->paxType() = type;
    paxType->number() = number;
    PaxTypeInfo* paxTypeInfo(0);
    trx.dataHandle().get(paxTypeInfo);
    paxTypeInfo->numberSeatsReq() = number;
    paxType->paxTypeInfo() = paxTypeInfo;
    trx.paxType().push_back(paxType);
  }

  void setup1(ShoppingTrx& trx)
  {
    AirSeg* airSeg1(0);
    trx.dataHandle().get(airSeg1);
    airSeg1->origAirport() = "JFK";
    airSeg1->departureDT() = DateTime(2011, 7, 20);
    const Loc* origin1(trx.dataHandle().getLoc(airSeg1->origAirport(), airSeg1->departureDT()));
    airSeg1->origin() = origin1;
    airSeg1->destAirport() = "DFW";
    airSeg1->arrivalDT() = DateTime(2011, 7, 20);
    const Loc* destination1(trx.dataHandle().getLoc(airSeg1->destAirport(), airSeg1->arrivalDT()));
    airSeg1->destination() = destination1;
    airSeg1->carrier() = "LH";
    airSeg1->setOperatingCarrierCode("LH");
    airSeg1->boardMultiCity() = origin1->loc();
    airSeg1->offMultiCity() = destination1->loc();

    AirSeg* airSeg2(0);
    trx.dataHandle().get(airSeg2);
    airSeg2->origAirport() = "DFW";
    airSeg2->departureDT() = DateTime(2011, 7, 21);
    const Loc* origin2(trx.dataHandle().getLoc(airSeg2->origAirport(), airSeg2->departureDT()));
    airSeg2->origin() = origin2;
    airSeg2->destAirport() = "LAX";
    airSeg2->arrivalDT() = DateTime(2011, 7, 21);
    const Loc* destination2(trx.dataHandle().getLoc(airSeg2->destAirport(), airSeg2->arrivalDT()));
    airSeg2->destination() = destination2;
    airSeg2->carrier() = "AA";
    airSeg2->setOperatingCarrierCode("AA");
    airSeg2->boardMultiCity() = origin2->loc();
    airSeg2->offMultiCity() = destination2->loc();
    BookingCode bc("Y");
    ClassOfService* oneEconomyClass(0);
    trx.dataHandle().get(oneEconomyClass);
    oneEconomyClass->numSeats() = 9;
    oneEconomyClass->cabin().setEconomyClass();
    oneEconomyClass->bookingCode() = bc;
    airSeg1->classOfService().push_back(oneEconomyClass);
    airSeg2->classOfService().push_back(oneEconomyClass);
    // Build itinerary
    Itin* itn(0);
    trx.dataHandle().get(itn);
    itn->travelSeg() += airSeg1, airSeg2;

    trx.itin().push_back(itn);
    size_t leg(trx.legs().size());
    trx.legs().push_back(ShoppingTrx::Leg());
    trx.schedulingOptionIndices().resize(leg + 1);
    trx.indicesToSchedulingOption().resize(leg + 1);
    trx.legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itn, 1, true));
  }

  void setup2(ShoppingTrx& trx)
  {
    setup1(trx);
    AirSeg* airSeg1(0);
    trx.dataHandle().get(airSeg1);
    airSeg1->origAirport() = "JFK";
    airSeg1->departureDT() = DateTime(2011, 7, 22);
    const Loc* origin1(trx.dataHandle().getLoc(airSeg1->origAirport(), airSeg1->departureDT()));
    airSeg1->origin() = origin1;
    airSeg1->destAirport() = "DFW";
    airSeg1->arrivalDT() = DateTime(2011, 7, 22);
    const Loc* destination1(trx.dataHandle().getLoc(airSeg1->destAirport(), airSeg1->arrivalDT()));
    airSeg1->destination() = destination1;
    airSeg1->carrier() = "DL";
    airSeg1->setOperatingCarrierCode("DL");
    airSeg1->boardMultiCity() = origin1->loc();
    airSeg1->offMultiCity() = destination1->loc();

    AirSeg* airSeg2(0);
    trx.dataHandle().get(airSeg2);
    airSeg2->origAirport() = "DFW";
    airSeg2->departureDT() = DateTime(2011, 7, 23);
    const Loc* origin2(trx.dataHandle().getLoc(airSeg2->origAirport(), airSeg2->departureDT()));
    airSeg2->origin() = origin2;
    airSeg2->destAirport() = "LAX";
    airSeg2->arrivalDT() = DateTime(2011, 7, 23);
    const Loc* destination2(trx.dataHandle().getLoc(airSeg2->destAirport(), airSeg2->arrivalDT()));
    airSeg2->destination() = destination2;
    airSeg2->carrier() = "AC";
    airSeg2->setOperatingCarrierCode("AC");
    airSeg2->boardMultiCity() = origin2->loc();
    airSeg2->offMultiCity() = destination2->loc();
    BookingCode bc("Y");
    ClassOfService* oneEconomyClass(0);
    trx.dataHandle().get(oneEconomyClass);
    oneEconomyClass->numSeats() = 9;
    oneEconomyClass->cabin().setEconomyClass();
    oneEconomyClass->bookingCode() = bc;
    airSeg1->classOfService().push_back(oneEconomyClass);
    airSeg2->classOfService().push_back(oneEconomyClass);
    // Build itinerary
    Itin* itn(0);
    trx.dataHandle().get(itn);
    itn->travelSeg() += airSeg1, airSeg2;

    trx.itin().push_back(itn);
    uint32_t leg(trx.legs().size());
    trx.legs().push_back(ShoppingTrx::Leg());
    trx.schedulingOptionIndices().resize(leg + 1);
    trx.indicesToSchedulingOption().resize(leg + 1);
    uint32_t mapto(trx.legs()[leg].sop().size());
    uint32_t sop(0);
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, 1, true));
    trx.schedulingOptionIndices()[leg][sop] = mapto;
    trx.indicesToSchedulingOption()[leg][mapto] = sop;
  }

  AirSeg* createAirSegment(ShoppingTrx& trx,
                           std::string origin,
                           std::string destination,
                           std::string carrier,
                           std::string operatingCarrier)
  {
    AirSeg* airSeg(0);
    trx.dataHandle().get(airSeg);
    airSeg->origAirport() = origin;
    airSeg->boardMultiCity() = origin;
    airSeg->departureDT() = DateTime(2011, 7, 20);
    airSeg->origin() = trx.dataHandle().getLoc(airSeg->origAirport(), airSeg->departureDT());
    airSeg->destAirport() = destination;
    airSeg->offMultiCity() = destination;
    airSeg->arrivalDT() = DateTime(2011, 7, 20);
    airSeg->destination() = trx.dataHandle().getLoc(airSeg->destAirport(), airSeg->arrivalDT());
    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(operatingCarrier);

    return airSeg;
  }

  void setupCodeShare(ShoppingTrx& trx, bool international)
  {
    AirSeg* airSeg1, *airSeg2, *airSeg3, *airSeg4;
    if (international)
    {
      airSeg1 = createAirSegment(trx, "JFK", "DFW", "LH", "LH");
      airSeg2 = createAirSegment(trx, "DFW", "FRA", "LH", "AA");
      airSeg3 = createAirSegment(trx, "JFK", "DFW", "LH", "LH");
      airSeg4 = createAirSegment(trx, "DFW", "FRA", "LH", "LH");
    }
    else
    {
      airSeg1 = createAirSegment(trx, "JFK", "DFW", "AA", "CO");
      airSeg2 = createAirSegment(trx, "DFW", "MIA", "AA", "CO");
      airSeg3 = createAirSegment(trx, "JFK", "DFW", "DL", "BA");
      airSeg4 = createAirSegment(trx, "DFW", "MIA", "DL", "BA");
    }

    BookingCode bc("Y");
    ClassOfService* oneEconomyClass(0);
    trx.dataHandle().get(oneEconomyClass);
    oneEconomyClass->numSeats() = 9;
    oneEconomyClass->cabin().setEconomyClass();
    oneEconomyClass->bookingCode() = bc;
    airSeg1->classOfService().push_back(oneEconomyClass);
    airSeg2->classOfService().push_back(oneEconomyClass);
    airSeg3->classOfService().push_back(oneEconomyClass);
    airSeg4->classOfService().push_back(oneEconomyClass);
    // Build itinerary
    Itin* itin1(0);
    trx.dataHandle().get(itin1);
    itin1->travelSeg() += airSeg1, airSeg2;

    Itin* itin2(0);
    trx.dataHandle().get(itin2);
    itin2->travelSeg() += airSeg3, airSeg4;

    trx.itin().push_back(itin1);
    trx.itin().push_back(itin2);
    size_t leg(trx.legs().size());
    trx.legs().push_back(ShoppingTrx::Leg());
    trx.legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itin1, 1, true));
    trx.legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itin2, 2, true));
    trx.schedulingOptionIndices().resize(leg + 1);
    trx.indicesToSchedulingOption().resize(leg + 1);
  }

  void setupCodeShareThreeSegments(ShoppingTrx& trx, bool international)
  {
    AirSeg* airSeg1, *airSeg2, *airSeg3, *airSeg4, *airSeg5, *airSeg6;
    if (international)
    {
      airSeg1 = createAirSegment(trx, "JFK", "DFW", "LH", "LH");
      airSeg2 = createAirSegment(trx, "DFW", "FRA", "LH", "AA");
      airSeg3 = createAirSegment(trx, "FRA", "KTW", "LH", "LO");
      airSeg4 = createAirSegment(trx, "JFK", "DFW", "LH", "LH");
      airSeg5 = createAirSegment(trx, "DFW", "FRA", "LH", "LH");
      airSeg6 = createAirSegment(trx, "FRA", "KTW", "LH", "LO");
    }
    else
    {
      airSeg1 = createAirSegment(trx, "JFK", "DFW", "AA", "CO");
      airSeg2 = createAirSegment(trx, "DFW", "LAX", "AA", "CO");
      airSeg3 = createAirSegment(trx, "LAX", "SFO", "AA", "CO");
      airSeg4 = createAirSegment(trx, "JFK", "DFW", "DL", "BA");
      airSeg5 = createAirSegment(trx, "DFW", "LAX", "DL", "BA");
      airSeg6 = createAirSegment(trx, "LAX", "SFO", "DL", "BA");
    }

    BookingCode bc("Y");
    ClassOfService* oneEconomyClass(0);
    trx.dataHandle().get(oneEconomyClass);
    oneEconomyClass->numSeats() = 9;
    oneEconomyClass->cabin().setEconomyClass();
    oneEconomyClass->bookingCode() = bc;
    airSeg1->classOfService().push_back(oneEconomyClass);
    airSeg2->classOfService().push_back(oneEconomyClass);
    airSeg3->classOfService().push_back(oneEconomyClass);
    airSeg4->classOfService().push_back(oneEconomyClass);
    airSeg5->classOfService().push_back(oneEconomyClass);
    airSeg6->classOfService().push_back(oneEconomyClass);
    // Build itinerary
    Itin* itin1(0);
    trx.dataHandle().get(itin1);
    itin1->travelSeg() += airSeg1, airSeg2, airSeg3;

    Itin* itin2(0);
    trx.dataHandle().get(itin2);
    itin2->travelSeg() += airSeg4, airSeg5, airSeg6;

    trx.itin().push_back(itin1);
    trx.itin().push_back(itin2);
    size_t leg(trx.legs().size());
    trx.legs().push_back(ShoppingTrx::Leg());
    trx.legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itin1, 1, true));
    trx.legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itin2, 2, true));
    trx.schedulingOptionIndices().resize(leg + 1);
    trx.indicesToSchedulingOption().resize(leg + 1);
  }

  void testMultiDestination()
  {
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setup1(trx);
    trx.setSimpleTrip(false);
    trx.diversity().setEnabled();
    trx.getRequest()->setSuppressSumOfLocals(false);
    CPPUNIT_ASSERT(!trx.isSumOfLocalsProcessingEnabled());
  }

  void testNumberPaxTypes()
  {
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setup1(trx);
    addPaxType(trx, "SPS", 1);
    trx.setSimpleTrip(true);
    trx.diversity().setEnabled();
    trx.getRequest()->setSuppressSumOfLocals(false);
    CPPUNIT_ASSERT(!trx.isSumOfLocalsProcessingEnabled());
  }

  void testAssignAvailability()
  {
    TseServerStub server;
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setup2(trx);
    trx.setSimpleTrip(true);
    setupAvl(trx);
    trx.diversity().setEnabled();
    trx.getRequest()->setSuppressSumOfLocals(false);
    itinAnalyzer.process(trx);
    checkAvail(trx);
  }

  void setupAvl(ShoppingTrx& trx)
  {
    int cos(0);
    for (const ShoppingTrx::Leg& leg : trx.legs())
    {
      for (const ShoppingTrx::SchedulingOption& sop : leg.sop())
      {
        const Itin* itin(sop.itin());
        const std::vector<TravelSeg*>& trvlSegs(itin->travelSeg());
        setAvail(trx, trvlSegs, cos);
      }
    }
  }

  void setAvail(ShoppingTrx& trx, const std::vector<TravelSeg*>& trvlSegs, int& cos)
  {
    typedef std::vector<ClassOfServiceList>* ClassOfServiceListVectorPtr;
    std::pair<AvailabilityMap::iterator, bool> res(
        trx.availabilityMap().insert(std::make_pair(ShoppingUtil::buildAvlKey(trvlSegs),
                                                    static_cast<ClassOfServiceListVectorPtr>(0))));
    if (res.second)
    {
      ClassOfServiceListVectorPtr& cosListVect(res.first->second);
      trx.dataHandle().get(cosListVect);
      for (size_t i = 0; i < trvlSegs.size(); ++i)
      {
        cosListVect->push_back(ClassOfServiceList());
        int sz(cos % 10 + 10);
        cosListVect->back().reserve(sz);
        for (int i = 0; i < sz; ++i)
        {
          cosListVect->back().push_back(generateClassOfService(trx, cos));
        }
      }
      if (trvlSegs.size() > 1)
      {
        std::vector<TravelSeg*> left, right;
        std::copy(trvlSegs.begin(), trvlSegs.end() - 1, std::back_inserter(left));
        setAvail(trx, left, cos);
        std::copy(trvlSegs.begin() + 1, trvlSegs.end(), std::back_inserter(right));
        setAvail(trx, right, cos);
      }
    }
  }
  ClassOfService* generateClassOfService(ShoppingTrx& trx, int& cos)
  {
    ClassOfService* classOfService(0);
    trx.dataHandle().get(classOfService);
    classOfService->numSeats() = cos % 9;
    classOfService->cabin().setEconomyClass();
    classOfService->bookingCode() = static_cast<char>('A' + cos % 26);
    ++cos;
    return classOfService;
  }

  void checkAvail(const ShoppingTrx& trx)
  {
    for (const ShoppingTrx::Leg& leg : trx.legs())
    {
      const Itin* itin(leg.sop().front().itin());
      // CPPUNIT_ASSERT(2 == itin->fareMarket().size());
      // CPPUNIT_ASSERT(3 == trx.fareMarket().size());
      const FareMarket* fm(itin->fareMarket().back());
      const ApplicableSOP* sops(fm->getApplicableSOPs());
      for (const ApplicableSOP::value_type& pr : *sops)
      {
        const SOPUsages& usages(pr.second);
        const Itin* applicableItin(0);
        for (const SOPUsage& usage : usages)
        {
          if (usage.applicable_)
          {
            applicableItin = usage.itin_;
            CPPUNIT_ASSERT(applicableItin != 0);
            int segIdx(0);
            for (TravelSeg* ts : applicableItin->travelSeg())
            {
              if (segIdx >= usage.startSegment_ && segIdx <= usage.endSegment_)
              {
                CPPUNIT_ASSERT(ts->isAir());
                AirSeg* airSeg(static_cast<AirSeg*>(ts));
                if (airSeg->flowJourneyCarrier())
                {
                  AvailabilityMap::const_iterator it(trx.availabilityMap().find(
                      ShoppingUtil::buildAvlKey(applicableItin->travelSeg())));
                  CPPUNIT_ASSERT(it != trx.availabilityMap().end());
                  const std::vector<ClassOfServiceList>* cosListVect(it->second);
                  CPPUNIT_ASSERT(applicableItin->travelSeg().size() == cosListVect->size());
                  const ClassOfServiceList& cosListMap((*cosListVect)[segIdx]);
                  const std::vector<ClassOfServiceList*>& usageCosListVect(usage.cos_);
                  CPPUNIT_ASSERT(usage.endSegment_ - usage.startSegment_ + 1 ==
                                 static_cast<int>(usageCosListVect.size()));
                  const ClassOfServiceList* usageCosList(
                      usageCosListVect[segIdx - usage.startSegment_]);
                  CPPUNIT_ASSERT(cosListMap.size() == usageCosList->size());
                  for (size_t i = 0; i < cosListMap.size(); ++i)
                  {
                    CPPUNIT_ASSERT(cosListMap[i]->bookingCode() ==
                                   (*usageCosList)[i]->bookingCode());
                    CPPUNIT_ASSERT(cosListMap[i]->numSeats() == (*usageCosList)[i]->numSeats());
                  }
                }
                else if (airSeg->localJourneyCarrier())
                {
                  CPPUNIT_ASSERT(1 == airSeg->classOfService().size());
                  CPPUNIT_ASSERT("Y" == airSeg->classOfService().front()->bookingCode());
                  CPPUNIT_ASSERT(9 == airSeg->classOfService().front()->numSeats());
                }
                else
                {
                  std::vector<TravelSeg*> key;
                  for (int i = usage.startSegment_; i <= usage.endSegment_; ++i)
                  {
                    key.push_back(applicableItin->travelSeg()[i]);
                  }
                  AvailabilityMap::const_iterator it(
                      trx.availabilityMap().find(ShoppingUtil::buildAvlKey(key)));
                  CPPUNIT_ASSERT(it != trx.availabilityMap().end());
                  const std::vector<ClassOfServiceList>* cosListVect(it->second);
                  CPPUNIT_ASSERT(usage.endSegment_ - usage.startSegment_ + 1 ==
                                 static_cast<int>(cosListVect->size()));
                  const ClassOfServiceList& cosListMap(
                      (*cosListVect)[segIdx - usage.startSegment_]);
                  const std::vector<ClassOfServiceList*>& usageCosListVect(usage.cos_);
                  const ClassOfServiceList* usageCosList(
                      usageCosListVect[segIdx - usage.startSegment_]);
                  CPPUNIT_ASSERT(cosListMap.size() == usageCosList->size());
                  for (size_t i = 0; i < cosListMap.size(); ++i)
                  {
                    CPPUNIT_ASSERT(cosListMap[i]->bookingCode() ==
                                   (*usageCosList)[i]->bookingCode());
                    CPPUNIT_ASSERT(cosListMap[i]->numSeats() == (*usageCosList)[i]->numSeats());
                  }
                }
              }
              ++segIdx;
            }
          }
        }
        CPPUNIT_ASSERT(applicableItin != 0);
      }
    }
  }

  void setupSMD(ShoppingTrx& trx)
  {
    trx.legs().push_back(ShoppingTrx::Leg());
    // Setup SOP1 AA JFK-LAX
    AirSeg* airSeg1(0);
    trx.dataHandle().get(airSeg1);
    airSeg1->origAirport() = "JFK";
    airSeg1->departureDT() = DateTime(2011, 7, 20, 1, 0, 0);
    const Loc* origin1(trx.dataHandle().getLoc(airSeg1->origAirport(), airSeg1->departureDT()));
    airSeg1->origin() = origin1;
    airSeg1->destAirport() = "LAX";
    airSeg1->arrivalDT() = DateTime(2011, 7, 20, 8, 0, 0);
    const Loc* destination1(trx.dataHandle().getLoc(airSeg1->destAirport(), airSeg1->arrivalDT()));
    airSeg1->destination() = destination1;
    airSeg1->carrier() = "AA";
    airSeg1->setOperatingCarrierCode("AA");
    airSeg1->boardMultiCity() = origin1->loc();
    airSeg1->offMultiCity() = destination1->loc();
    Itin* itn1(0);
    trx.dataHandle().get(itn1);
    itn1->travelSeg() += airSeg1;
    trx.itin().push_back(itn1);
    trx.legs().front().sop().push_back(ShoppingTrx::SchedulingOption(itn1, 1));
    // Setup SOP2 AA JFK-DFW AA DWF-LAX
    AirSeg* airSeg2(0);
    trx.dataHandle().get(airSeg2);
    airSeg2->origAirport() = "JFK";
    airSeg2->departureDT() = DateTime(2011, 7, 20, 10, 0, 0);
    const Loc* origin2(trx.dataHandle().getLoc(airSeg2->origAirport(), airSeg2->departureDT()));
    airSeg2->origin() = origin2;
    airSeg2->destAirport() = "DFW";
    airSeg2->arrivalDT() = DateTime(2011, 7, 20, 16, 0, 0);
    const Loc* destination2(trx.dataHandle().getLoc(airSeg2->destAirport(), airSeg2->arrivalDT()));
    airSeg2->destination() = destination2;
    airSeg2->carrier() = "AA";
    airSeg2->setOperatingCarrierCode("AA");
    airSeg2->boardMultiCity() = origin2->loc();
    airSeg2->offMultiCity() = destination2->loc();
    AirSeg* airSeg3(0);
    trx.dataHandle().get(airSeg3);
    airSeg3->origAirport() = "DFW";
    airSeg3->departureDT() = DateTime(2011, 7, 20, 17, 0, 0);
    const Loc* origin3(trx.dataHandle().getLoc(airSeg3->origAirport(), airSeg3->departureDT()));
    airSeg3->origin() = origin3;
    airSeg3->destAirport() = "LAX";
    airSeg3->arrivalDT() = DateTime(2011, 7, 20, 19, 0, 0);
    const Loc* destination3(trx.dataHandle().getLoc(airSeg3->destAirport(), airSeg3->arrivalDT()));
    airSeg3->destination() = destination3;
    airSeg3->carrier() = "AA";
    airSeg3->setOperatingCarrierCode("AA");
    airSeg3->boardMultiCity() = origin3->loc();
    airSeg3->offMultiCity() = destination3->loc();
    Itin* itn2(0);
    trx.dataHandle().get(itn2);
    itn2->travelSeg() += airSeg2, airSeg3;
    trx.itin().push_back(itn2);
    trx.legs().front().sop().push_back(ShoppingTrx::SchedulingOption(itn2, 2));
    // Setup SOP3 US JFK-LAX
    AirSeg* airSeg4(0);
    trx.dataHandle().get(airSeg4);
    airSeg4->origAirport() = "JFK";
    airSeg4->departureDT() = DateTime(2011, 7, 20, 13, 0, 0);
    const Loc* origin4(trx.dataHandle().getLoc(airSeg4->origAirport(), airSeg4->departureDT()));
    airSeg4->origin() = origin4;
    airSeg4->destAirport() = "LAX";
    airSeg4->arrivalDT() = DateTime(2011, 7, 20, 18, 0, 0);
    const Loc* destination4(trx.dataHandle().getLoc(airSeg4->destAirport(), airSeg4->arrivalDT()));
    airSeg4->destination() = destination4;
    airSeg4->carrier() = "US";
    airSeg4->setOperatingCarrierCode("US");
    airSeg4->boardMultiCity() = origin4->loc();
    airSeg4->offMultiCity() = destination4->loc();
    Itin* itn3(0);
    trx.dataHandle().get(itn3);
    itn3->travelSeg() += airSeg4;
    trx.itin().push_back(itn3);
    trx.legs().front().sop().push_back(ShoppingTrx::SchedulingOption(itn3, 3));
    // Setup SOP4 US JFK-DFW AA DWF-LAX
    AirSeg* airSeg5(0);
    trx.dataHandle().get(airSeg5);
    airSeg5->origAirport() = "JFK";
    airSeg5->departureDT() = DateTime(2011, 7, 20, 11, 0, 0);
    const Loc* origin5(trx.dataHandle().getLoc(airSeg5->origAirport(), airSeg5->departureDT()));
    airSeg5->origin() = origin5;
    airSeg5->destAirport() = "DFW";
    airSeg5->arrivalDT() = DateTime(2011, 7, 20, 16, 0, 0);
    const Loc* destination5(trx.dataHandle().getLoc(airSeg5->destAirport(), airSeg5->arrivalDT()));
    airSeg5->destination() = destination5;
    airSeg5->carrier() = "US";
    airSeg5->setOperatingCarrierCode("US");
    airSeg5->boardMultiCity() = origin5->loc();
    airSeg5->offMultiCity() = destination5->loc();
    Itin* itn4(0);
    trx.dataHandle().get(itn4);
    itn4->travelSeg() += airSeg5, airSeg3;
    trx.itin().push_back(itn4);
    trx.legs().front().sop().push_back(ShoppingTrx::SchedulingOption(itn4, 4));

    BookingCode bc("Y");
    ClassOfService* oneEconomyClass(0);
    trx.dataHandle().get(oneEconomyClass);
    oneEconomyClass->numSeats() = 9;
    oneEconomyClass->cabin().setEconomyClass();
    oneEconomyClass->bookingCode() = bc;
    airSeg1->classOfService().push_back(oneEconomyClass);
    airSeg2->classOfService().push_back(oneEconomyClass);
    airSeg3->classOfService().push_back(oneEconomyClass);
    airSeg4->classOfService().push_back(oneEconomyClass);
    airSeg5->classOfService().push_back(oneEconomyClass);
    trx.schedulingOptionIndices().resize(1);
  }

  void testScheduleMarketData()
  {
    TseServerStub server;
    ItinAnalyzerService itinAnalyzer("ITIN_SVC", server);
    char** dummy(0);
    itinAnalyzer.initialize(1, dummy);
    ShoppingTrx trx(_diversity);
    setupTrx(trx);
    setupSMD(trx);
    trx.setSimpleTrip(true);
    trx.diversity().setEnabled();
    trx.getRequest()->setSuppressSumOfLocals(false);
    std::vector<std::pair<uint16_t, uint16_t> > todRanges;
    todRanges.push_back(std::make_pair(0, 719));
    todRanges.push_back(std::make_pair(720, 1439));
    trx.diversity().setTODRanges(todRanges);
    itinAnalyzer.process(trx);
    const Diversity::ScheduleMarketDataStorage& smdStorage =
        trx.diversity().getScheduleMarketDataStorage();
    CPPUNIT_ASSERT_EQUAL(size_t(5), smdStorage.size());
    CPPUNIT_ASSERT_EQUAL(
        int32_t(300),
        trx.legs()[0].sop()[trx.legs()[0].getMinDurationSopIdx()].itin()->getFlightTimeMinutes());
    for (Diversity::ScheduleMarketDataStorage::const_iterator stIt = smdStorage.begin();
         stIt != smdStorage.end();
         ++stIt)
    {
      const FareMarket* fm = stIt->first;
      for (std::map<CarrierCode, Diversity::ScheduleMarketData>::const_iterator smdIt =
               stIt->second.begin();
           smdIt != stIt->second.end();
           ++smdIt)
      {
        const Diversity::ScheduleMarketData& smd = smdIt->second;
        if (smdIt->first == "AA")
        {
          // This should be AA JFK-LAX FM
          if (fm->origin()->loc() == "JFK" && fm->destination()->loc() == "LAX")
          {
            CPPUNIT_ASSERT_EQUAL(uint16_t(1), smd.nonStopSOPs);
            CPPUNIT_ASSERT_EQUAL(540, smd.maxFlightDuration);
            CPPUNIT_ASSERT_EQUAL(420, smd.minFlightDuration);
            CPPUNIT_ASSERT_EQUAL(2, smd.sopPerTOD[0]);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[1]);
          }
          // This should be AA JFK-DFW FM
          else if (fm->origin()->loc() == "JFK" && fm->destination()->loc() == "DFW")
          {
            CPPUNIT_ASSERT_EQUAL(uint16_t(0), smd.nonStopSOPs);
            CPPUNIT_ASSERT_EQUAL(540, smd.maxFlightDuration);
            CPPUNIT_ASSERT_EQUAL(540, smd.minFlightDuration);
            CPPUNIT_ASSERT_EQUAL(1, smd.sopPerTOD[0]);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[1]);
          }
          // This should be AA DFW-LAX FM
          else if (fm->origin()->loc() == "DFW" && fm->destination()->loc() == "LAX")
          {
            CPPUNIT_ASSERT_EQUAL(uint16_t(0), smd.nonStopSOPs);
            CPPUNIT_ASSERT_EQUAL(540, smd.maxFlightDuration);
            CPPUNIT_ASSERT_EQUAL(540, smd.minFlightDuration);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[0]);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[1]);
          }
        }
        else if (smdIt->first == "US")
        {
          // This should be US JFK-LAX FM
          if (fm->origin()->loc() == "JFK" && fm->destination()->loc() == "LAX")
          {
            CPPUNIT_ASSERT_EQUAL(uint16_t(1), smd.nonStopSOPs);
            CPPUNIT_ASSERT_EQUAL(300, smd.maxFlightDuration);
            CPPUNIT_ASSERT_EQUAL(300, smd.minFlightDuration);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[0]);
            CPPUNIT_ASSERT_EQUAL(1, smd.sopPerTOD[1]);
          }
          // This should be US JFK-DFW FM
          else if (fm->origin()->loc() == "JFK" && fm->destination()->loc() == "DFW")
          {
            CPPUNIT_ASSERT_EQUAL(uint16_t(0), smd.nonStopSOPs);
            CPPUNIT_ASSERT_EQUAL(480, smd.maxFlightDuration);
            CPPUNIT_ASSERT_EQUAL(480, smd.minFlightDuration);
            CPPUNIT_ASSERT_EQUAL(1, smd.sopPerTOD[0]);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[1]);
          }
          // This should be AA DFW-LAX FM
          else if (fm->origin()->loc() == "DFW" && fm->destination()->loc() == "LAX")
          {
            CPPUNIT_ASSERT_EQUAL(uint16_t(0), smd.nonStopSOPs);
            CPPUNIT_ASSERT_EQUAL(480, smd.maxFlightDuration);
            CPPUNIT_ASSERT_EQUAL(480, smd.minFlightDuration);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[0]);
            CPPUNIT_ASSERT_EQUAL(0, smd.sopPerTOD[1]);
          }
        }
      }
    }
  }

private:
  TestMemHandle _memHandle;
  Diversity* _diversity;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SOLTest);
} // tse
