#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#define private public
#define protected public

#include "Common/FcConfig.h"
#include "DataModel/FarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/NoPNROptions.h"
#include "FareCalc/NoPNRFareCalcCollector.h"
#include "FareCalc/NoPNRFareCalcController.h"
#include "FareCalc/NoPNRFareCalculation.h"
#include "FareCalc/NoPNRFcConfigWrapper.h"

#undef private
#undef protected

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"

using namespace std;

namespace tse
{
namespace
{
std::string ruler = "0_________1_________2_________3_________4_________5_________6___4";

const char* applyTksDsgLenTestData[] = {
  /* FAREBASISTKTDESLNG value, fare basis, expected fare basis after applying */
  "1", "ABCDEFGH",                  "ABCDEFGH",
  "2", "ABCDEFGH",                  "ABCDEFGH",
  "3", "ABCDEFGH",                  "ABCDEFGH",
  " ", "ABCDEFGH",                  "ABCDEFGH",
  "1", "ABCDEFGH/ABCD",             "ABCDEFGH/ABCD",
  "2", "ABCDEFGH/ABCD",             "ABCDEFGH/ABCD",
  "3", "ABCDEFGH/ABCD",             "ABCDEFGH/ABCD",
  " ", "ABCDEFGH/ABCD",             "ABCDEFGH/ABCD",
  "1", "ABCDEFGHIJKL",              "ABCDEFGH",
  "2", "ABCDEFGHIJKL",              "ABCDEFGHI",
  "3", "ABCDEFGHIJKL",              "ABCDEFGHIJ",
  " ", "ABCDEFGHIJKL",              "ABCDEFGHIJKL",
  "1", "ABCDEFGHIJKL/ABCDEFGHIJKL", "ABCDEFGH/ABCDEF",
  "2", "ABCDEFGHIJKL/ABCDEFGHIJKL", "ABCDEFGHI/ABCDE",
  "3", "ABCDEFGHIJKL/ABCDEFGHIJKL", "ABCDEFGHIJ/ABCD",
  " ", "ABCDEFGHIJKL/ABCDEFGHIJKL", "ABCDEFGHIJKL/AB"
};

namespace
{
const std::string strWqPrefix = "ATTN*";
const std::string strWqXm = "SEE OTHER FARES - USE XM QUALIFIER, E.G. WQCTY/ACRCTY-XM\n";
const std::string strWqAl = "SEE ALL FARES - USE AL QUALIFIER, E.G. WQCTY/ACRCTY-AL\n";
}
}

class NoPNRFareCalculationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NoPNRFareCalculationTest);

  CPPUNIT_TEST(test_displayIndicesVector);
  CPPUNIT_TEST(test_display_pax_line);
  CPPUNIT_TEST(test_nofaresforpaxtype);
  CPPUNIT_TEST(test_displayFareLineInfo);
  CPPUNIT_TEST(test_checkDetailFormat);
  CPPUNIT_TEST(test_displayNonCOCCurrencyIndicator);
  CPPUNIT_TEST(test_applyTktDsgLen);
  CPPUNIT_TEST(test_the_test);
  CPPUNIT_TEST(test_getPSSBookingCodesLine);
  CPPUNIT_TEST(test_getApplicableBookingCodesLine);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_MatchOnly_trailer1);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_MatchOnly_trailer2);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_MatchOnly_trailer3);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_MatchNoMatch_trailer1);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_MatchNoMatch_trailer2);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_MatchNoMatch_trailer3);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQ_NoMatchOnly);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQcXM);
  CPPUNIT_TEST(test_displayWqTrailerMessage_WQcAL);

  CPPUNIT_TEST_SUITE_END();

  typedef FareCalcCollector::CalcTotalsMap CalcTotalsMap;

