//-------------------------------------------------------------------
//
//  File:        TseCallableTrxTask.h
//  Created:     April 11, 2005
//  Authors:     Daryl Champagne
//
//  Description: Task subclass of TseCallableTask that maintains thread toplevel
//    latency data for metrics gathering.
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

#include "Common/Thread/TseCallableTask.h"
#include "Common/TseStringTypes.h"

#include <iostream>
#include <vector>

#include <pthread.h>
#include <stdint.h>

namespace tse
{
class Trx;
class PricingTrx;

// Base class for tasks that wish to record thread toplevel metrics data.
class TseCallableTrxTask : public TseCallableTask
{
private:
  typedef int32_t InternalSequenceNum;

public:
  /**
   * TseCallableTrxTask::run() calls sequence is indeterministic due to multithreading.
   *
   * However, TseCallableTrxTask is still being constructed from parent thread
   * and we are going to rely on this determinism and track the SequenceID.
   *
   * Then after, SequenceID is used in Diagnostic to sort the output accordingly.
   *
   * So, SequenceID is PricingTrx-wide task sequence id
   */
  struct SequenceID : public std::vector<InternalSequenceNum>
  {
    SequenceID() {}

    template <typename N>
    SequenceID(const std::vector<N>& vec)
      : std::vector<InternalSequenceNum>(vec.begin(), vec.end())
    {
    }

    template <class I>
    SequenceID(I begin, I end)
      : std::vector<InternalSequenceNum>(begin, end)
    {
    }

    friend std::ostream& operator<<(std::ostream& os, const SequenceID& seq);

    static const SequenceID EMPTY;
  };

  TseCallableTrxTask();

  void trx(PricingTrx* trx);
  PricingTrx* trx()
  {
    return _trx;
  };
  void desc(const char* desc)
  {
    _desc = desc;
  };
  const char* desc() const
  {
    return _desc;
  };

  static Trx* currentTrx();
  struct CurrentTrxSetter
  {
    CurrentTrxSetter(Trx* trx, bool activated = true);
    ~CurrentTrxSetter();

  private:
    bool _activated;
  };

  /**
   * return NULL if is not being run for current thread
   */
  static const TseCallableTrxTask* currentTask();
  SequenceID getSequenceID() const;

  static int getCurrentTrxId();
  static class DataHandle* getCurrentDataHandle();

protected:
  const char* _desc;

  TseCallableTrxTask(const TseCallableTrxTask&);

private:
  PricingTrx* _trx;
  pthread_t _parentThreadID;
  TseCallableTrxTask* _parentTask;
  InternalSequenceNum _taskSeqNum;
  InternalSequenceNum _childTaskNextSeqNum;

  void initTaskSeqNumFromParent();
  void initTaskSeqNum(InternalSequenceNum tsn)
  {
    _taskSeqNum = tsn;
    _childTaskNextSeqNum = 0;
  }
  InternalSequenceNum advanceNextChildTaskSeqNum() { return _childTaskNextSeqNum++; }

public:
  void run() override;
};
};

