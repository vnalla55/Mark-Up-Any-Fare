//-------------------------------------------------------------------
//
//  File:        TseTaskMetrics.h
//  Created:     February 22, 2005
//  Authors:     Mike Lillis
//
//  Description: Structure to maintain the metrics for the threads in a pool
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
    struct TseTaskMetrics
    {
      TseTaskMetrics()
        : _servicingThreadCount(0)
        , _submittedTaskCount(0)
        , _blockingTaskCount(0)
        , _executedTaskCount(0)
        , _deletedTaskCount(0)
        , _currentTaskQueueDepth(0)
        , _maxTaskQueueDepth(0)
        , _currentActiveTasks(0)
        , _maxActiveTasks(0)
        , _numSynchExceptions(0)
      {
      }
      size_t  _servicingThreadCount;
      uint32_t  _submittedTaskCount;
      uint32_t  _blockingTaskCount;
      uint32_t  _executedTaskCount;
      uint32_t  _deletedTaskCount;
      uint32_t  _currentTaskQueueDepth;
      uint32_t  _maxTaskQueueDepth;
      uint32_t  _currentActiveTasks;
      uint32_t  _maxActiveTasks;
      uint32_t  _numSynchExceptions;
    };
};

