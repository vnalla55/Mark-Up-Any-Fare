#include "test/include/CppUnitHelperMacros.h"

#include "Common/RestrictionsText.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/RoutingRestriction.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include <boost/assign/std/vector.hpp>
#include <boost/function.hpp>

using namespace std;
using namespace boost::assign;

namespace tse
{

class RestrictionsTextTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RestrictionsTextTest);

  CPPUNIT_TEST(testRestriction1_ApplPermitted);
  CPPUNIT_TEST(testRestriction1_ApplBlank);
  CPPUNIT_TEST(testRestriction1_ApplRequired);
  CPPUNIT_TEST(testRestriction1_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction2_ApplPermitted);
  CPPUNIT_TEST(testRestriction2_ApplBlank);
  CPPUNIT_TEST(testRestriction2_ApplRequired);
  CPPUNIT_TEST(testRestriction2_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction3_ApplPermitted_Nonstop);
  CPPUNIT_TEST(testRestriction3_ApplPermitted_Direct);
  CPPUNIT_TEST(testRestriction3_ApplPermitted_Blank);
  CPPUNIT_TEST(testRestriction3_ApplBlank_Nonstop);
  CPPUNIT_TEST(testRestriction3_ApplBlank_Direct);
  CPPUNIT_TEST(testRestriction3_ApplBlank_Blank);
  CPPUNIT_TEST(testRestriction3_ApplRequired_Nonstop);
  CPPUNIT_TEST(testRestriction3_ApplRequired_Direct);
  CPPUNIT_TEST(testRestriction3_ApplRequired_Blank);
  CPPUNIT_TEST(testRestriction3_ApplNotPermitted_Nonstop);
  CPPUNIT_TEST(testRestriction3_ApplNotPermitted_Direct);
  CPPUNIT_TEST(testRestriction3_ApplNotPermitted_Blank);
  CPPUNIT_TEST(testRestriction4_ApplPermitted_Nonstop);
  CPPUNIT_TEST(testRestriction4_ApplPermitted_Direct);
  CPPUNIT_TEST(testRestriction4_ApplPermitted_Blank);
  CPPUNIT_TEST(testRestriction4_ApplBlank_Nonstop);
  CPPUNIT_TEST(testRestriction4_ApplBlank_Direct);
  CPPUNIT_TEST(testRestriction4_ApplBlank_Blank);
  CPPUNIT_TEST(testRestriction4_ApplRequired_Nonstop);
  CPPUNIT_TEST(testRestriction4_ApplRequired_Direct);
  CPPUNIT_TEST(testRestriction4_ApplRequired_Blank);
  CPPUNIT_TEST(testRestriction4_ApplNotPermitted_Nonstop);
  CPPUNIT_TEST(testRestriction4_ApplNotPermitted_Direct);
  CPPUNIT_TEST(testRestriction4_ApplNotPermitted_Blank);
  CPPUNIT_TEST(testRestriction5_ApplPermitted);
  CPPUNIT_TEST(testRestriction5_ApplBlank);
  CPPUNIT_TEST(testRestriction5_ApplRequired);
  CPPUNIT_TEST(testRestriction5_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction6_ApplPermitted_Nonstop);
  CPPUNIT_TEST(testRestriction6_ApplPermitted_Direct);
  CPPUNIT_TEST(testRestriction6_ApplPermitted_Blank);
  CPPUNIT_TEST(testRestriction6_ApplBlank_Nonstop);
  CPPUNIT_TEST(testRestriction6_ApplBlank_Direct);
  CPPUNIT_TEST(testRestriction6_ApplBlank_Blank);
  CPPUNIT_TEST(testRestriction6_ApplRequired_Nonstop);
  CPPUNIT_TEST(testRestriction6_ApplRequired_Direct);
  CPPUNIT_TEST(testRestriction6_ApplRequired_Blank);
  CPPUNIT_TEST(testRestriction6_ApplNotPermitted_Nonstop);
  CPPUNIT_TEST(testRestriction6_ApplNotPermitted_Direct);
  CPPUNIT_TEST(testRestriction6_ApplNotPermitted_Blank);
  CPPUNIT_TEST(testRestriction7_ApplPermitted);
  CPPUNIT_TEST(testRestriction7_ApplBlank);
  CPPUNIT_TEST(testRestriction7_ApplRequired);
  CPPUNIT_TEST(testRestriction7_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction8_ApplPermitted);
  CPPUNIT_TEST(testRestriction8_ApplBlank);
  CPPUNIT_TEST(testRestriction8_ApplRequired);
  CPPUNIT_TEST(testRestriction8_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction9FMR);
  CPPUNIT_TEST(testRestriction9FMR_noRTW);
  CPPUNIT_TEST(testRestriction10_ApplPermitted);
  CPPUNIT_TEST(testRestriction10_ApplBlank);
  CPPUNIT_TEST(testRestriction10_ApplRequired);
  CPPUNIT_TEST(testRestriction10_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction11_ApplPermitted_Air);
  CPPUNIT_TEST(testRestriction11_ApplPermitted_Surface);
  CPPUNIT_TEST(testRestriction11_ApplPermitted_Blank);
  CPPUNIT_TEST(testRestriction11_ApplBlank_Air);
  CPPUNIT_TEST(testRestriction11_ApplBlank_Surface);
  CPPUNIT_TEST(testRestriction11_ApplBlank_Blank);
  CPPUNIT_TEST(testRestriction11_ApplRequired_Air);
  CPPUNIT_TEST(testRestriction11_ApplRequired_Surface);
  CPPUNIT_TEST(testRestriction11_ApplRequired_Blank);
  CPPUNIT_TEST(testRestriction11_ApplNotPermitted_Air);
  CPPUNIT_TEST(testRestriction11_ApplNotPermitted_Surface);
  CPPUNIT_TEST(testRestriction11_ApplNotPermitted_Blank);
  CPPUNIT_TEST(testRestriction12_ApplPermitted);
  CPPUNIT_TEST(testRestriction12_ApplBlank);
  CPPUNIT_TEST(testRestriction12_ApplRequired);
  CPPUNIT_TEST(testRestriction12_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction16);
  CPPUNIT_TEST(testRestriction17_ZZ);
  CPPUNIT_TEST(testRestriction17_Blank);
  CPPUNIT_TEST(testRestriction21_ApplPermitted);
  CPPUNIT_TEST(testRestriction21_ApplBlank);
  CPPUNIT_TEST(testRestriction21_ApplRequired);
  CPPUNIT_TEST(testRestriction21_ApplNotPermitted);

  CPPUNIT_TEST(testRestriction1It_ApplPermitted);
  CPPUNIT_TEST(testRestriction1It_ApplBlank);
  CPPUNIT_TEST(testRestriction1It_ApplRequired);
  CPPUNIT_TEST(testRestriction1It_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction2It_ApplPermitted);
  CPPUNIT_TEST(testRestriction2It_ApplBlank);
  CPPUNIT_TEST(testRestriction2It_ApplRequired);
  CPPUNIT_TEST(testRestriction2It_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction5It_ApplPermitted);
  CPPUNIT_TEST(testRestriction5It_ApplBlank);
  CPPUNIT_TEST(testRestriction5It_ApplRequired);
  CPPUNIT_TEST(testRestriction5It_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction7It_ApplPermitted);
  CPPUNIT_TEST(testRestriction7It_ApplBlank);
  CPPUNIT_TEST(testRestriction7It_ApplRequired);
  CPPUNIT_TEST(testRestriction7It_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction10It_ApplPermitted);
  CPPUNIT_TEST(testRestriction10It_ApplBlank);
  CPPUNIT_TEST(testRestriction10It_ApplRequired);
  CPPUNIT_TEST(testRestriction10It_ApplNotPermitted);
  CPPUNIT_TEST(testRestriction17It_ZZ);
  CPPUNIT_TEST(testRestriction17It_Blank);
  CPPUNIT_TEST(testRestriction21It_ApplPermitted);
  CPPUNIT_TEST(testRestriction21It_ApplBlank);
  CPPUNIT_TEST(testRestriction21It_ApplRequired);
  CPPUNIT_TEST(testRestriction21It_ApplNotPermitted);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _stream = _memHandle.create<std::ostringstream>();
    _res = _memHandle.insert(new RestrictionsText(*_stream));
    _rest = _memHandle.create<RoutingRestriction>();
    _rest->negViaAppl() = PERMITTED;
    _rest->market1() = "XXX";
    _rest->market2() = "YYY";
    _rest->viaMarket() = "ZZZ";
    _rest->nonStopDirectInd() = NONSTOP;
    _rest->viaCarrier() = "ZZ";
    std::vector<RoutingRestriction*>* vec = _memHandle.create<std::vector<RoutingRestriction*> >();
    *vec += _rest, getRtgRest("AAA"), getRtgRest("BBB"), getRtgRest("CCC"), getRtgRest("DDD"),
        getRtgRest("EEE");
    _restItI = vec->begin();
    _restItE = vec->end();
  }
  void tearDown() { _memHandle.clear(); }
  RoutingRestriction* getRtgRest(LocCode viaMarket)
  {
    RoutingRestriction* ret = _memHandle.create<RoutingRestriction>();
    ret->viaMarket() = viaMarket;
    ret->viaCarrier() = viaMarket.substr(0, 2);
    return ret;
  }
  template <typename T>
  void
  testResponse(const char* res, void (RestrictionsText::*methodToTest)(T, const bool&), T param)
  {
    // restrction function
    *_stream << " 101*  ";
    (_res->*methodToTest)(param, false);
    *_stream << " ";
    (_res->*methodToTest)(param, true);
    std::ostringstream resp;
    resp << " 101*  " << " " << res;
    // if empty res expected, then we won't add indent
    resp << (strlen(res) ? "        " : "") << res;
    CPPUNIT_ASSERT_EQUAL(resp.str(), _res->oss().str());
  }
  void testItResponse(const char* res)
  {
    // iterator function
    *_stream << " 101*  ";
    _funItPtr(_res, _restItI, _restItE, false);
    *_stream << " ";
    _funItPtr(_res, _restItI, _restItE, true);
    std::ostringstream resp;
    resp << " 101*  " << " " << res;
    // if empty res expected, then we won't add indent
    resp << (strlen(res) ? "        " : "") << res;
    CPPUNIT_ASSERT_EQUAL(resp.str(), _res->oss().str());
  }

  void testRestriction1_ApplPermitted()
  {
    testResponse<const RoutingRestriction&>(
        "RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n",
        &RestrictionsText::restriction1,
        *_rest);
  }
  void testRestriction1_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    testResponse<const RoutingRestriction&>(
        "RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n",
        &RestrictionsText::restriction1,
        *_rest);
  }
  void testRestriction1_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    //_funPtr = static_cast<void(RestrictionsText::*)(const RoutingRestriction&, const
    //bool&)>(&RestrictionsText::restriction1);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE VIA ZZZ\n", &RestrictionsText::restriction1, *_rest);
  }
  void testRestriction1_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction1);
    testResponse<const RoutingRestriction&>("TRAVEL BETWEEN XXX AND YYY MUST NOT BE VIA ZZZ\n",
                                            &RestrictionsText::restriction1,
                                            *_rest);
  }
  void testRestriction2_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction2);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE VIA ZZZ\n", &RestrictionsText::restriction2, *_rest);
  }
  void testRestriction2_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction2);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE VIA ZZZ\n", &RestrictionsText::restriction2, *_rest);
  }
  void testRestriction2_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction2);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE VIA ZZZ\n", &RestrictionsText::restriction2, *_rest);
  }
  void testRestriction2_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction2);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE VIA ZZZ\n", &RestrictionsText::restriction2, *_rest);
  }
  void testRestriction3_ApplPermitted_Nonstop()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE NONSTOP\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplPermitted_Direct()
  {
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplPermitted_Blank()
  {
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE NONSTOP OR DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplBlank_Nonstop()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE NONSTOP\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplBlank_Direct()
  {
    _rest->negViaAppl() = BLANK;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplBlank_Blank()
  {
    _rest->negViaAppl() = BLANK;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE NONSTOP OR DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplRequired_Nonstop()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE NONSTOP\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplRequired_Direct()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplRequired_Blank()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST BE NONSTOP OR DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplNotPermitted_Nonstop()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE NONSTOP\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplNotPermitted_Direct()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction3_ApplNotPermitted_Blank()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction3);
    testResponse<const RoutingRestriction&>(
        "TRAVEL MUST NOT BE NONSTOP OR DIRECT\n", &RestrictionsText::restriction3, *_rest);
  }
  void testRestriction4_ApplPermitted_Nonstop()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE NONSTOP\n", &RestrictionsText::restriction4, *_rest);
  }
  void testRestriction4_ApplPermitted_Direct()
  {
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE DIRECT\n", &RestrictionsText::restriction4, *_rest);
  }
  void testRestriction4_ApplPermitted_Blank()
  {
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE NONSTOP OR DIRECT\n",
        &RestrictionsText::restriction4,
        *_rest);
  }
  void testRestriction4_ApplBlank_Nonstop()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>("TRAVEL BETWEEN XXX AND YYY MUST NOT BE NONSTOP\n",
                                            &RestrictionsText::restriction4,
                                            *_rest);
  }
  void testRestriction4_ApplBlank_Direct()
  {
    _rest->negViaAppl() = BLANK;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST NOT BE DIRECT\n", &RestrictionsText::restriction4, *_rest);
  }
  void testRestriction4_ApplBlank_Blank()
  {
    _rest->negViaAppl() = BLANK;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST NOT BE NONSTOP OR DIRECT\n",
        &RestrictionsText::restriction4,
        *_rest);
  }
  void testRestriction4_ApplRequired_Nonstop()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE NONSTOP\n", &RestrictionsText::restriction4, *_rest);
  }
  void testRestriction4_ApplRequired_Direct()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE DIRECT\n", &RestrictionsText::restriction4, *_rest);
  }
  void testRestriction4_ApplRequired_Blank()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST BE NONSTOP OR DIRECT\n",
        &RestrictionsText::restriction4,
        *_rest);
  }
  void testRestriction4_ApplNotPermitted_Nonstop()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>("TRAVEL BETWEEN XXX AND YYY MUST NOT BE NONSTOP\n",
                                            &RestrictionsText::restriction4,
                                            *_rest);
  }
  void testRestriction4_ApplNotPermitted_Direct()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST NOT BE DIRECT\n", &RestrictionsText::restriction4, *_rest);
  }
  void testRestriction4_ApplNotPermitted_Blank()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction4);
    testResponse<const RoutingRestriction&>(
        "TRAVEL BETWEEN XXX AND YYY MUST NOT BE NONSTOP OR DIRECT\n",
        &RestrictionsText::restriction4,
        *_rest);
  }
  void testRestriction5_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction5);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE VIA ZZZ\n", &RestrictionsText::restriction5, *_rest);
  }
  void testRestriction5_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction5);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction5, *_rest);
  }
  void testRestriction5_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction5);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE VIA ZZZ\n", &RestrictionsText::restriction5, *_rest);
  }
  void testRestriction5_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction5);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST NOT BE VIA ZZZ\n", &RestrictionsText::restriction5, *_rest);
  }
  void testRestriction6_ApplPermitted_Nonstop()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE NONSTOP\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplPermitted_Direct()
  {
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE DIRECT\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplPermitted_Blank()
  {
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE NONSTOP OR DIRECT\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplBlank_Nonstop()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplBlank_Direct()
  {
    _rest->negViaAppl() = BLANK;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplBlank_Blank()
  {
    _rest->negViaAppl() = BLANK;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplRequired_Nonstop()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE NONSTOP\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplRequired_Direct()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE DIRECT\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplRequired_Blank()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST BE NONSTOP OR DIRECT\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplNotPermitted_Nonstop()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST NOT BE NONSTOP\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplNotPermitted_Direct()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->nonStopDirectInd() = DIRECT;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>(
        "TRAVEL TO/FROM XXX MUST NOT BE DIRECT\n", &RestrictionsText::restriction6, *_rest);
  }
  void testRestriction6_ApplNotPermitted_Blank()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->nonStopDirectInd() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction6);
    testResponse<const RoutingRestriction&>("TRAVEL TO/FROM XXX MUST NOT BE NONSTOP OR DIRECT\n",
                                            &RestrictionsText::restriction6,
                                            *_rest);
  }
  void testRestriction7_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction7);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY STOPOVER IN ZZZ PERMITTED\n", &RestrictionsText::restriction7, *_rest);
  }
  void testRestriction7_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction7);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction7, *_rest);
  }
  void testRestriction7_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction7);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY STOPOVER IN ZZZ REQUIRED\n", &RestrictionsText::restriction7, *_rest);
  }
  void testRestriction7_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction7);
    testResponse<const RoutingRestriction&>("BETWEEN XXX AND YYY STOPOVER IN ZZZ NOT PERMITTED\n",
                                            &RestrictionsText::restriction7,
                                            *_rest);
  }
  void testRestriction8_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction8);
    testResponse<const RoutingRestriction&>(
        "STOPOVER IN ZZZ PERMITTED\n", &RestrictionsText::restriction8, *_rest);
  }
  void testRestriction8_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction8);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction8, *_rest);
  }
  void testRestriction8_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction8);
    testResponse<const RoutingRestriction&>(
        "STOPOVER IN ZZZ REQUIRED\n", &RestrictionsText::restriction8, *_rest);
  }
  void testRestriction8_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction8);
    testResponse<const RoutingRestriction&>(
        "STOPOVER IN ZZZ NOT PERMITTED\n", &RestrictionsText::restriction8, *_rest);
  }
  void testRestriction9FMR()
  {
    FareDisplayTrx fareDisplayTrx;
    AirSeg travelSeg;
    Loc loc1, loc2;
    Loc::dummyData(loc1);
    Loc::dummyData(loc2);
    travelSeg.origin() = &loc1;
    travelSeg.destination() = &loc2;
    travelSeg.boardMultiCity() = loc1.loc();
    travelSeg.offMultiCity() = loc2.loc();
    fareDisplayTrx.travelSeg().push_back(&travelSeg);
    testResponse<const FareDisplayTrx&>(
        "TRAVEL IS NOT PERMITTED VIA THE FARE ORIGIN\n", &RestrictionsText::restriction9FMR, fareDisplayTrx);
  }
  void testRestriction9FMR_noRTW()
  {
    FareDisplayTrx fareDisplayTrx;
    AirSeg travelSeg;
    Loc loc1, loc2;
    Loc::dummyData(loc1);
    Loc::dummyData(loc2);
    loc2.loc() = "QWERTYUI";
    travelSeg.origin() = &loc1;
    travelSeg.destination() = &loc2;
    travelSeg.boardMultiCity() = loc1.loc();
    travelSeg.offMultiCity() = loc2.loc();
    fareDisplayTrx.travelSeg().push_back(&travelSeg);
    testResponse<const FareDisplayTrx&>("", &RestrictionsText::restriction9FMR, fareDisplayTrx);
  }

  void testRestriction10_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction10);
    testResponse<const RoutingRestriction&>(
        "CHANGE OF AIRCRAFT IN ZZZ PERMITTED\n", &RestrictionsText::restriction10, *_rest);
  }
  void testRestriction10_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction10);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction10, *_rest);
  }
  void testRestriction10_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction10);
    testResponse<const RoutingRestriction&>(
        "CHANGE OF AIRCRAFT IN ZZZ REQUIRED\n", &RestrictionsText::restriction10, *_rest);
  }
  void testRestriction10_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction10);
    testResponse<const RoutingRestriction&>(
        "CHANGE OF AIRCRAFT IN ZZZ NOT PERMITTED\n", &RestrictionsText::restriction10, *_rest);
  }
  void testRestriction11_ApplPermitted_Air()
  {
    _rest->airSurfaceInd() = AIR;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY AIR SECTOR PERMITTED\n", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplPermitted_Surface()
  {
    _rest->airSurfaceInd() = SURFACE;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY SURFACE SECTOR PERMITTED\n", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplPermitted_Blank()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>("BETWEEN XXX AND YYY AIR OR SURFACE SECTOR PERMITTED\n",
                                            &RestrictionsText::restriction11,
                                            *_rest);
  }
  void testRestriction11_ApplBlank_Air()
  {
    _rest->negViaAppl() = BLANK;
    _rest->airSurfaceInd() = AIR;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplBlank_Surface()
  {
    _rest->negViaAppl() = BLANK;
    _rest->airSurfaceInd() = SURFACE;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplBlank_Blank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplRequired_Air()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->airSurfaceInd() = AIR;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY AIR SECTOR REQUIRED\n", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplRequired_Surface()
  {
    _rest->negViaAppl() = REQUIRED;
    _rest->airSurfaceInd() = SURFACE;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY SURFACE SECTOR REQUIRED\n", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplRequired_Blank()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>("BETWEEN XXX AND YYY AIR OR SURFACE SECTOR REQUIRED\n",
                                            &RestrictionsText::restriction11,
                                            *_rest);
  }
  void testRestriction11_ApplNotPermitted_Air()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->airSurfaceInd() = AIR;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY AIR SECTOR NOT PERMITTED\n", &RestrictionsText::restriction11, *_rest);
  }
  void testRestriction11_ApplNotPermitted_Surface()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _rest->airSurfaceInd() = SURFACE;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>("BETWEEN XXX AND YYY SURFACE SECTOR NOT PERMITTED\n",
                                            &RestrictionsText::restriction11,
                                            *_rest);
  }
  void testRestriction11_ApplNotPermitted_Blank()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction11);
    testResponse<const RoutingRestriction&>(
        "BETWEEN XXX AND YYY AIR OR SURFACE SECTOR NOT PERMITTED\n",
        &RestrictionsText::restriction11,
        *_rest);
  }
  void testRestriction12_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction12);
    testResponse<const RoutingRestriction&>(
        "MAXIMUM PERMITTED MILEAGE TO/FROM XXX\n", &RestrictionsText::restriction12, *_rest);
  }
  void testRestriction12_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction12);
    testResponse<const RoutingRestriction&>(
        "MAXIMUM PERMITTED MILEAGE TO/FROM XXX\n", &RestrictionsText::restriction12, *_rest);
  }
  void testRestriction12_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction12);
    testResponse<const RoutingRestriction&>(
        "MAXIMUM PERMITTED MILEAGE TO/FROM XXX\n", &RestrictionsText::restriction12, *_rest);
  }
  void testRestriction12_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction12);
    testResponse<const RoutingRestriction&>(
        "MAXIMUM PERMITTED MILEAGE TO/FROM XXX\n", &RestrictionsText::restriction12, *_rest);
  }

  void testRestriction16()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction16);
    testResponse<const RoutingRestriction&>(
        "MILEAGE SYSTEM APPLIES ORIGIN TO DESTINATION\n", &RestrictionsText::restriction16, *_rest);
  }
  void testRestriction17_ZZ()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction17);
    testResponse<const RoutingRestriction&>("ZZ ONLY\n", &RestrictionsText::restriction17, *_rest);
  }
  void testRestriction17_Blank()
  {
    _rest->viaCarrier() = "";
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction17);
    testResponse<const RoutingRestriction&>(
        "CARRIER LISTING ONLY\n", &RestrictionsText::restriction17, *_rest);
  }

  void testRestriction21_ApplPermitted()
  {
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction21);
    testResponse<const RoutingRestriction&>(
        "WHEN ORIGIN IS XXX AND DEST IS YYY TRAVEL MUST BE VIA ZZZ\n",
        &RestrictionsText::restriction21,
        *_rest);
  }
  void testRestriction21_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction21);
    testResponse<const RoutingRestriction&>("", &RestrictionsText::restriction21, *_rest);
  }
  void testRestriction21_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction21);
    testResponse<const RoutingRestriction&>(
        "WHEN ORIGIN IS XXX AND DEST IS YYY TRAVEL MUST BE VIA ZZZ\n",
        &RestrictionsText::restriction21,
        *_rest);
  }
  void testRestriction21_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funPtr = static_cast<void (RestrictionsText::*)(const RoutingRestriction&, const bool&)>(
        &RestrictionsText::restriction21);
    testResponse<const RoutingRestriction&>(
        "WHEN ORIGIN IS XXX AND DEST IS YYY TRAVEL MUST NOT BE\n"
        "       VIA ZZZ\n",
        &RestrictionsText::restriction21,
        *_rest);
  }
  ////////////////////////////////////////
  ////////////////////////////////////////
  void testRestriction1It_ApplPermitted()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction1);
    testItResponse("RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n");
  }
  void testRestriction1It_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction1);
    testItResponse("RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n");
  }
  void testRestriction1It_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction1);
    testItResponse("TRAVEL BETWEEN XXX AND YYY MUST BE VIA ZZZ OR AAA OR BBB\n"
                   "       OR CCC OR DDD OR EEE\n");
  }
  void testRestriction1It_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction1);
    testItResponse("TRAVEL BETWEEN XXX AND YYY MUST NOT BE VIA ZZZ OR AAA OR\n"
                   "       BBB OR CCC OR DDD OR EEE\n");
  }
  void testRestriction2It_ApplPermitted()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction2);
    testItResponse("TRAVEL MUST BE VIA ZZZ OR AAA OR BBB OR CCC OR DDD OR EEE\n");
  }
  void testRestriction2It_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction2);
    testItResponse("TRAVEL MUST NOT BE VIA ZZZ OR AAA OR BBB OR CCC OR DDD\n"
                   "       OR EEE\n");
  }
  void testRestriction2It_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction2);
    testItResponse("TRAVEL MUST BE VIA ZZZ OR AAA OR BBB OR CCC OR DDD OR EEE\n");
  }
  void testRestriction2It_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction2);
    testItResponse("TRAVEL MUST NOT BE VIA ZZZ OR AAA OR BBB OR CCC OR DDD\n"
                   "       OR EEE\n");
  }
  void testRestriction5It_ApplPermitted()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction5);
    testItResponse("TRAVEL TO/FROM XXX MUST BE VIA ZZZ OR AAA OR BBB OR CCC\n"
                   "       OR DDD OR EEE\n");
  }
  void testRestriction5It_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction5);
    testItResponse("");
  }
  void testRestriction5It_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction5);
    testItResponse("TRAVEL TO/FROM XXX MUST BE VIA ZZZ OR AAA OR BBB OR CCC\n"
                   "       OR DDD OR EEE\n");
  }
  void testRestriction5It_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction5);
    testItResponse("TRAVEL TO/FROM XXX MUST NOT BE VIA ZZZ OR AAA OR BBB OR\n"
                   "       CCC OR DDD OR EEE\n");
  }
  void testRestriction7It_ApplPermitted()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction7);
    testItResponse("BETWEEN XXX AND YYY STOPOVER IN ZZZ OR AAA OR BBB OR CCC\n"
                   "       OR DDD OR EEE PERMITTED\n");
  }
  void testRestriction7It_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction7);
    testItResponse("");
  }
  void testRestriction7It_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction7);
    testItResponse("BETWEEN XXX AND YYY STOPOVER IN ZZZ OR AAA OR BBB OR CCC\n"
                   "       OR DDD OR EEE REQUIRED\n");
  }
  void testRestriction7It_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction7);
    testItResponse("BETWEEN XXX AND YYY STOPOVER IN ZZZ OR AAA OR BBB OR CCC\n"
                   "       OR DDD OR EEE NOT PERMITTED\n");
  }

  void testRestriction10It_ApplPermitted()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction10);
    testItResponse("CHANGE OF AIRCRAFT IN ZZZ OR AAA OR BBB OR CCC OR DDD OR\n"
                   "       EEE PERMITTED\n");
  }
  void testRestriction10It_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction10);
    testItResponse("");
  }
  void testRestriction10It_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction10);
    testItResponse("CHANGE OF AIRCRAFT IN ZZZ OR AAA OR BBB OR CCC OR DDD OR\n"
                   "       EEE REQUIRED\n");
  }
  void testRestriction10It_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction10);
    testItResponse("CHANGE OF AIRCRAFT IN ZZZ OR AAA OR BBB OR CCC OR DDD OR\n"
                   "       EEE NOT PERMITTED\n");
  }
  void testRestriction17It_ZZ()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction17);
    testItResponse("ZZ OR AA OR BB OR CC OR DD OR EE ONLY\n");
  }
  void testRestriction17It_Blank()
  {
    _rest->viaCarrier() = "";
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction17);
    testItResponse("CARRIER LISTING ONLY\n");
  }

  void testRestriction21It_ApplPermitted()
  {
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction21);
    testItResponse("WHEN ORIGIN IS XXX AND DEST IS YYY TRAVEL MUST BE VIA\n"
                   "       ZZZ OR AAA OR BBB OR CCC OR DDD OR EEE\n");
  }
  void testRestriction21It_ApplBlank()
  {
    _rest->negViaAppl() = BLANK;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction21);
    testItResponse("");
  }
  void testRestriction21It_ApplRequired()
  {
    _rest->negViaAppl() = REQUIRED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction21);
    testItResponse("WHEN ORIGIN IS XXX AND DEST IS YYY TRAVEL MUST BE VIA\n"
                   "       ZZZ OR AAA OR BBB OR CCC OR DDD OR EEE\n");
  }
  void testRestriction21It_ApplNotPermitted()
  {
    _rest->negViaAppl() = NOT_PERMITTED;
    _funItPtr = static_cast<void (RestrictionsText::*)(RtgRestCI, RtgRestCI, const bool&)>(
        &RestrictionsText::restriction21);
    testItResponse("WHEN ORIGIN IS XXX AND DEST IS YYY TRAVEL MUST NOT BE\n"
                   "       VIA ZZZ OR AAA OR BBB OR CCC OR DDD OR EEE\n");
  }

private:
  RestrictionsText* _res;
  TestMemHandle _memHandle;
  RoutingRestriction* _rest;
  std::vector<RoutingRestriction*>::iterator _restItI, _restItE;
  boost::function<void(RestrictionsText*, const RoutingRestriction&, const bool&)> _funPtr;
  boost::function<void(RestrictionsText*, RtgRestCI, RtgRestCI, const bool&)> _funItPtr;
  std::ostringstream* _stream;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RestrictionsTextTest);
}
