#include "Common/Config/ConfigMan.h"
#include "Pricing/EstimatedSeatValue.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class EstimatedSeatValueTest : public CPPUNIT_NS::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(EstimatedSeatValueTest);
  CPPUNIT_SKIP_TEST(testCheckCarrier);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("ESV_ONLINE_ONLY_CARRIERS", "PP|QQ", "SHOPPING_OPT");
    TestConfigInitializer::setValue(
        "ESV_RESTRICTED_CARRIERS", "*|AA|BB|CC|*|XX|YY|ZZ|*", "SHOPPING_OPT");
    TestConfigInitializer::setValue("ENABLE_INTERLINE_TICKET_CARRIER", "N", "ESV_PERFORMANCE_OPT");
    EstimatedSeatValue::classInit();
  }

  void tearDown() { _memH.clear(); }

  void testCheckCarrier()
  {
    ShoppingTrx trx;
    std::set<CarrierCode> cxrs;

    CarrierCode code("AA");

    cxrs.clear();
    cxrs.insert("QQ");
    CPPUNIT_ASSERT_MESSAGE("online w/ online only carrier",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("QQ");
    cxrs.insert("RR");
    cxrs.insert("SS");
    cxrs.insert("TT");
    CPPUNIT_ASSERT_MESSAGE("interline w/ online only carrier",
                           !EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("RR");
    cxrs.insert("SS");
    cxrs.insert("TT");
    CPPUNIT_ASSERT_MESSAGE("interline w/ no online only carrier",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("BB");
    cxrs.insert("CC");
    cxrs.insert("DD");
    cxrs.insert("YY");
    cxrs.insert("ZZ");
    CPPUNIT_ASSERT_MESSAGE("interline w/ 'allowed' carriers",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("AA");
    cxrs.insert("BB");
    CPPUNIT_ASSERT_MESSAGE("interline w/ restricted and 'allowed'",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("XX");
    cxrs.insert("ZZ");
    CPPUNIT_ASSERT_MESSAGE("interline w/ 2nd restricted and 'allowed'",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("XX");
    cxrs.insert("BB");
    CPPUNIT_ASSERT_MESSAGE("interline w/ 2nd restricted and 1st 'allowed'",
                           !EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("AA");
    cxrs.insert("YY");
    cxrs.insert("XX");
    cxrs.insert("BB");
    CPPUNIT_ASSERT_MESSAGE("interline w/ 1st and 2nd restricted carriers",
                           !EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("AA");
    CPPUNIT_ASSERT_MESSAGE("online w/ 1st restricted carrier",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));

    cxrs.clear();
    cxrs.insert("XX");
    CPPUNIT_ASSERT_MESSAGE("online w/ 2nd restricted carrier",
                           EstimatedSeatValue::checkCarrier(trx, code, NULL, cxrs));
  }

private:
  ShoppingTrx* _trxP;
  DataHandle _dataHandle;
  TestMemHandle _memH;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EstimatedSeatValueTest);
}
