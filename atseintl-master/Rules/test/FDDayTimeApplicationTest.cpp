#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include "test/include/TestMemHandle.h"
#include "Rules/FDDayTimeApplication.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/FareDisplayPref.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "Diagnostic/DiagManager.h"

namespace tse
{

class FDDayTimeApplicationTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(FDDayTimeApplicationTest);
  CPPUNIT_TEST(testProcessOWSingleDateFare_Match);
  CPPUNIT_TEST(testProcessOWSingleDateFare_FailDOW);
  CPPUNIT_TEST(testProcessOWSingleDateFare_false_NoFDUserPref);
  CPPUNIT_TEST(testProcessOWSingleDateFare_false_ReturnDate);
  CPPUNIT_TEST(testProcessOWSingleDateFare_false_DateRange);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fdApp = _memHandle.create<FDDayTimeApplication>();
    _dta = _memHandle.create<DayTimeAppInfo>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _opt = _memHandle.create<FareDisplayOptions>();
    _req = _memHandle.create<FareDisplayRequest>();
    _prf = _memHandle.create<FareDisplayPref>();

    _ptf = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _fi = _memHandle.create<FareInfo>();
    _seg = _memHandle.create<AirSeg>();
    _diag = _memHandle.insert(new DiagManager(*_trx));

    _trx->setOptions(_opt);
    _trx->setRequest(_req);
    _opt->fareDisplayPref() = _prf;
    _ptf->setFare(_fare);
    _fare->setFareInfo(_fi);
    _fi->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    _fdApp->_itemInfo = _dta;
    _dta->dow() = "123"; // mon-wed
    _prf->applyDOWvalidationToOWFares() = 'Y';

    _seg->earliestDepartureDT() = DateTime(2009, 11, 23);
    _seg->latestDepartureDT() = DateTime(2009, 11, 23);
  }
  void tearDown() { _memHandle.clear(); }

  void testProcessOWSingleDateFare_Match()
  {
    CPPUNIT_ASSERT(_fdApp->processOWSingleDateFare(_trx, *_ptf, _seg, *_diag, _retVal));
    CPPUNIT_ASSERT(_retVal == PASS);
  }
  void testProcessOWSingleDateFare_FailDOW()
  {
    _dta->dow() = "23";
    CPPUNIT_ASSERT(_fdApp->processOWSingleDateFare(_trx, *_ptf, _seg, *_diag, _retVal));
    CPPUNIT_ASSERT(_retVal == FAIL);
  }
  void testProcessOWSingleDateFare_false_NoFDUserPref()
  {
    _prf->applyDOWvalidationToOWFares() = 'N';
    CPPUNIT_ASSERT(!_fdApp->processOWSingleDateFare(_trx, *_ptf, _seg, *_diag, _retVal));
  }
  void testProcessOWSingleDateFare_false_ReturnDate()
  {
    _req->returnDate() = DateTime(2009, 12, 6);
    CPPUNIT_ASSERT(!_fdApp->processOWSingleDateFare(_trx, *_ptf, _seg, *_diag, _retVal));
  }
  void testProcessOWSingleDateFare_false_DateRange()
  {
    _seg->latestDepartureDT() = DateTime(2009, 11, 24);
    CPPUNIT_ASSERT(!_fdApp->processOWSingleDateFare(_trx, *_ptf, _seg, *_diag, _retVal));
  }

protected:
  FDDayTimeApplication* _fdApp;
  DayTimeAppInfo* _dta;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _opt;
  FareDisplayPref* _prf;
  FareDisplayRequest* _req;
  PaxTypeFare* _ptf;
  Fare* _fare;
  FareInfo* _fi;
  AirSeg* _seg;
  DiagManager* _diag;
  Record3ReturnTypes _retVal;

  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDDayTimeApplicationTest);
}
