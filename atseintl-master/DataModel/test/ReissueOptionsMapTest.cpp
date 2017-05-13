#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/ReissueOptionsMap.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/ReissueSequence.h"

#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

namespace tse
{
class ReissueOptionsMapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReissueOptionsMapTest);
  CPPUNIT_TEST(testGetR3);
  CPPUNIT_TEST(testGetT988Seqs);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void testGetR3()
  {
    DateTime begin(2016, 5, 1, 0, 0, 0);
    DateTime end(2016, 5, 31, 0, 0, 0);
    
    _reissueOptions.insertOption(&_ptf, &_r3, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3);
    _reissueOptions.insertOption(&_ptf, &_r3);

    std::vector<ReissueOptions::R3WithDateRange> r3s;
    ASSERT_EQ(5, _reissueOptions.getRec3s(&_ptf, r3s));
  }

  void testGetT988Seqs()
  {
    DateTime begin(2016, 5, 1, 0, 0, 0);
    DateTime end(2016, 5, 31, 0, 0, 0);
    _reissueOptions.insertOption(&_ptf, &_r3, &_seq, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3, &_seq, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3, &_seq, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3, &_seq, {begin, end});
    _reissueOptions.insertOption(&_ptf, &_r3, &_seq);

    std::vector<ReissueOptions::ReissueSeqWithDateRange> t988s;
    ASSERT_EQ(5, _reissueOptions.getT988s(&_ptf, &_r3, t988s));
  }


protected:
  ReissueOptions _reissueOptions;
  VoluntaryChangesInfo _r3;
  ReissueSequence _seq;
  PaxTypeFare _ptf;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReissueOptionsMapTest);

}
