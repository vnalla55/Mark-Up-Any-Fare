//-------------------------------------------------------------------
//
//  File:        TseCallableTask.h
//  Created:     February 22, 2005
//  Authors:     Mike Lillis
//
//  Description: Task class invoked by the Thread Utility classes
//
//  Updates:    
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

namespace tse
{

// Base class for all tasks submitted to the thread pool for processing.
// The task's "run" function will be invoked by the thread 
class TseCallableTask
{
public:
  virtual ~TseCallableTask() = default;

  virtual void run() { performTask(); }

  virtual void performTask() {}

  void setThrottled() { _throttled = true; }

protected:
  bool _throttled = false;
};
}// tse
