#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Common/ErrorResponseException.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <string>

using namespace std;

namespace tse
{
class TaxItineraryTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxItineraryTest);

  CPPUNIT_TEST(testRemoveCurrentTaxResponse);
  CPPUNIT_TEST(testSetValCxrTaxResponse_OneTaxResponse_Cxr_AA);
  CPPUNIT_TEST(testSetValCxrTaxResponse_TwoTaxResponse_Cxr_AA_And_AB);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForValidatingCxr_EmptyContainer);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForValidatingCxr_MixedMultipleSPWithTCH);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForValidatingCxr_MultipleSPWithTCH);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForValidatingCxr_MultipleSP);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForValidatingCxr_SingleSPWithTCH);
  CPPUNIT_TEST(testGetCountrySettlementPlanInfoForValidatingCxr_SingleSP);
  CPPUNIT_TEST(testGetNonTCHCountrySettlementPlanInfo_EmptyColTest);
  CPPUNIT_TEST(testGetNonTCHCountrySettlementPlanInfo_TCH_Exists);
  CPPUNIT_TEST(testGetNonTCHCountrySettlementPlanInfo_TCH_DoesNotExists);
  CPPUNIT_TEST(testRemoveTaxResponseFromItin_NullTest);
  CPPUNIT_TEST(testRemoveTaxResponseFromItin_TaxResponseExists);
  CPPUNIT_TEST(testRemoveTaxResponseFromItin_TaxResponseDoesNotExists);

  CPPUNIT_TEST(testStoreTaxResponse_NoAlternates_NoTCH);
  CPPUNIT_TEST(testStoreTaxResponse_NoAlternates);
  CPPUNIT_TEST(testStoreTaxResponse_HasTCHFormultipleSP);
  CPPUNIT_TEST(testStoreTaxResponse_TestWithDiagOn);

  CPPUNIT_TEST_SUITE_END();

