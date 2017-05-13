#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <gtest/gtest.h>

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Trx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag990Collector.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/PU.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/test/MockFareMarket.h"
#include "Server/TseServer.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackAPO37838Record1EffDateCheck);

using boost::assign::operator+=;

class PricingOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingOrchestratorTest);

  CPPUNIT_TEST(testProcess_Diagnostic600);
  CPPUNIT_TEST(testProcess_Diagnostic601);
  CPPUNIT_TEST(testProcess_Diagnostic603);
  CPPUNIT_TEST(testProcess_Diagnostic605_throw);
  CPPUNIT_TEST(testProcess_Diagnostic609);
  CPPUNIT_TEST(testProcess_Diagnostic610_throw);
  CPPUNIT_TEST(testProcess_Diagnostic620_throw);
  CPPUNIT_TEST(testProcess_Diagnostic631_throw);
  CPPUNIT_TEST(testProcess_Diagnostic634_throw);
  CPPUNIT_TEST(testProcess_Diagnostic636_throw);
  CPPUNIT_TEST(testProcess_Diagnostic639_throw);
  CPPUNIT_TEST(testProcess_Diagnostic653_throw);
  CPPUNIT_TEST(testProcess_Diagnostic654_throw);
  CPPUNIT_TEST(testProcess_Diagnostic660_throw);
  CPPUNIT_TEST(testProcess_Diagnostic690_throw);

  CPPUNIT_TEST(testRemoveDuplicatedSolutions);

  CPPUNIT_TEST(testResetSurchargeForDummyFareUsage_noDummy);
  CPPUNIT_TEST(testResetSurchargeForDummyFareUsage_noSurcharge);
  CPPUNIT_TEST(testResetSurchargeForDummyFareUsage);
  CPPUNIT_TEST(testFlexFareGroupForceCorpFares_MainGroupXCON_isFlexFare);
  CPPUNIT_TEST(testFlexFareGroupForceCorpFares_MainGroupXCOFF_isFlexFare);
  CPPUNIT_TEST(testFlexFareGroupForceCorpFares_MainGroupXCON_notFlexFare);
  CPPUNIT_TEST(testFFGXOFareON_MainGroupXOFareON);
  CPPUNIT_TEST(testFFGXOFareOFF_MainGroupXOFareON);
  CPPUNIT_TEST(testFFGXOFareON_MainGroupXOFareOFF);
  CPPUNIT_TEST(testFFGXOFareOFF_MainGroupXOFareOFF);
