//----------------------------------------------------------------------------
//
//  File:               AsyncJsonContainer.h
//  Description:        Collect JsonMessages by push, send json to sender by waitForSend
//
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
#pragma once

#include "Xray/IXraySender.h"

#include <memory>

namespace tse
{
namespace xray
{
class JsonMessage;
namespace asyncjsoncontainer
{
void push(std::unique_ptr<JsonMessage> jsonMessage);
void initializeWaitForSendThread(std::unique_ptr<IXraySender> sender);
void closeWaitForSendThread();
} // end of asyncjsoncontainer
} // end of xray namespace
} // end of tse namespace

