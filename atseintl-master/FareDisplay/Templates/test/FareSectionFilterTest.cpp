#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/Templates/FaresSectionFilter.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"

#include "FareDisplay/CabinGroupComparator.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupHeader.h"
#include "FareDisplay/S8BrandComparator.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace std;

namespace tse
{
class FaresSectionFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FaresSectionFilterTest);

  CPPUNIT_TEST(testInitialize);
  CPPUNIT_TEST(testAddS8BrandSectionText_Match_First_ProgBrand);
  CPPUNIT_TEST(testAddS8BrandSectionText_Match_Second_ProgBrand);
  CPPUNIT_TEST(testAddS8BrandSectionText_Match_Third_ProgBrand);
  CPPUNIT_TEST(testDoFiltering);

  CPPUNIT_TEST(testInitialize_with_Cabin);
  CPPUNIT_TEST(testAddCabinSectionText_real_CabinInclusionCode);
  CPPUNIT_TEST(testAddCabinSectionText_wrong_CabinInclusionCode);
  CPPUNIT_TEST(testDoFiltering_Cabin);

  CPPUNIT_TEST(testDisplayYYHdrMsgForMileageRequest);
  CPPUNIT_TEST(testDisplayYYHdrMsgForNotPublishedAndSingleInclusionCode);
  CPPUNIT_TEST(testDisplayYYHdrMsgForNotPublishedAndMultipleInclusionCode);
  CPPUNIT_TEST(testDisplayYYHdrMsgForNotPublishedAndABInclusionCode);
  CPPUNIT_TEST(testDisplayYYHdrMsgForNotPermittedAndSingleInclusionCode);
  CPPUNIT_TEST(testDisplayYYHdrMsgForNotPermittedAndMultipleInclusionCode);
  CPPUNIT_TEST(testDisplayYYHdrMsgForNotPermittedAndABInclusionCode);

  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _fTrx = _memHandle.create<FareDisplayTrx>();
    _fTrx->fdResponse() = _memHandle.create<FareDisplayResponse>();
    _fTrx->setRequest(_memHandle.create<FareDisplayRequest>());
    _fTrx->setOptions(_memHandle.create<FareDisplayOptions>());
    _tSeg = _memHandle.create<AirSeg>();
    _fTrx->travelSeg().push_back(_tSeg);
    _tSeg->departureDT() = DateTime(2013, 3, 31);
    _itin = _memHandle.create<Itin>();
    _itin->geoTravelType() = GeoTravelType::International;
    _fTrx->itin().push_back(_itin);

    _group = _memHandle.create<Group>();
    _market = _memHandle.create<FareMarket>();
    _market->governingCarrier() = "AA";
    _market->boardMultiCity() = "DFW";
    _market->offMultiCity() = "JFK";
    _fTrx->fareMarket().push_back(_market);
    _itin->fareMarket().push_back(_market);
  }
  void tearDown() { _memHandle.clear(); }

  void buildProgramBrandMap()
  {
    std::vector<OneProgramOneBrand*> spbVec;
    OneProgramOneBrand* onepb1 = _memHandle.create<OneProgramOneBrand>();
    OneProgramOneBrand* onepb2 = _memHandle.create<OneProgramOneBrand>();
    OneProgramOneBrand* onepb3 = _memHandle.create<OneProgramOneBrand>();
    spbVec.push_back(onepb1);
    spbVec.push_back(onepb2);
    spbVec.push_back(onepb3);

    onepb1->carrier() = "AA";
    onepb1->programCode() = "US";
    onepb1->programName() = "DOMESTIC US";
    onepb1->brandCode() = "APP";
    onepb1->brandName() = "Apple";
    onepb1->tier() = 99;
    onepb1->passengerType().push_back("RUG");

    onepb2->carrier() = "AA";
    onepb2->programCode() = "US";
    onepb2->programName() = "DOMESTIC US";
    onepb2->brandCode() = "ABB";
    onepb2->brandName() = "Abbreviate";
    onepb2->tier() = 10;
    onepb2->passengerType().push_back("ALK");

    onepb3->carrier() = "AA";
    onepb3->programCode() = "XX";
    onepb3->programName() = "FLIGHT ANYWHERE";
    onepb3->brandCode() = "YYY";
    onepb3->brandName() = "Forever";
    onepb3->tier() = 1;
    onepb3->passengerType().push_back("FRR");
    _group->programBrandMap().insert(std::make_pair("AA", spbVec));
  }

  void prepareGroupHeader()
  {
    GroupHeader header(*_fTrx);
    header.setS8BrandHeader();
  }
  void prepareCabinGroupHeader()
  {
    GroupHeader header(*_fTrx);
    header.setCabinHeader();
  }

  void testInitialize()
  {
    prepareGroupHeader();
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    CPPUNIT_ASSERT(_fTrx->fdResponse()->groupHeaders()[0] == Group::GROUP_BY_S8BRAND);
    CPPUNIT_ASSERT(fSF._isS8BrandHeader);
  }

  void testAddS8BrandSectionText_Match_First_ProgBrand()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("US", "APP");
    prepareGroupHeader();

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);

    buildProgramBrandMap();

    S8BrandComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);

    fSF.addS8BrandSectionText(progBrand1);
    CPPUNIT_ASSERT(_fTrx->fdResponse()->groupHeaders()[0] == Group::GROUP_BY_S8BRAND);

    CPPUNIT_ASSERT_EQUAL(std::string("AA-AA /APP - APPLE\n"), _fTrx->response().str());
  }

  void testAddS8BrandSectionText_Match_Second_ProgBrand()
  {
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");
    std::pair<ProgramCode, BrandCode> progBrand3 = make_pair("XX", "YYY");
    prepareGroupHeader();

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);

    buildProgramBrandMap();

    S8BrandComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);
    fSF.addS8BrandSectionText(progBrand2);
    CPPUNIT_ASSERT_EQUAL(std::string("AA-AA /ABB - ABBREVIATE\n"), _fTrx->response().str());
  }

  void testAddS8BrandSectionText_Match_Third_ProgBrand()
  {
    std::pair<ProgramCode, BrandCode> progBrand3 = make_pair("XX", "YYY");
    prepareGroupHeader();

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);

    buildProgramBrandMap();

    S8BrandComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);

    fSF.addS8BrandSectionText(progBrand3);
    CPPUNIT_ASSERT_EQUAL(std::string("AA-AA /YYY - FOREVER\n"), _fTrx->response().str());
  }

  void testDoFiltering()
  {
    std::pair<ProgramCode, BrandCode> progBrand = make_pair("XX", "YYY");
    PaxTypeFare paxTF;
    paxTF.fareDisplayInfo() = _memHandle.create<FareDisplayInfo>();
    paxTF.fareDisplayInfo()->setProgramBrand(progBrand);

    prepareGroupHeader();

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);

    buildProgramBrandMap();

    S8BrandComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);
    fSF._isAxessGlobalDirHeader = true; // to prevent more data sets up

    CPPUNIT_ASSERT(fSF.doFiltering(paxTF, true, false));
  }

  // Cabin section
  void testInitialize_with_Cabin()
  {
    prepareCabinGroupHeader();
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    CPPUNIT_ASSERT(_fTrx->fdResponse()->groupHeaders()[0] == Group::GROUP_BY_CABIN);
    CPPUNIT_ASSERT(fSF._isCabinHeader);
  }
  void testAddCabinSectionText_real_CabinInclusionCode()
  {
    uint8_t inclusionCodeNumber = 4;
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    fSF.addCabinSectionText(inclusionCodeNumber);
    CPPUNIT_ASSERT_EQUAL(std::string("BUSINESS CABIN\n"), _fTrx->response().str());
  }
  void testAddCabinSectionText_wrong_CabinInclusionCode()
  {
    uint8_t inclusionCodeNumber = 9;
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    fSF.addCabinSectionText(inclusionCodeNumber);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID CABIN\n"), _fTrx->response().str());
  }
  void testDoFiltering_Cabin()
  {
    uint8_t inclusionNumber = 5; // SB

    PaxTypeFare paxTF;
    paxTF.fareDisplayInfo() = _memHandle.create<FareDisplayInfo>();
    paxTF.fareDisplayInfo()->setPassedInclusion(inclusionNumber);

    prepareCabinGroupHeader();

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->requestedInclusionCode() = "SB";

    CabinGroupComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);
    fSF._isAxessGlobalDirHeader = true; // to prevent more data sets up

    CPPUNIT_ASSERT(fSF.doFiltering(paxTF, true, true));
  }

  void testDisplayYYHdrMsgForMileageRequest()
  {
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    _fTrx->getRequest()->requestType() = FARE_MILEAGE_REQUEST;
    CPPUNIT_ASSERT(!fSF.displayYYHdrMsg());
  }

  void testDisplayYYHdrMsgForNotPublishedAndSingleInclusionCode()
  {
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->inclusionCode() = "SB";
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    CPPUNIT_ASSERT(fSF.displayYYHdrMsg());
    CPPUNIT_ASSERT_EQUAL(std::string("*** THERE ARE NO YY SB FARES PUBLISHED DFW-JFK *** \n"),
                                     _fTrx->response().str());
  }

  void testDisplayYYHdrMsgForNotPublishedAndMultipleInclusionCode()
  {
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->inclusionCode() = "SBPBJBFB";
    _fTrx->getRequest()->requestedInclusionCode() = "SBPBJBFB";
    _fTrx->getRequest()->multiInclusionCodes() = true;
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    CPPUNIT_ASSERT(fSF.displayYYHdrMsg());
    CPPUNIT_ASSERT_EQUAL(std::string("*** THERE ARE NO YY SB FARES PUBLISHED DFW-JFK *** \n"
                                     "*** THERE ARE NO YY PB FARES PUBLISHED DFW-JFK *** \n"
                                     "*** THERE ARE NO YY JB FARES PUBLISHED DFW-JFK *** \n"
                                     "*** THERE ARE NO YY FB FARES PUBLISHED DFW-JFK *** \n"),
                                     _fTrx->response().str());
  }

  void testDisplayYYHdrMsgForNotPublishedAndABInclusionCode()
  {
    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->inclusionCode() = "PBFBJBBBSBYB";
    _fTrx->getRequest()->requestedInclusionCode() = "PBFBJBBBSBYB";
    _fTrx->getRequest()->multiInclusionCodes() = true;
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    CPPUNIT_ASSERT(fSF.displayYYHdrMsg());
    CPPUNIT_ASSERT_EQUAL(std::string("*** THERE ARE NO YY AB FARES PUBLISHED DFW-JFK *** \n"),
                                     _fTrx->response().str());
  }

  void testDisplayYYHdrMsgForNotPermittedAndSingleInclusionCode()
  {
    PaxTypeFare paxTF;
    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    fareInfo.carrier() = INDUSTRY_CARRIER;
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_market, &tCRInfo);
    paxTF.setFare(&fare);
    paxTF.setFailedFareGroup();
    _market->allPaxTypeFare().emplace_back(&paxTF);

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->inclusionCode() = "SB";
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    CPPUNIT_ASSERT(fSF.displayYYHdrMsg());
    CPPUNIT_ASSERT_EQUAL(std::string("*** YY SB FARES NOT PERMITTED DFW-JFK ON AA *** \n"),
                                     _fTrx->response().str());
  }

  void testDisplayYYHdrMsgForNotPermittedAndMultipleInclusionCode()
  {
    PaxTypeFare paxTF;
    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    fareInfo.carrier() = INDUSTRY_CARRIER;
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_market, &tCRInfo);
    paxTF.setFare(&fare);
    paxTF.setFailedFareGroup();
    _market->allPaxTypeFare().emplace_back(&paxTF);

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->inclusionCode() = "SBPBJBFB";
    _fTrx->getRequest()->requestedInclusionCode() = "SBPBJBFB";
    _fTrx->getRequest()->multiInclusionCodes() = true;
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    CPPUNIT_ASSERT(fSF.displayYYHdrMsg());
    CPPUNIT_ASSERT_EQUAL(std::string("*** YY SB FARES NOT PERMITTED DFW-JFK ON AA *** \n"
                                     "*** YY PB FARES NOT PERMITTED DFW-JFK ON AA *** \n"
                                     "*** YY JB FARES NOT PERMITTED DFW-JFK ON AA *** \n"
                                     "*** YY FB FARES NOT PERMITTED DFW-JFK ON AA *** \n"),
                                     _fTrx->response().str());
  }

  void testDisplayYYHdrMsgForNotPermittedAndABInclusionCode()
  {
    PaxTypeFare paxTF;
    FareInfo fareInfo;
    fareInfo.fareClass() = "Y26";
    fareInfo.carrier() = INDUSTRY_CARRIER;
    Fare fare;

    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_market, &tCRInfo);
    paxTF.setFare(&fare);
    paxTF.setFailedFareGroup();
    _market->allPaxTypeFare().emplace_back(&paxTF);

    FaresSectionFilter fSF;
    fSF.initialize(*_fTrx);
    _fTrx->getRequest()->inclusionCode() = "PBFBJBBBSBYB";
    _fTrx->getRequest()->requestedInclusionCode() = "PBFBJBBBSBYB";
    _fTrx->getRequest()->multiInclusionCodes() = true;
    _fTrx->multipleCarriersEntered() = false;
    _fTrx->getOptions()->allCarriers() = false;
    CPPUNIT_ASSERT(fSF.displayYYHdrMsg());
    CPPUNIT_ASSERT_EQUAL(std::string("*** YY AB FARES NOT PERMITTED DFW-JFK ON AA *** \n"),
                                     _fTrx->response().str());
  }

  /*
     string expected;
     string actual = _fTrx->response().str();

     expected += "AA/US/APP - APPLE\n";

  printExpectedVsActualValues(expected, actual);

        CPPUNIT_ASSERT_EQUAL( expected, actual );
  */
  void printExpectedVsActualValues(string& expected, string& actual)
  {
    cout << std::endl;
    cout << "XXXXXX" << endl;
    cout << actual << endl;
    cout << "XXXXX" << endl;
    cout << "XXXXXX" << endl;
    cout << expected << endl;
    cout << "XXXXX" << endl;

    cout << "length of Expected: " << expected.size();
    cout << "length of Actual  : " << actual.size();
  }

private:
  FareDisplayTrx* _fTrx;
  Group* _group;
  FareMarket* _market;
  AirSeg* _tSeg;
  Itin* _itin;

  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FaresSectionFilterTest);
}
