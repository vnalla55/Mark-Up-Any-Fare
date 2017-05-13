//----------------------------------------------------------------------------
//	File: FDService_merge.cpp
//
//	Author: Gern Blanston
//  	created:      02/01/2007
//  	description:  this is a unit test class for the routines in
//  	FareDisplayService that depend on how fares are merged
//
//  copyright sabre 2007
//
//          the copyright to the computer program(s) herein
//          is the property of sabre.
//          the program(s) may be used and/or copied only with
//          the written permission of sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/FareDisplayService.h"
#include "FareDisplay/MergeFares.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Server/TseServer.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>


using namespace tse;
using namespace std;

namespace utest_FDService_merge
{

class FakePaxTypeFare : public PaxTypeFare
{
public:
  FakePaxTypeFare(FareDisplayTrx& trx, PaxTypeCode ptc)
  {
    _fareDisplayInfo = new FareDisplayInfo;
    _fareDisplayInfo->initialize(trx, *this);
    _actualPaxType = new PaxType;
    PaxTypeInfo* pti = new PaxTypeInfo;

    _actualPaxType->paxType() = ptc;
    pti->paxType() = ptc;

    if (ptc != "ADT" && ptc != "NEG")
      pti->childInd() = 'Y';
    _actualPaxType->paxTypeInfo() = pti;
  }
  virtual ~FakePaxTypeFare()
  {
    delete _actualPaxType->paxTypeInfo();
    delete _actualPaxType;
    delete _fareDisplayInfo;
  }
  bool isValidForPricing() const
  {
    return true;
  };
};

class FakeTseServer : public TseServer
{
public:
  FakeTseServer() : TseServer() {};
  ~FakeTseServer() {};
};

} // end namespace

using namespace utest_FDService_merge;
namespace tse
{

class FDService_merge : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDService_merge);
  CPPUNIT_TEST(testRD_noFare);
  CPPUNIT_TEST(testRD_oneFare);
  CPPUNIT_TEST(testRD_blankFares);
  CPPUNIT_TEST(testRD_eraseFare);
  CPPUNIT_TEST(testRD_keepMerged);
  CPPUNIT_TEST(testRD_keepADT);
  CPPUNIT_TEST(testRD_keepADT_requestNEG);
  CPPUNIT_TEST(testRD_keepCNN);
  CPPUNIT_TEST(testRD_keepCNN_requestUNN);
  CPPUNIT_TEST(testRD_keepCNN_requestINS);
  CPPUNIT_TEST(testRD_diagADT);
  CPPUNIT_TEST(testRD_diagUNN);
  CPPUNIT_TEST_SUITE_END();