/*
  CPPUNIT_TEST(testValidatingCxrForClonedFarePath_shouldFail);
  CPPUNIT_TEST(testValidatingCxrForClonedFarePath_shouldPass);*/

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<PoDataHandleMock>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());
    FareCalcConfig* fcConfig = _memHandle.create<FareCalcConfig>();
    _trx->fareCalcConfig() = fcConfig;
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = TestLocFactory::create("/vobs/atseintl/test/sampledata/DFW_Loc.xml");
    _trx->getRequest()->ticketingAgent() = agent;

    Loc* locLAX = _memHandle.create<LocMock>("LAX");
    Loc* locDFW = _memHandle.create<LocMock>("DFW");
    Loc* locTUL = _memHandle.create<LocMock>("TUL");
    Loc* locORD = _memHandle.create<LocMock>("ORD");

    TravelSegMock* tvlsegLAXDFW = _memHandle(new TravelSegMock(locLAX, locDFW, 1));
    TravelSegMock* tvlsegDFWTUL = _memHandle(new TravelSegMock(locDFW, locTUL, 2));
    TravelSegMock* tvlsegTULDFW = _memHandle(new TravelSegMock(locTUL, locDFW, 3));
    TravelSegMock* tvlsegDFWORD = _memHandle(new TravelSegMock(locDFW, locORD, 4));

    _trx->travelSeg().push_back(tvlsegLAXDFW);
    _trx->travelSeg().push_back(tvlsegDFWTUL);
    _trx->travelSeg().push_back(tvlsegTULDFW);
    _trx->travelSeg().push_back(tvlsegDFWORD);

    std::vector<TravelSeg*>* sideTrip = _memHandle.create<std::vector<TravelSeg*> >();
    sideTrip->push_back(tvlsegDFWTUL);
    sideTrip->push_back(tvlsegTULDFW);

    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->number() = 1;
    paxType->paxType() = "ADT";
    PaxTypeInfo pti;
    paxType->paxTypeInfo() = &pti;

    _trx->paxType().push_back(paxType);

    FareMarketMock* mktLAXDFW = _memHandle.create<FareMarketMock>(locLAX, locDFW, paxType);
    mktLAXDFW->travelSeg().push_back(tvlsegLAXDFW);

    FareMarketMock* mktLAXTUL = _memHandle.create<FareMarketMock>(locLAX, locTUL, paxType);
    mktLAXTUL->travelSeg().push_back(tvlsegLAXDFW);
    mktLAXTUL->travelSeg().push_back(tvlsegDFWTUL);

    FareMarketMock* mktLAXORD = _memHandle.create<FareMarketMock>(locLAX, locORD, paxType);
    mktLAXORD->travelSeg().push_back(tvlsegLAXDFW);
    mktLAXORD->travelSeg().push_back(tvlsegDFWTUL);
    mktLAXORD->travelSeg().push_back(tvlsegTULDFW);
    mktLAXORD->travelSeg().push_back(tvlsegDFWORD);

    FareMarketMock* mktLAXORD_ST = _memHandle.create<FareMarketMock>(locLAX, locORD, paxType);
    mktLAXORD_ST->travelSeg().push_back(tvlsegLAXDFW);
    mktLAXORD_ST->travelSeg().push_back(tvlsegDFWORD);
    mktLAXORD_ST->sideTripTravelSeg().push_back(*sideTrip);

    FareMarketMock* mktDFWTUL = _memHandle.create<FareMarketMock>(locDFW, locTUL, paxType);
    mktDFWTUL->travelSeg().push_back(tvlsegDFWTUL);

    FareMarketMock* mktDFWORD = _memHandle.create<FareMarketMock>(locDFW, locORD, paxType);
    mktDFWORD->travelSeg().push_back(tvlsegDFWORD);

    FareMarketMock* mktTULDFW = _memHandle.create<FareMarketMock>(locTUL, locDFW, paxType);
    mktTULDFW->travelSeg().push_back(tvlsegTULDFW);

    Itin* itin = _memHandle.create<Itin>();

    itin->travelSeg().push_back(tvlsegLAXDFW);
    itin->travelSeg().push_back(tvlsegDFWTUL);
    itin->travelSeg().push_back(tvlsegTULDFW);
    itin->travelSeg().push_back(tvlsegDFWORD);

    itin->fareMarket().push_back(mktLAXDFW);
    itin->fareMarket().push_back(mktLAXTUL);
    itin->fareMarket().push_back(mktLAXORD);
    itin->fareMarket().push_back(mktLAXORD_ST);
    itin->fareMarket().push_back(mktDFWTUL);
    itin->fareMarket().push_back(mktDFWORD);
    itin->fareMarket().push_back(mktTULDFW);

    _trx->itin().push_back(itin);

    _po = _memHandle.create<PricingOrchestratorMock>(*_memHandle.create<TseServerMock>());

    _trx->diagnostic().activate();
    fallback::value::fallbackAPO37838Record1EffDateCheck.set(true);
  }

  void tearDown() { _memHandle.clear(); }


  void testProcess_Diagnostic600()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic600;
    CPPUNIT_ASSERT(_po->process(*_trx));
  }

  void testProcess_Diagnostic601()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic601;
    CPPUNIT_ASSERT(_po->process(*_trx));
  }

  void testProcess_Diagnostic603()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic603;
    CPPUNIT_ASSERT(_po->process(*_trx));
  }

  void testProcess_Diagnostic605_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic605;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic609()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic609;
    CPPUNIT_ASSERT(_po->process(*_trx));
  }

  void testProcess_Diagnostic610_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic610;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic620_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic620;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic631_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic631;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic634_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic634;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic636_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic636;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic639_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic639;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic653_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic653;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic654_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic654;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic660_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic660;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testProcess_Diagnostic690_throw()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic690;
    CPPUNIT_ASSERT_THROW(_po->process(*_trx), ErrorResponseException);
  }

  void testRemoveDuplicatedSolutions()
  {

    GroupFarePath gfp; // just to have a non-null pointer
    GroupFarePath gfpEstimated;
    gfpEstimated.setTotalNUCAmount(1000000);

    std::vector<int> key1, key2, key3;
    key1.push_back(1);
    key1.push_back(2);

    key2.push_back(3);
    key2.push_back(4);

    key3.push_back(5);
    key3.push_back(6);

    // case 1: both options have a fare
    {
      ShoppingTrx::FlightMatrix flightMatrix;
      ShoppingTrx::EstimateMatrix estimateMatrix;

      flightMatrix[key1] = &gfp;
      flightMatrix[key2] = &gfp;

      estimateMatrix[key1] = std::make_pair(std::vector<int>(), &gfp);
      estimateMatrix[key3] = std::make_pair(std::vector<int>(), &gfp);

      _po->removeDuplicatedSolutions(flightMatrix, estimateMatrix);

      // we expect that estimate was removed
      CPPUNIT_ASSERT(flightMatrix.count(key1) == 1);
      CPPUNIT_ASSERT(flightMatrix.count(key2) == 1);
      CPPUNIT_ASSERT(estimateMatrix.count(key1) == 0);
      CPPUNIT_ASSERT(estimateMatrix.count(key3) == 1);
    }

    // case 2: estimate has no fare
    {
      ShoppingTrx::FlightMatrix flightMatrix;
      ShoppingTrx::EstimateMatrix estimateMatrix;

      flightMatrix[key1] = &gfp;
      flightMatrix[key2] = &gfp;

      estimateMatrix[key1] = std::make_pair(std::vector<int>(), static_cast<GroupFarePath*>(0));
      estimateMatrix[key3] = std::make_pair(std::vector<int>(), &gfp);

      _po->removeDuplicatedSolutions(flightMatrix, estimateMatrix);

      // we expect that estimate was removed
      CPPUNIT_ASSERT(flightMatrix.count(key1) == 1);
      CPPUNIT_ASSERT(flightMatrix.count(key2) == 1);
      CPPUNIT_ASSERT(estimateMatrix.count(key1) == 0);
      CPPUNIT_ASSERT(estimateMatrix.count(key3) == 1);
    }

    // case 3: flight has no fare
    {
      ShoppingTrx::FlightMatrix flightMatrix;
      ShoppingTrx::EstimateMatrix estimateMatrix;

      flightMatrix[key1] = 0;
      flightMatrix[key2] = &gfp;

      estimateMatrix[key1] = std::make_pair(std::vector<int>(), &gfp);
      estimateMatrix[key3] = std::make_pair(std::vector<int>(), &gfp);

      _po->removeDuplicatedSolutions(flightMatrix, estimateMatrix);

      // we expect that estimate was removed
      CPPUNIT_ASSERT(flightMatrix.count(key1) == 0);
      CPPUNIT_ASSERT(flightMatrix.count(key2) == 1);
      CPPUNIT_ASSERT(estimateMatrix.count(key1) == 1);
      CPPUNIT_ASSERT(estimateMatrix.count(key3) == 1);
    }

    // case 4: flight has a fare but it's 1000000
    {
      ShoppingTrx::FlightMatrix flightMatrix;
      ShoppingTrx::EstimateMatrix estimateMatrix;

      flightMatrix[key1] = &gfpEstimated;
      flightMatrix[key2] = &gfp;

      estimateMatrix[key1] = std::make_pair(std::vector<int>(), &gfp);
      estimateMatrix[key3] = std::make_pair(std::vector<int>(), &gfp);

      _po->removeDuplicatedSolutions(flightMatrix, estimateMatrix);

      // we expect that estimate was removed
      CPPUNIT_ASSERT(flightMatrix.count(key1) == 0);
      CPPUNIT_ASSERT(flightMatrix.count(key2) == 1);
      CPPUNIT_ASSERT(estimateMatrix.count(key1) == 1);
      CPPUNIT_ASSERT(estimateMatrix.count(key3) == 1);
    }
  }

  FareUsage* createFareUsage(bool isDummy, const MoneyAmount& surcharge)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = _memHandle.create<PaxTypeFare>();
    fu->paxTypeFare()->fareMarket() = _memHandle.create<FareMarket>();
    if (isDummy)
    {
      fu->paxTypeFare()->fareMarket()->fareCalcFareAmt() = "100.0";
      fu->paxTypeFare()->fareMarket()->fareBasisCode() = "AAA";
    }
    fu->surchargeAmt() = surcharge;
    return fu;
  }

  enum
  {
    NOT_DUMMY_FARE = 0,
    DUMMY_FARE = 1
  };

  FareUsage* addFareUsage(Itin* itin, FareUsage* fu)
  {
    itin->farePath().front()->pricingUnit().front()->fareUsage() += fu;
    itin->farePath().front()->increaseTotalNUCAmount(fu->surchargeAmt());
    return fu;
  }

  Itin* createItin()
  {
    Itin* itin = _memHandle.create<Itin>();
    itin->farePath() += _memHandle.create<FarePath>();
    itin->farePath().front()->pricingUnit() += _memHandle.create<PricingUnit>();
    addFareUsage(itin, createFareUsage(NOT_DUMMY_FARE, 100.0));
    return itin;
  }

  void testResetSurchargeForDummyFareUsage_noDummy()
  {
    std::vector<Itin*> itins(1, createItin());
    FareUsage* fu = addFareUsage(itins[0], createFareUsage(NOT_DUMMY_FARE, 50.0));

    _po->resetSurchargeForDummyFareUsage(itins);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(150.0, itins[0]->farePath().front()->getTotalNUCAmount(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, fu->surchargeAmt(), EPSILON);
  }

  void testResetSurchargeForDummyFareUsage_noSurcharge()
  {
    std::vector<Itin*> itins(1, createItin());
    FareUsage* fu = addFareUsage(itins[0], createFareUsage(DUMMY_FARE, 0.0));

    _po->resetSurchargeForDummyFareUsage(itins);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, itins[0]->farePath().front()->getTotalNUCAmount(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, fu->surchargeAmt(), EPSILON);
  }

  void testResetSurchargeForDummyFareUsage()
  {
    std::vector<Itin*> itins(1, createItin());
    FareUsage* fu = addFareUsage(itins[0], createFareUsage(DUMMY_FARE, 200.0));

    _po->resetSurchargeForDummyFareUsage(itins);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, itins[0]->farePath().front()->getTotalNUCAmount(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, fu->surchargeAmt(), EPSILON);
  }

  void createFlexFareGroups(bool isFlexfare, bool isMainGroupXC, bool corpID)
  {
    //Main defaultGroup, XC
    _trx->setFlexFare(isFlexfare);
    _trx->getOptions()->forceCorpFares() = isMainGroupXC;
    if (corpID)
      _trx->getRequest()->getMutableFlexFaresGroupsData().addCorpId("SAB00",0);

    if (isFlexfare) //If the request is for FlexFare
    {
      //FFG 2, XC=ON, Has corpID
      _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(true, 2);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, 2);
      _trx->getRequest()->getMutableFlexFaresGroupsData().addCorpId("SAB22", 2);

      //FFG 3, XC=ON, No corpID
      _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(3);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(true,3);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true,3);

      //FFG 4, XC=OFF, Has corpID
      _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(4);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(false, 4);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, 4);
      _trx->getRequest()->getMutableFlexFaresGroupsData().addCorpId("SAB44",4);

      //FFG 5, XC=OFF, No corpID
      _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(5);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXCIndicatorStatus(false, 5);
      _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareGroupStatus(true, 5);
    }
  }

  void testFlexFareGroupForceCorpFares_MainGroupXCON_isFlexFare()
  {
    createFlexFareGroups(true, true, true);//Main Group with XC=ON, has CorpID

    uint32_t groupIndex = 0; //Main Group 0
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup());
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON());
    CPPUNIT_ASSERT_EQUAL(true, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds().empty());

    ++groupIndex;//FFG 2, XC=ON, Has corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(2));
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(2));
    CPPUNIT_ASSERT_EQUAL(true, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(2).empty());

    ++groupIndex;//FFG 3, XC=ON, No corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(3));
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(3));
    CPPUNIT_ASSERT_EQUAL(false, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(3).empty());

    ++groupIndex;//FFG 4, XC=OFF, Has corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(4));
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(4));
    CPPUNIT_ASSERT_EQUAL(true, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(4).empty());

    ++groupIndex;//FFG 5, XC=OFF, No corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(5));
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(5));
    CPPUNIT_ASSERT_EQUAL(false, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(5).empty());
  }

  void testFlexFareGroupForceCorpFares_MainGroupXCOFF_isFlexFare()
  {
    createFlexFareGroups(true, false, true);//Main Group with XC=OFF, has CorpID

    uint32_t groupIndex = 0; //Main Group 0
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup());
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON());
    CPPUNIT_ASSERT_EQUAL(true, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds().empty());

    ++groupIndex;//FFG 2, XC=ON, Has corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(2));
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(2));
    CPPUNIT_ASSERT_EQUAL(true, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(2).empty());

    ++groupIndex;//FFG 3, XC=ON, No corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(3));
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(3));
    CPPUNIT_ASSERT_EQUAL(false, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(3).empty());

    ++groupIndex;//FFG 4, XC=OFF, Has corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(4));
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(4));
    CPPUNIT_ASSERT_EQUAL(true, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(4).empty());

    ++groupIndex;//FFG 5, XC=OFF, No corpID
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(5));
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(5));
    CPPUNIT_ASSERT_EQUAL(false, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds(5).empty());
  }

  void testFlexFareGroupForceCorpFares_MainGroupXCON_notFlexFare()
  {
    createFlexFareGroups(false, true, false);//Main Group with XC=ON, No CorpID, Not FlexFareGroup

    uint32_t groupIndex = 0;
    _po->checkFlexFaresForceCorpFares(*_trx, groupIndex);
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->forceCorpFares());
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup());
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON());
    CPPUNIT_ASSERT_EQUAL(false, !_trx->getRequest()->getMutableFlexFaresGroupsData().getCorpIds().empty());
  }

  void testFFGXOFareON_MainGroupXOFareON()
  {
    //Main defaultGroup, XO=ON
    uint32_t groupIndex = 0; //Main Group 0
    _trx->setFlexFare(true);
    _trx->getOptions()->xoFares()='T';

    //FFG 2, XO=ON
    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXOFares('T',2);

    _po->checkFlexFareGroupXOFareStatus(*_trx, ++groupIndex);
    CPPUNIT_ASSERT_EQUAL('T', _trx->getRequest()->getMutableFlexFaresGroupsData().getFlexFareXOFares(2));
  }

  void testFFGXOFareOFF_MainGroupXOFareON()
  {
    //Main defaultGroup, XO=ON
    uint32_t groupIndex = 0; //Main Group 0
    _trx->setFlexFare(true);
    _trx->getOptions()->xoFares()='T';

    //FFG 2, XO=OFF
    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXOFares('F',2);

    _po->checkFlexFareGroupXOFareStatus(*_trx, ++groupIndex);
    CPPUNIT_ASSERT_EQUAL('F', _trx->getRequest()->getMutableFlexFaresGroupsData().getFlexFareXOFares(2));
  }

  void testFFGXOFareON_MainGroupXOFareOFF()
  {
    //Main defaultGroup, XO=OFF
    uint32_t groupIndex = 0; //Main Group 0
    _trx->setFlexFare(true);
    _trx->getOptions()->xoFares()='F';

    //FFG 2, XO=ON
    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXOFares('T',2);

    _po->checkFlexFareGroupXOFareStatus(*_trx, ++groupIndex);
    CPPUNIT_ASSERT_EQUAL('T', _trx->getRequest()->getMutableFlexFaresGroupsData().getFlexFareXOFares(2));
  }

  void testFFGXOFareOFF_MainGroupXOFareOFF()
  {
    //Main defaultGroup, XO=OFF
    uint32_t groupIndex = 0; //Main Group 0
    _trx->setFlexFare(true);
    _trx->getOptions()->xoFares()='F';

    //FFG 2, XO=OFF
    _trx->getRequest()->getMutableFlexFaresGroupsData().createNewGroup(2);
    _trx->getRequest()->getMutableFlexFaresGroupsData().setFlexFareXOFares('F',2);

    _po->checkFlexFareGroupXOFareStatus(*_trx, ++groupIndex);
    CPPUNIT_ASSERT_EQUAL('F', _trx->getRequest()->getMutableFlexFaresGroupsData().getFlexFareXOFares(2));
  }

