//----------------------------------------------------------------------------
//	File: RDSection_psgLine.cpp
//
//	Author: Gern Blanston
//	created:      04/18/2007
//	description:  this is a unit test class for the routines in
//	FareDisplayService that depend on how fares are merged
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
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/MergeFares.h"
#include "FareDisplay/Templates/RDSection.h"
#include "Server/TseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class RDSection_psgLine : public CppUnit::TestFixture
{
  class FakePaxTypeFare : public PaxTypeFare
  {
    TestMemHandle _memHandle;

  public:
    FakePaxTypeFare(FareDisplayTrx& trx, PaxTypeCode ptc)
    {
      _fareDisplayInfo = _memHandle.create<FareDisplayInfo>();
      _fareDisplayInfo->initialize(trx, *this);
      _actualPaxType = _memHandle.create<PaxType>();
      PaxTypeInfo* pti = _memHandle.create<PaxTypeInfo>();

      _actualPaxType->paxType() = ptc;
      pti->paxType() = ptc;

      if (ptc != "ADT" && ptc != "NEG")
        pti->childInd() = 'Y';
      _actualPaxType->paxTypeInfo() = pti;

      // dummy auto-price info (is in same diplay line w/ psg type)
      FareInfo* fi = _memHandle.create<FareInfo>();
      fi->vendor() = Vendor::ATPCO;
      fi->fareTariff() = 202;
      _fare = _memHandle.create<Fare>();
      _fare->setFareInfo(fi);
    }
    virtual ~FakePaxTypeFare() { _memHandle.clear(); }
  };
  class FakeRDSection : public RDSection
  {
  public:
    FakeRDSection(FareDisplayTrx& trx) : RDSection(trx) {}

    void addPassengerTypeLine(PaxTypeFare& paxTypeFare)
    {
      RDSection::addPassengerTypeLine(paxTypeFare);
    }
  };

  CPPUNIT_TEST_SUITE(RDSection_psgLine);
  CPPUNIT_TEST(testRD_oneFare);
  CPPUNIT_TEST(testRD_blankFare);
  CPPUNIT_TEST(testRD_mergedFare);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    TestConfigInitializer::setValue("NEW_RDHEADER", "Y", "FAREDISPLAY_SVC");
    _rdSection = _memHandle.create<FakeRDSection>(*_trx);

    _ptfCNN = _memHandle.insert(new FakePaxTypeFare(*_trx, "CNN"));
    _ptfUNN = _memHandle.insert(new FakePaxTypeFare(*_trx, "UNN"));
    _ptfINS = _memHandle.insert(new FakePaxTypeFare(*_trx, "INS"));
  }
  void tearDown() { _memHandle.clear(); }

  void callAndAssert(PaxTypeFare* ptf, const char* expectedPTCs)
  {
    _rdSection->addPassengerTypeLine(*ptf);
    std::ostringstream expected;
    expected << "PASSENGER TYPE-" << std::setw(20) << std::left << expectedPTCs
             << "AUTO PRICE-NOT SUPPORTED    \n";
    CPPUNIT_ASSERT_EQUAL(expected.str(), _trx->response().str());
  }

  /**********************************************************************/

  void testRD_oneFare() { callAndAssert(_ptfCNN, "CNN"); }

  void testRD_blankFare()
  {
    _ptfCNN->actualPaxType() = 0;
    callAndAssert(_ptfCNN, "   ");
  }

  void testRD_mergedFare()
  {
    MergeFares::doMergeInPTF(_ptfCNN, _ptfUNN);
    MergeFares::doMergeInPTF(_ptfCNN, _ptfINS);

    callAndAssert(_ptfCNN, "CNN/INS/UNN");
  }

  TestMemHandle _memHandle;
  FareDisplayTrx* _trx;
  FakeRDSection* _rdSection;
  FakePaxTypeFare* _ptfCNN, *_ptfUNN, *_ptfINS;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RDSection_psgLine);
}
