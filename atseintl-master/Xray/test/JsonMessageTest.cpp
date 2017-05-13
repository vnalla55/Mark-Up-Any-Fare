//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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
#include "Common/DateTime.h"
#include "Xray/JsonMessage.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include <boost/regex.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>

namespace tse
{
namespace xray
{
using namespace ::testing;

class JsonMessageTest : public Test
{
public:
  JsonMessageTest() {}

  JsonMessage generateSampleJsonMessage()
  {
    DateTime startDate("2016-Aug-17 04:44:30.650373");
    DateTime hurryUpDate = startDate.addSeconds(7);
    DateTime endDate = startDate.addSeconds(10);

    JsonMessage jsonMessage("mid", "cid"); // create JsonMessage
    jsonMessage.setId(std::string("id"));
    jsonMessage.setInstance(std::string("instance"));
    jsonMessage.setService(std::string("service"));
    jsonMessage.setHostname(std::string("hostname"));
    jsonMessage.setStartDate(startDate, "-0000");
    jsonMessage.setHurryUpEndDate(hurryUpDate);
    jsonMessage.setEndDate(endDate);
    jsonMessage.setError("error");

    return jsonMessage;
  }

  std::string jsonMessageToString(JsonMessage& jsonMessage)
  {
    std::ostringstream stream;
    jsonMessage.appendStringStream(stream);
    return stream.str();
  }

private:
  TestConfigInitializer cfg;
};

TEST_F(JsonMessageTest, messageFormatViaBus)
{
  TestConfigInitializer::setValue("DIRECT_CONNECTION", "N", "XRAY");
  const std::string expectedFormat =
      R"({"cid":"cid","mid":"mid","id":"id","ts":"2016-08-17T04:44:30.650-0000","host":"hostname",)"
      R"("service":"service","inst":"instance","totaltime":10000,"postHurryUpTime":3000,")"
      R"(error":"error"})";

  JsonMessage jsonMessage = generateSampleJsonMessage();
  ASSERT_EQ(jsonMessageToString(jsonMessage), expectedFormat);
}

TEST_F(JsonMessageTest, messageFormatViaDirectConnection)
{
  TestConfigInitializer::setValue("DIRECT_CONNECTION", "Y", "XRAY");
  const std::string expectedFormat =
      R"({"cid":"cid","mid":"mid","id":"id","ts":"2016-08-17T04:44:30.650-0000","host":"hostname",)"
      R"("service":"service","inst":"instance","totaltime":10000,"postHurryUpTime":3000,"sys":)"
      R"("atsev2","error":"error"})";

  JsonMessage jsonMessage = generateSampleJsonMessage();
  ASSERT_EQ(jsonMessageToString(jsonMessage), expectedFormat);
}

TEST_F(JsonMessageTest, llsFormat)
{
  const auto expectedFormatRegex1 = boost::regex(
      R"("lls":\[\{"serviceName":"\w+"\},\{"anotherServiceName":"\w+"\}\]\}$)");

  const auto expectedFormatRegex2 = boost::regex(
      R"("lls":\[\{"anotherServiceName":"\w+"\},\{"serviceName":"\w+"\}\]\}$)");

  JsonMessage jsonMessage = generateSampleJsonMessage();
  jsonMessage.generateMessageId("serviceName");
  jsonMessage.generateMessageId("anotherServiceName");

  ASSERT_TRUE(boost::regex_search(jsonMessageToString(jsonMessage), expectedFormatRegex1) ||
              boost::regex_search(jsonMessageToString(jsonMessage), expectedFormatRegex2));
}

TEST_F(JsonMessageTest, llsWithNetTimeFormat)
{
  const auto expectedFormatRegex = boost::regex(
      R"("lls":\[\{"serviceName":"\w+","net":100\}\]\})");

  JsonMessage jsonMessage = generateSampleJsonMessage();
  jsonMessage.generateMessageId("serviceName");
  jsonMessage.setLlsNetworkTime("serviceName", 100);

  ASSERT_TRUE(boost::regex_search(jsonMessageToString(jsonMessage), expectedFormatRegex));
}

TEST_F(JsonMessageTest, conversationIdFormat)
{
  const std::string expectedConversationId = "cid.id";

  JsonMessage jsonMessage("mid", "cid");
  jsonMessage.setId("id");

  ASSERT_EQ(jsonMessage.generateConversationId(), expectedConversationId);
}

} // end of xray namespace
} // end of tse namespace
