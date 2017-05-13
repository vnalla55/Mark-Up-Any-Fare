// Copyright 2016 Sabre

#include "Rules/VoluntaryChanges.h"

#include "Common/DateTime.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Waiver.h"

#include "Rules/test/TestCommon.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <limits>
#include <memory>
#include <sstream>
#include <vector>

namespace tse
{
class VoluntaryChangesOverride : public VoluntaryChanges
{
  friend class VoluntaryChangesTest;

protected:
  TestMemHandle _memHandle;
  std::vector<FareCompInfo*>* getAllFc() { return &_allFc; }
  bool isInternationalItin() { return true; }
  const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo, const DateTime& applDate)
  {
    return _carrierAppl;
  };

public:
  std::vector<FareCompInfo*> _allFc;
  std::vector<CarrierApplicationInfo*> _carrierAppl;

  VoluntaryChangesOverride(RexPricingTrx& trx)
    : VoluntaryChanges(trx, 0, 0), _validR3(true), _validT988(true) {};
  VoluntaryChangesOverride(RexPricingTrx& trx, PricingUnit* pu)
    : VoluntaryChanges(trx, 0, pu), _validR3(true), _validT988(true) {};
  VoluntaryChangesOverride(RexPricingTrx& trx, Itin* itin, PricingUnit* pu)
    : VoluntaryChanges(trx, itin, pu), _validR3(true), _validT988(true) {};

  virtual bool matchR3(PaxTypeFare& paxTypeFare, const VoluntaryChangesInfo& vcRec3)
  {
    return _validR3;
  }
  virtual bool matchT988(PaxTypeFare& ptf,
                         const VoluntaryChangesInfo& vcRec3,
                         bool overridenData,
                         ReissueTable& t988Validator,
                         const std::set<int>* prevalidatedSeqTab988 = 0)
  {
    return _validT988;
  }

  bool _validR3;
  bool _validT988;
};

class MockFareCompInfo : public FareCompInfo
{
  friend class VoluntaryChangesTest;
};

class VoluntaryChangesTest : public CppUnit::TestFixture
{
private:
  static constexpr int ONE_PASSING_RULE_ITEM = 1;
  static constexpr int ONE_DISCONTINUED_RULE_ITEM = 2;
  static constexpr int MULTIPLE_PASSING_RULE_ITEMS = 3;
  static constexpr int MULTIPLE_DISCONTINUED_RULE_ITEMS = 4;
  static constexpr int AT_LEAST_ONE_PASSING_RULE_ITEM = 5;

  class MockedDataHandle : public DataHandleMock
  {
  public:
    const std::vector<DateOverrideRuleItem*>&
    getDateOverrideRuleItem(const VendorCode& vendor, int itemNo, const DateTime& applDate)
    {
      std::vector<DateOverrideRuleItem*>* retVectorPtr(new std::vector<DateOverrideRuleItem*>);
      _deleteList.adopt(retVectorPtr);

      auto createAndAddDORItemToVec = [&](const DateTime& dateEffDate, const DateTime& dateDiscDate)
      {
        DateOverrideRuleItem* newItem = new DateOverrideRuleItem();
        newItem->tvlEffDate() = dateEffDate;
        newItem->tktEffDate() = dateEffDate;
        newItem->tvlDiscDate() = dateDiscDate;
        newItem->tktDiscDate() = dateDiscDate;
        _deleteList.adopt(newItem);
        retVectorPtr->push_back(newItem);
      };

      switch (itemNo)
      {
      case ONE_PASSING_RULE_ITEM:
        createAndAddDORItemToVec(applDate, DateTime(2137, 1, 1));
        break;
      case ONE_DISCONTINUED_RULE_ITEM:
        createAndAddDORItemToVec(applDate.addMonths(1), applDate);
        break;
      case MULTIPLE_PASSING_RULE_ITEMS:
        createAndAddDORItemToVec(applDate, applDate.nextDay());
        createAndAddDORItemToVec(applDate, DateTime(2137, 1, 1));
        createAndAddDORItemToVec(applDate, DateTime(2237, 1, 1));
        break;
      case MULTIPLE_DISCONTINUED_RULE_ITEMS:
        createAndAddDORItemToVec(applDate, DateTime(1999, 1, 1));
        createAndAddDORItemToVec(applDate, applDate.subtractMonths(1));
        createAndAddDORItemToVec(applDate.addMonths(1), applDate);
        break;
      case AT_LEAST_ONE_PASSING_RULE_ITEM:
        createAndAddDORItemToVec(applDate, DateTime(1999, 1, 1));
        createAndAddDORItemToVec(applDate, applDate.subtractMonths(1));
        createAndAddDORItemToVec(applDate.addWeeks(1), applDate);
        createAndAddDORItemToVec(applDate, DateTime(2337, 1, 1));
        break;
      }

      return *retVectorPtr;
    }
  };

