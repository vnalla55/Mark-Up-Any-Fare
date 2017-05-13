#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "Diagnostic/Diag452Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/DCFactory.h"

using namespace std;

namespace tse
{
class DCFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DCFactoryTest);
  CPPUNIT_TEST(testThreadCreate_Diagnostic85);
  CPPUNIT_TEST(testThreadCreate_Diagnostic191);
  CPPUNIT_TEST(testThreadCreate_Diagnostic194);
  CPPUNIT_TEST(testThreadCreate_Diagnostic198);
  CPPUNIT_TEST(testThreadCreate_Diagnostic199);
  CPPUNIT_TEST(testThreadCreate_AllFareDiagnostic);
  CPPUNIT_TEST(testThreadCreate_Diagnostic201);
  CPPUNIT_TEST(testThreadCreate_Diagnostic607);
  CPPUNIT_TEST(testThreadCreate_Diagnostic204);
  CPPUNIT_TEST(testThreadCreate_Diagnostic208);
  CPPUNIT_TEST(testThreadCreate_Diagnostic212);
  CPPUNIT_TEST(testThreadCreate_Diagnostic220);
  CPPUNIT_TEST(testThreadCreate_Diagnostic223);
  CPPUNIT_TEST(testThreadCreate_Diagnostic225);
  CPPUNIT_TEST(testThreadCreate_Diagnostic231);
  CPPUNIT_TEST(testThreadCreate_Diagnostic233);
  CPPUNIT_TEST(testThreadCreate_Diagnostic251);
  CPPUNIT_TEST(testThreadCreate_Diagnostic252);
  CPPUNIT_TEST(testThreadCreate_Diagnostic253);
  CPPUNIT_TEST(testThreadCreate_Diagnostic254);
  CPPUNIT_TEST(testThreadCreate_Diagnostic255);
  CPPUNIT_TEST(testThreadCreate_Diagnostic256);
  CPPUNIT_TEST(testThreadCreate_Diagnostic257);
  CPPUNIT_TEST(testThreadCreate_Diagnostic258);
  CPPUNIT_TEST(testThreadCreate_Diagnostic259);
  CPPUNIT_TEST(testThreadCreate_Diagnostic270);
  CPPUNIT_TEST(testThreadCreate_Diagnostic302);
  CPPUNIT_TEST(testThreadCreate_Diagnostic304);
  CPPUNIT_TEST(testThreadCreate_Diagnostic311);
  CPPUNIT_TEST(testThreadCreate_Diagnostic312);
  CPPUNIT_TEST(testThreadCreate_Diagnostic315);
  CPPUNIT_TEST(testThreadCreate_Diagnostic316);
  CPPUNIT_TEST(testThreadCreate_Diagnostic323);
  CPPUNIT_TEST(testThreadCreate_Diagnostic325);
  CPPUNIT_TEST(testThreadCreate_Diagnostic327);
  CPPUNIT_TEST(testThreadCreate_Diagnostic335);
  CPPUNIT_TEST(testThreadCreate_Diagnostic372);
  CPPUNIT_TEST(testThreadCreate_Diagnostic400);
  CPPUNIT_TEST(testThreadCreate_Diagnostic404);
  CPPUNIT_TEST(testThreadCreate_Diagnostic405);
  CPPUNIT_TEST(testThreadCreate_Diagnostic411);
  CPPUNIT_TEST(testThreadCreate_Diagnostic413);
  CPPUNIT_TEST(testThreadCreate_Diagnostic419);
  CPPUNIT_TEST(testThreadCreate_Diagnostic430);
  CPPUNIT_TEST(testThreadCreate_Diagnostic450);
  CPPUNIT_TEST(testThreadCreate_Diagnostic451);
  CPPUNIT_TEST(testThreadCreate_Diagnostic452);
  CPPUNIT_TEST(testThreadCreate_Diagnostic455);
  CPPUNIT_TEST(testThreadCreate_Diagnostic457);
  CPPUNIT_TEST(testThreadCreate_Diagnostic460);
  CPPUNIT_TEST(testThreadCreate_Diagnostic500);
  CPPUNIT_TEST(testThreadCreate_Diagnostic502);
  CPPUNIT_TEST(testThreadCreate_Diagnostic512);
  CPPUNIT_TEST(testThreadCreate_Diagnostic527);
  CPPUNIT_TEST(testThreadCreate_Diagnostic535);
  CPPUNIT_TEST(testThreadCreate_Diagnostic550);
  CPPUNIT_TEST(testThreadCreate_Diagnostic601);
  CPPUNIT_TEST(testThreadCreate_Diagnostic602);
  CPPUNIT_TEST(testThreadCreate_Diagnostic603);
  CPPUNIT_TEST(testThreadCreate_Diagnostic605);
  CPPUNIT_TEST(testThreadCreate_Diagnostic606);
  CPPUNIT_TEST(testThreadCreate_Diagnostic610);
  CPPUNIT_TEST(testThreadCreate_Diagnostic611);
  CPPUNIT_TEST(testThreadCreate_Diagnostic614);
  CPPUNIT_TEST(testThreadCreate_Diagnostic620);
  CPPUNIT_TEST(testThreadCreate_Diagnostic625);
  CPPUNIT_TEST(testThreadCreate_Diagnostic660);
  CPPUNIT_TEST(testThreadCreate_Diagnostic689);
  CPPUNIT_TEST(testThreadCreate_Diagnostic690);
  CPPUNIT_TEST(testThreadCreate_Diagnostic691);
  CPPUNIT_TEST(testThreadCreate_FailTaxCodeDiagnostic);
  CPPUNIT_TEST(testThreadCreate_AllPassTaxDiagnostic281);
  CPPUNIT_TEST(testThreadCreate_TaxRecSummaryDiagnostic);
  CPPUNIT_TEST(testThreadCreate_PFCRecSummaryDiagnostic);
  CPPUNIT_TEST(testThreadCreate_Diagnostic807);
  CPPUNIT_TEST(testThreadCreate_Diagnostic817);
  CPPUNIT_TEST(testThreadCreate_Diagnostic852);
  CPPUNIT_TEST(testThreadCreate_Diagnostic853);
  CPPUNIT_TEST(testThreadCreate_Diagnostic854);
  CPPUNIT_TEST(testThreadCreate_Diagnostic856);
  CPPUNIT_TEST(testThreadCreate_Diagnostic860);
  CPPUNIT_TEST(testThreadCreate_Diagnostic861);
  CPPUNIT_TEST(testThreadCreate_Diagnostic864);
  CPPUNIT_TEST(testThreadCreate_Diagnostic865);
  CPPUNIT_TEST(testThreadCreate_Diagnostic866);
  CPPUNIT_TEST(testThreadCreate_Diagnostic867);
  CPPUNIT_TEST(testThreadCreate_Diagnostic870);
  CPPUNIT_TEST(testThreadCreate_Diagnostic900);
  CPPUNIT_TEST(testThreadCreate_Diagnostic901);
  CPPUNIT_TEST(testThreadCreate_Diagnostic902);
  CPPUNIT_TEST(testThreadCreate_Diagnostic903);
  CPPUNIT_TEST(testThreadCreate_Diagnostic904);
  CPPUNIT_TEST(testThreadCreate_Diagnostic905);
  CPPUNIT_TEST(testThreadCreate_Diagnostic906);
  CPPUNIT_TEST(testThreadCreate_Diagnostic907);
  CPPUNIT_TEST(testThreadCreate_Diagnostic908);
  CPPUNIT_TEST(testThreadCreate_Diagnostic909);
  CPPUNIT_TEST(testThreadCreate_Diagnostic910);
  CPPUNIT_TEST(testThreadCreate_Diagnostic911);
  CPPUNIT_TEST(testThreadCreate_Diagnostic912);
  CPPUNIT_TEST(testThreadCreate_Diagnostic930);
  CPPUNIT_TEST(testThreadCreate_Diagnostic931);
  CPPUNIT_TEST(testThreadCreate_Diagnostic914);
  CPPUNIT_TEST(testThreadCreate_Diagnostic952);
  CPPUNIT_TEST(testThreadCreate_Diagnostic953);
  CPPUNIT_TEST(testThreadCreate_Diagnostic954);
  CPPUNIT_TEST(testThreadCreate_Diagnostic959);
  CPPUNIT_TEST(testThreadCreate_Diagnostic969);
  CPPUNIT_TEST(testThreadCreate_Diagnostic980);
  CPPUNIT_TEST(testThreadCreate_Diagnostic981);
  CPPUNIT_TEST(testThreadCreate_Diagnostic982);
  CPPUNIT_TEST(testThreadCreate_Diagnostic983);
  CPPUNIT_TEST(testThreadCreate_Diagnostic984);
  CPPUNIT_TEST(testThreadCreate_Diagnostic985);
  CPPUNIT_TEST(testThreadCreate_Diagnostic986);
  CPPUNIT_TEST(testThreadCreate_Diagnostic988);
  CPPUNIT_TEST(testThreadCreate_Diagnostic989);
  CPPUNIT_TEST(testThreadCreate_Diagnostic990);
  CPPUNIT_TEST(testThreadCreate_Diagnostic991);
  CPPUNIT_TEST(testThreadCreate_Diagnostic993);
  CPPUNIT_TEST(testThreadCreate_Diagnostic994);
  CPPUNIT_TEST(testThreadCreate_Diagnostic995);
  CPPUNIT_TEST(testThreadCreate_Diagnostic996);
  CPPUNIT_TEST(testThreadCreate_Diagnostic997);
  CPPUNIT_TEST(testThreadCreate_Diagnostic24);
  CPPUNIT_TEST(testThreadCreate_LegacyTaxDiagnostic24);
  CPPUNIT_TEST_SUITE_END();

  // data
