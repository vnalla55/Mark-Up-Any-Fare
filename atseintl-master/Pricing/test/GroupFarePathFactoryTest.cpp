#include <boost/assign/std/vector.hpp>
#include <vector>

#include "DBAccess/DataHandle.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Diagnostic/DiagCollector.h"

#include "Pricing/test/FactoriesConfigStub.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;
using namespace boost::assign;

namespace tse
{

class GroupFarePathFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GroupFarePathFactoryTest);
  CPPUNIT_TEST(testProcessMultiPax_adtFU_extend_cnnFU);
  CPPUNIT_TEST(testProcessMultiPax_2adtFU_tvl_1cnnFU);
  CPPUNIT_TEST(testProcessMultiPax_adtFU_sameFB_cnnFU);
  CPPUNIT_TEST(testNegPriorityIsSameAsFppqPriorityWhenSamePriority);
  CPPUNIT_TEST(testNegPriorityChangesToLowerWhenFppqPriorityHasLowerPriority);
  CPPUNIT_TEST(testNegPriorityNoChangeWhenFppqPriorityHasHigherPriority);
  CPPUNIT_TEST(testHigherPriorityIsOnTopWhenHigherPriorityGfpItemIsPushedLast);
  CPPUNIT_TEST(testHigherPriorityIsOnTopWhenHigherPriorityGfpItemIsPushedFirst);
  CPPUNIT_TEST(testisValidGFPForValidatingCxr_NoValidatingCxr);
  CPPUNIT_SKIP_TEST(testisValidGFPForValidatingCxr_ValidCxrList);
  CPPUNIT_TEST(testisValidGFPForValidatingCxr_ShouldFail);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _gfp = _memHandle.create<GroupFarePathFactory>(*_trx);
    buildAllTestFareMarkets();
    buildItin();
    buildAllPaxTypeFaresAndFareUsages();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void buildAllTestFareMarkets()
  {
    _dataHandle.get(_fm12);
    _fm12->travelSeg().push_back(&_tvlSeg12);

    _dataHandle.get(_fm13);
    _fm13->travelSeg().push_back(&_tvlSeg12);
    _fm13->travelSeg().push_back(&_tvlSeg23);

    _dataHandle.get(_fm23);
    _fm23->travelSeg().push_back(&_tvlSeg23);

    _dataHandle.get(_fm24);
    _fm24->travelSeg().push_back(&_tvlSeg23);
    _fm24->travelSeg().push_back(&_tvlSeg34);

    _dataHandle.get(_fm34);
    _fm34->travelSeg().push_back(&_tvlSeg34);

    _dataHandle.get(_fm35);
    _fm35->travelSeg().push_back(&_tvlSeg34);
    _fm35->travelSeg().push_back(&_tvlSeg45);

    _dataHandle.get(_fm45);
    _fm45->travelSeg().push_back(&_tvlSeg45);
  }

  void buildItin()
  {
    _dataHandle.get(_itin);
    _itin->travelSeg().push_back(&_tvlSeg12);
    _itin->travelSeg().push_back(&_tvlSeg23);
    _itin->travelSeg().push_back(&_tvlSeg34);
    _itin->travelSeg().push_back(&_tvlSeg45);
  }

  void buildAllPaxTypeFaresAndFareUsages()
  {
    _dataHandle.get(_ptf12);
    _ptf12->fareMarket() = _fm12;
    _dataHandle.get(_fu12);
    _fu12->paxTypeFare() = _ptf12;
    _fu12->travelSeg().push_back(&_tvlSeg12);

    _dataHandle.get(_ptf13);
    _ptf13->fareMarket() = _fm13;
    _dataHandle.get(_fu13);
    _fu13->paxTypeFare() = _ptf13;
    _fu13->travelSeg().push_back(&_tvlSeg12);
    _fu13->travelSeg().push_back(&_tvlSeg23);

    _dataHandle.get(_ptf23);
    _ptf23->fareMarket() = _fm23;
    _dataHandle.get(_fu23);
    _fu23->paxTypeFare() = _ptf23;
    _fu23->travelSeg().push_back(&_tvlSeg23);

    _dataHandle.get(_ptf24);
    _ptf24->fareMarket() = _fm24;
    _dataHandle.get(_fu24);
    _fu24->paxTypeFare() = _ptf24;
    _fu24->travelSeg().push_back(&_tvlSeg23);
    _fu24->travelSeg().push_back(&_tvlSeg34);

    _dataHandle.get(_ptf34);
    _ptf34->fareMarket() = _fm34;
    _dataHandle.get(_fu34);
    _fu34->paxTypeFare() = _ptf34;
    _fu34->travelSeg().push_back(&_tvlSeg34);

    _dataHandle.get(_ptf35);
    _ptf35->fareMarket() = _fm35;
    _dataHandle.get(_fu35);
    _fu35->paxTypeFare() = _ptf35;
    _fu35->travelSeg().push_back(&_tvlSeg34);
    _fu35->travelSeg().push_back(&_tvlSeg45);

    _dataHandle.get(_ptf45);
    _ptf45->fareMarket() = _fm45;
    _dataHandle.get(_fu45);
    _fu45->paxTypeFare() = _ptf45;
    _fu45->travelSeg().push_back(&_tvlSeg45);

    _dataHandle.get(_adtPtf12);
    _adtPtf12->fareMarket() = _fm12;
    _dataHandle.get(_adtFu12);
    _adtFu12->paxTypeFare() = _adtPtf12;
    _adtFu12->travelSeg().push_back(&_tvlSeg12);

    _dataHandle.get(_adtPtf13);
    _adtPtf13->fareMarket() = _fm13;
    _dataHandle.get(_adtFu13);
    _adtFu13->paxTypeFare() = _adtPtf13;
    _adtFu13->travelSeg().push_back(&_tvlSeg12);
    _adtFu13->travelSeg().push_back(&_tvlSeg23);

    _dataHandle.get(_adtPtf23);
    _adtPtf23->fareMarket() = _fm23;
    _dataHandle.get(_adtFu23);
    _adtFu23->paxTypeFare() = _adtPtf23;
    _adtFu23->travelSeg().push_back(&_tvlSeg23);

    _dataHandle.get(_adtPtf24);
    _adtPtf24->fareMarket() = _fm24;
    _dataHandle.get(_adtFu24);
    _adtFu24->paxTypeFare() = _adtPtf24;
    _adtFu24->travelSeg().push_back(&_tvlSeg23);
    _adtFu24->travelSeg().push_back(&_tvlSeg34);

    _dataHandle.get(_adtPtf34);
    _adtPtf34->fareMarket() = _fm34;
    _dataHandle.get(_adtFu34);
    _adtFu34->paxTypeFare() = _adtPtf34;
    _adtFu34->travelSeg().push_back(&_tvlSeg34);

    _dataHandle.get(_adtPtf35);
    _adtPtf35->fareMarket() = _fm35;
    _dataHandle.get(_adtFu35);
    _adtFu35->paxTypeFare() = _adtPtf35;
    _adtFu35->travelSeg().push_back(&_tvlSeg34);
    _adtFu35->travelSeg().push_back(&_tvlSeg45);

    _dataHandle.get(_adtPtf45);
    _adtPtf45->fareMarket() = _fm45;
    _dataHandle.get(_adtFu45);
    _adtFu45->paxTypeFare() = _adtPtf45;
    _adtFu45->travelSeg().push_back(&_tvlSeg45);
  }

  void testProcessMultiPax_adtFU_extend_cnnFU()
  {
    FarePath cnnFP, adtFP;
    cnnFP.itin() = _itin;
    adtFP.itin() = _itin;

    PricingUnit pu1, pu2;
    pu1.fareUsage().push_back(_fu12);
    pu1.fareUsage().push_back(_fu45);
    pu2.fareUsage().push_back(_fu23);
    pu2.fareUsage().push_back(_fu34);
    cnnFP.pricingUnit().push_back(&pu1);
    cnnFP.pricingUnit().push_back(&pu2);

    PricingUnit adtPU1;
    adtPU1.fareUsage().push_back(_adtFu13);
    adtPU1.fareUsage().push_back(_adtFu35);
    adtFP.pricingUnit().push_back(&adtPU1);

    std::vector<FarePath*> gfpVect;
    gfpVect.push_back(&adtFP);
    gfpVect.push_back(&cnnFP);

    _ptf23->cabin().setFirstClass();

    _adtPtf13->cabin().setFirstClass();
    _adtPtf35->cabin().setBusinessClass();

    bool needSameFareBreak = false;
    bool isInSideTripPU = false;
    // should pass segment 23 same cabin
    CPPUNIT_ASSERT_EQUAL(
        true, _gfp->processMultiPax(gfpVect, *_fu23, needSameFareBreak, isInSideTripPU, _diag));

    _ptf12->cabin().setBusinessClass();
    // should fail segment 12 different cabin
    CPPUNIT_ASSERT_EQUAL(
        false, _gfp->processMultiPax(gfpVect, *_fu12, needSameFareBreak, isInSideTripPU, _diag));

    needSameFareBreak = true;
    // should fail
    CPPUNIT_ASSERT_EQUAL(
        false, _gfp->processMultiPax(gfpVect, *_fu23, needSameFareBreak, isInSideTripPU, _diag));

    needSameFareBreak = false;
    _adtPtf13->cabin().setBusinessClass();
    _adtPtf35->cabin().setFirstClass();
    // should fail as cabin different
    CPPUNIT_ASSERT_EQUAL(
        false, _gfp->processMultiPax(gfpVect, *_fu23, needSameFareBreak, isInSideTripPU, _diag));
  }

  void testProcessMultiPax_2adtFU_tvl_1cnnFU()
  {
    FarePath cnnFP, adtFP;
    cnnFP.itin() = _itin;
    adtFP.itin() = _itin;

    PricingUnit pu1, pu2, pu3;
    pu1.fareUsage().push_back(_fu12);
    pu2.fareUsage().push_back(_fu24);
    pu3.fareUsage().push_back(_fu45);

    cnnFP.pricingUnit().push_back(&pu1);
    cnnFP.pricingUnit().push_back(&pu2);
    cnnFP.pricingUnit().push_back(&pu3);

    PricingUnit adtPU1;
    adtPU1.fareUsage().push_back(_adtFu13);
    adtPU1.fareUsage().push_back(_adtFu35);
    adtFP.pricingUnit().push_back(&adtPU1);

    std::vector<FarePath*> gfpVect;
    gfpVect.push_back(&adtFP);
    gfpVect.push_back(&cnnFP);

    _ptf24->cabin().setFirstClass();

    _adtPtf13->cabin().setFirstClass();
    _adtPtf35->cabin().setFirstClass();

    bool needSameFareBreak = false;
    bool isInSideTripPU = false;
    // should pass segment 23 same cabin
    CPPUNIT_ASSERT_EQUAL(
        true, _gfp->processMultiPax(gfpVect, *_fu24, needSameFareBreak, isInSideTripPU, _diag));

    needSameFareBreak = true;
    // should fail
    CPPUNIT_ASSERT_EQUAL(
        false, _gfp->processMultiPax(gfpVect, *_fu24, needSameFareBreak, isInSideTripPU, _diag));

    needSameFareBreak = false;
    _adtPtf13->cabin().setBusinessClass();
    _adtPtf35->cabin().setFirstClass();
    // should fail as cabin different
    CPPUNIT_ASSERT_EQUAL(
        false, _gfp->processMultiPax(gfpVect, *_fu24, needSameFareBreak, isInSideTripPU, _diag));

    _adtPtf13->cabin().setFirstClass();
    _adtPtf35->cabin().setBusinessClass();
    // should fail as cabin different
    CPPUNIT_ASSERT_EQUAL(
        false, _gfp->processMultiPax(gfpVect, *_fu24, needSameFareBreak, isInSideTripPU, _diag));
  }

  void testProcessMultiPax_adtFU_sameFB_cnnFU()
  {
    FarePath cnnFP, adtFP;
    cnnFP.itin() = _itin;
    adtFP.itin() = _itin;

    PricingUnit pu1, pu2;
    pu1.fareUsage().push_back(_fu12);
    pu1.fareUsage().push_back(_fu45);
    pu2.fareUsage().push_back(_fu23);
    pu2.fareUsage().push_back(_fu34);
    cnnFP.pricingUnit().push_back(&pu1);
    cnnFP.pricingUnit().push_back(&pu2);

    PricingUnit adtPU1, adtPU2;
    adtPU1.fareUsage().push_back(_adtFu12);
    adtPU1.fareUsage().push_back(_adtFu45);
    adtPU2.fareUsage().push_back(_adtFu23);
    adtPU2.fareUsage().push_back(_adtFu34);
    adtFP.pricingUnit().push_back(&adtPU1);
    adtFP.pricingUnit().push_back(&adtPU2);

    std::vector<FarePath*> gfpVect;
    gfpVect.push_back(&adtFP);
    gfpVect.push_back(&cnnFP);

    _ptf23->cabin().setFirstClass();
    _adtPtf23->cabin().setFirstClass();

    bool needSameFareBreak = false;
    bool isInSideTripPU = false;
    // should pass segment 23 same cabin
    CPPUNIT_ASSERT_EQUAL(
        true, _gfp->processMultiPax(gfpVect, *_fu23, needSameFareBreak, isInSideTripPU, _diag));

    needSameFareBreak = true;
    CPPUNIT_ASSERT_EQUAL(
        true, _gfp->processMultiPax(gfpVect, *_fu23, needSameFareBreak, isInSideTripPU, _diag));
  }

  void testNegPriorityIsSameAsFppqPriorityWhenSamePriority()
  {
    FPPQItem fppqItem;
    GroupFarePath gfpath;
    fppqItem.mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    gfpath.mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    _gfp->setPriority(fppqItem, gfpath);
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, gfpath.priorityStatus().negotiatedFarePriority());
  }

  void testNegPriorityChangesToLowerWhenFppqPriorityHasLowerPriority()
  {
    FPPQItem fppqItem;
    GroupFarePath gfpath;
    fppqItem.mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    gfpath.mutablePriorityStatus().setNegotiatedFarePriority(DEFAULT_PRIORITY);
    _gfp->setPriority(fppqItem, gfpath);
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, gfpath.priorityStatus().negotiatedFarePriority());
  }

  void testNegPriorityNoChangeWhenFppqPriorityHasHigherPriority()
  {
    FPPQItem fppqItem;
    GroupFarePath gfpath;
    fppqItem.mutablePriorityStatus().setNegotiatedFarePriority(DEFAULT_PRIORITY);
    gfpath.mutablePriorityStatus().setNegotiatedFarePriority(PRIORITY_LOW);
    _gfp->setPriority(fppqItem, gfpath);
    CPPUNIT_ASSERT_EQUAL(PRIORITY_LOW, gfpath.priorityStatus().negotiatedFarePriority());
  }

  void testHigherPriorityIsOnTopWhenHigherPriorityGfpItemIsPushedLast()
  {
    GroupFarePath gfpath1, gfpath2, gfpath3;
    creategfpath(gfpath1, PRIORITY_LOW);
    creategfpath(gfpath2, PRIORITY_LOW);
    creategfpath(gfpath3, DEFAULT_PRIORITY);
    _gfp->groupFarePathPQ().enqueue(&gfpath1);
    _gfp->groupFarePathPQ().enqueue(&gfpath2);
    _gfp->groupFarePathPQ().enqueue(&gfpath3);
    assertPopFromPriorityStack(DEFAULT_PRIORITY);
    assertPopFromPriorityStack(PRIORITY_LOW);
    assertPopFromPriorityStack(PRIORITY_LOW);
  }

  void testHigherPriorityIsOnTopWhenHigherPriorityGfpItemIsPushedFirst()
  {
    GroupFarePath gfpath1, gfpath2;
    creategfpath(gfpath1, DEFAULT_PRIORITY);
    creategfpath(gfpath2, PRIORITY_LOW);
    _gfp->groupFarePathPQ().enqueue(&gfpath1);
    _gfp->groupFarePathPQ().enqueue(&gfpath2);
    assertPopFromPriorityStack(DEFAULT_PRIORITY);
    assertPopFromPriorityStack(PRIORITY_LOW);
  }

  void testisValidGFPForValidatingCxr_NoValidatingCxr()
  {
    GroupFarePath gfPath;
    FPPQItem* fppqItem1 = _memHandle.create<FPPQItem>();
    FPPQItem* fppqItem2 = _memHandle.create<FPPQItem>();

    fppqItem1->farePath() = _memHandle.create<FarePath>();
    fppqItem2->farePath() = _memHandle.create<FarePath>();

    gfPath.groupFPPQItem().push_back(fppqItem1);
    gfPath.groupFPPQItem().push_back(fppqItem2);

    CPPUNIT_ASSERT(_gfp->isValidGFPForValidatingCxr(gfPath, _diag) == true);
  }

  void testisValidGFPForValidatingCxr_ValidCxrList()
  {
    DiagCollector diag;
    diag.activate();
    Diagnostic* rootDiag = _memHandle.create<Diagnostic>();
    rootDiag->activate();
    diag.rootDiag() = rootDiag;

    GroupFarePath gfPath;
    FPPQItem* fppqItem1 = _memHandle.create<FPPQItem>();
    FPPQItem* fppqItem2 = _memHandle.create<FPPQItem>();
    FPPQItem* fppqItem3 = _memHandle.create<FPPQItem>();

    fppqItem1->farePath() = _memHandle.create<FarePath>();
    fppqItem2->farePath() = _memHandle.create<FarePath>();
    fppqItem3->farePath() = _memHandle.create<FarePath>();

    CarrierCode set1 [] = {"AA", "AB", "DL", "EK", "BA"};
    CarrierCode set2 [] = {"AB", "AA", "AF", "EK"};
    CarrierCode set3 [] = {"EK", "AA"};

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 5);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 4);
    std::vector<CarrierCode> list3;
    list3.insert(list3.begin(), set3, set3 + 2);

    fppqItem1->farePath()->validatingCarriers() = list1;
    fppqItem2->farePath()->validatingCarriers() = list2;
    fppqItem3->farePath()->validatingCarriers() = list3;

    gfPath.groupFPPQItem().push_back(fppqItem1);
    gfPath.groupFPPQItem().push_back(fppqItem2);
    gfPath.groupFPPQItem().push_back(fppqItem3);

    CPPUNIT_ASSERT(_gfp->isValidGFPForValidatingCxr(gfPath, diag) == true);
    std::string expectedDiagResponse(" GROUP FAREPATH VALID FOR GSA. VALIDATING CXR LIST: AA  EK  \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testisValidGFPForValidatingCxr_ShouldFail()
  {
    GroupFarePath gfPath;
    FPPQItem* fppqItem1 = _memHandle.create<FPPQItem>();
    FPPQItem* fppqItem2 = _memHandle.create<FPPQItem>();
    FPPQItem* fppqItem3 = _memHandle.create<FPPQItem>();

    fppqItem1->farePath() = _memHandle.create<FarePath>();
    fppqItem2->farePath() = _memHandle.create<FarePath>();
    fppqItem3->farePath() = _memHandle.create<FarePath>();

    CarrierCode set1 [] = {"AA", "AB", "DL", "EK", "BA"};
    CarrierCode set2 [] = {"AB", "AA", "AF", "EK"};
    CarrierCode set3 [] = {"DL", "BA", "AF"};

    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 5);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 4);
    std::vector<CarrierCode> list3;
    list3.insert(list3.begin(), set3, set3 + 3);

    fppqItem1->farePath()->validatingCarriers() = list1;
    fppqItem2->farePath()->validatingCarriers() = list2;
    fppqItem3->farePath()->validatingCarriers() = list3;

    gfPath.groupFPPQItem().push_back(fppqItem1);
    gfPath.groupFPPQItem().push_back(fppqItem2);
    gfPath.groupFPPQItem().push_back(fppqItem3);

    CPPUNIT_ASSERT(_gfp->isValidGFPForValidatingCxr(gfPath, _diag) == false);
  }

  void assertPopFromPriorityStack(PRIORITY expectedPriority)
  {
    CPPUNIT_ASSERT_EQUAL(
        expectedPriority,
        _gfp->groupFarePathPQ().dequeue()->priorityStatus().negotiatedFarePriority());
  }

  void creategfpath(GroupFarePath& gfpath, PRIORITY priority)
  {
    gfpath.setTotalNUCAmount(100);
    gfpath.mutablePriorityStatus().setNegotiatedFarePriority(priority);
  }

