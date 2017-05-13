//-------------------------------------------------------------------
//
//  File:        DiskspaceThread.h
//  Created:     June 25, 2009
//  Authors:     Mark Wedge
//
//  Copyright Sabre 2009
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

#include "Common/Thread/TimerTask.h"

#include <string>

namespace tse
{
class DiskspaceWatcher : public TimerTask
{
public:
  DiskspaceWatcher(const std::string& path,
                   int sleepSecs,
                   int freeThreshold,
                   const std::string& alertEmail);

  DiskspaceWatcher(const DiskspaceWatcher&) = delete;
  DiskspaceWatcher& operator=(const DiskspaceWatcher&) = delete;

  ~DiskspaceWatcher();

  static DiskspaceWatcher* startWatching(const std::string& path,
                                         int sleepSecs,
                                         int freeThreshold,
                                         const std::string& alertEmail);

  static void stopWatching();

  static bool isDiskFull() { return _instance && _instance->isFull(); }

  virtual void run() override;
  bool isFull() { return _full; }

protected:
  virtual void onCancel() override;

private:
  static DiskspaceWatcher* _instance;

  bool getcwd();
  void sendAlert(const std::string& message);

  std::string _path;
  int _freeThreshold;
  std::string _alertEmail;
  float _currentFree = 0;
  bool _full = false;
};

}
