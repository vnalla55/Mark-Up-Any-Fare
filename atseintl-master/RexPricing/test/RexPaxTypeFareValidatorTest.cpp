#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include "RexPricing/RexPaxTypeFareValidator.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/Diag602Collector.h"
#include "Common/Vendor.h"

#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{
class MyDiag602Collector : public Diag602Collector
{
public:
  Diag602Collector& operator<<(const PaxTypeFare& ptf) { return *this; }
};

class MyRexPaxTypeFareValidator : public RexPaxTypeFareValidator
{
public:
  MyRexPaxTypeFareValidator(RexPricingTrx& rexTrx) : RexPaxTypeFareValidator(rexTrx) {}

  std::string _diagOutput;

protected:
  DCFactory* setupDiagnostic(Diag602Collector*& dc602)
  {
    dc602 = _memHandle.create<MyDiag602Collector>();
    dc602->rootDiag() = &_rexTrx.diagnostic();
    dc602->trx() = &_rexTrx;
    dc602->activate();

    return 0;
  }
  void releaseDiagnostic(DCFactory* factory, Diag602Collector*& dc602)
  {
    _diagOutput = dc602->str();
  }

  TestMemHandle _memHandle;
};

class RexPaxTypeFareValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexPaxTypeFareValidatorTest);
  CPPUNIT_TEST(testGet1FlownFare);
  CPPUNIT_TEST(testGetNoFlownFares);
  CPPUNIT_TEST(testGet2FlownFares);
  CPPUNIT_TEST(testWhenNotMatchFareBreaks);
  CPPUNIT_TEST(testWhenMatchOutboundFareBreaksAndGreaterAmountOnExchange);
  CPPUNIT_TEST(testWhenMatchInboundFareBreaksAndGreaterAmountOnExchange);
  CPPUNIT_TEST(testWhenMatchOutboundFareBreaksAndSameAmount);
  CPPUNIT_TEST(testWhenMatchInboundFareBreaksAndSameAmount);
  CPPUNIT_TEST(testWhenMatch2ExcFCsWithOneOnRepriceAndGreaterAmountOnExc);
  CPPUNIT_TEST(testWhenMatch2ExcFCsWithOneOnRepriceAndSameAmount);
  CPPUNIT_TEST(testWhenMatchExcFCWith2RepriceFCsAndGreaterAmountOnExc);
  CPPUNIT_TEST(testWhenMatchExcFCWith2RepriceFCsAndSameAmount);
  CPPUNIT_TEST(testWhenFailOnSecondCombination);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _rexTrx = _memHandle.create<RexPricingTrx>();
    _rexTrx->diagnostic().diagnosticType() = Diagnostic609;
    _rexTrx->diagnostic().activate();

    _dc = DCFactory::instance()->create(*_rexTrx);
    if (_dc)
      _dc->enable(Diagnostic609);

    _rexTrx->diagnostic().diagnosticType() = Diagnostic602;
    _rexTrx->diagnostic().activate();

    _exchangeItin = _memHandle.create<ExcItin>();
    _rexTrx->exchangeItin().push_back(_exchangeItin);
    _ptfValidator =
        _memHandle.insert<MyRexPaxTypeFareValidator>(new MyRexPaxTypeFareValidator(*_rexTrx));
    _ptfValidatorDiag =
        _memHandle.insert<RexPaxTypeFareValidator>(new RexPaxTypeFareValidator(*_rexTrx, _dc));
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _seg1 = _memHandle.create<AirSeg>();
    _seg2 = _memHandle.create<AirSeg>();
    _seg3 = _memHandle.create<AirSeg>();
    _seg4 = _memHandle.create<AirSeg>();
    _seg5 = _memHandle.create<AirSeg>();
    _seg6 = _memHandle.create<AirSeg>();
    _excFarePath = _memHandle.create<FarePath>();
    _farePath = _memHandle.create<FarePath>();

    _seg1->segmentOrder() = 1;
    _seg2->segmentOrder() = 2;
    _seg3->segmentOrder() = 3;
    _seg4->segmentOrder() = 4;
    _seg5->segmentOrder() = 5;
    _seg6->segmentOrder() = 6;
    _paxTypeFare->setFare(_fare);
    _exchangeItin->farePath().push_back(_excFarePath);
  }

  void tearDown()
  {
    if (_dc != 0)
    {
      _dc->flushMsg();
      _dc = 0;
    }
    _memHandle.clear();
  }

  void addOneWayPU(FarePath*& farePath,
                   FCChangeStatus changeStatus,
                   TravelSeg* seg1,
                   TravelSeg* seg2,
                   MoneyAmount amount)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    fareMarket->boardMultiCity() = "LAX";
    fareMarket->offMultiCity() = "CHI";
    fareMarket->changeStatus() = changeStatus;
    farePath->pricingUnit().push_back(pu);
    pu->fareUsage().push_back(fu);
    fu->paxTypeFare() = ptf;
    ptf->fareMarket() = fareMarket;
    ptf->setFare(fare);
    fareMarket->travelSeg().push_back(seg1);
    fareMarket->travelSeg().push_back(seg2);
    fare->nucFareAmount() = amount;
  }

  void addOpenJawPU(FarePath*& farePath,
                    FCChangeStatus outboundChangeStatus,
                    TravelSeg* seg1,
                    TravelSeg* seg2,
                    MoneyAmount outboundAmount,
                    FCChangeStatus inboundChangeStatus,
                    TravelSeg* seg3,
                    TravelSeg* seg4,
                    MoneyAmount inboundAmount)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* outboundFU = _memHandle.create<FareUsage>();
    FareUsage* inboundFU = _memHandle.create<FareUsage>();
    PaxTypeFare* outboundPTF = _memHandle.create<PaxTypeFare>();
    Fare* outboundFare = _memHandle.create<Fare>();
    PaxTypeFare* inboundPTF = _memHandle.create<PaxTypeFare>();
    Fare* inboundFare = _memHandle.create<Fare>();
    FareMarket* outboundFareMarket = _memHandle.create<FareMarket>();
    FareMarket* inboundFareMarket = _memHandle.create<FareMarket>();

    outboundFareMarket->boardMultiCity() = "CHI";
    outboundFareMarket->offMultiCity() = "LON";
    inboundFareMarket->boardMultiCity() = "LON";
    inboundFareMarket->offMultiCity() = "LAX";
    outboundFareMarket->changeStatus() = outboundChangeStatus;
    inboundFareMarket->changeStatus() = inboundChangeStatus;
    farePath->pricingUnit().push_back(pu);
    pu->fareUsage().push_back(outboundFU);
    outboundFU->paxTypeFare() = outboundPTF;
    outboundPTF->fareMarket() = outboundFareMarket;
    outboundPTF->setFare(outboundFare);
    outboundFareMarket->travelSeg().push_back(seg1);
    outboundFareMarket->travelSeg().push_back(seg2);
    outboundFare->nucFareAmount() = outboundAmount;

    pu->fareUsage().push_back(inboundFU);
    inboundFU->paxTypeFare() = inboundPTF;
    inboundPTF->fareMarket() = inboundFareMarket;
    inboundPTF->setFare(inboundFare);
    inboundFareMarket->travelSeg().push_back(seg3);
    inboundFareMarket->travelSeg().push_back(seg4);
    inboundFare->nucFareAmount() = inboundAmount;
  }

  void testGet1FlownFare()
  {
    addOneWayPU(_excFarePath, FL, _seg1, _seg2, 100);

    CPPUNIT_ASSERT(_ptfValidator->collectFullyFlownFares(_ptfValidator->_excFares,
                                                         *_exchangeItin->farePath().front()));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ptfValidator->_excFares.size());
    CPPUNIT_ASSERT(_excFarePath->pricingUnit().front()->fareUsage().front()->paxTypeFare() ==
                   _ptfValidator->_excFares.front());
  }

  void testGetNoFlownFares()
  {
    addOneWayPU(_excFarePath, UU, _seg1, _seg4, 200);

    CPPUNIT_ASSERT(!_ptfValidator->collectFullyFlownFares(_ptfValidator->_excFares,
                                                          *_exchangeItin->farePath().front()));
    CPPUNIT_ASSERT_EQUAL(size_t(0), _ptfValidator->_excFares.size());
  }

  void testGet2FlownFares()
  {
    addOneWayPU(_excFarePath, FL, _seg1, _seg2, 200);
    addOpenJawPU(_excFarePath, FL, _seg3, _seg4, 127, UU, _seg5, _seg6, 128);

    CPPUNIT_ASSERT(_ptfValidator->collectFullyFlownFares(_ptfValidator->_excFares,
                                                         *_exchangeItin->farePath().front()));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ptfValidator->_excFares.size());
    CPPUNIT_ASSERT(_excFarePath->pricingUnit().front()->fareUsage().front()->paxTypeFare() ==
                   _ptfValidator->_excFares.front());
    CPPUNIT_ASSERT(_excFarePath->pricingUnit()[1]->fareUsage().front()->paxTypeFare() ==
                   _ptfValidator->_excFares[1]);
  }

  void testWhenNotMatchFareBreaks()
  {
    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg1);

    CPPUNIT_ASSERT(_ptfValidator->validate(*_paxTypeFare));
  }

  void testWhenMatchOutboundFareBreaksAndGreaterAmountOnExchange()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);

    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg1);
    fareMarket.travelSeg().push_back(_seg2);
    _fare->nucFareAmount() = 126;

    CPPUNIT_ASSERT(!_ptfValidator->validate(*_paxTypeFare));
    CPPUNIT_ASSERT_EQUAL(string(" FAILED: EXCHANGE FARE AMOUNT: 127 NUC IS GREATER THAN\n"
                                " REPRICE SOLUTION FARE AMOUNT: 126 NUC\n"),
                         _ptfValidator->_diagOutput);
  }

  void testWhenMatchInboundFareBreaksAndGreaterAmountOnExchange()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);

    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg3);
    fareMarket.travelSeg().push_back(_seg4);
    _fare->nucFareAmount() = 127;

    CPPUNIT_ASSERT(!_ptfValidator->validate(*_paxTypeFare));
    CPPUNIT_ASSERT_EQUAL(string(" FAILED: EXCHANGE FARE AMOUNT: 128 NUC IS GREATER THAN\n"
                                " REPRICE SOLUTION FARE AMOUNT: 127 NUC\n"),
                         _ptfValidator->_diagOutput);
  }

  void testWhenMatchOutboundFareBreaksAndSameAmount()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);

    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg1);
    fareMarket.travelSeg().push_back(_seg2);
    _fare->nucFareAmount() = 127;

    CPPUNIT_ASSERT(_ptfValidator->validate(*_paxTypeFare));
  }

  void testWhenMatchInboundFareBreaksAndSameAmount()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);

    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg3);
    fareMarket.travelSeg().push_back(_seg4);
    _fare->nucFareAmount() = 128;

    CPPUNIT_ASSERT(_ptfValidator->validate(*_paxTypeFare));
  }

  void testWhenMatch2ExcFCsWithRepriceFareAndGreaterAmountOnExc()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);

    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg1);
    fareMarket.travelSeg().push_back(_seg4);
    _fare->nucFareAmount() = 254;

    CPPUNIT_ASSERT(!_ptfValidator->validate(*_paxTypeFare));
    CPPUNIT_ASSERT_EQUAL(string(" FAILED: EXCHANGE FARE AMOUNT: 255 NUC IS GREATER THAN\n"
                                " REPRICE SOLUTION FARE AMOUNT: 254 NUC\n"),
                         _ptfValidator->_diagOutput);
  }

  void testWhenMatch2ExcFCsWithRepriceFareAndSameAmount()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);

    FareMarket fareMarket;
    fareMarket.changeStatus() = FL;
    _paxTypeFare->fareMarket() = &fareMarket;
    fareMarket.travelSeg().push_back(_seg1);
    fareMarket.travelSeg().push_back(_seg4);
    _fare->nucFareAmount() = 255;

    CPPUNIT_ASSERT(_ptfValidator->validate(*_paxTypeFare));
  }

  void testWhenMatch2ExcFCsWithOneOnRepriceAndGreaterAmountOnExc()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);
    addOneWayPU(_farePath, FL, _seg1, _seg4, 254);

    CPPUNIT_ASSERT(!_ptfValidatorDiag->validate(*_farePath));
    CPPUNIT_ASSERT_EQUAL(string(" FAILED: EXCHANGE CHI-LAX FARE AMOUNT: 255 NUC IS GREATER THAN\n"
                                " REPRICE SOLUTION FARE AMOUNT: 254 NUC\n"),
                         _dc->str());
  }

  void testWhenMatch2ExcFCsWithOneOnRepriceAndSameAmount()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);
    addOneWayPU(_farePath, FL, _seg1, _seg4, 255);

    CPPUNIT_ASSERT(_ptfValidatorDiag->validate(*_farePath));
  }

  void testWhenMatchExcFCWith2RepriceFCsAndGreaterAmountOnExc()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg6, 256);
    addOpenJawPU(_farePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);
    addOneWayPU(_farePath, FL, _seg5, _seg6, 127);

    CPPUNIT_ASSERT(!_ptfValidatorDiag->validate(*_farePath));
    CPPUNIT_ASSERT_EQUAL(string(" FAILED: EXCHANGE LON-LAX FARE AMOUNT: 256 NUC IS GREATER THAN\n"
                                " REPRICE SOLUTION FARE AMOUNT: 255 NUC\n"),
                         _dc->str());
  }

  void testWhenMatchExcFCWith2RepriceFCsAndSameAmount()
  {
    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg6, 255);
    addOpenJawPU(_farePath, FL, _seg1, _seg2, 127, FL, _seg3, _seg4, 128);
    addOneWayPU(_farePath, FL, _seg5, _seg6, 127);

    CPPUNIT_ASSERT(_ptfValidatorDiag->validate(*_farePath));
  }

  void testWhenFailOnSecondCombination()
  {
    AirSeg seg7;
    seg7.segmentOrder() = 7;
    AirSeg seg8;
    seg8.segmentOrder() = 8;
    AirSeg seg9;
    seg9.segmentOrder() = 9;
    AirSeg seg10;
    seg10.segmentOrder() = 10;

    addOpenJawPU(_excFarePath, FL, _seg1, _seg2, 100, FL, _seg3, _seg4, 200);
    addOneWayPU(_excFarePath, FL, _seg5, _seg6, 400);
    addOneWayPU(_excFarePath, FL, &seg7, &seg8, 500);
    addOneWayPU(_excFarePath, FL, &seg9, &seg10, 600);
    addOneWayPU(_farePath, FL, _seg1, _seg4, 300);
    addOpenJawPU(_farePath, FL, _seg5, _seg6, 300, FL, &seg7, &seg10, 1099);

    CPPUNIT_ASSERT(!_ptfValidatorDiag->validate(*_farePath));
    CPPUNIT_ASSERT_EQUAL(string(" FAILED: EXCHANGE LAX-CHI FARE AMOUNT: 1100 NUC IS GREATER THAN\n"
                                " REPRICE SOLUTION FARE AMOUNT: 1099 NUC\n"),
                         _dc->str());
  }

protected:
  TestMemHandle _memHandle;
  RexPricingTrx* _rexTrx;
  DiagCollector* _dc;
  MyRexPaxTypeFareValidator* _ptfValidator;
  RexPaxTypeFareValidator* _ptfValidatorDiag;
  PaxTypeFare* _paxTypeFare;
  Fare* _fare;
  ExcItin* _exchangeItin;
  AirSeg* _seg1;
  AirSeg* _seg2;
  AirSeg* _seg3;
  AirSeg* _seg4;
  AirSeg* _seg5;
  AirSeg* _seg6;
  FarePath* _excFarePath;
  FarePath* _farePath;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexPaxTypeFareValidatorTest);
}
