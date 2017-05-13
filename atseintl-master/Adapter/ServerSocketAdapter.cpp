//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Adapter/ServerSocketAdapter.h"

#include "Adapter/BigIPClient.h"
#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#include <arpa/inet.h>
#include <dirent.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace tse
{
static LoadableModuleRegister<Adapter, ServerSocketAdapter>
_("libServerSocketAdapter.so");

namespace
{
ConfigurableValue<bool>
linger("SERVER_SOCKET_ADP", "LINGER", false);
ConfigurableValue<bool>
keepAlive("SERVER_SOCKET_ADP", "KEEP_ALIVE", true);
ConfigurableValue<uint32_t>
ioBufSize("SERVER_SOCKET_ADP", "IO_BUF_SIZE", 40960);
ConfigurableValue<uint16_t>
timeout("SERVER_SOCKET_ADP", "TIMEOUT", 500);
ConfigurableValue<uint16_t>
port("SERVER_SOCKET_ADP", "PORT", 53501);
ConfigurableValue<std::string>
bigip("SERVER_SOCKET_ADP", "BIGIP", "");
ConfigurableValue<std::string>
poolname("SERVER_SOCKET_ADP", "POOLNAME", "");
ConfigurableValue<std::string>
systype("SERVER_SOCKET_ADP", "SYSTYPE", "");
ConfigurableValue<uint16_t>
connlimit("SERVER_SOCKET_ADP", "CONNLIMIT", 0);
ConfigurableValue<std::string>
connlimitBasis("SERVER_SOCKET_ADP", "CONNLIMIT_BASIS", "member");
ConfigurableValue<uint32_t>
numListeners("SERVER_SOCKET_ADP", "NUM_LISTENERS", 1);

constexpr uint16_t MAX_HOST_NAME = 255;
constexpr uint16_t MAX_DOMAIN_NAME = 255;
constexpr long BIGIP_DEREGISTRATION_TIMEOUT = 30;

Logger
logger("atseintl.Adapter.ServerSocketAdapter");

const char* BIGIP_LOG = "tsebigip.log";

const std::string
DEFAULT_ATSE_COMMON("/opt/atse/common");

const std::string
BIGIP_CRED_DIR("~/.bigip/hosts");
}

bool
ServerSocketAdapter::initialize()
{
  // Get config parameters
  _linger = linger.getValue();
  _keepAlive = keepAlive.getValue();
  _ioBufSize = ioBufSize.getValue();
  _timeout = timeout.getValue();
  _port = port.getValue();
  _bigip = bigip.getValue();
  _poolname = poolname.getValue();
  _systype = systype.getValue();
  _connlimit = connlimit.getValue();
  _connlimitBasis = connlimitBasis.getValue();
  _numListeners = numListeners.getValue();

  _sockets = new eo::Socket* [_numListeners];
  for (uint32_t i = 0; i < _numListeners; i++)
    _sockets[i] = nullptr;

  _bigIPsRegistered.assign(_numListeners, false);

  // Figure out the hostname
  {
    char hostname[MAX_HOST_NAME + 1];

    if (::gethostname(hostname, MAX_HOST_NAME) < 0)
    {
      LOG4CXX_FATAL(logger, "Call to gethostname() failed");
      return false;
    }

    _hostname = hostname;
    _realHostname = hostname;

    // If the hostname is not a fully qualified domain name then add the
    // domainname.
    if (_hostname.find('.') == std::string::npos)
    {
      char domainname[MAX_DOMAIN_NAME + 1];

      if (::getdomainname(domainname, MAX_DOMAIN_NAME) < 0)
      {
        LOG4CXX_FATAL(logger, "Call to getdomainname() failed");
        return false;
      }

      _hostname.append(".");
      _hostname.append(domainname);
    }
  }

  // create the BigIP interface object
  if ((!_bigip.empty()) && (!_poolname.empty()) && (!_systype.empty()) && (_port != 0))
  {
    _bigIPClient = new BigIPClient();

    _bigInterface = new BigIP(_bigip, _systype, BIGIP_LOG);
  }
  else
  {
    LOG4CXX_WARN(logger, "Not configured for BigIP registration.");
  }

  // Try to open the socket temporarily just to see if our port is available
  // We dont want to keep it open because we do not want to receive
  // transactions until run() is called
  bool rc = true;
  for (uint32_t i = 0; i < _numListeners; i++)
  {
    rc = openSocket(i);
    closeSocket(i);
    deleteSocket(i);

    if (!rc)
    {
      LOG4CXX_FATAL(logger, "Unable to open listening socket on port " << (_port + i));
      break;
    }
  }

  return rc;
}

eo::Socket*
ServerSocketAdapter::run(uint32_t index, DateTime& startTime)
{
  if (UNLIKELY(_sockets[index] == nullptr))
  {
    if (!openSocket(index, true))
    {
      closeSocket(index, true);
      deleteSocket(index);

      return nullptr;
    }
  }

  if (UNLIKELY(_exiting))
    return nullptr;

  int clientFd = _sockets[index]->accept();
  { // Start the transaction elapsed time clock as soon as possible
    startTime = boost::posix_time::microsec_clock::local_time();
    LOG4CXX_DEBUG(logger,
                  "Trx Start Time: " << boost::posix_time::to_iso_extended_string(startTime));
  }

  if (UNLIKELY(_exiting))
    return nullptr;

  if (UNLIKELY(clientFd < 0))
  {
    LOG4CXX_FATAL(logger, "Accept failed: " << clientFd);
    return nullptr;
  }
  else
  {
    eo::Socket* clientSocket = new eo::Socket(clientFd, &_sockets[index]->getClientSocket());
    LOG4CXX_DEBUG(logger,
                  "New client socket: " << clientSocket
                                        << ", channel: " << clientSocket->getChannel());
    return clientSocket;
  }
}

void
ServerSocketAdapter::shutdown()
{
  LOG4CXX_INFO(logger, "ServerSocketAdapter shutting down");
  _exiting = true;

  for (uint32_t i = 0; i < _numListeners; i++)
    closeSocket(i, true);
}

bool
ServerSocketAdapter::openSocket(uint32_t index, bool withBigIP)
{
  if (_bTrxLimitExceeded)
  {
    return false;
  }
  LOG4CXX_INFO(logger, "openSocket called socket=" << _sockets[index] << " _exiting=" << _exiting);
  if (_sockets[index] != nullptr)
    return true;

  _sockets[index] = new eo::Socket;

  // Set what we have so far
  if (_linger == true)
    _sockets[index]->setLinger(true, 1000);
  else
    _sockets[index]->setLinger(false);

  _sockets[index]->setKeepAlive(_keepAlive);

  // Go ahead and listen
  if (_sockets[index]->getChannel() < 0)
  {
    LOG4CXX_FATAL(logger, "getChannel failed!");
    return false;
  }

  // Only set IO_BUF_SIZE if greater than EO current value
  if (_ioBufSize > 0)
    _sockets[index]->setBufferSize(eo::BOTH, _ioBufSize);

  const uint16_t port = uint16_t(_port + index);

  // Do connect
  if (!_sockets[index]->listen(port))
  {
    LOG4CXX_FATAL(logger, "Listen failed!");
    return false;
  }

  // register with BigIP
  if (withBigIP && (_bigInterface != nullptr))
  {
    bool registered(false);
    for (int retries = 0; !registered && (retries < 3); retries++)
    {
      if (retries > 0)
        sleep(5);

      BigIP::STRINGVEC errs;
      registered =
          _bigInterface->registerMember(_poolname, port, _connlimit, _connlimitBasis, errs);
      if (registered)
      {
        LOG4CXX_INFO(logger,
                     "Registered port [" << port << "] in pool [" << _poolname
                                         << "] on BigIP device [" << _bigip
                                         << "] with connection limit [" << _connlimit << "] basis ["
                                         << _connlimitBasis << "].");
        _bigRegistered = true;

        if (_bigIPClient && !_bigIPClient->isInitialized())
        {
          initBigIPClient();
        }

        _bigIPsRegistered.at(index) = true;
      }
      else
      {
        std::string tagline;
        bool showErrors(false);

        if (access(BIGIP_LOG, F_OK) == 0)
        {
          tagline = "See ";
          tagline.append(BIGIP_LOG);
          tagline.append(" for more information.");
        }
        else if (errs.size() > 0)
        {
          tagline.append("More information follows...");
          showErrors = true;
        }

        LOG4CXX_ERROR(logger,
                      "Unable to register port [" << port << "] in pool [" << _poolname
                                                  << "] on BigIP device [" << _bigip << "]!  "
                                                  << tagline);

        if (showErrors)
        {
          for (BigIP::STRINGVEC::const_iterator s = errs.begin(); s != errs.end(); ++s)
          {
            LOG4CXX_ERROR(logger, "BIGIP: " << (*s));
          }
        }
      }
    }
  }

  return true;
}

void
ServerSocketAdapter::deleteSocket(uint32_t index)
{
  delete _sockets[index];
  _sockets[index] = nullptr;
}

void
ServerSocketAdapter::closeSocket(uint32_t index, bool withBigIP)
{
  if (_sockets[index] == nullptr)
    return;

  // Deregister from BigIP
  // if( ( _bigInterface != NULL ) && ( _bigRegistered ) )
  if (withBigIP && _bigInterface != nullptr)
  {
    const uint16_t port = uint16_t(_port + index);
    BigIP::STRINGVEC errs;
    if (_bigInterface->deregisterMember(_poolname, port, false, errs))
    {
      LOG4CXX_INFO(logger,
                   "Deregistered port [" << port << "] in pool [" << _poolname
                                         << "] on BigIP device [" << _bigip << "].");
      _bigRegistered = false;
      _bigIPsRegistered.at(index) = false;
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "Unable to deregister port [" << port << "] in pool [" << _poolname
                                                  << "] on BigIP device [" << _bigip << "]!  See "
                                                  << BIGIP_LOG << " for details.");
      for (BigIP::STRINGVEC::const_iterator s = errs.begin(); s != errs.end(); ++s)
      {
        LOG4CXX_ERROR(logger, "BIGIP: " << (*s));
      }

      if (_bigIPClient && _bigIPClient->isInitialized())
      {
        if (_bigIPClient->disableMember(port, BIGIP_DEREGISTRATION_TIMEOUT))
        {
          _bigIPsRegistered.at(index) = false;

          LOG4CXX_INFO(logger, "BigIPClient disabled port: " << port);
        }
        else
        {
          LOG4CXX_ERROR(logger, "BigIPClient error: " << _bigIPClient->getErrorMessage());
        }
      }
    }
  }

  _sockets[index]->shutdown();
  LOG4CXX_INFO(logger, "socket closed");
}

