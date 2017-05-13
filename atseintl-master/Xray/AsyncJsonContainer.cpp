//----------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "Xray/AsyncJsonContainer.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Util/unlock_guard.h"
#include "Xray/IXraySender.h"
#include "Xray/JsonMessage.h"
#include "XrayUtil.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace tse
{
namespace xray
{
namespace asyncjsoncontainer
{
using Container = std::vector<std::unique_ptr<JsonMessage>>;

static Logger
logger("atseintl.Xray.AsyncJsonContainer");

namespace
{
  bool isWaitForSendThreadEnabled = false;
  std::thread waitForSendThread;
  Container currentContainer;
  std::mutex currentContainerMutex;
  std::condition_variable isReadyToSend;
  std::condition_variable isPushAllowed;
}

void
push(std::unique_ptr<JsonMessage> jsonMessage)
{
  {
    LOG4CXX_DEBUG(logger, "Pushing jsonMessage (id:" << jsonMessage->getId() << ") to container");
    std::unique_lock<std::mutex> currentContainerLock(currentContainerMutex);
    if(!isWaitForSendThreadEnabled)
    {
      LOG4CXX_ERROR(logger, "Couldn't push jsonMessage (id:" << jsonMessage->getId()
                                  << ") to container. WaitForSendThread is disabled");
      return;
    }
    if(currentContainer.size() >= maxNumberOfMessages.getValue())
    {
      LOG4CXX_DEBUG(logger, "Container is full");
      {
        unlock_guard<std::unique_lock<std::mutex>> unlockForNotify(currentContainerLock);
        isReadyToSend.notify_one();
      }
      isPushAllowed.wait(currentContainerLock, [](){
        return currentContainer.size() < maxNumberOfMessages.getValue();
      });
    }
    LOG4CXX_DEBUG(logger, "JsonMessage (id:" << jsonMessage->getId() << ") pushed to container");
    currentContainer.push_back(std::move(jsonMessage));
  }
}

std::string
buildJsonPack(const Container& container)
{
  if(container.empty())
  {
    LOG4CXX_DEBUG(logger, "Cannot build JsonPack, because container is empty");
    return std::string();
  }

  LOG4CXX_DEBUG(logger, "Building JsonPack with " << container.size() << " messages");
  std::ostringstream jsonPackStream;
  jsonPackStream << "[";
  for(const auto& jsonMessage: container)
  {
    jsonMessage->appendStringStream(jsonPackStream);
    jsonPackStream << ",";
  }
  std::string jsonPack = jsonPackStream.str();
  jsonPack.pop_back(); //remove last comma
  jsonPack += "]";
  return jsonPack;
}

void
sendToSender(IXraySender& sender, Container& container)
{
  if(container.empty())
  {
    return;
  }
  std::string jsonPack = buildJsonPack(container);
  sender.send(jsonPack);
  container.clear();
}

void
waitForSend(std::unique_ptr<IXraySender> sender)
{
  try
  {
    TSE_ASSERT(sender);
    Container container;
    container.reserve(maxNumberOfMessages.getValue());
    auto sendTimeout = std::chrono::seconds(containerThreadSendTimeout.getValue());

    bool isExecuting = true;
    while(isExecuting)
    {
      {
        std::unique_lock<std::mutex> containerLock(currentContainerMutex);

        isReadyToSend.wait_for(containerLock, sendTimeout, [](){
          return (currentContainer.size() >= maxNumberOfMessages.getValue() ||
                  !isWaitForSendThreadEnabled);
        });

        isExecuting = isWaitForSendThreadEnabled;
        std::swap(container, currentContainer);
      }
      isPushAllowed.notify_all();
      sendToSender(*sender, container);
    }
  }
  catch(...)
  {
    LOG4CXX_ERROR(logger, "Error occurred while waitForSendThread was working. Ending thread");
    std::unique_lock<std::mutex> containerLock(currentContainerMutex);
    isWaitForSendThreadEnabled = false;
  }
}

void
initializeWaitForSendThread(std::unique_ptr<IXraySender> sender)
{
  if(!sender)
    return;
  {
    LOG4CXX_DEBUG(logger, "Initializing waitForSendThread");
    std::lock_guard<std::mutex> currentContainerLock(currentContainerMutex);
    isWaitForSendThreadEnabled = true;
    TSE_ASSERT(!waitForSendThread.joinable());
    try
    {
      waitForSendThread = std::thread(asyncjsoncontainer::waitForSend, std::move(sender));
    }
    catch(const std::system_error& e)
    {
      isWaitForSendThreadEnabled = false;
      LOG4CXX_ERROR(logger, "Failed to create waitForSendThread: " << e.what());
    }
  }
}

void
closeWaitForSendThread()
{
  {
    LOG4CXX_DEBUG(logger, "Destroying waitForSendThread");
    std::lock_guard<std::mutex> containerLock(currentContainerMutex);
    isWaitForSendThreadEnabled = false;
  }
  isReadyToSend.notify_one();

  if (waitForSendThread.joinable())
    waitForSendThread.join();
}

} // end of asyncjsoncontainer
} // end of xray namespace
} // end of tse namespace
