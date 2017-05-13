// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxPercentageUS.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "DataModel/TaxTrx.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "DataModel/FarePath.h"

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"

#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag804Collector.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "DBAccess/TaxAkHiFactor.h"
#include "DBAccess/TaxSpecConfigReg.h"

#include "test/DBAccessMock/DataHandleMock.h"

#include <memory>

using namespace std;

namespace tse
{
const std::string EXPECTED = "Expected";
const std::string RESULT = "Result";

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle() { _memHandle.clear(); }

  const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& taxCode, const DateTime& date)
  {
    std::vector<TaxCodeReg*>& ret = *_memHandle.create<std::vector<TaxCodeReg*> >();
    if (taxCode == "US1")
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml", true));
    else if (taxCode == "US2")
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml", true));
    else if (taxCode == "USD")
    {
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml", true));
      ret.front()->specConfigName() = "D";
    }
    else if (taxCode == "USU")
    {
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml", true));
      ret.front()->specConfigName() = "U";
    }
    else if (taxCode == "USN")
    {
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml", true));
      ret.front()->specConfigName() = "N";
    }
    else
      return DataHandleMock::getTaxCode(taxCode, date);

    return ret;
  }

  const std::vector<TaxAkHiFactor*>& getTaxAkHiFactor(const LocCode& key, const DateTime& date)
  {
    std::vector<TaxAkHiFactor*>& ret = *_memHandle.create<std::vector<TaxAkHiFactor*> >();
    if (key == "ANC")
    {
      ret.push_back(_memHandle.create<TaxAkHiFactor>());
      ret.front()->zoneAPercent() = 0.044;
      ret.front()->zoneBPercent() = 0.0467;
      ret.front()->zoneCPercent() = 0.0499;
      ret.front()->zoneDPercent() = 0.054;
      ret.front()->hawaiiPercent() = 0.0244;
    }
    else if (key == "DFW")
    {
      ret.push_back(_memHandle.create<TaxAkHiFactor>());
      ret.front()->zoneAPercent() = 0.0353;
      ret.front()->zoneBPercent() = 0.038;
      ret.front()->zoneCPercent() = 0.0419;
      ret.front()->zoneDPercent() = 0.0454;
      ret.front()->hawaiiPercent() = 0.0391;
    }
    else if (key == "NYC")
    {
      // nothing in DB
    }
    else
      return DataHandleMock::getTaxAkHiFactor(key, date);
    return ret;
  }

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    Loc* ret = _memHandle.create<Loc>();
    ret->loc() = locCode;
    ret->nation() = "US";
    if (locCode == "ANC")
      ret->state() = "USAK";
    else if (locCode == "HNL")
      ret->state() = "USHI";
    else
      ret->state() = "TX";

    return ret;
  }

  std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name)
  {
    std::vector<TaxSpecConfigReg*>& ret = *_memHandle.create<std::vector<TaxSpecConfigReg*> >();
    TaxSpecConfigReg* specConfigReg = new TaxSpecConfigReg();
    TaxSpecConfigReg::TaxSpecConfigRegSeq* specConfigRegSeq =
        new TaxSpecConfigReg::TaxSpecConfigRegSeq();
    specConfigReg->effDate() = DateTime(2000, 1, 20);
    specConfigReg->discDate() = DateTime(2030, 1, 20);
    specConfigRegSeq->paramName() = "HALFTAXROUND";
    specConfigRegSeq->paramValue() = name;
    specConfigReg->seqs().push_back(specConfigRegSeq);
    ret.push_back(specConfigReg);
    return ret;
  }
};
}
class TaxPercentageUSTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPercentageUSTest);

  CPPUNIT_TEST(instanceTest);
  CPPUNIT_TEST(buildTest);
  CPPUNIT_TEST(continentalDisplayTest);
  CPPUNIT_TEST(factorsDisplayTest);
  CPPUNIT_TEST(taxContinentalBuildTest);
  CPPUNIT_TEST(taxFactorsBuildTest);
  CPPUNIT_TEST(taxRoundTest_default);
  CPPUNIT_TEST(taxRoundTest_down);
  CPPUNIT_TEST(taxRoundTest_up);

  CPPUNIT_TEST_SUITE_END();

