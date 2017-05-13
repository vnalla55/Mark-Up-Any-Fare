#pragma once

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/FareFocusAccountCdInfo.h"
#include "DBAccess/FareFocusRoutingInfo.h"
#include "DBAccess/FareFocusLocationPairInfo.h"
#include "DBAccess/FareFocusDisplayCatTypeInfo.h"
#include "DBAccess/FareFocusPsgTypeInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareFocusCarrierInfo.h"
#include "DBAccess/FareFocusRuleCodeInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusLookupInfo.h"
#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/RBDByCabinInfo.h"

namespace tse
{
class SerializationTestMiscellaneousClassess : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestMiscellaneousClassess);

  CPPUNIT_TEST(testInfoType<CustomerSecurityHandshakeInfo>);
  CPPUNIT_TEST(testInfoVectorType<CustomerSecurityHandshakeInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusAccountCdInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusAccountCdInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusRoutingInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusRoutingInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusLocationPairInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusLocationPairInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusDisplayCatTypeInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusDisplayCatTypeInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusPsgTypeInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusPsgTypeInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusBookingCodeInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusBookingCodeInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusFareClassInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusFareClassInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusLookupInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusLookupInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusRuleInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusRuleInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusSecurityInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusSecurityInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusCarrierInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusCarrierInfo>);

  CPPUNIT_TEST(testInfoType<FareFocusRuleCodeInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareFocusRuleCodeInfo>);

  CPPUNIT_TEST(testInfoType<FareRetailerCalcInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareRetailerCalcInfo>);

  CPPUNIT_TEST(testInfoType<FareRetailerRuleInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareRetailerRuleInfo>);

  CPPUNIT_TEST(testInfoType<FareRetailerResultingFareAttrInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareRetailerResultingFareAttrInfo>);

  CPPUNIT_TEST(testInfoType<FareRetailerRuleLookupInfo>);
  CPPUNIT_TEST(testInfoVectorType<FareRetailerRuleLookupInfo>);

  CPPUNIT_TEST(testInfoType<RBDByCabinInfo>);
  CPPUNIT_TEST(testInfoVectorType<RBDByCabinInfo>);

  CPPUNIT_TEST(testInfoType<CommissionRuleInfo>);
  CPPUNIT_TEST(testInfoVectorType<CommissionRuleInfo>);

  CPPUNIT_TEST(testInfoType<CommissionProgramInfo>);
  CPPUNIT_TEST(testInfoVectorType<CommissionProgramInfo>);

  CPPUNIT_TEST(testInfoType<CommissionContractInfo>);
  CPPUNIT_TEST(testInfoVectorType<CommissionContractInfo>);

  CPPUNIT_TEST_SUITE_END();
};

}// tse
