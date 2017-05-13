#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Taxes/LegacyTaxes/TaxSP9700.h"
#include "Taxes/LegacyTaxes/test/TaxLocIteratorMock.h"
#include "DBAccess/Loc.h"

namespace tse
{

class TaxSP9700Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP9700Test);
  CPPUNIT_TEST(isInternationalCT_nonCT);
  CPPUNIT_TEST(isInternationalCT_CT);
  CPPUNIT_TEST(isInternationalCT_CT_stopInGR);
  CPPUNIT_TEST(isInternationalCT_CT_DEpoint);
  CPPUNIT_TEST(isInternationalCT_Mirror);
  CPPUNIT_TEST(isDomesticCT_CT);
  CPPUNIT_TEST(isDomesticCT_nonCT);
  CPPUNIT_TEST(isDomesticCT_CT_stop);
  CPPUNIT_TEST(isDomesticCT_CT_internationalPoint);
  CPPUNIT_TEST_SUITE_END();

  Loc* locGB;
  Loc* locDE;
  Loc* locGR;
  Loc* locGR2;

  TaxLocIteratorMock* _locItMock;

  TaxSP9700* tax;

public:
  void setUp()
  {
    _locItMock = new TaxLocIteratorMock();
    tax = new TaxSP9700();
    locGB = new Loc();
    locGB->nation() = "GB";
    locDE = new Loc();
    locDE->nation() = "DE";
    locGR = new Loc();
    locGR->nation() = "GR";
    locGR2 = new Loc();
    locGR2->nation() = "GR";
  }

  void tearDown()
  {
    delete _locItMock;
    delete tax;
    delete locGB;
    delete locDE;
    delete locGR;
  }

  void isInternationalCT_nonCT()
  {
    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGB;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    bool res = tax->isInternationalCT(_locItMock);
    CPPUNIT_ASSERT(!res);
  }

  void isInternationalCT_CT()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGB;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locGB;
    _locItMock->addLoc(item);

    bool res = tax->isInternationalCT(_locItMock);
    CPPUNIT_ASSERT(res);
  }

  void isInternationalCT_CT_stopInGR()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGB;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locGR;
    item.stop = true;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locGB;
    _locItMock->addLoc(item);

    bool res = tax->isInternationalCT(_locItMock);
    CPPUNIT_ASSERT(!res);
  }

  void isInternationalCT_CT_DEpoint()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGB;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locDE;
    _locItMock->addLoc(item);

    item.nextSegNo = 4;
    item.loc = locGB;
    _locItMock->addLoc(item);

    bool res = tax->isInternationalCT(_locItMock);
    CPPUNIT_ASSERT(res);
  }

  void isInternationalCT_Mirror()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGB;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locGB;
    _locItMock->addLoc(item);

    bool res = tax->isInternationalCT(_locItMock);
    CPPUNIT_ASSERT(!res);
  }

  void isDomesticCT_CT()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locGR;
    _locItMock->addLoc(item);

    bool res = tax->isDomesticCT(_locItMock);
    CPPUNIT_ASSERT(res);
  }

  void isDomesticCT_nonCT()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locGR2;
    _locItMock->addLoc(item);

    bool res = tax->isDomesticCT(_locItMock);
    CPPUNIT_ASSERT(!res);
  }

  void isDomesticCT_CT_stop()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.stop = true;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locGR;
    _locItMock->addLoc(item);

    bool res = tax->isDomesticCT(_locItMock);
    CPPUNIT_ASSERT(!res);
  }

  void isDomesticCT_CT_internationalPoint()
  {
    TaxLocItMockItem item;

    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 1;
    item.loc = locGR;
    _locItMock->addLoc(item);

    item.nextSegNo = 2;
    item.loc = locDE;
    _locItMock->addLoc(item);

    item.nextSegNo = 3;
    item.loc = locGR;
    _locItMock->addLoc(item);

    bool res = tax->isDomesticCT(_locItMock);
    CPPUNIT_ASSERT(!res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP9700Test);
}
