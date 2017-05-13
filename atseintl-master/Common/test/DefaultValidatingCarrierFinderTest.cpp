#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ValidatingCxrGSAData.h"
#include "DBAccess/Loc.h"
#include "DBAccess/CountrySettlementPlanInfo.h"

#include <vector>
#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;
struct DefaultValidatingCarrierFinderTest : public Test
{
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  FarePath* _farePath;
  PaxType* _paxTypeADT;

  const Loc* _locSFO;
  const Loc* _locDFW;
  const Loc* _locNYC;
  const Loc* _locLON;

  DefaultValidatingCarrierFinder* _dvf;

  void SetUp()
  {
    _farePath = 0;
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _itin = _memHandle.create<Itin>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _paxTypeADT = _memHandle.create<PaxType>();
    _paxTypeADT->paxType() = ADULT;

    _locSFO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locNYC = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    _locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
  }

  void TearDown() { _memHandle.clear(); }

  AirSeg* createSegment(const Loc* origin = NULL, const Loc* destination = NULL)
  {
    if (!_farePath)
    {
      _farePath = _memHandle.create<FarePath>();
      _farePath->paxType() = _paxTypeADT;
    }
    if (!_itin)
      _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;

    AirSeg* air = _memHandle.create<AirSeg>();
    air->origin() = origin;
    air->destination() = destination;

    _itin->travelSeg().push_back(air);
    return air;
  }
};

TEST_F(DefaultValidatingCarrierFinderTest, ConstructorTest)
{
  DefaultValidatingCarrierFinder dvf1(*_trx, *_itin);
  ASSERT_TRUE(dvf1.settlementPlan().empty());

  _itin->spValidatingCxrGsaDataMap() = nullptr;
  CountrySettlementPlanInfo cspi;
  cspi.setSettlementPlanTypeCode("ARC");
  cspi.setCountryCode("CA");
  cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi);

  DefaultValidatingCarrierFinder dvf2(*_trx, *_itin);
  EXPECT_EQ("ARC", std::string(dvf2.settlementPlan()));

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["AA"] = v;
  validatingCxrGSAData.isNeutralValCxr() = false;
  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["GEN"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  DefaultValidatingCarrierFinder dvf3(*_trx, *_itin);
  EXPECT_EQ("GEN", std::string(dvf3.settlementPlan()));

  //Adding BSP to list. It should be selected since it is higher
  // in the hierarchy
  spGsaDataMap["BSP"] = &validatingCxrGSAData;
  DefaultValidatingCarrierFinder dvf4(*_trx, *_itin);
  EXPECT_EQ("BSP", std::string(dvf4.settlementPlan()));
}