private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Diagnostic* _root;
  // helper methods
public:
  void setUp()
  {
    _trx = _memHandle.insert(new PricingTrx());
    _root = _memHandle.insert(new Diagnostic);
  }

  void tearDown() { _memHandle.clear(); }

  // tests
public:
  void testThreadCreate_Diagnostic85()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic85;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic85, diagCollector->diagnosticType());
  }
  void testThreadCreate_Diagnostic191()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic191;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic191, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic194()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic194;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic194, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic198()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic198;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic198, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic199()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic199;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic199, diagCollector->diagnosticType());
  }

  void testThreadCreate_AllFareDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = AllFareDiagnostic;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(AllFareDiagnostic, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic201()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic201;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic201, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic607()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic607;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic607, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic204()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic204;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic204, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic208()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic208;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic208, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic212()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic212;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic212, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic220()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic220;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic220, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic223()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic223;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic223, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic225()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic225;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic225, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic231()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic231;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic231, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic233()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic233;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic233, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic251()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic251;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic251, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic252()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic252;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic252, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic253()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic253;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic253, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic254()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic254;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic254, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic255()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic255;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic255, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic256()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic256;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic256, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic257()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic257;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic257, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic258()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic258;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic258, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic259()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic259;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic259, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic270()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic270;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic270, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic302()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic302;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic302, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic304()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic304;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic304, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic311()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic311;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic311, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic312()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic312;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic312, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic315()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic315;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic315, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic316()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic316;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic316, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic323()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic323;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic323, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic325()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic325;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic325, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic327()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic327;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic327, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic335()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic335, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic372()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic372;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic372, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic400()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic400;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic400, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic404()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic404;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic404, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic405()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic405;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic405, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic411()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic411;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic411, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic413()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic413;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic413, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic419()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic419;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic419, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic430()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic430;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic430, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic450()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic450;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic450, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic451()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic451;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic451, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic452()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic452;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic452, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic455()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic455;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic455, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic457()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic457;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic457, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic460()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic460;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic460, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic500()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic500;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic500, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic502()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic502;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic502, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic512()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic512;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic512, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic527()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic527;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic527, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic535()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic535;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic535, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic550()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic550;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic550, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic601()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic601;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic601, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic602()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic602;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic602, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic603()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic603;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic603, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic605()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic605;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic605, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic606()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic606;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic606, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic610()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic610;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic610, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic611()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic611;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic611, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic614()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic614;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic614, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic620()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic620;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic620, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic625()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic625;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic625, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic660()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic660;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic660, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic689()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic689;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic689, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic690()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic690;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic690, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic691()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic691;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic691, diagCollector->diagnosticType());
  }

  void testThreadCreate_FailTaxCodeDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = FailTaxCodeDiagnostic;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(FailTaxCodeDiagnostic, diagCollector->diagnosticType());
  }

  void testThreadCreate_AllPassTaxDiagnostic281()
  {
    _trx->diagnostic().diagnosticType() = AllPassTaxDiagnostic281;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(AllPassTaxDiagnostic281, diagCollector->diagnosticType());
  }

  void testThreadCreate_TaxRecSummaryDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = TaxRecSummaryDiagnostic;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(TaxRecSummaryDiagnostic, diagCollector->diagnosticType());
  }

  void testThreadCreate_PFCRecSummaryDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = PFCRecSummaryDiagnostic;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(PFCRecSummaryDiagnostic, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic807()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic807;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic807, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic817()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic817;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic817, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic852()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic852;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic852, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic853()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic853;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic853, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic854()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic854;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic854, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic856()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic856;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic856, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic860()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic860;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic860, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic861()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic861;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic861, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic864()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic864;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic864, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic865()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic865;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic865, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic866()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic866;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic866, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic867()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic867;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic867, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic870()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic870;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic870, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic900()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic900;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic900, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic901()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic901;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic901, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic902()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic902;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic902, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic903()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic903;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic903, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic904()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic904;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic904, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic905()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic905;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic905, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic906()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic906;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic906, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic907()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic907;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic907, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic908()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic908;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic908, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic909()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic909;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic909, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic910()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic910;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic910, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic911()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic911;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic911, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic912()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic912;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic912, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic930()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic930;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic930, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic931()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic931;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic931, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic914()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic914;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic914, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic952()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic952;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic952, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic953()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic953;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic953, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic954()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic954;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic954, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic959()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic959;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic959, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic969()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic969;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic969, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic980()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic980;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic980, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic981()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic981;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic981, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic982()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic982;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic982, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic983()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic983;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic983, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic984()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic984;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic984, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic985()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic985;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic985, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic986()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic986;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic986, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic988()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic988;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic988, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic989()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic989;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic989, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic990()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic990;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic990, diagCollector->diagnosticType());
  }
  void testThreadCreate_Diagnostic991()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic991;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic991, diagCollector->diagnosticType());
  }
  void testThreadCreate_Diagnostic993()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic993;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic993, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic994()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic994;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic994, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic995()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic995;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic995, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic996()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic996;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic996, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic997()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic997;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic997, diagCollector->diagnosticType());
  }

  void testThreadCreate_Diagnostic24()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic24;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(Diagnostic24, diagCollector->diagnosticType());
  }

  void testThreadCreate_LegacyTaxDiagnostic24()
  {
    _trx->diagnostic().diagnosticType() = LegacyTaxDiagnostic24;
    DiagCollector* diagCollector = DCFactory::instance()->threadCreate(*_trx, *_root);
    CPPUNIT_ASSERT_EQUAL(LegacyTaxDiagnostic24, diagCollector->diagnosticType());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DCFactoryTest);
}
