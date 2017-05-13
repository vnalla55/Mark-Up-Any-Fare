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

#include "Adapter/BigIPClient.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>

namespace tse
{
namespace
{
const char* disable_member_tmpl =
"<?xml version=\"1.0\"?>"
"    xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\""
"    xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\""
"    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
"    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
"  <SOAP-ENV:Body>"
"    <jsx1:set_session_enabled_state xmlns:jsx1=\"urn:iControl:LocalLB/PoolMember\">"
"      <pool_names xsi:type=\"SOAP-ENC:Array\" SOAP-ENC:arrayType=\"xsd:string[1]\">"
"        <item>xxxpoolxxx</item>"
"      </pool_names>"
"      <session_states xsi:type=\"SOAP-ENC:Array\" SOAP-ENC:arrayType=\"iControl:LocalLB.PoolMember.MemberSessionState[][1]\">"
"        <item SOAP-ENC:arrayType=\"iControl:LocalLB.PoolMember.MemberSessionState[1]\">"
"          <item>"
"            <member xsi:type=\"iControl:Common.IPPortDefinition\">"
"              <address xsi:type=\"xsd:string\">xxxmemberxxx</address>"
"              <port xsi:type=\"xsd:long\">xxxportxxx</port>"
"            </member>"
"            <session_state xsi:type=\"iControl:Common.EnabledState\">STATE_ENABLED</session_state>"
"          </item>"
"        </item>"
"      </session_states>"
"    </jsx1:set_session_enabled_state>"
"  </SOAP-ENV:Body> "
"</SOAP-ENV:Envelope>";
}

BigIPClient::BigIPClient()
{
  _curlErrorBuffer.assign(0);
}

void
BigIPClient::init(const std::string& bigIPHost, const std::string& user, const std::string& pass,
                  const std::string& poolname, const std::string& memberIP)
{
  std::ostringstream url;
  url << "https://" << user << ":" << pass << "@" << bigIPHost;
  if (bigIPHost.find(':') == std::string::npos)
    url << ":443";
  url << "/iControl/iControlPortal.cgi";

  _poolName = poolname;
  _memberIP = memberIP;
  _url = url.str();
}

bool
BigIPClient::disableMember(long memberPort, long timeoutInSeconds)
{
  std::string request(disable_member_tmpl);
  boost::replace_all(request, "xxxpoolxxx", _poolName.c_str());
  boost::replace_all(request, "xxxmemberxxx", _memberIP.c_str());
  boost::replace_all(request, "xxxportxxx", boost::lexical_cast<std::string>(memberPort));

  CURL *handle = curl_easy_init();
  if (!handle)
    return setError(nullptr, "Unable to initialize the CURL handle");

  // using defaults for VERBOSE=0, HEADER=0, NOPROGRESS=1
  if (curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, _curlErrorBuffer.c_array()) != CURLE_OK)
    return setError(handle, CURLOPT_ERRORBUFFER); 
  if( curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1) != CURLE_OK)
    return setError(handle, CURLOPT_NOSIGNAL);
  if( curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeoutInSeconds) != CURLE_OK)
    return setError(handle, CURLOPT_TIMEOUT);
  if( curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0) != CURLE_OK)
    return setError(handle, CURLOPT_SSL_VERIFYPEER);
  if( curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0) != CURLE_OK)
    return setError(handle, CURLOPT_SSL_VERIFYHOST);
  if( curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request.c_str()) != CURLE_OK)
    return setError(handle, CURLOPT_POSTFIELDS);
  if( curl_easy_setopt(handle, CURLOPT_URL, _url.c_str()) != CURLE_OK)
    return setError(handle, CURLOPT_URL);
  if( curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &BigIPClient::writeResponse) != CURLE_OK)
    return setError(handle, CURLOPT_WRITEFUNCTION); 
  if (curl_easy_setopt(handle, CURLOPT_WRITEDATA, this) != CURLE_OK)
    return setError(handle, CURLOPT_WRITEDATA);

  // now call BigIP
  CURLcode retCode = curl_easy_perform(handle);
  if (retCode != CURLE_OK)
    return setError(handle, _curlErrorBuffer.data());

  curl_easy_cleanup(handle);

  if (setErrorFromResponse())
    return false;

  return true;
}

bool
BigIPClient::setError(CURL *handle, const std::string& errorMessage)
{
  _errorMessage = errorMessage;

  if (handle)
    curl_easy_cleanup(handle);

  return false;
}

bool
BigIPClient::setError(CURL *handle, CURLoption option)
{
  switch(option)
  {
  case CURLOPT_ERRORBUFFER:
    _errorMessage = "Failed to set the CURLOPT_ERRORBUFFER option";
    break;
  case CURLOPT_VERBOSE:
    _errorMessage = "Failed to set the CURLOPT_VERBOSE option";
    break;
  case CURLOPT_HEADER:
    _errorMessage = "Failed to set the CURLOPT_HEADER option";
    break;
  case CURLOPT_TIMEOUT:
    _errorMessage = "Failed to set the CURLOPT_TIMEOUT option";
    break;
  case CURLOPT_NOPROGRESS:
    _errorMessage = "Failed to set the CURLOPT_NOPROGRESS option";
    break;
  case CURLOPT_NOSIGNAL:
    _errorMessage = "Failed to set the CURLOPT_NOSIGNAL option";
    break;
  case CURLOPT_SSL_VERIFYPEER:
    _errorMessage = "Failed to set the CURLOPT_SSL_VERIFYPEER option";
    break;
  case CURLOPT_SSL_VERIFYHOST:
    _errorMessage = "Failed to set the CURLOPT_SSL_VERIFYHOST option";
    break;
  case CURLOPT_POSTFIELDS:
    _errorMessage = "Failed to set the CURLOPT_POSTFIELDS option";
    break;
  case CURLOPT_URL:
    _errorMessage = "Failed to set the CURLOPT_URL option";
    break;
  case CURLOPT_WRITEFUNCTION:
    _errorMessage = "Failed to set the CURLOPT_WRITEFUNCTION option";
    break;
  case CURLOPT_WRITEDATA:
    _errorMessage = "Failed to set the CURLOPT_WRITEDATA option";
    break;
  default:
    _errorMessage.clear();
    break;
  }

  if (handle)
    curl_easy_cleanup(handle);

  return false;
}

bool
BigIPClient::setErrorFromResponse()
{
  size_t error_string_pos = _response.find("error_string");
  if (std::string::npos == error_string_pos)
    return false;

  size_t error_string_end_pos = _response.find("</faultstring", error_string_pos);
  if (std::string::npos == error_string_pos)
    return false;

  _errorMessage = _response.substr(error_string_pos, error_string_end_pos - error_string_pos);
  return true;
}

size_t
BigIPClient::writeResponse(void *buffer, size_t size, size_t nmemb, void *userp)
{
  size_t chunkSize = size * nmemb;
  BigIPClient* bigIPClient = static_cast<BigIPClient*>(userp);
  bigIPClient->_response.append(static_cast<char*>(buffer), chunkSize);
  return chunkSize;
}

}
