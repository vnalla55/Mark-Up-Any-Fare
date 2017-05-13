#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleItem.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <cppunit/TestFixture.h>
#include <string>

namespace tse
{
class TestRuleItem : public RuleItem
{
  bool pass;

public:
  TestRuleItem(PricingTrx& trx,
               const CategoryRuleInfo& cri,
               const PricingUnit* pricingUnit,
               const PaxTypeFare& paxTypeFare,
               const RuleItemInfo* ruleItemInfo)
    : RuleItem(trx, cri, pricingUnit, paxTypeFare, ruleItemInfo)
  {
    pass = true;
  }

  virtual Record3ReturnTypes validateDateOverrideRuleItem(const uint16_t categoryNumber,
                                                          DiagCollector* diagPtr,
                                                          const DiagnosticTypes& callerDiag)
  {
    return (pass) ? PASS : FAIL;
  }

  void setPass(bool val = true) { pass = val; }
};

class RuleItemTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleItemTest);
  CPPUNIT_TEST(testHandleToursPassWhenNoTours);
  CPPUNIT_TEST(testHandleToursPassForPricingUnitPhase);
  CPPUNIT_TEST(testHandleToursPassForFarePathFactoryPhase);
  CPPUNIT_TEST(testHandleToursPassForNotValidPhase);
  CPPUNIT_TEST(testHandleToursTable994PassForPUPhase);
  CPPUNIT_TEST(testHandleToursTable994FailedForPUPhase);
  CPPUNIT_TEST(testHandleToursTable994PassForFPPhase);
  CPPUNIT_TEST(testHandleToursTable994FailedForFPPhase);
  CPPUNIT_TEST(testPrintDiag33XHeader_RexRADset);
  CPPUNIT_TEST(testPrintDiag33XHeader_RexRADnotSet);
  CPPUNIT_TEST(testPrintDiag33XHeader_RefundRADnotSet);
  CPPUNIT_TEST(testPrintDiag33XHeader_phase);
  CPPUNIT_TEST(testPrintDiag33XHeader_r3itemNo);
  CPPUNIT_TEST(testPrintDiag33XHeader_r2farerule);
  CPPUNIT_TEST(testPrintDiag33XHeader_pu);

  CPPUNIT_TEST(testHandlePenaltiesDiag316WithPenaltyInfoRecord);
  CPPUNIT_SKIP_TEST(testHandlePenaltiesAddPenaltyInfoRecord);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  TestRuleItem* _item;
  TestRuleItem* _penaltyItem;
  CategoryRuleInfo* _cri;
  PricingUnit* _pricingUnit;
  PaxTypeFare* _paxTypeFare;
  RuleItemInfo* _ruleItemInfo;
  PenaltyInfo* _penaltyInfo;

  TestMemHandle _memHandle;

