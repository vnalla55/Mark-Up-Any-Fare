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

#include "Common/DiskspaceWatcher.h"

#include "Common/Logger.h"
#include "Common/Thread/TimerTaskExecutor.h"
#include "Common/TseUtil.h"

#include <sstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>

namespace tse
{
namespace
{
Logger
logger("atseintl.Common.DiskspaceWatcher");
constexpr int DEFAULT_CHECK_INTERVAL = 300;
constexpr int DEFAULT_FREE_THRESHOLD = 5;
}

DiskspaceWatcher* DiskspaceWatcher::_instance(nullptr);

DiskspaceWatcher::DiskspaceWatcher(const std::string& path,
                                   int sleepSecs,
                                   int freeThreshold,
                                   const std::string& alertEmail)
  : TimerTask(TimerTask::REPEATING,
              std::chrono::seconds(sleepSecs > 0 ? sleepSecs : DEFAULT_CHECK_INTERVAL)),
    _path(path),
    _freeThreshold(freeThreshold),
    _alertEmail(alertEmail)
{
  if (_path.empty())
    getcwd();

  if (_freeThreshold == 0)
    _freeThreshold = DEFAULT_FREE_THRESHOLD;

  std::replace(_alertEmail.begin(), _alertEmail.end(), ';', ' ');

  LOG4CXX_INFO(logger, "Monitoring " << _path);
  const int seconds = std::chrono::duration_cast<std::chrono::seconds>(period()).count();
  LOG4CXX_INFO(logger, "Check interval is set to " << seconds << " seconds.");
  LOG4CXX_INFO(logger, "Free threshold is set to " << _freeThreshold << " percent.");

  if (!_alertEmail.empty())
    LOG4CXX_INFO(logger, "Alerts will be sent to " << _alertEmail);

  TimerTaskExecutor::instance().scheduleNow(*this);
}

DiskspaceWatcher::~DiskspaceWatcher()
{
  TimerTaskExecutor::instance().cancelAndWait(*this);
}

bool
DiskspaceWatcher::getcwd()
{
  char directory[1024];
  if (!::getcwd(directory, sizeof(directory)))
  {
    LOG4CXX_ERROR(logger, "Unable to get current working directory!");
    return false;
  }

  _path = directory;
  return true;
}

void
DiskspaceWatcher::sendAlert(const std::string& message)
{
  tse::TseUtil::alert(message.c_str());

  static std::string mailCommand;
  static std::string processInfo;
  static std::string hostName;
  static std::string mypid;

  if (hostName.empty())
  {
    char hostname[1024];
    if (::gethostname(hostname, sizeof(hostname) - 1) < 0)
    {
      LOG4CXX_ERROR(logger, "Unable to determine hostname");
      hostName = "UNKNOWN_HOST";
    }
    else
    {
      hostName = hostname;
    }
  }

  if (mailCommand.empty())
  {
    if (access("/bin/xmail", X_OK) == 0)
      mailCommand = "/bin/xmail";
    else if (access("/usr/bin/xmail", X_OK) == 0)
      mailCommand = "/usr/bin/xmail";
    else if (access("/bin/mail", X_OK) == 0)
      mailCommand = "/bin/mail";
    else if (access("/usr/bin/mail", X_OK) == 0)
      mailCommand = "/usr/bin/mail";
    else if (access("/bin/mailx", X_OK) == 0)
      mailCommand = "/bin/mailx";
    else if (access("/usr/bin/mailx", X_OK) == 0)
      mailCommand = "/usr/bin/mailx";
    else
      mailCommand = "NOMAIL";
  }

  if (mypid.empty())
  {
    std::ostringstream sPid;
    sPid << getpid();
    mypid = sPid.str();
  }

  if (processInfo.empty())
  {
    processInfo = "PROCESS INFORMATION\n";
    processInfo.append("-------------------\n");
    processInfo.append("Process ID : ");
    processInfo.append(mypid);
    processInfo.append("\nCommand    : ");

    std::string pidfile("/proc/");
    pidfile.append(mypid);
    pidfile.append("/cmdline");

    char arg_list[1024];

    const int fd = open(pidfile.c_str(), O_RDONLY);
    const size_t len = read(fd, arg_list, sizeof(arg_list));
    close(fd);
    arg_list[len] = '\0';

    bool firstArgPrinted = false;
    char* next_arg = arg_list;
    while (next_arg < (arg_list + len))
    {
      if (next_arg != arg_list)
      {
        if (firstArgPrinted)
          processInfo.append("             ");
        else
          firstArgPrinted = true;
      }

      processInfo.append(next_arg);
      processInfo.append("\n");

      if (next_arg == arg_list)
        processInfo.append("Arguments  : ");

      next_arg += strlen(next_arg) + 1;
    }
  }

  if ((!_alertEmail.empty()) && (mailCommand != "NOMAIL"))
  {
    std::ostringstream os;
    os << mailCommand << " -s '*** ATSEv2 Disk Space Alert for [" << hostName << "] ***' '"
       << _alertEmail << "'";
    FILE* mailer = popen(os.str().c_str(), "w");
    fprintf(mailer, "MESSAGE\n");
    fprintf(mailer, "-------\n");
    fprintf(mailer, message.c_str());
    fprintf(mailer, "\n\n");
    fprintf(mailer, processInfo.c_str());
    fprintf(mailer, "\n\n");
    pclose(mailer);
  }
}

DiskspaceWatcher*
DiskspaceWatcher::startWatching(const std::string& path,
                                int sleepSecs,
                                int freeThreshold,
                                const std::string& alertEmail)
{
  if (!_instance)
    _instance = new DiskspaceWatcher(path, sleepSecs, freeThreshold, alertEmail);
  return _instance;
}

void
DiskspaceWatcher::stopWatching()
{
  if (_instance)
  {
    delete _instance;
    _instance = nullptr;
  }
}

void
DiskspaceWatcher::run()
{
  bool hasPath = _path.empty();
  if (!hasPath)
    hasPath = getcwd();

  if (!hasPath)
  {
    _full = true;
    sendAlert("Failed to determine current working directory for log files, unable to log any "
              "messages!");
    return;
  }

  struct statvfs diskData;
  const int rc = statvfs(_path.c_str(), &diskData);

  if (rc < 0)
  {
    _full = true;
    sendAlert("Failed to get disk stats for Disk " + _path + ".  Unable to log any more messages!");
    return;
  }

  _currentFree = ((float)diskData.f_bavail / (float)diskData.f_blocks) * 100.00f;
  LOG4CXX_INFO(logger, "Disk " << _path << " is " << _currentFree << " percent free");

  if (_currentFree >= _freeThreshold)
  {
    _full = false;
    return;
  }

  _full = true;

  std::stringstream os;

  os << "Disk " << _path << " is below the " << _freeThreshold
     << " percent free threshold!\n"
     << "Free space is at " << _currentFree << " percent.";
  sendAlert(os.str());
}

void
DiskspaceWatcher::onCancel()
{
  LOG4CXX_INFO(logger, "Disk Watcher has been shut down.");
}

}
