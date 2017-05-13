#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp);

class ItinTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinTest);
  CPPUNIT_TEST(testTravelSegHelpers);
  CPPUNIT_TEST(testGetValidatingCarriers);
  CPPUNIT_TEST(testGetValidatingCarriers_WithValidatingCxrGsaData);
  CPPUNIT_TEST(testGetValidatingCarriers_WithMultiSp);
  CPPUNIT_TEST(testIsValidatingCxrGsaDataForMultiSp);
  CPPUNIT_TEST(testHasNeutralValidatingCarrier);
  CPPUNIT_TEST(testGsaSwapMap);
  CPPUNIT_TEST(testGetSwapCarriers);
  CPPUNIT_TEST(testRemoveAlternateValidatingCarriers_NullTest);
  CPPUNIT_TEST(testRemoveAlternateValidatingCarriers_SingleSpSingleCxr);
  CPPUNIT_TEST(testRemoveAlternateValidatingCarriers_SingleSpMultiCxr);
  CPPUNIT_TEST(testRemoveAlternateValidatingCarriers_MultiSpSameCxr);
  CPPUNIT_TEST(testRemoveAlternateValidatingCarriers_MultiSpDiffCxr);
  CPPUNIT_TEST(testRemoveAlternateValidatingCarriers_MultiSpDiffCxrs);
  CPPUNIT_TEST(testGetPricedBrandCombinationIndexes);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testTravelSegHelpers()
  {
    Itin itin;

    AirSeg a, b, c, d, x, y;

    ArunkSeg arunk;

    // We must force either boardMultiCity, offMultiCity, or departureDT
    // to differ , or the call to segmentOrder will assume if these
    // match the Airsegs are identical
    x.boardMultiCity() = "DIFF";

    // we set the order of travel segments: [a,b,c,arunk,d] with
    //'x' and 'y' being non-members

    itin.travelSeg().push_back(&a);
    itin.travelSeg().push_back(&b);
    itin.travelSeg().push_back(&c);
    itin.travelSeg().push_back(&arunk);
    itin.travelSeg().push_back(&d);

    CPPUNIT_ASSERT(itin.isFirstSegment(&a));
    CPPUNIT_ASSERT(itin.isFirstSegment(&b) == false);
    CPPUNIT_ASSERT(itin.isFirstSegment(&x) == false);
    CPPUNIT_ASSERT(itin.isFirstSegment(&y) == false);
    CPPUNIT_ASSERT(itin.isLastSegment(&d));
    CPPUNIT_ASSERT(itin.isLastSegment(&a) == false);
    CPPUNIT_ASSERT(itin.isLastSegment(&x) == false);
    CPPUNIT_ASSERT(itin.isLastSegment(&y) == false);

    CPPUNIT_ASSERT(itin.segmentExists(&a));
    CPPUNIT_ASSERT(itin.segmentExists(&b));
    CPPUNIT_ASSERT(itin.segmentExists(&c));
    CPPUNIT_ASSERT(itin.segmentExists(&arunk));
    CPPUNIT_ASSERT(itin.segmentExists(&d));
    CPPUNIT_ASSERT(itin.segmentExists(&x) == false);
    CPPUNIT_ASSERT(itin.segmentExists(&y) == true); // only because of multicity and departure dt

    CPPUNIT_ASSERT(itin.segmentOrder(&a) == 1);
    CPPUNIT_ASSERT(itin.segmentOrder(&b) == 2);
    CPPUNIT_ASSERT(itin.segmentOrder(&c) == 3);
    CPPUNIT_ASSERT(itin.segmentOrder(&arunk) == 4);
    CPPUNIT_ASSERT(itin.segmentOrder(&d) == 5);
    CPPUNIT_ASSERT(itin.segmentOrder(&x) == -1);
    CPPUNIT_ASSERT(itin.segmentOrder(&y) == 1); // only because of multicity and departure dt

    CPPUNIT_ASSERT(itin.segmentFollows(&a, &b));
    CPPUNIT_ASSERT(itin.segmentFollows(&a, &c) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&a, &a) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&x, &a) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&y, &a) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&a, &arunk) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&b, &c));
    CPPUNIT_ASSERT(itin.segmentFollows(&b, &a) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&b, &b) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&c, &arunk));
    CPPUNIT_ASSERT(itin.segmentFollows(&c, &d) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&arunk, &d));
    CPPUNIT_ASSERT(itin.segmentFollows(&d, &d) == false);
    CPPUNIT_ASSERT(itin.segmentFollows(&d, &arunk) == false);

    CPPUNIT_ASSERT(itin.segmentFollowsAfterArunk(&c, &d));
    CPPUNIT_ASSERT(itin.segmentFollowsAfterArunk(&a, &b) == false);
    CPPUNIT_ASSERT(itin.segmentFollowsAfterArunk(&a, &c) == false);
    CPPUNIT_ASSERT(itin.segmentFollowsAfterArunk(&arunk, &d) == false);
    CPPUNIT_ASSERT(itin.segmentFollowsAfterArunk(&b, &arunk) == false);
  }

  void testGetValidatingCarriers()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);

    Itin itin;
    std::vector<CarrierCode> vcs;
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    v.validatingCarriersData() = vcm;
    itin.validatingCxrGsaData() = &v;

    PricingTrx trx;
    itin.getValidatingCarriers(trx, vcs);
    CPPUNIT_ASSERT(vcs.size() == 1);
    CPPUNIT_ASSERT(vcs[0] == "AA");
  }

  void testGetValidatingCarriers_WithValidatingCxrGsaData()
  {
    fallback::value::fallbackValidatingCxrMultiSp.set(true);
    Itin itin;
    std::vector<CarrierCode> vcs;
    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["BA"] = vcd;
    v.validatingCarriersData() = vcm;
    itin.validatingCxrGsaData() = &v;

    itin.getValidatingCarriers(v, vcs);
    CPPUNIT_ASSERT(vcs.size() == 2);
    CPPUNIT_ASSERT(vcs[0] == "AA");
    CPPUNIT_ASSERT(vcs[1] == "BA");
  }

  void testGetValidatingCarriers_WithMultiSp()
  {
    ValidatingCxrGSAData v1, v2;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    vcm["BA"] = vcd;
    v1.validatingCarriersData() = vcm;

    vcm["UA"] = vcd;
    v2.validatingCarriersData() = vcm;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &v1;
    spGsaDataMap["TCH"] = &v2;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;

    PricingTrx trx;
    std::vector<CarrierCode> vcs;
    itin.getValidatingCarriers(trx, vcs);

    CPPUNIT_ASSERT(vcs.size() == 3);
    CPPUNIT_ASSERT(vcs[0] == "AA");
    CPPUNIT_ASSERT(vcs[1] == "BA");
    CPPUNIT_ASSERT(vcs[2] == "UA");
  }

  void testIsValidatingCxrGsaDataForMultiSp()
  {
    Itin itin;
    CPPUNIT_ASSERT(!itin.isValidatingCxrGsaDataForMultiSp());

    ValidatingCxrGSAData v;
    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap["BSP"] = &v;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
    CPPUNIT_ASSERT(itin.isValidatingCxrGsaDataForMultiSp());
  }

  void testHasNeutralValidatingCarrier()
  {
    SettlementPlanType sp("BSP");
    Itin itin;
    CPPUNIT_ASSERT(!itin.hasNeutralValidatingCarrier(sp));

    ValidatingCxrGSAData v;
    v.isNeutralValCxr()=true;
    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap[sp] = &v;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
    CPPUNIT_ASSERT(itin.hasNeutralValidatingCarrier(sp));
  }

  void testGsaSwapMap()
  {
    SettlementPlanType sp("BSP");
    Itin itin;
    CPPUNIT_ASSERT(itin.gsaSwapMap(sp).size()==0);

    std::set<CarrierCode> gsaSwaps;
    gsaSwaps.insert("NW");

    ValidatingCxrGSAData v;
    v.gsaSwapMap()["DL"]=gsaSwaps;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap[sp] = &v;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
    CPPUNIT_ASSERT(itin.gsaSwapMap(sp).size()==1);
  }

  void testGetSwapCarriers()
  {
    SettlementPlanType sp("BSP");
    Itin itin;
    CPPUNIT_ASSERT(itin.gsaSwapMap(sp).size()==0);

    std::set<CarrierCode> gsaSwaps;
    gsaSwaps.insert("NW");
    gsaSwaps.insert("WN");

    ValidatingCxrGSAData v;
    v.gsaSwapMap()["DL"]=gsaSwaps;

    SpValidatingCxrGSADataMap spGsaDataMap;
    spGsaDataMap[sp] = &v;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataMap;
    CarrierCode cxr = "DL";

    std::set<CarrierCode> result;
    CPPUNIT_ASSERT(itin.getSwapCarriers(cxr, result, sp));
    CPPUNIT_ASSERT(result.size()==2);
    CPPUNIT_ASSERT(result.find("NW") != result.end());
    CPPUNIT_ASSERT(result.find("WN") != result.end());
  }

  size_t getValidatingCarriersDataSize(Itin& itin, const SettlementPlanType& spType)
  {
    auto it = itin.spValidatingCxrGsaDataMap()->begin();
    if (it->second)
      return it->second->validatingCarriersData().size();
    return 0;
  }

  CarrierCode getValidatingCarriers(Itin& itin, const SettlementPlanType& spType)
  {
    auto it = itin.spValidatingCxrGsaDataMap()->cbegin();
    if (it != itin.spValidatingCxrGsaDataMap()->cend() && it->second)
    {
      const ValidatingCxrGSAData& valCxrGsaData = *(it->second);
      auto iv = valCxrGsaData.validatingCarriersData().begin();
      if (iv != valCxrGsaData.validatingCarriersData().end())
        return iv->first;
    }
    return CarrierCode();
  }

  //removeAlternateValidatingCarriers
  void testRemoveAlternateValidatingCarriers_NullTest()
  {
    CarrierCode cxr("AA");

    // Null test
    Itin itin;
    itin.spValidatingCxrGsaDataMap() = nullptr;
    itin.validatingCarrier() = cxr;

    itin.removeAlternateValidatingCarriers();

    SpValidatingCxrGSADataMap spGsaDataM1;
    spGsaDataM1["ARC"] = nullptr;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataM1;
    itin.removeAlternateValidatingCarriers();
  }

  void testRemoveAlternateValidatingCarriers_SingleSpSingleCxr()
  {
    CarrierCode cxr("AA");
    Itin itin;
    SpValidatingCxrGSADataMap spGsaDataM;

    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;
    vcm[cxr] = vcd;

    v.validatingCarriersData() = vcm;
    spGsaDataM["ARC"] = &v;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataM;
    itin.validatingCarrier() = cxr;

    itin.removeAlternateValidatingCarriers();

    //Nothing should be removed
    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 1);
  }

  void testRemoveAlternateValidatingCarriers_SingleSpMultiCxr()
  {
    CarrierCode cxr("AA");
    Itin itin;
    SpValidatingCxrGSADataMap spGsaDataM;

    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;
    vcm["AA"] = vcd;
    vcm["BA"] = vcd;

    v.validatingCarriersData() = vcm;
    spGsaDataM["ARC"] = &v;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataM;
    itin.validatingCarrier() = cxr;

    CPPUNIT_ASSERT(getValidatingCarriersDataSize(itin, "ARC")==2);
    itin.removeAlternateValidatingCarriers(); // "BB" removed
    CPPUNIT_ASSERT(getValidatingCarriersDataSize(itin, "ARC")==1);
  }

  void testRemoveAlternateValidatingCarriers_MultiSpSameCxr()
  {
    CarrierCode cxr("AA");

    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;
    vcm["AA"] = vcd;

    v.validatingCarriersData() = vcm;
    SpValidatingCxrGSADataMap spGsaDataM;
    spGsaDataM["ARC"] = &v;
    spGsaDataM["BSP"] = &v;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataM;
    itin.validatingCarrier() = cxr;

    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 2);
    itin.removeAlternateValidatingCarriers(); // multi VCL
    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 2);
  }

  void testRemoveAlternateValidatingCarriers_MultiSpDiffCxr()
  {
    CarrierCode cxr("AA");

    ValidatingCxrGSAData v1,v2;
    ValidatingCxrDataMap vcm1, vcm2;
    vcx::ValidatingCxrData vcd;
    vcm1["AA"] = vcd;
    vcm2["BA"] = vcd;

    v1.validatingCarriersData() = vcm1;
    v2.validatingCarriersData() = vcm2;

    SpValidatingCxrGSADataMap spGsaDataM;
    spGsaDataM["ARC"] = &v1;
    spGsaDataM["BSP"] = &v2;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataM;
    itin.validatingCarrier() = cxr;

    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 2);
    itin.removeAlternateValidatingCarriers(); // "BSP" should be removed
    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 1);
  }

  void testRemoveAlternateValidatingCarriers_MultiSpDiffCxrs()
  {
    CarrierCode cxr("AA");

    ValidatingCxrGSAData v1,v2;
    ValidatingCxrDataMap vcm1, vcm2;
    vcx::ValidatingCxrData vcd;
    vcm1["AA"] = vcd;
    vcm2["AA"] = vcd;
    vcm2["BA"] = vcd;

    v1.validatingCarriersData() = vcm1;
    v2.validatingCarriersData() = vcm2;

    SpValidatingCxrGSADataMap spGsaDataM;
    spGsaDataM["ARC"] = &v1;
    spGsaDataM["BSP"] = &v2;

    Itin itin;
    itin.spValidatingCxrGsaDataMap() = &spGsaDataM;
    itin.validatingCarrier() = cxr;

    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 2);
    itin.removeAlternateValidatingCarriers(); // "BA" should be removed
    CPPUNIT_ASSERT(itin.spValidatingCxrGsaDataMap()->size() == 2);
    CPPUNIT_ASSERT(getValidatingCarriersDataSize(itin, "BSP")==1);
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), std::string(getValidatingCarriers(itin, "BSP")));
  }

  void testGetPricedBrandCombinationIndexes()
  {
    Itin itin;

    for (uint16_t idx=0; idx < 6; ++idx)
    {
      FarePath* farePath = _memHandle.create<FarePath>();
      farePath->brandIndex() = idx / 2;
      itin.farePath().push_back(farePath);
    }

    auto indexes = itin.getPricedBrandCombinationIndexes();

    CPPUNIT_ASSERT(indexes.size() == 3);
    CPPUNIT_ASSERT(indexes.count(0));
    CPPUNIT_ASSERT(indexes.count(1));
    CPPUNIT_ASSERT(indexes.count(2));
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinTest);

} // tse
