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

#include <boost/array.hpp>

#include <string>

#include <curl/curl.h>
#include <cstdint>

namespace tse
{

class BigIPClient
{
public:
  BigIPClient();

  void init(const std::string& bigIPHost, const std::string& user, const std::string& pass,
            const std::string& pooname, const std::string& memberIP);

  bool disableMember(long memberPort, long timeoutInSeconds);

  bool isInitialized() const { return !_memberIP.empty() && !_url.empty() && !_poolName.empty(); }
  const std::string& getErrorMessage() const { return _errorMessage; }

protected:
  bool setError(CURL *handle, const std::string& errorMessage);
  bool setError(CURL *handle, CURLoption option);
  bool setErrorFromResponse();

private:
  std::string _poolName;
  std::string _memberIP;
  std::string _url;
  std::string _errorMessage;
  std::string _response;
  boost::array<char, CURL_ERROR_SIZE + 1> _curlErrorBuffer;

  static size_t writeResponse(void *buffer, size_t size, size_t nmemb, void *userp);
};

}

