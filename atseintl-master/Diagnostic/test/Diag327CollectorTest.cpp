//-----------------------------------------------------------------------------
//
//  File:     Diag327CollectorTest.cpp
//
//  Author :  Konrad Koch
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag327Collector.h"
#include "DBAccess/Tours.h"

namespace tse
{

class Diag327CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag327CollectorTest);
  CPPUNIT_TEST(testDiagActivated);
  CPPUNIT_TEST(testDiagDeactivated);
  CPPUNIT_TEST(testDisplayHeader);
  CPPUNIT_TEST(testDisplayRecord2Info);
  CPPUNIT_TEST(testDisplayRecord2SegmentsInfo);
  CPPUNIT_TEST(testDisplayRecord2SegmentInfo);
  CPPUNIT_TEST(testDisplayRecord3Info);
  CPPUNIT_TEST(testDisplayStatusMatch);
  CPPUNIT_TEST(testDisplayStatusNotMatch);
  CPPUNIT_TEST_SUITE_END();

private:
  Diag327Collector* _collector;
  Diagnostic* _diagRoot;

  class MockFareMarket : public FareMarket
  {
  public:
    MockFareMarket() : FareMarket()
    {
      _loc1.loc() = "DEN";
      _loc2.loc() = "LON";

      origin() = &_loc1;
      destination() = &_loc2;
    }

  protected:
    Loc _loc1;
    Loc _loc2;
  };

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare() : PaxTypeFare()
    {
      _tariffCrossRefInfo.ruleTariff() = 389;

      _fareInfo.vendor() = "ATP";
      _fareInfo.carrier() = "BA";
      _fareInfo.ruleNumber() = "JP01";
      _fareInfo.fareClass() = "Y";

      _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket, &_tariffCrossRefInfo);

      setFare(&_fare);
      fareMarket() = &_fareMarket;
    }

  protected:
    FareInfo _fareInfo;
    Fare _fare;
    TariffCrossRefInfo _tariffCrossRefInfo;
    MockFareMarket _fareMarket;
  };

  class MockCategoryRuleInfo : public CategoryRuleInfo
  {
  public:
    MockCategoryRuleInfo() : CategoryRuleInfo()
    {
      CategoryRuleItemInfoSet* carRuleItemInfoSet = new CategoryRuleItemInfoSet();
      CategoryRuleItemInfo catRuleItemInfo;
      catRuleItemInfo.setRelationalInd(CategoryRuleItemInfo::THEN);
      catRuleItemInfo.setItemNo(123456);
      catRuleItemInfo.setDirectionality('1');
      catRuleItemInfo.setInOutInd('I');

      _vendorCode = "ATP";
      _tariffNumber = 777;
      _carrierCode = "LH";
      _ruleNumber = "JP02";

      carRuleItemInfoSet->push_back(catRuleItemInfo);
      addItemInfoSetNosync(carRuleItemInfoSet);
      sequenceNumber() = 5000100;
    }
  };

  class MockTours : public Tours
  {
  public:
    MockTours() : Tours()
    {
      itemNo() = 123580;
      overrideDateTblItemNo() = 123456789;
      tourNo() = "JORDAN07";
    }
  };

public:
  void setUp()
  {
    _diagRoot = new Diagnostic(Diagnostic327);
    _diagRoot->activate();
    _collector = new Diag327Collector(*_diagRoot);
    _collector->enable(Diagnostic327);
  }

  void tearDown()
  {
    delete _collector;
    delete _diagRoot;
  }

  void testDiagActivated()
  {
    MockPaxTypeFare paxTypeFare;
    MockCategoryRuleInfo catRuleInfo;
    MockTours tours;

    _diagRoot->activate();
    _collector->collect(RuleItem::PricingUnitPhase, paxTypeFare, catRuleInfo, tours);

    CPPUNIT_ASSERT(_collector->str().length());
  }

  void testDiagDeactivated()
  {
    MockPaxTypeFare paxTypeFare;
    MockCategoryRuleInfo catRuleInfo;
    MockTours tours;

    _collector->deActivate();
    _collector->collect(RuleItem::PricingUnitPhase, paxTypeFare, catRuleInfo, tours);

    CPPUNIT_ASSERT(_collector->str().length() == 0);
  }

  void testDisplayHeader()
  {
    MockPaxTypeFare paxTypeFare;
    MockCategoryRuleInfo catRuleInfo;

    _collector->displayHeader(RuleItem::PricingUnitPhase, paxTypeFare, catRuleInfo);

    std::string expectedResult("***************************************************************\n"
                               "*               ATSE CATEGORY 27 TOURS DIAGNOSTICS            *\n"
                               "***************************************************************\n"
                               "PHASE: PRICING UNIT\n"
                               "DEN LON Y         R2: FARERULE : ATP  777  LH JP02 \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayRecord2Info()
  {
    MockCategoryRuleInfo catRuleInfo;

    _collector->displayRecord2Info(catRuleInfo);

    std::string expectedResult("R2: SEQ NBR 005000100\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayRecord2SegmentsInfo()
  {
    MockCategoryRuleInfo catRuleInfo;

    _collector->displayRecord2SegmentsInfo(catRuleInfo);

    std::string expectedResult("R2S: THEN 123456       DIR-1  I/O-I\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayRecord2SegmentInfo()
  {
    CategoryRuleItemInfo ruleItemInfo;
    ruleItemInfo.setRelationalInd(CategoryRuleItemInfo::THEN);
    ruleItemInfo.setItemNo(123456);
    ruleItemInfo.setDirectionality('1');
    ruleItemInfo.setInOutInd('I');

    _collector->displayRecord2SegmentInfo(ruleItemInfo);

    std::string expectedResult("R2S: THEN 123456       DIR-1  I/O-I\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayRecord3Info()
  {
    MockTours tours;

    _collector->displayRecord3Info(tours);

    std::string expectedResult("R3 : 123580            T994-123456789   TOUR CODE:JORDAN07\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayStatusMatch()
  {
    _collector->displayStatus(PASS);

    std::string expectedResult("STATUS: MATCH\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayStatusNotMatch()
  {
    _collector->displayStatus(FAIL);

    std::string expectedResult("STATUS: NOT MATCH\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag327CollectorTest);

} // namespace
