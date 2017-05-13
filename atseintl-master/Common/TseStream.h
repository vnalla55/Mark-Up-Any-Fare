//-------------------------------------------------------------------
//
//  File:        TSEStream.h
//  Created:     April 2, 2007
//  Authors:     Artur Krezel
//
//  Description: Base class to display members from derived class
//
//  Updates:
//
//  Copyright Sabre 2007
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

#include <iostream>

namespace tse
{

// Base class will serve for helper class and print all members from derived class
class Stream
{
  template <class T>
  friend T& operator<<(T& os, Stream& a);

  virtual void operator>>(std::ostream& os) = 0;

public:
  virtual ~Stream() {}
};

template <class T>
T& operator<<(T& os, Stream& a)
{
  a >> os;

  return os;
}
};