public:
  /*
    FDService_merge();
    void setUp() ;
    void tearDown();
    void putEachPaxTypeIntoTrx();
    void callAndAssertOnePTF(PaxTypeFare *ptf, PaxTypeCode ptc);
    void callAndAssertDiag(PaxTypeCode ptc);

    void testRD_noFare();
    void testRD_oneFare();
    void testRD_blankFares();
    void testRD_eraseFare();
    void testRD_keepMerged();
    void testRD_keepADT();
    void testRD_keepADT_requestNEG();
    void testRD_keepCNN();
    void testRD_keepCNN_requestUNN();
    void testRD_keepCNN_requestINS();
    void testRD_diagADT();
    void testRD_diagUNN();
  */
  TestMemHandle _memHandle;
  FareDisplayTrx* _trx;
  vector<PaxTypeFare*>* _allPTF;
  FareDisplayService* _fdService;
  FakeTseServer* _tseServer;
  FakePaxTypeFare* _ptfADT, *_ptfNEG, *_ptfCNN, *_ptfUNN, *_ptfINS;

  FDService_merge() {}

  void setUp()
  {
    _trx = _memHandle.create<FareDisplayTrx>();
    _trx->setRequest(_memHandle.create<FareDisplayRequest>());
    _allPTF = &_trx->allPaxTypeFare();
    _ptfADT = _memHandle.create<FakePaxTypeFare>(*_trx, "ADT");
    _ptfNEG = _memHandle.create<FakePaxTypeFare>(*_trx, "NEG");
    _ptfCNN = _memHandle.create<FakePaxTypeFare>(*_trx, "CNN");
    _ptfUNN = _memHandle.create<FakePaxTypeFare>(*_trx, "UNN");
    _ptfINS = _memHandle.create<FakePaxTypeFare>(*_trx, "INS");

    _tseServer = _memHandle.create<FakeTseServer>();
    _fdService = _memHandle.create<FareDisplayService>("test", *_tseServer);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void putEachPaxTypeIntoTrx()
  {
    _allPTF->push_back(_ptfADT);
    _allPTF->push_back(_ptfCNN);

    // may need to call all of MergeFare for full test
    MergeFares::doMergeInPTF(_ptfADT, _ptfNEG);
    MergeFares::doMergeInPTF(_ptfCNN, _ptfUNN);
    MergeFares::doMergeInPTF(_ptfCNN, _ptfINS);
  }

  void callAndAssertOnePTF(PaxTypeFare* ptf, PaxTypeCode ptc)
  {
    _trx->getRequest()->displayPassengerTypes().push_back(ptc);
    _fdService->checkPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _allPTF->size());
    CPPUNIT_ASSERT_EQUAL((void*)ptf, (void*)_allPTF->front());
  }

  void callAndAssertDiag(PaxTypeCode ptc)
  {
    _trx->getRequest()->diagnosticNumber() = DIAG_200_ID;
    _trx->getRequest()->displayPassengerTypes().push_back(ptc);

    // for diagnostics, keep fare but marked invalid
    _fdService->checkPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _allPTF->size());
    CPPUNIT_ASSERT_EQUAL((void*)_ptfADT, (void*)_allPTF->front());
    CPPUNIT_ASSERT_EQUAL((void*)_ptfCNN, (void*)_allPTF->back());
  }

  /**********************************************************************/

  // do nothing when less than 2 fares
  void testRD_noFare()
  {
    _fdService->checkPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)0, _allPTF->size());
  }

  void testRD_oneFare()
  {
    PaxTypeFare ptf;
    _allPTF->push_back(&ptf);

    _fdService->checkPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _allPTF->size());
  }

  void testRD_blankFares()
  {
    PaxTypeFare ptf1;
    PaxTypeFare ptf2;
    ptf1.fareDisplayInfo() = new FareDisplayInfo;
    ptf1.fareDisplayInfo()->initialize(*_trx, ptf1);
    ptf2.fareDisplayInfo() = new FareDisplayInfo;
    ptf2.fareDisplayInfo()->initialize(*_trx, ptf2);

    _allPTF->push_back(&ptf1);
    _allPTF->push_back(&ptf2);
    _trx->getRequest()->displayPassengerTypes().push_back("ADT");

    _fdService->checkPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _allPTF->size());
  }

  void testRD_eraseFare()
  {
    FakePaxTypeFare ptf1(*_trx, "ADT");
    FakePaxTypeFare ptf2(*_trx, "CNN");
    _allPTF->push_back(&ptf1);
    _allPTF->push_back(&ptf2);

    callAndAssertOnePTF(&ptf1, "ADT");
  }

  void testRD_keepMerged()
  {
    FakePaxTypeFare ptf0(*_trx, "ADT");
    FakePaxTypeFare ptf1(*_trx, "NEG");
    FakePaxTypeFare ptf1b(*_trx, "ADT");
    _allPTF->push_back(&ptf0);
    _allPTF->push_back(&ptf1);
    MergeFares::doMergeInPTF(&ptf1, &ptf1b);
    _trx->getRequest()->displayPassengerTypes().push_back("ADT");

    _fdService->checkPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _allPTF->size());
    CPPUNIT_ASSERT_EQUAL((void*)&ptf0, (void*)_allPTF->front());
    CPPUNIT_ASSERT_EQUAL((void*)&ptf1, (void*)_allPTF->back());
  }

  void testRD_keepADT()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertOnePTF(_ptfADT, "ADT");
  }
  void testRD_keepADT_requestNEG()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertOnePTF(_ptfADT, "NEG");
  }
  void testRD_keepCNN()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertOnePTF(_ptfCNN, "CNN");
  }
  void testRD_keepCNN_requestUNN()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertOnePTF(_ptfCNN, "UNN");
  }
  void testRD_keepCNN_requestINS()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertOnePTF(_ptfCNN, "INS");
  }

  void testRD_diagADT()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertDiag("ADT");

    CPPUNIT_ASSERT(_ptfADT->fareDisplayStatus().isNull());
    CPPUNIT_ASSERT(_ptfCNN->notSelectedForRD());
  }
  void testRD_diagUNN()
  {
    putEachPaxTypeIntoTrx();
    callAndAssertDiag("UNN");

    CPPUNIT_ASSERT(_ptfADT->notSelectedForRD());
    CPPUNIT_ASSERT(_ptfCNN->fareDisplayStatus().isNull());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDService_merge);

} // end namespace