  CPPUNIT_TEST_SUITE(VoluntaryChangesTest);
  CPPUNIT_TEST(testAllTravelBeforeTicketValidityDate);
  CPPUNIT_TEST(testTravelNotBeforeTicketValidityDate);
  CPPUNIT_TEST(testTicketValidityForCalendar);
  CPPUNIT_TEST(testTicketValidityCheckNotRequired);
  CPPUNIT_TEST(testTvlDateBeforeTvlEffDate);
  CPPUNIT_TEST(testTvlDateOnTvlEffDate);
  CPPUNIT_TEST(testTvlDateAfterTvlEffDateBeforeTvlDiscDate);
  CPPUNIT_TEST(testTvlDateAfterTvlDiscDate);
  CPPUNIT_TEST(testTktDateBeforeTktEffDate);
  CPPUNIT_TEST(testTktDateOnTktEffDate);
  CPPUNIT_TEST(testTktDateAfterTktEffDateBeforeTktDiscDate);
  CPPUNIT_TEST(testTktDateAfterTktDiscDate);

  CPPUNIT_TEST(testOverrideDateTableEmptyTblItemNo);
  CPPUNIT_TEST(testOverrideDateTableNoneDORItemsMatched);
  CPPUNIT_TEST(testOverrideDateTableMatchingDate);
  CPPUNIT_TEST(testOverrideDateTableNotMatchingDate);

  CPPUNIT_TEST(testNoChgPermitted);
  CPPUNIT_TEST(testReissuePermittedOnceOrMore);
  CPPUNIT_TEST(testNoReissueLimit);
  CPPUNIT_TEST(testMatchTktLimitIndBlankNoRestriction);
  CPPUNIT_TEST(testMatchTktLimitBeforePU);
  CPPUNIT_TEST(testNoMatchTktLimitBeforePU);
  CPPUNIT_TEST(testMatchNoValidTktLimitBeforePU);
  CPPUNIT_TEST(testMatchTktLimitAfterPU);
  CPPUNIT_TEST(testNoMatchTktLimitAfterPU);
  CPPUNIT_TEST(testMatchTktLimitBeforeJourney);
  CPPUNIT_TEST(testNoMatchTktLimitBeforeJourney);
  CPPUNIT_TEST(testMatchNoValidTktLimitBeforeJourney);
  CPPUNIT_TEST(testMatchTktLimitAfterJourney);
  CPPUNIT_TEST(testNoMatchTktLimitAfterJourney);
  CPPUNIT_TEST(testMatchNoValidTktLimitAfterJourney);

