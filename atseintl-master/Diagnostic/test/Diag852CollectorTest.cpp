#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include <sstream>

#include "Common/BaggageTripType.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/test/Diag852ParsedParamsTester.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"

namespace tse
{
using boost::assign::operator+=;

// MOCKS
namespace
{
class MyDataHandle : public DataHandleMock
{
  bool isCarrierUSDot(const CarrierCode& carrier) { return carrier == "AA"; }
  bool isCarrierCTA(const CarrierCode& carrier) { return carrier == "AC"; }
};
}

class Diag852CollectorTest : public CppUnit::TestFixture
{
  class S7PrinterMock : public Diag852Collector::S7PrinterBase
  {
    friend class Diag852CollectorTest;

  public:
    S7PrinterMock(Diag852Collector& diag) : Diag852Collector::S7PrinterBase(diag) {}

    void printS7Processing(const PricingTrx& trx, const BaggageTravel* baggageTravel) {}
    void printTravelInfo(const BaggageTravel* baggageTravel, const Ts2ss& ts2ss) {}
    void printS7DetailInfo(const OptionalServicesInfo* info, const PricingTrx& trx) {}
  };

  CPPUNIT_TEST_SUITE(Diag852CollectorTest);

  CPPUNIT_TEST(testPrintItinAnalysisResults_JourneyTypeToFromUs);
  CPPUNIT_TEST(testPrintItinAnalysisResults_JourneyTypeToFromCa);
  CPPUNIT_TEST(testPrintItinAnalysisResults_JourneyTypeNonUsDot);
  CPPUNIT_TEST(testPrintItinAnalysisResults_TicketingDate);

  CPPUNIT_TEST(testPrintCheckedPoints_Origin);
  CPPUNIT_TEST(testPrintCheckedPoints_Destination);
  CPPUNIT_TEST(testPrintCheckedPoints_Mixed);

  CPPUNIT_TEST(testPrintFurthestCheckedPoint_Origin);
  CPPUNIT_TEST(testPrintFurthestCheckedPoint_Destination);
  CPPUNIT_TEST(testPrintFurthestCheckedPoint_Empty);

  CPPUNIT_TEST(testPrintOriginAndDestination_UsDot);
  CPPUNIT_TEST(testPrintOriginAndDestination_NonUsDot);

  CPPUNIT_TEST(testPrintBaggageTravels_MultipleSegments);
  CPPUNIT_TEST(testPrintBaggageTravels_SingleSegments);
  CPPUNIT_TEST(testPrintBaggageTravels_MscUsDot);
  CPPUNIT_TEST(testPrintBaggageTravels_FcicUsDot);
  CPPUNIT_TEST(testPrintBaggageTravels_MscNonUsDot);
  CPPUNIT_TEST(testPrintBaggageTravels_FcicNonUsDot);

  CPPUNIT_TEST(testPrintS5Record_Atpco);
  CPPUNIT_TEST(testPrintS5Record_Mmgr);
  CPPUNIT_TEST(testPrintS5Record_Marketing);

  CPPUNIT_TEST(testDisplayAmount_Weight);
  CPPUNIT_TEST(testDisplayAmount_Pieces);
  CPPUNIT_TEST(testDisplayAmount_Both);
  CPPUNIT_TEST(testDisplayAmount_Charges);
  CPPUNIT_TEST(testDisplayAmount_CarryOnCharges);

  CPPUNIT_TEST(testPrintS7DetailInfo_AllPresent);
  CPPUNIT_TEST(testPrintS7DetailInfo_NonePresent);

  CPPUNIT_TEST(testPrintBaggageHeader_AllowanceNotOverridden_ChargesNotOverridden);
  CPPUNIT_TEST(testPrintBaggageHeader_AllowanceOverridden_ChargesNotOverriden);
  CPPUNIT_TEST(testPrintBaggageHeader_AllowanceNotOverridden_chargesOverridden);
  CPPUNIT_TEST(testPrintBaggageHeader_AllowanceOverridden_ChargesOverridden);

  CPPUNIT_TEST(testPrintCarryOnBaggageHeader_CarryOnAllowanceOption);
  CPPUNIT_TEST(testPrintCarryOnBaggageHeader_CarryOnChargesOption);
  CPPUNIT_TEST(testPrintCarryOnBaggageHeader_EmbargoesOption);
  CPPUNIT_TEST(testPrintCarryOnBaggageHeader_NoOption);

  CPPUNIT_TEST(testPrintS7CommonHeader_Allowance);
  CPPUNIT_TEST(testPrintS7CommonHeader_Charges);
  CPPUNIT_TEST(testPrintS7CommonHeader_CarryOnCharges);
  CPPUNIT_TEST(testPrintS7CommonHeader_Embargoes);

  CPPUNIT_TEST(testHasDisplayChargesOption_True);
  CPPUNIT_TEST(testHasDisplayChargesOption_False);

  CPPUNIT_TEST(testPrintChargesHeader);

  CPPUNIT_TEST(testPrintCharge_First);
  CPPUNIT_TEST(testPrintCharge_Second);
  CPPUNIT_TEST(testPrintCharge_Atp);
  CPPUNIT_TEST(testPrintCharge_Mm);
  CPPUNIT_TEST(testPrintCharge_BlankOccurrence);
  CPPUNIT_TEST(testPrintCharge_NoAvailNoCharges_X);

  CPPUNIT_TEST(testDisplayFFMileageAppl_1);
  CPPUNIT_TEST(testDisplayFFMileageAppl_2);
  CPPUNIT_TEST(testDisplayFFMileageAppl_3);
  CPPUNIT_TEST(testDisplayFFMileageAppl_4);
  CPPUNIT_TEST(testDisplayFFMileageAppl_5);
  CPPUNIT_TEST(testDisplayFFMileageAppl_Default);

  CPPUNIT_TEST(testPrintTariffCarriers_ToFromUs);
  CPPUNIT_TEST(testPrintTariffCarriers_ToFromCa);
  CPPUNIT_TEST(testPrintTariffCarriers_BetweenUsCa);

  CPPUNIT_TEST(testPrintCheckedPoint_AtOrigin);
  CPPUNIT_TEST(testPrintCheckedPoint_AtDestination);

  CPPUNIT_TEST(testPrintS7ChargesHeader_1stBag);
  CPPUNIT_TEST(testPrintS7ChargesHeader_2ndBag);

  CPPUNIT_TEST(testPrintTable196DetailSetup_NoData);
  CPPUNIT_TEST(testPrintTable196DetailSetup_seqNo);
  CPPUNIT_TEST(testPrintTable196DetailSetup_cp);
  CPPUNIT_TEST(testPrintTable196DetailSetup_fl);
  CPPUNIT_TEST(testPrintTable196DetailSetup_allOK);

  CPPUNIT_TEST(testPrintTable196ForBaggageDetailSetup_baggageAllowanceDiag);
  CPPUNIT_TEST(testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag1);
  CPPUNIT_TEST(testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag2);
  CPPUNIT_TEST(testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag3);
  CPPUNIT_TEST(testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag4);

  CPPUNIT_TEST(testPrintTable196ForCarryOnDetailSetup_carryOnAllowanceDiag1);
  CPPUNIT_TEST(testPrintTable196ForCarryOnDetailSetup_carryOnAllowanceDiag2);
  CPPUNIT_TEST(testPrintTable196ForCarryOnDetailSetup_notCarryOnAllowanceDiag1);
  CPPUNIT_TEST(testPrintTable196ForCarryOnDetailSetup_notCarryOnAllowanceDiag2);
  CPPUNIT_TEST(testPrintTable196ForCarryOnDetailSetup_notCarryOnAllowanceDiag3);

  CPPUNIT_TEST(test_printFareCheckInfo_FAACTIVE);
  CPPUNIT_TEST(test_printFareCheckInfo_NOT_FAACTIVE);
  CPPUNIT_TEST(test_printFareCheckInfo_check_precision);
  CPPUNIT_TEST(test_printS7OptionalServiceStatus_FAACTIVE);
  CPPUNIT_TEST(test_printS7OptionalServiceStatus_NOT_FAACTIVE);

  CPPUNIT_TEST(test_S7printerBase_printS7ProcessingHeader);
  CPPUNIT_TEST(test_S7printerBase_printPTC);
  CPPUNIT_TEST(test_S7printerBase_printCorporateId_notEmptyCorpIdVector);
  CPPUNIT_TEST(test_S7printerBase_printCorporateId_emptyCorpIdVector);
  CPPUNIT_TEST(test_S7printerBase_printAccountCode_notEmptyAccountCodeVector);
  CPPUNIT_TEST(test_S7printerBase_printAccountCode_emptyAccountCodeVector);
  CPPUNIT_TEST(test_S7printerBase_printInputDesignator);
  CPPUNIT_TEST(test_S7printerBase_printOutputDesignator);
  CPPUNIT_TEST(test_S7printerBase_printTourCode_cat35);
  CPPUNIT_TEST(test_S7printerBase_printTourCode_cat27);
  CPPUNIT_TEST(test_S7printerBase_printDayOfWeek);
  CPPUNIT_TEST(test_S7printerBase_printEquipment);
  CPPUNIT_TEST(test_S7PrinterDdinfo_printTravelInfoForAllSectors_testFieldsHeaders);
  CPPUNIT_TEST(test_S7PrinterDdinfo_printS7RecordValidationFooter_t198Present);
  CPPUNIT_TEST(test_S7PrinterDdinfo_printS7RecordValidationFooter_t171Present);
  CPPUNIT_TEST(test_S7PrinterDdinfo_printS7RecordValidationFooter_t173Present);
  CPPUNIT_TEST(test_S7PrinterDdinfo_printS7RecordValidationFooter_t186Present);
  CPPUNIT_TEST(test_S7PrinterDdinfo_printS7RecordValidationFooter_emptyText);

