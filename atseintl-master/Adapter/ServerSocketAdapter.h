//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#pragma once

#include "Adapter/Adapter.h"
#include "Adapter/BigIP.h"
#include "ClientSocket/EOSocket.h"
#include "Common/DateTime.h"

#include <vector>

namespace tse
{
class ServerSocketAdapter;
class TseServer;
class BigIPClient;

class ServerSocketAdapter final : public Adapter
{
public:
  ServerSocketAdapter(const std::string&, TseServer&) {}
  ~ServerSocketAdapter() override;

  //-------------------------------------------------------------------------
  // @function ServerSocketAdapter::run
  // Description: Handle a client connection request
  // @param index - socket index
  // @return - void
  //-------------------------------------------------------------------------
  eo::Socket* run(uint32_t index, DateTime& startTime);

  void shutdown() override;

  //-------------------------------------------------------------------------
  // @function ServerSocketAdapter::getIOBufSize
  //
  // Description: Get the adapters idea of an IO_BUF_SIZE
  //
  // @param none
  // @return - the IO buffer size
  //-------------------------------------------------------------------------
  uint32_t getIOBufSize() { return _ioBufSize; }

  //-------------------------------------------------------------------------
  // @function ServerSocketAdapter::getTimeout
  //
  // Description: Get the adapters idea of a TIMEOUT value
  //
  // @param none
  // @return - the timeout value
  //-------------------------------------------------------------------------
  uint16_t getTimeout() { return _timeout; }

  //-------------------------------------------------------------------------
  // @function ServerSocketAdapter::getPort
  //
  // Description: Get the adapters idea of an Port
  //
  // @param none
  // @return - the Port
  //-------------------------------------------------------------------------
  uint16_t getPort() { return _port; }

  //-------------------------------------------------------------------------
  // @function ServerSocketAdapter::setIOR
  //
  // Description: Set the 'IOR' (host:port) filename
  //
  // @param ior filename
  // @return none
  //-------------------------------------------------------------------------
  void setIOR(const std::string& ior) { _ior = ior; }

  //-------------------------------------------------------------------------
  // @function ServerSocketAdapter::getNumListeners
  //
  // Description: Get the number of sockets configured to listen
  //
  // @param none
  // @return - Number of listeners
  //-------------------------------------------------------------------------
  uint32_t getNumListeners() { return _numListeners; }

  void onTrxLimitExceeded();

  void generateIOR();
  bool isAnyBigIPRegistered() const;

private:
  bool initialize() override;

  eo::Socket* volatile* _sockets = nullptr;
  bool _linger = false;
  bool _keepAlive = false;
  volatile bool _exiting = false;
  uint32_t _ioBufSize = 0;
  uint16_t _timeout = 0;
  uint16_t _port = 0;
  uint32_t _numListeners = 1;
  std::string _ior;
  std::string _hostname;
  std::string _realHostname;
  std::string _bigip;
  std::string _poolname;
  std::string _systype;
  uint16_t _connlimit = 0;
  std::string _connlimitBasis;

  BigIPClient* _bigIPClient = nullptr;
  BigIP* _bigInterface = nullptr;
  bool _bigRegistered = false;
  std::vector<bool> _bigIPsRegistered;
  bool _bTrxLimitExceeded = false;

  bool openSocket(uint32_t index, bool withBigIP = false);
  void closeSocket(uint32_t index, bool withBigIP = false);
  void deleteSocket(uint32_t index);

  bool split(const std::string& in, char sep, std::string& out1, std::string& out2) const ;
  bool getBigIPCredentials(const std::string& host, std::string& user, std::string& pass);
  bool parseBigIPPoolName(
      std::string& pool, std::string& user, std::string& pass, std::string& device);
  bool getHostIP(std::string& hostIP);
  void initBigIPClient();

  static bool
  writeIOR(const std::string& ior, const std::string& host, const std::vector<uint16_t>& ports);
};
} // namespace tse
