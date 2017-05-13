//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Adapter/Adapter.h"
#include "Common/LoggerPtr.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <log4cxx/helpers/objectptr.h>

#include <cstdint>

namespace tse
{
class DiskCacheAdapter;
class Service;
class TseServer;

class DiskCacheAdapter final : public Adapter
{
public:
  DiskCacheAdapter(const std::string&, TseServer&) {}
  ~DiskCacheAdapter() override { shutdown(); }

  bool run(Service& srv);

  void shutdown() override;

  void preShutdown() override;

private:
  bool initialize() override;

  volatile bool _exiting = false;

  mutable boost::mutex _shutdownMutex;

  static log4cxx::LoggerPtr _logger;

  boost::mutex _waitBoostMutex;
  boost::condition_variable _waitBoostCondition;

  int _lastFullVerificationYear = 0;
  int _lastFullVerificationDayOfTheYear = 0;
};
} // namespace tse
