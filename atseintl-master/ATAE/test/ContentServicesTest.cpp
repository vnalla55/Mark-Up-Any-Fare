#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "ATAE/ContentServices.h"
#include "ATAE/PricingDssResponseHandler.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "test/include/TestMemHandle.h"
#include "Common/ClassOfService.h"
#include "DBAccess/Cabin.h"
#include "DataModel/AvailData.h"
#include "DataModel/ReservationData.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/ExcItin.h"
#include "Common/FareMarketUtil.h"
#include "DataModel/Billing.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"

#include "Common/Config/ConfigMan.h"


using namespace boost::assign;
using namespace std;

namespace tse
{

const string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
const string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";

class ContentServicesStub : public ContentServices
{
public:
  ContentServicesStub() : ContentServices(), _cabinZero(false), _dssV2Call(0) {}

  ~ContentServicesStub() {}

  const Cabin* getCabin(DataHandle& dataHandle,
                        const CarrierCode& carrier,
                        const BookingCode& classOfService,
                        const DateTime& date)
  {
    Cabin* cab = 0;
    if (_cabinZero)
      return cab;
    cab = _memHandle.create<Cabin>();
    if (classOfService == "F" || classOfService == "P" || classOfService == "R" ||
        classOfService == "A")
    {
      cab->cabin().setFirstClass();
      return cab;
    }
    if (classOfService == "C" || classOfService == "D" || classOfService == "J")
    {
      cab->cabin().setBusinessClass();
      return cab;
    }
    cab->cabin().setEconomyClass();
    return cab;
  }

  uint16_t numSeatsNeeded(PricingTrx& trx) { return 1; }
  TestMemHandle _memHandle;
  bool _cabinZero;

  bool callAs2(PricingTrx& trx) { throw string("AS2 call"); }

  bool callDssV2(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule)
  {
    ++_dssV2Call;
    return true;
  }