  CPPUNIT_TEST(test_validContextForBTA_rbd_yes);
  CPPUNIT_TEST(test_validContextForBTA_rbd_no);
  CPPUNIT_TEST(test_validContextForBTA_t171_yes);
  CPPUNIT_TEST(test_validContextForBTA_t171_no);
  CPPUNIT_TEST(test_validContextForBTA_outTicketDesig_yes);
  CPPUNIT_TEST(test_validContextForBTA_outTicketDesig_no);
  CPPUNIT_TEST(test_validContextForBTA_t186_yes);
  CPPUNIT_TEST(test_validContextForBTA_t186_no);
  CPPUNIT_TEST(test_validContextForBTA_accountCodes_yes);
  CPPUNIT_TEST(test_validContextForBTA_accountCodes_no);
  CPPUNIT_TEST(test_validContextForBTA_inputTicketDesig_yes);
  CPPUNIT_TEST(test_validContextForBTA_inputTicketDesig_no);
  CPPUNIT_TEST(test_validContextForBTA_t170_yes);
  CPPUNIT_TEST(test_validContextForBTA_t170_no);
  CPPUNIT_TEST(test_validContextForBTA_t183_yes);
  CPPUNIT_TEST(test_validContextForBTA_t183_no);

  CPPUNIT_TEST(test_isBTAContextOn_yes);
  CPPUNIT_TEST(test_isBTAContextOn_no_noDDINFO);
  CPPUNIT_TEST(test_isBTAContextOn_no_noBTA);

  CPPUNIT_TEST(test_printBTASegmentHeader);
  CPPUNIT_TEST(test_printBTASegmentFooter_pass);
  CPPUNIT_TEST(test_printBTASegmentFooter_fail);
  CPPUNIT_TEST(test_printBTAFieldStatusCabin_noMatch);
  CPPUNIT_TEST(test_printBTAFieldStatusCabin_match);
  CPPUNIT_TEST(test_printBTAStatusTableT171_match);
  CPPUNIT_TEST(test_printBTAStatusTableT171_noMatch);
  CPPUNIT_TEST(test_printBTAFieldStatusRule_match);
  CPPUNIT_TEST(test_printBTAFieldStatusRule_noMatch);
  CPPUNIT_TEST(test_printBTAFieldStatusRuleTariff_match);
  CPPUNIT_TEST(test_printBTAFieldStatusRuleTariff_noMatch);
  CPPUNIT_TEST(test_printBTAFieldStatusFareInd_match);
  CPPUNIT_TEST(test_printBTAFieldStatusFareInd_noMatch);
  CPPUNIT_TEST(test_printBTAFieldStatusCarrierFlightApplT186_match);
  CPPUNIT_TEST(test_printBTAFieldStatusCarrierFlightApplT186_noMatch);
  CPPUNIT_TEST(test_printBTAStatusOutputTicketDesignator_match);
  CPPUNIT_TEST(test_printBTAStatusOutputTicketDesignator_noMatch);

  CPPUNIT_TEST(testPrintMileageTpmNoData);
  CPPUNIT_TEST(testPrintMileageMpmNoData);
  CPPUNIT_TEST(testPrintMileageMpmMatched);

  CPPUNIT_TEST(testGetCarrierListAsString);
  CPPUNIT_TEST(testPrintDetailInterlineEmdProcessingS5Info);
  CPPUNIT_TEST(testPrintNoInterlineDataFoundInfo);
  CPPUNIT_TEST(testPrintDetailInterlineEmdAgreementInfo);
  CPPUNIT_TEST(testPrintEmdValidationResultNotMatchingCharge);
  CPPUNIT_TEST(testPrintEmdValidationResultRES);
  CPPUNIT_TEST(testPrintEmdValidationResultCKI);

  CPPUNIT_TEST(testPrintInfoAboutLackingBaggageCharges);

  CPPUNIT_TEST_SUITE_END();

public:
  Diag852CollectorTest() { _ut_data_path = "/vobs/atseintl/test/testdata/data/"; }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _itin = _memHandle.create<Itin>();
    _itin->travelSeg() += createAirSeg("KRK", "DFW");
    _itin->travelSeg() += createAirSeg("DFW", "KRK");
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = _itin;
    _itin->farePath() += _farePath;
    _request = _memHandle.create<PricingRequest>();
    _request->ticketingAgent() = _memHandle.create<Agent>();
    _request->ticketingDT() = DateTime(9999, 12, 31);
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->itin() += _itin;
    _trx->setRequest(_request);
    _trx->billing() = _memHandle.create<Billing>();

    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    _dc = _memHandle.create<Diag852Collector>();
    _dc->rootDiag() = rootDiag;
    _dc->trx() = _trx;
    _dc->activate();

    _dcParamTester = _memHandle.insert(new Diag852ParsedParamsTester(_dc->_params));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

protected:
  AirSeg* createAirSeg(const LocCode& originCode, const LocCode& destinationCode, uint16_t pnrSegment = 0)
  {
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = originCode;
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = destinationCode;

    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->origin() = origin;
    airSeg->destination() = destination;
    airSeg->pnrSegment() = pnrSegment;

    return airSeg;
  }

  BaggageTravel* createBaggageTravel(TravelSegPtrVecCI begin, TravelSegPtrVecCI end)
  {
    BaggageTravel* baggageTravel = _memHandle.create<BaggageTravel>();
    baggageTravel->_trx = _trx;
    baggageTravel->setupTravelData(*_farePath);
    baggageTravel->updateSegmentsRange(begin, end);
    baggageTravel->_MSS = begin;
    baggageTravel->_MSSJourney = begin;
    return baggageTravel;
  }

  std::vector<const BaggageTravel*>* createBaggageTravelsForMscFcicTests(const bool usDot = true)
  {
    _itin->travelSeg().clear();
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();

    AirSeg* airSeg = createAirSeg("KRK", "MUC");
    airSeg->setMarketingCarrierCode("LH");
    airSeg->setOperatingCarrierCode("LT");
    travelSegs += airSeg;

    airSeg = createAirSeg("MUC", "FRA");
    airSeg->setMarketingCarrierCode("GW");
    airSeg->setOperatingCarrierCode("AB");
    travelSegs += airSeg;

    airSeg = createAirSeg("FRA", "BVA");
    airSeg->setMarketingCarrierCode("EW");
    airSeg->setOperatingCarrierCode("CW");
    travelSegs += airSeg;

    airSeg = createAirSeg("BVA", "KTW");
    airSeg->setMarketingCarrierCode("AF");
    airSeg->setOperatingCarrierCode("TJ");
    travelSegs += airSeg;

    std::vector<const BaggageTravel*>* baggageTravels =
        _memHandle.create<std::vector<const BaggageTravel*> >();

    BaggageTravel* bggageTravel = createBaggageTravel(travelSegs.begin(), travelSegs.begin() + 2);
    bggageTravel->_MSS = travelSegs.begin() + 1;
    bggageTravel->_MSSJourney = travelSegs.begin() + 1;
    *baggageTravels += bggageTravel;

    bggageTravel = createBaggageTravel(travelSegs.begin() + 2, travelSegs.end());
    bggageTravel->_MSS = travelSegs.begin() + 3;
    bggageTravel->_MSSJourney = usDot ? (travelSegs.begin() + 1) : bggageTravel->_MSS;
    *baggageTravels += bggageTravel;

    return baggageTravels;
  }

  SubCodeInfo* createS5Record(const VendorCode& vendor)
  {
    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->vendor() = vendor;
    s5->carrier() = "AA";
    s5->serviceTypeCode() = "OC";
    s5->industryCarrierInd() = 'I';
    s5->serviceGroup() = "BG";
    s5->serviceSubGroup() = "NN";
    s5->serviceSubTypeCode() = "0DF";
    return s5;
  }

  OptionalServicesInfo* createS7Record()
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->vendor() = ATPCO_VENDOR_CODE;
    s7->serviceSubTypeCode() = "ABC";
    s7->seqNo() = 1000000;
    s7->psgType() = "ADT";
    s7->baggageOccurrenceFirstPc() = 1;
    s7->baggageOccurrenceLastPc() = 2;

    return s7;
  }

  BaggageCharge* createBaggageCharge(const MoneyAmount amount, const CurrencyCode& currency)
  {
    BaggageCharge* charge = _memHandle.create<BaggageCharge>();
    charge->feeAmount() = amount;
    charge->feeCurrency() = currency;

    return charge;
  }

  OCFees* createOCFees(int32_t pieces, int32_t weight, char weightUnit)
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->baggageWeight() = weight;
    s7->baggageWeightUnit() = weightUnit;
    s7->freeBaggagePcs() = pieces;

    OCFees* ocf = _memHandle.create<OCFees>();
    ocf->optFee() = s7;
    return ocf;
  }

  std::string _ut_data_path;
  TestMemHandle _memHandle;
  DataHandleMock* _mdh;
  PricingTrx* _trx;
  PricingRequest* _request;
  Itin* _itin;
  FarePath* _farePath;
  Diag852Collector* _dc;
  Diag852ParsedParamsTester* _dcParamTester;

