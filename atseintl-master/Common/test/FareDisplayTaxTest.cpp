#include "test/include/CppUnitHelperMacros.h"

#include "Common/FareDisplayTax.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/FareDisplayPref.h"
#include "Common/test/FareDisplayTaxTest.h"

namespace tse
{

using namespace std;

void
FareDisplayTaxTest::setUp()
{
  _trx = createFDTrx();
  _taxCode.clear();
  _globalDir = GlobalDirection::NO_DIR;
  _fm = createFareMarket();
  _trx->fareMarket().push_back(_fm);
}

void
FareDisplayTaxTest::tearDown()
{
  destroyFDTrx(_trx);
  destroyFareMarket(_fm);
}

//////////////////////////
////    UTILS       //////
//////////////////////////

FareDisplayTrx*
FareDisplayTaxTest::createFDTrx()
{
  FareDisplayTrx* trx = new FareDisplayTrx;
  FareDisplayRequest* req = new FareDisplayRequest;
  Agent* agent = new Agent;
  Loc* agentLoc = new Loc;

  trx->setRequest(req);
  req->ticketingAgent() = agent;
  agent->agentLocation() = agentLoc;

  return trx;
}

void
FareDisplayTaxTest::destroyFDTrx(FareDisplayTrx* trx)
{
  // see createFDTrx() to ensure all alloc'ed are deleted
  delete trx->getRequest()->ticketingAgent()->agentLocation();
  delete trx->getRequest()->ticketingAgent();
  delete trx->getRequest();
  delete trx;
}

FareMarket*
FareDisplayTaxTest::createFareMarket()
{
  FareMarket* fm = new FareMarket;
  Loc* orig = new Loc;
  Loc* dest = new Loc;

  fm->origin() = orig;
  fm->destination() = dest;
  return fm;
}

void
FareDisplayTaxTest::destroyFareMarket(FareMarket* fm)
{
  delete fm->origin();
  delete fm->destination();
  delete fm;
}

void
FareDisplayTaxTest::assertTaxCodes(int expected, TaxCode taxCodes[])
{
  CPPUNIT_ASSERT_EQUAL(size_t(expected), _taxCode.size());
  for (int i = 0; i < expected; i++)
  {
    CPPUNIT_ASSERT(_taxCode.end() != _taxCode.find(taxCodes[i]));
  }
}

void
FareDisplayTaxTest::setLoc(const Loc* loc, const NationCode& nation, const StateCode& state)
{
  const_cast<Loc*>(loc)->nation() = nation;
  const_cast<Loc*>(loc)->state() = state;
}

void
FareDisplayTaxTest::setBoardOff(FareMarket* fm, const LocCode& board, const LocCode& off)
{
  fm->boardMultiCity() = board;
  fm->offMultiCity() = off;
}

//////////////////////////
////    TESTS       //////
//////////////////////////

void
FareDisplayTaxTest::testGetTaxCodeForFQ_fareTaxRequest()
{
  _trx->getRequest()->requestType() = FARE_TAX_REQUEST;

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir);
  CPPUNIT_ASSERT_EQUAL(size_t(0), _taxCode.size());
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_SingleUS()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = UNITED_STATES;

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);
  TaxCode expectedCodes[] = { FareDisplayTax::TAX_CODE_YZ1 };
  assertTaxCodes(1, expectedCodes);
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_DoubleOrigUS()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = UNITED_STATES;
  const_cast<Loc*>(_fm->origin())->nation() = UNITED_STATES;

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);

  TaxCode expectedCodes[] = { FareDisplayTax::TAX_CODE_YZ1, FareDisplayTax::TAX_CODE_US1 };
  assertTaxCodes(2, expectedCodes);
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_NoUSTaxForOrigUSTerr()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = UNITED_STATES;
  setLoc(_fm->origin(), UNITED_STATES, GUAM);

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);
  TaxCode expectedCodes[] = { FareDisplayTax::TAX_CODE_YZ1 };
  assertTaxCodes(1, expectedCodes);
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_OrigDestNotTerrUS()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = UNITED_STATES;
  setLoc(_fm->origin(), UNITED_STATES, "TX");
  setLoc(_fm->destination(), UNITED_STATES, ALASKA);

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);

  TaxCode expectedCodes[] = { FareDisplayTax::TAX_CODE_YZ1, FareDisplayTax::TAX_CODE_US1,
                              FareDisplayTax::TAX_CODE_US2 };
  assertTaxCodes(3, expectedCodes);
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_AgentLocAustraliaDifferentCityPair()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = AUSTRALIA;
  setBoardOff(_fm, "LAX", "DFW");

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);

  TaxCode expectedCodes[] = { FareDisplayTax::TAX_CODE_YZ1, FareDisplayTax::TAX_CODE_UO2 };

  assertTaxCodes(2, expectedCodes);
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_AgentLocAustraliaSameCityPair()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = AUSTRALIA;
  // fake-out questionable null check in trx::isSameCityPairRequest()
  AirSeg seg;
  _trx->travelSeg().push_back(&seg);
  setBoardOff(_fm, "DFW", "DFW");

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);

  TaxCode expectedCodes[] = {};
  assertTaxCodes(0, expectedCodes);
}

void
FareDisplayTaxTest::testGetTaxCodeForFQ_AgentLocKiwi()
{
  const_cast<Loc*>(_trx->getRequest()->ticketingAgent()->agentLocation())->nation() = NEW_ZEALAND;

  FareDisplayTax::getTaxCodeForFQ(*_trx, _taxCode, _globalDir, _fm);

  TaxCode expectedCodes[] = { FareDisplayTax::TAX_CODE_YZ1, FareDisplayTax::TAX_CODE_NZ1 };
  assertTaxCodes(2, expectedCodes);
}

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayTaxTest);

} // tse
