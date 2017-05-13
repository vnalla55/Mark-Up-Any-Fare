//
// Copyright Sabre 2012-05-24
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "Xform/XMLShoppingResponse.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TicketingFeesInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/PaxType.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TseEnums.h"
#include "Xform/XMLWriter.h"
#include "Common/ShoppingUtil.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/NegFareRest.h"
#include "DataModel/AdjustedSellingCalcData.h"

#include <iostream>
namespace tse
{

namespace
{
class OCFeesBuilder
{
  TestMemHandle* _memHandle;
  OCFees* _ocFees;
  OCFees::OCFeesSeg* _ocFeesSeg;

public:
  OCFeesBuilder(TestMemHandle& memHandle)
  {
    _memHandle = &memHandle;
    _ocFees = _memHandle->create<OCFees>();
    _ocFees->setSeg(0);
  }

  OCFees* build() { return _ocFees; }

  OCFeesBuilder& addOcFeesUsage(const std::string& commercialName)
  {
    OCFeesUsage* ocFeesUsage = _memHandle->create<OCFeesUsage>();
    ocFeesUsage->oCFees() = _ocFees;
    ocFeesUsage->upgradeT198CommercialName() = commercialName;
    _ocFees->ocfeeUsage().push_back(ocFeesUsage);
    return *this;
  }

  OCFeesBuilder& setFarePath(FarePath* farePath)
  {
    _ocFees->farePath() = farePath;
    return *this;
  }

  OCFeesBuilder& setTravelStart(TravelSeg* travelStart)
  {
    _ocFees->travelStart() = travelStart;
    return *this;
  }

  OCFeesBuilder& setTravelEnd(TravelSeg* travelEnd)
  {
    _ocFees->travelEnd() = travelEnd;
    return *this;
  }

  OCFeesBuilder&
  setSubCodeInfo(ServiceSubTypeCode serviceSubTypeCode, const std::string& commercialName = "")
  {
    SubCodeInfo* subCodeInfo = _memHandle->create<SubCodeInfo>();
    subCodeInfo->serviceSubTypeCode() = serviceSubTypeCode;
    subCodeInfo->commercialName() = commercialName;

    _ocFees->subCodeInfo() = subCodeInfo;
    return *this;
  }
};

class TravelSegBuilder
{
  TestMemHandle* _memHandle;
  TravelSeg* _travelSeg;

public:
  TravelSegBuilder(TestMemHandle& memHandle)
  {
    _memHandle = &memHandle;
    _travelSeg = _memHandle->create<AirSeg>();
  }

  TravelSeg* build() { return _travelSeg; }

  TravelSegBuilder& setTravelStart(const LocCode& originLocCode)
  {
    Loc* origin = _memHandle->create<Loc>();
    origin->loc() = originLocCode;
    _travelSeg->origin() = origin;
    return *this;
  }

  TravelSegBuilder& setTravelEnd(const LocCode& destinationLocCode)
  {
    Loc* destination = _memHandle->create<Loc>();
    destination->loc() = destinationLocCode;
    _travelSeg->destination() = destination;
    return *this;
  }
};

class ItinBuilder
{
  TestMemHandle* _memHandle;
  Itin* _itin;

public:
  ItinBuilder(TestMemHandle& memHandle)
  {
    _memHandle = &memHandle;
    _itin = _memHandle->create<Itin>();
  }

  Itin* build() { return _itin; }

  ItinBuilder& addTravelSeg(TravelSeg* travelSeg)
  {
    _itin->travelSeg().push_back(travelSeg);
    return *this;
  }
};

class ServiceFeesGroupBuilder
{
  TestMemHandle* _memHandle;
  ServiceFeesGroup* _sfg;
  FarePath* _farePath;

public:
  ServiceFeesGroupBuilder(TestMemHandle& memHandle)
  {
    _memHandle = &memHandle;
    _sfg = _memHandle->create<ServiceFeesGroup>();
  }

  ServiceFeesGroup* build() { return _sfg; }

  ServiceFeesGroupBuilder& setFarePath(FarePath* farePath)
  {
    _farePath = farePath;
    return *this;
  }

  ServiceFeesGroupBuilder& addOCFees(OCFees* ocFees)
  {
    _sfg->ocFeesMap()[_farePath].push_back(ocFees);
    return *this;
  }

