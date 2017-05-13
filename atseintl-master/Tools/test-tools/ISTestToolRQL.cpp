#define _MULTI_THREADED


#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> /* close() */
#include <errno.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

static const int BUFFERSIZE(10000);
static const int CMD_SIZE(4);
namespace
{
  std::string _fileName;
  size_t _pos(0);
  bool _bFinished(false);
  std::string _content;
  struct hostent *_hostInfo = 0;
  unsigned short int _serverPort(0);
  void *run (void *arg);
  void writeRequest (const std::string &request);

  pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
}

extern int errno;

int main (int argc,
          char **argv)
{
  string host;
  if (argc > 1)
  {
    host.assign(argv[1]);
  }
  else
  {
    cerr << "Enter server host name or IP address: ";
    char buf[BUFFERSIZE] = "";
    cin.get(buf, BUFFERSIZE - 1, '\n');
    host.assign(buf);
  }
  // gethostbyname() takes a host name or ip address in "numbers and
  // dots" notation, and returns a pointer to a hostent structure,
  // which we'll need later.  It's not important for us what this
  // structure is actually composed of.
  _hostInfo = gethostbyname(host.c_str());
  if (_hostInfo == 0)
  {
    cerr << "problem interpreting host:" << host << endl;
    return 1;
  }
  if (argc > 2)
  {
    _serverPort = atoi(argv[2]);
  }
  else
  {
    cerr << "Enter server port number: ";
    cin >> _serverPort;
    char c;
    cin.get(c); // dispose of the newline
  }
  if (argc > 2)
  {
    _serverPort = atoi(argv[2]);
  }
  if (argc > 3)
  {
    _fileName.assign(argv[3]);
  }
  cerr << "host=" << host << ",_serverPort=" << _serverPort << ",_fileName:" << _fileName << endl;
  try
  {
    std::ifstream ifile(_fileName.c_str());
    if (!ifile)
    {
      std::cerr << "Error opening file:" << _fileName << std::endl;
    }
    char buffer[BUFFERSIZE];
    while (ifile)
    {
      memset(buffer, 0, sizeof(buffer));
      ifile.read(buffer, BUFFERSIZE - 1);
      _content.append(buffer);
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception:" << e.what() << std::endl;
  }
  int total(0);
  while (!_bFinished)
  {
    const int numThreads(10);
    pthread_t id[numThreads];
    for (int i = 0; i < numThreads; ++i)
    {
      char buf[4] = { 0, 0, 0, 0 };
      sprintf(buf, "%d", i);
      if (pthread_create(&id[i], NULL, run, (void *) buf) != 0)
      {
        perror("pthread_create");
        return 1;
      }
      cerr << "started " << i << endl;
    }
    for (int j = 0; j < numThreads; ++j)
    {
      pthread_join(id[j], NULL);
      cerr << "joined " << j << endl;
    }
    total += numThreads;
    std::cerr << "********* total=" << total << " ****************" << std::endl;
  }
  return 0;
}

namespace
{
  void *run (void *arg)
  {
    std::string beg("<ShoppingRequest"),
                end("</ShoppingRequest>");
    size_t begPos(std::string::npos),
           endPos(std::string::npos);
    {
      pthread_mutex_lock(&_mutex);
      begPos = _content.find(beg, _pos);
      if (std::string::npos == begPos)
      {
        pthread_mutex_unlock(&_mutex);
        _pos = 0;
        return 0;
      }
      endPos = _content.find(end, begPos);
      if (std::string::npos == endPos)
      {
        pthread_mutex_unlock(&_mutex);
        _pos = 0;
        return 0;
      }
      _pos = endPos;
      //std::string request(_content.substr(begPos, endPos - begPos + end.length()));
      //writeRequest(request);
      pthread_mutex_unlock(&_mutex);
    }
    std::string request(_content.substr(begPos, endPos - begPos + end.length()));
    //std::cerr << request << std::endl;
    int socketDescriptor;
    struct sockaddr_in localAddress, serverAddress;
    // Create a socket.  "AF_INET" means it will use the IPv4 protocol.
    // "SOCK_STREAM" means it will be a reliable connection (i.e., TCP,
    // for UDP use SOCK_DGRAM), and I'm not sure what the 0 for the last
    // parameter means, but it seems to work.
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor < 0)
    {
      cerr << "cannot create socket" << endl;
      return 0;
    }
    // Bind the socket to any local port number.  First we have to set
    // some fields in the localAddress structure.  AF_INET means the
    // IPv4 protocol, as above.  0 means choose whatever port the
    // system wants to give us.  I'm not sure what the INADDR_ANY is
    // for.  htonl() and htons() convert long int and short int,
    // respectively, from host byte order (big or little endian) to
    // Internet standard network byte order.
    localAddress.sin_family = AF_INET;
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddress.sin_port = htons(0);

    if (bind(socketDescriptor,
             (struct sockaddr *) &localAddress,
             sizeof(localAddress)) < 0)
    {
      cerr << "cannot bind socket" << endl;
      return 0;
    }
    // Connect to server.  First we have to set some fields in the
    // serverAddress structure.  
    serverAddress.sin_family = _hostInfo->h_addrtype;
    memcpy((char *) &serverAddress.sin_addr.s_addr,
           _hostInfo->h_addr_list[0],
           _hostInfo->h_length);
    serverAddress.sin_port = htons(_serverPort);
				
    if (connect(socketDescriptor,
                (struct sockaddr *) &serverAddress,
                sizeof(serverAddress)) < 0)
    {
      cerr << "cannot connect" << endl;
      char errorStr[BUFFERSIZE] = { 0 };
      strerror_r(errno, errorStr, BUFFERSIZE - 1);
      cerr << errorStr << endl;
      return 0;
    }
    //cerr << "request=" << request << endl;
    // write header and request to the socket
    // Prepare record header
    const int headerSize(12);
    char header[headerSize + 1];
    memset(header, 0, headerSize + 1);
    // "size" field
    unsigned int hSize = request.size() + headerSize;
    unsigned int nSize = htonl(hSize);  
    memcpy(header, &nSize, 4);
    // "command" field
    string cmd("RQST");
    const char* src = cmd.c_str();
    for (int idx = 4; idx < 8; ++idx)
    {
      header[idx] = *src;
      if (*src != '\0')
        ++src;
    }
    // "version" field
    std::string ver("2.0");
    src = ver.c_str();
    for (int idx = 8; idx < 12; ++idx)
    {
      header[idx] = *src;
      if (*src != '\0')
        ++src;
    }
    // Send the header
    if (write(socketDescriptor, header, headerSize) < 0)
    {
      cerr << "Write failed" << endl;
      return 0;
    }
    int bytesWritten(0);
    if ((bytesWritten = write(socketDescriptor, request.c_str(), request.length())) < 0)
    {
      cerr << "Write failed:bytesWritten=" << bytesWritten << endl;
      return 0;
    }
    // Read the header
    memset(header, 0, headerSize + 1);
    int readBytes(0);
    if ((readBytes = read(socketDescriptor, header, headerSize)) != headerSize)
    {
      cerr << "error reading header:readBytes=" << readBytes << endl;
      return 0;
    }
    // size
    unsigned long responseLength(ntohl(*reinterpret_cast<unsigned long *>(header)));
    responseLength -= headerSize;
    cerr << "in header:responseLength=" << responseLength << endl;
    // version
    string version(header + 8, 4);
    cerr << "version=" << version << endl;
    // command
    cmd.assign(header + 4, 4);
    cerr << "cmd=" << cmd << endl;
    // Read the response back from the server
    std::string response;
    long total(0);
    vector<char> buffer(responseLength);
    if (responseLength > 0)
    {
      long bytesRead(0);
      while ((bytesRead = read(socketDescriptor, &buffer[0] + total, responseLength - total)) > 0)
      {
        total += bytesRead;
      }
      response.assign(&buffer[0], total);
    }
    cerr << "total=" << total << endl;
    //cerr << "response:\n" << response << std::endl;
    close(socketDescriptor);
    return 0;
  }

  void writeRequest (const std::string &request)
  {
    std::ofstream file("esv.xml");
    file.write(request.c_str(), request.length());
  }
}