public:
  void setUp()
  {
    adt = _memHandle.create<PaxType>();
    adt->paxType() = "ADT";
    cnn = _memHandle.create<PaxType>();
    cnn->paxType() = "CNN";
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  class NoPNRFareCalculation_mockForTests : public NoPNRFareCalculation
  {
  public:
    std::vector<std::string> bcodesToReturn;

  protected:
    virtual void
    collectBookingCodes(std::vector<std::string>& appplicableBookingClasses, CalcTotals& calcTotals)
    {
      appplicableBookingClasses.resize(bcodesToReturn.size());
      for (unsigned i = 0; i < bcodesToReturn.size(); ++i)
      {
        appplicableBookingClasses.at(i) = bcodesToReturn.at(i);
      }
    }
  };

  void test_getPSSBookingCodesLine()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation_mockForTests* calc = getMockCalculation(trx);
    // CalcTotals should not be needed when 'collectBookingCodes' is overloaded
    CalcTotals shouldBeUnimportant;

    std::vector<std::string>& bcodes = calc->bcodesToReturn;
    bcodes.resize(5);
    bcodes[0] = "A";
    bcodes[1] = "BC";
    bcodes[2] = "DE";
    bcodes[3] = "F";
    bcodes[4] = "GH";

    std::string obtained = calc->getPSSBookingCodesLine(shouldBeUnimportant);
    CPPUNIT_ASSERT_MESSAGE(std::string("response obtained: '") + obtained + "'",
                           obtained == "[A/BC/DE/F/GH]\n");
    bcodes.resize(1);
    obtained = calc->getPSSBookingCodesLine(shouldBeUnimportant);
    CPPUNIT_ASSERT_MESSAGE(std::string("response obtained: '") + obtained + "'",
                           obtained == "[A]\n");
  }

  void test_getApplicableBookingCodesLine()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation_mockForTests* calc = getMockCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    calc->_noPNRConfigOptions = npnro;
    // CalcTotals should not be needed when 'collectBookingCodes' is overloaded
    CalcTotals calcTotals;
    FarePath* fp = _memHandle.create<FarePath>();
    fp->_noMatchOption = true;
    fp->_itin = _memHandle.create<Itin>();
    const char* itinBookingCodes[] = { "H", "X", "KL", "X", "X" };

    for (int i = 0; i < 5; ++i)
    {
      TravelSeg* ts = _memHandle.create<AirSeg>();
      ts->_segmentType = Open;
      ts->_bookingCode = itinBookingCodes[i];
      fp->_itin->travelSeg().push_back(ts);
    }

    calcTotals.farePath = fp;

    std::vector<std::string>& bcodes = calc->bcodesToReturn;
    bcodes.resize(5);
    bcodes[0] = "H";
    bcodes[1] = "IJ";
    bcodes[2] = "KL";
    bcodes[3] = "M";
    bcodes[4] = "NO";

    std::string obtained = calc->getApplicableBookingCodesLine(calcTotals);

    std::string expected = "APPLICABLE BOOKING CLASS - 2IJ 4M 5NO";
    CPPUNIT_ASSERT_MESSAGE(std::string("response obtained: \n'") + obtained + "'\n" +
                               ", expected - \n'" + expected + "'\n",
                           obtained.find(expected) != std::string::npos);
  }

  void test_applyTktDsgLen()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getMockCalculation(trx);

    for (unsigned i = 0; i < sizeof(applyTksDsgLenTestData) / sizeof(applyTksDsgLenTestData[0]);
         i += 3)
    {
      std::string testBasis = applyTksDsgLenTestData[i + 1];
      calc->applyTksDsgLen(applyTksDsgLenTestData[i][0], testBasis);
      std::ostringstream failMsg;
      failMsg << "failed case: " << (i / 3) << " for " << applyTksDsgLenTestData[i + 1]
              << " with <FAREBASISTKTDESLNG> = '" << applyTksDsgLenTestData[i] << "'";
      CPPUNIT_ASSERT_MESSAGE(failMsg.str(), testBasis == applyTksDsgLenTestData[i + 2]);
    }
  }

  void test_displayNonCOCCurrencyIndicator()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();

    trx->itin().front()->originationCurrency() = "PDC"; // polish ducat

    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    calc->_noPNRConfigOptions = npnro;
    // calc->_originationCurrency = "USD" ;

    FareType someFareType = "ABC";
    CalcTotals* ct1 = getCalcTotals(newPathForPaxType(adt), someFareType);

    ct1->convertedBaseFareCurrencyCode = "USD";
    ct1->equivCurrencyCode = "PLN";

    npnro->displayNonCOCCurrencyInd() = 'N';

    calc->_fareCalcDisp.clear();
    calc->displayNonCOCCurrencyIndicator(*ct1);
    CPPUNIT_ASSERT(std::string(" ") == calc->_fareCalcDisp.str());
  }

  void test_checkDetailFormat()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    trx->paxType().clear();
    trx->paxType().push_back(adt);
    trx->paxType().push_back(cnn);

    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    calc->_noPNRConfigOptions = npnro;

    CalcTotalsMap& cmp = (calc->_fcCollector->_calcTotalsMap);

    calc->_secondaryResp = true;
    calc->_primaryResp = false;

    CPPUNIT_ASSERT(calc->checkDetailFormat());
    calc->calcTotalsObtained.clear();

    calc->_secondaryResp = false;
    calc->_primaryResp = true;

    CPPUNIT_ASSERT(!calc->checkDetailFormat());
    calc->calcTotalsObtained.clear();

    FareType someFareType = "ABC";

    CalcTotals* ct1 = getCalcTotals(newPathForPaxType(adt, true, false), someFareType);
    CalcTotals* ct2 = getCalcTotals(newPathForPaxType(cnn, true, false), someFareType);

    // insert only 1 match fare for both pax types
    cmp[ct1->farePath] = ct1;
    cmp[ct2->farePath] = ct2;
    npnro->_passengerDetailLineFormat = '2'; // 'never display Passenger Detail Format'

    CPPUNIT_ASSERT(!calc->checkDetailFormat());
  }

  void test_displayIndicesVector()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    std::vector<int> vec;

    vec.push_back(1);
    vec.push_back(5);
    vec.push_back(6);
    vec.push_back(7);
    vec.push_back(8);
    vec.push_back(11);

    calc->_fareCalcDisp.clear();
    calc->displayIndicesVector("", vec, "", false, calc->_fareCalcDisp, true);
    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "01,05-08,11\n");

    vec.clear();
    calc->_fareCalcDisp.clear();

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(5);
    vec.push_back(6);
    vec.push_back(8);
    vec.push_back(10);
    vec.push_back(11);
    vec.push_back(12);

    calc->displayIndicesVector("", vec, "", false, calc->_fareCalcDisp, false);
    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "01-03,05,06,08,10-12");

    vec.clear();
    calc->_fareCalcDisp.clear();
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(8);
    vec.push_back(11);
    vec.push_back(3);
    vec.push_back(10);
    vec.push_back(5);
    vec.push_back(6);
    vec.push_back(12);

    std::random_shuffle(vec.begin(), vec.end());

    calc->displayIndicesVector("", vec, "", false, calc->_fareCalcDisp, false);
    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "01-03,05,06,08,10-12");

    vec.clear();
    calc->_fareCalcDisp.clear();
    calc->displayIndicesVector("", vec, "", false, calc->_fareCalcDisp, true);
    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "  \n");

    vec.clear();
    vec.push_back(1);
    calc->_fareCalcDisp.clear();
    calc->displayIndicesVector("", vec, "", false, calc->_fareCalcDisp, false);
    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "01");

    srand(time(NULL));
    for (int i = 0; i < 40; i++)
    {
      int r = 1;
      while (std::find(vec.begin(), vec.end(), r) != vec.end())
        r = rand() % 99;
      vec.push_back(r);
    }

    calc->_fareCalcDisp.clear();
    calc->displayIndicesVector("", vec, "", false, calc->_fareCalcDisp, false);

    CPPUNIT_ASSERT(noNumberBroken(calc->_fareCalcDisp.str()));
  }

  bool noNumberBroken(const std::string& str)
  {
    // iterate through endlines,and make sure that none of them breaks any number
    std::string::size_type endline = 0;
    while ((endline = str.find('\n', endline)) != std::string::npos)
    {
      if (endline > 0 && endline < (str.size() - 1))
      {
        if (::isdigit(str[endline - 1]) && ::isdigit(str[endline + 1]))
          return false;
      }
      ++endline;
    }
    return true;
  }

  void test_display_pax_line()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = _memHandle.create<NoPNROptions>();
    calc->_noPNRConfigOptions = npnro;
    CalcTotals& ct = *getAllNewCalcTotals();

