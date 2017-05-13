//----------------------------------------------------------------------------
//	File: FareDisplaySortDAOTest.cpp
//
//	Author: Partha Kumar Chakraborti
//  	Created:      04/18/2006
//  	Description:  This is a unit test class for FareDisplaySortDAO.cpp
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "DBAccess/test/FareDisplaySortDAOTest.h"
#include "DBAccess/DeleteList.h"
#include "Common/DateTime.h"
#include "DBAccess/FareDisplaySort.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/Agent.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include <log4cxx/propertyconfigurator.h>
#include "DataModel/FareDisplayOptions.h"

namespace tse
{

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplaySortDAOTest);

// ======================================================
//		class: FareDisplaySortDAOMock
// ======================================================

// -------------------------------------------------------
//		Mock function which overrides base class method
//		for testing individually bypassing some methods in
//		base class too. As most of them are protected
//		that would not have been possible otherwise.
//	-------------------------------------------------------

log4cxx::LoggerPtr
FareDisplaySortDAOMock::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.test.FareDisplaySortDAOMock"));

std::vector<FareDisplaySort*>*
FareDisplaySortDAOMock::create(FareDisplaySortKey key)
{
  std::vector<FareDisplaySort*>* ptr = new std::vector<FareDisplaySort*>;

  // Insert 1st FareDisplaySort
  FareDisplaySort* obj1 = new FareDisplaySort;
  obj1->createDate() = DateTime::localTime().subtractDays(10);
  obj1->effDate() = DateTime::localTime().subtractDays(8);
  obj1->expireDate() = DateTime::localTime().addDays(2);
  obj1->discDate() = DateTime::localTime().addDays(2);
  ptr->push_back(obj1);

  // Insert 2nd FareDisplaySort - Should be filtered by isEffective
  FareDisplaySort* obj2 = new FareDisplaySort;
  obj1->createDate() = DateTime::localTime().subtractDays(10);
  obj1->effDate() = DateTime::localTime().subtractDays(8);
  obj1->expireDate() = DateTime::localTime().subtractDays(4);
  obj1->discDate() = DateTime::localTime().subtractDays(2);
  ptr->push_back(obj2);

  // Insert 3rd FareDisplaySort - Should be filtered by isEffective
  FareDisplaySort* obj3 = new FareDisplaySort;
  obj3->createDate() = DateTime::localTime().subtractDays(10);
  obj3->effDate() = DateTime::localTime().subtractDays(8);
  obj3->expireDate() = DateTime::localTime().subtractDays(4);
  obj3->discDate() = DateTime::localTime().addDays(2);
  ptr->push_back(obj3);

  // Insert 4th FareDisplaySort - Should be filtered by isEffective
  FareDisplaySort* obj4 = new FareDisplaySort;
  obj4->createDate() = DateTime::localTime().subtractDays(10);
  obj4->effDate() = DateTime::localTime().addDays(2);
  obj4->expireDate() = DateTime::localTime().addDays(4);
  obj4->discDate() = DateTime::localTime().subtractDays(3);
  ptr->push_back(obj4);

  // Insert 5th FareDisplaySort
  FareDisplaySort* obj5 = new FareDisplaySort;
  obj5->createDate() = DateTime::localTime().subtractDays(2);
  obj5->effDate() = DateTime::localTime();
  obj5->expireDate() = DateTime::localTime();
  obj5->discDate() = DateTime::localTime().addDays(2);
  ptr->push_back(obj5);

  return ptr;
}

// ======================================================
//		class: FareDisplaySortDAOTest
// ======================================================

log4cxx::LoggerPtr
FareDisplaySortDAOTest::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.test.FareDisplaySortDAOTest"));

// ------------------------------------------------------
// @MethodName  FDHeaderMsgControllerTest::setUp()
// ------------------------------------------------------
void
FareDisplaySortDAOTest::setUp()
{

  LOG4CXX_DEBUG(_logger, "Enter setUp()");

  // log4cxx::PropertyConfigurator::configure("FDHeaderMsgController/log4cxx.properties");

  // ----------------------------------------------------------------------
  //	Creare an object put the reference to it so that everyone can use it.
  // ----------------------------------------------------------------------
  _fdMock = new FareDisplaySortDAOMock();

  LOG4CXX_DEBUG(_logger, "Leaving setUp(),_fdMock:" << _fdMock);
  return;
}

// -------------------------------------------------------------------
// @MethodName  FDHeaderMsgControllerTest::tearDown()
// -----------------------------------------------------------
void
FareDisplaySortDAOTest::tearDown()
{
  LOG4CXX_DEBUG(_logger, "Enter tearDown()");

  // if ( _fdSFCMock )
  delete _fdMock;

  LOG4CXX_DEBUG(_logger, "Leaving tearDown()");

  return;
}

// -------------------------------------------------------------------
// @MethodName  FDHeaderMsgControllerTest::testisMatchLocation()
// -----------------------------------------------------------
void
FareDisplaySortDAOTest::testget()
{
  DeleteList deleteList;
  const std::vector<FareDisplaySort*>& fareDisplaySortList = _fdMock->get(
      deleteList, 'C', "AXES", ' ', "", false);
  CPPUNIT_ASSERT(fareDisplaySortList.size() == 2);
  return;
}
} //tse
