#include "Common/FareDisplayResponseUtil.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Routing.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FareDisplayResponseUtilStub : public FareDisplayResponseUtil
{
public:
  FareDisplayResponseUtilStub() : FareDisplayResponseUtil() {}

  ~FareDisplayResponseUtilStub() {}

  std::vector<TariffCrossRefInfo*> _tariffCRList;

  // ----------------------------------------------------------------
  //  @MethodName   FareDisplayResponseUtilStub::getCrossRefType()
  //  This is a mock function. Written to bypass base class function.
  // ----------------------------------------------------------------
  RecordScope getCrossRefType(const FareDisplayTrx& trx) { return DOMESTIC; }

  // ----------------------------------------------------------------
  //  @MethodName   getTariffXRefByRuleTariff()
  //  This is a mock function. Written to bypass base class function.
  // ----------------------------------------------------------------
  const std::vector<TariffCrossRefInfo*>& getTariffXRefByRuleTariff(DataHandle& dataHandle,
                                                                    const VendorCode& vendor,
                                                                    const CarrierCode& carrier,
                                                                    const RecordScope& crossRefType,
                                                                    const TariffNumber& ruleTariff,
                                                                    const DateTime& travelDate)
  {
    if (_tariffCRList.size() > 0)
      _tariffCRList.clear();

    static TariffCrossRefInfo tCRI1, tCRI2;

    tCRI1.fareTariff() = 1;
    tCRI1.ruleTariffCode() = "DOUG";

    tCRI2.fareTariff() = 0;
    tCRI2.ruleTariffCode() = "PARTHA";

    _tariffCRList.push_back(&tCRI1);
    _tariffCRList.push_back(&tCRI2);

    return _tariffCRList;
  }

  // ----------------------------------------------------------------
  //  @MethodName   getTariffXRefByGenRuleTariff()
  //  This is a mock function. Written to bypass base class function.
  // ----------------------------------------------------------------
  const std::vector<TariffCrossRefInfo*>&
  getTariffXRefByGenRuleTariff(DataHandle& dataHandle,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const RecordScope& crossRefType,
                               const TariffNumber& ruleTariff,
                               const DateTime& travelDate)
  {
    if (_tariffCRList.size() > 0)
      _tariffCRList.clear();

    static TariffCrossRefInfo tCRI1, tCRI2;

    tCRI1.fareTariff() = 2;
    tCRI1.governingTariffCode() = "GREG";

    tCRI2.fareTariff() = 3;
    tCRI2.governingTariffCode() = "MIKE";

    _tariffCRList.push_back(&tCRI1);
    _tariffCRList.push_back(&tCRI2);

    return _tariffCRList;
  }
};

class FareDisplayResponseUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayResponseUtilTest);
  CPPUNIT_TEST(testIsCTRWReturnFalseWhenNotCTRW);
  CPPUNIT_TEST(testGetRuleTariffDescription);
  CPPUNIT_TEST(testGetGenRuleTariffDescription);

  CPPUNIT_TEST(testRoutingNumberToNumeric_Numeric);
  CPPUNIT_TEST(testRoutingNumberToNumeric_AlphaNumeric);
  CPPUNIT_TEST(testRoutingNumberToNumeric_Alpha);
  CPPUNIT_TEST(testRoutingNumberToString_Numeric);
  CPPUNIT_TEST(testRoutingNumberToString_AlphaNumeric);
  CPPUNIT_TEST(testRoutingNumberToString_Alpha);
  CPPUNIT_TEST(testRoutingNumberToStringFormat_Numeric);
  CPPUNIT_TEST(testRoutingNumberToStringFormat_AlphaNumeric);
  CPPUNIT_TEST(testRoutingNumberToStringFormat_Alpha);
  CPPUNIT_TEST(testRoutingNumberToStringFormat2_left);
  CPPUNIT_TEST(testRoutingNumberToStringFormat2_right);

  // Existing functionality
  CPPUNIT_TEST(displayConstructed_RTG_RTG_MPM);
  CPPUNIT_TEST(displayConstructed_MPM_RTG_RTG);
  CPPUNIT_TEST(displayConstructed_RTG_MPM_MPM);
  CPPUNIT_TEST(displayConstructed_MPM_RTG_MPM);
  CPPUNIT_TEST(displayConstructed_MPM_MPM_RTG);
  CPPUNIT_TEST(displayConstructed_RTG_MPM____);
  CPPUNIT_TEST(displayConstructed_____MPM_RTG);
  CPPUNIT_TEST(displayConstructed_MPM_RTG____);
  CPPUNIT_TEST(displayConstructed_____RTG_MPM);

  CPPUNIT_TEST(displayConstructed_RNO_RTG____);
  CPPUNIT_TEST(displayConstructed_____RTG_RNO);

  CPPUNIT_TEST(displayConstructed_RNO_RTG_MPM);
  CPPUNIT_TEST(displayConstructed_MPM_RTG_RNO);
  CPPUNIT_TEST(displayConstructed_RNO_RTG_RTG);
  CPPUNIT_TEST(displayConstructed_RTG_RTG_RNO);
  CPPUNIT_TEST(displayConstructed_RNO_RTG_RNO);

  CPPUNIT_TEST(addRTGMPM_RTW);
  CPPUNIT_TEST(addRTGMPM);

  CPPUNIT_TEST(testAddCatText_Seasons);
  CPPUNIT_TEST(testAddCatText_Ic);
  CPPUNIT_TEST(testAddCatText_Retiler);

  CPPUNIT_TEST_SUITE_END();

  enum RoutingType
  {
    RT_MPM,
    RT_RTG,
    RT_RNO, // routing without map (no map)
    RT_EMP // empty, no routing
  };

  class MyDataHandle : public DataHandleMock
  {
  public:
    const RuleCategoryDescInfo* getRuleCategoryDesc(const CatNumber& key)
    {
      _rcdi.defaultMsg() = std::to_string(key);
      return &_rcdi;
    }
  private:
    RuleCategoryDescInfo _rcdi;
  };