#define TEST_IT(FLPTC, PTCREFNO, EXPECTED)                                                         \
  npnro->_fareLinePTC = FLPTC;                                                                     \
  npnro->_primePTCRefNo = PTCREFNO;                                                                \
  calc->_fareCalcDisp.clear();                                                                     \
  calc->displayPsgrInfo(ct, false);                                                                \
  CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == EXPECTED);

    TEST_IT('1', 'Y', "PSGR TYPE  CNN - 01\n");
    TEST_IT('2', 'Y', "INPUT PSGR TYPE  CNN - 01\n");
    TEST_IT('3', 'Y', "1ST PSGR TYPE  CNN - 01\n");
    TEST_IT('1', 'N', "PSGR TYPE  CNN\n");
    TEST_IT('2', 'N', "INPUT PSGR TYPE  CNN\n");
    TEST_IT('3', 'N', "1ST PSGR TYPE  CNN\n");

#undef TEST_IT
  }

  void test_nofaresforpaxtype()
  {
    NoPNRPricingTrx* trx;
    NoPNRFareCalculation* calc;
    FarePath* fpath;
    NoPNROptions* npnro;
    CalcTotals* ct;

    calc = getCalculationDfwLax(trx, npnro, fpath, ct);
    calc->_fcCollector->_calcTotalsMap[ct->farePath] = ct;

    std::set<PaxType*, PaxType::InputOrder> ptset;
    ptset.insert(ct->farePath->_paxType);
    std::set<PaxType*, PaxType::InputOrder>::const_iterator pti = ptset.begin();

    fpath->_processed = false;

    int psgCount = 0;

    NoPNRFcConfigWrapper fcConfWrapper(
        calc->_fcConfig, (&(calc->_fcConfig)), calc->_noPNRConfigOptions);

    npnro->noMatchNoFaresErrorMsg() = '1';
    calc->processPrimaryFormatSinglePaxType(*pti, psgCount, true);

    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "PSGR TYPE  CNN\n*NO FARES FOR CLASS USED\n");

    npnro->noMatchNoFaresErrorMsg() = '2';
    calc->_fareCalcDisp.clear();
    calc->processPrimaryFormatSinglePaxType(*pti, psgCount, true);
    CPPUNIT_ASSERT(calc->_fareCalcDisp.str() == "PSGR TYPE  CNN\n*NO FARES/RBD/CARRIER\n");
  }

  void test_displayFareLineInfo()
  {
    // need: CalcTotals,
    // calcTotals.convertedBaseFare;,convertedBaseFareNoDec,convertedBaseFareCurrencyCode
    // calcTotals.wpaInfo.psgDetailRefNo
    // getPrivateIndicator -> ?
    // calcTotals.farePath -> musi iterowac po fareusage (FareUsageIter)
    // getBookingCode -> farePath->itin()->travelSeg().begin()  airSeg->bookingCode()
  }

  void test_the_test()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    CPPUNIT_ASSERT(trx != NULL);

    // test noNumberBroken
    std::string teststr1 = "sgims mdsrg 3\n5 wet";
    CPPUNIT_ASSERT(!noNumberBroken(teststr1));
    std::string teststr2 = "\nsgims mdsrg 3\n";
    CPPUNIT_ASSERT(noNumberBroken(teststr2));
    std::string teststr3 = "sgims mdsrg 3\n5";
    CPPUNIT_ASSERT(!noNumberBroken(teststr3));
  }

  void test_displayWqTrailerMessage_WQ_MatchOnly_trailer1()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '1';
    npnro->matchIntegratedTrailer() = '1';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWqPrefix + strWqXm + strWqPrefix + strWqAl, calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQ_MatchOnly_trailer2()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '2';
    npnro->matchIntegratedTrailer() = '2';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWqXm + strWqAl, calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQ_MatchOnly_trailer3()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '3';
    npnro->matchIntegratedTrailer() = '3';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQ_MatchNoMatch_trailer1()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    calc->_optionsToRebook.push_back(1);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '1';
    npnro->matchIntegratedTrailer() = '1';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWqPrefix + strWqXm + strWqPrefix + strWqAl, calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQ_MatchNoMatch_trailer2()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    calc->_optionsToRebook.push_back(1);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '2';
    npnro->matchIntegratedTrailer() = '2';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(strWqXm + strWqAl, calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQ_MatchNoMatch_trailer3()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    calc->_optionsToRebook.push_back(1);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '3';
    npnro->matchIntegratedTrailer() = '3';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQ_NoMatchOnly()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    calc->_allRequireRebook = true;
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '2';
    npnro->matchIntegratedTrailer() = '2';
    calc->_noPNRConfigOptions = npnro;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQcXM()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '2';
    npnro->matchIntegratedTrailer() = '2';
    calc->_noPNRConfigOptions = npnro;

    NoPNRPricingOptions* nppopt = trx->getOptions();
    if (nppopt)
      nppopt->noMatch() = 'Y';
    else
      std::cout << "AAAAAAAAAAAAA" << std::endl;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), calc->_fareCalcDisp.str());
  }

  void test_displayWqTrailerMessage_WQcAL()
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    NoPNRFareCalculation* calc = getCalculation(trx);
    NoPNROptions* npnro = getTestOptions();
    npnro->allMatchTrailerMessage() = '2';
    npnro->matchIntegratedTrailer() = '2';
    calc->_noPNRConfigOptions = npnro;

    NoPNRPricingOptions* nppopt = trx->getOptions();
    if (nppopt)
      nppopt->setMatchAndNoMatchRequested(true);
    else
      std::cout << "BBBBBBBBBBB" << std::endl;

    calc->displayWqTrailerMessage();
    CPPUNIT_ASSERT_EQUAL(std::string(""), calc->_fareCalcDisp.str());
  }