  CPPUNIT_TEST(testShouldOverrideWithIntlFc_NoFcs);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_OneFc);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComN);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYNoItemNo1);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYNoItemNo2);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYNoItemNo3);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYItemNoTheSameCarrier);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYItemNoDiffrentCarrierCaEmpty);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYItemNoDollarCarrier);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYItemNoCarrierInCaAllowed1);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYItemNoCarrierInCaAllowed2);
  CPPUNIT_TEST(testShouldOverrideWithIntlFc_DomIntlComYItemNoCarrierInCaDenied);

  CPPUNIT_TEST(testMatchSameAirportBlankPass);
  CPPUNIT_TEST(testMatchSameAirportXPass);
  CPPUNIT_TEST(testMatchSameAirportXFullyFlownPass);
  CPPUNIT_TEST(testMatchSameAirportXUnflownArunkUnflownAirPass);
  CPPUNIT_TEST(testMatchSameAirportXFlownArunkFlownAirPass);
  CPPUNIT_TEST(testMatchSameAirportXFail);
  CPPUNIT_TEST(testMatchSameAirportXFailOnSecondTravelSegment);
  CPPUNIT_TEST(testMatchSameAirportPassOnInternational);
  CPPUNIT_TEST(testResultFalseWith1Pass1FailByPrevReissued);
  CPPUNIT_TEST(testResultFalseWith1Fail1PassByPrevReissued);
  CPPUNIT_TEST(testResultTrueWithAllFailByPrevReissued);
  CPPUNIT_TEST(testResultFalseWithAllPassByPrevReissued);

  CPPUNIT_TEST(testCheckSegmentsChangeFalse);
  CPPUNIT_TEST(testCheckSegmentsChangeTrue);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MockedDataHandle>();

    _trx = _memHandle.create<RexPricingTrx>();
    _trx->diagnostic().diagnosticType() = Diagnostic331;
    _trx->diagnostic().activate();

    _farePath = _memHandle.create<FarePath>();

    _exchangePU = _memHandle.create<PricingUnit>();
    _secondExchangePU = _memHandle.create<PricingUnit>();

    _farePath->pricingUnit().push_back(_exchangePU);
    _farePath->pricingUnit().push_back(_secondExchangePU);

    AirSeg* firstSeg = _trx->dataHandle().create<AirSeg>();
    firstSeg->departureDT() = DateTime(2007, 3, 1, 10, 0, 0);
    firstSeg->unflown() = true;
    AirSeg* secondSeg = _trx->dataHandle().create<AirSeg>();
    secondSeg->departureDT() = DateTime(2007, 10, 1, 11, 59, 0);
    secondSeg->unflown() = true;

    _exchangePU->travelSeg().push_back(firstSeg);
    _exchangePU->travelSeg().push_back(secondSeg);

    _vcRec3 = _memHandle.create<VoluntaryChangesInfo>();
    _vcRec3->itemNo() = 1234;

    _trx->setOriginalTktIssueDT() = DateTime::localTime();
    _excPT = _memHandle.create<PaxType>();
    _trx->exchangePaxType() = _excPT;
    _carrier = "AA";

    _dummyItin = _memHandle.create<Itin>();

    _newItin = _memHandle.create<Itin>();
    _newItin->travelSeg().push_back(firstSeg);
    _exchangeItin = _memHandle.create<ExcItin>();
    _trx->exchangeItin().push_back(_exchangeItin);
    _trx->itin().push_back(_newItin);

    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->fareMarket() = _memHandle.create<FareMarket>();
    _paxTypeFare->fareMarket()->travelSeg().push_back(firstSeg);
    _paxTypeFare->fareMarket()->travelSeg().push_back(secondSeg);

    FareCompInfo* fc = _memHandle.create<FareCompInfo>();
    fc->fareMarket() = _paxTypeFare->fareMarket();
    _exchangeItin->fareComponent().push_back(fc);
    _paxTypeFare->fareMarket()->fareCompInfo() = fc;
    _exchangeItin->farePath().push_back(_farePath);

    _vcRuleController = _memHandle.insert<VoluntaryChangesOverride>(
        new VoluntaryChangesOverride(*_trx, _exchangeItin, _exchangePU));

    _ptf = _memHandle.create<PaxTypeFare>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testAllTravelBeforeTicketValidityDate()
  {
    AirSeg firstSeg;
    firstSeg.departureDT() = DateTime(2007, 9, 30);
    _newItin->travelSeg().push_back(&firstSeg);

    _exchangeItin->setTktValidityDate(2007, 10, 1);
    _exchangeItin->travelSeg().push_back(&firstSeg);
    _vcRec3->tktValidityInd() = VoluntaryChanges::TICKET_VALIDITY_DATE_CHECK_REQUIRED;

    CPPUNIT_ASSERT_EQUAL(true, _vcRuleController->matchTicketValidity(*_vcRec3));
  }

  void testTravelNotBeforeTicketValidityDate()
  {
    AirSeg firstSeg;
    firstSeg.departureDT() = DateTime(2007, 3, 1);
    AirSeg secondSeg;
    secondSeg.departureDT() = DateTime(2007, 10, 1);
    _newItin->travelSeg().clear();
    _newItin->travelSeg().push_back(&firstSeg);
    _newItin->travelSeg().push_back(&secondSeg);

    _vcRuleController->_lastSegment.firstDate = DateTime(2007, 10, 1);
    _vcRuleController->_lastSegment.lastDate = DateTime(2007, 10, 1);

    _exchangeItin->setTktValidityDate(2007, 10, 1);
    _exchangeItin->travelSeg().push_back(&firstSeg);
    _vcRec3->tktValidityInd() = VoluntaryChanges::TICKET_VALIDITY_DATE_CHECK_REQUIRED;
    CPPUNIT_ASSERT_EQUAL(false, _vcRuleController->matchTicketValidity(*_vcRec3));
  }

  void testTicketValidityForCalendar()
  {
    PricingTrx::OriginDestination ond;
    ond.travelDate = DateTime(2007, 10, 14);
    ond.calDaysAfter = 3;
    ond.calDaysBefore = 3;
    _trx->originDest().push_back(ond);
    _trx->diagnostic().deActivate();

    _vcRuleController->_lastSegment = {DateTime(2007, 10, 11), DateTime(2007, 10, 17)};
    _vcRuleController->_isEXSCalendar = true;
    _vcRuleController->_r3DateValidationResults =
        std::make_unique<ExchShopCalendar::R3ValidationResult>(
            _trx->orgDest, *_dummyItin, *_dummyItin, *_memHandle.create<DataHandle>());
    _vcRuleController->_dc = nullptr;

    _exchangeItin->setTktValidityDate(2007, 10, 16);
    _vcRec3->tktValidityInd() = VoluntaryChanges::TICKET_VALIDITY_DATE_CHECK_REQUIRED;
    CPPUNIT_ASSERT_EQUAL(true, _vcRuleController->matchTicketValidity(*_vcRec3));

    ExchShopCalendar::DateRange validityPeriod =
                                      _vcRuleController->_r3DateValidationResults->getDateRange();

    CPPUNIT_ASSERT_EQUAL(std::string("11-10-2007"), validityPeriod.firstDate.dateToString(DateFormat::DDMMYYYY,"-"));
    CPPUNIT_ASSERT_EQUAL(std::string("15-10-2007"), validityPeriod.lastDate.dateToString(DateFormat::DDMMYYYY,"-"));
  }

  void testTicketValidityCheckNotRequired()
  {
    _vcRec3->tktValidityInd() = BLANK;
    CPPUNIT_ASSERT(VoluntaryChanges::TICKET_VALIDITY_DATE_CHECK_REQUIRED !=
                   _vcRec3->tktValidityInd());
    CPPUNIT_ASSERT(_vcRuleController->matchTicketValidity(*_vcRec3));
  }

  void testTvlDateBeforeTvlEffDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tvlEffDate() = DateTime(2007, 5, 16);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _vcRuleController->matchOverrideDateRule(dorItem, DateTime(2007, 5, 15), DateTime()));
  }

  void testTvlDateOnTvlEffDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tvlEffDate() = DateTime(2007, 5, 16);
    CPPUNIT_ASSERT_EQUAL(
        true, _vcRuleController->matchOverrideDateRule(dorItem, DateTime(2007, 5, 16), DateTime()));
  }

  void testTvlDateAfterTvlEffDateBeforeTvlDiscDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tvlEffDate() = DateTime(2007, 5, 16);
    dorItem.tvlDiscDate() = DateTime(2007, 8, 30);
    CPPUNIT_ASSERT_EQUAL(
        true, _vcRuleController->matchOverrideDateRule(dorItem, DateTime(2007, 5, 17), DateTime()));
  }

  void testTvlDateAfterTvlDiscDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tvlDiscDate() = DateTime(2007, 8, 30);
    CPPUNIT_ASSERT_EQUAL(
        false, _vcRuleController->matchOverrideDateRule(dorItem, DateTime(2007, 9, 1), DateTime()));
  }

  void testTktDateBeforeTktEffDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tktEffDate() = DateTime(2007, 5, 16);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _vcRuleController->matchOverrideDateRule(dorItem, DateTime(), DateTime(2007, 5, 15)));
  }

  void testTktDateOnTktEffDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tktEffDate() = DateTime(2007, 5, 16);
    CPPUNIT_ASSERT_EQUAL(
        true, _vcRuleController->matchOverrideDateRule(dorItem, DateTime(), DateTime(2007, 5, 16)));
  }

  void testTktDateAfterTktEffDateBeforeTktDiscDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tktEffDate() = DateTime(2007, 5, 16);
    dorItem.tktDiscDate() = DateTime(2007, 8, 30);
    CPPUNIT_ASSERT_EQUAL(
        true, _vcRuleController->matchOverrideDateRule(dorItem, DateTime(), DateTime(2007, 5, 17)));
  }

  void testTktDateAfterTktDiscDate()
  {
    DateOverrideRuleItem dorItem;
    dorItem.tktDiscDate() = DateTime(2007, 8, 30);
    CPPUNIT_ASSERT_EQUAL(
        false, _vcRuleController->matchOverrideDateRule(dorItem, DateTime(), DateTime(2007, 9, 1)));
  }

  void testOverrideDateTableEmptyTblItemNo()
  {
    _vcRec3->overrideDateTblItemNo() = 0;

    CPPUNIT_ASSERT(_vcRuleController->matchOverrideDateTable(*_vcRec3, DateTime(), DateTime()));
  }

  void testOverrideDateTableNoneDORItemsMatched()
  {
    _vcRec3->overrideDateTblItemNo() = std::numeric_limits<int>::max();

    CPPUNIT_ASSERT(_vcRuleController->matchOverrideDateTable(*_vcRec3, DateTime(), DateTime()));
  }

  void testOverrideDateTableMatchingDate()
  {
    DateTime date = DateTime(2016, 8, 8);
    _trx->currentTicketingDT() = date;

    std::array<int, 3> matchingDORItemsTags = {
        ONE_PASSING_RULE_ITEM, MULTIPLE_PASSING_RULE_ITEMS, AT_LEAST_ONE_PASSING_RULE_ITEM};

    for (const auto dorItemTag : matchingDORItemsTags)
    {
      _vcRec3->overrideDateTblItemNo() = dorItemTag;
      CPPUNIT_ASSERT(_vcRuleController->matchOverrideDateTable(*_vcRec3, date, date));
    }
  }

  void testOverrideDateTableNotMatchingDate()
  {
    DateTime date = DateTime(2016, 8, 8);
    _trx->currentTicketingDT() = date;
    std::array<int, 2> notMatchingDORItemsTags = {ONE_DISCONTINUED_RULE_ITEM,
                                                  MULTIPLE_DISCONTINUED_RULE_ITEMS};

    for (const auto dorItemTag : notMatchingDORItemsTags)
    {
      _vcRec3->overrideDateTblItemNo() = dorItemTag;
      CPPUNIT_ASSERT(!_vcRuleController->matchOverrideDateTable(*_vcRec3, date, date));
    }
  }

  void testNoChgPermitted()
  {
    _newItin->travelSeg().clear();
    _vcRec3->changeInd() = VoluntaryChanges::NOT_PERMITTED;

    FareCompInfo* fcInfo =
        _trx->exchangeItin().front()->findFareCompInfo(_paxTypeFare->fareMarket());
    CPPUNIT_ASSERT(0 != fcInfo);

    setNewItinChanged(_paxTypeFare->fareMarket());
    CPPUNIT_ASSERT(false ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    setNewItinInventoryChanged(_paxTypeFare->fareMarket());
    CPPUNIT_ASSERT(false ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    setNewItinNotChanged(_paxTypeFare->fareMarket());
    CPPUNIT_ASSERT(true ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    _vcRec3->changeInd() = VoluntaryChanges::NOT_APPLY;
  }

  void testReissuePermittedOnceOrMore()
  {
    _vcRec3->changeInd() = '1';

    setReissuedNum(0);
    FareCompInfo* fcInfo =
        _trx->exchangeItin().front()->findFareCompInfo(_paxTypeFare->fareMarket());
    CPPUNIT_ASSERT(0 != fcInfo);

    CPPUNIT_ASSERT(true ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    setReissuedNum(1);

    CPPUNIT_ASSERT(false ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    _vcRec3->changeInd() = '2';

    setReissuedNum(0);
    CPPUNIT_ASSERT(true ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    setReissuedNum(1);
    CPPUNIT_ASSERT(false ==
                   _vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    _vcRec3->changeInd() = VoluntaryChanges::NOT_APPLY;
  }

  void testNoReissueLimit()
  {
    _vcRec3->changeInd() = VoluntaryChanges::NOT_APPLY;

    setReissuedNum(0);
    FareCompInfo* fcInfo =
        _trx->exchangeItin().front()->findFareCompInfo(_paxTypeFare->fareMarket());
    CPPUNIT_ASSERT(0 != fcInfo);

    CPPUNIT_ASSERT(_vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));

    setReissuedNum(1);
    CPPUNIT_ASSERT(_vcRuleController->chkNumOfReissue(*_paxTypeFare->fareMarket(), *_vcRec3));
  }

  void setReissuedNum(const uint16_t numOfReissue)
  {
    if (numOfReissue == 0)
    {
      _trx->lastTktReIssueDT() = DateTime::emptyDate();
      return;
    }

    // numOfReissue is not a factor now
    _trx->setOriginalTktIssueDT() = DateTime(2007, 3, 11, 10, 0, 0);
    _trx->lastTktReIssueDT() = _trx->setOriginalTktIssueDT().addDays(2);
  }

  void setNewItinChanged(FareMarket* excFM)
  {
    excFM->travelSeg().front()->changeStatus() = TravelSeg::CHANGED;
  }

  void setNewItinInventoryChanged(FareMarket* excFM)
  {
    excFM->travelSeg().front()->changeStatus() = TravelSeg::INVENTORYCHANGED;
  }

  void setNewItinNotChanged(FareMarket* excFM)
  {
    std::vector<TravelSeg*>::iterator tvlSegI = excFM->travelSeg().begin();
    const std::vector<TravelSeg*>::iterator tvlSegIEnd = excFM->travelSeg().end();

    for (; tvlSegI != tvlSegIEnd; tvlSegI++)
    {
      (*tvlSegI)->changeStatus() = TravelSeg::UNCHANGED;
    }
  }

  void testMatchTktLimitIndBlankNoRestriction()
  {
    _vcRec3->tktTimeLimit() = ' ';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchTktLimitBeforePU()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'D';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testNoMatchTktLimitBeforePU()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 17, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'D';
    CPPUNIT_ASSERT(!_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchNoValidTktLimitBeforePU()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime::emptyDate();

    _vcRec3->tktTimeLimit() = 'D';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchTktLimitAfterPU()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 17, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'E';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testNoMatchTktLimitAfterPU()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'E';
    CPPUNIT_ASSERT(!_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchNoValidTktLimitAfterPU()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime::emptyDate();

    _vcRec3->tktTimeLimit() = 'E';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchTktLimitBeforeJourney()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 19, 23, 59, 0);
    _secondExchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'B';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testNoMatchTktLimitBeforeJourney()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);
    _secondExchangePU->latestTktDT() = DateTime(2003, 12, 17, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'B';
    CPPUNIT_ASSERT(!_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchNoValidTktLimitBeforeJourney()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);
    _secondExchangePU->latestTktDT() = DateTime::emptyDate();

    _vcRec3->tktTimeLimit() = 'B';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchTktLimitAfterJourney()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);
    _secondExchangePU->latestTktDT() = DateTime(2003, 12, 17, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'A';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testNoMatchTktLimitAfterJourney()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime(2003, 12, 19, 23, 59, 0);
    _secondExchangePU->latestTktDT() = DateTime(2003, 12, 18, 23, 59, 0);

    _vcRec3->tktTimeLimit() = 'A';
    CPPUNIT_ASSERT(!_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void testMatchNoValidTktLimitAfterJourney()
  {
    _trx->currentTicketingDT() = DateTime(2003, 12, 18, 0, 0, 0);
    _exchangePU->latestTktDT() = DateTime::emptyDate();
    _secondExchangePU->latestTktDT() = DateTime::emptyDate();

    _vcRec3->tktTimeLimit() = 'A';
    CPPUNIT_ASSERT(_vcRuleController->matchTktTimeLimit(*_vcRec3));
  }

  void setUpSameAirport(LocCode agentCity,
                        LocCode origAirport,
                        Indicator sameAirport,
                        bool firstSegFlown = true,
                        TravelSegType firstSegType = Air,
                        bool secondSegFlown = true,
                        TravelSegType secondSegType = Air,
                        GeoTravelType geoTravelType = GeoTravelType::Domestic)
  {
    RexPricingRequest* _rexPricingRequest = _memHandle.create<RexPricingRequest>();
    Agent* _agent = _memHandle.create<Agent>();
    _agent->agentCity() = agentCity;
    _rexPricingRequest->prevTicketIssueAgent() = _agent;
    _rexPricingRequest->ticketingAgent() = _agent;
    _vcRuleController->_trx.setRequest(_rexPricingRequest);
    _vcRec3->sameAirport() = sameAirport;

    TravelSeg* travelSeg;
    AirSeg* airSeg = _trx->dataHandle().create<AirSeg>();
    travelSeg = airSeg;
    travelSeg->unflown() = !firstSegFlown;
    travelSeg->segmentType() = firstSegType;
    travelSeg->boardMultiCity() = origAirport;
    travelSeg->origAirport() = origAirport;
    if (firstSegFlown)
      travelSeg->changeStatus() = TravelSeg::UNCHANGED;
    else
      travelSeg->changeStatus() = TravelSeg::CHANGED;

    TravelSeg* travelSeg2;
    AirSeg* airSeg2 = _trx->dataHandle().create<AirSeg>();
    travelSeg2 = airSeg2;
    travelSeg2->unflown() = !secondSegFlown;
    travelSeg2->segmentType() = secondSegType;
    travelSeg2->boardMultiCity() = origAirport;
    travelSeg2->origAirport() = origAirport;
    if (secondSegFlown)
      travelSeg2->changeStatus() = TravelSeg::UNCHANGED;
    else
      travelSeg2->changeStatus() = TravelSeg::CHANGED;

    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back(travelSeg);
    travelSegs.push_back(travelSeg2);
    fareMarket.travelSeg() = travelSegs;
    fareMarket.geoTravelType() = geoTravelType;
    _fareComponent.fareMarket() = &fareMarket;
  }

  void testMatchSameAirportBlankPass()
  {
    _vcRec3->sameAirport() = ' ';
    CPPUNIT_ASSERT(_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportXPass()
  {
    setUpSameAirport("DFW", "DFW", 'X', false);
    CPPUNIT_ASSERT(_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportXFullyFlownPass()
  {
    setUpSameAirport("DFW", "DFW", 'X');
    CPPUNIT_ASSERT(_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportXUnflownArunkUnflownAirPass()
  {
    setUpSameAirport("DFW", "DFW", 'X', false, Arunk, false, Air);
    CPPUNIT_ASSERT(_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportXFlownArunkFlownAirPass()
  {
    setUpSameAirport("DFW", "DFW", 'X', true, Arunk, true, Air);
    CPPUNIT_ASSERT(_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportXFail()
  {
    setUpSameAirport("DFW", "NYC", 'X', false, Air);
    CPPUNIT_ASSERT(!_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportXFailOnSecondTravelSegment()
  {
    setUpSameAirport("DFW", "NYC", 'X', true, Air, false, Air);
    CPPUNIT_ASSERT(!_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  void testMatchSameAirportPassOnInternational()
  {
    setUpSameAirport("DFW", "NYC", 'X', true, Air, false, Air, GeoTravelType::International);
    CPPUNIT_ASSERT(_vcRuleController->matchSameAirport(_fareComponent, *_vcRec3));
  }

  FareCompInfo* addFcToAllFcVector(GeoTravelType type, CarrierCode carrier)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->geoTravelType() = type;
    fm->governingCarrier() = carrier;

    FareCompInfo* fc = _memHandle.create<FareCompInfo>();
    fc->fareMarket() = fm;
    _vcRuleController->_allFc.push_back(fc);
    fc->fareCompNumber() = _vcRuleController->_allFc.size();

    return fc;
  }

  void clearAllFcVector() {}

  void addCaToCarrierApplVector(CarrierCode carrier, Indicator ind)
  {
    CarrierApplicationInfo* ca = _memHandle.create<CarrierApplicationInfo>();
    ca->carrier() = carrier;
    ca->applInd() = ind;
    _vcRuleController->_carrierAppl.push_back(ca);
  }

  void clearCarrierApplVector() { _vcRuleController->_carrierAppl.clear(); }

  void prepareR3ForDomIntlCom(Indicator domIntlComb, uint32_t itemNo)
  {
    _vcRec3->domesticIntlComb() = domIntlComb;
    _vcRec3->carrierApplTblItemNo() = itemNo;
    _vcRec3->reissueTblItemNo() = 1;
  }

  void testShouldOverrideWithIntlFc(FareCompInfo* fcDom, FareCompInfo* fcIntl)
  {
    bool ret = _vcRuleController->shouldOverrideWithIntlFc(fcDom, *_vcRec3);
    CPPUNIT_ASSERT_EQUAL(true, ret);
    CPPUNIT_ASSERT_EQUAL((size_t)1, fcDom->overridingFcs().size());

    FareCompInfo::OverridingIntlFcData* od = fcDom->findOverridingData(_vcRec3);
    CPPUNIT_ASSERT(od != 0);
    CPPUNIT_ASSERT_EQUAL(fcIntl->fareCompNumber(), fcDom->getOverridingFc(od));

    FareCompInfo::SkippedValidationsSet* svs = fcDom->getSkippedValidationsSet(od);
    CPPUNIT_ASSERT(svs != 0);
    // CPPUNIT_ASSERT(svs->isSet(FareCompInfo::svNumOfReissue)); //will be tested in iter26
  }

  void testShouldOverrideWithIntlFc_DomIntlComYNoItemNo1()
  {
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "AA");
    addFcToAllFcVector(GeoTravelType::International, "BA");

    prepareR3ForDomIntlCom('Y', 0);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYNoItemNo2()
  {
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "AA");
    addFcToAllFcVector(GeoTravelType::International, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");

    prepareR3ForDomIntlCom('Y', 0);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYNoItemNo3()
  {
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "AA");
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "BA");

    prepareR3ForDomIntlCom('Y', 0);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_NoFcs()
  {
    prepareR3ForDomIntlCom('Y', 0);
    CPPUNIT_ASSERT(!_vcRuleController->shouldOverrideWithIntlFc(0, *_vcRec3));
  }

  void testShouldOverrideWithIntlFc_OneFc()
  {
    FareCompInfo* fc = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    prepareR3ForDomIntlCom('Y', 0);
    CPPUNIT_ASSERT(!_vcRuleController->shouldOverrideWithIntlFc(fc, *_vcRec3));
    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComN()
  {
    FareCompInfo* fc = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    prepareR3ForDomIntlCom('N', 0);
    CPPUNIT_ASSERT(!_vcRuleController->shouldOverrideWithIntlFc(fc, *_vcRec3));
    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYItemNoTheSameCarrier()
  {
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "AA");
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "BA");

    prepareR3ForDomIntlCom('Y', 1000);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYItemNoDiffrentCarrierCaEmpty()
  {
    FareCompInfo* fc = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "AA");
    addFcToAllFcVector(GeoTravelType::International, "LH");

    prepareR3ForDomIntlCom('Y', 1000);
    CPPUNIT_ASSERT(!_vcRuleController->shouldOverrideWithIntlFc(fc, *_vcRec3));

    clearAllFcVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYItemNoDollarCarrier()
  {
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "AA");
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "LH");

    addCaToCarrierApplVector("AA", 'X');
    addCaToCarrierApplVector("BA", ' ');
    addCaToCarrierApplVector("LO", 'X');
    addCaToCarrierApplVector(DOLLAR_CARRIER, ' ');

    prepareR3ForDomIntlCom('Y', 1000);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
    clearCarrierApplVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYItemNoCarrierInCaAllowed1()
  {
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::International, "AA");
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "LH");

    addCaToCarrierApplVector("AA", 'X');
    addCaToCarrierApplVector("LH", ' ');
    addCaToCarrierApplVector("LO", 'X');
    addCaToCarrierApplVector("CA", ' ');

    prepareR3ForDomIntlCom('Y', 1000);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
    clearCarrierApplVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYItemNoCarrierInCaAllowed2()
  {
    FareCompInfo* fcIntl = addFcToAllFcVector(GeoTravelType::International, "LH");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::International, "AA");
    FareCompInfo* fcDom = addFcToAllFcVector(GeoTravelType::Domestic, "LO");

    addCaToCarrierApplVector("AA", 'X');
    addCaToCarrierApplVector("LH", ' ');
    addCaToCarrierApplVector("LO", 'X');
    addCaToCarrierApplVector("CA", ' ');

    prepareR3ForDomIntlCom('Y', 1000);
    testShouldOverrideWithIntlFc(fcDom, fcIntl);

    clearAllFcVector();
    clearCarrierApplVector();
  }

  void testShouldOverrideWithIntlFc_DomIntlComYItemNoCarrierInCaDenied()
  {
    FareCompInfo* fc = addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::Domestic, "BA");
    addFcToAllFcVector(GeoTravelType::International, "AA");
    addFcToAllFcVector(GeoTravelType::International, "LH");

    addCaToCarrierApplVector("AA", 'X');
    addCaToCarrierApplVector("NA", ' ');
    addCaToCarrierApplVector("LH", 'X');
    addCaToCarrierApplVector("CA", ' ');

    prepareR3ForDomIntlCom('Y', 1000);
    CPPUNIT_ASSERT(!_vcRuleController->shouldOverrideWithIntlFc(fc, *_vcRec3));

    clearAllFcVector();
    clearCarrierApplVector();
  }

  void testResultFalseWith1Pass1FailByPrevReissued()
  {
    FareCompInfo fcInfo;
    PaxTypeFare fare;
    VoluntaryChanges cat31Val(*_trx, 0, 0);
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == false);
    cat31Val._failByPrevReissued = true;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == false);
  }

  void testResultFalseWith1Fail1PassByPrevReissued()
  {
    FareCompInfo fcInfo;
    PaxTypeFare fare;
    VoluntaryChanges cat31Val(*_trx, 0, 0);
    cat31Val._failByPrevReissued = true;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == true);
    cat31Val._failByPrevReissued = false;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == false);
  }

  void testResultTrueWithAllFailByPrevReissued()
  {
    FareCompInfo fcInfo;
    PaxTypeFare fare;
    VoluntaryChanges cat31Val(*_trx, 0, 0);
    cat31Val._failByPrevReissued = true;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == true);
    cat31Val._failByPrevReissued = true;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == true);
  }

  void testResultFalseWithAllPassByPrevReissued()
  {
    FareCompInfo fcInfo;
    PaxTypeFare fare;
    VoluntaryChanges cat31Val(*_trx, 0, 0);
    cat31Val._failByPrevReissued = false;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == false);
    cat31Val._failByPrevReissued = false;
    cat31Val.updateFailByPrevReissued(fcInfo, &fare);
    CPPUNIT_ASSERT(fcInfo.failByPrevReissued().find(&fare)->second == false);
  }

  void testCheckSegmentsChangeFalse()
  {
    _vcRec3->changeInd() = VoluntaryChanges::NOT_PERMITTED;
    _paxTypeFare->fareMarket()->travelSeg().back()->changeStatus() = TravelSeg::UNCHANGED;
    _paxTypeFare->fareMarket()->travelSeg().front()->changeStatus() = TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT(_vcRuleController->segsNotChanged(*_paxTypeFare->fareMarket(), *_vcRec3));
  }
  void testCheckSegmentsChangeTrue()
  {
    _newItin->travelSeg().clear();
    _vcRec3->changeInd() = VoluntaryChanges::NOT_PERMITTED;
    _paxTypeFare->fareMarket()->travelSeg().back()->changeStatus() = TravelSeg::UNCHANGED;
    _paxTypeFare->fareMarket()->travelSeg().front()->changeStatus() = TravelSeg::CHANGED;

    CPPUNIT_ASSERT(!_vcRuleController->segsNotChanged(*_paxTypeFare->fareMarket(), *_vcRec3));
  }

protected:
  RexPricingTrx* _trx;
  PricingRequest* _request;
  VoluntaryChangesOverride* _vcRuleController;
  VoluntaryChangesInfo* _vcRec3;
  PaxType* _excPT;
  CarrierCode _carrier;
  Itin* _newItin, *_dummyItin;
  FarePath* _farePath;
  PricingUnit* _exchangePU;
  PricingUnit* _secondExchangePU;
  ExcItin* _exchangeItin;
  PaxTypeFare* _paxTypeFare;
  PaxTypeFare* _ptf;
  RexPricingRequest* _rexPricingRequest;
  Agent* _agent;
  FareMarket fareMarket;
  FareCompInfo _fareComponent;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(VoluntaryChangesTest);
}
