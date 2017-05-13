// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"

#include "Common/PassengerUtil.h"
#include "DomainDataObjects/Passenger.h"
#include "DataModel/Common/CodeIO.h"

namespace tax
{

class PassengerUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PassengerUtilTest);

  CPPUNIT_TEST(testGetCode);
  CPPUNIT_TEST(testGetBirthDate);
  CPPUNIT_TEST(testGetStateCode);
  CPPUNIT_TEST(testGetNationWithStateAsNation);
  CPPUNIT_TEST(testGetNationality);
  CPPUNIT_TEST(testGetResidency);
  CPPUNIT_TEST(testGetEmployment);
  CPPUNIT_TEST(testGetBlank);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _passenger = new Passenger();
    _passenger->_code = "ABC";
    _passenger->_birthDate = type::Date(2013, 10, 18);
    _passenger->_passengerStatus._nationality = "PL";
    _passenger->_passengerStatus._residency = "FRA";
    _passenger->_passengerStatus._employment = "RU";

    _passengerUtil = new PassengerUtil(*_passenger);
  }

  void tearDown()
  {
    delete _passengerUtil;
    delete _passenger;
  }

  void testGetCode()
  {
    CPPUNIT_ASSERT_EQUAL(type::PassengerCode("ABC"), _passengerUtil->getCode());
  }

  void testGetBirthDate()
  {
    CPPUNIT_ASSERT_EQUAL(type::Date(2013, 10, 18), _passengerUtil->getBirthDate());
  }

  void testGetStateCode()
  {
    _passenger->_stateCode = "VI";
    CPPUNIT_ASSERT_EQUAL(type::StateProvinceCode("VI"), _passengerUtil->getStateCode());
  }

  void testGetNationWithStateAsNation()
  {
    _passenger->_stateCode = "VI";
    type::PassengerStatusTag tag = type::PassengerStatusTag::Employee;
    CPPUNIT_ASSERT_EQUAL(type::StateProvinceCode("US"), _passengerUtil->getNation(tag).asString());
  }

  void testGetNationality()
  {
    type::PassengerStatusTag tag = type::PassengerStatusTag::National;
    CPPUNIT_ASSERT_EQUAL(type::StateProvinceCode("PL"), _passengerUtil->getNation(tag).asString());
  }

  void testGetResidency()
  {
    CPPUNIT_ASSERT_EQUAL(type::StateProvinceCode("FRA"), _passengerUtil->getLocZoneText().asString());
  }

  void testGetEmployment()
  {
    type::PassengerStatusTag tag = type::PassengerStatusTag::Employee;
    CPPUNIT_ASSERT_EQUAL(type::StateProvinceCode("RU"), _passengerUtil->getNation(tag).asString());
  }

  void testGetBlank()
  {
    type::PassengerStatusTag tag = type::PassengerStatusTag::Blank;
    CPPUNIT_ASSERT_EQUAL(type::StateProvinceCode(""), _passengerUtil->getNation(tag).asString());
  }

private:
  Passenger* _passenger;
  PassengerUtil* _passengerUtil;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PassengerUtilTest);

} // namespace tax