private:
  TestMemHandle _memHandle;
  DataHandle _dataHandle;
  AirSeg _tvlSeg12, _tvlSeg23, _tvlSeg34, _tvlSeg45;

  FareMarket* _fm12, *_fm13, *_fm23, *_fm24, *_fm34, *_fm35, *_fm45;
  PaxTypeFare* _ptf12, *_ptf13, *_ptf23, *_ptf24, *_ptf34, *_ptf35, *_ptf45;
  PaxTypeFare* _adtPtf12, *_adtPtf13, *_adtPtf23, *_adtPtf24, *_adtPtf34, *_adtPtf35, *_adtPtf45;
  FareUsage* _fu12, *_fu13, *_fu23, *_fu24, *_fu34, *_fu35, *_fu45;
  FareUsage* _adtFu12, *_adtFu13, *_adtFu23, *_adtFu24, *_adtFu34, *_adtFu35, *_adtFu45;
  Itin* _itin;
  PricingTrx* _trx;
  GroupFarePathFactory* _gfp;
  DiagCollector _diag;
};

class GroupFarePathFactoryExpansionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GroupFarePathFactoryExpansionTest);
  CPPUNIT_TEST(testInitThreads);
  CPPUNIT_TEST(testCollectThreadResult_AllValid);
  CPPUNIT_TEST(testCollectThreadResult_NotAllValid);
  CPPUNIT_TEST(testCollectThreadResult_NotAllXc);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _req = _memHandle.create<PricingRequest>();
    _opt = _memHandle.create<PricingOptions>();
    _trx->setRequest(_req);
    _trx->setOptions(_opt);

    _pfpf[0] = createPFPF("ADT");
    _pfpf[1] = createPFPF("CNN");
    _pfpf[2] = createPFPF("INF");

    _gfpf = _memHandle.create<GroupFarePathFactory>(*_trx);
    _gfpf->_totalFPFactory = 3;
    _gfpf->_paxFarePathFactoryBucket += _pfpf[0], _pfpf[1], _pfpf[2];
    _gfpf->_infantFactoryInd.resize(3);

    _diag = _memHandle.create<DiagCollector>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testInitThreads()
  {
    std::vector<uint32_t> fpIndices;
    fpIndices += 111, GroupFarePathFactory::INVALID_FP_INDEX, 333;

    std::vector<GroupFarePathFactory::GETFPInput> thr;
    _gfpf->initThreads(thr, false, 0, fpIndices);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), thr.size());
    validateGETFPInput(thr[0], 0, 112, "Uninitialized task for 0th pax type");
    validateGETFPInput(thr[1], 2, 333, "Uninitialized task for 2th pax type");
  }

  void testCollectThreadResult_AllValid()
  {
    std::vector<GroupFarePathFactory::GETFPInput> thr(3);
    initGETFPInput(0, 111, thr[0]);
    initGETFPInput(1, 222, thr[1]);
    initGETFPInput(2, 333, thr[2]);

    GroupFarePath gfp;
    CPPUNIT_ASSERT(_gfpf->collectThreadResult(thr, gfp, true, *_diag));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), gfp.farePathIndices().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(111), gfp.farePathIndices().at(0));
    CPPUNIT_ASSERT_EQUAL(uint32_t(222), gfp.farePathIndices().at(1));
    CPPUNIT_ASSERT_EQUAL(uint32_t(333), gfp.farePathIndices().at(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), gfp.groupFPPQItem().size());
    CPPUNIT_ASSERT_EQUAL(thr[0].fppqItem, gfp.groupFPPQItem().at(0));
    CPPUNIT_ASSERT_EQUAL(thr[1].fppqItem, gfp.groupFPPQItem().at(1));
    CPPUNIT_ASSERT_EQUAL(thr[2].fppqItem, gfp.groupFPPQItem().at(2));
  }

  void testCollectThreadResult_NotAllValid()
  {
    std::vector<GroupFarePathFactory::GETFPInput> thr(3);
    initGETFPInput(0, 111, thr[0]);
    initGETFPInput(1, 222, thr[1]);
    initGETFPInput(2, 333, thr[2]);
    thr[1].done = false;

    GroupFarePath gfp;
    CPPUNIT_ASSERT(!_gfpf->collectThreadResult(thr, gfp, true, *_diag));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), gfp.farePathIndices().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(111), gfp.farePathIndices().at(0));
    CPPUNIT_ASSERT_EQUAL(uint32_t(222), gfp.farePathIndices().at(1));
    CPPUNIT_ASSERT_EQUAL(uint32_t(333), gfp.farePathIndices().at(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), gfp.groupFPPQItem().size());
    CPPUNIT_ASSERT_EQUAL(thr[0].fppqItem, gfp.groupFPPQItem().at(0));
    CPPUNIT_ASSERT_EQUAL(thr[2].fppqItem, gfp.groupFPPQItem().at(1));
  }

  void testCollectThreadResult_NotAllXc()
  {
    std::vector<GroupFarePathFactory::GETFPInput> thr(3);
    initGETFPInput(0, 111, thr[0]);
    initGETFPInput(2, 333, thr[1]);
    thr[0].done = false;
    _gfpf->_isXcRequest = true;

    GroupFarePath gfp;
    CPPUNIT_ASSERT(_gfpf->collectThreadResult(thr, gfp, true, *_diag));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), gfp.farePathIndices().size());
    CPPUNIT_ASSERT_EQUAL(GroupFarePathFactory::INVALID_FP_INDEX, gfp.farePathIndices().at(0));
    CPPUNIT_ASSERT_EQUAL(GroupFarePathFactory::INVALID_FP_INDEX, gfp.farePathIndices().at(1));
    CPPUNIT_ASSERT_EQUAL(uint32_t(333), gfp.farePathIndices().at(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), gfp.groupFPPQItem().size());
    CPPUNIT_ASSERT_EQUAL(thr[1].fppqItem, gfp.groupFPPQItem().at(0));
  }

