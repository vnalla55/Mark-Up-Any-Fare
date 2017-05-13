//-------------------------------------------------------------------
//
//  File:        DecodeContentHandlerTest.cpp
//  Created:     September 5, 2014
//  Authors:     Roland Kwolek
//
//  Description: Decode Content Handler tests
//
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Xform/DecodeContentHandler.h"

#include "Common/Config/ConfigMan.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Billing.h"
#include "DataModel/DecodeTrx.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>
#include <xercesc/util/PlatformUtils.hpp>

namespace
{
bool
areBillingsTheSame(const tse::Billing& arg1, const tse::Billing& arg2)
{
  return arg1.userPseudoCityCode() == arg2.userPseudoCityCode() &&
         arg1.aaaCity() == arg2.aaaCity() && arg1.actionCode() == arg2.actionCode() &&
         arg1.aaaSine() == arg2.aaaSine() && arg1.userSetAddress() == arg2.userSetAddress() &&
         arg1.partitionID() == arg2.partitionID() && arg1.transactionID() == arg2.transactionID() &&
         arg1.parentServiceName() == arg2.parentServiceName() &&
         arg1.serviceName() == arg2.serviceName() &&
         arg1.clientServiceName() == arg2.clientServiceName() &&
         arg1.requestPath() == arg2.requestPath() && arg1.userBranch() == arg2.userBranch() &&
         arg1.userStation() == arg2.userStation();
}

void
setBilling(tse::Billing& billingExpected, const tse::Trx& trx)
{
  billingExpected.userPseudoCityCode() = "HDQ";
  billingExpected.aaaCity() = "HDQ";
  billingExpected.actionCode() = "W/RTG";
  billingExpected.aaaSine() = "JP@";
  billingExpected.userSetAddress() = "194D96";
  billingExpected.partitionID() = "00";
  billingExpected.parentTransactionID() = tse::Billing::string2transactionId("128650836083128579");
  billingExpected.transactionID() = trx.transactionId();
  billingExpected.clientTransactionID() = trx.transactionId();
  billingExpected.parentServiceName() = "RTWFMR";
  billingExpected.serviceName() = "RTWFMR";
  billingExpected.clientServiceName() = "TAXGEN";
  billingExpected.requestPath() = tse::Billing::string2transactionId("PPSS");
  billingExpected.userBranch() = "02995";
  billingExpected.userStation() = "01026";
}
}

namespace tse
{
class DecodeContentHandlerTest : public testing::Test
{
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    std::string serverType("PRICING");
    TestConfigInitializer::setValue("SERVER_TYPE", serverType, "APPLICATION_CONSOLE");
    _trx = _memHandle(new DecodeTrx());
    _dch = _memHandle(new DecodeContentHandler());
    xercesc::XMLPlatformUtils::Initialize();
  }

  void TearDown()
  {
    _memHandle.clear();
    xercesc::XMLPlatformUtils::Terminate();
  }

protected:
  bool parseXmlAndSetTrx(const char* xml)
  {
    bool ret = _dch->parse(xml);
    _dch->setTrx(*_trx);

    return ret;
  }

  TestMemHandle _memHandle;
  DecodeTrx* _trx;
  DecodeContentHandler* _dch;
};

TEST_F(DecodeContentHandlerTest, ZoneTest)
{
  const char* xml = "<DecodeRequest version=\"1.0.0\"><AGI A10=\"HDQ\"/><BIL A20=\"HDQ\" "
                    "A22=\"HDQ\" A70=\"W/RTG\" AA0=\"JP@\" AD0=\"194D96\" AE0=\"00\" "
                    "C00=\"128650836083128579\" C20=\"RTWFMR\" S0R=\"PPSS\" Q02=\"02995\" "
                    "Q03=\"01026\"/><PRO B05=\"AA\" /><OPT RTG=\"210\"/></DecodeRequest>";

  Billing billingExpected;
  setBilling(billingExpected, *_trx);

  EXPECT_TRUE(parseXmlAndSetTrx(xml));
  EXPECT_EQ(LocCode("210"), _trx->getLocation());
  EXPECT_TRUE(areBillingsTheSame(billingExpected, *(_trx->getBilling())));
}

TEST_F(DecodeContentHandlerTest, AllianceTest)
{
  const char* xml = "<DecodeRequest version=\"1.0.0\"><AGI A10=\"HDQ\"/><BIL A20=\"HDQ\" "
                    "A22=\"HDQ\" A70=\"W/RTG\" AA0=\"JP@\" AD0=\"194D96\" AE0=\"00\" "
                    "C00=\"128650836083128579\" C20=\"RTWFMR\" S0R=\"PPSS\" Q02=\"02995\" "
                    "Q03=\"01026\"/><PRO D07=\"2013-08-01\" /><OPT RTG=\"*O\" /></DecodeRequest>";

  Billing billingExpected;
  setBilling(billingExpected, *_trx);

  EXPECT_TRUE(parseXmlAndSetTrx(xml));
  EXPECT_EQ(LocCode("*O"), _trx->getLocation());
  EXPECT_TRUE(areBillingsTheSame(billingExpected, *(_trx->getBilling())));
}

TEST_F(DecodeContentHandlerTest, GenericCityCodeTest)
{
  const char* xml = "<DecodeRequest version=\"1.0.0\"><AGI A10=\"HDQ\"/><BIL A20=\"HDQ\" "
                    "A22=\"HDQ\" A70=\"W/RTG\" AA0=\"JP@\" AD0=\"194D96\" AE0=\"00\" "
                    "C00=\"128650836083128579\" C20=\"RTWFMR\" S0R=\"PPSS\" Q02=\"02995\" "
                    "Q03=\"01026\"/><PRO D07=\"2013-08-01\" /><OPT RTG=\"ECC\" /></DecodeRequest>";

  Billing billingExpected;
  setBilling(billingExpected, *_trx);

  EXPECT_TRUE(parseXmlAndSetTrx(xml));
  EXPECT_EQ(LocCode("ECC"), _trx->getLocation());
  EXPECT_TRUE(areBillingsTheSame(billingExpected, *(_trx->getBilling())));
}
}