bool
ServerSocketAdapter::split(const std::string& in, char sep, std::string& out1, std::string& out2)
    const
{
  size_t sepPos = in.find(sep);
  if (sepPos == std::string::npos)
    return false;
  out1 = in.substr(0, sepPos);
  out2 = in.substr(sepPos + 1);
  return true;
}

bool
ServerSocketAdapter::getBigIPCredentials(const std::string& host,
                                         std::string& user,
                                         std::string& pass)
{
  const char* atseCommonEnv = getenv("ATSE_COMMON");
  std::string atseCommonDir = atseCommonEnv ? atseCommonEnv : DEFAULT_ATSE_COMMON;

  std::ostringstream cmd;
  cmd << "perl -I" << atseCommonDir << "/perl " << BIGIP_CRED_DIR << "/" << host << " 2>/dev/null";

  FILE* cmdPipe = popen(cmd.str().c_str(), "r");

  if (!cmdPipe)
  {
    LOG4CXX_ERROR(logger, "Failed to get BigIP credentials: " << strerror(errno));
    return false;
  }

  char cmdOutput[256];
  if (!fgets(cmdOutput, sizeof(cmdOutput), cmdPipe))
  {
    LOG4CXX_ERROR(logger, "Failed to read BigIP credentials.");
    pclose(cmdPipe);
    return false;
  }

  if (pclose(cmdPipe))
  {
    LOG4CXX_ERROR(logger, "Failed to close a pipe: " << strerror(errno));
  }

  if (!split(cmdOutput, ':', user, pass))
  {
    LOG4CXX_ERROR(logger, "Failed to parse BigIP credentials.");
    return false;
  }

  return true;
}