public:
  void setUp()
  {
    createPaxTypeFare();
    _trx = _memH.create<FareDisplayTrx>();
    _memH.create<MyDataHandle>();
  }

  void tearDown() {}

  FareMarket* createFareMarket()
  {
    FareMarket* fm = _memH(new FareMarket);
    fm->paxTypeCortege().resize(1);
    return fm;
  }

  PaxType* createPaxType(PaxTypeCode code)
  {
    PaxType* pt = _memH(new PaxType);
    pt->paxType() = code;
    // pt->paxTypeInfo() = _memH(new PaxTypeInfo);
    return pt;
  }

  void createPaxTypeFare(uint16_t brandId = 0)
  {
    FareInfo* fi = _memH(new FareInfo);
    fi->carrier() = "CX";
    fi->currency() = NUC;

    TariffCrossRefInfo* tcr = _memH(new TariffCrossRefInfo);
    tcr->ruleTariff() = 0;

    Fare* f = _memH(new Fare);
    f->setTariffCrossRefInfo(tcr);

    f->setInvBrand(brandId, false);

    f->setFareInfo(fi);

    _paxTypeFare = _memH(new PaxTypeFare);
    _paxTypeFare->setFare(f);
    _paxTypeFare->fareMarket() = createFareMarket();
    FareClassAppInfo* fca = _memH(new FareClassAppInfo);
    fca->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    _paxTypeFare->fareClassAppInfo() = fca;
    _paxTypeFare->paxType() = createPaxType(ADULT);
    _paxTypeFare->nucFareAmount() = 100;
  }

  void testIsCTRWReturnFalseWhenNotCTRW()
  {
    FareDisplayTrx trx;
    GlobalDirection gld = GlobalDirection::AT;
    RoutingInfo info;
    FareDisplayResponseUtil fdru;
    CPPUNIT_ASSERT(!fdru.isCTRW(trx, gld, info));
  }

  void testGetRuleTariffDescription()
  {
    TariffCode ruleTariffCode;
    FareDisplayResponseUtilStub fdruStub;
    // -------------------------------
    // Test1: Input : FareTariff = 0
    //        Otherinputs are dummy
    // Expected output: "PARTHA" in ruleTariffCode
    // -------------------------------
    fdruStub.getRuleTariffDescription(*_trx, 0, "ATP", "AA", 11, ruleTariffCode);
    CPPUNIT_ASSERT(ruleTariffCode == "PARTHA");

    // -------------------------------
    // Test2: Input : FareTariff = 2
    //        Otherinputs are dummy
    // Expected output: <blank_string>
    // -------------------------------
    fdruStub.getRuleTariffDescription(*_trx, 2, "ATP", "AA", 11, ruleTariffCode);
    CPPUNIT_ASSERT(ruleTariffCode.empty());
  }

  void testGetGenRuleTariffDescription()
  {
    TariffCode ruleTariffCode;
    GeneralRuleRecord2Info fGR2;
    fGR2.vendorCode() = "ATP";
    fGR2.carrierCode() = "AA";
    fGR2.tariffNumber() = 10;
    FareDisplayResponseUtilStub fdruStub;
    // -------------------------------
    // Test1: Input : FareTariff = 3
    //        Otherinputs are dummy
    // Expected output: "MIKE" in ruleTariffCode
    // -------------------------------
    fdruStub.getRuleTariffDescription(*_trx, 3, &fGR2, ruleTariffCode);
    CPPUNIT_ASSERT(ruleTariffCode == "MIKE");

    // -------------------------------
    // Test2: Input : FareTariff = 1
    //        Otherinputs are dummy
    // Expected output: <blank string> in ruleTariffCode
    // -------------------------------
    fdruStub.getRuleTariffDescription(*_trx, 1, &fGR2, ruleTariffCode);
    CPPUNIT_ASSERT(ruleTariffCode.empty());
  }

  void testRoutingNumberToNumeric_Numeric()
  {
    bool res = false;
    int16_t rn = 0;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    fareInfo->routingNumber() = "0000";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 0);

    fareInfo->routingNumber() = "0002";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 2);

    fareInfo->routingNumber() = "1001";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 1001);

    fareInfo->routingNumber() = "0800";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 800);

    fareInfo->routingNumber() = "6072";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 6072);

    fareInfo->routingNumber() = "44";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 44);
  }

  void testRoutingNumberToNumeric_AlphaNumeric()
  {
    bool res = false;
    int16_t rn = 0;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    rn = 0;
    fareInfo->routingNumber() = "000A";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "001M";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "05FX";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "05G0";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "05G9";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);
  }

  void testRoutingNumberToNumeric_Alpha()
  {
    bool res = false;
    int16_t rn = 0;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    rn = 0;
    fareInfo->routingNumber() = "ABCD";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "ABC";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "AB";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "A";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == false && rn == 0);

    rn = 0;
    fareInfo->routingNumber() = "";
    res = FareDisplayResponseUtil::routingNumberToNumeric(_paxTypeFare->routingNumber(), rn);
    CPPUNIT_ASSERT(res == true && rn == 0);
  }

  void testRoutingNumberToString_Numeric()
  {
    std::string res;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    fareInfo->routingNumber() = "0000";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "0");

    fareInfo->routingNumber() = "0002";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "2");

    fareInfo->routingNumber() = "1001";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "1001");

    fareInfo->routingNumber() = "0800";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "800");

    fareInfo->routingNumber() = "6072";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "6072");

    fareInfo->routingNumber() = "44";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "44");
  }

  void testRoutingNumberToString_AlphaNumeric()
  {
    std::string res;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    fareInfo->routingNumber() = "000A";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "A");

    fareInfo->routingNumber() = "001M";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "1M");

    fareInfo->routingNumber() = "05FX";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "5FX");

    fareInfo->routingNumber() = "05G0";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "5G0");

    fareInfo->routingNumber() = "05G9";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "5G9");
  }

  void testRoutingNumberToString_Alpha()
  {
    std::string res;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    fareInfo->routingNumber() = "ABCD";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "ABCD");

    fareInfo->routingNumber() = "ABC";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "ABC");

    fareInfo->routingNumber() = "AB";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "AB");

    fareInfo->routingNumber() = "A";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "A");

    fareInfo->routingNumber() = "";
    res = FareDisplayResponseUtil::routingNumberToString(_paxTypeFare->routingNumber());
    CPPUNIT_ASSERT(res == "0");
  }

  void testRoutingNumberToStringFormat_Numeric()
  {
    std::string res;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    // Please take a look at this!
    fareInfo->routingNumber() = "0000";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::left);
    CPPUNIT_ASSERT(res == "0   ");

    fareInfo->routingNumber() = "0002";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "   2");

    fareInfo->routingNumber() = "1001";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::left);
    CPPUNIT_ASSERT(res == "1001");

    fareInfo->routingNumber() = "0800";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == " 800");

    fareInfo->routingNumber() = "6072";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::left);
    CPPUNIT_ASSERT(res == "6072");

    fareInfo->routingNumber() = "44";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "  44");
  }

  void testRoutingNumberToStringFormat_AlphaNumeric()
  {
    std::string res;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    fareInfo->routingNumber() = "000A";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "   A");

    fareInfo->routingNumber() = "001M";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "  1M");

    fareInfo->routingNumber() = "05FX";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == " 5FX");

    fareInfo->routingNumber() = "05G0";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == " 5G0");

    fareInfo->routingNumber() = "05G9";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == " 5G9");
  }

  void testRoutingNumberToStringFormat_Alpha()
  {
    std::string res;
    FareInfo* fareInfo = const_cast<FareInfo*>(_paxTypeFare->fare()->fareInfo());

    fareInfo->routingNumber() = "ABCD";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "ABCD");

    fareInfo->routingNumber() = "ABC";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == " ABC");

    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::left);
    CPPUNIT_ASSERT(res == "ABC ");

    fareInfo->routingNumber() = "AB";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "  AB");

    fareInfo->routingNumber() = "A";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "   A");

    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::left);
    CPPUNIT_ASSERT(res == "A   ");

    fareInfo->routingNumber() = "";
    res = FareDisplayResponseUtil::routingNumberToStringFormat(_paxTypeFare->routingNumber(),
                                                               std::ios::right);
    CPPUNIT_ASSERT(res == "   0");
  }

  void testRoutingNumberToStringFormat2_left()
  {
    uint16_t rn;
    std::string res;

    rn = 0;
    res = FareDisplayResponseUtil::routingNumberToStringFormat(rn, std::ios::left);
    CPPUNIT_ASSERT(res == "0   ");

    rn = 6072;
    res = FareDisplayResponseUtil::routingNumberToStringFormat(rn, std::ios::left);
    CPPUNIT_ASSERT(res == "6072");

    rn = 44;
    res = FareDisplayResponseUtil::routingNumberToStringFormat(rn, std::ios::left);
    CPPUNIT_ASSERT(res == "44  ");

    rn = 1001;
    res = FareDisplayResponseUtil::routingNumberToStringFormat(rn, std::ios::left);
    CPPUNIT_ASSERT(res == "1001");
  }

  void testRoutingNumberToStringFormat2_right()
  {
    uint16_t rn;
    std::string res;

    rn = 800;
    res = FareDisplayResponseUtil::routingNumberToStringFormat(rn, std::ios::right);
    CPPUNIT_ASSERT(res == " 800");

    rn = 2;
    res = FareDisplayResponseUtil::routingNumberToStringFormat(rn, std::ios::right);
    CPPUNIT_ASSERT(res == "   2");
  }

  void displayConstructed_RTG_RTG_MPM()
  {
    initDisplayConstructed(RT_RTG, RT_RTG, RT_MPM);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "KRK AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_MPM_RTG_RTG()
  {
    initDisplayConstructed(RT_MPM, RT_RTG, RT_RTG);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND NYC ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_RTG_MPM_MPM()
  {
    initDisplayConstructed(RT_RTG, RT_MPM, RT_MPM);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "KRK AND PL GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_MPM_RTG_MPM()
  {
    initDisplayConstructed(RT_MPM, RT_RTG, RT_MPM);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_MPM_MPM_RTG()
  {
    initDisplayConstructed(RT_MPM, RT_MPM, RT_RTG);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "US CA GATEWAY AND NYC ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_RTG_MPM____()
  {
    initDisplayConstructed(RT_RTG, RT_MPM, RT_EMP);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "KRK AND PL GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_____MPM_RTG()
  {
    initDisplayConstructed(RT_EMP, RT_MPM, RT_RTG);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "US CA GATEWAY AND NYC ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_MPM_RTG____()
  {
    initDisplayConstructed(RT_MPM, RT_RTG, RT_EMP);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND NYC ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_____RTG_MPM()
  {
    initDisplayConstructed(RT_EMP, RT_RTG, RT_MPM);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "KRK AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  // new functionality
  void displayConstructed_RNO_RTG____()
  {
    initDisplayConstructed(RT_RNO, RT_RTG, RT_EMP);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "THE ORIGIN AND PL GATEWAY ONLY\n";

    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND NYC ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_____RTG_RNO()
  {
    initDisplayConstructed(RT_EMP, RT_RTG, RT_RNO);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "US CA GATEWAY AND THE DESTINATION ONLY\n";

    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "KRK AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_RNO_RTG_MPM()
  {
    initDisplayConstructed(RT_RNO, RT_RTG, RT_MPM);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "THE ORIGIN AND PL GATEWAY ONLY\n";

    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_MPM_RTG_RNO()
  {
    initDisplayConstructed(RT_MPM, RT_RTG, RT_RNO);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "US CA GATEWAY AND THE DESTINATION ONLY\n";

    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_RNO_RTG_RTG()
  {
    initDisplayConstructed(RT_RNO, RT_RTG, RT_RTG);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "THE ORIGIN AND PL GATEWAY ONLY\n";

    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND NYC ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_RTG_RTG_RNO()
  {
    initDisplayConstructed(RT_RTG, RT_RTG, RT_RNO);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "US CA GATEWAY AND THE DESTINATION ONLY\n";

    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "KRK AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void displayConstructed_RNO_RTG_RNO()
  {
    initDisplayConstructed(RT_RNO, RT_RTG, RT_RNO);
    FareDisplayResponseUtil::displayConstructed(*_trx, *_paxTypeFare, _routingInfo, false);

    std::ostringstream response;
    response << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
    response << "PL GATEWAY AND US CA GATEWAY ONLY\n";

    CPPUNIT_ASSERT_EQUAL(response.str(), _trx->response().str());
  }

  void addRTGMPM()
  {
    initDisplayConstructed(RT_MPM, RT_MPM, RT_MPM);
    _trx->setOptions(_memH.create<FareDisplayOptions>());
    FareDisplayResponseUtil::addRTGMPM(*_trx, *_paxTypeFare, _routingInfo);

    std::string response(" MPM 0\n");

    CPPUNIT_ASSERT_EQUAL(response, _trx->response().str());
  }

  void addRTGMPM_RTW()
  {
    initDisplayConstructed(RT_MPM, RT_MPM, RT_MPM);
    FareDisplayOptions* opt = _memH.create<FareDisplayOptions>();
    opt->setRtw(true);
    _trx->setOptions(opt);
    FareDisplayResponseUtil::addRTGMPM(*_trx, *_paxTypeFare, _routingInfo);

    std::string response(" MPM \n");

    CPPUNIT_ASSERT_EQUAL(response, _trx->response().str());
  }

  void initDisplayConstructed(RoutingType origAddOn, RoutingType base, RoutingType destAddOn)
  {
    _routingInfo = _memH.create<RoutingInfo>();

    _routingInfo->mileageInfo() = _memH.create<MileageInfo>();
    _routingInfo->origAddOnRouting() = makeRouting(origAddOn);
    _routingInfo->routing() = makeRouting(base);
    _routingInfo->destAddOnRouting() = makeRouting(destAddOn);

    _paxTypeFare->fareMarket()->boardMultiCity() = "KRK";
    _paxTypeFare->fareMarket()->offMultiCity() = "NYC";

    Loc* originLoc = _memH.create<Loc>();
    Loc* destLoc = _memH.create<Loc>();

    originLoc->nation() = "PL";
    destLoc->nation() = "US";

    _paxTypeFare->fareMarket()->origin() = originLoc;
    _paxTypeFare->fareMarket()->destination() = destLoc;
  }

  Routing* makeRouting(RoutingType type)
  {
    Routing* routing = _memH.create<Routing>();
    switch (type)
    {
    case RT_EMP:
      return NULL;
      break;

    case RT_MPM:
      routing->routing() = MILEAGE_ROUTING;
      break;

    case RT_RTG:
      // any routing but not MILEAGE_ROUTING
      routing->routing() = "0001";
      routing->rmaps().push_back(new RoutingMap());
      break;

    case RT_RNO:
      // any routing but not MILEAGE_ROUTING
      routing->routing() = "0001";
      break;
    }
    return routing;
  }

  void testAddCatText_Seasons()
  {
    std::ostringstream stringStream;
    FareDisplayResponseUtil::addCatText(
        *_trx, SEASONS_RULE_CATEGORY, createFareDisplayInfo(), &stringStream);
    CPPUNIT_ASSERT_EQUAL((std::string) "   3\n", stringStream.str());
  }

  void testAddCatText_Ic()
  {
    std::ostringstream stringStream;
    FareDisplayResponseUtil::addCatText(
        *_trx, IC_RULE_CATEGORY, createFareDisplayInfo(), &stringStream);
    CPPUNIT_ASSERT_EQUAL((std::string) "", stringStream.str());
  }

  void testAddCatText_Retiler()
  {
    std::ostringstream stringStream;
    FareDisplayResponseUtil::addCatText(
        *_trx, RETAILER_CATEGORY, createFareDisplayInfo(), &stringStream);
    CPPUNIT_ASSERT_EQUAL((std::string) "", stringStream.str());
  }

  FareDisplayInfo& createFareDisplayInfo() { return *_memH.create<FareDisplayInfo>(); }

  FareDisplayTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  TestMemHandle _memH;
  RoutingInfo* _routingInfo;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayResponseUtilTest);
}
