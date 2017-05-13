//----------------------------------------------------------------------------
//
//  File:               IXraySender.h
//  Description:        Interface for Xray sender
//  Created:            07/29/2016
//  Authors:            Grzegorz Ryniak
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

#include <string>

class IXraySender
{
public:
  virtual void send(const std::string& content) = 0;
  virtual ~IXraySender() {};
};
