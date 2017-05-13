#include "test/include/CppUnitHelperMacros.h"
#include "Rules/AdvanceReservationValidator.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/DiskCache.h"
#include "Diagnostic/DiagCollector.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "Diagnostic/DCFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace std;

namespace tse
{
class ReissueDateLimitAdvanceReservationValidatorMock : public AdvanceReservationValidator
{
public:
  ReissueDateLimitAdvanceReservationValidatorMock(RexBaseTrx& trx,
                                                  DiagCollector* dc,
                                                  log4cxx::LoggerPtr& logger,
                                                  const Indicator& advResTo,
                                                  const Itin* itin,
                                                  const PricingUnit* pu,
                                                  const TravelSeg& firstSegOfFCconst,
                                                  const uint32_t& itemNo)
    : AdvanceReservationValidator(trx, dc, logger, advResTo, itin, pu, firstSegOfFCconst, itemNo)
  {
  }

protected:
  bool
  getUtcOffsetDifference(short& utcOffsetInMinutes, DateTime& fromDT, DateTime& toDT, Loc& toLoc)
  {
    if (_trx.getRequest()->ticketingAgent()->agentLocation()->loc() == "NYC" &&
        toLoc.loc() == "LON")
    {
      utcOffsetInMinutes = -5 * 60;
      return true;
    }
    else if (_trx.getRequest()->ticketingAgent()->agentLocation()->loc() == "NYC" &&
             toLoc.loc() == "WAW")
    {
      utcOffsetInMinutes = -6 * 60;
      return true;
    }
    else if (_trx.getRequest()->ticketingAgent()->agentLocation()->loc() == "NYC" &&
             toLoc.loc() == "MOW")
    {
      utcOffsetInMinutes = -8 * 60;
      return true;
    }
    return false;
  }
};

class AdvanceReservationValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdvanceReservationValidatorTest);
  CPPUNIT_TEST(testGetToDateWhenDepartureOfJourney);
  CPPUNIT_TEST(testGetToDateWhenDepartureOfPU);
  CPPUNIT_TEST(testGetToDateWhenDepartureOfPUAndPUIsEmpty);
  CPPUNIT_TEST(testGetToDateWhenDepartureOfFC);
  CPPUNIT_TEST(testGetToDateWhenBadDateInd);
  CPPUNIT_TEST(testPrintInputData);
  CPPUNIT_TEST(testPrintOutputDates);
  CPPUNIT_TEST(testValidateWhenReissueDateIsBeforeLimitDateAndTimeZone);
  CPPUNIT_TEST(testValidateWhenReissueDateIsAfterLimitDateAndTimeZone);

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
    _rexBaseTrx = _memHandle.create<RefundPricingTrx>();
    _rexBaseTrx->diagnostic().diagnosticType() = Diagnostic333;
    _rexBaseTrx->diagnostic().activate();

    DCFactory* factory = DCFactory::instance();
    _dc = factory->create(*_rexBaseTrx);
    if (_dc != 0)
    {
      _dc->enable(Diagnostic333);
      if (!_dc->isActive())
      {
        _dc = 0;
      }
    }

    _request = _memHandle.create<PricingRequest>();
    _rexBaseTrx->setRequest(_request);
    _agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = _agent;
    Loc* _agentLocation = _memHandle.create<Loc>();
    _agentLocation->loc() = "NYC";
    _agent->agentLocation() = _agentLocation;

    _itin = _memHandle.create<Itin>();

    _rexBaseTrx->itin().push_back(_itin);

    _rexBaseTrx->currentTicketingDT() = DateTime(2009, 3, 2, 11, 30, 0);

    Loc* wawLoc = _memHandle.create<Loc>();
    wawLoc->loc() = "WAW";
    AirSeg* firstSegOfTravel = _memHandle.create<AirSeg>();
    firstSegOfTravel->origin() = wawLoc;
    firstSegOfTravel->departureDT() = DateTime(2009, 3, 1);
    _itin->travelSeg().push_back(firstSegOfTravel);

    _isSoftPass = false;
    _emptyPU = NULL;

    _pu = _memHandle.create<PricingUnit>();
    Loc* lonLoc = _memHandle.create<Loc>();
    lonLoc->loc() = "LON";
    AirSeg* firstSegOfPU = _memHandle.create<AirSeg>();
    firstSegOfPU->origin() = lonLoc;
    const_cast<PricingUnit*>(_pu)->travelSeg().push_back(firstSegOfPU);
    firstSegOfPU->departureDT() = DateTime(2009, 3, 6, 10, 0, 0);

    Loc* mowLoc = _memHandle.create<Loc>();
    mowLoc->loc() = "MOW";
    _firstSegOfFC = _memHandle.create<AirSeg>();
    _firstSegOfFC->origin() = mowLoc;
    _firstSegOfFC->departureDT() = DateTime(2009, 3, 10, 18, 20, 0);

    _advResPeriod = "4";
    _advResUnit = "D";

    _PUvalidator = _memHandle.insert<ReissueDateLimitAdvanceReservationValidatorMock>(
        new ReissueDateLimitAdvanceReservationValidatorMock(
            *_rexBaseTrx,
            _dc,
            _logger,
            AdvanceReservationValidator::ADVRSVN_PRICING_UNIT,
            _itin,
            _pu,
            *_firstSegOfFC,
            5));
  }

  void tearDown()
  {
    if (_dc != 0)
    {
      _dc->flushMsg();

      _dc = 0;
    }
    _memHandle.clear();
  }

  void testGetToDateWhenDepartureOfJourney()
  {
    DateTime toDT;
    Loc toLoc;
    AdvanceReservationValidator Jvalidator(*_rexBaseTrx,
                                           _dc,
                                           _logger,
                                           AdvanceReservationValidator::ADVRSVN_JOURNEY,
                                           _itin,
                                           _pu,
                                           *_firstSegOfFC,
                                           4);

    CPPUNIT_ASSERT(Jvalidator.getToDateAndLoc(toDT, toLoc, _isSoftPass));
    CPPUNIT_ASSERT(!_isSoftPass);
    CPPUNIT_ASSERT_EQUAL(Jvalidator._itin->travelSeg().front()->departureDT(), toDT);
  }

  void testGetToDateWhenDepartureOfPU()
  {
    DateTime toDT;
    Loc toLoc;

    CPPUNIT_ASSERT(_PUvalidator->getToDateAndLoc(toDT, toLoc, _isSoftPass));
    CPPUNIT_ASSERT(!_isSoftPass);
    CPPUNIT_ASSERT_EQUAL(_pu->travelSeg().front()->departureDT(), toDT);
  }

  void testGetToDateWhenDepartureOfPUAndPUIsEmpty()
  {
    DateTime toDT;
    Loc toLoc;
    AdvanceReservationValidator emptyPUValidator(*_rexBaseTrx,
                                                 _dc,
                                                 _logger,
                                                 AdvanceReservationValidator::ADVRSVN_PRICING_UNIT,
                                                 _itin,
                                                 _emptyPU,
                                                 *_firstSegOfFC,
                                                 3);

    CPPUNIT_ASSERT(emptyPUValidator.getToDateAndLoc(toDT, toLoc, _isSoftPass));
    CPPUNIT_ASSERT(_isSoftPass);
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(), toDT);
    CPPUNIT_ASSERT_EQUAL(string("  NO PU INFO: SOFTPASS\n"), emptyPUValidator._dc->str());
  }

  void testGetToDateWhenDepartureOfFC()
  {
    DateTime toDT;
    Loc toLoc;
    AdvanceReservationValidator FCvalidator(*_rexBaseTrx,
                                            _dc,
                                            _logger,
                                            AdvanceReservationValidator::ADVRSVN_FARE_COMPONENT,
                                            _itin,
                                            _pu,
                                            *_firstSegOfFC,
                                            2);

    CPPUNIT_ASSERT(FCvalidator.getToDateAndLoc(toDT, toLoc, _isSoftPass));
    CPPUNIT_ASSERT(!_isSoftPass);
    CPPUNIT_ASSERT_EQUAL(FCvalidator._firstSegOfFC.departureDT(), toDT);
  }

  void testGetToDateWhenBadDateInd()
  {
    DateTime toDT;
    Loc toLoc;
    AdvanceReservationValidator badDateValidator(
        *_rexBaseTrx, _dc, _logger, '$', _itin, _pu, *_firstSegOfFC, 1);

    CPPUNIT_ASSERT(!badDateValidator.getToDateAndLoc(toDT, toLoc, _isSoftPass));
    CPPUNIT_ASSERT(!_isSoftPass);
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(), toDT);
    CPPUNIT_ASSERT_EQUAL(string("ERROR: VOLUNTARY REFUND R3 ITEM NO 1 INCORRECT BYTE 23\n"),
                         badDateValidator._dc->str());
  }

  void testPrintInputData()
  {
    AdvanceReservationValidator FCvalidator(*_rexBaseTrx,
                                            _dc,
                                            _logger,
                                            AdvanceReservationValidator::ADVRSVN_FARE_COMPONENT,
                                            _itin,
                                            _pu,
                                            *_firstSegOfFC,
                                            2);

    FCvalidator.printInputData(_advResPeriod, _advResUnit);

    CPPUNIT_ASSERT_EQUAL(string("\nADVANCE CANCELLATION: 4 DAYS BEFORE DEPARTURE OF FC\n"),
                         FCvalidator._dc->str());
  }

  void testPrintOutputDates()
  {
    DateTime advResLimit = DateTime(2009, 3, 2, 18, 20, 0);
    DateTime& fromDT = _rexBaseTrx->currentTicketingDT();
    AdvanceReservationValidator FCvalidator(*_rexBaseTrx,
                                            _dc,
                                            _logger,
                                            AdvanceReservationValidator::ADVRSVN_FARE_COMPONENT,
                                            _itin,
                                            _pu,
                                            *_firstSegOfFC,
                                            2);

    FCvalidator.printOutputDates(advResLimit, fromDT, fromDT - Minutes(-6 * 60));

    CPPUNIT_ASSERT_EQUAL(string(" REFUND DATE/TIME          : 2009-03-02T11:30:00\n"
                                " REFUND DATE DEP TIME ZONE : 2009-03-02T17:30:00\n"
                                " LIMIT DATE/TIME           : 2009-03-02T18:20:00\n"),
                         FCvalidator._dc->str());
  }

  void testValidateWhenReissueDateIsBeforeLimitDateAndTimeZone()
  {
    _rexBaseTrx->currentTicketingDT() = DateTime(2009, 3, 2, 4, 50, 0);

    CPPUNIT_ASSERT(_PUvalidator->validate(_isSoftPass, _advResPeriod, _advResUnit));
    CPPUNIT_ASSERT_EQUAL(string("\nADVANCE CANCELLATION: 4 DAYS BEFORE DEPARTURE OF PU\n"
                                " REFUND DATE/TIME          : 2009-03-02T04:50:00\n"
                                " REFUND DATE DEP TIME ZONE : 2009-03-02T09:50:00\n"
                                " LIMIT DATE/TIME           : 2009-03-02T10:00:00\n"),
                         _PUvalidator->_dc->str());
  }

  void testValidateWhenReissueDateIsAfterLimitDateAndTimeZone()
  {
    _rexBaseTrx->currentTicketingDT() = DateTime(2009, 3, 2, 5, 10, 0);

    CPPUNIT_ASSERT(!_PUvalidator->validate(_isSoftPass, _advResPeriod, _advResUnit));
    CPPUNIT_ASSERT_EQUAL(string("\nADVANCE CANCELLATION: 4 DAYS BEFORE DEPARTURE OF PU\n"
                                " REFUND DATE/TIME          : 2009-03-02T05:10:00\n"
                                " REFUND DATE DEP TIME ZONE : 2009-03-02T10:10:00\n"
                                " LIMIT DATE/TIME           : 2009-03-02T10:00:00\n"
                                "  FAILED ITEM 5 - ADV CANX DATE NOT MET\n"),
                         _PUvalidator->_dc->str());
  }

protected:
  TestMemHandle _memHandle;
  RexBaseTrx* _rexBaseTrx;
  DiagCollector* _dc;
  static log4cxx::LoggerPtr _logger;
  Itin* _itin;
  AdvanceReservationValidator* _PUvalidator;
  bool _isSoftPass;
  DateTime _itinTravelDate;
  DateTime _fareMarketDepartureDT;
  AirSeg* _firstSegOfFC;
  const PricingUnit* _emptyPU;
  const PricingUnit* _pu;
  ResPeriod _advResPeriod;
  ResUnit _advResUnit;
  PricingRequest* _request;
  Agent* _agent;
  Loc* _agentLocation;
};

log4cxx::LoggerPtr
AdvanceReservationValidatorTest::_logger(log4cxx::Logger::getLogger("atseintl.Rules.Reissue"));

CPPUNIT_TEST_SUITE_REGISTRATION(AdvanceReservationValidatorTest);
}