// Simple case without GSA and NVC
TEST_F(DefaultValidatingCarrierFinderTest, FindDefaultValidatingCxrForSingleCxr)
{
  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("BSP");
  cspi2.setCountryCode("US");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["AA"] = v;

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["BSP"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<TravelSeg*> tSegs;
  AirSeg airSeg1;
  airSeg1.setMarketingCarrierCode("AA");
  tSegs.push_back(&airSeg1);
  _itin->travelSeg() = tSegs;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("AA");

  // Itin with single marketing carrier
  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("AA", defaultValidatingCxr);

  // Itin with multiple marketing carrier
  AirSeg airSeg2;
  airSeg2.setMarketingCarrierCode("UA");
  tSegs.push_back(&airSeg2);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("AA", defaultValidatingCxr);
}

// NVC: Neutral Validating Carrier
TEST_F(DefaultValidatingCarrierFinderTest, FindDefaultValidatingCxrForNVC)
{
  CountrySettlementPlanInfo cspi;
  cspi.setSettlementPlanTypeCode("BSP");
  cspi.setCountryCode("US");
  cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["HO"] = v;
  validatingCxrGSAData.isNeutralValCxr() = true;

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["BSP"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("HO");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);

  // single NVC
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("HO", defaultValidatingCxr);

  validatingCxrGSAData.validatingCarriersData()["YO"] = v;
  validatingCxr.push_back("YO");

  // For multiple NVC, we return no default validating-cxr
  ASSERT_FALSE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
}

// GSA: General Sales Agent
TEST_F(DefaultValidatingCarrierFinderTest, FindDefaultValidatingCxrForGSA)
{
  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("ARC");
  cspi2.setCountryCode("US");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  std::vector<TravelSeg*> tSegs;
  AirSeg airSeg1;
  airSeg1.setMarketingCarrierCode("NW");
  tSegs.push_back(&airSeg1);
  _itin->travelSeg() = tSegs;

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["DL"] = v;
  validatingCxrGSAData.gsaSwapMap()["NW"].insert("DL");

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["ARC"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("DL");

  // Single GSA swap
  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("DL", defaultValidatingCxr);
  EXPECT_EQ("NW", defaultMarketingCxr);

  // Multiple GSA swap
  validatingCxrGSAData.validatingCarriersData()["XL"] = v;
  validatingCxrGSAData.gsaSwapMap()["NW"].insert("XL");
  validatingCxr.push_back("XL");
  ASSERT_FALSE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
}

TEST_F(DefaultValidatingCarrierFinderTest, FindDefaultValidatingCxrForMultiSP)
{
  CountrySettlementPlanInfo cspi1;
  cspi1.setSettlementPlanTypeCode("BSP");
  cspi1.setCountryCode("GB");
  cspi1.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi1);

  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("GEN");
  cspi2.setCountryCode("GB");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData gsaData1;
  gsaData1.validatingCarriersData()["LH"] = v;

  ValidatingCxrGSAData gsaData2;
  gsaData2.validatingCarriersData()["NW"] = v;

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["SAT"] = &gsaData1;
  spGsaDataMap["GEN"] = &gsaData2;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<TravelSeg*> tSegs;

  // Domestic segment
  AirSeg* air1 = createSegment();
  air1->pnrSegment() = 1;
  air1->segmentOrder() = 1;
  air1->origin() = _locDFW;
  air1->origAirport() = _locDFW->loc();
  air1->destination() = _locNYC;
  air1->destAirport() = _locNYC->loc();

  AirSeg* air2 = createSegment();
  air2->pnrSegment() = 2;
  air2->segmentOrder() = 2;
  air2->origin() = _locNYC;
  air2->origAirport() = _locNYC->loc();
  air2->destination() = _locLON;
  air2->destAirport() = _locLON->loc();

  AirSeg* air3 = createSegment();
  air3->pnrSegment() = 3;
  air3->segmentOrder() = 3;
  air3->origin() = _locLON;
  air3->origAirport() = _locLON->loc();
  air3->destination() = _locDFW;
  air3->destAirport() = _locDFW->loc();

  air1->setMarketingCarrierCode("LH");
  air2->setMarketingCarrierCode("NW");
  air3->setMarketingCarrierCode("BA");

  tSegs.push_back(air1);
  tSegs.push_back(air2);
  _itin->travelSeg() = tSegs;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("LH");
  validatingCxr.push_back("NW");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("NW", defaultValidatingCxr);

  // Since BSP is top of hierarchy, we should select BSP's default
  _itin->travelSeg().push_back(air3);
  ValidatingCxrGSAData gsaData3;
  gsaData3.validatingCarriersData()["BA"] = v;
  spGsaDataMap["BSP"] = &gsaData3;
  validatingCxr.push_back("BA");

  DefaultValidatingCarrierFinder dvf2(*_trx, *_itin);
  ASSERT_TRUE(dvf2.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("BA", defaultValidatingCxr);
}

TEST_F(DefaultValidatingCarrierFinderTest, findDefValCxrFromPreferredWithSingleVC)
{
  std::vector<CarrierCode> validatingCxr;
  std::vector<CarrierCode> preferredValidatingCxr;
  CarrierCode defaultValidatingCxr;

  validatingCxr.push_back("LH");
  validatingCxr.push_back("NW");
  validatingCxr.push_back("AA");

  preferredValidatingCxr.push_back("NW");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.findDefValCxrFromPreferred(validatingCxr, preferredValidatingCxr, defaultValidatingCxr));
  EXPECT_EQ("NW", defaultValidatingCxr);

}

TEST_F(DefaultValidatingCarrierFinderTest, findDefValCxrFromPreferredWithMultiVC)
{
  std::vector<CarrierCode> validatingCxr;
  std::vector<CarrierCode> preferredValidatingCxr;
  CarrierCode defaultValidatingCxr;

  validatingCxr.push_back("LH");
  validatingCxr.push_back("NW");
  validatingCxr.push_back("AA");

  preferredValidatingCxr.push_back("BA");
  preferredValidatingCxr.push_back("AA");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.findDefValCxrFromPreferred(validatingCxr, preferredValidatingCxr, defaultValidatingCxr));
  EXPECT_EQ("AA", defaultValidatingCxr);
}