  ServiceFeesGroupBuilder& setItin(Itin* itin)
  {
    _farePath->itin() = itin;
    return *this;
  }

  ServiceFeesGroupBuilder& setPaxType(PaxTypeCode paxTypeCode)
  {
    PaxType* paxType = _memHandle->create<PaxType>();
    paxType->paxType() = paxTypeCode;
    _farePath->paxType() = paxType;
    return *this;
  }
};
}

class XMLShoppingResponseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLShoppingResponseTest);
  CPPUNIT_TEST(testGetFareUsageId);
  CPPUNIT_TEST(test_getCommercialName_OCFeesUsage);
  CPPUNIT_TEST(test_getCommercialName_OCFeesUsage_Empty);
  CPPUNIT_TEST(testGenerateOCF);
  CPPUNIT_TEST(testGenerateOBG);
  CPPUNIT_TEST(testGenerateCPL);

  CPPUNIT_TEST(testCalculateObFeeAmountFromAmountSameCurrency);
  CPPUNIT_TEST(testAddFareBrandDetails);
  CPPUNIT_TEST(testAddFareBrandDetailsOverride);

  CPPUNIT_TEST(testPrintAdjMsg);
  CPPUNIT_TEST(testGenerateHPSAdjustedSellingData);
  CPPUNIT_TEST(testGenerateErrorDiagnostic);
  CPPUNIT_TEST(testVendorCodeShouldBeAddedToFdcTag);
  CPPUNIT_TEST(testGenerateErrorDiagnosticNone);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  PricingOptions _options;
  PricingRequest _request;
  XMLShoppingResponse* _shoppingResponse;
  TicketingFeesInfo* _feeInfo;
  PricingTrx* _pTrx;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _request.ticketingAgent() = _memHandle.create<Agent>();
    _trx = _memHandle.create<ShoppingTrx>();
    _trx->setRequest(&_request);
    _trx->setOptions(&_options);

    _shoppingResponse = _memHandle.create<XMLShoppingResponse>(*_trx);

    _feeInfo = _memHandle.create<TicketingFeesInfo>();

    _pTrx = _memHandle.insert(new PricingTrx);
  }

  void tearDown() { _memHandle.clear(); }

  void fillOCFeesUsage(OCFees* ocFees, const std::string padisTranslation)
  {
    OCFeesUsage* ocfUsage = _memHandle.create<OCFeesUsage>();
    ocfUsage->oCFees() = ocFees;
    ocfUsage->setSegIndex(0);
    ocfUsage->upgradeT198Sequence() = _memHandle.create<SvcFeesResBkgDesigInfo>();
    ocfUsage->upgradeT198CommercialName() = padisTranslation;
    ocFees->ocfeeUsage().push_back(ocfUsage);
  }

  void testGetFareUsageId()
  {
    XMLShoppingResponse response(*_trx);

    FareUsage fu1, fu2;
    size_t id1 = response.getFareUsageId(&fu1);
    size_t id2 = response.getFareUsageId(&fu2);
    size_t id3 = response.getFareUsageId(&fu1);

    CPPUNIT_ASSERT(id1 != id2);
    CPPUNIT_ASSERT(id1 == id3);
  }

  void test_getCommercialName_OCFeesUsage()
  {
    FarePath* farePath = _memHandle.create<FarePath>();

    TravelSeg* travelSeg_1 =
        TravelSegBuilder(_memHandle).setTravelStart("KRK").setTravelEnd("ARN").build();

    OCFees* ocFees_1 = OCFeesBuilder(_memHandle)
                           .setFarePath(farePath)
                           .setTravelStart(travelSeg_1)
                           .setTravelEnd(travelSeg_1)
                           .setSubCodeInfo("SA")
                           .addOcFeesUsage("LEFT SIDE")
                           .build();

    std::string commercialName =
        _shoppingResponse->getCommercialName(ocFees_1->ocfeeUsage().front());
    CPPUNIT_ASSERT_EQUAL(std::string("LEFT SIDE"), commercialName);
  }

  void test_getCommercialName_OCFeesUsage_Empty()
  {
    FarePath* farePath = _memHandle.create<FarePath>();

    TravelSeg* travelSeg_1 =
        TravelSegBuilder(_memHandle).setTravelStart("KRK").setTravelEnd("ARN").build();

    OCFees* ocFees_1 = OCFeesBuilder(_memHandle)
                           .setFarePath(farePath)
                           .setTravelStart(travelSeg_1)
                           .setTravelEnd(travelSeg_1)
                           .setSubCodeInfo("SA", "WINDOW")
                           .addOcFeesUsage("")
                           .build();

    std::string commercialName =
        _shoppingResponse->getCommercialName(ocFees_1->ocfeeUsage().front());
    CPPUNIT_ASSERT_EQUAL(std::string("WINDOW"), commercialName);
  }

  void testGenerateOCF()
  {
    FarePath* farePath = _memHandle.create<FarePath>();

    TravelSeg* travelSeg_1 =
        TravelSegBuilder(_memHandle).setTravelStart("KRK").setTravelEnd("ARN").build();
    TravelSeg* travelSeg_2 =
        TravelSegBuilder(_memHandle).setTravelStart("ARN").setTravelEnd("NYO").build();
    Itin* itin =
        ItinBuilder(_memHandle).addTravelSeg(travelSeg_1).addTravelSeg(travelSeg_2).build();

    ServiceFeesGroupBuilder serviceFeesGroupBuilder(_memHandle);
    serviceFeesGroupBuilder.setFarePath(farePath).setItin(itin).setPaxType("ADT");

    OCFees* ocFees_1 = OCFeesBuilder(_memHandle)
                           .setFarePath(farePath)
                           .setTravelStart(travelSeg_1)
                           .setTravelEnd(travelSeg_1)
                           .setSubCodeInfo("SA", "WINDOW")
                           .addOcFeesUsage("LEFT SIDE")
                           .addOcFeesUsage("RIGHT SIDE")
                           .build();

    OCFees* ocFees_2 = OCFeesBuilder(_memHandle)
                           .setFarePath(farePath)
                           .setTravelStart(travelSeg_2)
                           .setTravelEnd(travelSeg_2)
                           .setSubCodeInfo("SA", "WINDOW")
                           .addOcFeesUsage("")
                           .build();

    ServiceFeesGroup* sfg =
        serviceFeesGroupBuilder.setItin(itin).addOCFees(ocFees_1).addOCFees(ocFees_2).build();

    std::string result;
    _shoppingResponse->generateOCF(itin, sfg);

    _shoppingResponse->getXML(result);

    CPPUNIT_ASSERT(result.find("SF7=\"LEFT SIDE\" A01=\"KRK\" A02=\"ARN\"") != std::string::npos);
    CPPUNIT_ASSERT(result.find("SF7=\"RIGHT SIDE\" A01=\"KRK\" A02=\"ARN\"") != std::string::npos);
    CPPUNIT_ASSERT(result.find("SF7=\"WINDOW\" A01=\"ARN\" A02=\"NYO\"") != std::string::npos);
  }

  void testGenerateOBG()
  {
    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "USD";
    calcTotals.convertedBaseFareCurrencyCode = "";

    FarePath* farePath = _memHandle.create<FarePath>();

    _trx->ticketingDate() = DateTime::localTime();

    createOBFeeVector(farePath->collectedTTypeOBFee(), 'T', 2);
    _trx->getRequest()->setCollectTTypeOBFee(true);

    createOBFeeVector(farePath->collectedRTypeOBFee(), 'R', 2);
    _trx->getRequest()->setCollectRTypeOBFee(true);

    calcTotals.farePath = farePath;

    std::string result;
    _shoppingResponse->generateOBG(calcTotals);
    _shoppingResponse->getXML(result);
    //std::cout << std::endl << result << std::endl;

    CPPUNIT_ASSERT(result.find("<OBF SF0=\"OBT01\" SF1=\"5.00\" SDD=\"CARRIER TICKETING FEE01\"/>") != std::string::npos);
    CPPUNIT_ASSERT(result.find("<OBF SF0=\"OBT02\" SF1=\"5.00\" SDD=\"CARRIER TICKETING FEE02\"/>") != std::string::npos);
    CPPUNIT_ASSERT(result.find("<OBF SF0=\"OBR01\" SF1=\"5.00\" SDD=\"OVERNIGHT DELIVERY CHARGE\"/>") != std::string::npos);
    CPPUNIT_ASSERT(result.find("<OBF SF0=\"OBR02\" SF1=\"5.00\" SDD=\"COURIER CHARGE\"/>") != std::string::npos);
  }

  void testGenerateCPL()
  {
    TravelSeg* seg1 =
        TravelSegBuilder(_memHandle).setTravelStart("KRK").setTravelEnd("WAW").build();
    seg1->legId() = 0;
    dynamic_cast<AirSeg*>(seg1)->carrier() = "LO";

    TravelSeg* seg2 =
        TravelSegBuilder(_memHandle).setTravelStart("WAW").setTravelEnd("BER").build();
    seg2->legId() = 1;
    dynamic_cast<AirSeg*>(seg2)->carrier() = "LH";

    TravelSeg* seg3 =
        TravelSegBuilder(_memHandle).setTravelStart("BER").setTravelEnd("JFK").build();
    seg3->legId() = 2;
    dynamic_cast<AirSeg*>(seg3)->carrier() = "AB";

    TravelSeg* seg4 =
        TravelSegBuilder(_memHandle).setTravelStart("JFK").setTravelEnd("OYW").build();
    seg4->legId() = 3;
    dynamic_cast<AirSeg*>(seg4)->carrier() = "AA";

    Itin* itin =
        ItinBuilder(_memHandle).addTravelSeg(seg1).addTravelSeg(seg2).addTravelSeg(seg3).addTravelSeg(seg4).build();

    XMLWriter writer;
    XMLWriter::Node nodeITN(writer, "ITN");

    _shoppingResponse->generateCPL(itin, nodeITN);

    CPPUNIT_ASSERT(writer.result().find("CPL=\"LO|LH|AB|AA\"") != std::string::npos);

    // test generating string from empty vestor of carrier codes
    std::vector<CarrierCode> gcPerLeg;
    CPPUNIT_ASSERT(_shoppingResponse->generateCPLValueString(gcPerLeg).compare("") == 0); //strings are equal
  }

  void createOBFeeVector(std::vector<TicketingFeesInfo*>& obFeeVect, char type, size_t num)
  {
    for (size_t i = 1; i <= num; ++i)
    {
      obFeeVect.push_back(_memHandle(new TicketingFeesInfo));
      obFeeVect.back()->feeAmount() = 5.00;
      obFeeVect.back()->cur() = "USD";
      obFeeVect.back()->serviceTypeCode() = "OB";
      std::stringstream ss;
      ss << type << std::setw(2) << std::setfill('0') << i;
      obFeeVect.back()->serviceSubTypeCode() = ss.str();
    }
  }

  void testCalculateObFeeAmountFromAmountSameCurrency()
  {
    CurrencyCode paymentCur = "GBP";
    _feeInfo->cur() = "GBP";
    MoneyAmount testAmt = 100.00;
    _feeInfo->feeAmount() = testAmt;

    MoneyAmount calcAmt =
        _shoppingResponse->calculateObFeeAmountFromAmount(*_trx, _feeInfo, paymentCur);
    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmt);
  }

  void testPrintAdjMsg()
  {
    _options.setPDOForFRRule(false);
    _pTrx->setOptions(&_options);

    CalcTotals totals;
    _shoppingResponse->printAdjMsg(&totals);

    std::string result;
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find("ADJT AMT") == std::string::npos);

    _options.setPDOForFRRule(true);
    _shoppingResponse->printAdjMsg(&totals);
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find("ADJT AMT") == std::string::npos);

    CalcTotals adjustedCalcTotals;
    totals.adjustedCalcTotal = &adjustedCalcTotals;

    _shoppingResponse->printAdjMsg(&totals);
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find("ADJT AMT") == std::string::npos);

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    adjustedCalcTotals.adjustedSellingDiffInfo.push_back(a1);

    _shoppingResponse->printAdjMsg(&totals);
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find(
      "<MSG N06=\"X\" Q0K=\"0\">ADJT AMT  100.00</MSG>") != std::string::npos);

    AdjustedSellingDiffInfo a2("A2", "G", "88.88");
    adjustedCalcTotals.adjustedSellingDiffInfo.push_back(a2);
    _shoppingResponse->printAdjMsg(&totals);
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find(
      "<MSG N06=\"X\" Q0K=\"0\">ADJT AMT  100.00/88.88A2</MSG>") != std::string::npos);

    AdjustedSellingDiffInfo a3("X2", "G", "7.7");
    adjustedCalcTotals.adjustedSellingDiffInfo.push_back(a3);
    _shoppingResponse->printAdjMsg(&totals);
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find(
      "<MSG N06=\"X\" Q0K=\"0\">ADJT AMT  100.00/88.88A2/7.7X2</MSG>") != std::string::npos);
  }

  void testGenerateHPSAdjustedSellingData()
  {
    CalcTotals totals;
    _shoppingResponse->generateHPSAdjustedSellingData(&totals);

    std::string result;
    _shoppingResponse->getXML(result);
    CPPUNIT_ASSERT(result.find("HPS") == std::string::npos);

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    totals.adjustedSellingDiffInfo.push_back(a1);

    _shoppingResponse->generateHPSAdjustedSellingData(&totals);
    _shoppingResponse->getXML(result);

    CPPUNIT_ASSERT(result.find(
      "<HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/>") != std::string::npos);

    AdjustedSellingDiffInfo a2("A2", "G", "88.88");
    totals.adjustedSellingDiffInfo.push_back(a2);

   _shoppingResponse->generateHPSAdjustedSellingData(&totals);
   _shoppingResponse->getXML(result);

   CPPUNIT_ASSERT(result.find(
     "<HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/>") != std::string::npos);
   CPPUNIT_ASSERT(result.find(
     "<HPS T52=\"G\" N52=\"A2\" C52=\"88.88\"/>") != std::string::npos);
  }

  void testAddFareBrandDetails()
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareMarket* fM = _memHandle.create<FareMarket>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fare->initialize(Fare::FS_ConstructedFare, fareInfo, *fM, 0, 0);
    ptf->initialize(fare, 0, fM);
    ptf->fareMarket()->brandProgramIndexVec().push_back(0);
    ptf->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));

    BrandInfo* brand = _memHandle.create<BrandInfo>();
    brand->brandCode() = "BCODE";
    brand->brandName() = "BNAME";
    BrandProgram* bProgram = _memHandle.create<BrandProgram>();
    bProgram->brandsData().push_back(brand);
    bProgram->programCode() = "PCODE";
    bProgram->programName() = "PNAME";
    bProgram->programID() = "PID";
    bProgram->systemCode() = 'S';
    _pTrx->brandProgramVec().push_back(std::make_pair(bProgram, brand));

    _pTrx->setTnShoppingBrandingMode(TnShoppingBrandingMode::MULTIPLE_BRANDS);
    _pTrx->setRequest(&_request);
    _pTrx->setOptions(&_options);
    _pTrx->ticketingDate() = DateTime::localTime();

    XMLShoppingResponse sResponse(*_pTrx);
    XMLWriter writer;
    {
      XMLWriter::Node nodeFDC(writer, "FDC");
      // In _NO_BRAND case first valid brand from the fare should be used
      const BrandCode brandFromOuterSpace = NO_BRAND;
      sResponse.addFareBrandDetails(nodeFDC, *ptf, brandFromOuterSpace, Direction::BOTHWAYS);
    }
    std::ostringstream expectedAttributes;
    expectedAttributes << "SB2=\"BCODE\" "
                       << "SB3=\"BNAME\" "
                       << "SC2=\"PNAME\" "
                       << "SC1=\"S\" "
                       << "SC3=\"PID\" "
                       << "SC4=\"PCODE\" "
                       << "SC0=\"PCODE\"";
    CPPUNIT_ASSERT(writer.result().find(expectedAttributes.str()) != std::string::npos);
  }

  void testAddFareBrandDetailsOverride()
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareMarket* fM = _memHandle.create<FareMarket>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fare->initialize(Fare::FS_ConstructedFare, fareInfo, *fM, 0, 0);
    ptf->initialize(fare, 0, fM);
    ptf->fareMarket()->brandProgramIndexVec().push_back(0);
    ptf->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));

    //Now we have two valid brands in this fare
    ptf->fareMarket()->brandProgramIndexVec().push_back(1);
    ptf->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));

    BrandInfo* brand = _memHandle.create<BrandInfo>();
    brand->brandCode() = "FR";
    brand->brandName() = "FIRST";
    BrandInfo* brand2 = _memHandle.create<BrandInfo>();
    brand2->brandCode() = "SN";
    brand2->brandName() = "SECOND";
    BrandProgram* bProgram = _memHandle.create<BrandProgram>();
    bProgram->brandsData().push_back(brand);
    bProgram->programCode() = "PCODE";
    bProgram->programName() = "PNAME";
    bProgram->programID() = "PID";
    bProgram->systemCode() = 'S';
    _pTrx->brandProgramVec().push_back(std::make_pair(bProgram, brand));
    _pTrx->brandProgramVec().push_back(std::make_pair(bProgram, brand2));

    _pTrx->setTnShoppingBrandingMode(TnShoppingBrandingMode::MULTIPLE_BRANDS);
    _pTrx->setRequest(&_request);
    _pTrx->setOptions(&_options);
    _pTrx->ticketingDate() = DateTime::localTime();

    XMLShoppingResponse sResponse(*_pTrx);
    XMLWriter writer;
    {
      XMLWriter::Node nodeFDC(writer, "FDC");
      // in case a brand from the context is suggested it should be used instead of the first valid one
      const BrandCode brandFromOuterSpace = "SN";
      sResponse.addFareBrandDetails(nodeFDC, *ptf, brandFromOuterSpace, Direction::BOTHWAYS);
    }
    std::ostringstream expectedAttributes;
    expectedAttributes << "SB2=\"SN\" "
                       << "SB3=\"SECOND\" "
                       << "SC2=\"PNAME\" "
                       << "SC1=\"S\" "
                       << "SC3=\"PID\" "
                       << "SC4=\"PCODE\" "
                       << "SC0=\"PCODE\"";
    CPPUNIT_ASSERT(writer.result().find(expectedAttributes.str()) != std::string::npos);
  }

  void testGenerateErrorDiagnostic()
  {
    ErrorResponseException* error =
        _memHandle.create<ErrorResponseException>(
            ErrorResponseException::ErrorResponseCode::DATA_ERROR_DETECTED,
            "PAX TYPE FAR NOT FOUND FOR GIVEN FAREINFO");
    XMLShoppingResponse response(*_pTrx, error);
    _pTrx->setRequest(_memHandle.create<PricingRequest>());
    _pTrx->diagnostic().insertDiagMsg("DIAGNOSTIC MESSAGE");
    _pTrx->diagnostic().diagnosticType() = Diagnostic854;
    _pTrx->diagnostic().activate();

    response.generateError();
    std::string result;
    response.getXML(result);

    CPPUNIT_ASSERT(result.find(
        "  <DIA Q0K=\"5137\">\n"
        "    <MTP>\n"
        "<![CDATA[PAX TYPE FAR NOT FOUND FOR GIVEN FAREINFO\n"
        "DIAGNOSTIC MESSAGE]]>    </MTP>\n") != std::string::npos);
  }

  void testVendorCodeShouldBeAddedToFdcTag()
  {
    XMLShoppingResponse sResponse(*_pTrx);
    XMLWriter writer;
    XMLWriter::Node nodeFDC(writer, "FDC");
    const VendorCode vendoreCode = "ABC";
    sResponse.addVendorCodeToFDCNode(nodeFDC, vendoreCode);
    std::ostringstream expectedAttribute;
    expectedAttribute << "S37=\"ABC";
    CPPUNIT_ASSERT(writer.result().find(expectedAttribute.str()) != std::string::npos);
  }

  void testGenerateErrorDiagnosticNone()
  {
    ErrorResponseException* error =
        _memHandle.create<ErrorResponseException>(
            ErrorResponseException::ErrorResponseCode::DATA_ERROR_DETECTED,
            "PAX TYPE FAR NOT FOUND FOR GIVEN FAREINFO");
    XMLShoppingResponse response(*_pTrx, error);
    _pTrx->setRequest(_memHandle.create<PricingRequest>());
    _pTrx->diagnostic().insertDiagMsg("DIAGNOSTIC MESSAGE");
    _pTrx->diagnostic().diagnosticType() = DiagnosticNone;
    _pTrx->diagnostic().activate();

    response.generateError();
    std::string result;
    response.getXML(result);

    CPPUNIT_ASSERT(result.find(
        "  <DIA Q0K=\"5137\">\n"
        "    <MTP>\n"
        "<![CDATA[PAX TYPE FAR NOT FOUND FOR GIVEN FAREINFO\n"
        "]]>    </MTP>\n") != std::string::npos);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(XMLShoppingResponseTest);
}
