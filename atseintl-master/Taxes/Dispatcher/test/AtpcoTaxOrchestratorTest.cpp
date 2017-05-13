// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/TaxTrx.h"
#include "Server/TseServer.h"
#include "Taxes/Dispatcher/AtpcoTaxOrchestrator.h"

#include <memory>

namespace tse
{
class TseServerMock : public tse::TseServer
{
public:
  TseServerMock() : TseServer("TseServerMock") {}
};

class AtpcoTaxOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AtpcoTaxOrchestratorTest);
  CPPUNIT_TEST(testProcessTaxTrx);
  CPPUNIT_SKIP_TEST(testTaxOverrides);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _tseServerMock = _memHandle.create<TseServerMock>();

    _trx = _memHandle.create<PricingTrx>();

    _trx->setRequest(_memHandle.create<PricingRequest>());
  }

  void tearDown() { _memHandle.clear(); }

  void testProcessTaxTrx()
  {
    AtpcoTaxOrchestrator orchestrator("AtpcoTaxOrchestratorUnderTest", *_tseServerMock);
    //    TaxTrx taxTrx;
    //    orchestrator.process(taxTrx);
  }

  // TODO: skipped, need to set a lot of things in _trx for this to work now
  void testTaxOverrides()
  {
    TaxOverride taxOverride;
    _trx->getRequest()->taxOverride().push_back(&taxOverride);

    AtpcoTaxOrchestrator orchestrator("AtpcoTaxOrchestratorUnderTest", *_tseServerMock);

    CPPUNIT_ASSERT(orchestrator.atpcoProcess(*_trx)); //, tax::implementationPhase3
    CPPUNIT_ASSERT_EQUAL(size_t(0), _trx->taxResponse().size());
  }

private:
  TestMemHandle _memHandle;
  TseServerMock* _tseServerMock;
  PricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AtpcoTaxOrchestratorTest);
} // namespace tse