public:
  // TESTS
  void testPrintItinAnalysisResults_JourneyTypeToFromUs()
  {
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);
    _dc->printItinAnalysisResults(*_trx,
                                  CheckedPointVector(),
                                  CheckedPoint(_itin->travelSeg().begin(), CP_AT_ORIGIN),
                                  _itin->travelSeg().front(),
                                  *_itin,
                                  false);
    CPPUNIT_ASSERT(_dc->str().find("JOURNEY TYPE : US DOT") != std::string::npos);
  }

  void testPrintItinAnalysisResults_JourneyTypeToFromCa()
  {
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_CA);
    _dc->printItinAnalysisResults(*_trx,
                                  CheckedPointVector(),
                                  CheckedPoint(_itin->travelSeg().begin(), CP_AT_ORIGIN),
                                  _itin->travelSeg().front(),
                                  *_itin,
                                  false);
    CPPUNIT_ASSERT(_dc->str().find("JOURNEY TYPE : CTA") != std::string::npos);
  }

  void testPrintItinAnalysisResults_JourneyTypeNonUsDot()
  {
    _itin->setBaggageTripType(BaggageTripType::OTHER);
    _dc->printItinAnalysisResults(*_trx,
                                  CheckedPointVector(),
                                  CheckedPoint(_itin->travelSeg().begin(), CP_AT_ORIGIN),
                                  _itin->travelSeg().front(),
                                  *_itin,
                                  false);
    CPPUNIT_ASSERT(_dc->str().find("JOURNEY TYPE : NON US DOT") != std::string::npos);
  }

  void testPrintItinAnalysisResults_TicketingDate()
  {
    std::string dateStr = "1410-07-15";
    std::string dateTimeStr = dateStr + " 00:00:00.000";
    _request->ticketingDT() = DateTime(dateTimeStr);

    _dc->printItinAnalysisResults(*_trx,
                                  CheckedPointVector(),
                                  CheckedPoint(_itin->travelSeg().begin(), CP_AT_ORIGIN),
                                  _itin->travelSeg().front(),
                                  *_itin,
                                  false);
    CPPUNIT_ASSERT(_dc->str().find("TKT DATE : " + dateStr) != std::string::npos);
  }

  void testPrintCheckedPoints_Origin()
  {
    _itin->travelSeg().clear();
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += createAirSeg("KRK", "MUC");
    travelSegs += createAirSeg("MUC", "FRA");
    travelSegs += createAirSeg("FRA", "BLR");

    CheckedPointVector checkedPoints;
    checkedPoints += CheckedPoint(travelSegs.begin(), CP_AT_ORIGIN);
    checkedPoints += CheckedPoint(travelSegs.begin() + 1, CP_AT_ORIGIN);
    checkedPoints += CheckedPoint(travelSegs.begin() + 2, CP_AT_ORIGIN);

    _dc->printCheckedPoints(checkedPoints, 0);
    CPPUNIT_ASSERT(_dc->str().find("CHECKED POINTS : KRK MUC FRA") != std::string::npos);
  }

  void testPrintCheckedPoints_Destination()
  {
    _itin->travelSeg().clear();
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += createAirSeg("KRK", "MUC");
    travelSegs += createAirSeg("MUC", "FRA");
    travelSegs += createAirSeg("FRA", "BLR");

    CheckedPointVector checkedPoints;
    checkedPoints += CheckedPoint(travelSegs.begin(), CP_AT_DESTINATION);
    checkedPoints += CheckedPoint(travelSegs.begin() + 1, CP_AT_DESTINATION);
    checkedPoints += CheckedPoint(travelSegs.begin() + 2, CP_AT_DESTINATION);

    _dc->printCheckedPoints(checkedPoints, 0);
    CPPUNIT_ASSERT(_dc->str().find("CHECKED POINTS : MUC FRA BLR") != std::string::npos);
  }

  void testPrintCheckedPoints_Mixed()
  {
    _itin->travelSeg().clear();
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += createAirSeg("KRK", "MUC");
    travelSegs += createAirSeg("FRA", "DFW");
    travelSegs += createAirSeg("NYC", "FRA");

    CheckedPointVector checkedPoints;
    checkedPoints += CheckedPoint(travelSegs.begin(), CP_AT_ORIGIN);
    checkedPoints += CheckedPoint(travelSegs.begin() + 1, CP_AT_DESTINATION);
    checkedPoints += CheckedPoint(travelSegs.begin() + 2, CP_AT_ORIGIN);

    _dc->printCheckedPoints(checkedPoints, 0);
    CPPUNIT_ASSERT(_dc->str().find("CHECKED POINTS : KRK DFW NYC") != std::string::npos);
  }

  void testPrintFurthestCheckedPoint_Origin()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("FRA", "DFW");

    CheckedPoint furthestPoint(travelSegs.begin(), CP_AT_ORIGIN);
    CheckedPointVector checkedPoints;
    checkedPoints += furthestPoint;

    _dc->printFurthestCheckedPoint(checkedPoints, furthestPoint);
    CPPUNIT_ASSERT(_dc->str().find("FURTHEST CHECKED POINT : FRA") != std::string::npos);
  }

  void testPrintFurthestCheckedPoint_Destination()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("FRA", "DFW");

    CheckedPoint furthestPoint(travelSegs.begin(), CP_AT_DESTINATION);
    CheckedPointVector checkedPoints;
    checkedPoints += furthestPoint;

    _dc->printFurthestCheckedPoint(checkedPoints, furthestPoint);
    CPPUNIT_ASSERT(_dc->str().find("FURTHEST CHECKED POINT : DFW") != std::string::npos);
  }

  void testPrintFurthestCheckedPoint_Empty()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("FRA", "DFW");

    CheckedPoint furthestPoint(travelSegs.begin(), CP_AT_DESTINATION);
    CheckedPointVector checkedPoints;

    _dc->printFurthestCheckedPoint(checkedPoints, furthestPoint);
    CPPUNIT_ASSERT(_dc->str().empty());
  }

  void testPrintOriginAndDestination_UsDot()
  {
    _itin->travelSeg() += createAirSeg("KRK", "WAW");
    _itin->travelSeg().back()->equipmentType() = "BUS";

    _dc->printOriginAndDestination(*_itin, true);
    CPPUNIT_ASSERT(_dc->str().find("ORIGIN : KRK") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("DESTINATION : WAW") != std::string::npos);
  }

  void testPrintOriginAndDestination_NonUsDot()
  {
    _itin->travelSeg() += createAirSeg("KRK", "WAW");
    _itin->travelSeg().back()->equipmentType() = "BUS";

    _dc->printOriginAndDestination(*_itin, false);
    CPPUNIT_ASSERT(_dc->str().find("ORIGIN : KRK") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("DESTINATION : KRK") != std::string::npos);
  }

  void testPrintBaggageTravels_MultipleSegments()
  {
    _itin->travelSeg().clear();
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += createAirSeg("KRK", "MUC", 1);
    travelSegs += createAirSeg("MUC", "FRA", 2);
    travelSegs += createAirSeg("FRA", "BVA", 3);
    travelSegs += createAirSeg("BVA", "ORY", 4);
    travelSegs += createAirSeg("ORY", "FDF", 5);
    travelSegs += createAirSeg("FDF", "MIA", 6);

    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels += createBaggageTravel(travelSegs.begin(), travelSegs.begin() + 3);
    baggageTravels += createBaggageTravel(travelSegs.end() - 2, travelSegs.end());

    _dc->printBaggageTravels(baggageTravels, false);
    CPPUNIT_ASSERT(_dc->str().find("01 CHECKED PORTION : KRK 1 - MUC 1,2 - FRA 2,3 - BVA 3") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("02 CHECKED PORTION : ORY 5 - FDF 5,6 - MIA 6") != std::string::npos);
  }

  void testPrintBaggageTravels_SingleSegments()
  {
    _itin->travelSeg().clear();
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += createAirSeg("KRK", "MUC", 1);
    travelSegs += createAirSeg("MUC", "FRA", 2);
    travelSegs += createAirSeg("FRA", "BVA", 3);

    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels += createBaggageTravel(travelSegs.begin(), travelSegs.begin() + 1);
    baggageTravels += createBaggageTravel(travelSegs.begin() + 1, travelSegs.begin() + 2);
    baggageTravels += createBaggageTravel(travelSegs.begin() + 2, travelSegs.end());

    _dc->printBaggageTravels(baggageTravels, false);
    CPPUNIT_ASSERT(_dc->str().find("01 CHECKED PORTION : KRK 1 - MUC 1") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("02 CHECKED PORTION : MUC 2 - FRA 2") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("03 CHECKED PORTION : FRA 3 - BVA 3") != std::string::npos);
  }

  void testPrintBaggageTravels_MscUsDot()
  {
    std::vector<const BaggageTravel*>* baggageTravels = createBaggageTravelsForMscFcicTests();

    _dc->printBaggageTravels(*baggageTravels, true);
    size_t offset = _dc->str().find("MSC MARKETING : GW");
    CPPUNIT_ASSERT(offset != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("MSC MARKETING : GW", offset + 1) != std::string::npos);
  }

  void testPrintBaggageTravels_FcicUsDot()
  {
    std::vector<const BaggageTravel*>* baggageTravels = createBaggageTravelsForMscFcicTests();

    _dc->printBaggageTravels(*baggageTravels, true);
    size_t offset = _dc->str().find("FCIC MARKETING : LH");
    CPPUNIT_ASSERT(offset != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("FCIC MARKETING : LH", offset + 1) != std::string::npos);
  }

  void testPrintBaggageTravels_MscNonUsDot()
  {
    std::vector<const BaggageTravel*>* baggageTravels = createBaggageTravelsForMscFcicTests(false);

    _dc->printBaggageTravels(*baggageTravels, false);
    size_t offset = _dc->str().find("MSC MARKETING : GW");
    CPPUNIT_ASSERT(offset != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("MSC MARKETING : AF", offset + 1) != std::string::npos);
  }

  void testPrintBaggageTravels_FcicNonUsDot()
  {
    std::vector<const BaggageTravel*>* baggageTravels = createBaggageTravelsForMscFcicTests(false);

    _dc->printBaggageTravels(*baggageTravels, false);
    size_t offset = _dc->str().find("FCIC MARKETING : LH");
    CPPUNIT_ASSERT(offset != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("FCIC MARKETING : EW", offset + 1) != std::string::npos);
  }

  void testPrintS5Record_Atpco()
  {
    _dc->printS5Record(createS5Record(ATPCO_VENDOR_CODE), false);
    CPPUNIT_ASSERT(_dc->str().find("A AA  O   OC      I   BG  NN     0DF") != std::string::npos);
  }

  void testPrintS5Record_Mmgr()
  {
    _dc->printS5Record(createS5Record(MERCH_MANAGER_VENDOR_CODE), false);
    CPPUNIT_ASSERT(_dc->str().find("M AA  O   OC      I   BG  NN     0DF") != std::string::npos);
  }

  void testPrintS5Record_Marketing()
  {
    _dc->printS5Record(createS5Record(ATPCO_VENDOR_CODE), true);
    CPPUNIT_ASSERT(_dc->str().find("A AA  M   OC      I   BG  NN     0DF") != std::string::npos);
  }

  void testDisplayAmount_Weight()
  {
    OCFees* ocf = createOCFees(-1, 30, 'K');

    _dc->displayAmount(*ocf);
    CPPUNIT_ASSERT(_dc->str().find(" 30K ") != std::string::npos);
  }

  void testDisplayAmount_Pieces()
  {
    OCFees* ocf = createOCFees(3, -1, ' ');

    _dc->displayAmount(*ocf);
    CPPUNIT_ASSERT(_dc->str().find(" 03PC ") != std::string::npos);
  }

  void testDisplayAmount_Both()
  {
    OCFees* ocf = createOCFees(3, 30, 'K');

    _dc->displayAmount(*ocf);
    CPPUNIT_ASSERT(_dc->str().find(" 03PC/30K ") != std::string::npos);
  }

  void testDisplayAmount_Charges()
  {
    OCFees ocf;
    ocf.feeAmount() = 123.45;
    ocf.feeNoDec() = 2;
    ocf.feeCurrency() = "USD";
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    _dc->displayAmount(ocf);
    CPPUNIT_ASSERT(_dc->str().find("123.45USD") != std::string::npos);
  }

  void testDisplayAmount_CarryOnCharges()
  {
    OCFees ocf;
    ocf.feeAmount() = 123.45;
    ocf.feeNoDec() = 2;
    ocf.feeCurrency() = "USD";
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("CC", ""));

    _dc->displayAmount(ocf);
    CPPUNIT_ASSERT(_dc->str().find("123.45USD") != std::string::npos);
  }

  void testPrintS7DetailInfo_AllPresent()
  {
    OptionalServicesInfo s7;
    s7.freeBaggagePcs() = 1;
    s7.baggageOccurrenceFirstPc() = 2;
    s7.baggageOccurrenceLastPc() = 3;
    s7.baggageWeight() = 40;
    s7.baggageWeightUnit() = 'K';

    _dc->printS7DetailInfo(&s7, *_trx);
    CPPUNIT_ASSERT(_dc->str().find("FREE BAGGAGE PCS : 01") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE OCCURRENCE FIRST PC : 02") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE OCCURRENCE LAST PC : 03") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE WEIGHT : 40") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE WEIGHT UNIT : K") != std::string::npos);
  }

  void testPrintS7DetailInfo_NonePresent()
  {
    OptionalServicesInfo s7;

    _dc->printS7DetailInfo(&s7, *_trx);
    CPPUNIT_ASSERT(_dc->str().find("FREE BAGGAGE PCS : \n") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE OCCURRENCE FIRST PC : \n") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE OCCURRENCE LAST PC : \n") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE WEIGHT : \n") != std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAGGAGE WEIGHT UNIT :  ") != std::string::npos);
  }

  void testPrintBaggageHeader_AllowanceNotOverridden_ChargesNotOverridden()
  {
    AncRequest* req = _memHandle.create<AncRequest>();
    req->majorSchemaVersion() = 2;
    _trx->setRequest(req);

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("--------------------BAGGAGE ALLOWANCE-------------------------\n") !=
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE ALLOWANCE OVERRIDE-------------------\n") ==
        std::string::npos);

    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("---------------------BAGGAGE CHARGES--------------------------\n") !=
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE CHARGES OVERRIDE---------------------\n") ==
        std::string::npos);
  }

  void testPrintBaggageHeader_AllowanceOverridden_ChargesNotOverriden()
  {
    AncRequest* req = _memHandle.create<AncRequest>();
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageAllowance() = "LH";
    _trx->setRequest(req);

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("--------------------BAGGAGE ALLOWANCE-------------------------\n") !=
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE ALLOWANCE OVERRIDE-------------------\n") ==
        std::string::npos);

    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("---------------------BAGGAGE CHARGES--------------------------\n") !=
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE CHARGES OVERRIDE---------------------\n") ==
        std::string::npos);
  }

  void testPrintBaggageHeader_AllowanceNotOverridden_chargesOverridden()
  {
    AncRequest* req = _memHandle.create<AncRequest>();
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageCharges() = "LH";
    _trx->setRequest(req);

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("--------------------BAGGAGE ALLOWANCE-------------------------\n") !=
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE ALLOWANCE OVERRIDE-------------------\n") ==
        std::string::npos);

    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("---------------------BAGGAGE CHARGES--------------------------\n") ==
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE CHARGES OVERRIDE---------------------\n") !=
        std::string::npos);
  }

  void testPrintBaggageHeader_AllowanceOverridden_ChargesOverridden()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    AncRequest* req = _memHandle.create<AncRequest>();
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageAllowance() = "LH";
    req->carrierOverriddenForBaggageCharges() = "LH";
    _trx->setRequest(req);

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("--------------------BAGGAGE ALLOWANCE-------------------------\n") ==
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE ALLOWANCE OVERRIDE-------------------\n") ==
        std::string::npos);

    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    _dc->printBaggageHeader(*_trx);
    CPPUNIT_ASSERT(
        _dc->str().find("---------------------BAGGAGE CHARGES--------------------------\n") ==
        std::string::npos);
    CPPUNIT_ASSERT(
        _dc->str().find("-----------------BAGGAGE CHARGES OVERRIDE---------------------\n") !=
        std::string::npos);
  }

  void testPrintCarryOnBaggageHeader_CarryOnAllowanceOption()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("CA", ""));
    _dc->printCarryOnBaggageHeader(true, false);

    CPPUNIT_ASSERT(
        _dc->str().find("-----------------CARRY ON BAGGAGE ALLOWANCE-------------------\n") !=
        std::string::npos);
  }

  void testPrintCarryOnBaggageHeader_CarryOnChargesOption()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("CC", ""));
    _dc->printCarryOnBaggageHeader(true, false);

    CPPUNIT_ASSERT(
        _dc->str().find("-----------------CARRY ON BAGGAGE CHARGES---------------------\n") !=
        std::string::npos);
  }

  void testPrintCarryOnBaggageHeader_EmbargoesOption()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("EE", ""));
    _dc->printCarryOnBaggageHeader(false, true);

    CPPUNIT_ASSERT(
        _dc->str().find("-----------------------BAGGAGE EMBARGO------------------------\n") !=
        std::string::npos);
  }

  void testPrintCarryOnBaggageHeader_NoOption()
  {
    _dc->printCarryOnBaggageHeader(false, false);

    CPPUNIT_ASSERT(
        _dc->str().find("--------CARRY ON BAGGAGE ALLOWANCE AND CHARGES/EMBARGO--------\n") !=
        std::string::npos);
  }

  void testPrintS7CommonHeader_Allowance()
  {
    _dc->printS7CommonHeader();
    CPPUNIT_ASSERT(_dc->str().find("V SCODE SERVICE   SEQ NUM  PAX PC/WEIGHT  STATUS\n") !=
                   std::string::npos);
  }

  void testPrintS7CommonHeader_Charges()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    _dc->printS7CommonHeader();
    CPPUNIT_ASSERT(_dc->str().find("V SCODE SERVICE   SEQ NUM  PAX CHARGE     STATUS\n") !=
                   std::string::npos);
  }

  void testPrintS7CommonHeader_CarryOnCharges()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("CC", ""));

    _dc->printS7CommonHeader();
    CPPUNIT_ASSERT(_dc->str().find("V SCODE SERVICE   SEQ NUM  PAX CHARGE     STATUS\n") !=
                   std::string::npos);
  }

  void testPrintS7CommonHeader_Embargoes()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("EE", ""));

    _dc->printS7CommonHeader();
    CPPUNIT_ASSERT(_dc->str().find("V SCODE SERVICE   SEQ NUM                 STATUS\n") !=
                   std::string::npos);
  }

  void testHasDisplayChargesOption_True()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair("DC", ""));

    CPPUNIT_ASSERT(_dc->hasDisplayChargesOption());
  }

  void testHasDisplayChargesOption_False() { CPPUNIT_ASSERT(!_dc->hasDisplayChargesOption()); }

  void testPrintChargesHeader()
  {
    _dc->printChargesHeader();
    CPPUNIT_ASSERT(
        _dc->str().find("\n-----------------------BAGGAGE CHARGES------------------------\n") !=
        std::string::npos);
    CPPUNIT_ASSERT(_dc->str().find("BAG V SCODE SEQ NUM  PAX CHARGE     OCC FIRST  OCC LAST\n") !=
                   std::string::npos);
  }

  void testPrintCharge_First()
  {
    _dc->printCharge(0, 0);
    CPPUNIT_ASSERT(_dc->str().find("1ST ") != std::string::npos);
  }

  void testPrintCharge_Second()
  {
    _dc->printCharge(0, 1);
    CPPUNIT_ASSERT(_dc->str().find("2ND ") != std::string::npos);
  }

  void testPrintCharge_Atp()
  {
    OptionalServicesInfo* s7 = createS7Record();
    s7->vendor() = ATPCO_VENDOR_CODE;

    BaggageCharge* charge = createBaggageCharge(123, "USD");
    charge->optFee() = s7;

    _dc->printCharge(charge, 0);
    CPPUNIT_ASSERT(_dc->str().find("1ST A ABC   1000000  ADT USD123.00  1          2") !=
                   std::string::npos);
  }

  void testPrintCharge_Mm()
  {
    OptionalServicesInfo* s7 = createS7Record();
    s7->vendor() = MERCH_MANAGER_VENDOR_CODE;

    BaggageCharge* charge = createBaggageCharge(123, "USD");
    charge->optFee() = s7;

    _dc->printCharge(charge, 0);
    CPPUNIT_ASSERT(_dc->str().find("1ST M ABC   1000000  ADT USD123.00  1          2") !=
                   std::string::npos);
  }

  void testPrintCharge_BlankOccurrence()
  {
    OptionalServicesInfo* s7 = createS7Record();
    s7->baggageOccurrenceFirstPc() = -1;
    s7->baggageOccurrenceLastPc() = -1;

    BaggageCharge* charge = createBaggageCharge(123, "USD");
    charge->optFee() = s7;

    _dc->printCharge(charge, 0);
    CPPUNIT_ASSERT(_dc->str().find("1ST A ABC   1000000  ADT USD123.00  BLANK      BLANK") !=
                   std::string::npos);
  }

  void testPrintCharge_NoAvailNoCharges_X()
  {
    OptionalServicesInfo* s7 = createS7Record();
    s7->baggageOccurrenceFirstPc() = -1;
    s7->baggageOccurrenceLastPc() = -1;
    s7->notAvailNoChargeInd() = 'X';

    BaggageCharge* charge = createBaggageCharge(123, "USD");
    charge->optFee() = s7;

    _dc->printCharge(charge, 0);
    CPPUNIT_ASSERT(_dc->str().find("1ST A ABC   1000000  ADT NOTAVAIL   BLANK      BLANK") !=
                   std::string::npos);
  }

  void testDisplayFFMileageAppl_1()
  {
    _dc->displayFFMileageAppl('1');
    CPPUNIT_ASSERT(_dc->str().find(" - PER ONE WAY") != std::string::npos);
  }

  void testDisplayFFMileageAppl_2()
  {
    _dc->displayFFMileageAppl('2');
    CPPUNIT_ASSERT(_dc->str().find(" - PER ROUND TRIP") != std::string::npos);
  }

  void testDisplayFFMileageAppl_3()
  {
    _dc->displayFFMileageAppl('3');
    CPPUNIT_ASSERT(_dc->str().find(" - PER CHECKED PORTION") != std::string::npos);
  }

  void testDisplayFFMileageAppl_4()
  {
    _dc->displayFFMileageAppl('4');
    CPPUNIT_ASSERT(_dc->str().find(" - PER BAGGAGE TRAVEL") != std::string::npos);
  }

  void testDisplayFFMileageAppl_5()
  {
    _dc->displayFFMileageAppl('5');
    CPPUNIT_ASSERT(_dc->str().find(" - PER TICKET") != std::string::npos);
  }

  void testDisplayFFMileageAppl_Default()
  {
    _dc->displayFFMileageAppl(' ');
    CPPUNIT_ASSERT_EQUAL(std::string(" "), _dc->str());
  }

  void testPrintTariffCarriers_ToFromUs()
  {
    AirSeg airAa1;
    airAa1.setMarketingCarrierCode("AA");
    AirSeg airAa2;
    airAa2.setMarketingCarrierCode("AA");
    ArunkSeg arunk;
    AirSeg airLh;
    airLh.setMarketingCarrierCode("LOT");

    _itin->travelSeg().clear();
    _itin->travelSeg() += &airAa1, &airAa2, &arunk, &airLh;
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);

    std::string expected = "--------------------------------------------------------------\n"
                           "AA : US TARIFF: YES\n"
                           "LOT: US TARIFF: NO\n";

    _request->ticketingDT() = DateTime(9999, 12, 31);
    _dc->printTariffCarriers(*_trx, *_itin);
    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

  void testPrintTariffCarriers_ToFromCa()
  {
    AirSeg airAa;
    airAa.setMarketingCarrierCode("AA");
    AirSeg airAc;
    airAc.setMarketingCarrierCode("AC");
    ArunkSeg arunk;

    _itin->travelSeg().clear();
    _itin->travelSeg() += &airAa, &airAc, &arunk;
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_CA);

    std::string expected = "--------------------------------------------------------------\n"
                           "AA : CTA TARIFF: NO\n"
                           "AC : CTA TARIFF: YES\n";

    _request->ticketingDT() = DateTime(9999, 12, 31);
    _dc->printTariffCarriers(*_trx, *_itin);
    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

  void testPrintTariffCarriers_BetweenUsCa()
  {
    AirSeg airAa;
    airAa.setMarketingCarrierCode("AA");
    AirSeg airAc;
    airAc.setMarketingCarrierCode("AC");
    AirSeg airLh;
    airLh.setMarketingCarrierCode("LH");

    _itin->travelSeg().clear();
    _itin->travelSeg() += &airAa, &airAc, &airLh;
    _itin->setBaggageTripType(BaggageTripType::BETWEEN_US_CA);

    std::string expected = "--------------------------------------------------------------\n"
                           "AA : US TARIFF: YES\n"
                           "AA : CTA TARIFF: NO\n"
                           "AC : US TARIFF: NO\n"
                           "AC : CTA TARIFF: YES\n"
                           "LH : US TARIFF: NO\n"
                           "LH : CTA TARIFF: NO\n";

    _request->ticketingDT() = DateTime(9999, 12, 31);
    _dc->printTariffCarriers(*_trx, *_itin);
    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

  void testPrintCheckedPoint_AtOrigin()
  {
    std::vector<TravelSeg*> segments;
    segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegDFW_LAX.xml"));

    CheckedPoint cp;
    cp.first = segments.begin();
    cp.second = CP_AT_ORIGIN;

    _dc->printCheckedPoint(cp);
    CPPUNIT_ASSERT_EQUAL(std::string("DFW"), _dc->str());
  }

  void testPrintCheckedPoint_AtDestination()
  {
    std::vector<TravelSeg*> segments;
    segments.push_back(TestAirSegFactory::create(_ut_data_path + "AirSegDFW_LAX.xml"));

    CheckedPoint cp;
    cp.first = segments.begin();
    cp.second = CP_AT_DESTINATION;

    _dc->printCheckedPoint(cp);
    CPPUNIT_ASSERT_EQUAL(std::string("LAX"), _dc->str());
  }

  void testPrintS7ChargesHeader_1stBag()
  {
    _dc->printS7ChargesHeader(0);
    CPPUNIT_ASSERT_EQUAL(
        std::string("-----------------------1ST CHECKED BAG------------------------\n"
                    "V SCODE SERVICE   SEQ NUM  PAX PC/WEIGHT  STATUS\n"),
        _dc->str());
  }

  void testPrintS7ChargesHeader_2ndBag()
  {
    _dc->printS7ChargesHeader(1);
    CPPUNIT_ASSERT_EQUAL(
        std::string("-----------------------2ND CHECKED BAG------------------------\n"
                    "V SCODE SERVICE   SEQ NUM  PAX PC/WEIGHT  STATUS\n"),
        _dc->str());
  }

  void testPrintTable196DetailSetup_NoData()
  {
    _dc->printTable196DetailSetup(1, NULL, *_farePath, *_trx, true);
    CPPUNIT_ASSERT(!_dc->_displayT196);

    OptionalServicesInfo* s7 = createS7Record();
    _dc->printTable196DetailSetup(1, s7, *_farePath, *_trx, true);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196DetailSetup_seqNo()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 155;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->printTable196DetailSetup(checkedPortion, s7, *_farePath, *_trx, true);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196DetailSetup_cp()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 5;

    _dc->printTable196DetailSetup(checkedPortion, s7, *_farePath, *_trx, true);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196DetailSetup_fl()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = 7;
    uint32_t checkedPortion = 1;

    _dc->printTable196DetailSetup(checkedPortion, s7, *_farePath, *_trx, true);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196DetailSetup_allOK()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->printTable196DetailSetup(checkedPortion, s7, *_farePath, *_trx, true);
    CPPUNIT_ASSERT(_dc->_displayT196);
  }

  void testPrintTable196ForBaggageDetailSetup_baggageAllowanceDiag()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::DISPLAY_CHARGES);
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::CARRY_ON_ALLOWANCE);
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::CARRY_ON_CHARGES);
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::EMBARGOES);

    _dc->printTable196ForBaggageDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(_dc->_displayT196);
  }

  void testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag1()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_CHARGES] = "";
    _dc->printTable196ForBaggageDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag2()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::CARRY_ON_ALLOWANCE] = "";
    _dc->printTable196ForBaggageDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag3()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::CARRY_ON_CHARGES] = "";
    _dc->printTable196ForBaggageDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196ForBaggageDetailSetup_notBaggageAllowanceDiag4()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::EMBARGOES] = "";
    _dc->printTable196ForBaggageDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196ForCarryOnDetailSetup_carryOnAllowanceDiag1()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::CARRY_ON_ALLOWANCE] = "";
    _dc->printTable196ForCarryOnDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(_dc->_displayT196);
  }

  void testPrintTable196ForCarryOnDetailSetup_carryOnAllowanceDiag2()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::DISPLAY_CHARGES);
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::CARRY_ON_ALLOWANCE);
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::CARRY_ON_CHARGES);
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::EMBARGOES);

    _dc->printTable196ForCarryOnDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(_dc->_displayT196);
  }

  void testPrintTable196ForCarryOnDetailSetup_notCarryOnAllowanceDiag1()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::CARRY_ON_CHARGES] = "";

    _dc->printTable196ForCarryOnDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196ForCarryOnDetailSetup_notCarryOnAllowanceDiag2()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::EMBARGOES] = "";

    _dc->printTable196ForCarryOnDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void testPrintTable196ForCarryOnDetailSetup_notCarryOnAllowanceDiag3()
  {
    initPrintTable196DetailSetup();
    OptionalServicesInfo* s7 = createS7Record();
    s7->seqNo() = 152;
    _farePath->paxType()->inputOrder() = (_dc->fareLine() - 1);
    uint32_t checkedPortion = 1;

    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_CHARGES] = "";

    _dc->printTable196ForCarryOnDetailSetup(checkedPortion, s7, *_farePath, *_trx);
    CPPUNIT_ASSERT(!_dc->_displayT196);
  }

  void initPrintTable196DetailSetup()
  {
    _dcParamTester->updateCheckedPortion(1);
    _dcParamTester->updateFareLine(2);

    _farePath->paxType() = _memHandle.create<PaxType>();
    _dc->rootDiag()->diagParamMap()[Diagnostic::SEQ_NUMBER] = "152";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
  }

  void test_printFareCheckInfo_FAACTIVE()
  {
    _dcParamTester->updateDiagType(Diag852Collector::FAACTIVE);
    _dc->activate();

    CarrierCode carrier = "LH";
    std::string fareCheckStatus = "PASS";
    std::string paxFareBasis = "basis";
    bool isSelected = false;
    MoneyAmount moneyAmount = 100;
    CurrencyCode currency = "PLN";

    std::stringstream expected;
    expected << std::setw(Diag852Collector::CARRIER_LENGTH) << carrier
             << std::setw(Diag852Collector::FARE_BASIS_LENGTH) << paxFareBasis
             << std::setw(Diag852Collector::STATUS_LENGTH) << fareCheckStatus
             << std::setw(Diag852Collector::AMOUNT_LENGTH) << moneyAmount
             << std::setw(Diag852Collector::CURRENCY_LENGTH) << currency
             << std::setw(Diag852Collector::SELECTION_MARK_LENGTH) << " "
             << "\n";

    _dc->printFareCheckInfo(
        carrier, fareCheckStatus, paxFareBasis, moneyAmount, currency, isSelected);
    CPPUNIT_ASSERT_EQUAL(expected.str(), _dc->str());
  }

  void test_printFareCheckInfo_NOT_FAACTIVE()
  {
    _dcParamTester->updateDiagType(Diag852Collector::BASIC);
    _dc->activate();

    CarrierCode carrier = "LH";
    std::string fareCheckStatus = "PASS";
    std::string paxFareBasis = "basis";
    bool isSelected = false;
    MoneyAmount moneyAmount = 100;
    CurrencyCode currency = "PLN";

    _dc->printFareCheckInfo(
        carrier, fareCheckStatus, paxFareBasis, moneyAmount, currency, isSelected);

    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());
  }

  void test_printFareCheckInfo_check_precision()
  {
    std::streamsize oldPrecision = _dc->precision();
    _dcParamTester->updateDiagType(Diag852Collector::BASIC);
    _dc->activate();

    CarrierCode carrier = "LH";
    std::string fareCheckStatus = "PASS";
    std::string paxFareBasis = "basis";
    bool isSelected = false;
    MoneyAmount moneyAmount = 100;
    CurrencyCode currency = "PLN";

    _dc->printFareCheckInfo(
        carrier, fareCheckStatus, paxFareBasis, moneyAmount, currency, isSelected);

    CPPUNIT_ASSERT_EQUAL(_dc->precision(), oldPrecision);
  }

  void test_printS7OptionalServiceStatus_FAACTIVE()
  {
    _dcParamTester->updateDiagType(Diag852Collector::FAACTIVE);
    _dc->activate();
    _dc->printS7OptionalServiceStatus(PASS_S7);
    std::string expectedResponse = "";
    CPPUNIT_ASSERT_EQUAL(expectedResponse, _dc->str());
  }

  void test_printS7OptionalServiceStatus_NOT_FAACTIVE()
  {
    _dcParamTester->updateDiagType(Diag852Collector::BASIC);
    _dc->activate();
    _dc->printS7OptionalServiceStatus(PASS_S7);
    CPPUNIT_ASSERT(_dc->str().find("S7 STATUS : PASS\n") != std::string::npos);
  }

  void test_S7printerBase_printS7ProcessingHeader()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("XXX", "YYY");
    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());
    _farePath->paxType() = _memHandle.create<PaxType>();
    bt->setPaxType(*_farePath->paxType());

    S7PrinterMock printer(*_dc);
    printer.printS7ProcessingHeader(*_trx, bt);

    CPPUNIT_ASSERT(
        _dc->str().find("------------------S7 RECORD DATA PROCESSING-------------------\n") !=
        std::string::npos);

    CPPUNIT_ASSERT(
        _dc->str().find("--------------------------------------------------------------\n") !=
        std::string::npos);
  }

  void test_S7printerBase_printPTC()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("XXX", "YYY");
    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());
    _farePath->paxType() = _memHandle.create<PaxType>();
    bt->setPaxType(*_farePath->paxType());

    std::stringstream expected;
    expected << "PTC:" << bt->paxType()->paxType() << " TRAVEL DATE: "
             << (*(bt->getTravelSegBegin()))->departureDT().dateToString(DDMMMYY, "")
             << " RETRIEVAL DATE: " << bt->_trx->ticketingDate().dateToString(DDMMMYY, "");

    S7PrinterMock printer(*_dc);
    printer.printPTC(*_trx, bt);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printCorporateId_notEmptyCorpIdVector()
  {
    _trx->getRequest()->corpIdVec().push_back("xxx");
    _trx->getRequest()->corpIdVec().push_back("yyy");
    _trx->getRequest()->corpIdVec().push_back("zzz");

    std::stringstream expected;
    expected << "\nCORPORATE ID:";

    for (const std::string& corpId : _trx->getRequest()->corpIdVec())
    {
      expected << " " << corpId;
    }

    S7PrinterMock printer(*_dc);
    printer.printCorporateId(*_trx);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printCorporateId_emptyCorpIdVector()
  {
    _trx->getRequest()->corpIdVec().clear();

    std::stringstream expected;
    expected << "\nCORPORATE ID:"
             << " " << _trx->getRequest()->corporateID();

    S7PrinterMock printer(*_dc);
    printer.printCorporateId(*_trx);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printAccountCode_notEmptyAccountCodeVector()
  {
    _trx->getRequest()->accCodeVec().push_back("xxx");
    _trx->getRequest()->accCodeVec().push_back("yyy");

    std::stringstream expected;

    expected << "\nACCOUNT CODE:";
    for (const std::string& accCode : _trx->getRequest()->accCodeVec())
    {
      expected << " " << accCode;
    }

    S7PrinterMock printer(*_dc);
    printer.printAccountCode(*_trx);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printAccountCode_emptyAccountCodeVector()
  {
    _trx->getRequest()->accCodeVec().clear();

    std::stringstream expected;
    expected << "\nACCOUNT CODE:"
             << " " << _trx->getRequest()->accountCode();

    S7PrinterMock printer(*_dc);
    printer.printAccountCode(*_trx);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printInputDesignator()
  {
    _trx->getRequest()->tktDesignator()[1] = "xxx";
    _trx->getRequest()->tktDesignator()[2] = "yyy";

    std::stringstream expected;
    expected << "\nTKT DESIG INPUT: ";
    typedef const std::pair<int16_t, TktDesignator> TktDes;
    for (TktDes& tktDes : _trx->getRequest()->tktDesignator())
    {
      expected << " " << tktDes.second;
    }

    S7PrinterMock printer(*_dc);
    printer.printInputDesignator(*_trx);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printOutputDesignator()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("XXX", "YYY");
    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());
    bt->farePath()->pricingUnit().clear();

    S7PrinterMock printer(*_dc);
    printer.printOutputDesignator(*_trx, bt);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("\nTKT DESIG OUTPUT:"));
  }

  void test_S7printerBase_printTourCode_cat35()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("XXX", "YYY");
    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());

    bt->farePath()->collectedNegFareData() = _memHandle.create<CollectedNegFareData>();
    bt->farePath()->collectedNegFareData()->indicatorCat35() = true;
    bt->farePath()->collectedNegFareData()->tourCode() = "xxx";

    S7PrinterMock printer(*_dc);
    printer.printTourCode(bt);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("\nTOUR CODE: xxx"));
  }

  void test_S7printerBase_printTourCode_cat27()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("XXX", "YYY");
    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());

    bt->farePath()->cat27TourCode() = "yyy";

    S7PrinterMock printer(*_dc);
    printer.printTourCode(bt);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("\nTOUR CODE: yyy"));
  }

  void test_S7printerBase_printDayOfWeek()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("XXX", "YYY");
    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());

    std::stringstream expected;
    expected << "\nDAY OF WEEK: " << (*(bt->getTravelSegBegin()))->departureDT().dayOfWeek()
             << " START/STOP TIME: "
             << (*(bt->getTravelSegBegin()))->departureDT().dateToString(YYYYMMDD, "-") << " ";

    if ((*(bt->getTravelSegBegin()))->segmentType() == Open)
      expected << "00-00-00";
    else
      expected << (*(bt->getTravelSegBegin()))->departureDT().timeToString(HHMMSS, "-");

    S7PrinterMock printer(*_dc);
    printer.printDayOfWeek(bt);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7printerBase_printEquipment()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("aaa", "bbb");
    travelSegs.back()->equipmentType() = "eq1";

    travelSegs += createAirSeg("ccc", "ddd");
    travelSegs.back()->equipmentType() = "eq2";

    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());

    std::stringstream expected;
    expected << "\nEQUIPMENT:";
    for (const TravelSeg* tvlSeg : travelSegs)
    {
      expected << " " << tvlSeg->equipmentType();
    }
    expected << "\n";

    S7PrinterMock printer(*_dc);
    printer.printEquipment(bt);

    CPPUNIT_ASSERT_EQUAL(_dc->str(), expected.str());
  }

  void test_S7PrinterDdinfo_printTravelInfoForAllSectors_testFieldsHeaders()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += createAirSeg("aaa", "bbb");
    travelSegs += createAirSeg("mss", "mss");
    travelSegs += createAirSeg("ccc", "ddd");

    BaggageTravel* bt = createBaggageTravel(travelSegs.begin(), travelSegs.end());

    Diag852Collector::S7PrinterDdinfo printer(*_dc);
    printer.printTravelInfoForAllSectors(
        travelSegs.begin(), travelSegs.end(), travelSegs.begin() + 1, false, bt, Ts2ss());

    std::string result = _dc->str();

    CPPUNIT_ASSERT(result.find("\nCABIN: ") != std::string::npos);
    CPPUNIT_ASSERT(result.find("\nRBD:") != std::string::npos);
    CPPUNIT_ASSERT(result.find("\nFARE BASIS:") != std::string::npos);
    CPPUNIT_ASSERT(result.find("\nTARIFF:") != std::string::npos);
    CPPUNIT_ASSERT(result.find("\nRULE:") != std::string::npos);
    CPPUNIT_ASSERT(result.find("\nFLIGHT APPLICATION:\n") != std::string::npos);
  }

  void test_S7PrinterDdinfo_printS7RecordValidationFooter_t198Present()
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->serviceFeesResBkgDesigTblItemNo() = 1;

    Diag852Collector::S7PrinterDdinfo printer(*_dc);
    printer.printS7RecordValidationFooter(*s7);

    std::string result = _dc->str();
    std::string expected = "* USE BTA QUALIFIER FOR DETAILED PROCESSING\n\n";

    CPPUNIT_ASSERT_EQUAL(result, expected);
  }

  void test_S7PrinterDdinfo_printS7RecordValidationFooter_t171Present()
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->serviceFeesCxrResultingFclTblItemNo() = 1;

    Diag852Collector::S7PrinterDdinfo printer(*_dc);
    printer.printS7RecordValidationFooter(*s7);

    std::string result = _dc->str();
    std::string expected = "* USE BTA QUALIFIER FOR DETAILED PROCESSING\n\n";

    CPPUNIT_ASSERT_EQUAL(result, expected);
  }

  void test_S7PrinterDdinfo_printS7RecordValidationFooter_t173Present()
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->resultServiceFeesTktDesigTblItemNo() = 1;

    Diag852Collector::S7PrinterDdinfo printer(*_dc);
    printer.printS7RecordValidationFooter(*s7);

    std::string result = _dc->str();
    std::string expected = "* USE BTA QUALIFIER FOR DETAILED PROCESSING\n\n";

    CPPUNIT_ASSERT_EQUAL(result, expected);
  }

  void test_S7PrinterDdinfo_printS7RecordValidationFooter_t186Present()
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->carrierFltTblItemNo() = 1;

    Diag852Collector::S7PrinterDdinfo printer(*_dc);
    printer.printS7RecordValidationFooter(*s7);

    std::string result = _dc->str();
    std::string expected = "* USE BTA QUALIFIER FOR DETAILED PROCESSING\n\n";

    CPPUNIT_ASSERT_EQUAL(result, expected);
  }

  void test_S7PrinterDdinfo_printS7RecordValidationFooter_emptyText()
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();

    s7->serviceFeesResBkgDesigTblItemNo() = 0;
    s7->serviceFeesCxrResultingFclTblItemNo() = 0;
    s7->resultServiceFeesTktDesigTblItemNo() = 0;
    s7->carrierFltTblItemNo() = 0;

    Diag852Collector::S7PrinterDdinfo printer(*_dc);
    printer.printS7RecordValidationFooter(*s7);

    std::string result = _dc->str();
    std::string expected = "";

    CPPUNIT_ASSERT_EQUAL(result, expected);
  }

  void test_validContextForBTA_rbd_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(_dc->validContextForBTA(Diag877Collector::PROCESSING_RBD));
  }

  void test_validContextForBTA_rbd_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(!_dc->validContextForBTA(Diag877Collector::PROCESSING_RBD));
  }

  void test_validContextForBTA_t171_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(_dc->validContextForBTA(Diag877Collector::PROCESSING_T171));
  }

  void test_validContextForBTA_t171_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(!_dc->validContextForBTA(Diag877Collector::PROCESSING_T171));
  }

  void test_validContextForBTA_outTicketDesig_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(
        _dc->validContextForBTA(Diag877Collector::PROCESSING_OUTPUT_TICKET_DESIG));
  }

  void test_validContextForBTA_outTicketDesig_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(
        !_dc->validContextForBTA(Diag877Collector::PROCESSING_OUTPUT_TICKET_DESIG));
  }

  void test_validContextForBTA_t186_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(_dc->validContextForBTA(Diag877Collector::PROCESSING_T186));
  }

  void test_validContextForBTA_t186_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(!_dc->validContextForBTA(Diag877Collector::PROCESSING_T186));
  }

  void test_validContextForBTA_accountCodes_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(
        _dc->validContextForBTA(Diag877Collector::PROCESSING_ACCOUNT_CODES));
  }

  void test_validContextForBTA_accountCodes_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(
        !_dc->validContextForBTA(Diag877Collector::PROCESSING_ACCOUNT_CODES));
  }

  void test_validContextForBTA_inputTicketDesig_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(
        _dc->validContextForBTA(Diag877Collector::PROCESSING_INPUT_TICKET_DESIG));
  }

  void test_validContextForBTA_inputTicketDesig_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(
        !_dc->validContextForBTA(Diag877Collector::PROCESSING_INPUT_TICKET_DESIG));
  }

  void test_validContextForBTA_t170_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(_dc->validContextForBTA(Diag877Collector::PROCESSING_T170));
  }

  void test_validContextForBTA_t170_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(!_dc->validContextForBTA(Diag877Collector::PROCESSING_T170));
  }

  void test_validContextForBTA_t183_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(_dc->validContextForBTA(Diag877Collector::PROCESSING_T183));
  }
  void test_validContextForBTA_t183_no()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(!_dc->validContextForBTA(Diag877Collector::PROCESSING_T183));
  }

  void test_isBTAContextOn_yes()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(_dc->isBTAContextOn());
  }

  void test_isBTAContextOn_no_noDDINFO()
  {
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::DISPLAY_DETAIL);
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    CPPUNIT_ASSERT(!_dc->isBTAContextOn());
  }

  void test_isBTAContextOn_no_noBTA()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap().erase(Diag852Collector::BTA);

    CPPUNIT_ASSERT(!_dc->isBTAContextOn());
  }

  void test_printBTASegmentHeader()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    AirSeg* segment = _memHandle.create<AirSeg>();
    segment->origAirport() = "XXX";
    segment->destAirport() = "YYY";

    _dc->printBTASegmentHeader(*segment);
    CPPUNIT_ASSERT_EQUAL(
        _dc->str(),
        std::string("-------------------------SECTOR XXX-YYY----------------------------\n"));
  }

  void test_printBTASegmentFooter_pass()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    AirSeg* segment = _memHandle.create<AirSeg>();
    segment->origAirport() = "XXX";
    segment->destAirport() = "YYY";

    _dc->printBTASegmentFooter(true, *segment);
    CPPUNIT_ASSERT_EQUAL(
        _dc->str(),
        std::string("                                           S7 STATUS : PASS XXX-YYY\n"));
  }

  void test_printBTASegmentFooter_fail()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    AirSeg* segment = _memHandle.create<AirSeg>();
    segment->origAirport() = "XXX";
    segment->destAirport() = "YYY";

    _dc->printBTASegmentFooter(false, *segment);
    CPPUNIT_ASSERT_EQUAL(
        _dc->str(),
        std::string("                                           S7 STATUS : FAIL XXX-YYY\n"));
  }

  void test_printBTAFieldStatusCabin_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusCabin(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("CABIN : PASS\n"));
  }

  void test_printBTAFieldStatusCabin_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusCabin(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("CABIN : FAIL\n"));
  }

  void test_printBTAStatusTableT171_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAStatusTableT171(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("CXR/RESULT FARE CLASS T171 : PASS\n"));
  }

  void test_printBTAStatusTableT171_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAStatusTableT171(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("CXR/RESULT FARE CLASS T171 : FAIL\n"));
  }

  void test_printBTAFieldStatusRule_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusRule(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("RULE : PASS\n"));
  }

  void test_printBTAFieldStatusRule_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusRule(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("RULE : FAIL\n"));
  }

  void test_printBTAFieldStatusRuleTariff_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusRuleTariff(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("RULE TARIFF : PASS\n"));
  }

  void test_printBTAFieldStatusRuleTariff_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusRuleTariff(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("RULE TARIFF : FAIL\n"));
  }

  void test_printBTAFieldStatusFareInd_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusFareInd(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("FARE IND : PASS\n"));
  }

  void test_printBTAFieldStatusFareInd_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusFareInd(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("FARE IND : FAIL\n"));
  }

  void test_printBTAFieldStatusCarrierFlightApplT186_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusCarrierFlightApplT186(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("CXR/FLT T186 : PASS\n"));
  }

  void test_printBTAFieldStatusCarrierFlightApplT186_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAFieldStatusCarrierFlightApplT186(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("CXR/FLT T186 : FAIL\n"));
  }

  void testPrintMileageTpmNoData()
  {
    std::vector<Mileage*> mil;
    _dc->printMileage("AAA", "BBB", mil, {GlobalDirection::AT}, 'T');
    CPPUNIT_ASSERT_EQUAL(
        std::string("TPM ANALYSIS FOR CITY PAIR AAA BBB GD-AT \n"
                    "NO DATA\n"
                    "--------------------------------------------------------------\n"),
        _dc->str());
  }

  void testPrintMileageMpmNoData()
  {
    std::vector<Mileage*> mil;
    _dc->printMileage("AAA", "BBB", mil, {GlobalDirection::AT}, 'M');
    CPPUNIT_ASSERT_EQUAL(
        std::string("MPM ANALYSIS FOR CITY PAIR AAA BBB GD-AT \n"
                    "NO DATA\n"
                    "GREAT CIRCLE MILEAGE WILL BE USED\n"
                    "--------------------------------------------------------------\n"),
        _dc->str());
  }

  void test_printBTAStatusOutputTicketDesignator_match()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAStatusOutputTicketDesignator(true);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("FARE TICKT DESIGNATOR T173 : PASS\n"));
  }

  void test_printBTAStatusOutputTicketDesignator_noMatch()
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dc->rootDiag()->diagParamMap()[Diag852Collector::BTA] = "A";

    _dc->printBTAStatusOutputTicketDesignator(false);
    CPPUNIT_ASSERT_EQUAL(_dc->str(), std::string("FARE TICKT DESIGNATOR T173 : FAIL\n"));
  }

  void testPrintMileageMpmMatched()
  {
    std::vector<Mileage*> mil;
    Mileage m;
    m.globaldir() = GlobalDirection::AT;
    m.mileage() = 120;
    mil.push_back(&m);
    _dc->printMileage("AAA", "BBB", mil, {GlobalDirection::AT, GlobalDirection::PA}, 'M');
    CPPUNIT_ASSERT_EQUAL(
        std::string("MPM ANALYSIS FOR CITY PAIR AAA BBB GD-AT PA \n"
                    "AT                 120/100 GLOBAL DIRECTION MATCHED\n"
                    "--------------------------------------------------------------\n"),
        _dc->str());
  }

  void testGetCarrierListAsString()
  {
    std::set<CarrierCode> carriers = {"AA", "BB", "CC", "DD", "EE", "FF", "GG", "HH", "II", "JJ",
                                      "KK", "LL", "MM", "NN" , "OO", "PP"};
    CPPUNIT_ASSERT_EQUAL(std::string("AA,BB,CC,DD,EE,FF,GG,HH,II,JJ,KK,LL,MM,\n                      NN,OO,PP"),
                         _dc->getCarrierListAsString(carriers, ","));
  }

  void setSubTypeCode(const std::string& fareLine, const std::string& checkedPortion,
                      const std::string& displayCharges, const std::string& subTypeCode)
  {
    _dc->rootDiag()->diagParamMap()[Diag852Collector::FARE_LINE] = fareLine;
    _dc->rootDiag()->diagParamMap()[Diag852Collector::CHECKED_PORTION] = checkedPortion;
    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_CHARGES] = displayCharges;
    _dc->rootDiag()->diagParamMap()[Diag852Collector::SUB_TYPE_CODE] = subTypeCode;
  }

  void testPrintDetailInterlineEmdProcessingS5Info()
  {
    _dc->activate();
    std::set<CarrierCode> marketingCarriers = {"M1", "M2", "M3"};
    std::set<CarrierCode> operatingCarriers = {"O1", "O2", "O3", "O4"};
    _dc->printDetailInterlineEmdProcessingS5Info("PL", "GDS", "VA", marketingCarriers, operatingCarriers);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dcParamTester->forceInitialization();
    _dc->printDetailInterlineEmdProcessingS5Info("PL", "GDS", "VA", marketingCarriers, operatingCarriers);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    setSubTypeCode("01", "01", "", "0GO");
    _dcParamTester->forceInitialization();
    _dc->printDetailInterlineEmdProcessingS5Info("PL", "GDS", "VA", marketingCarriers, operatingCarriers);
    CPPUNIT_ASSERT_EQUAL(std::string(
      "--------- INTERLINE EMD AGREEMENT PROCESSING DETAILS ---------\n"
      " NATION             : PL\n"
      " GDS                : GDS\n"
      " VALIDATING CXR     : VA\n"
      " MARKETING  CXR     : M1 M2 M3\n"
      " OPERATING  CXR     : O1 O2 O3 O4\n"),
      _dc->str());
  }

  void testPrintNoInterlineDataFoundInfo()
  {
    _dc->activate();
    _dc->printNoInterlineDataFoundInfo();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dcParamTester->forceInitialization();
    _dc->printNoInterlineDataFoundInfo();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    setSubTypeCode("01", "01", "", "0GO");
    _dcParamTester->forceInitialization();
    _dc->printNoInterlineDataFoundInfo();
    CPPUNIT_ASSERT_EQUAL(std::string("--------- NO INTERLINE EMD AGREEMENT DATA FOUND ---------\n"), _dc->str());
  }

  void testPrintDetailInterlineEmdAgreementInfo()
  {
    _dc->activate();
    EmdInterlineAgreementInfo info1, info2, info3, info4;
    info1.setParticipatingCarrier("P1");
    info2.setParticipatingCarrier("P2");
    info3.setParticipatingCarrier("P3");
    info4.setParticipatingCarrier("P4");

    std::vector<EmdInterlineAgreementInfo*> emdAgreementInfoVector
      {&info1, &info2, &info3, &info4, &info1, &info2, &info3, &info4,
       &info1, &info2, &info3, &info4, &info1, &info2, &info3, &info4 };

    CarrierCode validatingCarrier("VC");

    _dc->printDetailInterlineEmdAgreementInfo(emdAgreementInfoVector, validatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    setSubTypeCode("01", "01", "", "0GO");

    _dcParamTester->forceInitialization();
    _dc->printDetailInterlineEmdAgreementInfo(emdAgreementInfoVector, validatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->rootDiag()->diagParamMap()[Diag852Collector::DISPLAY_DETAIL] = "INFO";
    _dcParamTester->forceInitialization();
    _dc->printDetailInterlineEmdAgreementInfo(emdAgreementInfoVector, validatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->rootDiag()->diagParamMap()[Diagnostic::ALL_VALID] = "EMDIA";
    _dcParamTester->forceInitialization();
    _dc->printDetailInterlineEmdAgreementInfo(emdAgreementInfoVector, validatingCarrier);
    CPPUNIT_ASSERT_EQUAL(std::string(" ALLOWED    CXR     : VC P1 P2 P3 P4 P1 P2 P3 "
                                     "P4 P1 P2 P3 P4 \n                      P1 P2 P3 P4 \n"), _dc->str());
  }

  void testPrintEmdValidationResultNotMatchingCharge()
  {
    setSubTypeCode("01", "01", "", "0DF");
    _dc->activate();

    BaggageCharge* charge = createBaggageCharge(1, "PLN");
    BaggageTravel* baggageTravel = _memHandle.create<BaggageTravel>();
    baggageTravel->_chargeVector.push_back(charge);
    BaggageTravelInfo bagInfo(0, 0);

    _dc->printEmdValidationResult(true, AncRequestPath::AncCheckInPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    SubCodeInfo* subCodeInfo = createS5Record("AABB");
    subCodeInfo->serviceSubTypeCode() = "0DF";
    subCodeInfo->emdType() = '4';
    charge->subCodeInfo() = subCodeInfo;

    _dc->printEmdValidationResult(true, AncRequestPath::AncCheckInPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->printEmdValidationResult(false, AncRequestPath::AncCheckInPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->printEmdValidationResult(true, AncRequestPath::AncReservationPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());

    _dc->printEmdValidationResult(false, AncRequestPath::AncReservationPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _dc->str());
  }

  void testPrintEmdValidationResultCKI()
  {
    setSubTypeCode("01", "01", "", "0DF");
    _dc->activate();

    BaggageCharge* charge = createBaggageCharge(1, "PLN");
    SubCodeInfo* subCodeInfo = createS5Record("AABB");
    BaggageTravelInfo bagInfo(0, 0);

    subCodeInfo->serviceSubTypeCode() = "0DF";
    subCodeInfo->emdType() = '3';
    charge->subCodeInfo() = subCodeInfo;

    BaggageTravel* baggageTravel = _memHandle.create<BaggageTravel>();
    baggageTravel->_chargeVector.push_back(charge);

    _dc->printEmdValidationResult(true, AncRequestPath::AncCheckInPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS          : I-EMD PASS\n"), _dc->str());

    _dc->str("");
    _dc->printEmdValidationResult(false, AncRequestPath::AncCheckInPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS          : I-EMD FAILED/SOFT PASS\n"), _dc->str());
  }

  void testPrintEmdValidationResultRES()
  {
    setSubTypeCode("01", "01", "", "0DF");
    _dc->activate();

    BaggageCharge* charge = createBaggageCharge(1, "PLN");
    BaggageTravelInfo bagInfo(0, 0);
    SubCodeInfo* subCodeInfo = createS5Record("AABB");
    subCodeInfo->serviceSubTypeCode() = "0DF";
    subCodeInfo->emdType() = '3';
    charge->subCodeInfo() = subCodeInfo;

    BaggageTravel* baggageTravel = _memHandle.create<BaggageTravel>();
    baggageTravel->_chargeVector.push_back(charge);

    _dc->printEmdValidationResult(true, AncRequestPath::AncReservationPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS          : I-EMD PASS\n"), _dc->str());

    _dc->str("");
    _dc->printEmdValidationResult(false, AncRequestPath::AncReservationPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS          : I-EMD FAILED \n"), _dc->str());

    _dc->str("");
    baggageTravel->_charges[0] = charge;
    _dc->printEmdValidationResult(false, AncRequestPath::AncReservationPath, baggageTravel, bagInfo);
    CPPUNIT_ASSERT_EQUAL(std::string(" S5 STATUS          : I-EMD AGREEMENT NOT CHECKED - SDO/FDO\n"), _dc->str());
  }

  void testPrintInfoAboutLackingBaggageCharges()
  {
    _dc->rootDiag()->diagParamMap().insert(std::make_pair(Diag852Collector::DISPLAY_CHARGES, ""));

    std::vector<TravelSeg*> travelSegs;
    const LocCode locKrk = LocCode("KRK");
    const LocCode locLon = LocCode("LON");
    const LocCode locJfk = LocCode("JFK");

    travelSegs.push_back(createAirSeg(locKrk, locLon));
    travelSegs.push_back(createAirSeg(locLon, locJfk));

    BaggageTravel* baggageTravel = createBaggageTravel(travelSegs.begin(), travelSegs.end());
    OptionalServicesInfo* optFee = _memHandle.create<OptionalServicesInfo>();
    optFee->freeBaggagePcs() = 1;
    baggageTravel->_allowance =  _memHandle.create<OCFees>();
    baggageTravel->_allowance->optFee() = optFee;
    baggageTravel->_charges[1] = _memHandle.create<BaggageCharge>();

    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels.push_back(baggageTravel);

    std::vector<FarePath*> farePath;
    farePath.push_back(_memHandle.create<FarePath>());
    farePath.front()->baggageTravels() = baggageTravels;

    Itin* itin1 = _memHandle.create<Itin>();
    itin1->itinNum() = 1;
    itin1->errResponseCode() = ErrorResponseException::UNKNOWN_BAGGAGE_CHARGES;
    itin1->farePath() = farePath;

    Itin *itin2 = _memHandle.create<Itin>();
    itin2->itinNum() = 2;

    Itin *itin3 = _memHandle.create<Itin>();
    itin3->errResponseCode() = ErrorResponseException::UNKNOWN_BAGGAGE_CHARGES;
    itin3->itinNum() = 3;
    itin3->farePath() = farePath;

    std::vector<Itin*> itins;
    itins.push_back(itin1);
    itins.push_back(itin2);
    itins.push_back(itin3);

    PricingTrx trx;
    trx.itin() = itins;
    trx.mutableBaggagePolicy().setRequestedBagPieces(4);

    _dc->printInfoAboutUnknownBaggageCharges(trx);
    CPPUNIT_ASSERT_EQUAL(std::string("--------------------- FAILED ITINERARY: 1 --------------------\n"
                                     "BAGGAGE TRAVEL: KRK 0 - LON 0,0 - JFK 0\n"
                                     "FREE BAGGAGE PIECES: 1\n"
                                     "ALL BAGGAGE PIECES: 2\n"
                                     "2 BAGGAGE CHARGES UNKNOWN\n"
                                     "--------------------- FAILED ITINERARY: 3 --------------------\n"
                                     "BAGGAGE TRAVEL: KRK 0 - LON 0,0 - JFK 0\n"
                                     "FREE BAGGAGE PIECES: 1\n"
                                     "ALL BAGGAGE PIECES: 2\n"
                                     "2 BAGGAGE CHARGES UNKNOWN\n"
                                     "--------------------------------------------------------------\n"),
                                     _dc->str());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag852CollectorTest);
} // tse
