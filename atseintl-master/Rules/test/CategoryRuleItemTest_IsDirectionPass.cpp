#include "Common/TseEnums.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "Rules/CategoryRuleItem.h"
#include "Rules/RuleConst.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class CategoryRuleItemTestIsDirectionPass : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CategoryRuleItemTestIsDirectionPass);
  CPPUNIT_TEST(testIsDirectionPass_Rtw_True);
  CPPUNIT_TEST(testIsDirectionPass_Rtw_False);
  CPPUNIT_TEST(testIsDirectionPass_AlwaysApplies);
  CPPUNIT_TEST(testIsDirectionPass_Out_NoItin_SoftPass);
  CPPUNIT_TEST(testIsDirectionPass_Out_FROM_Intl_SoftPass);
  CPPUNIT_TEST(testIsDirectionPass_Out_FROM_Domestic_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Out_TO_Fail);
  CPPUNIT_TEST(testIsDirectionPass_Out_BOTH_FirstFM_Pass);
  CPPUNIT_TEST(testIsDirectionPass_In_FROM_Intl_SoftPass);
  CPPUNIT_TEST(testIsDirectionPass_In_FROM_Domestic_Fail);
  CPPUNIT_TEST(testIsDirectionPass_In_BOTH_FirstFM_Fail);
  CPPUNIT_TEST(testIsDirectionPass_In_BOTH_TO_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir1_ForwardMatch_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir1_BackwardMatch_Fail);
  CPPUNIT_TEST(testIsDirectionPass_Dir2_ForwardMatch_Fail);
  CPPUNIT_TEST(testIsDirectionPass_Dir2_BackwardMatch_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir3_Outbound_ForwardMatch_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir3_Outbound_BackwardMach_Fail);
  CPPUNIT_TEST(testIsDirectionPass_Dir3_Inbound_ForwardMatch_Fail);
  CPPUNIT_TEST(testIsDirectionPass_Dir3_Outbound_BackwardMach_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir4_Outbound_ForwardMatch_Fail);
  CPPUNIT_TEST(testIsDirectionPass_Dir4_Outbound_BackwardMatch_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir4_Inbound_ForwardMatch_Pass);
  CPPUNIT_TEST(testIsDirectionPass_Dir4_Inbound_BackwardMatch_Fail);

  // FareUsage
  CPPUNIT_TEST(testFU_RTW_Pass);
  CPPUNIT_TEST(testFU_RTW_Fail);
  CPPUNIT_TEST(testFU_InOut_Outbound_Pass);
  CPPUNIT_TEST(testFU_InOut_Outbound_Fail);
  CPPUNIT_TEST(testFU_InOut_Inbound_Pass);
  CPPUNIT_TEST(testFU_InOut_Inbound_Fail);
  CPPUNIT_TEST(testFU_Dir_1_Pass);
  CPPUNIT_TEST(testFU_Dir_1_Fail);
  CPPUNIT_TEST(testFU_Dir_2_Pass);
  CPPUNIT_TEST(testFU_Dir_2_Fail);
  CPPUNIT_TEST(testFU_Dir_3_Pass);
  CPPUNIT_TEST(testFU_Dir_3_Fail);
  CPPUNIT_TEST(testFU_Dir_4_Pass);
  CPPUNIT_TEST(testFU_Dir_4_Fail);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;

  PricingTrx* _trx = nullptr;
  ShoppingTrx* _shTrx = nullptr;
  PricingOptions* _prOpt = nullptr;
  CategoryRuleItemInfo* _r2s = nullptr;
  TravelSeg* _segAB = nullptr, *_segBA = nullptr, *_segAC = nullptr;