protected:
  TestMemHandle _memHandle;
  PaxType* adt;
  PaxType* cnn;

  FarePath* newPathForPaxType(PaxType* pax, bool processed = true, bool nomatch = false)
  {
    FarePath* fp = _memHandle.create<FarePath>();
    fp->_paxType = pax;
    fp->processed() = processed;
    fp->noMatchOption() = nomatch;
    return fp;
  }

  NoPNRFareCalculation* getCalculationDfwLax(NoPNRPricingTrx*& _trx,
                                             NoPNROptions*& _npnropt,
                                             FarePath*& _fpath,
                                             CalcTotals*& _ct)
  {
    NoPNRPricingTrx* trx = createTrx_DFWLAX();
    _trx = trx;
    NoPNRFareCalculation* calc = getCalculation(trx);
    calc->_fareCalcDisp.clear();
    NoPNROptions* npnropt = _memHandle.create<NoPNROptions>();
    npnropt->maxNoOptions() = 17;
    calc->_noPNRConfigOptions = npnropt;
    _npnropt = npnropt;
    CalcTotals& ct = *getAllNewCalcTotals();
    _ct = &ct;

    FarePath* fp = const_cast<FarePath*>(ct.farePath);
    fp->itin() = trx->itin().front();
    _fpath = fp;

    calc->_fcCollector->_calcTotalsMap[ct.farePath] = &ct;

    return calc;
  }

  CalcTotals* getCalcTotals(FarePath* fp, FareType ft)
  {
    CalcTotals* ct = _memHandle.create<CalcTotals>();
    ct->farePath = fp;
    ct->requestedPaxType = fp->_paxType->paxType();

    // fill calcTotals->pricingUnits with one PaxTypeFare with given FareType
    // std::map<const FareUsage*, const PricingUnit*> pricingUnits
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* fu = _memHandle.create<FareUsage>();

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    FareClassAppInfo* fcasi = _memHandle.create<FareClassAppInfo>();
    fcasi->_fareType = ft;
    ptf->_fareClassAppInfo = fcasi;

    fu->paxTypeFare() = ptf;
    ct->pricingUnits[fu] = pu;

    return ct;
  }

  CalcTotals* getAllNewCalcTotals()
  {
    FarePath* fpath = _memHandle.create<FarePath>();
    PaxType* ptype = _memHandle.create<PaxType>();
    CalcTotals* ct = _memHandle.create<CalcTotals>();
    ct->requestedPaxType = "CNN";
    ptype->paxType() = "ADT";
    fpath->_paxType = ptype;
    ct->farePath = fpath;
    return ct;
  }

  NoPNRFareCalculation_mockForTests* getMockCalculation(NoPNRPricingTrx* trx)
  {
    NoPNRFareCalculation_mockForTests* noPnrfareCalculation =
        _memHandle.create<NoPNRFareCalculation_mockForTests>();

    std::vector<Itin*>::iterator itinI = trx->itin().begin();
    NoPNRFareCalcController controller(*trx);
    NoPNRFareCalcCollector* collector =
        dynamic_cast<NoPNRFareCalcCollector*>(controller.getFareCalcCollector(*itinI));

    if (noPnrfareCalculation != 0)
    {
      // noPnrfareCalculation->initialize(trx, fcConfig, collector);

      noPnrfareCalculation->_trx = trx;
      noPnrfareCalculation->_fcConfig = _memHandle.create<FcConfig>();

      noPnrfareCalculation->_fcCollector = collector;

      noPnrfareCalculation->_needXTLine = false;
      noPnrfareCalculation->_dispSegmentFeeMsg = false;
      noPnrfareCalculation->_totalBaseAmount = 0;
      noPnrfareCalculation->_xtAmount = 0;
      noPnrfareCalculation->_totalEquivAmount = 0;
      noPnrfareCalculation->_totalFareAmount = 0;
      noPnrfareCalculation->_tempWorkAmount = 0;
      noPnrfareCalculation->_nbrDec = 0;
      noPnrfareCalculation->_equivNoDec = 0;
      noPnrfareCalculation->_fareNoDec = 0;
      noPnrfareCalculation->_psgrCount = 0;
      noPnrfareCalculation->_fareAmountLen = 0;

      noPnrfareCalculation->_warning = "";

      noPnrfareCalculation->_needNetRemitCalc = false;
    }
    return noPnrfareCalculation;
  }

  NoPNRFareCalculation* getCalculation(NoPNRPricingTrx* trx)
  {
    NoPNRFareCalculation* noPnrfareCalculation = trx->dataHandle().create<NoPNRFareCalculation>();

    std::vector<Itin*>::iterator itinI = trx->itin().begin();
    NoPNRFareCalcController controller(*trx);
    NoPNRFareCalcCollector* collector =
        dynamic_cast<NoPNRFareCalcCollector*>(controller.getFareCalcCollector(*itinI));

    if (noPnrfareCalculation != 0)
    {
      noPnrfareCalculation->_trx = trx;
      noPnrfareCalculation->_fcConfig = _memHandle.create<FcConfig>();

      noPnrfareCalculation->_fcCollector = collector;

      noPnrfareCalculation->_needXTLine = false;
      noPnrfareCalculation->_dispSegmentFeeMsg = false;
      noPnrfareCalculation->_totalBaseAmount = 0;
      noPnrfareCalculation->_xtAmount = 0;
      noPnrfareCalculation->_totalEquivAmount = 0;
      noPnrfareCalculation->_totalFareAmount = 0;
      noPnrfareCalculation->_tempWorkAmount = 0;
      noPnrfareCalculation->_nbrDec = 0;
      noPnrfareCalculation->_equivNoDec = 0;
      noPnrfareCalculation->_fareNoDec = 0;
      noPnrfareCalculation->_psgrCount = 0;
      noPnrfareCalculation->_fareAmountLen = 0;

      noPnrfareCalculation->_warning = "";

      noPnrfareCalculation->_needNetRemitCalc = false;

      noPnrfareCalculation->_allRequireRebook = false;
      noPnrfareCalculation->_optionsToRebook.clear();
    }
    return noPnrfareCalculation;
  }

  NoPNRPricingTrx* createTrx_DFWLAX()
  {
    NoPNRPricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();

    Agent* agent = _memHandle.create<Agent>();
    agent->currencyCodeAgent() = "USD";

    PricingRequest* request = _memHandle.create<PricingRequest>();
    trx->setRequest(request);

    request->ticketingAgent() = agent;
    DateTime dt;

    request->ticketingDT() = dt.localTime();

    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    request->ticketingAgent()->agentLocation() = loc;

    NoPNRPricingOptions* options = _memHandle.create<NoPNRPricingOptions>();
    trx->setOptions(options);

    options->mOverride() = 'Y';
    options->noMatch() = 'N';
    options->setMatchAndNoMatchRequested(false);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");

    airSeg->departureDT() = dt.localTime();
    airSeg->arrivalDT() = dt.localTime();
    airSeg->bookingDT() = dt.localTime();

    airSeg->setBookingCode(BookingCode('Y'));

    Itin* itin = _memHandle.create<Itin>();
    itin->originationCurrency() = "USD";
    itin->calculationCurrency() = "USD";

    trx->itin().push_back(itin);

    FarePath* farePath = _memHandle.create<FarePath>();

    farePath->setTotalNUCAmount(500.00);
    farePath->itin() = itin;
    farePath->itin()->travelSeg().push_back(airSeg);
    itin->farePath().push_back(farePath);

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg().push_back(airSeg);

    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = std::string("ADT");
    PaxTypeInfo* pti = _memHandle.create<PaxTypeInfo>();
    paxType->paxTypeInfo() = pti;
    itin->farePath()[0]->paxType() = paxType;

    trx->paxType().push_back(paxType);
    trx->travelSeg().push_back(airSeg);
    trx->fareMarket().push_back(fareMarket);
    trx->getRequest()->ticketEntry() = true;
    trx->getOptions()->ticketStock() = 206;

    return trx;
  }

  NoPNROptions* getTestOptions()
  {
    // return some mock data
    NoPNROptions* opt = _memHandle.create<NoPNROptions>();

    opt->userApplType() = 'C';
    opt->userAppl() = "RIBA";
    opt->loc1().locType() = 'N';
    opt->loc1().loc() = "PL";
    opt->wqNotPermitted() = 'A';
    opt->maxNoOptions() = 17;
    opt->wqSort() = 'B';
    opt->wqDuplicateAmounts() = 'C';
    opt->fareLineHeaderFormat() = 'D';
    opt->passengerDetailLineFormat() = 'E';
    opt->fareLinePTC() = 'F';
    opt->primePTCRefNo() = 'G';
    opt->secondaryPTCRefNo() = 'H';
    opt->fareLinePTCBreak() = 'I';
    opt->passengerDetailPTCBreak() = 'J';
    opt->negPassengerTypeMapping() = 'K';
    opt->noMatchOptionsDisplay() = 'L';
    opt->allMatchTrailerMessage() = 'M';
    opt->matchIntegratedTrailer() = 'N';
    opt->accompaniedTvlTrailerMsg() = 'O';
    opt->rbdMatchTrailerMsg() = 'P';
    opt->rbdNoMatchTrailerMsg() = 'Q';
    opt->rbdNoMatchTrailerMsg2() = 'R';
    opt->displayFareRuleWarningMsg() = 'S';
    opt->displayFareRuleWarningMsg2() = 'T';
    opt->displayFinalWarningMsg() = 'U';
    opt->displayFinalWarningMsg2() = 'V';
    opt->displayTaxWarningMsg() = 'W';
    opt->displayTaxWarningMsg2() = 'Y';
    opt->displayPrivateFareInd() = 'X';
    opt->displayNonCOCCurrencyInd() = 'Z';
    opt->displayTruePTCInFareLine() = '1';
    opt->applyROInFareDisplay() = '2';
    opt->alwaysMapToADTPsgrType() = '3';
    opt->noMatchRBDMessage() = '4';
    opt->noMatchNoFaresErrorMsg() = '5';
    opt->totalNoOptions() = 15;

    NoPNROptionsSeg* seg1 = new NoPNROptionsSeg; // don't add to memhandle
    NoPNROptionsSeg* seg2 = new NoPNROptionsSeg;
    NoPNROptionsSeg* seg3 = new NoPNROptionsSeg;

    seg1->seqNo() = 0;
    seg1->noDisplayOptions() = 4;
    seg1->fareTypeGroup() = 6;

    seg2->seqNo() = 33;
    seg2->noDisplayOptions() = 5;
    seg2->fareTypeGroup() = 2;

    seg3->seqNo() = 92;
    seg3->noDisplayOptions() = 6;
    seg3->fareTypeGroup() = 3;

    opt->segs().push_back(seg1);
    opt->segs().push_back(seg2);
    opt->segs().push_back(seg3);

    return opt;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(NoPNRFareCalculationTest);

} // namespace tse
