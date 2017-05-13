// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/FilterPredicates.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Services/RulesRecord.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/PaymentRuleData.h"

#include <memory>

namespace tax
{

class FilterPredicatesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FilterPredicatesTest);

  CPPUNIT_TEST(validBusinessRuleDatePredicateTest);
  CPPUNIT_TEST(validRulesRecordDatePredicateTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void validBusinessRuleDatePredicateTest()
  {
    type::Timestamp ticketingDate(type::Date(2015, 10, 9), type::Time(14, 24));

    RulesRecord rulesRecord1;
    rulesRecord1.effDate = type::Date(2015, 1, 1);                        // ok
    rulesRecord1.discDate = type::Date(2015, 12, 31);                     // ok
    rulesRecord1.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                               type::Time(23, 59));
    std::shared_ptr<BusinessRulesContainer> container1 =
        std::make_shared<BusinessRulesContainer>(rulesRecord1, type::ProcessingGroup::Itinerary);

    CPPUNIT_ASSERT_EQUAL(validBusinessRuleDatePredicate(container1, ticketingDate),
                         true);


    RulesRecord rulesRecord2;
    rulesRecord2.effDate = type::Date(2016, 1, 1);                        // too late
    rulesRecord2.discDate = type::Date(2015, 12, 31);                     // ok
    rulesRecord2.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                               type::Time(23, 59));
    std::shared_ptr<BusinessRulesContainer> container2 =
        std::make_shared<BusinessRulesContainer>(rulesRecord2, type::ProcessingGroup::Itinerary);

    CPPUNIT_ASSERT_EQUAL(validBusinessRuleDatePredicate(container2, ticketingDate),
                         false);


    RulesRecord rulesRecord3;
    rulesRecord3.effDate = type::Date(2015, 1, 1);                        // ok
    rulesRecord3.discDate = type::Date(2014, 12, 31);                     // too early
    rulesRecord3.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                               type::Time(23, 59));
    std::shared_ptr<BusinessRulesContainer> container3 =
        std::make_shared<BusinessRulesContainer>(rulesRecord3, type::ProcessingGroup::Itinerary);

    CPPUNIT_ASSERT_EQUAL(validBusinessRuleDatePredicate(container3, ticketingDate),
                         false);


    RulesRecord rulesRecord4;
    rulesRecord4.effDate = type::Date(2015, 1, 1);                        // ok
    rulesRecord4.discDate = type::Date(2015, 12, 31);                     // ok
    rulesRecord4.expiredDate = type::Timestamp(type::Date(2014, 12, 31),  // too early
                                               type::Time(23, 59));
    std::shared_ptr<BusinessRulesContainer> container4 =
        std::make_shared<BusinessRulesContainer>(rulesRecord4, type::ProcessingGroup::Itinerary);

    CPPUNIT_ASSERT_EQUAL(validBusinessRuleDatePredicate(container4, ticketingDate),
                         false);

    RulesRecord rulesRecord5;
    rulesRecord5.effDate = type::Date::blank_date();                      // blank
    rulesRecord5.discDate = type::Date(2015, 12, 31);                     // ok
    rulesRecord5.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                               type::Time(23, 59));
    std::shared_ptr<BusinessRulesContainer> container5 =
            std::make_shared<BusinessRulesContainer>(rulesRecord5, type::ProcessingGroup::Itinerary);
    bool result = true;
    try
    {
      validBusinessRuleDatePredicate(container5, ticketingDate);
    }
    catch (const std::domain_error&)
    {
      result = false;
    }
    CPPUNIT_ASSERT_EQUAL(result, false);


    RulesRecord rulesRecord6;
    rulesRecord6.effDate = type::Date(2015, 1, 1);                        // ok
    rulesRecord6.discDate = type::Date::invalid_date();                   // invalid
    rulesRecord6.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                               type::Time(23, 59));
    std::shared_ptr<BusinessRulesContainer> container6 =
            std::make_shared<BusinessRulesContainer>(rulesRecord6, type::ProcessingGroup::Itinerary);
    result = true;
    try
    {
      validBusinessRuleDatePredicate(container6, ticketingDate);
    }
    catch (const std::domain_error&)
    {
      result = false;
    }
    CPPUNIT_ASSERT_EQUAL(result, false);

    RulesRecord rulesRecord7;
    rulesRecord7.effDate = type::Date(2015, 1, 1);                        // ok
    rulesRecord7.discDate = type::Date(2015, 12, 31);                     // ok
    rulesRecord7.expiredDate = type::Timestamp(type::Date(2015, 12, 31),
                                               type::Time(23, 59));
    rulesRecord7.expiredDate.date() = type::Date::invalid_date(); // invalid date, must not be set in the constructor

    std::shared_ptr<BusinessRulesContainer> container7 =
            std::make_shared<BusinessRulesContainer>(rulesRecord7, type::ProcessingGroup::Itinerary);
    result = true;
    try
    {
      validBusinessRuleDatePredicate(container7, ticketingDate);
    }
    catch (const std::domain_error&)
    {
      result = false;
    }
    CPPUNIT_ASSERT_EQUAL(result, false);
  }

  void validRulesRecordDatePredicateTest()
    {
      type::Timestamp ticketingDate(type::Date(2015, 10, 9), type::Time(14, 24));

      RulesRecord rulesRecord1;
      rulesRecord1.effDate = type::Date(2015, 1, 1);                        // ok
      rulesRecord1.discDate = type::Date(2015, 12, 31);                     // ok
      rulesRecord1.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                                 type::Time(23, 59));
      CPPUNIT_ASSERT_EQUAL(validRulesRecordDatePredicate(rulesRecord1, ticketingDate),
                           true);


      RulesRecord rulesRecord2;
      rulesRecord2.effDate = type::Date(2016, 1, 1);                        // too late
      rulesRecord2.discDate = type::Date(2015, 12, 31);                     // ok
      rulesRecord2.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                                 type::Time(23, 59));
      CPPUNIT_ASSERT_EQUAL(validRulesRecordDatePredicate(rulesRecord2, ticketingDate),
                           false);


      RulesRecord rulesRecord3;
      rulesRecord3.effDate = type::Date(2015, 1, 1);                        // ok
      rulesRecord3.discDate = type::Date(2014, 12, 31);                     // too early
      rulesRecord3.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                                 type::Time(23, 59));
      CPPUNIT_ASSERT_EQUAL(validRulesRecordDatePredicate(rulesRecord3, ticketingDate),
                           false);


      RulesRecord rulesRecord4;
      rulesRecord4.effDate = type::Date(2015, 1, 1);                        // ok
      rulesRecord4.discDate = type::Date(2015, 12, 31);                     // ok
      rulesRecord4.expiredDate = type::Timestamp(type::Date(2014, 12, 31),  // too early
                                                 type::Time(23, 59));
      CPPUNIT_ASSERT_EQUAL(validRulesRecordDatePredicate(rulesRecord4, ticketingDate),
                           false);

      RulesRecord rulesRecord5;
      rulesRecord5.effDate = type::Date::blank_date();                      // blank
      rulesRecord5.discDate = type::Date(2015, 12, 31);                     // ok
      rulesRecord5.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                                 type::Time(23, 59));
      bool result = true;
      try
      {
        validRulesRecordDatePredicate(rulesRecord5, ticketingDate);
      }
      catch (const std::domain_error&)
      {
        result = false;
      }
      CPPUNIT_ASSERT_EQUAL(result, false);

      RulesRecord rulesRecord6;
      rulesRecord6.effDate = type::Date(2015, 1, 1);                        // ok
      rulesRecord6.discDate = type::Date::invalid_date();                   // invalid
      rulesRecord6.expiredDate = type::Timestamp(type::Date(2015, 12, 31),  // ok
                                                 type::Time(23, 59));
      result = true;
      try
      {
        validRulesRecordDatePredicate(rulesRecord6, ticketingDate);
      }
      catch (const std::domain_error&)
      {
        result = false;
      }
      CPPUNIT_ASSERT_EQUAL(result, false);

      RulesRecord rulesRecord7;
      rulesRecord7.effDate = type::Date(2015, 1, 1);                        // ok
      rulesRecord7.discDate = type::Date(2015, 12, 31);                     // ok
      rulesRecord7.expiredDate = type::Timestamp(type::Date(2015, 12, 31),
                                                 type::Time(23, 59));
      rulesRecord7.expiredDate.date() = type::Date::invalid_date(); // invalid date, must not be set in the constructor
      result = true;
      try
      {
        validRulesRecordDatePredicate(rulesRecord7, ticketingDate);
      }
      catch (const std::domain_error&)
      {
        result = false;
      }
      CPPUNIT_ASSERT_EQUAL(result, false);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FilterPredicatesTest);

} // namespace tax