/*
  void testValidatingCxrForClonedFarePath_shouldFail()
  {
    Diag990Collector diag;
    diag.activate();
    _trx->diagnostic().diagnosticType() = Diagnostic990;
    Itin *mother = createItin();
    Itin *child = createItin();

    ValidatingCxrGSAData v_mother;
    ValidatingCxrGSAData v_child;
    ValidatingCxrDataMap vcm_mother;
    ValidatingCxrDataMap vcm_child;
    vcx::ValidatingCxrData vcd;

    vcm_mother["AA"] = vcd;
    vcm_mother["EK"] = vcd;
    vcm_mother["BA"] = vcd;
    v_mother.validatingCarriersData() = vcm_mother;
    mother->validatingCxrGsaData() = &v_mother;

    vcm_child["JJ"] = vcd;
    vcm_child["AB"] = vcd;
    vcm_child["EK"] = vcd;
    v_child.validatingCarriersData() = vcm_child;
    child->validatingCxrGsaData() = &v_child;

    FarePath *clonedFP = _memHandle.create<FarePath>();
    std::vector<CarrierCode> fpCxrVec;
    fpCxrVec.push_back("AA");
    fpCxrVec.push_back("BA");
    clonedFP->validatingCarriers() = fpCxrVec;

    // Now puputale the FarePath with PU and FUs
    PricingUnit *pu = _memHandle.create<PricingUnit>();
    FareUsage *fu1 = _memHandle.create<FareUsage>();
    FareUsage *fu2 = _memHandle.create<FareUsage>();
    PaxTypeFare *ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare *ptf2 = _memHandle.create<PaxTypeFare>();

    std::vector<CarrierCode> ptf1Vec;
    std::vector<CarrierCode> ptf2Vec;

    ptf1Vec.push_back("AA");
    ptf1Vec.push_back("AB");

    ptf2Vec.push_back("AA");
    ptf2Vec.push_back("EK");

    ptf1->validatingCarriers() = ptf1Vec;
    ptf2->validatingCarriers() = ptf2Vec;

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    pu->fareUsage().push_back(fu1);
    pu->fareUsage().push_back(fu2);
    clonedFP->pricingUnit().push_back(pu);

    CPPUNIT_ASSERT_EQUAL(false, _po->validatingCxrForClonedFarePath(*_trx, 0, *mother, *child, clonedFP, diag));

    std::string expectedDiagResponse(" CLONED FAREPATH IS INVALID FOR VALIDATING CARRIER INTERLINE \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }

  void testValidatingCxrForClonedFarePath_shouldPass()
  {
    Diag990Collector diag;
    diag.activate();
    _trx->diagnostic().diagnosticType() = Diagnostic990;

    Itin *mother = createItin();
    Itin *child = createItin();

    ValidatingCxrGSAData v_mother;
    ValidatingCxrGSAData v_child;
    ValidatingCxrDataMap vcm_mother;
    ValidatingCxrDataMap vcm_child;
    vcx::ValidatingCxrData vcd;

    vcm_mother["AA"] = vcd;
    vcm_mother["EK"] = vcd;
    vcm_mother["BA"] = vcd;
    v_mother.validatingCarriersData() = vcm_mother;
    mother->validatingCxrGsaData() = &v_mother;

    vcm_child["AA"] = vcd;
    vcm_child["AB"] = vcd;
    vcm_child["AF"] = vcd;
    vcm_child["EK"] = vcd;
    v_child.validatingCarriersData() = vcm_child;
    child->validatingCxrGsaData() = &v_child;

    FarePath *clonedFP = _memHandle.create<FarePath>();
    std::vector<CarrierCode> fpCxrVec;
    fpCxrVec.push_back("AA");
    fpCxrVec.push_back("BA");
    clonedFP->validatingCarriers() = fpCxrVec;

    // Now puputale the FarePath with PU and FUs
    PricingUnit *pu = _memHandle.create<PricingUnit>();
    FareUsage *fu1 = _memHandle.create<FareUsage>();
    FareUsage *fu2 = _memHandle.create<FareUsage>();
    PaxTypeFare *ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare *ptf2 = _memHandle.create<PaxTypeFare>();

    std::vector<CarrierCode> ptf1Vec;
    std::vector<CarrierCode> ptf2Vec;

    ptf1Vec.push_back("BB");
    ptf1Vec.push_back("AB");
    ptf1Vec.push_back("AF");

    ptf2Vec.push_back("BB");
    ptf2Vec.push_back("EK");
    ptf2Vec.push_back("AF");

    ptf1->validatingCarriers() = ptf1Vec;
    ptf2->validatingCarriers() = ptf2Vec;

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    pu->fareUsage().push_back(fu1);
    pu->fareUsage().push_back(fu2);
    clonedFP->pricingUnit().push_back(pu);

    CPPUNIT_ASSERT_EQUAL(true, _po->validatingCxrForClonedFarePath(*_trx, 0, *mother, *child, clonedFP, diag));
    CPPUNIT_ASSERT(2 == clonedFP->validatingCarriers().size());

    std::string expectedDiagResponse(" CLONED FAREPATH VALID FOR GSA. VALIDATING CXR LIST: AA  AF  \n");
    CPPUNIT_ASSERT_EQUAL(expectedDiagResponse, diag.str());
  }*/

