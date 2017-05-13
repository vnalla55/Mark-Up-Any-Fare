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
#include "Xray/AsyncJsonContainer.h"
#include "Xray/IXraySender.h"
#include "Xray/JsonMessage.h"
#include "Xray/XrayUtil.h"
#include "Common/DateTime.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <thread>

namespace tse
{
namespace xray
{

class MockXraySender: public IXraySender
{
public:
  MOCK_METHOD1(send, void(const std::string& content));
};

namespace asyncjsoncontainer
{
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::Return;
using ::testing::_;
using ::testing::Invoke;

class AsyncJsonContainerTest : public ::testing::Test
{
public:
  TestConfigInitializer cfg;
  AsyncJsonContainerTest()
  {
    TestConfigInitializer::setValue<uint32_t>("MAX_NUMBER_OF_MESSAGES",
                                              static_cast<uint32_t>(300),
                                              "XRAY");
    TestConfigInitializer::setValue<uint32_t>("CONTAINER_THREAD_SEND_TIMEOUT",
                                              static_cast<uint32_t>(999999),
                                              "XRAY");
  }

  void SetUp()
  {
    numberOfMessagesReceivedBySender = 0;
  }

  void TearDown()
  {
  }

  void senderFunction(const std::string& content)
  {
    //count { ... } occurence, if { ... } are not inside other { ... }
    //every occurence of top level { ... } indicate next message
    int depth = 0;
    int jsonPackSize = 0;
    for(char c: content)
    {
      if(c == '{')
      {
        ++depth;
      }
      else if(c == '}')
      {
        --depth;
        numberOfMessagesReceivedBySender += depth == 0 ? 1 : 0;
        jsonPackSize += depth == 0 ? 1 : 0;
      }
    }
    ASSERT_LE(jsonPackSize, maxNumberOfMessages.getValue());
  }

  int numberOfMessagesReceivedBySender;
};


TEST_F(AsyncJsonContainerTest, senderShouldReceiveSameInputAsExpectedInput)
{
  constexpr int MESSAGES_NUMBER_TO_BE_SEND = 3;
  ASSERT_GE(maxNumberOfMessages.getValue(), MESSAGES_NUMBER_TO_BE_SEND);
  const std::string expectedInputString =
      R"([{"cid":"cid","mid":"mid","id":"id","ts":"2016-08-17T04:44:30.650-0000","host":"hostname")"
      R"(,)"
      R"("service":"service","inst":"instance","totaltime":10000,"error":"error"},)"
      R"({"cid":"cid","mid":"mid","id":"id","ts":"2016-08-17T04:44:30.650-0000","host":"hostname",)"
      R"("service":"service","inst":"instance","totaltime":10000,"error":"error"},)"
      R"({"cid":"cid","mid":"mid","id":"id","ts":"2016-08-17T04:44:30.650-0000","host":"hostname",)"
      R"("service":"service","inst":"instance","totaltime":10000,"error":"error"}])";

  DateTime startDate("2016-Aug-17 04:44:30.650373");
  DateTime endDate = startDate.addSeconds(10);

  JsonMessage jsonMessage("mid", "cid"); //create JsonMessage
  jsonMessage.setId(std::string("id"));
  jsonMessage.setInstance(std::string("instance"));
  jsonMessage.setService(std::string("service"));
  jsonMessage.setError(std::string("error"));
  jsonMessage.setHostname(std::string("hostname"));
  jsonMessage.setStartDate(startDate, "-0000");
  jsonMessage.setEndDate(endDate);

  auto mockXraySenderPtr = std::make_unique<MockXraySender>();
  EXPECT_CALL(*mockXraySenderPtr, send(expectedInputString))
      .Times(1);

  asyncjsoncontainer::initializeWaitForSendThread(std::move(mockXraySenderPtr));

  for(auto i = 0; i < MESSAGES_NUMBER_TO_BE_SEND; ++i)
  {
    asyncjsoncontainer::push(std::make_unique<JsonMessage>(jsonMessage));
  }
  asyncjsoncontainer::closeWaitForSendThread();
}

TEST_F(AsyncJsonContainerTest, singleThreadPushNumberEqualsToReceivedBySender)
{
  constexpr int PUSH_NUMBER = 3000;

  std::unique_ptr<MockXraySender> mockXraySenderPtr(new MockXraySender);
  EXPECT_CALL(*mockXraySenderPtr, send(_))
      .WillRepeatedly(Invoke(this, &AsyncJsonContainerTest::senderFunction));

  asyncjsoncontainer::initializeWaitForSendThread(std::move(mockXraySenderPtr));

  for(auto i = 0; i < PUSH_NUMBER; ++i)
  {
    asyncjsoncontainer::push(std::make_unique<JsonMessage>("mid", "cid"));
  }
  asyncjsoncontainer::closeWaitForSendThread();
  EXPECT_EQ(numberOfMessagesReceivedBySender, PUSH_NUMBER);
}

TEST_F(AsyncJsonContainerTest, multithreadPushNumberEqualsToReceivedBySender)
{
  constexpr int THREADS_NUMBER = 2;
  constexpr int THREAD_PUSH_NUMBER = 3000;

  std::unique_ptr<MockXraySender> mockXraySenderPtr(new MockXraySender);
  EXPECT_CALL(*mockXraySenderPtr, send(_))
      .WillRepeatedly(Invoke(this, &AsyncJsonContainerTest::senderFunction));

  asyncjsoncontainer::initializeWaitForSendThread(std::move(mockXraySenderPtr));

  std::vector<std::thread> threads;
  for(auto i = 0; i < THREADS_NUMBER; ++i)
  {
    threads.push_back(std::thread(
     [this, THREAD_PUSH_NUMBER]()
      {
        for(auto j = 0; j < THREAD_PUSH_NUMBER; ++j)
        {
          asyncjsoncontainer::push(std::make_unique<JsonMessage>("mid", "cid"));
        }
      }
    ));
  }
  for(std::thread& thread: threads)
  {
    thread.join();
  }
  asyncjsoncontainer::closeWaitForSendThread();
  EXPECT_EQ(numberOfMessagesReceivedBySender, THREADS_NUMBER * THREAD_PUSH_NUMBER);
}

} // end of asyncjsoncontainer
} // end of xray namespace
} // end of tse namespace