public:
  class TaxPercentagaUSFake : public TaxPercentageUS
  {
  public:
    TaxPercentagaUSFake(TaxRequest& r) : TaxPercentageUS(r) {}
    void taxFactorsBuild() { TaxPercentageUS::taxFactorsBuild(); }
    void taxContinentalBuild() { TaxPercentageUS::taxContinentalBuild(); }
  };

  void setUp() { _memHandle.create<MyDataHandle>(); }
  void tearDown() { _memHandle.clear(); }

  void instanceTest()
  {

    TaxRequest req;
    std::unique_ptr<TaxPercentageUS> ptr(new TaxPercentageUS(req));

    CPPUNIT_ASSERT(ptr);
  }
  void buildTest()
  {
    std::string display, expected;

    TaxRequest req;
    TaxPercentageUS percentage(req);
    percentage.build();
    display = percentage.display();

    CPPUNIT_ASSERT(display == EMPTY_STRING());

    TaxRequest req1;
    req1.amtType() = 'T';
    req1.tripType() = 'X';
    req1.fareAmt() = -1; // wrong fare amount
    req1.loc1() = "HNL";
    req1.loc2() = "ANC";

    TaxPercentageUS percentage1(req1);
    percentage1.build();
    display = percentage1.display();

    CPPUNIT_ASSERT(display == EMPTY_STRING());

    TaxRequest req2;
    req2.amtType() = 'T';
    req2.tripType() = 'Z';
    req2.fareAmt() = 100; // wrong fare amount
    req2.loc1() = "HNL";
    req2.loc2() = "ANC";

    TaxPercentageUS percentage2(req2);
    percentage2.build();
    display = percentage2.display();

    CPPUNIT_ASSERT(display == EMPTY_STRING());

    TaxRequest req3;
    req3.amtType() = 'T';
    req3.tripType() = 'X';
    req3.fareAmt() = 100;
    req3.loc1() = "DFW";
    req3.loc2() = "NYC";

    TaxPercentageUS percentage3(req3);
    percentage3.build();
    display = percentage3.display();
    expected = "NO TAX FACTOR FOUND FOR CITY PAIR - MODIFY AND REENTER";

    CPPUNIT_ASSERT(display.size());

    TaxRequest req4;
    req4.amtType() = 'T';
    req4.fareAmt() = 100;

    TaxPercentageUS percentage4(req4);
    percentage4.build();
    display = percentage4.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req5;
    req5.amtType() = 'B';
    req5.fareAmt() = 1000;

    TaxPercentageUS percentage5(req5);
    percentage5.build();
    display = percentage5.display();

    CPPUNIT_ASSERT(display.size());
  }

  void continentalDisplayTest()
  {
    std::string display;

    TaxRequest req;
    req.amtType() = 'B';
    req.fareAmt() = 2021;

    TaxPercentagaUSFake fake(req);
    fake.taxContinentalBuild();
    display = fake.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req1;
    req1.amtType() = 'T';
    req1.fareAmt() = 2000;

    TaxPercentagaUSFake fake1(req1);
    fake1.taxContinentalBuild();
    display = fake1.display();

    CPPUNIT_ASSERT(display.size());
  }

  void taxContinentalBuildTest()
  {
    TaxRequest req1;
    req1.amtType() = 'T';
    req1.fareAmt() = 2000;
    req1.loc1() = "HNL";
    req1.loc2() = "ANC";
    req1.tripType() = 'R';

    TaxPercentageUS tp(req1);
    tp.taxContinentalBuild();

    CPPUNIT_ASSERT_EQUAL(std::string("1860.47/139.53"), tp._output);
  }

  void taxFactorsBuildTest()
  {
    TaxRequest req1;
    req1.amtType() = 'T';
    req1.fareAmt() = 2000;
    req1.loc1() = "HNL";
    req1.loc2() = "ANC";
    req1.tripType() = 'R';

    TaxPercentageUS tp(req1);
    tp.taxFactorsBuild();

    CPPUNIT_ASSERT_EQUAL(std::string("1938.89/13.80/47.31 - TAX PCT. 2.44"), tp._output);
  }

  void taxRoundTest_default()
  {
    TaxTrx trx;

    DateTime dt;
    Itin itin;
    TaxRequest taxRequest;
    TaxResponse taxResponse;

    trx.setRequest(&taxRequest);
    trx.itin().push_back(&itin);

    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    const std::vector<TaxCodeReg*>& taxCodeReg =
        trx.dataHandle().getTaxCode("US2", trx.getRequest()->ticketingDT());

    TaxPercentageUS tp(taxRequest);
    MoneyAmount taxUS2 = tp.taxRound(trx, *(taxCodeReg.front()), 8.05);
    CPPUNIT_ASSERT_EQUAL(8.1, taxUS2);
  }

  void taxRoundTest_down()
  {
    TaxTrx trx;

    DateTime dt;
    Itin itin;
    TaxRequest taxRequest;
    TaxResponse taxResponse;

    trx.setRequest(&taxRequest);
    trx.itin().push_back(&itin);
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    const std::vector<TaxCodeReg*>& taxCodeReg =
        trx.dataHandle().getTaxCode("USD", trx.getRequest()->ticketingDT());

    TaxPercentageUS tp(taxRequest);
    MoneyAmount taxUS2 = tp.taxRound(trx, *(taxCodeReg.front()), 8.05);
    CPPUNIT_ASSERT_EQUAL(8.0, taxUS2);
  }

  void taxRoundTest_up()
  {
    TaxTrx trx;

    DateTime dt;
    Itin itin;
    TaxRequest taxRequest;
    TaxResponse taxResponse;

    trx.setRequest(&taxRequest);
    trx.itin().push_back(&itin);
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    const std::vector<TaxCodeReg*>& taxCodeReg =
        trx.dataHandle().getTaxCode("USU", trx.getRequest()->ticketingDT());

    TaxPercentageUS tp(taxRequest);
    MoneyAmount taxUS2 = tp.taxRound(trx, *(taxCodeReg.front()), 8.05);
    CPPUNIT_ASSERT_EQUAL(8.1, taxUS2);
  }

  void factorsDisplayTest()
  {
    std::string display, expected;

    TaxRequest req1;
    req1.amtType() = 'T';
    req1.tripType() = 'X';
    req1.fareAmt() = 100;
    req1.loc1() = "DFW";
    req1.loc2() = "KRK";

    TaxPercentagaUSFake fake1(req1);
    fake1.taxFactorsBuild();
    display = fake1.display();
    expected = "NO TAX FACTOR FOUND FOR CITY PAIR - MODIFY AND REENTER";

    CPPUNIT_ASSERT(display == expected);

    TaxRequest req2;
    req2.amtType() = 'B';
    req2.tripType() = 'X';
    req2.fareAmt() = 100;
    req2.loc1() = "HNL";
    req2.loc2() = "ANC";

    TaxPercentagaUSFake fake2(req2);
    fake2.taxFactorsBuild();
    display = fake2.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req3;
    req3.amtType() = 'T';
    req3.tripType() = 'X';
    req3.fareAmt() = 100;
    req3.loc1() = "HNL";
    req3.loc2() = "ANC";

    TaxPercentagaUSFake fake3(req3);
    fake3.taxFactorsBuild();
    display = fake3.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req4;
    req4.amtType() = 'T';
    req4.tripType() = 'X';
    req4.fareAmt() = 100;
    req4.loc1() = "ANC";
    req4.loc2() = "HNL";

    TaxPercentagaUSFake fake4(req4);
    fake4.taxFactorsBuild();
    display = fake4.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req5;
    req5.amtType() = 'T';
    req5.tripType() = 'X';
    req5.fareAmt() = 100;
    req5.loc1() = "DFW";
    req5.loc2() = "HNL";

    TaxPercentagaUSFake fake5(req5);
    fake5.taxFactorsBuild();
    display = fake5.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req6;
    req6.amtType() = 'T';
    req6.tripType() = 'X';
    req6.fareAmt() = 100;
    req6.loc1() = "NYC";
    req6.loc2() = "HNL";

    TaxPercentagaUSFake fake6(req6);
    fake6.taxFactorsBuild();
    display = fake6.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req7;
    req7.amtType() = 'T';
    req7.tripType() = 'X';
    req7.fareAmt() = 100;
    req7.loc1() = "NYC";
    req7.loc2() = "ANC";

    TaxPercentagaUSFake fake7(req7);
    fake7.taxFactorsBuild();
    display = fake7.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req8;
    req8.amtType() = 'T';
    req8.tripType() = 'R';
    req8.fareAmt() = 100;
    req8.loc1() = "ANC";
    req8.loc2() = "NYC";

    TaxPercentagaUSFake fake8(req8);
    fake8.taxFactorsBuild();
    display = fake8.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req9;
    req9.amtType() = 'B';
    req9.tripType() = 'R';
    req9.fareAmt() = 1000;
    req9.loc1() = "ANC";
    req9.loc2() = "HNL";

    TaxPercentagaUSFake fake9(req9);
    fake9.taxFactorsBuild();
    display = fake9.display();

    CPPUNIT_ASSERT(display.size());

    TaxRequest req10;
    req10.amtType() = 'B';
    req10.tripType() = 'R';
    req10.fareAmt() = 1000;
    req10.loc1() = "ANC";
    req10.loc2() = "DFW";

    TaxPercentagaUSFake fake10(req10);
    fake10.taxFactorsBuild();
    display = fake10.display();

    CPPUNIT_ASSERT(display.size());
  }

private:
  void log(const std::string& label, const std::string& txt) const
  {
    std::cout << std::endl << label << ":" << std::endl << "Lenght: " << txt.size() << std::endl
              << txt << std::endl;
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxPercentageUSTest);
}