protected:
  // Data members
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingOrchestrator* _po;

  FarePath* _farePath;
  GroupFarePath* _gfp;
  Itin* _itin;
  PricingUnit* _pu;
  FareUsage* _fu;
  PaxTypeFare* _ptf;
  FareMarket* _fm;
  Fare* _fare;
  FareInfo* _fareInfo;
  std::vector<FarePath*> _farePathVec;

  // class overrides
  class LocMock : public Loc
  {
  public:
    LocMock(const LocCode& code) { _loc = code; }
  };

  class TravelSegMock : public AirSeg
  {
  public:
    TravelSegMock(const Loc* orig, const Loc* dest, int order)
    {
      origin() = orig;
      destination() = dest;
      origAirport() = orig->loc();
      destAirport() = dest->loc();
      boardMultiCity() = orig->loc();
      offMultiCity() = dest->loc();
      segmentOrder() = order;
      departureDT() = DateTime::localTime();
    }
  };

  class FareMarketMock : public FareMarket
  {
  public:
    FareMarketMock(Loc* orig, Loc* dest, PaxType* paxType)
    {
      _origin = orig;
      _destination = dest;
      boardMultiCity() = orig->loc();
      offMultiCity() = dest->loc();

      setGlobalDirection(GlobalDirection::WH);
      governingCarrier() = "AA";

      // LON-BA-NYC    /CXR-BA/ #GI-XX#  .OUT.
      // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   1845.26

      PaxTypeBucket* paxTypeCortege = _memHandle.create<PaxTypeBucket>();

      for (uint16_t i = 0; i < 5; ++i)
      {
        Fare* fare = _memHandle.create<Fare>();
        fare->nucFareAmount() = 1845.26 + i;

        FareInfo* fareInfo = _memHandle.create<FareInfo>();
        fareInfo->_carrier = "BA";
        fareInfo->_market1 = orig->loc();
        fareInfo->_market2 = dest->loc();
        fareInfo->_fareClass = "WMLUQOW";
        fareInfo->_fareAmount = 999.00 + i;
        fareInfo->_currency = "USD";
        // fareInfo->_currency = "GBP";

        if (i % 3 == 0)
          fareInfo->_owrt = ONE_WAY_MAY_BE_DOUBLED;
        else if (i % 3 == 1)
          fareInfo->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
        else
          fareInfo->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;

        fareInfo->_ruleNumber = "5135";
        fareInfo->_routingNumber = "XXXX";

        if (i % 3 == 0)
          fareInfo->_directionality = FROM;
        else if (i % 3 == 1)
          fareInfo->_directionality = TO;
        else
          fareInfo->_directionality = BOTH;

        if (i % 2 == 0)
          fareInfo->_globalDirection = GlobalDirection::WH;
        else
          fareInfo->_globalDirection = GlobalDirection::AT;

        fareInfo->_vendor = Vendor::ATPCO;

        TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
        tariffRefInfo->_fareTariffCode = "TAFPBA";

        fare->initialize(Fare::FS_International, fareInfo, *this, tariffRefInfo);

        PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
        paxTypeFare->initialize(fare, paxType, this);

        FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
        appInfo->_fareType = "EU";
        paxTypeFare->fareClassAppInfo() = appInfo;

        FareClassAppSegInfo* fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();
        fareClassAppSegInfo->_paxType = "ADT";
        paxTypeFare->fareClassAppSegInfo() = fareClassAppSegInfo;

        paxTypeCortege->requestedPaxType() = paxType;
        paxTypeCortege->paxTypeFare().push_back(paxTypeFare);
      }

      this->paxTypeCortege().push_back(*paxTypeCortege);
    }

    ~FareMarketMock() { _memHandle.clear(); }

  protected:
    TestMemHandle _memHandle;
  };

  class PoDataHandleMock : public DataHandleMock
  {
  public:
    PoDataHandleMock() : _generalRuleAppTariffRule(false), _mileage(NULL)
    {
      _getGeneralRuleAppTariffRulePtr = &_generalRuleAppTariffRule;
      //_getGeneralFareRuleVendorCodeCarrierCodeTariffNumberRuleNumberCatNumberDateTimeDateTimePtr
      _getGeneralFareRulePtr = &_generalFareRule;
      //_getCombinabilityRulePtr = &_combinabilityRule;
      _getFCLimitationPtr = &_fCLimitation;
      _getMileageLocCodeLocCodeIndicatorGlobalDirectionDateTimePtr = &_mileage;
    }

  protected:
    bool _generalRuleAppTariffRule;
    std::vector<GeneralFareRuleInfo*> _generalFareRule;
    std::vector<CombinabilityRuleInfo*> _combinabilityRule;
    std::vector<LimitationFare*> _fCLimitation;
    Mileage* _mileage;
  };

  class TseServerMock : public TseServer
  {
  };

  class PricingOrchestratorMock : public PricingOrchestrator
  {
  public:
    PricingOrchestratorMock(TseServer& srv) : PricingOrchestrator(srv) {}

    bool initFpFactory(GroupFarePathFactory& factory) const { return true; }
  };

  void initRoutingFP()
  {
    _farePath = _memHandle.create<FarePath>();
    _pu = _memHandle.create<PricingUnit>();
    _fu  = _memHandle.create<FareUsage>();
    _ptf  = _memHandle.create<PaxTypeFare>();
    _fm = _memHandle.create<FareMarket>();
    _fare  = _memHandle.create<Fare>();
    _fareInfo  = _memHandle.create<FareInfo>();

    _farePath->pricingUnit().push_back(_pu);
    _pu->fareUsage().push_back(_fu);
    _fu->paxTypeFare() = _ptf;
    _ptf->setFare(_fare);
    _ptf->fareMarket() = _fm;
    _fare->setFareInfo(_fareInfo);

    _farePathVec.push_back(_farePath);
  }

  void initRegularHasFP()
  {
    _trx->setFlexFare(false);
    _farePath = _memHandle.create<FarePath>();
    _gfp = _memHandle.create<GroupFarePath>();
    _itin = _memHandle.create<Itin>();
    _itin->farePath() += _farePath;
  }

  void initRegularNoFP()
  {
    _trx->setFlexFare(false);
    _gfp = _memHandle.create<GroupFarePath>();
    _itin = _memHandle.create<Itin>();
  }

  void initFlexFaresHasFP()
  {
    _trx->setFlexFare(true);
    FarePath* farePath2, *farePath3;
    _farePath = _memHandle.create<FarePath>();
    farePath2 = _memHandle.create<FarePath>();
    farePath3 = _memHandle.create<FarePath>();
    _farePath->setFlexFaresGroupId(1u);
    farePath2->setFlexFaresGroupId(1u);
    farePath3->setFlexFaresGroupId(3u);
    FPPQItem* fppqItem = _memHandle.create<FPPQItem>();
    fppqItem->farePath() = farePath2;
    _gfp = _memHandle.create<GroupFarePath>();
    _gfp->groupFPPQItem() += fppqItem;
    _itin = _memHandle.create<Itin>();
    _itin->farePath() += _farePath, farePath3;
  }

  void initFlexFaresNoFP()
  {
    _trx->setFlexFare(true);
    FarePath* farePath1, *farePath2, *farePath3;
    farePath1 = _memHandle.create<FarePath>();
    farePath2 = _memHandle.create<FarePath>();
    farePath3 = _memHandle.create<FarePath>();
    farePath1->setFlexFaresGroupId(1u);
    farePath2->setFlexFaresGroupId(2u);
    farePath3->setFlexFaresGroupId(3u);
    FPPQItem* fppqItem = _memHandle.create<FPPQItem>();
    fppqItem->farePath() = farePath2;
    _gfp = _memHandle.create<GroupFarePath>();
    _gfp->groupFPPQItem() += fppqItem;
    _itin = _memHandle.create<Itin>();
    _itin->farePath() += farePath1, farePath3;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingOrchestratorTest);
}