public:

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _prOpt = _memHandle.create<PricingOptions>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_prOpt);
    _shTrx = _memHandle.create<ShoppingTrx>();
    _shTrx->setOptions(_prOpt);

    _r2s = _memHandle.create<CategoryRuleItemInfo>();
    _r2s->setInOutInd(RuleConst::ALWAYS_APPLIES);
    _r2s->setDirectionality(RuleConst::ALWAYS_APPLIES);

    AirSegBuilder asbld(_memHandle);
    _segAB = asbld.withLocs("AAA", "BBB").build();
    _segBA = asbld.withLocs("BBB", "AAA").build();
    _segAC = asbld.withLocs("AAA", "CCC").build();
  }

  void tearDown() { _memHandle.clear(); }

  PaxTypeFare& createFare(FareMarket& fm, Directionality dir)
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->directionality() = dir;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);

    PaxTypeFare& ptf = *_memHandle.create<PaxTypeFare>();
    ptf.setFare(fare);
    ptf.fareMarket() = &fm;

    return ptf;
  }

  void testIsDirectionPass_Rtw_True()
  {
    _prOpt->setRtw(true);
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, BOTH);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, nullptr));
  }

  void testIsDirectionPass_Rtw_False()
  {
    _prOpt->setRtw(true);
    _r2s->setInOutInd('O');
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, BOTH);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, nullptr));
  }

  void testIsDirectionPass_AlwaysApplies()
  {
    _prOpt->setRtw(true);
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, nullptr));
  }

  void testIsDirectionPass_Out_NoItin_SoftPass()
  {
    _r2s->setInOutInd('O');
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(SOFTPASS,
                         CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, nullptr));
  }

  void testIsDirectionPass_Out_FROM_Intl_SoftPass()
  {
    _r2s->setInOutInd('O');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAC}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(SOFTPASS,
                         CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Out_FROM_Domestic_Pass()
  {
    _r2s->setInOutInd('O');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAC}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Out_TO_Fail()
  {
    _r2s->setInOutInd('O');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB, _segBA})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segBA}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Out_BOTH_FirstFM_Pass()
  {
    _r2s->setInOutInd('O');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_In_FROM_Intl_SoftPass()
  {
    _r2s->setInOutInd('I');

    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAC}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(SOFTPASS,
                         CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_In_FROM_Domestic_Fail()
  {
    _r2s->setInOutInd('I');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAC}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_In_BOTH_FirstFM_Fail()
  {
    _r2s->setInOutInd('I');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, BOTH);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_In_BOTH_TO_Pass()
  {
    _r2s->setInOutInd('I');
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB, _segBA, _segAC})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAC}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir1_ForwardMatch_Pass()
  {
    _r2s->setDirectionality(RuleConst::FROM_LOC1_TO_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir1_BackwardMatch_Fail()
  {
    _r2s->setDirectionality(RuleConst::FROM_LOC1_TO_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, true, &itin));
  }

  void testIsDirectionPass_Dir2_ForwardMatch_Fail()
  {
    _r2s->setDirectionality(RuleConst::TO_LOC1_FROM_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir2_BackwardMatch_Pass()
  {
    _r2s->setDirectionality(RuleConst::TO_LOC1_FROM_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::International)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, true, &itin));
  }

  void testIsDirectionPass_Dir3_Outbound_ForwardMatch_Pass()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir3_Outbound_BackwardMach_Fail()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, true, &itin));
  }

  void testIsDirectionPass_Dir3_Inbound_ForwardMatch_Fail()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB, _segBA})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segBA}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir3_Outbound_BackwardMach_Pass()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB, _segBA})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segBA}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, true, &itin));
  }

  void testIsDirectionPass_Dir4_Outbound_ForwardMatch_Fail()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir4_Outbound_BackwardMatch_Pass()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segAB}).build();
    PaxTypeFare& ptf = createFare(fm, FROM);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, true, &itin));
  }

  void testIsDirectionPass_Dir4_Inbound_ForwardMatch_Pass()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB, _segBA})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segBA}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, false, &itin));
  }

  void testIsDirectionPass_Dir4_Inbound_BackwardMatch_Fail()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);
    Itin& itin = *ItinBuilder(_memHandle)
                      .withGeoTravelType(GeoTravelType::Domestic)
                      .withSegs({_segAB, _segBA})
                      .build();
    FareMarket& fm = *FareMarketBuilder(_memHandle).withSegs({_segBA}).build();
    PaxTypeFare& ptf = createFare(fm, TO);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ptf, _r2s, true, &itin));
  }

  // Fare Usage tests

  void testFU_RTW_Pass()
  {
    PricingOptions opt;
    opt.setRtw(true);
    _trx->setOptions(&opt);

    CategoryRuleItemInfo cfrItem;
    CategoryRuleInfo cri;
    FareUsage fu;
    cfrItem.setInOutInd(RuleConst::ALWAYS_APPLIES);

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, &cfrItem, false));
  }

  void testFU_RTW_Fail()
  {
    PricingOptions opt;
    opt.setRtw(true);
    _trx->setOptions(&opt);

    CategoryRuleItemInfo cfrItem;
    CategoryRuleInfo cri;
    FareUsage fu;
    cfrItem.setInOutInd(RuleConst::FARE_MARKET_OUTBOUND);

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, &cfrItem, false));
  }

  void testFU_InOut_Outbound_Pass()
  {
    _r2s->setInOutInd(RuleConst::FARE_MARKET_OUTBOUND);

    CategoryRuleInfo cri;
    FareUsage fu;
    fu.inbound() = false;

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, false));
  }

  void testFU_InOut_Outbound_Fail()
  {
    _r2s->setInOutInd(RuleConst::FARE_MARKET_OUTBOUND);

    CategoryRuleInfo cri;
    FareUsage fu;
    fu.inbound() = true;

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, false));
  }

  void testFU_InOut_Inbound_Pass()
  {
    _r2s->setInOutInd(RuleConst::FARE_MARKET_INBOUND);

    CategoryRuleInfo cri;
    FareUsage fu;
    fu.inbound() = true;

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, false));
  }

  void testFU_InOut_Inbound_Fail()
  {
    _r2s->setInOutInd(RuleConst::FARE_MARKET_INBOUND);

    CategoryRuleInfo cri;
    FareUsage fu;
    fu.inbound() = false;

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, false));
  }

  void testFU_Dir_1_Pass()
  {
    _r2s->setDirectionality(RuleConst::FROM_LOC1_TO_LOC2);

    CategoryRuleInfo cri;
    FareUsage fu;

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, false));
  }

  void testFU_Dir_1_Fail()
  {
    _r2s->setDirectionality(RuleConst::FROM_LOC1_TO_LOC2);

    CategoryRuleInfo cri;
    FareUsage fu;

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, true));
  }

  void testFU_Dir_2_Pass()
  {
    _r2s->setDirectionality(RuleConst::TO_LOC1_FROM_LOC2);

    CategoryRuleInfo cri;
    FareUsage fu;

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, true));
  }

  void testFU_Dir_2_Fail()
  {
    _r2s->setDirectionality(RuleConst::TO_LOC1_FROM_LOC2);

    CategoryRuleInfo cri;
    FareUsage fu;

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, fu, cri, _r2s, false));
  }

  void testFU_Dir_3_Pass()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);

    CategoryRuleInfo cri;
    FareUsage obfu, ibfu;
    obfu.inbound() = false;
    ibfu.inbound() = true;

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, obfu, cri, _r2s, false));
    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ibfu, cri, _r2s, true));
  }

  void testFU_Dir_3_Fail()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);

    CategoryRuleInfo cri;
    FareUsage obfu, ibfu;
    obfu.inbound() = false;
    ibfu.inbound() = true;

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, obfu, cri, _r2s, true));
    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ibfu, cri, _r2s, false));
  }

  void testFU_Dir_4_Pass()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);

    CategoryRuleInfo cri;
    FareUsage obfu, ibfu;
    obfu.inbound() = false;
    ibfu.inbound() = true;

    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, obfu, cri, _r2s, true));
    CPPUNIT_ASSERT_EQUAL(PASS, CategoryRuleItem::isDirectionPass(*_trx, ibfu, cri, _r2s, false));
  }

  void testFU_Dir_4_Fail()
  {
    _r2s->setDirectionality(RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);

    CategoryRuleInfo cri;
    FareUsage obfu, ibfu;
    obfu.inbound() = false;
    ibfu.inbound() = true;

    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, obfu, cri, _r2s, false));
    CPPUNIT_ASSERT_EQUAL(FAIL, CategoryRuleItem::isDirectionPass(*_trx, ibfu, cri, _r2s, true));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CategoryRuleItemTestIsDirectionPass);
}
