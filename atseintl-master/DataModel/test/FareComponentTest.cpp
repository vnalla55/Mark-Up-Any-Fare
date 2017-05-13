#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
class FakeFareComponent : public FareCompInfo
{
  friend class FareComponentTest;

public:
  FakeFareComponent() : FareCompInfo() {}
};

class FareComponentTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareComponentTest);
  CPPUNIT_TEST(testDefaultValue);
  CPPUNIT_TEST(testSetFareCompNum);
  CPPUNIT_TEST(testSetFareCalcFareAmt);
  CPPUNIT_TEST(testSetFareBasisCode);
  CPPUNIT_TEST(testSetHip);
  CPPUNIT_TEST(testSetFareMarket);
  CPPUNIT_TEST(testDataHandleAllocate);
  CPPUNIT_TEST(testGetPaxTypeBucket_primaryFM);
  CPPUNIT_TEST(testGetPaxTypeBucket_secondaryFM);
  CPPUNIT_TEST(testUpdateFareMarketRefund);
  CPPUNIT_TEST(testUpdateFareMarketReissue);
  CPPUNIT_TEST(testUpdateFareMarket_empty);
  CPPUNIT_TEST(testUpdateFMMappingWhenUnflownFC);
  CPPUNIT_TEST(testUpdateFMMappingWhenSameFareBreaks);
  CPPUNIT_TEST(testUpdateFMMappingWhenFlownFCincludedInExcFC);
  CPPUNIT_TEST(testUpdateFMMappingWhenExcFCincludedInFlownFC);
  CPPUNIT_TEST(testUpdateFMMappingWhenFlownFChasSharedFirstSegment);
  CPPUNIT_TEST(testUpdateFMMappingWhenFlownFChasSharedLastSegment);
  CPPUNIT_TEST(testUpdateFMMappingWhenSideTripAnd3FMmapped);
  CPPUNIT_TEST(testUpdateFMMappingWhenSideTripAnd1FMmapped);
  CPPUNIT_TEST(testGetMultiNewItinData);
  CPPUNIT_TEST(testCloneAdditionalFareMarket);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _fc = _memHandle.create<FareCompInfo>();
    _seg1 = _memHandle.create<AirSeg>();
    _seg2 = _memHandle.create<AirSeg>();
    _seg3 = _memHandle.create<AirSeg>();
    _itin = _memHandle.create<Itin>();

    _seg1->segmentOrder() = 1;
    _seg1->unflown() = false;
    _seg2->segmentOrder() = 2;
    _seg2->unflown() = false;
    _seg3->segmentOrder() = 3;
    _seg3->unflown() = false;
  }

  void setUpConfiguration1()
  {
    _pt = _memHandle.create<PaxType>();
    _frbtrx = _memHandle.create<MockRexTrx>();
    _frbtrx->paxType().push_back(_pt);
    _fm = createFareMarket(_pt);
    _ffc = _memHandle.create<FakeFareComponent>();
    _excItin = _memHandle.create<ExcItin>();
    _fares = _memHandle.create<std::vector<PaxTypeFare*> >();
    _ffc->fareMarket() = _fm;
    _frbtrx->exchangeItin().push_back(_excItin);
  }

  void setUpConfiguration2()
  {
    setUpConfiguration1();
    _fares->push_back(_memHandle.create<PaxTypeFare>());
    _fares->push_back(_memHandle.create<PaxTypeFare>());
    _ffc->matchedFares().insert(_ffc->matchedFares().end(), _fares->begin(), _fares->end());
  }

  void tearDown() { _memHandle.clear(); }

  FareMarket* createFareMarket(AirSeg* seg1, AirSeg* seg2 = 0, AirSeg* seg3 = 0, AirSeg* seg4 = 0)
  {
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    fareMarket->travelSeg().push_back(seg1);
    if (seg2)
      fareMarket->travelSeg().push_back(seg2);
    if (seg3)
      fareMarket->travelSeg().push_back(seg3);
    if (seg4)
      fareMarket->travelSeg().push_back(seg4);

    return fareMarket;
  }

  void testDefaultValue()
  {
    CPPUNIT_ASSERT(_fc->fareCompNumber() == 0);
    CPPUNIT_ASSERT(_fc->fareCalcFareAmt() == 0);
    CPPUNIT_ASSERT(_fc->fareBasisCode().empty());
    CPPUNIT_ASSERT(_fc->hip() == 0);
    CPPUNIT_ASSERT(_fc->fareMarket() == 0);
  }

  void testSetFareCompNum()
  {
    _fc->fareCompNumber() = 1;
    CPPUNIT_ASSERT(_fc->fareCompNumber() == 1);
  }

  void testSetFareCalcFareAmt()
  {
    _fc->fareCalcFareAmt() = 100.00;
    CPPUNIT_ASSERT(_fc->fareCalcFareAmt() == 100.00);
  }

  void testSetFareBasisCode()
  {
    _fc->fareBasisCode() = "YEXT";
    CPPUNIT_ASSERT(_fc->fareBasisCode() == "YEXT");
  }

  void testSetHip()
  {
    MinFarePlusUpItem hip;
    _fc->hip() = &hip;
    CPPUNIT_ASSERT(_fc->hipIncluded());
  }

  void testSetFareMarket()
  {
    FareMarket fm;
    _fc->fareMarket() = &fm;
    CPPUNIT_ASSERT(_fc->fareMarket() != 0);
  }

  void testDataHandleAllocate()
  {
    RexPricingTrx trx;
    FareCompInfo* newFc = 0;
    trx.dataHandle().get(newFc);
    CPPUNIT_ASSERT(newFc != 0);
    CPPUNIT_ASSERT(newFc->fareMarket() == 0);
    CPPUNIT_ASSERT(newFc->fareCompNumber() == 0);
    CPPUNIT_ASSERT(newFc->fareBasisCode().empty());

    ExcItin* exchangeItin = 0;
    trx.dataHandle().get(exchangeItin);
    CPPUNIT_ASSERT(exchangeItin != 0);
    trx.exchangeItin().push_back(exchangeItin);

    AirSeg* tvlSeg = 0;
    trx.dataHandle().get(tvlSeg);
    CPPUNIT_ASSERT(tvlSeg != 0);
    exchangeItin->travelSeg().push_back(tvlSeg);

    exchangeItin->fareComponent().push_back(newFc);
  }

  void testGetPaxTypeBucket_primaryFM()
  {
    RexPricingTrx trx;
    PaxType pt;
    trx.paxType().push_back(&pt);

    PaxTypeBucket ptc;
    ptc.requestedPaxType() = &pt;

    FareMarket fm;
    fm.paxTypeCortege().push_back(ptc);

    FareCompInfo fc;
    _fc->fareMarket() = &fm;

    PaxTypeBucket* retptc = _fc->getPaxTypeBucket(trx);

    CPPUNIT_ASSERT_EQUAL(retptc->requestedPaxType(), &pt);
  }

  void testGetPaxTypeBucket_secondaryFM()
  {
    RexPricingTrx trx;
    PaxType pt;
    trx.paxType().push_back(&pt);

    PaxTypeBucket ptc;
    ptc.requestedPaxType() = &pt;

    FareMarket fm;
    fm.paxTypeCortege().push_back(ptc);

    FareCompInfo fc;
    _fc->secondaryFareMarket() = &fm;

    PaxTypeBucket* retptc = _fc->getPaxTypeBucket(trx, true);

    CPPUNIT_ASSERT_EQUAL(retptc->requestedPaxType(), &pt);
  }
  class MockRexTrx : public RexPricingTrx
  {
  public:
    virtual const DateTime& getRuleApplicationDate(const CarrierCode& govCarrier) const
    {
      return _rad;
    }

    MockRexTrx() : _rad(2008, 01, 20) {}
    virtual ~MockRexTrx() {}

  protected:
    DateTime _rad;
  };

  FareMarket* createFareMarket(PaxType* pt)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    PaxTypeBucket* cortege = _memHandle.create<PaxTypeBucket>();
    cortege->requestedPaxType() = pt;
    cortege->paxTypeFare() = std::vector<PaxTypeFare*>(15, _memHandle.create<PaxTypeFare>());

    fm->paxTypeCortege().push_back(*cortege);
    fm->allPaxTypeFare() = std::vector<PaxTypeFare*>(10, _memHandle.create<PaxTypeFare>());

    return fm;
  }

  void testUpdateFareMarketRefund()
  {
    setUpConfiguration2();
    _frbtrx->setExcTrxType(PricingTrx::AF_EXC_TRX);
    _ffc->updateFareMarket(*_frbtrx);

    CPPUNIT_ASSERT_EQUAL(*_fares, _fm->allPaxTypeFare());
    CPPUNIT_ASSERT_EQUAL(*_fares, _fm->paxTypeCortege(_pt)->paxTypeFare());
    CPPUNIT_ASSERT(_fm->serviceStatus().isSet(FareMarket::RexFareSelector));
  }

  void testUpdateFareMarketReissue()
  {
    setUpConfiguration2();
    _frbtrx->setExcTrxType(PricingTrx::AR_EXC_TRX);

    CPPUNIT_ASSERT_EQUAL(size_t(10), _fm->allPaxTypeFare().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _ffc->_otherFares.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ffc->matchedFares().size());

    _ffc->updateFareMarket(*_frbtrx);

    CPPUNIT_ASSERT(_fm->serviceStatus().isSet(FareMarket::RexFareSelector));

    CPPUNIT_ASSERT_EQUAL(size_t(2), _fm->allPaxTypeFare().size());
    CPPUNIT_ASSERT_EQUAL(size_t(10), _ffc->_otherFares.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ffc->matchedFares().size());
  }

  void testUpdateFareMarket_empty()
  {
    setUpConfiguration1();
    _ffc->updateFareMarket(*_frbtrx);

    CPPUNIT_ASSERT_EQUAL(*_fares, _fm->allPaxTypeFare());
    CPPUNIT_ASSERT_EQUAL(*_fares, _fm->paxTypeCortege(_pt)->paxTypeFare());

    CPPUNIT_ASSERT(!_fm->serviceStatus().isSet(FareMarket::RexFareSelector));
  }

  void testUpdateFMMappingWhenUnflownFC()
  {
    _seg1->unflown() = true;
    _fc->fareMarket() = createFareMarket(_seg1, _seg2);
    _itin->fareMarket().push_back(createFareMarket(_seg1, _seg2));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT(_fc->_mappedFC.empty());
  }

  void testUpdateFMMappingWhenSameFareBreaks()
  {
    _fc->fareMarket() = createFareMarket(_seg1, _seg2, _seg3);
    _itin->fareMarket().push_back(createFareMarket(_seg1, _seg2, _seg3));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
  }

  void testUpdateFMMappingWhenFlownFCincludedInExcFC()
  {
    _fc->fareMarket() = createFareMarket(_seg1, _seg2, _seg3);
    _itin->fareMarket().push_back(createFareMarket(_seg2));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
  }

  void testUpdateFMMappingWhenExcFCincludedInFlownFC()
  {
    _fc->fareMarket() = createFareMarket(_seg2);
    _itin->fareMarket().push_back(createFareMarket(_seg1, _seg2, _seg3));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
  }

  void testUpdateFMMappingWhenFlownFChasSharedFirstSegment()
  {
    _fc->fareMarket() = createFareMarket(_seg2, _seg3);
    _itin->fareMarket().push_back(createFareMarket(_seg1, _seg2));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
  }

  void testUpdateFMMappingWhenFlownFChasSharedLastSegment()
  {
    _fc->fareMarket() = createFareMarket(_seg1, _seg2);
    _itin->fareMarket().push_back(createFareMarket(_seg2, _seg3));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
  }

  void testUpdateFMMappingWhenSideTripAnd3FMmapped()
  {
    AirSeg seg4;
    seg4.segmentOrder() = 4;
    seg4.unflown() = false;
    AirSeg seg5;
    seg5.segmentOrder() = 5;
    seg5.unflown() = false;
    AirSeg seg6;
    seg6.segmentOrder() = 6;
    seg6.unflown() = false;

    _fc->fareMarket() = createFareMarket(_seg1, _seg2, &seg5, &seg6);
    _itin->fareMarket().push_back(createFareMarket(_seg1, &seg6));
    _itin->fareMarket().push_back(createFareMarket(_seg2, _seg3));
    _itin->fareMarket().push_back(createFareMarket(&seg4, &seg5));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _fc->_mappedFC.size());

    _fc->_mappedFC.clear();
    _fc->fareMarket() = createFareMarket(_seg3);
    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
    CPPUNIT_ASSERT_EQUAL(*_fc->_mappedFC.begin(), _itin->fareMarket()[1]);

    _fc->_mappedFC.clear();
    _fc->fareMarket() = createFareMarket(&seg4);
    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
    CPPUNIT_ASSERT_EQUAL(*_fc->_mappedFC.begin(), _itin->fareMarket()[2]);
  }

  void testUpdateFMMappingWhenSideTripAnd1FMmapped()
  {
    AirSeg seg4;
    seg4.segmentOrder() = 4;
    seg4.unflown() = false;
    AirSeg seg5;
    seg5.segmentOrder() = 5;
    seg5.unflown() = false;
    AirSeg seg6;
    seg6.segmentOrder() = 6;
    seg6.unflown() = false;

    _fc->fareMarket() = createFareMarket(_seg1, &seg6);
    _itin->fareMarket().push_back(createFareMarket(_seg1, _seg2, &seg5, &seg6));
    _itin->fareMarket().push_back(createFareMarket(_seg3));
    _itin->fareMarket().push_back(createFareMarket(&seg4));

    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fc->_mappedFC.size());
    CPPUNIT_ASSERT_EQUAL(*_fc->_mappedFC.begin(), _itin->fareMarket()[0]);

    _fc->_mappedFC.clear();
    _fc->fareMarket() = createFareMarket(_seg2, _seg3);
    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _fc->_mappedFC.size());
    CPPUNIT_ASSERT(_fc->_mappedFC.find(_itin->fareMarket()[0]) != _fc->_mappedFC.end());
    CPPUNIT_ASSERT(_fc->_mappedFC.find(_itin->fareMarket()[1]) != _fc->_mappedFC.end());

    _fc->_mappedFC.clear();
    _fc->fareMarket() = createFareMarket(&seg4, &seg5);
    _fc->updateFMMapping(_itin);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _fc->_mappedFC.size());
    CPPUNIT_ASSERT(_fc->_mappedFC.find(_itin->fareMarket()[0]) != _fc->_mappedFC.end());
    CPPUNIT_ASSERT(_fc->_mappedFC.find(_itin->fareMarket()[2]) != _fc->_mappedFC.end());
  }

  void testGetMultiNewItinData()
  {
    uint16_t itinIndex = 0;
    FakeFareComponent fc;
    CPPUNIT_ASSERT_EQUAL(fc._multiNewItinData.size(), size_t(0));

    FareCompInfo::MultiNewItinData multiNewItinData = fc.getMultiNewItinData();
    CPPUNIT_ASSERT_EQUAL(fc._multiNewItinData.size(), size_t(1));

    FareCompInfo::MultiNewItinData multiNewItinDataFisrt = fc.getMultiNewItinData(itinIndex);
    CPPUNIT_ASSERT_EQUAL(fc._multiNewItinData.size(), size_t(1));
  }

  void testCloneAdditionalFareMarket()
  {
    setUpConfiguration2();
    _ffc->_otherFares.push_back(_memHandle.create<PaxTypeFare>());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _frbtrx->fareMarket().size());
    _ffc->loadOtherFares(*_frbtrx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _frbtrx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(_ffc->_otherFares, _frbtrx->fareMarket().front()->allPaxTypeFare());
    CPPUNIT_ASSERT_EQUAL(_ffc->_otherFares.front()->fareMarket(), _frbtrx->fareMarket().front());
    CPPUNIT_ASSERT(_frbtrx->fareMarket().front()->breakIndicator());
    CPPUNIT_ASSERT(!_frbtrx->fareMarket().front()->fareCompInfo());
  }

private:
  TestMemHandle _memHandle;
  FareCompInfo* _fc;
  AirSeg* _seg1;
  AirSeg* _seg2;
  AirSeg* _seg3;
  Itin* _itin;
  FareMarket* _fm;
  PaxType* _pt;
  MockRexTrx* _frbtrx;
  FakeFareComponent* _ffc;
  ExcItin* _excItin;
  std::vector<PaxTypeFare*>* _fares;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareComponentTest);
}