TEST_F(DefaultValidatingCarrierFinderTest, findDefValCxrFromPreferredFail)
{
  std::vector<CarrierCode> validatingCxr;
  std::vector<CarrierCode> preferredValidatingCxr;
  CarrierCode defaultValidatingCxr;

  validatingCxr.push_back("LH");
  validatingCxr.push_back("NW");
  validatingCxr.push_back("AA");

  preferredValidatingCxr.push_back("BA");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_FALSE(dvf.findDefValCxrFromPreferred(validatingCxr, preferredValidatingCxr, defaultValidatingCxr));
  ASSERT_TRUE(defaultValidatingCxr.empty());
}

// Preferred VC - Simple case without GSA and NVC
TEST_F(DefaultValidatingCarrierFinderTest, pvcFindDefaultValidatingCxrForSingleCxr)
{
  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("BSP");
  cspi2.setCountryCode("US");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["AA"] = v;

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["BSP"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<TravelSeg*> tSegs;
  AirSeg airSeg1;
  airSeg1.setMarketingCarrierCode("AA");
  tSegs.push_back(&airSeg1);
  _itin->travelSeg() = tSegs;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("AA");

  _trx->getRequest()->preferredVCs().push_back("AA");

  // Itin with single marketing carrier
  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("AA", defaultValidatingCxr);

}

TEST_F(DefaultValidatingCarrierFinderTest, pvcFindDefaultValidatingCxrForMultiCxr)
{
  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("BSP");
  cspi2.setCountryCode("US");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["AA"] = v;
  validatingCxrGSAData.validatingCarriersData()["YO"] = v;


  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["BSP"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<TravelSeg*> tSegs;
  AirSeg airSeg1;
  airSeg1.setMarketingCarrierCode("AA");
  tSegs.push_back(&airSeg1);

  AirSeg airSeg2;
  airSeg2.setMarketingCarrierCode("YO");
  tSegs.push_back(&airSeg2);

  _itin->travelSeg() = tSegs;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("AA");
  validatingCxr.push_back("YO");;

  _trx->getRequest()->preferredVCs().push_back("YO");

  // Itin with single marketing carrier
  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("YO", defaultValidatingCxr);

}

// NVC: Neutral Validating Carrier
TEST_F(DefaultValidatingCarrierFinderTest, pvcFindDefaultValidatingCxrForNVC)
{
  CountrySettlementPlanInfo cspi;
  cspi.setSettlementPlanTypeCode("BSP");
  cspi.setCountryCode("US");
  cspi.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["HO"] = v;
  validatingCxrGSAData.isNeutralValCxr() = true;

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["BSP"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("HO");

  _trx->getRequest()->preferredVCs().push_back("YO");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);

  // single NVC
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("HO", defaultValidatingCxr);

  validatingCxrGSAData.validatingCarriersData()["YO"] = v;
  validatingCxr.push_back("YO");
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("YO", defaultValidatingCxr);
}

// GSA: General Sales Agent
TEST_F(DefaultValidatingCarrierFinderTest, pvcFindDefaultValidatingCxrForGSA)
{
  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("ARC");
  cspi2.setCountryCode("US");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  std::vector<TravelSeg*> tSegs;
  AirSeg airSeg1;
  airSeg1.setMarketingCarrierCode("NW");
  tSegs.push_back(&airSeg1);
  _itin->travelSeg() = tSegs;

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData validatingCxrGSAData;
  validatingCxrGSAData.validatingCarriersData()["DL"] = v;
  validatingCxrGSAData.gsaSwapMap()["NW"].insert("DL");

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["ARC"] = &validatingCxrGSAData;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("DL");

  _trx->getRequest()->preferredVCs().push_back("DL");

  // Single GSA swap
  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("DL", defaultValidatingCxr);
  EXPECT_EQ("NW", defaultMarketingCxr);

  // Multiple GSA swap
  validatingCxrGSAData.validatingCarriersData()["XL"] = v;
  validatingCxrGSAData.gsaSwapMap()["NW"].insert("XL");
  validatingCxr.push_back("XL");
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("DL", defaultValidatingCxr);
  EXPECT_EQ("NW", defaultMarketingCxr);
}

TEST_F(DefaultValidatingCarrierFinderTest, pvcFindDefaultValidatingCxrForMultiSP)
{
  CountrySettlementPlanInfo cspi1;
  cspi1.setSettlementPlanTypeCode("BSP");
  cspi1.setCountryCode("GB");
  cspi1.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi1);

  CountrySettlementPlanInfo cspi2;
  cspi2.setSettlementPlanTypeCode("GEN");
  cspi2.setCountryCode("GB");
  cspi2.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  _trx->addCountrySettlementPlanInfo(&cspi2);

  vcx::ValidatingCxrData v;
  ValidatingCxrGSAData gsaData1;
  gsaData1.validatingCarriersData()["LH"] = v;

  ValidatingCxrGSAData gsaData2;
  gsaData2.validatingCarriersData()["NW"] = v;

  SpValidatingCxrGSADataMap spGsaDataMap;
  spGsaDataMap["SAT"] = &gsaData1;
  spGsaDataMap["GEN"] = &gsaData2;
  _itin->spValidatingCxrGsaDataMap() = &spGsaDataMap;

  std::vector<TravelSeg*> tSegs;

  // Domestic segment
  AirSeg* air1 = createSegment();
  air1->pnrSegment() = 1;
  air1->segmentOrder() = 1;
  air1->origin() = _locDFW;
  air1->origAirport() = _locDFW->loc();
  air1->destination() = _locNYC;
  air1->destAirport() = _locNYC->loc();

  AirSeg* air2 = createSegment();
  air2->pnrSegment() = 2;
  air2->segmentOrder() = 2;
  air2->origin() = _locNYC;
  air2->origAirport() = _locNYC->loc();
  air2->destination() = _locLON;
  air2->destAirport() = _locLON->loc();

  AirSeg* air3 = createSegment();
  air3->pnrSegment() = 3;
  air3->segmentOrder() = 3;
  air3->origin() = _locLON;
  air3->origAirport() = _locLON->loc();
  air3->destination() = _locDFW;
  air3->destAirport() = _locDFW->loc();

  air1->setMarketingCarrierCode("LH");
  air2->setMarketingCarrierCode("NW");
  air3->setMarketingCarrierCode("BA");

  tSegs.push_back(air1);
  tSegs.push_back(air2);
  _itin->travelSeg() = tSegs;

  std::vector<CarrierCode> validatingCxr;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  validatingCxr.push_back("LH");
  validatingCxr.push_back("NW");

  _trx->getRequest()->preferredVCs().push_back("LH");

  DefaultValidatingCarrierFinder dvf(*_trx, *_itin);
  ASSERT_TRUE(dvf.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("LH", defaultValidatingCxr);

  // Since BSP is top of hierarchy, we should select BSP's default
  _itin->travelSeg().push_back(air3);
  ValidatingCxrGSAData gsaData3;
  gsaData3.validatingCarriersData()["BA"] = v;
  spGsaDataMap["BSP"] = &gsaData3;
  validatingCxr.push_back("BA");

  DefaultValidatingCarrierFinder dvf2(*_trx, *_itin);
  ASSERT_TRUE(dvf2.determineDefaultValidatingCarrier(validatingCxr,defaultValidatingCxr,defaultMarketingCxr));
  EXPECT_EQ("LH", defaultValidatingCxr);
}

}