  int _dssV2Call;
};

class ContentServicesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ContentServicesTest);
  CPPUNIT_TEST(testNumFlts_zero_noFlightsInFareMarket);
  CPPUNIT_TEST(testNumFlts_returnsCountSameCarriersFor2);
  CPPUNIT_TEST(testNumFlts_returnsCountSameCarriersUpTo3);
  CPPUNIT_TEST(testNumFlts_returnsCountSameCarriersStartingAtLaterIndex);
  CPPUNIT_TEST(testNumFlts_returnsCountSameConnectingCarriersFirstTwo);
  CPPUNIT_TEST(testNumFlts_returnsCountSameConnectingCarriersLastTwo);

  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseWhenFareMarketHasOnlineConnectingFlights);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnFalseWhenFareMarketHasOneFlight);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFareMarketHasNonMultiAirpotArunk);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFareMarketHasMultiAirpotArunkAsLastSeg);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFareMarketHasStopOver);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFareMarketHasOpenSeg);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFareMarketHasInterlineSeg);
  CPPUNIT_TEST(testStopOversArunkIncludedReturnTrueWhenFareMarketHasMoreThan3Segs);

  CPPUNIT_TEST(testFoundCosReturnFalseIfNoCosInFltAvail);
  CPPUNIT_TEST(testFoundCosReturnFalseIfBookingCodeDontMatch);
  CPPUNIT_TEST(testFoundCosReturnTrueIfBookingCodeMatch);

  CPPUNIT_TEST(testUpdateTravelSegAvailSetsNumSeatsToZeroWhenBookingCodeNotFound);
  CPPUNIT_TEST(testUpdateTravelSegAvailSetsNumSeatsNotChangedWhenBookingCodeFound);

  CPPUNIT_TEST(testUpdateFareMarketAvailDontChangeNumSeatsWhenBookingCodeFound);
  CPPUNIT_TEST(testUpdateFareMarketAvailZeroesNumSeatsWhenBookingCodeNotFound);
  CPPUNIT_TEST(testUpdateFareMarketAvailDontChangeNumSeatsWhenTravelSegNotFound);

  CPPUNIT_TEST(testUpdateAvailForCores);
  CPPUNIT_TEST(testAdjustAvailForCores);
  CPPUNIT_TEST(testAdjustAvailForCoresWhenAvailDataHasNoFlightAvails);

  CPPUNIT_TEST(testCheckDuplicateFareMarketsPopulateClassOfServiceVecWhenDuplicate);
  CPPUNIT_TEST(testCheckDuplicateFareMarketsDontPopulateClassOfServiceVecCosVecEmpty);

  CPPUNIT_TEST(testXmlDiagRequiredReturnFalseWhenDiagNotActive);
  CPPUNIT_TEST(testXmlDiagRequiredReturnFalseWhenNotDiag195);
  CPPUNIT_TEST(testXmlDiagRequiredReturnFalseWhenNoParametersInDiag195);
  CPPUNIT_TEST(testXmlDiagRequiredReturnFalseWhenEmptyParametersInDiag195);
  CPPUNIT_TEST(testXmlDiagRequiredReturnTrueDDATAEREQpresent);
  CPPUNIT_TEST(testXmlDiagRequiredReturnFalseWhenDDATAEREQnotPresent);

  CPPUNIT_TEST(testSendResDataToAtaeReturnFalseWhenNoResDataPresent);
  CPPUNIT_TEST(testSendResDataToAtaeReturnFalseWhenJourneyNotActivated);
  CPPUNIT_TEST(testSendResDataToAtaeReturnFalseWhenJourneyNotApplied);
  CPPUNIT_TEST(testSendResDataToAtaeReturnFalseWhenNoJourneyCxr);
  CPPUNIT_TEST(testSendResDataToAtaeReturnTrueWhenJourneyCxrPresent);

  CPPUNIT_TEST(testBuildUsingLocalDoesNotBuildCosVecInFmIfStartTvlSegNotFound);
  CPPUNIT_TEST(testBuildUsingLocalBuildCosVecInFm);
  CPPUNIT_TEST(testBuildUsingLocalBuildCosVecInFmForArunkSeg);

  CPPUNIT_TEST(testBuildMarketHandlesIfEndIteratorIsPassed);
  CPPUNIT_TEST(testBuildMarketBuildFromLocalWhenFareMarketNotFound);
  CPPUNIT_TEST(testBuildMarketPartlyBuildFromExistingFareMarket);

  CPPUNIT_TEST(testGroup3BuildCosForLastSeg);
  CPPUNIT_TEST(testGroup3BuildCosForLastArunkSeg);
  CPPUNIT_TEST(testGroup3BuildCosLastTwoFlights);
  CPPUNIT_TEST(testGroup3BuildCosLastTwoFlightsWhenFirstOfThemArunk);
  CPPUNIT_TEST(testGroup3BuildCosLastTwoFlightsWhenSecondOfThemArunk);
  CPPUNIT_TEST(testGroup3BuildCosFirstThreeFlightsWhenFirstArunk);
  CPPUNIT_TEST(testGroup3BuildCosFirstThreeFlightsWhenSecondArunk);
  CPPUNIT_TEST(testGroup3BuildCosFirstThreeFlightsWhenThirdArunk);
  CPPUNIT_TEST(testGroup3BuildCosFirstThreeFlightsWhenNoArunkAndAll3CxrDifferent);
  CPPUNIT_TEST(testGroup3BuildCosFirstThreeFlightsWhenNoArunkAndFirst2CxrSame);
  CPPUNIT_TEST(testGroup3BuildCosFirstThreeFlightsWhenNoArunkAndLast2CxrSame);

  CPPUNIT_TEST(testRestFareMarketsBuildCosForAllFareMarkets);

  CPPUNIT_TEST(testJourneyTestDoesNotChangeAvailIfNoDiagParamEntered);
  CPPUNIT_TEST(testJourneyTestDoesNotChangeAvailIfSOLOCALorSOFLOWnotEntered);
  CPPUNIT_TEST(testJourneyTestDoesNotChangeAvailIfSOFLOWEnteredButFareMarketNotFlow);
  CPPUNIT_TEST(testJourneyTestChangeAvailForFlowMarketIfSOFLOWEntered);
  CPPUNIT_TEST(testJourneyTestChangeDoesNotChangeAvailForLocalMarketIfCxrNotJourney);
  CPPUNIT_TEST(testJourneyTestChangeAvailForLocalMarket);

  CPPUNIT_TEST(testSoloTestDoesntChangeAvailIfNoDiagParamEntered);
  CPPUNIT_TEST(testSoloTestDoesntChangeAvailIfSOLOTESTNotEntered);
  CPPUNIT_TEST(testSoloTestChangeFlowAvailIfSOLOTESTentered);

  CPPUNIT_TEST(testDummyCOSReturnsCosForAllCharacters);

  CPPUNIT_TEST(testGetACosReturnAPointer);
  CPPUNIT_TEST(testGetCosFlownDoesNotBuildCosWhenNotRexTrx);
  CPPUNIT_TEST(testGetCosFlownDoesNotBuildCosWhenFirstSegArunk);
  CPPUNIT_TEST(testGetCosFlownDoesNotBuildCosWhenFirstSegUnflown);
  CPPUNIT_TEST(testGetCosFlownBuildCosWhenFirstSegFlown);

  CPPUNIT_TEST(testFillDummyCosDoesntBuildCosInSegWhenSegArunk);
  CPPUNIT_TEST(testFillDummyCosDoesntBuildCosInSegWhenSegNotOpen);
  CPPUNIT_TEST(testFillDummyCosBuildCosInSegAndFmWhenSegOpen);

  CPPUNIT_TEST(testGetSchedAndAvailCallAs2WhenIsNotRexBaseTrx);
  CPPUNIT_TEST(testGetSchedAndAvailCallAs2WhenIsNotAnalyzingExcItin);
  CPPUNIT_TEST(testGetSchedAndAvailDontCallAs2WhenIsAnalyzingExcItin);

  CPPUNIT_TEST(testCheckAndLogAs2Errors);

  CPPUNIT_TEST(testConstruct);
  CPPUNIT_TEST(testConstruct_ancillary);
  CPPUNIT_TEST(testConstruct_ancillary_ACS);
  CPPUNIT_TEST(testConstruct_ancillary_WPBG);
  CPPUNIT_TEST(testConstruct_ancillary_MISC6);
  CPPUNIT_TEST(testConstruct_ancillary_AB240);

  CPPUNIT_TEST(testEquipmentCodeNotSet);
  CPPUNIT_TEST(testEquipmentCodeSet);
  CPPUNIT_TEST(testEquipmentCodeNotSet_allItins);
  CPPUNIT_TEST(testEquipmentCodeSet_allItins);
  CPPUNIT_TEST(testHiddenStopDetails_0HiddenStops);
  CPPUNIT_TEST(testHiddenStopDetails_2HiddenStops);
  CPPUNIT_TEST(testHiddenStopDetails_HiddenStopDetails);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _fareMarket = _memHandle.create<FareMarket>();
    _continental1 = createAirSeg("CO");
    _continental2 = createAirSeg("CO");
    _continental3 = createAirSeg("CO");
    _aa1 = createAirSeg("AA");
    _aa2 = createAirSeg("AA");
    _aa3 = createAirSeg("AA");
    _aa4 = createAirSeg("AA");
    _trx->setRequest(&_request);
  }

  void tearDown()
  {
    _memHandle.clear();
    CppUnit::TestFixture::tearDown();
  }
  void testNumFlts_zero_noFlightsInFareMarket()
  {
    size_t startIndex(0);
    assertNumFlightsForStartIndex(startIndex, 0);
  }

  void testNumFlts_returnsCountSameCarriersFor2()
  {
    size_t startIndex(0);
    _fareMarket->travelSeg() += _continental1, _continental2, _aa1, _aa2;
    assertNumFlightsForStartIndex(startIndex, 2);
  }

  void testNumFlts_returnsCountSameCarriersUpTo3()
  {
    size_t startIndex(0);
    _fareMarket->travelSeg() += _continental1, _continental2, _continental3, _aa1;
    assertNumFlightsForStartIndex(startIndex, 3);
  }

  void testNumFlts_returnsCountSameCarriersStartingAtLaterIndex()
  {
    size_t startIndex(2);
    _fareMarket->travelSeg() += _continental1, _continental2, _aa1, _aa2;
    assertNumFlightsForStartIndex(startIndex, 2);
  }

  void testNumFlts_returnsCountSameConnectingCarriersFirstTwo()
  {
    prepareFourFlightsWithStopOverAtSecond();
    size_t startIndex(0);
    _fareMarket->travelSeg() += _aa1, _aa2, _aa3, _aa4;
    assertNumFlightsForStartIndex(startIndex, 2);
  }

  void testNumFlts_returnsCountSameConnectingCarriersLastTwo()
  {
    prepareFourFlightsWithStopOverAtSecond();
    size_t startIndex(2);
    _fareMarket->travelSeg() += _aa1, _aa2, _aa3, _aa4;
    assertNumFlightsForStartIndex(startIndex, 2);
  }

  void testStopOversArunkIncludedReturnFalseWhenFareMarketHasOnlineConnectingFlights()
  {
    prepareFareMarketWithOnlineConnections();
    CPPUNIT_ASSERT(!_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnFalseWhenFareMarketHasOneFlight()
  {
    prepareFareMarketWithOnlineConnections();
    _fareMarket->travelSeg().resize(1);
    CPPUNIT_ASSERT(!_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnTrueWhenFareMarketHasNonMultiAirpotArunk()
  {
    prepareFareMarketWithOnlineConnections();
    ArunkSeg arunkSeg;
    arunkSeg.boardMultiCity() = LOC_KUL;
    arunkSeg.offMultiCity() = LOC_NYC;
    _fareMarket->travelSeg().push_back(&arunkSeg);
    CPPUNIT_ASSERT(_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnTrueWhenFareMarketHasMultiAirpotArunkAsLastSeg()
  {
    prepareFareMarketWithOnlineConnections();
    ArunkSeg arunkSeg;
    arunkSeg.boardMultiCity() = LOC_EWR;
    arunkSeg.offMultiCity() = LOC_NYC;
    _fareMarket->travelSeg().push_back(&arunkSeg);
    CPPUNIT_ASSERT(_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnTrueWhenFareMarketHasStopOver()
  {
    prepareFareMarketWithOnlineConnections();
    _aa1->stopOver() = true;
    CPPUNIT_ASSERT(_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnTrueWhenFareMarketHasOpenSeg()
  {
    prepareFareMarketWithOnlineConnections();
    _aa1->segmentType() = Open;
    CPPUNIT_ASSERT(_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnTrueWhenFareMarketHasInterlineSeg()
  {
    prepareFareMarketWithOnlineConnections();
    _aa1->carrier() = "CO";
    CPPUNIT_ASSERT(_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testStopOversArunkIncludedReturnTrueWhenFareMarketHasMoreThan3Segs()
  {
    prepareFareMarketWithOnlineConnections();
    addTwoMoreSegsToFareMarket();
    CPPUNIT_ASSERT(_cs.stopOversArunkIncluded(*_fareMarket));
  }

  void testFoundCosReturnFalseIfNoCosInFltAvail()
  {
    ClassOfService cos;
    FlightAvail fltAvail;
    ClassOfService* pCos = 0;
    fltAvail.classOfService().push_back(pCos);
    CPPUNIT_ASSERT(!_cs.foundCos(cos, fltAvail));
  }

  void testFoundCosReturnFalseIfBookingCodeDontMatch()
  {
    ClassOfService cos;
    cos.bookingCode() = "Q";
    FlightAvail fltAvail;
    prepareCosVec(fltAvail.classOfService());
    CPPUNIT_ASSERT(!_cs.foundCos(cos, fltAvail));
  }

  void testFoundCosReturnTrueIfBookingCodeMatch()
  {
    ClassOfService cos;
    cos.bookingCode() = "Y";
    FlightAvail fltAvail;
    prepareCosVec(fltAvail.classOfService());
    CPPUNIT_ASSERT(_cs.foundCos(cos, fltAvail));
  }

  void testUpdateTravelSegAvailSetsNumSeatsToZeroWhenBookingCodeNotFound()
  {
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    FlightAvail fltAvail;
    prepareCosVec(fltAvail.classOfService());
    fltAvail.classOfService()[0]->bookingCode() = "Q";
    _cs.updateTravelSegAvail(cosVec, fltAvail);
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, cosVec[0]->numSeats());
  }

  void testUpdateTravelSegAvailSetsNumSeatsNotChangedWhenBookingCodeFound()
  {
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    FlightAvail fltAvail;
    prepareCosVec(fltAvail.classOfService());
    _cs.updateTravelSegAvail(cosVec, fltAvail);
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, cosVec[0]->numSeats());
  }

  void testUpdateFareMarketAvailDontChangeNumSeatsWhenBookingCodeFound()
  {
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    FlightAvail fltAvail;
    fltAvail.upSellPNRsegNum() = 1;
    _aa1->pnrSegment() = 1;
    prepareCosVec(fltAvail.classOfService());
    prepareCosVec(_aa1->classOfService());
    _fareMarket->travelSeg() += _aa1;
    _fareMarket->classOfServiceVec().push_back(&cosVec);
    _cs.updateFareMarketAvail(*_fareMarket, fltAvail);
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, _aa1->classOfService()[0]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, cosVec[0]->numSeats());
  }

  void testUpdateFareMarketAvailZeroesNumSeatsWhenBookingCodeNotFound()
  {
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    FlightAvail fltAvail;
    fltAvail.upSellPNRsegNum() = 1;
    _aa1->pnrSegment() = 1;
    prepareCosVec(fltAvail.classOfService());
    fltAvail.classOfService()[0]->bookingCode() = "Q";
    prepareCosVec(_aa1->classOfService());
    _fareMarket->travelSeg() += _aa1;
    _fareMarket->classOfServiceVec().push_back(&cosVec);
    _cs.updateFareMarketAvail(*_fareMarket, fltAvail);
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, _aa1->classOfService()[0]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, cosVec[0]->numSeats());
  }

  void testUpdateFareMarketAvailDontChangeNumSeatsWhenTravelSegNotFound()
  {
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    FlightAvail fltAvail;
    fltAvail.upSellPNRsegNum() = 0;
    _aa1->pnrSegment() = 1;
    prepareCosVec(fltAvail.classOfService());
    prepareCosVec(_aa1->classOfService());
    _fareMarket->travelSeg() += _aa1;
    _fareMarket->classOfServiceVec().push_back(&cosVec);
    _cs.updateFareMarketAvail(*_fareMarket, fltAvail);
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, _aa1->classOfService()[0]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, cosVec[0]->numSeats());
  }

  void testUpdateAvailForCores()
  {
    prepareItin();
    FlightAvail fltAvail;
    CPPUNIT_ASSERT_NO_THROW(_cs.updateAvail(*_trx, fltAvail));
  }

  void testAdjustAvailForCores()
  {
    AvailData* availData = prepareAvailData(2);
    _trx->getRequest()->availData().push_back(availData);
    prepareItin();
    CPPUNIT_ASSERT_NO_THROW(_cs.adjustAvail(*_trx));
  }

  void testAdjustAvailForCoresWhenAvailDataHasNoFlightAvails()
  {
    AvailData* availData = prepareAvailData(2);
    availData->flightAvails().clear();
    _trx->getRequest()->availData().push_back(availData);
    prepareItin();
    CPPUNIT_ASSERT_NO_THROW(_cs.adjustAvail(*_trx));
  }

  void testCheckDuplicateFareMarketsPopulateClassOfServiceVecWhenDuplicate()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg() += _aa1, _aa2;
    prepareItin();
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    _fareMarket->classOfServiceVec().push_back(&cosVec);
    _fareMarket->classOfServiceVec().push_back(&cosVec);
    _cs.checkDuplicateFareMarkets(*(_trx->itin()[0]), fm,*_trx );
    CPPUNIT_ASSERT_EQUAL(&cosVec, fm->classOfServiceVec()[0]);
  }

  void testCheckDuplicateFareMarketsDontPopulateClassOfServiceVecCosVecEmpty()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg() += _aa1, _aa2;
    prepareItin();
    _cs.checkDuplicateFareMarkets(*(_trx->itin()[0]), fm,*_trx);
    CPPUNIT_ASSERT(fm->classOfServiceVec().empty());
  }

  void testXmlDiagRequiredReturnFalseWhenDiagNotActive()
  {
    string diagParam = "ATAEREQ";
    _trx->diagnostic().deActivate();
    CPPUNIT_ASSERT(!_cs.xmlDiagRequired(*_trx, diagParam));
  }

  void testXmlDiagRequiredReturnFalseWhenNotDiag195()
  {
    string diagParam = "ATAEREQ";
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic199;
    CPPUNIT_ASSERT(!_cs.xmlDiagRequired(*_trx, diagParam));
  }

  void testXmlDiagRequiredReturnFalseWhenNoParametersInDiag195()
  {
    string diagParam = "ATAEREQ";
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    CPPUNIT_ASSERT(!_cs.xmlDiagRequired(*_trx, diagParam));
  }

  void testXmlDiagRequiredReturnFalseWhenEmptyParametersInDiag195()
  {
    string diagParam = "ATAEREQ";
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    _trx->diagnostic().diagParamMap().insert(pair<string, string>(Diagnostic::DISPLAY_DETAIL, ""));
    CPPUNIT_ASSERT(!_cs.xmlDiagRequired(*_trx, diagParam));
  }

  void testXmlDiagRequiredReturnTrueDDATAEREQpresent()
  {
    string diagParam = "ATAEREQ";
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::DISPLAY_DETAIL, "ATAEREQ"));
    CPPUNIT_ASSERT(_cs.xmlDiagRequired(*_trx, diagParam));
  }

  void testXmlDiagRequiredReturnFalseWhenDDATAEREQnotPresent()
  {
    string diagParam = "ATAEREQ";
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic195;
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::DISPLAY_DETAIL, "ASV2REQ"));
    CPPUNIT_ASSERT(!_cs.xmlDiagRequired(*_trx, diagParam));
  }

  void testSendResDataToAtaeReturnFalseWhenNoResDataPresent()
  {
    _trx->getRequest()->reservationData() = 0;
    CPPUNIT_ASSERT(!_cs.sendResDataToAtae(*_trx));
  }

  void testSendResDataToAtaeReturnFalseWhenJourneyNotActivated()
  {
    prepareForJourney();
    _trx->getOptions()->journeyActivatedForPricing() = false;
    CPPUNIT_ASSERT(!_cs.sendResDataToAtae(*_trx));
  }

  void testSendResDataToAtaeReturnFalseWhenJourneyNotApplied()
  {
    prepareForJourney();
    _trx->getOptions()->applyJourneyLogic() = false;
    CPPUNIT_ASSERT(!_cs.sendResDataToAtae(*_trx));
  }

  void testSendResDataToAtaeReturnFalseWhenNoJourneyCxr()
  {
    prepareForJourney();
    CPPUNIT_ASSERT(!_cs.sendResDataToAtae(*_trx));
  }

  void testSendResDataToAtaeReturnTrueWhenJourneyCxrPresent()
  {
    prepareForJourney();
    CarrierPreference* cxrPref = _memHandle.create<CarrierPreference>();
    _aa1->carrierPref() = cxrPref;
    cxrPref->flowMktJourneyType() = YES;
    CPPUNIT_ASSERT(_cs.sendResDataToAtae(*_trx));
  }

  void testBuildUsingLocalDoesNotBuildCosVecInFmIfStartTvlSegNotFound()
  {
    prepareItin();
    FareMarketUtil::buildUsingLocal(*_fareMarket, *_trx, 2, _aa3, true);
    CPPUNIT_ASSERT(_fareMarket->classOfServiceVec().empty());
  }

  void testBuildUsingLocalBuildCosVecInFm()
  {
    prepareItin();
    prepareCosVec(_aa1->classOfService());
    prepareCosVec(_aa2->classOfService());
    FareMarketUtil::buildUsingLocal(*_fareMarket, *_trx, 2, _aa1, true);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _fareMarket->classOfServiceVec().size());
  }

  void testBuildUsingLocalBuildCosVecInFmForArunkSeg()
  {
    prepareItin();
    prepareCosVec(_aa1->classOfService());
    prepareCosVec(_aa2->classOfService());
    ArunkSeg arunkSeg;
    _fareMarket->travelSeg() += &arunkSeg;
    FareMarketUtil::buildUsingLocal(*_fareMarket, *_trx, 3, _aa1, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, _fareMarket->classOfServiceVec().size());
  }

  void testBuildMarketHandlesIfEndIteratorIsPassed()
  {
    prepareItin();
    std::vector<TravelSeg*>::iterator endIt = _fareMarket->travelSeg().end();
    std::vector<FareMarket*> processedFM;
    CPPUNIT_ASSERT_NO_THROW(FareMarketUtil::buildMarket(
        *(_trx->itin()[0]), *_fareMarket, endIt, *_trx, 2, 0, processedFM, true));
  }

  void testBuildMarketBuildFromLocalWhenFareMarketNotFound()
  {
    prepareItin();
    FareMarket fm;
    fm.availBreaks().resize(1);
    fm.travelSeg() += _aa1;
    prepareCosVec(_aa1->classOfService());
    std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().begin();
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::buildMarket(*(_trx->itin()[0]), fm, tvlI, *_trx, 0, 1, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)1, fm.classOfServiceVec().size());
  }

  void testBuildMarketPartlyBuildFromExistingFareMarket()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    vector<ClassOfService*> cosVec;
    prepareCosVec(cosVec);
    _fareMarket->classOfServiceVec() += &cosVec, &cosVec;
    std::vector<TravelSeg*>::iterator tvlI = fm->travelSeg().begin();
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::buildMarket(*(_trx->itin()[0]), *fm, tvlI, *_trx, 0, 2, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)2, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
  }

  void testGroup3BuildCosForLastSeg()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 3, 1, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)1, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[3]);
  }

  void testGroup3BuildCosForLastArunkSeg()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    ArunkSeg arunkSeg;
    fm->travelSeg()[3] = &arunkSeg;
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 3, 1, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)1, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[3]);
  }

  void testGroup3BuildCosLastTwoFlights()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 2, 2, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)2, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[3]);
  }

  void testGroup3BuildCosLastTwoFlightsWhenFirstOfThemArunk()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    ArunkSeg arunkSeg;
    fm->travelSeg()[2] = &arunkSeg;
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 2, 2, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)2, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
    CPPUNIT_ASSERT(fm->availBreaks()[3]);
  }

  void testGroup3BuildCosLastTwoFlightsWhenSecondOfThemArunk()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    ArunkSeg arunkSeg;
    fm->travelSeg()[3] = &arunkSeg;
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 2, 2, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)2, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[3]);
  }

  void testGroup3BuildCosFirstThreeFlightsWhenFirstArunk()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    ArunkSeg arunkSeg;
    fm->travelSeg()[0] = &arunkSeg;
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 0, 3, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testGroup3BuildCosFirstThreeFlightsWhenSecondArunk()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    ArunkSeg arunkSeg;
    fm->travelSeg()[1] = &arunkSeg;
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 0, 3, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testGroup3BuildCosFirstThreeFlightsWhenThirdArunk()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    ArunkSeg arunkSeg;
    fm->travelSeg()[2] = &arunkSeg;
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 0, 3, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testGroup3BuildCosFirstThreeFlightsWhenNoArunkAndAll3CxrDifferent()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    _aa2->carrier() = "CO";
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 0, 3, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testGroup3BuildCosFirstThreeFlightsWhenNoArunkAndFirst2CxrSame()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    _aa3->carrier() = "CO";
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 0, 3, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testGroup3BuildCosFirstThreeFlightsWhenNoArunkAndLast2CxrSame()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    _aa1->carrier() = "CO";
    std::vector<FareMarket*> processedFM;
    FareMarketUtil::group3(*(_trx->itin()[0]), *fm, 0, 3, *_trx, processedFM, true);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testRestFareMarketsBuildCosForAllFareMarkets()
  {
    prepareItin();
    FareMarket* fm = prepareFareMarket();
    _trx->itin()[0]->fareMarket().push_back(fm);
    _aa2->carrier() = "CO";
    _cs.restFareMarkets(*(_trx->itin()[0]), *_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)4, fm->classOfServiceVec().size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, _fareMarket->classOfServiceVec().size());
  }

  void testJourneyTestDoesNotChangeAvailIfNoDiagParamEntered()
  {
    prepareItin();
    prepareJourneyTest();
    _cs.journeyTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
  }

  void testJourneyTestDoesNotChangeAvailIfSOLOCALorSOFLOWnotEntered()
  {
    prepareItin();
    prepareJourneyTest();
    _cs.journeyTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
  }

  void testJourneyTestDoesNotChangeAvailIfSOFLOWEnteredButFareMarketNotFlow()
  {
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::WPNC_SOLO_TEST, "FLOW"));
    prepareItin();
    prepareJourneyTest();
    _fareMarket->setFlowMarket(false);
    _cs.journeyTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
  }

  void testJourneyTestChangeAvailForFlowMarketIfSOFLOWEntered()
  {
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::WPNC_SOLO_TEST, "FLOW"));
    prepareItin();
    prepareJourneyTest();
    _cs.journeyTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (*(_fareMarket->classOfServiceVec()[1]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
  }

  void testJourneyTestChangeDoesNotChangeAvailForLocalMarketIfCxrNotJourney()
  {
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::WPNC_SOLO_TEST, "LOCAL"));
    prepareItin();
    prepareJourneyTest();
    _cs.journeyTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[1]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
  }

  void testJourneyTestChangeAvailForLocalMarket()
  {
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::WPNC_SOLO_TEST, "LOCAL"));
    prepareItin();
    prepareJourneyTest();
    CarrierPreference* cxrPref = _memHandle.create<CarrierPreference>();
    _aa1->carrierPref() = cxrPref;
    cxrPref->flowMktJourneyType() = YES;
    _cs.journeyTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[1]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)0, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, _aa1->classOfService()[1]->numSeats());
  }

  void testSoloTestDoesntChangeAvailIfNoDiagParamEntered()
  {
    prepareItin();
    prepareJourneyTest();
    _cs.soloTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[1]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    // make sure local avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, _aa1->classOfService()[1]->numSeats());
  }

  void testSoloTestDoesntChangeAvailIfSOLOTESTNotEntered()
  {
    _trx->diagnostic().diagParamMap().insert(pair<string, string>(Diagnostic::WPNC_SOLO_TEST, ""));
    prepareItin();
    prepareJourneyTest();
    _cs.soloTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_fareMarket->classOfServiceVec()[1]))[1]->numSeats()); // make sure flow
                                                                               // avail not changed
    // make sure local avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, _aa1->classOfService()[1]->numSeats());
  }

  void testSoloTestChangeFlowAvailIfSOLOTESTentered()
  {
    _trx->diagnostic().diagParamMap().insert(
        pair<string, string>(Diagnostic::WPNC_SOLO_TEST, "LOTEST"));
    prepareItin();
    prepareJourneyTest();
    _cs.soloTest(*_trx, *(_trx->itin()[0]));
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (*(_fareMarket->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (*(_fareMarket->classOfServiceVec()[1]))[1]->numSeats());
    // make sure local avail not changed
    CPPUNIT_ASSERT_EQUAL(
        (uint16_t)1, (*(_trx->itin()[0]->fareMarket()[1]->classOfServiceVec()[0]))[1]->numSeats());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, _aa1->classOfService()[1]->numSeats());
  }

  void testDummyCOSReturnsCosForAllCharacters()
  {
    ContentServicesStub csStub;
    BookingCode expectedBc = "A";
    ClassOfService* cos = csStub.dummyCOS(0, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isFirstClass());

    expectedBc = "B";
    cos = csStub.dummyCOS(1, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "C";
    cos = csStub.dummyCOS(2, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isBusinessClass());

    expectedBc = "D";
    cos = csStub.dummyCOS(3, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isBusinessClass());

    expectedBc = "E";
    cos = csStub.dummyCOS(4, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "F";
    cos = csStub.dummyCOS(5, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isFirstClass());

    expectedBc = "G";
    cos = csStub.dummyCOS(6, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "H";
    cos = csStub.dummyCOS(7, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "I";
    cos = csStub.dummyCOS(8, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "J";
    cos = csStub.dummyCOS(9, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isBusinessClass());

    expectedBc = "K";
    cos = csStub.dummyCOS(10, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "L";
    cos = csStub.dummyCOS(11, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "M";
    cos = csStub.dummyCOS(12, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "N";
    cos = csStub.dummyCOS(13, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "O";
    cos = csStub.dummyCOS(14, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "P";
    cos = csStub.dummyCOS(15, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isFirstClass());

    expectedBc = "Q";
    cos = csStub.dummyCOS(16, _aa1,_trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "R";
    cos = csStub.dummyCOS(17, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isFirstClass());

    expectedBc = "S";
    cos = csStub.dummyCOS(18, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "T";
    cos = csStub.dummyCOS(19, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "U";
    cos = csStub.dummyCOS(20, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "V";
    cos = csStub.dummyCOS(21, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "W";
    cos = csStub.dummyCOS(22, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "X";
    cos = csStub.dummyCOS(23, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "Y";
    cos = csStub.dummyCOS(24, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());

    expectedBc = "Z";
    cos = csStub.dummyCOS(25, _aa1, _trx->dataHandle());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cos->numSeats());
    CPPUNIT_ASSERT_EQUAL(expectedBc, cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isEconomyClass());
  }

  void testGetACosReturnAPointer()
  {
    ContentServicesStub csStub;
    _aa1->setBookingCode("F");
    _aa1->bookedCabin().setFirstClass();
    ClassOfService* cos = csStub.getACos(*_trx, _aa1);
    CPPUNIT_ASSERT_EQUAL(_aa1->getBookingCode(), cos->bookingCode());
    CPPUNIT_ASSERT(cos->cabin().isFirstClass());
    CPPUNIT_ASSERT_EQUAL(csStub.numSeatsNeeded(*_trx), cos->numSeats());
  }

  void testGetCosFlownDoesNotBuildCosWhenNotRexTrx()
  {
    ContentServicesStub csStub;
    prepareItin();
    csStub.getCosFlown(*_trx, *_fareMarket);
    CPPUNIT_ASSERT_EQUAL((size_t)0, _aa1->classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, _fareMarket->classOfServiceVec().size());
  }

  void testGetCosFlownDoesNotBuildCosWhenFirstSegArunk()
  {
    ContentServicesStub csStub;
    ExchangePricingTrx excTrx;
    prepareItin();
    ArunkSeg arunkSeg;
    _fareMarket->travelSeg()[0] = &arunkSeg;
    csStub.getCosFlown(excTrx, *_fareMarket);
    CPPUNIT_ASSERT_EQUAL((size_t)0, _aa1->classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, _fareMarket->classOfServiceVec().size());
  }

  void testGetCosFlownDoesNotBuildCosWhenFirstSegUnflown()
  {
    ContentServicesStub csStub;
    ExchangePricingTrx excTrx;
    prepareItin();
    csStub.getCosFlown(excTrx, *_fareMarket);
    CPPUNIT_ASSERT_EQUAL((size_t)0, _aa1->classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, _fareMarket->classOfServiceVec().size());
  }

  void testGetCosFlownBuildCosWhenFirstSegFlown()
  {
    ContentServicesStub csStub;
    ExchangePricingTrx excTrx;
    _aa1->unflown() = false;
    prepareItin();
    csStub.getCosFlown(excTrx, *_fareMarket);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _aa1->classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, _fareMarket->classOfServiceVec().size());
  }

  void testFillDummyCosDoesntBuildCosInSegWhenSegArunk()
  {
    ContentServicesStub csStub;
    ArunkSeg arunk;
    _fareMarket->travelSeg() += &arunk;
    csStub.fillDummyCOS(*_fareMarket, _trx->dataHandle(), *_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)0, arunk.classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, _fareMarket->classOfServiceVec().size());
  }

  void testFillDummyCosDoesntBuildCosInSegWhenSegNotOpen()
  {
    ContentServicesStub csStub;
    _fareMarket->travelSeg() += _aa1;
    csStub.fillDummyCOS(*_fareMarket, _trx->dataHandle(), *_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)0, _aa1->classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, _fareMarket->classOfServiceVec().size());
  }

  void testFillDummyCosBuildCosInSegAndFmWhenSegOpen()
  {
    ContentServicesStub csStub;
    _aa1->destination() = TestLocFactory::create(LOC_LON);
    _aa1->origin() = TestLocFactory::create(LOC_DFW);
    _fareMarket->travelSeg() += _aa1;
    _aa1->segmentType() = Open;
    csStub.fillDummyCOS(*_fareMarket, _trx->dataHandle(), *_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)26, _aa1->classOfService().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, _fareMarket->classOfServiceVec().size());
  }

  void testGetSchedAndAvailCallAs2WhenIsNotRexBaseTrx()
  {
    ContentServicesStub csStub;
    prepareItin();
    prepareForJourney();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    Billing bill;
    _trx->billing() = &bill;
    try { csStub.getSchedAndAvail(*_trx); }
    catch (string s) { CPPUNIT_ASSERT_EQUAL(string("AS2 call"), s); }
    catch (...) { CPPUNIT_FAIL("AS2 not called"); }
  }

  void testGetSchedAndAvailCallAs2WhenIsNotAnalyzingExcItin()
  {
    ContentServicesStub csStub;
    _trx = _memHandle.create<RexPricingTrx>();
    _trx->setRequest(&_request);
    Billing bill;
    _trx->billing() = &bill;
    prepareItin();
    prepareForJourney();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    try { csStub.getSchedAndAvail(*_trx); }
    catch (string s)
    {
      CPPUNIT_ASSERT_EQUAL(1, csStub._dssV2Call);
      CPPUNIT_ASSERT_EQUAL(string("AS2 call"), s);
    }
    catch (...) { CPPUNIT_FAIL("AS2 not called"); }
  }

  void testGetSchedAndAvailDontCallAs2WhenIsAnalyzingExcItin()
  {
    ContentServicesStub csStub;
    RexPricingTrx* rexTrx = _memHandle.create<RexPricingTrx>();
    Itin newItin;
    ExcItin excItin;
    PricingRequest request;
    PricingOptions option;
    _aa1->destination() = TestLocFactory::create(LOC_LON);
    rexTrx->travelSeg() += _aa1;
    excItin.travelSeg() += _aa1;
    newItin.travelSeg() += _aa1;
    rexTrx->exchangeItin() += &excItin;
    request.validatingCarrier() = "AA";
    rexTrx->setRequest(&request);
    rexTrx->setOptions(&option);

    rexTrx->setAnalyzingExcItin(true);
    _trx = rexTrx;
    Billing bill;
    _trx->billing() = &bill;
    prepareItin();
    prepareForJourney();
    CPPUNIT_ASSERT_NO_THROW(csStub.getSchedAndAvail(*_trx));
    CPPUNIT_ASSERT_EQUAL(1, csStub._dssV2Call);
  }

protected:
  ContentServices _cs;
  FareMarket* _fareMarket;
  AirSeg* _continental1;
  AirSeg* _continental2;
  AirSeg* _continental3;
  AirSeg* _aa1;
  AirSeg* _aa2;
  AirSeg* _aa3;
  AirSeg* _aa4;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest _request;

  void prepareJourneyTest()
  {
    prepareCosVec(_aa1->classOfService());
    prepareCosVec(_aa1->classOfService());
    _aa1->classOfService()[1]->bookingCode() = "J";
    prepareCosVec(_aa2->classOfService());
    prepareCosVec(_aa2->classOfService());
    _aa2->classOfService()[1]->bookingCode() = "J";

    _fareMarket->setFlowMarket(true);
    vector<ClassOfService*>* cosVec1 = _memHandle.create<vector<ClassOfService*> >();
    vector<ClassOfService*>* cosVec2 = _memHandle.create<vector<ClassOfService*> >();
    _fareMarket->classOfServiceVec() += cosVec1, cosVec2;
    prepareCosVec(*cosVec1);
    prepareCosVec(*cosVec1);
    prepareCosVec(*cosVec2);
    prepareCosVec(*cosVec2);
    (*cosVec1)[1]->bookingCode() = "J";
    (*cosVec2)[1]->bookingCode() = "J";

    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg() += _aa1;
    _trx->itin()[0]->fareMarket().push_back(fm);
    vector<ClassOfService*>* cosVec = _memHandle.create<vector<ClassOfService*> >();
    fm->classOfServiceVec().push_back(cosVec);
    prepareCosVec(*cosVec);
    prepareCosVec(*cosVec);
    (*(cosVec))[1]->bookingCode() = "J";

    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic195;
  }
  FareMarket* prepareFareMarket()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->availBreaks().resize(4);
    fm->availBreaks()[0] = fm->availBreaks()[1] = fm->availBreaks()[2] = false;
    fm->availBreaks()[3] = true;
    fm->travelSeg() += _aa1, _aa2, _aa3, _aa4;
    prepareCosVec(_aa1->classOfService());
    prepareCosVec(_aa2->classOfService());
    prepareCosVec(_aa3->classOfService());
    prepareCosVec(_aa4->classOfService());
    return fm;
  }

  void prepareForJourney()
  {
    ReservationData* res = _memHandle.create<ReservationData>();
    _trx->getRequest()->reservationData() = res;
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    _trx->setOptions(opt);
    opt->journeyActivatedForPricing() = true;
    opt->applyJourneyLogic() = true;
    prepareItin();
  }

  void prepareItin()
  {
    Itin* itin = _memHandle.create<Itin>();
    _fareMarket->travelSeg() += _aa1, _aa2;
    itin->travelSeg() = _fareMarket->travelSeg();
    _trx->itin().push_back(itin);
    itin->fareMarket().push_back(_fareMarket);
    _aa1->pnrSegment() = 1;
    _aa2->pnrSegment() = 2;
    _fareMarket->availBreaks().resize(2);
    _fareMarket->availBreaks()[0] = false;
    _fareMarket->availBreaks()[1] = true;
  }

  void processResponse(const std::string& response, const MethodGetFlownSchedule getFlownSchedule, PricingTrx& trx)
  {
    PricingDssFlightMap flightMap;
    ContentServicesStub csStub;
    csStub.populateFlightMap(trx, getFlownSchedule, flightMap);

    const bool isFirstSegmentUnflown = flightMap.cbegin()->first._unflown;
    PricingDssResponseHandler dssRespHandler(trx, isFirstSegmentUnflown);
    dssRespHandler.initialize();

    dssRespHandler.parse(response.c_str());
    csStub.processDssFlights(trx, dssRespHandler._dssFlights.cbegin(),
                      dssRespHandler._dssFlights.cend(), flightMap);
  }

  void prepareDssRequestData(PricingTrx& trx, const std::string& equipCode = std::string())
  {
    Billing* billing = _memHandle.create<Billing>();
    PricingRequest* pRequest = trx.getRequest();
    Agent* agent = _memHandle.create<Agent>();

    Customer* cust = _memHandle.create<Customer>();
    cust->crsCarrier() = "1F";
    cust->hostName() = "INFI";

    agent->agentTJR() = cust;
    pRequest->ticketingAgent() = agent;

    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->origAirport() = "DFW";
    airSeg->destAirport() = "LGW";
    airSeg->carrier() = "BA";
    airSeg->flightNumber() = 2192;

    AirSeg* airSegSecondItin = _memHandle.create<AirSeg>();
    airSegSecondItin->origAirport() = "DFW";
    airSegSecondItin->destAirport() = "LGW";
    airSegSecondItin->carrier() = "BA";
    airSegSecondItin->flightNumber() = 2192;

    if (!equipCode.empty())
    {
      airSeg->equipmentType() = equipCode.c_str();
      airSegSecondItin->equipmentType() = equipCode.c_str();
    }

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back((TravelSeg*)airSeg);
    Itin* secondItin = _memHandle.create<Itin>();
    secondItin->travelSeg().push_back((TravelSeg*)airSegSecondItin);

    trx.itin().push_back(itin);
    trx.itin().push_back(secondItin);
    trx.setRequest(pRequest);
    trx.billing() = billing;
  }


  AvailData* prepareAvailData(int numFlightAvails)
  {
    AvailData* avlData = _memHandle.create<AvailData>();
    FlightAvail* fltAvail = 0;
    for (int i = 0; i < numFlightAvails; ++i)
    {
      fltAvail = _memHandle.create<FlightAvail>();
      fltAvail->upSellPNRsegNum() = i + 1;
      fltAvail->availMethod() = ContentServices::AVL_STATUS_AVS;
      fltAvail->bookingCodeAndSeats() = "F1|J50|Y100";
      avlData->flightAvails() += fltAvail;
    }
    return avlData;
  }

  void prepareCosVec(vector<ClassOfService*>& cosVec)
  {
    ClassOfService* csp = _memHandle.create<ClassOfService>();
    csp->bookingCode() = "Y";
    csp->numSeats() = 1;
    cosVec.push_back(csp);
  }

  void assertNumFlightsForStartIndex(size_t startIndex, size_t expectedNumberOfFlights)
  {
    Itin itin;
    itin.travelSeg() = _fareMarket->travelSeg();
    CPPUNIT_ASSERT_EQUAL(expectedNumberOfFlights,
                         FareMarketUtil::numFlts(itin, *_fareMarket, startIndex));
  }

  AirSeg* createAirSeg(const string& carrier)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->carrier() = carrier;
    return seg;
  }

  void prepareFourFlightsWithStopOverAtSecond()
  {
    DateTime sameDateArrivalDeparture(2009, 2, 20);
    DateTime nextDayArrivalDeparture(2009, 2, 22);
    _aa1->departureDT() = sameDateArrivalDeparture;
    _aa1->arrivalDT() = sameDateArrivalDeparture;
    _aa2->departureDT() = sameDateArrivalDeparture;
    _aa2->arrivalDT() = sameDateArrivalDeparture;
    _aa3->departureDT() = nextDayArrivalDeparture;
    _aa3->arrivalDT() = nextDayArrivalDeparture;
    _aa4->departureDT() = nextDayArrivalDeparture;
    _aa4->arrivalDT() = nextDayArrivalDeparture;
  }

  void prepareFareMarketWithOnlineConnections()
  {
    _fareMarket->travelSeg() += (TravelSeg*)_aa1, (TravelSeg*)_aa2;
    DateTime departFirstSeg = DateTime(2009, 6, 26, 8, 15, 0);
    DateTime arrivalFirstSeg = DateTime(2009, 6, 26, 9, 15, 0);
    _aa1->departureDT() = departFirstSeg;
    _aa1->arrivalDT() = arrivalFirstSeg;
    DateTime departSecondSeg = DateTime(2009, 6, 26, 10, 15, 0);
    DateTime arrivalSecondSeg = DateTime(2009, 6, 26, 11, 15, 0);
    _aa2->departureDT() = departSecondSeg;
    _aa2->arrivalDT() = arrivalSecondSeg;
  }

  void addTwoMoreSegsToFareMarket()
  {
    _fareMarket->travelSeg() += (TravelSeg*)_aa3, (TravelSeg*)_aa4;
    DateTime departFirstSeg = DateTime(2009, 6, 26, 12, 15, 0);
    DateTime arrivalFirstSeg = DateTime(2009, 6, 26, 13, 15, 0);
    _aa3->departureDT() = departFirstSeg;
    _aa3->arrivalDT() = arrivalFirstSeg;
    DateTime departSecondSeg = DateTime(2009, 6, 26, 14, 15, 0);
    DateTime arrivalSecondSeg = DateTime(2009, 6, 26, 15, 15, 0);
    _aa4->departureDT() = departSecondSeg;
    _aa4->arrivalDT() = arrivalSecondSeg;
  }

  void testCheckAndLogAs2Errors()
  {
    const std::string xmlData = "<ATS Q3B=\"0\" S1I=\"PARS\">\n"
                          "<DIA>\n"
                          "<![CDATA[Transaction failed: SEG with the same refid=-1 found!]]></DIA>\n"
                          "</ATS>\n";
    TestLogger logger = TestLogger("atseintl.ATAE.ContentServices");
    ContentServices cs;
    cs.checkAndLogAs2Errors(xmlData);
    CPPUNIT_ASSERT_EQUAL(std::string("ERROR - ASv2: SEG with the same refid=-1 found!\n"), logger.str());
  }

  void testConstruct()
  {
    PricingTrx trx;

    ContentServices cs;

    CPPUNIT_ASSERT(!cs.shouldPopulateAllItineraries(trx));
  }

  void testConstruct_ancillary()
  {
    AncRequest ancRq;
    ancRq.majorSchemaVersion() = 1;
    AncillaryPricingTrx trx;
    trx.setRequest(&ancRq);

    ContentServices cs;

    CPPUNIT_ASSERT(!cs.shouldPopulateAllItineraries(trx));
  }

  void testConstruct_ancillary_ACS()
  {
    AncRequest ancRq;
    ancRq.majorSchemaVersion() = 2;
    ancRq.ancRequestType() = AncRequest::M70Request;
    Billing billing;
    billing.requestPath() = ACS_PO_ATSE_PATH;
    AncillaryPricingTrx trx;
    trx.setRequest(&ancRq);
    trx.billing() = &billing;

    ContentServices cs;

    CPPUNIT_ASSERT(cs.shouldPopulateAllItineraries(trx));
  }

  void testConstruct_ancillary_WPBG()
  {
    AncRequest ancRq;
    ancRq.majorSchemaVersion() = 2;
    ancRq.ancRequestType() = AncRequest::WPBGRequest;
    Billing billing;
    billing.requestPath() = PSS_PO_ATSE_PATH;
    AncillaryPricingTrx trx;
    trx.setRequest(&ancRq);
    trx.billing() = &billing;

    ContentServices cs;

    CPPUNIT_ASSERT(cs.shouldPopulateAllItineraries(trx));
  }

  void testConstruct_ancillary_MISC6()
  {
    AncRequest ancRq;
    ancRq.majorSchemaVersion() = 2;
    ancRq.ancRequestType() = AncRequest::M70Request;
    Billing billing;
    billing.requestPath() = PSS_PO_ATSE_PATH;
    billing.actionCode() = "MISC6";
    AncillaryPricingTrx trx;
    trx.setRequest(&ancRq);
    trx.billing() = &billing;

    ContentServices cs;
    CPPUNIT_ASSERT(cs.shouldPopulateAllItineraries(trx));
  }

  void testConstruct_ancillary_AB240()
  {
    AncRequest ancRq;
    ancRq.majorSchemaVersion() = 3;

    AncillaryPricingTrx trx;
    trx.setRequest(&ancRq);
    Billing billing;
    trx.billing() = &billing;
    trx.modifiableActivationFlags().setAB240(true);

    ContentServices cs;
    CPPUNIT_ASSERT(cs.shouldPopulateAllItineraries(trx));
  }

  void testEquipmentCodeNotSet()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx->setRequest(pRequest);
    prepareDssRequestData(*trx);

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\"><FLL><ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\"/></FLL><PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    EquipmentType equpType = trx->itin()[0]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("777"), equpType);
    equpType = trx->itin()[1]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT(equpType.empty());
  }

  void testEquipmentCodeSet()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx->setRequest(pRequest);

    prepareDssRequestData(*trx, "767");

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\"><FLL><ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\"/></FLL><PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    EquipmentType equpType = trx->itin()[0]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("767"), equpType);
    equpType = trx->itin()[1]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("767"), equpType);
  }

  void testEquipmentCodeNotSet_allItins()
  {
    //all itins
    PricingTrx* trx = _memHandle.create<AncillaryPricingTrx>();
    PricingRequest* pRequest = _memHandle.create<AncRequest>();
    pRequest->majorSchemaVersion() = 2;
    trx->modifiableActivationFlags().setAB240(true);
    trx->setRequest(pRequest);

    prepareDssRequestData(*trx);

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\"><FLL><ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\"/></FLL><PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    EquipmentType equpType = trx->itin()[0]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("777"), equpType);
    equpType = trx->itin()[1]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("777"), equpType);
  }

  void testEquipmentCodeSet_allItins()
  {
    //all itins
    PricingTrx* trx = _memHandle.create<AncillaryPricingTrx>();
    PricingRequest* pRequest = _memHandle.create<AncRequest>();
    pRequest->majorSchemaVersion() = 2;
    trx->modifiableActivationFlags().setAB240(true);
    trx->setRequest(pRequest);

    prepareDssRequestData(*trx, "767");

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\"><FLL><ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\"/></FLL><PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    EquipmentType equpType = trx->itin()[0]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("767"), equpType);
    equpType = trx->itin()[1]->travelSeg()[0]->equipmentType();
    CPPUNIT_ASSERT_EQUAL(EquipmentType("767"), equpType);
  }

  void testHiddenStopDetails_0HiddenStops()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx->setRequest(pRequest);

    prepareDssRequestData(*trx);

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">"
          "<FLL>"
          "<ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\">"
          "</ASG>"
          "</FLL>"
          "<PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    CPPUNIT_ASSERT_EQUAL(size_t(0), trx->itin()[0]->travelSeg()[0]->hiddenStopsDetails().size());
  }

  void testHiddenStopDetails_2HiddenStops()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx->setRequest(pRequest);

    trx->ticketingDate() = DateTime(2014, 5, 21);
    prepareDssRequestData(*trx);

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">"
          "<FLL>"
          "<ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\">"

          "<HSG A00=\"PHX\" B40=\"320\" D03=\"2014-05-21\" D04=\"2014-05-21\" D31=\"09:05\" "
            "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

          "<HSG A00=\"PH2\" B40=\"320\" D03=\"2014-05-21\" D04=\"2014-05-21\" D31=\"09:05\" "
            "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

          "</ASG>"
          "</FLL>"
          "<PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    CPPUNIT_ASSERT_EQUAL(size_t(2), trx->itin()[0]->travelSeg()[0]->hiddenStopsDetails().size());
  }

  void testHiddenStopDetails_HiddenStopDetails()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx->setRequest(pRequest);

    trx->ticketingDate() = DateTime(2014, 5, 21);

    prepareDssRequestData(*trx);

    std::string response =
          "<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\">"
          "<FLL>"
          "<ASG DDA=\"13018\" BBR=\"false\" "
          "NPR=\"false\" ICD=\"false\" CTT=\"false\" SSA=\"false\" BCC=\"\" ORG=\"DFW\" DST=\"LGW\" "
          "MXC=\"BA\" FLT=\"2192\" MFS=\"\" ED1=\"2005-07-23\" ED2=\"2005-10-28\" ED3=\"SMTWTFS\" "
          "SCH=\"1656480267\" ODA=\"0\" ODD=\"00:00\" OGA=\"-300\" OLN=\"1\" OTC=\"D\" DD2=\"1\" "
          "DSA=\"07:20\" DGA=\"60\" STC=\"0\" DTC=\"N\" IDA=\"0\" CXC=\"F A J C D I W T Y B H K M R "
          "V N L S Q O\" MX2=\"M M M M\" OCX=\"BA\" OFN=\"2192\" OPC=\"BA\" MXX=\"\" ALX=\"\" "
          "TRS=\"II\" TRX=\" \" TRA=\" \" ONT=\"\" DOT=\"false\" LGS=\"false\" ETX=\"true\" "
          "SMK=\"false\" CHT=\"false\" FNL=\"false\" EQP=\"777\" EQ1=\"777\" EQT=\"W\" DQ1=\"777\" "
          "DQT=\"W\" LOF=\"\" ORC=\"US\" DSC=\"GB\" ORS=\"TX\" DS2=\"\" ED4=\"SMTWTFS\" "
          "TMS=\"2005-07-18 16:20:36\">"

          "<HSG A00=\"PHX\" B40=\"320\" D03=\"2014-05-20\" D04=\"2014-05-21\" D31=\"09:05\" "
            "D32=\"08:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

          "<HSG A00=\"PH2\" B40=\"420\" D03=\"2015-05-20\" D04=\"2015-05-21\" D31=\"10:05\" "
            "D32=\"07:08\" D64=\"57\" D65=\"323\" D80=\"-420\" Q0M=\"1788\"/>"

          "</ASG>"
          "</FLL>"
          "<PTM D83=\"0.000000\" D84=\"0.000373\" "
          "D85=\"0.000000\" D86=\"0.000240\" D87=\"0.000000\" D88=\"0.000072\"/></DSS>";

    processResponse(response, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN, *trx);

    const HiddenStopDetails& details = trx->itin()[0]->travelSeg()[0]->hiddenStopsDetails()[1];

    CPPUNIT_ASSERT_EQUAL(LocCode("PH2"), details.airport());
    CPPUNIT_ASSERT_EQUAL(EquipmentType("420"), details.equipment());
    CPPUNIT_ASSERT_EQUAL(DateTime(2015, 5, 20, 10, 5), details.departureDate());
    CPPUNIT_ASSERT_EQUAL(DateTime(2015, 5, 21, 7, 8), details.arrivalDate());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ContentServicesTest);
}