bool
ServerSocketAdapter::parseBigIPPoolName(std::string& pool,
                                        std::string& user,
                                        std::string& pass,
                                        std::string& device)
{
  std::string dev;
  std::string cred;

  if (!split(_poolname, '%', pool, dev))
  {
    LOG4CXX_ERROR(logger, "Failed to parse BigIP pool name: " << _poolname);
    return false;
  }

  if (!split(dev, '@', cred, device))
  {
    device = dev;
    return true;
  }

  split(cred, '/', user, pass);
  return true;
}

bool
ServerSocketAdapter::getHostIP(std::string& hostIP)
{
  in_addr address;
  if (!eo::Socket::getIPAddress(_realHostname, address))
    return false;

  char ipBuffer[INET_ADDRSTRLEN];
  const char* ntopRc = inet_ntop(AF_INET, &address, ipBuffer, INET_ADDRSTRLEN);

  if (!ntopRc)
  {
    LOG4CXX_ERROR(logger, "Failed to convert IP to string: " << strerror(errno));
    return false;
  }

  hostIP = ipBuffer;
  return true;
}

void
ServerSocketAdapter::initBigIPClient()
{
  std::string memberIP;

  if (!getHostIP(memberIP))
    return;

  LOG4CXX_INFO(logger, "Host IP: " << memberIP);

  std::string poolName;
  std::string bigIPHost;
  std::string bigIPUser;
  std::string bigIPPass;

  if (!parseBigIPPoolName(poolName, bigIPUser, bigIPPass, bigIPHost))
    return;

  if (bigIPUser.empty() && !getBigIPCredentials(bigIPHost, bigIPUser, bigIPPass))
    return;

  _bigIPClient->init(bigIPHost, bigIPUser, bigIPPass, poolName, memberIP);
  LOG4CXX_INFO(logger, "BigIPClient initialized");
}