public:
  RuleItemTest() : _item(0) {}

  bool findStringInDiag33XHeader(const std::string& stringToFind)
  {
    DiagCollector dc;
    dc.activate();
    Loc* loc = _memHandle.insert(new Loc);
    loc->loc() = "KRK";
    _paxTypeFare->fareMarket()->origin() = loc;
    _paxTypeFare->fareMarket()->destination() = loc;
    _paxTypeFare->setFare(_memHandle.insert(new Fare));
    FareInfo* fi = _memHandle.insert(new FareInfo);
    fi->fareClass() = "12HENS";
    _paxTypeFare->fare()->setFareInfo(fi);
    _item->printDiag33XHeader(dc);
    std::string::size_type position = dc.str().find(stringToFind);
    if (position != std::string::npos)
    {
      const std::string finded = dc.str().substr(position, stringToFind.size());
      CPPUNIT_ASSERT_EQUAL(stringToFind, finded);
    }
    return position != std::string::npos;
  }

  void testPrintDiag33XHeader_RexRADset()
  {
    _paxTypeFare->fareMarket()->ruleApplicationDate() = DateTime(2001, 1, 1);
    CPPUNIT_ASSERT(findStringInDiag33XHeader("RULE APPLICATION DATE: 2001-01-01"));
  }

  void testPrintDiag33XHeader_RexRADnotSet()
  {
    CPPUNIT_ASSERT(!findStringInDiag33XHeader("RULE APPLICATION DATE:"));
  }

  void testPrintDiag33XHeader_RefundRADnotSet()
  {
    _item = _memHandle.insert(new TestRuleItem(*(_memHandle.insert(new RefundPricingTrx)),
                                               *_cri,
                                               _pricingUnit,
                                               *_paxTypeFare,
                                               _ruleItemInfo));
    CPPUNIT_ASSERT(findStringInDiag33XHeader("RULE APPLICATION DATE: N/A"));
  }

  void testPrintDiag33XHeader_phase()
  {
    _item->_phase = RuleItem::FarePhase;
    CPPUNIT_ASSERT(findStringInDiag33XHeader(" DIAGNOSTICS\nPHASE: FARE RULE MATCHING"));
  }

  void testPrintDiag33XHeader_r3itemNo()
  {
    _ruleItemInfo->itemNo() = 1234;
    CPPUNIT_ASSERT(findStringInDiag33XHeader("R3 ITEM NUMBER: 1234\nKRK KRK 12HENS"));
  }

  void testPrintDiag33XHeader_r2farerule()
  {
    _cri->vendorCode() = "PTA";
    _cri->tariffNumber() = 1234;
    _cri->carrierCode() = "XX";
    _cri->ruleNumber() = "FROG";
    CPPUNIT_ASSERT(findStringInDiag33XHeader("R2:FARERULE  :  PTA 1234 XX FROG"));
  }

  void testPrintDiag33XHeader_pu()
  {
    _item->_phase = RuleItem::PricingUnitFactoryPhase;
    CPPUNIT_ASSERT(findStringInDiag33XHeader("PU: EMPTY TRAVEL SEG VECTOR\n"));
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.insert(new PricingTrx);
    _cri = _memHandle.insert(new CategoryRuleInfo);
    _pricingUnit = _memHandle.insert(new PricingUnit);
    _paxTypeFare = _memHandle.insert(new PaxTypeFare);
    _paxTypeFare->fareMarket() = _memHandle.insert(new FareMarket);
    _ruleItemInfo = _memHandle.insert(new RuleItemInfo);
    _item = _memHandle.insert(
        new TestRuleItem(*_trx, *_cri, _pricingUnit, *_paxTypeFare, _ruleItemInfo));
    _item->_trx->setRequest(_memHandle.insert(new PricingRequest()));
    _item->_trx->getRequest()->ticketingDT() = DateTime::localTime().nextDay();
  }

  void tearDown() { _memHandle.clear(); }

  void testHandleToursPassWhenNoTours()
  {
    _item->_ruleItemInfo = 0;
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void testHandleToursPassForPricingUnitPhase()
  {
    _item->_phase = RuleItem::PricingUnitPhase;
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void testHandleToursPassForFarePathFactoryPhase()
  {
    _item->_phase = RuleItem::FarePathFactoryPhase;
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void testHandleToursPassForNotValidPhase()
  {
    _item->_phase = RuleItem::UnKnown;
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void testHandleToursTable994PassForPUPhase()
  {
    _item->_phase = RuleItem::PricingUnitPhase;
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }
  void testHandleToursTable994FailedForPUPhase()
  {
    _item->_phase = RuleItem::PricingUnitPhase;
    _item->setPass(false);
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void testHandleToursTable994PassForFPPhase()
  {
    _item->_phase = RuleItem::FarePathFactoryPhase;
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void testHandleToursTable994FailedForFPPhase()
  {
    _item->_phase = RuleItem::FarePathFactoryPhase;
    _item->setPass(false);
    CPPUNIT_ASSERT_EQUAL(PASS, _item->handleTours());
  }

  void maxPenalty_setUp()
  {
    PaxType* paxType = _memHandle.insert(new PaxType);
    paxType->maxPenaltyInfo() = _memHandle.insert(new MaxPenaltyInfo);
    _paxTypeFare->initialize(_memHandle.insert(new Fare),
                             paxType,
                             _memHandle.insert(new FareMarket),
                             *_trx);
    _paxTypeFare->paxType()->maxPenaltyInfo() = _memHandle.insert(new MaxPenaltyInfo);
    _penaltyInfo = _memHandle.insert(new PenaltyInfo);
    _penaltyItem = _memHandle.insert(
        new TestRuleItem(*_trx, *_cri, _pricingUnit, *_paxTypeFare, _penaltyInfo));
  }

  void testHandlePenaltiesDiag316WithPenaltyInfoRecord()
  {
    maxPenalty_setUp();

    Diagnostic& trxDiagnostic = _trx->diagnostic();
    trxDiagnostic.diagnosticType() = Diagnostic316;
    _penaltyItem->setPhase(RuleItem::FareDisplayPhase);
    CPPUNIT_ASSERT_EQUAL(FAIL, _penaltyItem->handlePenalties()); // not a FareDisplayTrx so FAIL
    std::string diagResponse = trxDiagnostic.toString();
    CPPUNIT_ASSERT(diagResponse.size() == 0); // never got a chance to print anything

    CPPUNIT_ASSERT_EQUAL(SKIP, _item->handlePenalties()); // not a PenaltyInfo so SKIP
    diagResponse = trxDiagnostic.toString();
    CPPUNIT_ASSERT(diagResponse.size() == 0); // same thing again
  }

  void testHandlePenaltiesAddPenaltyInfoRecord()
  {
    maxPenalty_setUp();

    CPPUNIT_ASSERT_EQUAL(PASS, _penaltyItem->handlePenalties());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleItemTest);
}
