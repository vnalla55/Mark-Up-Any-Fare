//----------------------------------------------------------------------------
//	File: FareDisplaySortDAOTest.h
//
//	Author: Partha Kumar Chakraborti
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

#ifndef FAREDISPLAYSORTDAO_TEST_H
#define FAREDISPLAYSORTDAO_TEST_H

#include "Common/Logger.h"
#include "DBAccess/FareDisplaySortDAO.h"

#include <cppunit/extensions/HelperMacros.h>

namespace tse
{
class FareDisplaySortDAOMock : public FareDisplaySortDAO
{
public:
  FareDisplaySortDAOMock() {}
  virtual ~FareDisplaySortDAOMock() {}

protected:
  std::vector<FareDisplaySort*>* create(FareDisplaySortKey key);

private:
  static log4cxx::LoggerPtr _logger;
};

class FareDisplaySortDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplaySortDAOTest);

  CPPUNIT_TEST(testget);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();
  void testget();

private:
  static log4cxx::LoggerPtr _logger;
  friend class FareDisplaySortDAOMock;
  friend class FareDisplaySortDAO;

  FareDisplaySortDAO* _fdMock;
};
}
#endif
