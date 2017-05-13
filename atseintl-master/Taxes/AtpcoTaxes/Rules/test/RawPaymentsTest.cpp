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
#include "Rules/RawPayments.h"
#include "Rules/PaymentRuleData.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tax
{

class RawPaymentsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RawPaymentsTest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(reserved0);
  CPPUNIT_TEST(reserved);
  CPPUNIT_TEST(some_content);
  CPPUNIT_TEST(move_ctor);
  CPPUNIT_TEST_SUITE_END();

  const TaxName ta, tb, tc;
  PaymentRuleData prData;

public:
  RawPaymentsTest()
    : prData(0, type::TicketedPointTag::MatchTicketedPointsOnly, TaxableUnitTagSet::none(), 0,
             type::CurrencyCode(UninitializedCode), type::TaxAppliesToTagInd::Blank)
    {}

  void setUp()
  {
  }

  void tearDown()
  {
  }

  void empty()
  {
    RawPayments rawPayments;
    CPPUNIT_ASSERT (rawPayments.empty());
    CPPUNIT_ASSERT (rawPayments.size() == 0);
    CPPUNIT_ASSERT (rawPayments.begin() == rawPayments.end());
  }

  void reserved0()
  {
    RawPayments rawPayments;
    rawPayments.reserve(0);
    CPPUNIT_ASSERT (rawPayments.empty());
    CPPUNIT_ASSERT (rawPayments.size() == 0);
    CPPUNIT_ASSERT (rawPayments.begin() == rawPayments.end());
  }

  void reserved()
  {
    RawPayments rawPayments;
    rawPayments.reserve(3);
    CPPUNIT_ASSERT (rawPayments.empty());
    CPPUNIT_ASSERT (rawPayments.size() == 0);
    CPPUNIT_ASSERT (rawPayments.capacity() >= 3);
    CPPUNIT_ASSERT (rawPayments.begin() == rawPayments.end());
  }

  void some_content()
  {
    RawPayments rawPayments;
    rawPayments.reserve(4);
    rawPayments.emplace_back(prData, Geo(), Geo(), ta);
    rawPayments.emplace_back(prData, Geo(), Geo(), tb);
    rawPayments.emplace_back(prData, Geo(), Geo(), tc);
    CPPUNIT_ASSERT (!rawPayments.empty());
    CPPUNIT_ASSERT (rawPayments.size() == 3);
    CPPUNIT_ASSERT (rawPayments.capacity() >= 4);
    CPPUNIT_ASSERT (std::distance(rawPayments.begin(), rawPayments.end()) == 3);
    RawPayments::iterator it = rawPayments.begin();
    CPPUNIT_ASSERT (it->taxName == &ta);
    ++it;
    CPPUNIT_ASSERT (it->taxName == &tb);
    ++it;
    CPPUNIT_ASSERT (it->taxName == &tc);

    CPPUNIT_ASSERT (rawPayments[0].taxName == &ta);
    CPPUNIT_ASSERT (rawPayments[1].taxName == &tb);
    CPPUNIT_ASSERT (rawPayments[2].taxName == &tc);
    CPPUNIT_ASSERT (rawPayments.back().taxName == &tc);

    RawPayments payments2;
    swap(rawPayments, payments2);
    CPPUNIT_ASSERT (rawPayments.size() == 0);
    CPPUNIT_ASSERT (payments2.size() == 3);
  }

  void move_ctor()
  {
    RawPayments rawPayments;
    rawPayments.reserve(4);
    PaymentDetail& pd1 = rawPayments.emplace_back(prData, Geo(), Geo(), ta);
    pd1.taxLabel() = "A";

    RawPayments payments2 = std::move(rawPayments);
    PaymentDetail& pd2 = payments2.emplace_back(prData, Geo(), Geo(), tb);
    pd2.taxLabel() = "B";

    CPPUNIT_ASSERT (rawPayments.empty());
    CPPUNIT_ASSERT (payments2.size() == 2);
    CPPUNIT_ASSERT (payments2[0].detail.taxLabel() == "A");
    CPPUNIT_ASSERT (payments2[1].detail.taxLabel() == "B");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RawPaymentsTest);

} // namespace tax