public:
  PricingTrx  _trx;
  PricingRequest  _pricingRequest;
  FarePath _fp;
  FareMarket::RetrievalInfo _info1;
  FareMarket::RetrievalInfo _info2;
  TestMemHandle _memHandle;
  TaxItinerary* _taxItin;
  TaxResponse*  _taxRsp1;
  TaxResponse*  _taxRsp2;
  TaxResponse*  _taxRsp3;
  TaxRecord* _taxRec1;
  TaxRecord* _taxRec2;
  TaxRecord* _taxRec3;
  Itin*      _itin;
  TaxMap::TaxFactoryMap* _taxFactoryMap;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx.setRequest(&_pricingRequest);

    _taxItin = _memHandle.create<TaxItinerary>();
    _taxRsp1 = _memHandle.create<TaxResponse>();
    _taxRsp2 = _memHandle.create<TaxResponse>();
    _taxRsp3 = _memHandle.create<TaxResponse>();
    _taxRec1 = _memHandle.create<TaxRecord>();
    _taxRec1->setTaxCode("LL");
    _taxRec2 = _memHandle.create<TaxRecord>();
    _taxRec2->setTaxCode("MM");
    _taxRec3 = _memHandle.create<TaxRecord>();
    _taxRec3->setTaxCode("YY");

    _itin = _memHandle.create<Itin>();
    _fp.itin() = _itin;
    _fp.paxType() = _memHandle.create<PaxType>();
    _fp.paxType()->paxType()=ADULT;
    _taxFactoryMap = _memHandle.create<TaxMap::TaxFactoryMap>();
    TaxMap::buildTaxFactoryMap(_trx.dataHandle(), *_taxFactoryMap);

    _trx.setValidatingCxrGsaApplicable(true);
    _taxItin->initialize(_trx, *_itin, *_taxFactoryMap);

    _taxRsp1->farePath() = &_fp;
    _taxRsp2->farePath() = &_fp;
    _taxRsp3->farePath() = &_fp;
  }

  void tearDown() { _memHandle.clear(); }

  void testRemoveCurrentTaxResponse()
  {
    MoneyAmount taxAmt1 = 10.00;
    _taxRec1->setTaxAmount(taxAmt1);
    _taxRec2->setTaxCode("LL");
    MoneyAmount taxAmt2 = 11.00;
    _taxRec2->setTaxAmount(taxAmt2);

    _taxRsp1->taxRecordVector().push_back(_taxRec1);
    _taxRsp2->taxRecordVector().push_back(_taxRec2);

    CarrierCode cxr = "AA";
    _itin->validatingCarrier() = "AB";
    _fp.validatingCarriers().push_back(cxr);
    _fp.validatingCarriers().push_back(_itin->validatingCarrier());

    _itin->mutableTaxResponses().push_back(_taxRsp1);
    _itin->mutableTaxResponses().push_back(_taxRsp2);

    _taxItin->removeCurrentTaxResponse(_fp);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _itin->mutableTaxResponses().size());
  }

  void testSetValCxrTaxResponse_OneTaxResponse_Cxr_AA()
  {
    CarrierCode cxr = "AA";
    _taxRsp1->taxRecordVector().push_back(_taxRec1);
    _itin->mutableTaxResponses().push_back(_taxRsp1);
    _fp.setValCxrTaxResponse(cxr, _taxRsp1);

    CPPUNIT_ASSERT_EQUAL(size_t(1), _fp.valCxrTaxResponseMap().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _itin->mutableTaxResponses().size());
    CPPUNIT_ASSERT_EQUAL(_taxRsp1, _itin->mutableTaxResponses().front());
  }

  void testSetValCxrTaxResponse_TwoTaxResponse_Cxr_AA_And_AB()
  {
    _taxRsp1->taxRecordVector().push_back(_taxRec1);
    _taxRsp2->taxRecordVector().push_back(_taxRec2);

    CarrierCode cxr = "AA";
    CarrierCode cxr1 = "AB";

    _itin->mutableTaxResponses().push_back(_taxRsp1);
    _itin->mutableTaxResponses().push_back(_taxRsp2);
    _fp.setValCxrTaxResponse(cxr, _taxRsp1);
    _fp.setValCxrTaxResponse(cxr1, _taxRsp2);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _fp.valCxrTaxResponseMap().size());
    CPPUNIT_ASSERT_EQUAL(_taxRsp1, _itin->mutableTaxResponses().front());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->mutableTaxResponses().size());
  }

  CountrySettlementPlanInfo* getCountrySettlementPlanInfo(const SettlementPlanType& sp)
  {
    CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
    if (!cspi)
      return nullptr;

    cspi->setSettlementPlanTypeCode(sp);
    cspi->setCountryCode("US");
    cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    return cspi;
  }

  void setCountrySettlementPlanInfo(std::initializer_list<std::string> list)
  {
    for (auto sp : list)
    {
      CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
      if (cspi)
      {
        cspi->setSettlementPlanTypeCode(sp);
        cspi->setCountryCode("US");
        cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        _trx.addCountrySettlementPlanInfo(cspi);
      }
    }
  }

  void setCountrySettlementPlanInfo(std::vector<CountrySettlementPlanInfo*>& cspiCol,
      std::initializer_list<std::string> list)
  {
    for (auto sp : list)
    {
      CountrySettlementPlanInfo* cspi = _memHandle.create<CountrySettlementPlanInfo>();
      if (cspi)
      {
        cspi->setSettlementPlanTypeCode(sp);
        cspi->setCountryCode("US");
        cspi->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
        cspiCol.push_back(cspi);
      }
    }
  }

  void testGetCountrySettlementPlanInfoForValidatingCxr_EmptyContainer()
  {
    // itin's spValidatingCxrGsaDataMap is null
    std::vector<CountrySettlementPlanInfo*> cxrSps;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);
    CPPUNIT_ASSERT(cxrSps.empty());

    // itin's spValidatingCxrGsaDataMap is not null
    SpValidatingCxrGSADataMap spGsaDataMap;
    _fp.itin()->spValidatingCxrGsaDataMap() = &spGsaDataMap;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);
    CPPUNIT_ASSERT(cxrSps.empty());
  }

  void testGetCountrySettlementPlanInfoForValidatingCxr_SingleSP()
  {
    setCountrySettlementPlanInfo({"BSP"});

    vcx::ValidatingCxrData valCxrData;
    ValidatingCxrGSAData valCxrGsaData;
    SpValidatingCxrGSADataMap spGsaDataMap;

    // BSP => AA, BA
    valCxrGsaData.validatingCarriersData()["AA"]=valCxrData;
    valCxrGsaData.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["BSP"] = &valCxrGsaData;

    _fp.itin()->spValidatingCxrGsaDataMap() = &spGsaDataMap;
    _fp.itin()->validatingCarrier()="AA";

    std::vector<CountrySettlementPlanInfo*> cxrSps;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);

    CPPUNIT_ASSERT(cxrSps.size() == 1);
    CPPUNIT_ASSERT(cxrSps[0]->getSettlementPlanTypeCode()=="BSP");
  }

  void testGetCountrySettlementPlanInfoForValidatingCxr_SingleSPWithTCH()
  {
    setCountrySettlementPlanInfo({"TCH"});

    vcx::ValidatingCxrData valCxrData;
    ValidatingCxrGSAData valCxrGsaData;
    SpValidatingCxrGSADataMap spGsaDataMap;

    // TCH => UA
    valCxrGsaData.validatingCarriersData()["UA"]=valCxrData;
    spGsaDataMap["TCH"] = &valCxrGsaData;

    _fp.itin()->spValidatingCxrGsaDataMap() = &spGsaDataMap;
    _fp.itin()->validatingCarrier()="UA";

    std::vector<CountrySettlementPlanInfo*> cxrSps;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);

    CPPUNIT_ASSERT(cxrSps.size() == 1);
    CPPUNIT_ASSERT(cxrSps.front()->getSettlementPlanTypeCode()=="TCH");
  }

  void testGetCountrySettlementPlanInfoForValidatingCxr_MultipleSP()
  {
    setCountrySettlementPlanInfo({"BSP", "GEN"});

    vcx::ValidatingCxrData valCxrData;
    ValidatingCxrGSAData valCxrGsaData1, valCxrGsaData2, valCxrGsaData3;
    SpValidatingCxrGSADataMap spGsaDataMap;

    // BSP => AA, BA
    valCxrGsaData1.validatingCarriersData()["AA"]=valCxrData;
    valCxrGsaData1.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["BSP"] = &valCxrGsaData1;

    // GEN => BA
    valCxrGsaData3.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["GEN"] = &valCxrGsaData3;

    _fp.itin()->spValidatingCxrGsaDataMap() = &spGsaDataMap;
    _fp.itin()->validatingCarrier()="BA";

    std::vector<CountrySettlementPlanInfo*> cxrSps;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);

    CPPUNIT_ASSERT(cxrSps.size() == 2);
    CPPUNIT_ASSERT(cxrSps[0]->getSettlementPlanTypeCode()=="BSP");
    CPPUNIT_ASSERT(cxrSps[1]->getSettlementPlanTypeCode()=="GEN");
  }

  void testGetCountrySettlementPlanInfoForValidatingCxr_MultipleSPWithTCH()
  {
    setCountrySettlementPlanInfo({"BSP", "GEN", "TCH"});

    vcx::ValidatingCxrData valCxrData;
    ValidatingCxrGSAData valCxrGsaData1, valCxrGsaData2, valCxrGsaData3;
    SpValidatingCxrGSADataMap spGsaDataMap;

    // BSP => AA, BA
    valCxrGsaData1.validatingCarriersData()["AA"]=valCxrData;
    valCxrGsaData1.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["BSP"] = &valCxrGsaData1;

    // GEN => BA
    valCxrGsaData3.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["GEN"] = &valCxrGsaData3;

    // TCH => UA
    valCxrGsaData2.validatingCarriersData()["UA"]=valCxrData;
    spGsaDataMap["TCH"] = &valCxrGsaData2;

    _fp.itin()->spValidatingCxrGsaDataMap() = &spGsaDataMap;
    _fp.itin()->validatingCarrier()="UA";

    std::vector<CountrySettlementPlanInfo*> cxrSps;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);

    CPPUNIT_ASSERT(cxrSps.size() == 1);
    CPPUNIT_ASSERT(cxrSps[0]->getSettlementPlanTypeCode()=="TCH");
  }

  void testGetCountrySettlementPlanInfoForValidatingCxr_MixedMultipleSPWithTCH()
  {
    setCountrySettlementPlanInfo({"BSP", "GEN", "TCH"});

    vcx::ValidatingCxrData valCxrData;
    ValidatingCxrGSAData valCxrGsaData1, valCxrGsaData2, valCxrGsaData3;
    SpValidatingCxrGSADataMap spGsaDataMap;

    // BSP => AA, BA
    valCxrGsaData1.validatingCarriersData()["AA"]=valCxrData;
    valCxrGsaData1.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["BSP"] = &valCxrGsaData1;

    // GEN => BA
    valCxrGsaData3.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["GEN"] = &valCxrGsaData3;

    // TCH => UA
    valCxrGsaData2.validatingCarriersData()["BA"]=valCxrData;
    spGsaDataMap["TCH"] = &valCxrGsaData2;

    _fp.itin()->spValidatingCxrGsaDataMap() = &spGsaDataMap;
    _fp.itin()->validatingCarrier()="BA";

    std::vector<CountrySettlementPlanInfo*> cxrSps;
    _taxItin->getCountrySettlementPlanInfoForValidatingCxr(_fp, cxrSps);

    CPPUNIT_ASSERT(cxrSps.size() == 3);
    CPPUNIT_ASSERT(cxrSps[0]->getSettlementPlanTypeCode()=="BSP");
    CPPUNIT_ASSERT(cxrSps[1]->getSettlementPlanTypeCode()=="GEN");
    CPPUNIT_ASSERT(cxrSps[2]->getSettlementPlanTypeCode()=="TCH");
  }

  void testGetNonTCHCountrySettlementPlanInfo_EmptyColTest()
  {
    std::vector<CountrySettlementPlanInfo*> cspiCol;
    CountrySettlementPlanInfo* cspi = _taxItin->getNonTCHCountrySettlementPlanInfo(cspiCol);
    CPPUNIT_ASSERT(cspi == nullptr);
  }

  void testGetNonTCHCountrySettlementPlanInfo_TCH_Exists()
  {
    std::vector<CountrySettlementPlanInfo*> cspiCol;
    setCountrySettlementPlanInfo(cspiCol, {"BSP", "TCH", "GEN"});
    CountrySettlementPlanInfo* cspi = _taxItin->getNonTCHCountrySettlementPlanInfo(cspiCol);
    CPPUNIT_ASSERT(cspi != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP"), std::string(cspi->getSettlementPlanTypeCode()));

    cspiCol.clear();
    setCountrySettlementPlanInfo(cspiCol, {"TCH", "GEN", "BSP"});
    cspi = _taxItin->getNonTCHCountrySettlementPlanInfo(cspiCol);
    CPPUNIT_ASSERT(cspi != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("GEN"), std::string(cspi->getSettlementPlanTypeCode()));
  }

  void testGetNonTCHCountrySettlementPlanInfo_TCH_DoesNotExists()
  {
    std::vector<CountrySettlementPlanInfo*> cspiCol;
    setCountrySettlementPlanInfo(cspiCol, {"BSP", "GEN"});
    CountrySettlementPlanInfo* cspi = _taxItin->getNonTCHCountrySettlementPlanInfo(cspiCol);
    CPPUNIT_ASSERT(cspi != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("BSP"), std::string(cspi->getSettlementPlanTypeCode()));

    cspiCol.clear();
    setCountrySettlementPlanInfo(cspiCol, {"GEN", "BSP"});
    cspi = _taxItin->getNonTCHCountrySettlementPlanInfo(cspiCol);
    CPPUNIT_ASSERT(cspi != nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("GEN"), std::string(cspi->getSettlementPlanTypeCode()));
  }

  void setTaxAmount(TaxRecord& taxRec, MoneyAmount taxAmt)
  {
    taxRec.setTaxAmount(taxAmt);
  }

  void testRemoveTaxResponseFromItin_NullTest()
  {
    TaxResponse* taxResp = nullptr;
    setTaxAmount(*_taxRec1, 10.00);
    setTaxAmount(*_taxRec2, 12.00);

    _itin->mutableTaxResponses().push_back(_taxRsp1);
    _itin->mutableTaxResponses().push_back(_taxRsp2);

    _taxItin->removeTaxResponseFromItin(_fp, taxResp);
    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->mutableTaxResponses().size());
  }

  void testRemoveTaxResponseFromItin_TaxResponseExists()
  {
    setTaxAmount(*_taxRec1, 10.00);
    setTaxAmount(*_taxRec2, 12.00);

    _taxRsp1->taxRecordVector().push_back(_taxRec1);
    _taxRsp2->taxRecordVector().push_back(_taxRec2);

    CarrierCode cxr = "AA";
    _itin->validatingCarrier() = "BA"; //default val-cxr
    _fp.validatingCarriers().push_back(cxr);
    _fp.validatingCarriers().push_back(_itin->validatingCarrier());

    _itin->mutableTaxResponses().push_back(_taxRsp1);
    _itin->mutableTaxResponses().push_back(_taxRsp2);
    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->mutableTaxResponses().size());

    _taxItin->removeTaxResponseFromItin(_fp, _taxRsp1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _itin->mutableTaxResponses().size());

    _taxItin->removeTaxResponseFromItin(_fp, _taxRsp2);
    CPPUNIT_ASSERT_EQUAL(size_t(0), _itin->mutableTaxResponses().size());
  }

  void testRemoveTaxResponseFromItin_TaxResponseDoesNotExists()
  {
    setTaxAmount(*_taxRec1, 10.00);
    setTaxAmount(*_taxRec2, 12.00);
    setTaxAmount(*_taxRec3, 13.00);

    _taxRsp1->taxRecordVector().push_back(_taxRec1);
    _taxRsp2->taxRecordVector().push_back(_taxRec2);
    _taxRsp3->taxRecordVector().push_back(_taxRec3);

    CarrierCode cxr = "AA";
    _itin->validatingCarrier() = "BA"; //default val-cxr
    _fp.validatingCarriers().push_back(cxr);
    _fp.validatingCarriers().push_back(_itin->validatingCarrier());

    _fp.itin()->mutableTaxResponses().push_back(_taxRsp1);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp2);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp3);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _itin->mutableTaxResponses().size());
    _taxItin->removeTaxResponseFromItin(_fp, _taxRsp3);
    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->mutableTaxResponses().size());
  }

  void setTaxResponse(
      TaxResponse& taxResp,
      TaxRecord& taxRec,
      const CarrierCode& cxr,
      std::initializer_list<std::string> list)
  {
    taxResp.taxRecordVector().push_back(&taxRec);
    taxResp.validatingCarrier() = "AA";
    for (const SettlementPlanType& sp : list)
      taxResp.settlementPlans().push_back(sp);
  }

  void testStoreTaxResponse_NoAlternates_NoTCH()
  {
    setTaxAmount(*_taxRec1, 10.00);
    setTaxResponse(*_taxRsp1, *_taxRec1, "AA", {"BSP", "GEN", "TCH"});

    _itin->validatingCarrier() = "BA"; //default val-cxr

    bool hasTCHForMultipleSp = false;
    std::vector<TaxResponse*> taxResponses;
    taxResponses.push_back(_taxRsp1);
    taxResponses.push_back(_taxRsp2);
    taxResponses.push_back(_taxRsp3);

    _fp.itin()->mutableTaxResponses().push_back(_taxRsp1);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp2);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp3);

    _taxItin->storeTaxResponses(_fp, taxResponses, hasTCHForMultipleSp);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _fp.valCxrTaxResponses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _fp.itin()->mutableTaxResponses().size());
  }

  void testStoreTaxResponse_NoAlternates()
  {
    setTaxAmount(*_taxRec1, 10.00);
    setTaxAmount(*_taxRec2, 12.00);
    setTaxAmount(*_taxRec3, 13.00);

    setTaxResponse(*_taxRsp1, *_taxRec1, "AA", {"BSP", "GEN", "TCH"});
    setTaxResponse(*_taxRsp2, *_taxRec2, "BA", {"BSP", "GEN", "TCH"});
    setTaxResponse(*_taxRsp3, *_taxRec3, "CA", {"BSP", "GEN", "TCH"});


    _itin->validatingCarrier() = "BA"; //default val-cxr

    _fp.validatingCarriers().push_back("AA");
    _fp.validatingCarriers().push_back("BA");
    _fp.validatingCarriers().push_back("CA");

    bool hasTCHForMultipleSp = false;
    std::vector<TaxResponse*> taxResponses;
    taxResponses.push_back(_taxRsp1);
    taxResponses.push_back(_taxRsp2);
    taxResponses.push_back(_taxRsp3);

    _fp.itin()->mutableTaxResponses().push_back(_taxRsp1);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp2);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp3);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _fp.itin()->mutableTaxResponses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _fp.itin()->valCxrTaxResponses().size());

    _taxItin->storeTaxResponses(_fp, taxResponses, hasTCHForMultipleSp);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _fp.valCxrTaxResponses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _fp.valCxrTaxResponses().begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _fp.itin()->mutableTaxResponses().size());
  }

  void testStoreTaxResponse_HasTCHFormultipleSP()
  {
    setTaxAmount(*_taxRec1, 10.00);
    setTaxAmount(*_taxRec2, 12.00);
    setTaxAmount(*_taxRec3, 13.00);

    setTaxResponse(*_taxRsp1, *_taxRec1, "AA", {"BSP", "GEN", "TCH"});
    setTaxResponse(*_taxRsp2, *_taxRec2, "BA", {"BSP", "GEN", "TCH"});
    setTaxResponse(*_taxRsp3, *_taxRec3, "CA", {"BSP", "GEN", "TCH"});


    _itin->validatingCarrier() = "BA"; //default val-cxr

    _fp.validatingCarriers().push_back("AA");
    _fp.validatingCarriers().push_back("BA");
    _fp.validatingCarriers().push_back("CA");

    bool hasTCHForMultipleSp = true;
    std::vector<TaxResponse*> taxResponses;
    taxResponses.push_back(_taxRsp1);
    taxResponses.push_back(_taxRsp2);
    taxResponses.push_back(_taxRsp3);

    _fp.itin()->mutableTaxResponses().push_back(_taxRsp1);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp2);
    _fp.itin()->mutableTaxResponses().push_back(_taxRsp3);

    _taxItin->storeTaxResponses(_fp, taxResponses, hasTCHForMultipleSp);
    CPPUNIT_ASSERT_EQUAL(size_t(1), _fp.valCxrTaxResponses().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _fp.itin()->mutableTaxResponses().size());
  }

  void testStoreTaxResponse_TestWithDiagOn()
  {
    setTaxAmount(*_taxRec1, 10.00);
    setTaxAmount(*_taxRec2, 12.00);
    setTaxAmount(*_taxRec3, 13.00);

    setTaxResponse(*_taxRsp1, *_taxRec1, "AA", {"BSP", "GEN", "TCH"});
    setTaxResponse(*_taxRsp2, *_taxRec2, "BA", {"BSP", "GEN", "TCH"});
    setTaxResponse(*_taxRsp3, *_taxRec3, "CA", {"BSP", "GEN", "TCH"});


    _itin->validatingCarrier() = "BA"; //default val-cxr

    _fp.validatingCarriers().push_back("AA");
    _fp.validatingCarriers().push_back("BA");
    _fp.validatingCarriers().push_back("CA");

    std::vector<TaxResponse*> taxResponses;
    taxResponses.push_back(_taxRsp1);
    taxResponses.push_back(_taxRsp2);
    taxResponses.push_back(_taxRsp3);

    _taxItin->_trx->diagnostic().diagnosticType() = AllPassTaxDiagnostic281;
    _taxItin->storeTaxResponses(_fp, taxResponses, false);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _fp.itin()->valCxrTaxResponses().size());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxItineraryTest);
}