ServerSocketAdapter::~ServerSocketAdapter()
{
  _exiting = true;
  for (uint32_t i = 0; i < _numListeners; i++)
  {
    closeSocket(i, true);
    deleteSocket(i);
  }

  delete[] _sockets;
  delete _bigInterface;
  delete _bigIPClient;
}

bool
ServerSocketAdapter::writeIOR(const std::string& ior,
                              const std::string& host,
                              const std::vector<uint16_t>& ports)
{
  if (ior.empty() || host.empty())
  {
    LOG4CXX_DEBUG(logger, "Cannot write IOR, filename or host is empty");
    return false;
  }

  // Write hostname:port to the IOR file for the landing zone
  std::ofstream oFile(ior.c_str());
  std::ostringstream oss;
  std::ostringstream portsoss;

  for (size_t i = 0; i < ports.size(); i++)
  {
    if (i != 0)
      oss << "\n";
    oss << host << ':' << ports[i];
    portsoss << ports[i] << " ";
  }

  LOG4CXX_DEBUG(logger, "Writing IOR values to '" << ior << "' of '" << oss.str() << "'");

  LOG4CXX_INFO(logger, "Used port [" << portsoss.str() << "] "); // temporary  check console
  // display port during startup
  oFile << oss.str();
  oFile.close();

  return true;
}

void
ServerSocketAdapter::onTrxLimitExceeded()
{
  _bTrxLimitExceeded = true;
}

void
ServerSocketAdapter::generateIOR()
{
  if (!_ior.empty())
  {
    std::vector<uint16_t> ports;
    if (_port == 0)
    {
      // Figure out what port we really got
      struct sockaddr_in server_addr;
      int addrlen = sizeof(server_addr);

      for (uint32_t i = 0; i < _numListeners; i++)
      {
        int waitSeconds = 5;
        while (_sockets[i] == nullptr && waitSeconds-- > 0)
          sleep(1);
        if (_sockets[i] == nullptr)
        {
          LOG4CXX_FATAL(logger, "Socket not initialized, not generating IOR");
          continue;
        }
        if (::getsockname(_sockets[i]->getChannel(),
                          (struct sockaddr*)&server_addr,
                          (socklen_t*)&addrlen) != 0)
        {
          LOG4CXX_FATAL(logger, "getsockname() failed!");
          return;
        }

        ports.push_back(ntohs(server_addr.sin_port));
      }
    }
    else
    {
      for (uint32_t i = 0; i < _numListeners; i++)
      {
        ports.push_back(uint16_t(_port + i));
      }
    }

    if (!writeIOR(_ior, _hostname, ports))
    {
      LOG4CXX_FATAL(logger, "Unable to write ior file '" << _ior << "'");
    }
  }
}

bool
ServerSocketAdapter::isAnyBigIPRegistered() const
{
  return std::find(_bigIPsRegistered.begin(), _bigIPsRegistered.end(), true) !=
         _bigIPsRegistered.end();
}
}
