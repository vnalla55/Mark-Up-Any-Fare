#include "test/include/CppUnitHelperMacros.h"
#include "Rules/RuleApplicationBase.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "Diagnostic/Diag302Collector.h"
#include "DataModel/PricingTrx.h"

namespace tse
{

class PricingTrx;
class Itin;
class PaxTypeFare;
class RuleItemInfo;
class FareMarket;
class FarePath;
class PricingUnit;
class FareUsage;

class RuleApplicationBaseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleApplicationBaseTest);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenPassDates);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenDatesEqual);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenFailOnTktEffDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenFailOnTktDiscDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenFailOnTvlEffDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenFailOnTvlDiscDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenFailOnResEffDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenFailOnResDiscDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenDiagEnabled);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenAllPassOnTvlDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhen1stFailOnTvlDateAnd2ndPass);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhen1stPassAnd2ndFailOnTvlDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenAllFailOnTvlEffDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenAllFailOnTvlDiscDate);
  CPPUNIT_TEST(testValidateDateOverrideRuleItemWhenAllFailAndDiagEnabled);
  CPPUNIT_TEST(testSetResultToAllDates);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ruleApp = _memHandle.create<RuleApplicationBaseMock>();
    _r3ReturnTypes = _memHandle.insert(new std::vector<Record3ReturnTypes>);
    _uniqueTvlDates = _memHandle.insert(new std::vector<DateTime>);
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
  }
  void tearDown() { _memHandle.clear(); }

  void addTvlDate(DateTime tvlDate)
  {
    _uniqueTvlDates->push_back(tvlDate);
    _r3ReturnTypes->push_back(PASS);
  }

  DateOverrideRuleItem& setDateOverrideRuleItems(int shift = 1)
  {
    DateOverrideRuleItem& dorItem = ((RuleApplicationBaseMock*)_ruleApp)->dorItem();
    dorItem.tktEffDate() = DateTime(2010, 1, 10 - 1);
    dorItem.tktDiscDate() = DateTime(2010, 1, 10 + 1);
    dorItem.tvlEffDate() = DateTime(2010, 1, 20 - 1);
    dorItem.tvlDiscDate() = DateTime(2010, 1, 20 + 1);
    dorItem.resEffDate() = DateTime(2010, 1, 10 - 1);
    dorItem.resDiscDate() = DateTime(2010, 1, 10 + 1);

    return dorItem;
  }

  void testValidateDateOverrideRuleItemWhenPassDates()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenDatesEqual()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems(0);
    addTvlDate(DateTime(2010, 1, 20));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenFailOnTktEffDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    DateOverrideRuleItem& dorItem = setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    dorItem.tktEffDate() = DateTime(2010, 1, 7);
    dorItem.tktDiscDate() = DateTime(2010, 1, 9);

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenFailOnTktDiscDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    DateOverrideRuleItem& dorItem = setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    dorItem.tktEffDate() = DateTime(2010, 1, 11);
    dorItem.tktDiscDate() = DateTime(2010, 1, 21);

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenFailOnTvlEffDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    DateOverrideRuleItem& dorItem = setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    dorItem.tvlEffDate() = DateTime(2010, 1, 9);
    dorItem.tvlDiscDate() = DateTime(2010, 1, 19);

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenFailOnTvlDiscDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    DateOverrideRuleItem& dorItem = setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    dorItem.tvlEffDate() = DateTime(2010, 1, 21);
    dorItem.tvlDiscDate() = DateTime(2010, 1, 23);

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenFailOnResEffDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    DateOverrideRuleItem& dorItem = setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    dorItem.resEffDate() = DateTime(2010, 1, 7);
    dorItem.resDiscDate() = DateTime(2010, 1, 9);

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
  }

  void testValidateDateOverrideRuleItemWhenFailOnResDiscDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    DateOverrideRuleItem& dorItem = setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    dorItem.resEffDate() = DateTime(2010, 1, 11);
    dorItem.resDiscDate() = DateTime(2010, 1, 21);

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
  }

  Diag302Collector* createDiagnostic302()
  {
    Diag302Collector* diag =
        _memHandle.insert(new Diag302Collector(*_memHandle.create<Diagnostic>()));
    diag->activate();
    return diag;
  }

  void testValidateDateOverrideRuleItemWhenDiagEnabled()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 1;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    Diag302Collector* diag = createDiagnostic302();
    _trx->diagnostic().diagnosticType() = Diagnostic302;
    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           diag,
                                           Diagnostic302);

    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("  -: TABLE 994 -OVERRIDE DATE DATA :- ATP - 1\n"
                                     "  TABLE 994 TVL DATE 2010-Jan-20: PASS\n"),
                         diag->str());
  }

  void testValidateDateOverrideRuleItemWhenAllPassOnTvlDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 20));
    addTvlDate(DateTime(2010, 1, 21));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[1]);
  }

  void testValidateDateOverrideRuleItemWhen1stFailOnTvlDateAnd2ndPass()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 18));
    addTvlDate(DateTime(2010, 1, 19));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[1]);
  }

  void testValidateDateOverrideRuleItemWhen1stPassAnd2ndFailOnTvlDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 21));
    addTvlDate(DateTime(2010, 1, 22));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(PASS, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[1]);
  }

  void testValidateDateOverrideRuleItemWhenAllFailOnTvlEffDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 17));
    addTvlDate(DateTime(2010, 1, 18));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[1]);
  }

  void testValidateDateOverrideRuleItemWhenAllFailOnTvlDiscDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 22));
    addTvlDate(DateTime(2010, 1, 23));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[1]);
  }

  void testValidateDateOverrideRuleItemWhenAllFailOnTvlDate()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 0;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 18));
    addTvlDate(DateTime(2010, 1, 22));

    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           0,
                                           DiagnosticNone);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[1]);
  }

  void testValidateDateOverrideRuleItemWhenAllFailAndDiagEnabled()
  {
    VendorCode vendorCode = "ATP";
    uint32_t overrideDateTblItemNo = 1;
    _request->ticketingDT() = DateTime(2010, 1, 10);
    setDateOverrideRuleItems();
    addTvlDate(DateTime(2010, 1, 17));
    addTvlDate(DateTime(2010, 1, 18));
    Diag302Collector* diag = createDiagnostic302();
    _trx->diagnostic().diagnosticType() = Diagnostic302;
    _ruleApp->validateDateOverrideRuleItem(*_r3ReturnTypes,
                                           *_uniqueTvlDates,
                                           *_trx,
                                           vendorCode,
                                           overrideDateTblItemNo,
                                           diag,
                                           Diagnostic302);

    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[0]);
    CPPUNIT_ASSERT_EQUAL(SKIP, (*_r3ReturnTypes)[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("  -: TABLE 994 -OVERRIDE DATE DATA :- ATP - 1\n"
                                     "  TABLE 994: NOT MATCH ALL DATES\n"),
                         diag->str());
  }

  void testSetResultToAllDates()
  {
    std::vector<Record3ReturnTypes> r3ReturnTypes;
    r3ReturnTypes.push_back(PASS);
    r3ReturnTypes.push_back(FAIL);
    r3ReturnTypes.push_back(PASS);

    _ruleApp->setResultToAllDates(r3ReturnTypes, SKIP);

    CPPUNIT_ASSERT_EQUAL(SKIP, r3ReturnTypes[0]);
    CPPUNIT_ASSERT_EQUAL(FAIL, r3ReturnTypes[1]);
    CPPUNIT_ASSERT_EQUAL(SKIP, r3ReturnTypes[2]);
  }

  class RuleApplicationBaseMock : public RuleApplicationBase
  {
  public:
    Record3ReturnTypes validate(PricingTrx& trx,
                                Itin& itin,
                                const PaxTypeFare& fare,
                                const RuleItemInfo* rule,
                                const FareMarket& fareMarket)
    {
      return PASS;
    }

    Record3ReturnTypes validate(PricingTrx& trx,
                                const RuleItemInfo* rule,
                                const FarePath& farePath,
                                const PricingUnit& pricingUnit,
                                const FareUsage& fareUsage)
    {
      return PASS;
    }

    DateOverrideRuleItem& dorItem() { return _dorItem; }
    const DateOverrideRuleItem& dorItem() const { return _dorItem; }

  protected:
    const DateOverrideRuleItem* getDateOverrideRuleItem(PricingTrx& trx,
                                                        const VendorCode& vendorCode,
                                                        const uint32_t& overrideDateTblItemNo) const
    {
      return &_dorItem;
    }

  private:
    DateOverrideRuleItem _dorItem;
  };

protected:
  TestMemHandle _memHandle;
  RuleApplicationBase* _ruleApp;
  std::vector<Record3ReturnTypes>* _r3ReturnTypes;
  std::vector<DateTime>* _uniqueTvlDates;
  PricingTrx* _trx;
  PricingRequest* _request;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleApplicationBaseTest);
}