private:

  void validateGETFPInput(const GroupFarePathFactory::GETFPInput& thr,
                          uint32_t factIdx, uint32_t idx, const std::string& message)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, static_cast<FPPQItem*>(0), thr.fppqItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, static_cast<const FPPQItem*>(0), thr.primaryFPPQItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, false, thr.done);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, _pfpf[factIdx], thr.pfpf);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, _gfpf, thr.gfpf);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, factIdx, thr.factIdx);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, idx, thr.idx);


  }

  void initGETFPInput(uint32_t factIdx, uint32_t idx, GroupFarePathFactory::GETFPInput& thr)
  {
    thr.factIdx = factIdx;
    thr.idx = idx;
    thr.gfpf = _gfpf;
    thr.pfpf = _pfpf[factIdx];
    thr.fppqItem = _memHandle.create<FPPQItem>();
    thr.fppqItem->farePath() = _memHandle.create<FarePath>();
    thr.diag = _memHandle.create<DiagCollector>();
    thr.done = true;
  }

  PaxFarePathFactory* createPFPF(const PaxTypeCode& code)
  {
    PaxType* pt = _memHandle.create<PaxType>();
    pt->paxType() = code;
    pt->number() = 1;
    PaxFarePathFactory* pfpf = _memHandle.create<PaxFarePathFactory>(_factoriesConfig);
    pfpf->paxType() = pt;
    return pfpf;
  }

  TestMemHandle _memHandle;
  test::FactoriesConfigStub _factoriesConfig;

  PricingTrx* _trx;
  PricingRequest* _req;
  PricingOptions* _opt;
  PaxFarePathFactoryBase* _pfpf[3];
  GroupFarePathFactory* _gfpf;
  DiagCollector* _diag;
};

CPPUNIT_TEST_SUITE_REGISTRATION(GroupFarePathFactoryTest);
CPPUNIT_TEST_SUITE_REGISTRATION(GroupFarePathFactoryExpansionTest);
}

