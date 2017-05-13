#include <stdexcept>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/FareInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"

namespace tse
{
class BoundFareTest : public CppUnit::TestFixture
{
  class _findReference
  {
  public:
    _findReference(uint32_t sequenceNumber) : _sequenceNumber(sequenceNumber) {}

    bool operator()(const Record2Reference& record) const
    {
      return _sequenceNumber == record._sequenceNumber;
    }

  private:
    uint32_t _sequenceNumber;
    // not implemented
    _findReference& operator=(const _findReference&);
  };

  CPPUNIT_TEST_SUITE(BoundFareTest);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST_SUITE_END();

public:
  void testMatch()
  {
    GeneralFareRuleInfo ruleInfo1;
    ruleInfo1.categoryNumber() = 15;
    ruleInfo1.sequenceNumber() = 100000;
    GeneralFareRuleInfo ruleInfo2;
    ruleInfo2.categoryNumber() = 3;
    ruleInfo2.sequenceNumber() = 140000;

    Record2ReferenceVector references;
    references.push_back(Record2Reference(1, 1111, 'T', FARERULE));
    references.push_back(Record2Reference(15, 100000, 'F', FOOTNOTE));
    references.push_back(Record2Reference(15, 100000, 'T', FARERULE));

    uint16_t catNumber(ruleInfo1.categoryNumber());
    uint32_t sequenceNumber(ruleInfo1.sequenceNumber());
    std::pair<Record2ReferenceVector::const_iterator, Record2ReferenceVector::const_iterator>
    bounds;
    Record2Reference dummy;
    dummy._catNumber = catNumber;
    dummy._matchType = FOOTNOTE;
    bounds = std::equal_range(references.begin(), references.end(), dummy);
    _findReference finder1(sequenceNumber);
    Record2ReferenceVector::const_iterator itEnd(bounds.second),
        it(std::find_if(bounds.first, itEnd, finder1));

    CPPUNIT_ASSERT(it != itEnd);

    dummy._catNumber = ruleInfo2.categoryNumber();
    dummy._matchType = FARERULE;
    bounds = std::equal_range(references.begin(), references.end(), dummy);
    _findReference finder2(ruleInfo2.sequenceNumber());
    itEnd = bounds.second;
    it = std::find_if(bounds.first, itEnd, finder2);

    CPPUNIT_ASSERT(it == itEnd);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BoundFareTest);

} // tse
