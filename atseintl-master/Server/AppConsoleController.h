//-------------------------------------------------------------------
//
//  File:        AppConsoleController.h
//  Created:     May 09, 2005
//  Authors:     Mark Kasprowicz
//
//  Description: Command and Control Interface
//
//  Copyright Sabre 2005
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

#pragma once

#include <string>

namespace tse
{
class TseServer;
class TseAppConsole;

namespace AppConsoleController
{
bool
startup(const std::string& name, TseServer* srv);

void
shutdown();

int
run();

bool
halt(const unsigned int delaySeconds = 0, bool ignoreSEGV = false);

void
setTerminate();

const std::string&
host();

const std::string&
process();

const std::string&
user();

const std::string&
build();

TseAppConsole&
appConsole();

const std::string&
redeployFilename();

const std::string&
activateFilename();
}
}
