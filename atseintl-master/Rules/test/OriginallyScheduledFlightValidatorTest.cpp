#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Rules/OriginallyScheduledFlightValidator.h"
#include "Common/DateTime.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"
#include "DataModel/RexPricingRequest.h"

namespace tse
{

using namespace boost::assign;

class MockOriginallyScheduledFlightValidator : public OriginallyScheduledFlightValidator
{
public:
  MockOriginallyScheduledFlightValidator(RexBaseTrx& trx)
    : OriginallyScheduledFlightValidator(trx, 0, log4cxx::Logger::getLogger("null")), _utcOffset(0)
  {
  }

  DateTime getReissueDate(const Loc& departureLoc, const DateTime& departureDT) const
  {
    return _trx.currentTicketingDT() - Minutes(_utcOffset);
  }

  int _utcOffset;
};

class OriginallyScheduledFlightValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OriginallyScheduledFlightValidatorTest);

  CPPUNIT_TEST(testOrigSchedFltWhenBlank);
  CPPUNIT_TEST(testOrigSchedFltWhenFullyFlown);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandOtherBlanksOneSegmentUnflownPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandOtherBlanksOneSegmentUnflownFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandOtherBlanksOneSegmentUnflownPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandOtherBlanksOneSegmentUnflownFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandOtherBlanksOneSegmentUnflownPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandOtherBlanksOneSegmentUnflownFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandOtherBlanksOneSegmentUnflownPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandOtherBlanksOneSegmentUnflownFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandOtherBlanksOneSegmentUnflownPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandOtherBlanksOneSegmentUnflownFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandOtherBlanksOneSegmentUnflownPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandOtherBlanksOneSegmentUnflownFail);
  CPPUNIT_TEST(testOrigSchedFltWhenThirdSegmentIsUnflown);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandOtherBlanksAndBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandOtherBlanksAndBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandOtherBlanksAndBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandOtherBlanksAndBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandOtherBlanksAndBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandOtherBlanksAndBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandOtherBlanksAndBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandOtherBlanksAndBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandOtherBlanksAndBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandOtherBlanksAndBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandOtherBlanksAndBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandOtherBlanksAndBoundaryCaseFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositivePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositiveBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositiveBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositiveBoundaryCaseFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroBoundaryCasePass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroBoundaryCaseFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCaseFailleft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCaseFailRight);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCaseFailleft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCaseFailRight);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCaseFailleft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCaseFailRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCasePassLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCaseFailLeft);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCasePassRight);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCaseFailRight);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandLastYearIsLeapPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandLastYearIsLeapFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandLastMonthIsShorterPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandLastMonthIsShorterFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandLastMonthIsLongerPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandLastMonthIsLongerFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandLastYearIsLeapPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandLastYearIsLeapFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandLastMonthIsShorterPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandLastMonthIsShorterFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandLastMonthIsLongerPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsDandLastMonthIsLongerFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastYearIsLeapPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastYearIsLeapPass2);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastYearIsLeapFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastMonthIsShorterPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastMonthIsShorterFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastMonthIsLongerPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsXandLastMonthIsLongerFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandNextYearIsLeapPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandNextYearIsLeapFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandNextMonthIsShorterPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandNextMonthIsShorterFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandNextMonthIsLongerPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsAandNextMonthIsLongerFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandNextYearIsLeapPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandNextYearIsLeapFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandNextMonthIsShorterPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandNextMonthIsShorterFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandNextMonthIsLongerPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsOandNextMonthIsLongerFail);

  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandNextYearIsLeapPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandNextYearIsLeapFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandNextMonthIsShorterPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandNextMonthIsShorterFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandNextMonthIsLongerPass);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsLandNextMonthIsLongerFail);
  CPPUNIT_TEST(testOrigSchedFltWhenFltIndIsBandAndOnlyChangeToBookingCode);

  CPPUNIT_TEST(testToStringBadValue);
  CPPUNIT_TEST(testToString_NOT_APPLY);
  CPPUNIT_TEST(testToString_ANYTIME_BEFORE);
  CPPUNIT_TEST(testToString_DAY_BEFORE);
  CPPUNIT_TEST(testToString_SAME_DAY_OR_EARLIER);
  CPPUNIT_TEST(testToString_ANYTIME_AFTER);
  CPPUNIT_TEST(testToString_SAME_DAY_OR_LATER);
  CPPUNIT_TEST(testToString_DAY_AFTER);
  CPPUNIT_TEST(testToString_MINUTE);
  CPPUNIT_TEST(testToString_HOUR);
  CPPUNIT_TEST(testToString_DAY);
  CPPUNIT_TEST(testToString_MONTH);

  CPPUNIT_TEST(testPrintMainMessage_2DaysSameDayErlier);
  CPPUNIT_TEST(testPrintMainMessage_1MonthAnytimeBefore);
  CPPUNIT_TEST(testPrintMainMessage_NotApply);

  CPPUNIT_TEST(testDisplayErrorMessage_VoluntaryRefunds);
  CPPUNIT_TEST(testDisplayErrorMessage_VoluntaryChanges);
  CPPUNIT_TEST(testDisplayFailMessage_VoluntaryRefunds);
  CPPUNIT_TEST(testDisplayFailMessage_VoluntaryChanges);
  CPPUNIT_TEST(testDisplayPassMessage_VoluntaryRefunds);
  CPPUNIT_TEST(testDisplaySoftPassMessage_VoluntaryRefunds);
  CPPUNIT_TEST(testDisplayPassMessage_AdjustedDate_VoluntaryRefunds);
  CPPUNIT_TEST(testDisplayPassMessage_VoluntaryChanges);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  OriginallyScheduledFlightValidator* _val;
  RexBaseTrx* _trx;

  OriginallyScheduledFlightValidatorTest()
  {
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
  }

  void setUp()
  {
    _trx = _memH.insert(new RexPricingTrx);
    _trx->setRequest(_memH.create<RexPricingRequest>());

    _trx->getRequest()->ticketingAgent() = _memH.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentLocation() = _memH.create<Loc>();

    _trx->currentTicketingDT() = DateTime(time(0));

    _val = _memH.insert(new MockOriginallyScheduledFlightValidator(*_trx));
  }

  void tearDown() { _memH.clear(); }

  void attachDiag()
  {
    _val->_dc = _memH.insert(new DiagCollector);
    _val->_dc->activate();
  }

  std::string getDiagString()
  {
    _val->_dc->flushMsg();
    return _val->_dc->str();
  }

  DateTime dateTime(std::string date) { return DateTime(date); }

  typedef OriginallyScheduledFlightValidator osfv;

  enum
  {
    Flown = 0,
    Unflown = 1
  };
  static const ResUnit BlankUnit;
  static const ResPeriod BlankPeriod;

  void testOrigSchedFltWhenBlank()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-07 07:07");
    CPPUNIT_ASSERT(_val->match(segs, osfv::NOT_APPLY, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFullyFlown()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Flown, "2007-07-01 08:23");
    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsBandOtherBlanksOneSegmentUnflownPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-06-28 08:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsBandOtherBlanksOneSegmentUnflownFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-05 03:25");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsDandOtherBlanksOneSegmentUnflownPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-29 18:23");
    _trx->currentTicketingDT() = dateTime("2006-12-05 13:55");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsDandOtherBlanksOneSegmentUnflownFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-29 18:23");
    _trx->currentTicketingDT() = dateTime("2007-07-15 23:55");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsXandOtherBlanksOneSegmentUnflownPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-04-23 11:13");
    _trx->currentTicketingDT() = dateTime("2007-01-05 13:55");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsXandOtherBlanksOneSegmentUnflownFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-05 14:01");
    _trx->currentTicketingDT() = dateTime("2008-06-05 14:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsAandOtherBlanksOneSegmentUnflownPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-27 14:33");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsAandOtherBlanksOneSegmentUnflownFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-17 14:33");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsOandOtherBlanksOneSegmentUnflownPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-23 14:33");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsOandOtherBlanksOneSegmentUnflownFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-10-22 06:51");
    _trx->currentTicketingDT() = dateTime("2007-10-11 14:38");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsLandOtherBlanksOneSegmentUnflownPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-10-22 16:42");
    _trx->currentTicketingDT() = dateTime("2007-10-30 01:05");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsLandOtherBlanksOneSegmentUnflownFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-10-22 16:42");
    _trx->currentTicketingDT() = dateTime("2007-01-16 11:39");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenThirdSegmentIsUnflown()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Flown, "2007-08-09 15:33"),
        createSeg(TravelSeg::CHANGED, Flown, "2007-08-15 16:17"),
        createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 22:48");
    _trx->currentTicketingDT() = dateTime("2007-08-20 17:19");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsBandOtherBlanksAndBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-11-20 18:10");
    _trx->currentTicketingDT() = dateTime("2007-11-20 18:09");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsBandOtherBlanksAndBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-11-20 18:10");
    _trx->currentTicketingDT() = dateTime("2007-11-20 18:10");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsBandAndOnlyChangeToBookingCode()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::INVENTORYCHANGED, Unflown, "2007-11-20 18:10");
    _trx->currentTicketingDT() = dateTime("2007-11-20 18:10");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsDandOtherBlanksAndBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-29 18:23");
    _trx->currentTicketingDT() = dateTime("2007-06-28 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsDandOtherBlanksAndBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-29 18:23");
    _trx->currentTicketingDT() = dateTime("2007-06-29 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsXandOtherBlanksAndBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-05 14:01");
    _trx->currentTicketingDT() = dateTime("2007-06-05 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsXandOtherBlanksAndBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-06-05 14:01");
    _trx->currentTicketingDT() = dateTime("2007-06-06 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsAandOtherBlanksAndBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-22 16:12");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsAandOtherBlanksAndBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-22 16:11");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsOandOtherBlanksAndBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-22 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsOandOtherBlanksAndBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-22 16:11");
    _trx->currentTicketingDT() = dateTime("2007-08-21 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsLandOtherBlanksAndBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-10-22 16:42");
    _trx->currentTicketingDT() = dateTime("2007-10-23 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsLandOtherBlanksAndBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-10-22 16:42");
    _trx->currentTicketingDT() = dateTime("2007-10-22 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, BlankPeriod, BlankUnit[0]));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-23 16:42");
    _trx->currentTicketingDT() = dateTime("2007-08-22 15:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:42");
    _trx->currentTicketingDT() = dateTime("2007-08-22 15:25");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-15 15:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "140", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 15:25");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "140", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-15 15:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'H'));
  }
  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 15:25");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-16 15:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "009", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 15:25");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "240", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 15:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");

    _trx->currentTicketingDT() = dateTime("2007-08-20 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");

    _trx->currentTicketingDT() = dateTime("2007-08-17 22:15");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");

    _trx->currentTicketingDT() = dateTime("2007-08-19 12:15");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 12:15");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-19 17:15");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-06-19 17:15");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2008-05-19 17:15");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 17:15");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 17:15");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-16 17:15");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "030", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 12:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "030", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 12:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 12:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 05:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "010", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 23:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "010", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 23:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 15:20");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-15 15:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-14 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "010", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 13:55");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 13:55");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2006-08-18 13:55");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "012", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-10 13:55");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 13:55");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-05-10 16:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "20", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 15:55");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "20", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-05 11:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 11:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 11:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "005", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:32");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "005", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:32");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2008-08-17 23:32");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-16 23:32");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:32");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-29 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-03-18 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "005", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-05-28 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "005", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:52");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:30");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:50");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 17:50");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 18:30");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 20:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 22:30");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 22:30");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 22:30");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-06-20 12:37");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-28 12:37");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-28 12:37");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-02 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-01 10:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "152", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 10:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 10:20");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:15");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "030", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 01:15");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "045", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:05");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 18:05");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 18:05");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "020", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 18:05");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "003", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 06:30");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-12-19 16:30");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-22 00:01");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "005", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-27 14:53");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "001", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-27 14:53");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-01 04:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-01 04:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "004", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-01-01 04:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "019", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 04:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-20 04:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:40");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "050", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 01:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "050", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:40");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-22 01:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 01:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "010", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 11:20");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "003", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 11:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 11:20");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 23:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-29 23:20");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-29 23:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-29 13:25");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositivePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-29 13:25");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "012", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-07-08 17:15");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:35");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsNandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:36");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 14:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsHandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 14:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsDandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-06-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandUnitIsMandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-06-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 23:48");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "012", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsNandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 23:49");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "012", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 14:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "010", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsHandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 14:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "010", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-15 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsDandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-16 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-04-18 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "004", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandUnitIsMandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-04-19 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "004", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "015", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsNandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "015", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 19:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "005", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsHandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 19:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "005", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-16 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsDandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-17 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositiveBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-06-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandUnitIsMandPeriodIsPositiveBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-06-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroBoundaryCasePass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsZeroBoundaryCaseFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:55");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsNandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:56");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "010", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 17:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 18:45");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsHandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 18:46");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsDandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-22 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCaseFailleft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-31 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:46");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 16:45");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandUnitIsMandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsNandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:21");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 01:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 02:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsHandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 02:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", 'H'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-22 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsDandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-23 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "003", 'D'));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCaseFailleft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-31 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-18 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-19 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandUnitIsMandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-10-20 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:20");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsNandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:21");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "020", osfv::MINUTE));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 01:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "002", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 02:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "002", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsHandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 02:01");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::HOUR));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-21 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "002", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-22 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "002", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsDandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-23 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::DAY));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCaseFailleft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-31 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsZeroBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-09-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "000", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCasePassLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-20 00:00");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "003", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCaseFailLeft()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-08-19 23:59");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "003", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCasePassRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-11-20 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "003", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandUnitIsMandPeriodIsPositiveBoundaryCaseFailRight()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-08-19 16:45");
    _trx->currentTicketingDT() = dateTime("2007-11-21 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "003", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandLastYearIsLeapPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 00:50");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandLastYearIsLeapFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 00:50");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandLastMonthIsShorterPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-05-31 12:50");
    _trx->currentTicketingDT() = dateTime("2009-04-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandLastMonthIsShorterFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-05-31 12:50");
    _trx->currentTicketingDT() = dateTime("2009-05-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandLastMonthIsLongerPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 12:50");
    _trx->currentTicketingDT() = dateTime("2009-03-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsBandLastMonthIsLongerFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 12:50");
    _trx->currentTicketingDT() = dateTime("2009-03-31 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandLastYearIsLeapPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 00:50");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandLastYearIsLeapFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 00:50");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandLastMonthIsShorterPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 12:50");
    _trx->currentTicketingDT() = dateTime("2008-12-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandLastMonthIsShorterFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 12:50");
    _trx->currentTicketingDT() = dateTime("2009-01-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandLastMonthIsLongerPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 12:50");
    _trx->currentTicketingDT() = dateTime("2009-03-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsDandLastMonthIsLongerFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 12:50");
    _trx->currentTicketingDT() = dateTime("2009-03-30 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_BEFORE, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastYearIsLeapPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 13:23");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastYearIsLeapPass2()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-30 13:23");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastYearIsLeapFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 13:23");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "011", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastMonthIsShorterPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 12:50");
    _trx->currentTicketingDT() = dateTime("2008-12-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastMonthIsShorterFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 12:50");
    _trx->currentTicketingDT() = dateTime("2009-01-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastMonthIsLongerPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 12:50");
    _trx->currentTicketingDT() = dateTime("2009-03-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsXandLastMonthIsLongerFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 12:50");
    _trx->currentTicketingDT() = dateTime("2009-03-31 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_EARLIER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandNextYearIsLeapPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-12-31 16:45");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandNextYearIsLeapFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-12-31 16:45");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandNextMonthIsShorterPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 00:50");
    _trx->currentTicketingDT() = dateTime("2009-02-28 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandNextMonthIsShorterFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-01-31 00:50");
    _trx->currentTicketingDT() = dateTime("2009-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandNextMonthIsLongerPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 00:50");
    _trx->currentTicketingDT() = dateTime("2009-05-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::ANYTIME_AFTER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsAandNextMonthIsLongerFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2009-04-30 00:50");
    _trx->currentTicketingDT() = dateTime("2009-05-31 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::ANYTIME_AFTER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandNextYearIsLeapPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-12-31 16:45");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandNextYearIsLeapFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-12-31 16:45");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandNextMonthIsShorterPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-01-31 16:45");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandNextMonthIsShorterFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-01-31 16:45");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandNextMonthIsLongerPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-02-29 16:45");
    _trx->currentTicketingDT() = dateTime("2008-03-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::SAME_DAY_OR_LATER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsOandNextMonthIsLongerFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-02-29 16:45");
    _trx->currentTicketingDT() = dateTime("2008-03-30 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::SAME_DAY_OR_LATER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandNextYearIsLeapPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-12-29 16:45");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandNextYearIsLeapFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-12-29 16:45");
    _trx->currentTicketingDT() = dateTime("2008-03-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandNextMonthIsShorterPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-01-30 16:45");
    _trx->currentTicketingDT() = dateTime("2008-02-29 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "001", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandNextMonthIsShorterFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-01-30 16:45");
    _trx->currentTicketingDT() = dateTime("2008-04-01 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandNextMonthIsLongerPass()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-04-29 16:45");
    _trx->currentTicketingDT() = dateTime("2008-06-30 23:59");

    CPPUNIT_ASSERT(_val->match(segs, osfv::DAY_AFTER, "002", osfv::MONTH));
  }

  void testOrigSchedFltWhenFltIndIsLandNextMonthIsLongerFail()
  {
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-04-29 16:45");
    _trx->currentTicketingDT() = dateTime("2008-05-31 00:00");

    CPPUNIT_ASSERT(!_val->match(segs, osfv::DAY_AFTER, "001", osfv::MONTH));
  }

  AirSeg* createSeg(TravelSeg::ChangeStatus status, bool unflown, std::string date)
  {
    AirSeg* seg = _memH.create<AirSeg>();
    seg->origin() = _memH.create<Loc>();
    seg->departureDT() = DateTime(date);
    seg->unflown() = unflown;
    seg->segmentType() = Air;
    seg->changeStatus() = status;
    return seg;
  }

  static const Indicator NOT_SUPPORTED = 'Y';

  void testToStringBadValue()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("Y"), _val->toString(NOT_SUPPORTED));
  }

  void testToString_NOT_APPLY()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NOT APPLY"), _val->toString(osfv::NOT_APPLY));
  }

  void testToString_ANYTIME_BEFORE()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ANYTIME BEFORE"), _val->toString(osfv::ANYTIME_BEFORE));
  }

  void testToString_DAY_BEFORE()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DAY BEFORE"), _val->toString(osfv::DAY_BEFORE));
  }

  void testToString_SAME_DAY_OR_EARLIER()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("SAME DAY OR EARLIER"),
                         _val->toString(osfv::SAME_DAY_OR_EARLIER));
  }

  void testToString_ANYTIME_AFTER()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ANYTIME AFTER"), _val->toString(osfv::ANYTIME_AFTER));
  }

  void testToString_SAME_DAY_OR_LATER()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("SAME DAY OR LATER"), _val->toString(osfv::SAME_DAY_OR_LATER));
  }

  void testToString_DAY_AFTER()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DAY AFTER"), _val->toString(osfv::DAY_AFTER));
  }

  enum
  {
    Singular = 0,
    Plural = 1
  };

  void testToString_MINUTE()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MINUTE"), _val->toString(osfv::MINUTE, Singular));
  }

  void testToString_HOUR()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("HOURS"), _val->toString(osfv::HOUR, Plural));
  }

  void testToString_DAY()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DAY"), _val->toString(osfv::DAY, Singular));
  }

  void testToString_MONTH()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MONTHS"), _val->toString(osfv::MONTH, Plural));
  }

  void testPrintMainMessage_2DaysSameDayErlier()
  {
    attachDiag();
    _val->printMainMessage(osfv::SAME_DAY_OR_EARLIER, "002", osfv::DAY);
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: "
                                     "2 DAYS SAME DAY OR EARLIER\n"),
                         getDiagString());
  }

  void testPrintMainMessage_1MonthAnytimeBefore()
  {
    attachDiag();
    _val->printMainMessage(osfv::ANYTIME_BEFORE, "001", osfv::MONTH);
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: "
                                     "1 MONTH ANYTIME BEFORE\n"),
                         getDiagString());
  }

  void testPrintMainMessage_NotApply()
  {
    attachDiag();
    _val->printMainMessage(osfv::NOT_APPLY, BlankPeriod, BlankUnit[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: NOT APPLY\n"), getDiagString());
  }

  enum
  {
    r3ItemNo = 12345,
    seqItemNo = 6789
  };

  void testDisplayErrorMessage_VoluntaryRefunds()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-01-30 16:45");

    CPPUNIT_ASSERT(!_val->validate(r3ItemNo, segs, NOT_SUPPORTED, BlankPeriod, BlankUnit));
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: Y\n"
                                     " ERROR: RECORD3 NO. 12345 INCORRECT BYTE 44\n"),
                         getDiagString());
  }

  void testDisplayErrorMessage_VoluntaryChanges()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2008-01-30 16:45");

    CPPUNIT_ASSERT(
        !_val->validate(r3ItemNo, seqItemNo, segs, NOT_SUPPORTED, BlankPeriod, BlankUnit[0]));
    CPPUNIT_ASSERT_EQUAL(std::string(" ORIGINALLY SCHEDULED FLIGHT CHECK: Y\n\n"
                                     "ERROR: TABLE 988 SEQ NO 6789 INCORRECT BYTE 32\n"),
                         getDiagString());
  }

  void testDisplayFailMessage_VoluntaryRefunds()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-05 03:25");

    CPPUNIT_ASSERT(!_val->validate(r3ItemNo, segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit));
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: ANYTIME BEFORE\n"
                                     " REFUND DATE/TIME          : 2007-07-05T03:25:00\n"
                                     " REFUND ALLOWED ON/BEFORE  : 2007-07-01T08:22:00\n"
                                     " FAILED ITEM 12345 - ORIG SCHED FLT RESTR NOT MET\n"),
                         getDiagString());
  }

  void testDisplayFailMessage_VoluntaryChanges()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-05 03:25");

    CPPUNIT_ASSERT(!_val->validate(
        r3ItemNo, seqItemNo, segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
    CPPUNIT_ASSERT_EQUAL(std::string(" ORIGINALLY SCHEDULED FLIGHT CHECK: B\n"
                                     "  SEQ 6789: BYTE 32 CHECK FAILED\n"
                                     "    ORIG SCHED: B\n"
                                     "    CHANGES ALLOWED ON/BEFORE 2007-07-01T08:22:00\n"),
                         getDiagString());
  }

  void testDisplayPassMessage_VoluntaryRefunds()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-01 03:25");

    CPPUNIT_ASSERT(_val->validate(r3ItemNo, segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit));
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: ANYTIME BEFORE\n"
                                     " REFUND DATE/TIME          : 2007-07-01T03:25:00\n"
                                     " REFUND ALLOWED ON/BEFORE  : 2007-07-01T08:22:00\n"),
                         getDiagString());
  }

  void testDisplaySoftPassMessage_VoluntaryRefunds()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::UNCHANGED, Flown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-01 03:25");

    CPPUNIT_ASSERT(_val->validate(r3ItemNo, segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit));
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: SOFTPASS\n"), getDiagString());
  }

  void testDisplayPassMessage_AdjustedDate_VoluntaryRefunds()
  {
    static_cast<MockOriginallyScheduledFlightValidator*>(_val)->_utcOffset = 60;

    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-01 09:00");

    CPPUNIT_ASSERT(_val->validate(r3ItemNo, segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit));
    CPPUNIT_ASSERT_EQUAL(std::string("ORIGINALLY SCHEDULED FLIGHT: ANYTIME BEFORE\n"
                                     " REFUND DATE/TIME          : 2007-07-01T09:00:00\n"
                                     " ADJUSTED REFUND DATE/TIME : 2007-07-01T08:00:00\n"
                                     " REFUND ALLOWED ON/BEFORE  : 2007-07-01T08:22:00\n"),
                         getDiagString());
  }

  void testDisplayPassMessage_VoluntaryChanges()
  {
    attachDiag();
    std::vector<TravelSeg*> segs;
    segs += createSeg(TravelSeg::CHANGED, Unflown, "2007-07-01 08:23");
    _trx->currentTicketingDT() = dateTime("2007-07-01 03:25");

    CPPUNIT_ASSERT(
        _val->validate(r3ItemNo, seqItemNo, segs, osfv::ANYTIME_BEFORE, BlankPeriod, BlankUnit[0]));
    CPPUNIT_ASSERT_EQUAL(std::string(" ORIGINALLY SCHEDULED FLIGHT CHECK: B\n"), getDiagString());
  }
};

const ResUnit
OriginallyScheduledFlightValidatorTest::BlankUnit(" ");
const ResPeriod
OriginallyScheduledFlightValidatorTest::BlankPeriod("");

CPPUNIT_TEST_SUITE_REGISTRATION(OriginallyScheduledFlightValidatorTest);
}
