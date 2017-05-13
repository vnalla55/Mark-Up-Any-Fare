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

#include "ClientSocket/EOSocket.h"

#include "Common/Logger.h"
#include "Util/BranchPrediction.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <string>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

namespace eo
{
namespace
{
struct ErrnoFormatted
{
  ErrnoFormatted() {}

  friend std::ostream& operator<<(std::ostream& os, const ErrnoFormatted&)
  {
    const int code = errno;
    char buffer[128];
    // NOTE This is GNU-specific strerror_r that returns char*.
    return os << ::strerror_r(code, buffer, sizeof buffer) << " [errno " << code << "]";
  }
};

const ErrnoFormatted errnoFormatted;

inline void
valueToString(char buffer[32], int value)
{
  std::snprintf(buffer, 32, "%d", value);
}

inline void
valueToString(char buffer[32], const struct linger& value)
{
  std::snprintf(buffer, 32, "(%d, %d)", value.l_onoff, value.l_linger);
}

inline void
timevalSum(struct timeval& t, const struct timeval& a, const struct timeval& b)
{
  t.tv_sec = a.tv_sec + b.tv_sec;
  t.tv_usec = a.tv_usec + b.tv_usec;
  if (t.tv_usec >= USEC_MAX)
  {
    t.tv_usec -= USEC_MAX;
    t.tv_sec += 1;
  }
}

inline void
timevalDifference(struct timeval& t, const struct timeval& a, const struct timeval& b)
{
  t.tv_sec = a.tv_sec - b.tv_sec;
  t.tv_usec = a.tv_usec - b.tv_usec;
  if (t.tv_usec < 0)
  {
    t.tv_usec += USEC_MAX;
    t.tv_sec -= 1;
  }
}

inline void
updateTimeout(struct timeval& timeout,
              const struct timeval& endTime,
              const bool shouldCheckTimeOut = true)
{
  if (!shouldCheckTimeOut)
    return;

  struct timeval now;
  gettimeofday(&now, nullptr);
  timevalDifference(timeout, endTime, now);
}
}

static tse::Logger
logger("atseintl.Socket");

template <typename Type>
bool
Socket::setSocketOption(int option, const char* name, const Type& value)
{
  if (LIKELY(::setsockopt(_channel, SOL_SOCKET, option, &value, socklen_t(sizeof value)) == 0))
    return true;

  char buffer[32];
  valueToString(buffer, value);

  LOG4CXX_ERROR(logger,
                "Socket - setsockopt() - SO_" << name << " = " << buffer
                                              << " failed: " << errnoFormatted);
  std::snprintf(_errorMessage,
                sizeof _errorMessage,
                "%s(%d): setsockopt() - %s = %s failed!",
                __FILE__,
                __LINE__,
                name,
                buffer);

  return false;
}

template <typename Type>
inline bool
Socket::getSocketOption(int option, const char* name, Type& value)
{
  socklen_t size = socklen_t(sizeof value);
  if (::getsockopt(_channel, SOL_SOCKET, option, &value, &size) == 0)
    return true;

  LOG4CXX_ERROR(logger, "Socket - getsockopt() to SO_" << name << " failed: " << errnoFormatted);
  std::snprintf(_errorMessage,
                sizeof _errorMessage,
                "%s(%d): getsockopt() - %s failed!",
                __FILE__,
                __LINE__,
                name);

  return false;
}

Socket::Socket()
{
  std::memset(_errorMessage, 0x0, ERROR_BUFFER_SIZE);

  if (!open())
  {
    LOG4CXX_ERROR(logger, "Socket Constructor failed")
    throw std::string(_errorMessage);
  }

  // Note:
  //		We should think about and make a decision on this linger stuff
  // In light of ...
  //     Setting the linger time to 0 and setting it nonzero are completely
  //     different.  The former sends a TCP RST when you close(), and
  //     discards any queued data.  The latter sends a TCP FIN and waits for
  //     all queued data to be sent and ACKed or for the linger time to
  //     expire.  Unfortunately it still depends on the implementation as to
  //     what a nonzero linger time means.  4.4BSD interprets the value as
  //     clock ticks (yuck) but I think Posix.1g specifies the units as
  //     seconds (much better).
  //         Rich Stevens

  // Currently making all Sockets non-lingering
  setLinger(false);
}

Socket::Socket(const int existingChannel, const sockaddr_in* clientAddress)
  : _channel(existingChannel)
{
  std::memset(_errorMessage, 0x0, ERROR_BUFFER_SIZE);

  std::memcpy(&_clientAddress, clientAddress, sizeof(sockaddr_in));

  // Currently making all Sockets non-lingering (see above note).
  setLinger(false);
}

Socket::~Socket()
{
  close();
}

bool
Socket::open()
{
  if (_channel < 0)
  {
    if ((_channel = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      LOG4CXX_ERROR(logger, "Socket - socket() failed: " << errnoFormatted)
      std::snprintf(_errorMessage,
                    sizeof _errorMessage,
                    "%s(%d): Failed to create socket.",
                    __FILE__,
                    __LINE__);
      return false;
    }
  }
  return true;
}

void
Socket::close()
{
  ::close(_channel);
  _channel = -1;
}

void
Socket::shutdown()
{
  ::shutdown(_channel, SHUT_RD);
}

bool
Socket::connect(const std::string& host, const uint16_t port)
{
  return connect(host, port, _t_sec, _t_usec);
}

bool
Socket::connect(const std::string& host, const uint16_t port, const int timeout)
{
  return connect(host, port, timeout, 0);
}

bool
Socket::connect(const std::string& host,
                const uint16_t port,
                const long seconds,
                const long microseconds)
{
  const bool shouldCheckTimeOut = (seconds || microseconds);
  struct timeval timeout, endTime;
  if (shouldCheckTimeOut)
  {
    struct timeval now;
    gettimeofday(&now, nullptr);
    setupTimeouts(seconds, microseconds, timeout);
    timevalSum(endTime, now, timeout);
  }

  // in case we closed the channel
  if (!open())
    return false;

  struct sockaddr_in address;
  std::memset(&address, 0, sizeof address);

  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if (!getIPAddress(host, address.sin_addr))
  {
    std::snprintf(_errorMessage,
                  sizeof _errorMessage,
                  "%s(%d): Can't get IP address for host %s.",
                  __FILE__,
                  __LINE__,
                  host.c_str());
    return false;
  }

  int rc;
  rc = ::fcntl(_channel, F_SETFL, O_NONBLOCK);
  if (rc < 0)
  {
    LOG4CXX_ERROR(
        logger,
        "Couldn't set connecting socket to non-blocking. fcntl() failed: " << errnoFormatted)
    std::snprintf(_errorMessage,
                  sizeof _errorMessage,
                  "%s(%d): Couldn't set connecting socket to non-blocking.",
                  __FILE__,
                  __LINE__);
    return false;
  }

  rc = ::connect(_channel, (struct sockaddr*)&address, socklen_t(sizeof address));
  if (rc < 0)
  {
    // **** better be in progress
    if (errno != EINPROGRESS)
    {
      LOG4CXX_ERROR(logger, "Socket - connect() failed: " << errnoFormatted);
      std::snprintf(_errorMessage,
                    sizeof _errorMessage,
                    "%s(%d): Couldn't connect to socket.",
                    __FILE__,
                    __LINE__);
      return false;
    }
  }

  // Check to see if we should execute the select for a timeout
  if (shouldCheckTimeOut)
  {
    while (true)
    {
      fd_set wr_set;
      FD_ZERO(&wr_set);
      FD_SET(_channel, &wr_set);

      fd_set rd_set;
      FD_ZERO(&rd_set);
      FD_SET(_channel, &rd_set);

      errno = 0;
      rc = ::select(_channel + 1, &rd_set, &wr_set, nullptr, &timeout);
      if (rc <= 0)
      {
        if (errno == EINTR)
        {
          updateTimeout(timeout, endTime);
          continue;
        }

        LOG4CXX_ERROR(logger, "Socket - select() failed: " << errnoFormatted);
        std::snprintf(_errorMessage,
                      sizeof _errorMessage,
                      "%s(%d): Couldn't select on socket.",
                      __FILE__,
                      __LINE__);
        return false;
      }

      // **** check for error on socket
      if (FD_ISSET(_channel, &rd_set))
      {
        int error = 0;
        if (!getSocketOption(SO_ERROR, "ERROR", error))
          return false;
        if (error != 0)
        {
          LOG4CXX_ERROR(logger, "Socket - getsockopt(ERROR) returned " << error);
          std::snprintf(_errorMessage,
                        sizeof _errorMessage,
                        "%s(%d): getsockopt(ERROR) returned an error.",
                        __FILE__,
                        __LINE__);
          return false;
        }
      }

      // **** socket should be ready for writing
      if (!FD_ISSET(_channel, &wr_set))
      {
        LOG4CXX_ERROR(logger, "Socket is not ready for writing.");
        std::snprintf(_errorMessage,
                      sizeof _errorMessage,
                      "%s(%d): Socket is not ready for writing.",
                      __FILE__,
                      __LINE__);
        return false;
      }

      break;
    }
  }

  return true;
}

bool
Socket::reConnect(const std::string& host, const uint16_t port)
{
  if (_channel > 0)
    close();

  return connect(host, port);
}

bool
Socket::listen(const char* serviceName, const char* protocol)
{
  struct servent* entry;

  // Get the service port
  if ((entry = ::getservbyname(serviceName, protocol)) == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "Socket - getservbyname() failed for " << serviceName << ": " << errnoFormatted);
    std::snprintf(_errorMessage,
                  sizeof _errorMessage,
                  "%s(%d): couldn't find service - %s",
                  __FILE__,
                  __LINE__,
                  serviceName);
    return false;
  }

  return listen(uint16_t(entry->s_port));
}

bool
Socket::listen(const uint16_t listenPort, const int backlog)
{
  struct sockaddr_in address;

  // Since this is a listen socket we decided to
  // Bump up the receive buffer (I don't think that we need to do this).
  if (!setBufferSize(RECVBUF, MAX_SOCKET_BUFFER_SIZE))
    return false;

  // Make address reusable (BEFORE the bind() per R. Stephens)
  if (!setSocketOption(SO_REUSEADDR, "REUSEADDR", int(true)))
    return false;

  std::memset(&address, 0, sizeof address);
  address.sin_family = AF_INET;
  address.sin_port = htons(listenPort);
  address.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the address to the listening socket AFTER setting the SO_REUSEADDR
  // option per R. Stephens
  if (::bind(_channel, (struct sockaddr*)&address, socklen_t(sizeof(address))) < 0)
  {
    LOG4CXX_ERROR(logger, "Socket - bind() failed: " << errnoFormatted);
    std::snprintf(
        _errorMessage, sizeof _errorMessage, "%s(%d): bind() failed!", __FILE__, __LINE__);
    return false;
  }

  if (::listen(_channel, backlog) < 0)
  {
    LOG4CXX_ERROR(logger, "Socket - listen() failed: " << errnoFormatted);
    std::snprintf(
        _errorMessage, sizeof _errorMessage, "%s(%d): listen() failed!", __FILE__, __LINE__);
    return false;
  }

  return true;
}

int
Socket::accept()
{
  socklen_t addressLen = socklen_t(sizeof(_clientAddress));

  const int connectionSocket = ::accept(_channel, (struct sockaddr*)&_clientAddress, &addressLen);
  if (connectionSocket < 0)
  {
    LOG4CXX_ERROR(logger, "Socket - accept() failed: " << errnoFormatted);
  }
  return connectionSocket;
}

long
Socket::read(void* data, const long size)
{
  return read(data, size, _t_sec, _t_usec);
}

long
Socket::read(void* data, const long size, const long seconds, const long microseconds)
{
  long dummy;
  return read(data, size, seconds, microseconds, dummy);
}

// read() - Specific timeout
//
// Returns number of bytes read, 0 if the select times out, READ_ERROR if
// an error condition occurs, READ_INTERUPTED if READ_FINISHED (Need better
// name here) if the read would block.
//
// Known Revisions Pending:
// - Should probably check to see if the read would return an EWOULDBLOCK
//   before executing the select.
//
long
Socket::read(void* vdata, const long size, const long seconds, const long microseconds, long& read)
{
  char* data = static_cast<char*>(vdata);
  long left = size;

  const bool shouldCheckTimeOut = (seconds || microseconds);
  struct timeval timeout, endTime;
  if (shouldCheckTimeOut)
  {
    struct timeval now;
    gettimeofday(&now, nullptr);
    setupTimeouts(seconds, microseconds, timeout);
    timevalSum(endTime, now, timeout);
  }

  fd_set read_set, socket_set;
  FD_ZERO(&socket_set);
  FD_SET(_channel, &socket_set);

  read = 0;

  for (; left > 0; updateTimeout(timeout, endTime, shouldCheckTimeOut))
  {
    // Check to see if we should execute the select for a timeout
    if (shouldCheckTimeOut)
    {
      read_set = socket_set;

      errno = 0;
      const int status = ::select(_channel + 1, &read_set, nullptr, nullptr, &timeout);
      if (UNLIKELY(status == 0))
      {
        LOG4CXX_ERROR(logger,
                      "Socket - select() timed out after " << seconds << " sec "
                                                           << microseconds << " ms");
        std::snprintf(_errorMessage,
                      sizeof _errorMessage,
                      "%s(%d): select() timed out after %ld sec %ld ms",
                      __FILE__,
                      __LINE__,
                      seconds,
                      microseconds);
        return READ_TIMEDOUT;
      }

      if (UNLIKELY(status < 0))
      {
        if (errno == EINTR)
          continue;

        LOG4CXX_ERROR(logger, "Socket - select() failed on read: " << errnoFormatted);
        std::snprintf(
            _errorMessage, sizeof _errorMessage, "%s(%d): select() failed.", __FILE__, __LINE__);
        return READ_ERROR;
      }

      // I am assuming that since we only have 1 socket in the socket_set,
      // only activity on that socket, a timeout or an error will cause
      // the select() to return.  Therefore, I am not using FD_ISSET to
      // check for activity on Channel.  I am just assuming that if the
      // select returns with a value greater than 0, Channel is ready to
      // be read.
    }

    errno = 0;
    const long partSize = ::read(_channel, data + read, left);

    if (partSize > 0)
    {
      read += partSize;
      left -= partSize;
      _bytesRead += partSize;
      continue;
    }

    if (partSize == 0)
    {
      if (left == size)
      {
        LOG4CXX_INFO(logger, "Socket - connection has been closed by remote host.");
        std::snprintf(_errorMessage,
                      sizeof _errorMessage,
                      "%s(%d): Connection has been closed by remote host.",
                      __FILE__,
                      __LINE__);
        return READ_ERROR;
      }

      LOG4CXX_ERROR(logger, "Socket - connection went down before all bytes were read.");
      std::snprintf(_errorMessage,
                    sizeof _errorMessage,
                    "%s(%d): Connection went down before all bytes were read.",
                    __FILE__,
                    __LINE__);
      return READ_ERROR;
    }

    // Error condition
    switch (errno)
    {
    case EINTR: // read() interupted by a signal
      continue;

    case EWOULDBLOCK: // read() would block
      // If no timeout value, timeout here
      LOG4CXX_ERROR(logger,
                    "Socket - read() timed out after " << timeout.tv_sec << " sec "
                                                       << timeout.tv_usec << " ms");
      std::snprintf(_errorMessage,
                    sizeof _errorMessage,
                    "%s(%d): read() timed out after %ld sec %ld ms",
                    __FILE__,
                    __LINE__,
                    timeout.tv_sec,
                    timeout.tv_usec);
      return READ_TIMEDOUT;

    default: // Other error?
      LOG4CXX_ERROR(logger, "Socket - read() failed: " << errnoFormatted);
      std::snprintf(
          _errorMessage, sizeof _errorMessage, "%s(%d): read failed.", __FILE__, __LINE__);
      return READ_ERROR;
    }
  }

  return size;
}

long
Socket::write(const void* data, const long size)
{
  return write(data, size, _t_sec, _t_usec);
}

long
Socket::write(const void* data, const long size, const long seconds, const long microseconds)
{
  long dummy;
  return write(data, size, seconds, microseconds, dummy);
}

// write()
//
// Known Revisions Pending:
// - Should probably check to see if the write would return an EWOULDBLOCK
//   before executing the select.
//
long
Socket::write(
    const void* vdata, const long size, const long seconds, const long microseconds, long& written)
{
  const char* data = static_cast<const char*>(vdata);
  long left = size;

  const bool shouldCheckTimeOut = (seconds || microseconds);
  struct timeval timeout, endTime;
  if (UNLIKELY(shouldCheckTimeOut))
  {
    struct timeval now;
    gettimeofday(&now, nullptr);
    setupTimeouts(seconds, microseconds, timeout);
    timevalSum(endTime, now, timeout);
  }

  fd_set write_set, socket_set;
  FD_ZERO(&socket_set);
  FD_SET(_channel, &socket_set);

  // Should this be treated the same as the Read and block if it is unable to
  // write? I don't think so but we should probably think about this.

  written = 0;

  for (; left > 0; updateTimeout(timeout, endTime, UNLIKELY(shouldCheckTimeOut)))
  {
    // Check to see if we should execute the select for a timeout
    if (UNLIKELY(shouldCheckTimeOut))
    {
      write_set = socket_set;

      errno = 0;
      const int status = ::select(_channel + 1, nullptr, &write_set, nullptr, &timeout);
      if (status == 0)
      {
        LOG4CXX_ERROR(logger,
                      "Socket - select() timed out after " << seconds << " sec "
                                                           << microseconds << " ms");
        std::snprintf(_errorMessage,
                      sizeof _errorMessage,
                      "%s(%d): select() timed out after %ld sec %ld ms",
                      __FILE__,
                      __LINE__,
                      seconds,
                      microseconds);
        return WRITE_TIMEDOUT;
      }

      if (status < 0)
      {
        if (errno == EINTR)
          continue;

        LOG4CXX_ERROR(logger, "Socket - select() failed on write: " << errnoFormatted);
        std::snprintf(
            _errorMessage, sizeof _errorMessage, "%s(%d): select() failed", __FILE__, __LINE__);
        return WRITE_ERROR;
      }

      // I am assuming that since we only have 1 socket in the socket_set,
      // only activity on that socket, a timeout or an error will cause
      // the select() to return.  Therefore, I am not using FD_ISSET to
      // check for activity on Channel.  I am just assuming that if the
      // select returns with a value greater than 0, Channel is ready to
      // be written
      // to.
    }

    errno = 0;
    const long partSize = ::write(_channel, data + written, left);

    if (partSize >= 0)
    {
      written += partSize;
      left -= partSize;
      _bytesWritten += partSize;
      continue;
    }

    // Error condition
    switch (errno)
    {
    case EINTR:
    case EWOULDBLOCK:
      continue;
    default:
      LOG4CXX_ERROR(logger, "Socket - write() failed: " << errnoFormatted);
      std::snprintf(
          _errorMessage, sizeof _errorMessage, "%s(%d): write() failed!", __FILE__, __LINE__);
      return WRITE_ERROR;
    }
  }

  return size;
}

// setTimeOut() - Sets the timout of the select() on Reads. If the value of
//                both the seconds and microseconds is 0, the select() will not
//                be executed.  This is the default.
//
void
Socket::setTimeOut(const long seconds, const long microseconds)
{
  _t_sec = std::min(seconds, SEC_MAX);
  _t_usec = std::min(microseconds, USEC_MAX);
}

bool
Socket::getIPAddress(const std::string& host, in_addr& output)
{
  addrinfo hints;
  addrinfo* outAddr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  const int rc = ::getaddrinfo(host.c_str(), nullptr, &hints, &outAddr);

  if (rc)
  {
    LOG4CXX_ERROR(logger, "Failed to get IP of " << host << ": " << gai_strerror(rc));
    return false;
  }

  output = reinterpret_cast<sockaddr_in*>(outAddr->ai_addr)->sin_addr;
  ::freeaddrinfo(outAddr);
  return true;
}

bool
Socket::setBufferSize(const BufferType type, const int size)
{
  const int effectiveSize = std::min(size, MAX_SOCKET_BUFFER_SIZE);

  if (effectiveSize <= 0)
  {
    LOG4CXX_ERROR(logger, "Invalid argument to setBufferSize: " << size << ". Must be > 0.");
    std::snprintf(_errorMessage,
                  sizeof _errorMessage,
                  "%s(%d): Invalid argument - %d. Must be > 0",
                  __FILE__,
                  __LINE__,
                  effectiveSize);
    return false;
  }

  if (type == SENDBUF || type == BOTH)
  {
    if (!setSocketOption(SO_SNDBUF, "SNDBUF", effectiveSize))
      return false;
  }

  if (type == RECVBUF || type == BOTH)
  {
    if (!setSocketOption(SO_RCVBUF, "RCVBUF", effectiveSize))
      return false;
  }

  return true;
}

bool
Socket::setLinger(const bool linger, const int time)
{
  struct linger lingerOption;
  lingerOption.l_onoff = linger;
  lingerOption.l_linger = time;

  if (!setSocketOption(SO_LINGER, "LINGER", lingerOption))
    return false;

  return true;
}

bool
Socket::setKeepAlive(const bool flag)
{
  if (!setSocketOption(SO_KEEPALIVE, "KEEPALIVE", int(flag)))
    return false;

  return true;
}

int
Socket::getLastError(const char** errMsg)
{
  *errMsg = _errorMessage;
  return errno;
}

void
Socket::setupTimeouts(const long seconds, const long microseconds, struct timeval& timeout)
{
  timeout.tv_sec = std::max(0L, std::min(seconds, SEC_MAX));
  timeout.tv_usec = std::max(0L, std::min(microseconds, USEC_MAX));
}
}
