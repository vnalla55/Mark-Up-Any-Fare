#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseEnums.h"
#include "DBAccess/DataHandle.h"
#include "Common/DateTime.h"
#include <iostream>
#include <time.h>
#include <vector>
#include <set>
#include "DataModel/Trx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/PricingRequest.h"
#include "Rules/RuleConst.h"
#include "DBAccess/MiscFareTag.h"
#include "Rules/MiscFareTagsRule.h"
#include "Rules/RuleUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class MiscFareTagsRuleTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(MiscFareTagsRuleTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testValidate()
  {
    MiscFareTag mr;
    MiscFareTagsRule mftr;
    PricingTrx trx;

    PricingRequest request;
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareAmount = 10;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);
    paxTypeFare->setFare(fare);

    trx.setRequest(&request);

    fareInfo->vendor() = "ATP";
    fareInfo->fareClass() = "Y26";

    // 0.
    mr.unavailtag() = 'X';
    mr.overrideDateTblItemNo() = 0;

    Record3ReturnTypes ret = mftr.validate(trx, *paxTypeFare, &mr);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);

    // 1.
    mr.unavailtag() = 'Y';

    ret = mftr.validate(trx, *paxTypeFare, &mr);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);

    // 2.

    mr.unavailtag() = ' ';
    mr.prorateInd() = RuleConst::MUST_BE_USED;

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->fareBasisCode() = "Y26";

    paxTypeFare->fareMarket() = fm;

    ret = mftr.validate(trx, *paxTypeFare, &mr);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);

    // 3
    mr.prorateInd() = RuleConst::BLANK;
    // fareInfo->vendor() = "SITA";
    ret = mftr.validate(trx, *paxTypeFare, &mr);

    CPPUNIT_ASSERT_EQUAL(PASS, ret);
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MiscFareTagsRuleTest);
}
