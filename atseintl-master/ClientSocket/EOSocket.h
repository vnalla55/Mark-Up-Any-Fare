// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include <netinet/in.h>

#include <string>

namespace eo
{
const int MAX_SOCKET_BUFFER_SIZE = 49152; // 48K (48*1024) (Now up to 256K)
const long ERROR_BUFFER_SIZE = 1024;
const int BACKLOG_LIMIT = 4096;

enum BufferType
{
  SENDBUF = 1,
  RECVBUF,
  BOTH
};

const long READ_TIMEDOUT = -1;
const long READ_ERROR = -99;

const long WRITE_TIMEDOUT = -1;
const long WRITE_ERROR = -99;

const long SEC_MAX = 100000000;
const long USEC_MAX = 1000000;

class Socket final
{
  template <typename Type>
  bool setSocketOption(int option, const char* name, const Type& value);

  template <typename Type>
  bool getSocketOption(int option, const char* name, Type& value);

private:
  sockaddr_in _clientAddress; // For listen sockets

  long _bytesRead = 0;
  long _bytesWritten = 0;
  long _t_sec = 0;
  long _t_usec = 0; // Timeout seconds, microseconds
  int _channel = -1;
  char _errorMessage[ERROR_BUFFER_SIZE];

  void setupTimeouts(const long sec, const long micros, struct timeval& timeouts);

public:
  // Constructors/Destructors
  Socket();
  Socket(const int existingChannel, const sockaddr_in* clientAddress);
  ~Socket();

  // I/O methods
  long read(void* data, const long size);
  long write(const void* data, const long size);

  long read(void* data, const long size, const long seconds, const long microseconds);
  long write(const void* data, const long size, const long seconds, const long microseconds);

  long read(void* data, const long size, const long seconds, const long microseconds, long& read);
  long write(const void* data,
             const long size,
             const long seconds,
             const long microseconds,
             long& written);

  // Other methods
  bool connect(const std::string& host, const uint16_t port);
  bool connect(const std::string&, const uint16_t port, const int timeout);
  bool connect(const std::string& host,
               const uint16_t port,
               const long seconds,
               const long microseconds);
  bool reConnect(const std::string& host, const uint16_t port);

  bool listen(const char* serviceName, const char* protocol);
  bool listen(const uint16_t port, const int backlog = BACKLOG_LIMIT);
  int accept();

  bool open();
  void close();
  void shutdown();

  // Socket Attribute methods
  bool setBufferSize(const BufferType, const int size);
  bool setLinger(const bool linger = false, const int time = 0);
  bool setKeepAlive(const bool = true);

  void setTimeOut(const long = -1, const long = -1);
  int getChannel() { return _channel; }
  sockaddr_in& getClientSocket() { return _clientAddress; }

  int getLastError(const char** errMsg); // Returns errno

  static bool getIPAddress(const std::string& host, struct in_addr& output);
};
}

