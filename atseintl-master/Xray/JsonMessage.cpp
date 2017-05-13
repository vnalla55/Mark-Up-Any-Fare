// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include "Xray/JsonMessage.h"

#include "Xray/XrayUtil.h"

#include <random>

#include <ctime>
#include <unistd.h>

namespace tse
{
namespace xray
{
namespace
{
constexpr uint16_t MAX_HOST_NAME = 255;
constexpr uint8_t ID_LENGTH = 8;

inline std::string
generateId()
{
  static std::random_device rd;
  const static std::string alphanums =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string id;

  for (uint8_t i = 0; i < ID_LENGTH; i++)
    id += alphanums[rd() % alphanums.length()];

  return id;
}

static const std::string&
getHostname()
{
  char buffer[MAX_HOST_NAME + 1];
  static const std::string hostname = ::gethostname(buffer, MAX_HOST_NAME) < 0 ? "unknown" : buffer;
  return hostname;
}

std::string
getTimeZoneOffset(const DateTime& date)
{
  time_t ts = to_time_t(date);
  struct tm t;
  char buf[16];
  ::localtime_r(&ts, &t);
  ::strftime(buf, sizeof(buf), "%z", &t);
  return std::string(buf);
}
}

JsonMessage::JsonMessage(const std::string& mid, const std::string& cid)
  : _mid(mid), _cid(cid), _id(generateId())
{
}

void
JsonMessage::setStartDate(const DateTime& startDate)
{
  const std::string timeZoneOffset(getTimeZoneOffset(startDate));
  _startDate = startDate;
  generateTimestamp(timeZoneOffset);
}

void
JsonMessage::setStartDate(const DateTime& startDate, const std::string& timeZoneOffset)
{
  _startDate = startDate;
  generateTimestamp(timeZoneOffset);
}

void
JsonMessage::setEndDate(const DateTime& endDate)
{
  _endDate = endDate;
  calculateTotalTime();
}

void
JsonMessage::appendStringStream(std::ostringstream& strStream)
{
  if (_hostname.empty())
    _hostname = getHostname();
  if (_instance.empty())
    setInstance(_hostname);
  strStream << "{"
            << R"("cid":")" << _cid << "\""
            << ","
            << R"("mid":")" << _mid << "\""
            << ","
            << R"("id":")" << _id << "\""
            << ","
            << R"("ts":")" << _timestamp << "\""
            << ","
            << R"("host":")" << _hostname << "\""
            << ","
            << R"("service":")" << _service << "\""
            << ","
            << R"("inst":")" << _instance << "\""
            << ","
            << R"("totaltime":)" << _totaltime;

  if (_postHurryUpPeriod)
    strStream << ","
              << R"("postHurryUpTime":)" << _postHurryUpPeriod;

  if (directConnection.getValue())
  {
    strStream << ","
              << R"("sys":"atsev2")";
  }

  if (!_error.empty())
    strStream << ","
              << R"("error":")" << _error << "\"";

  if (!_llsData.empty())
  {
    std::string lls = R"(,"lls":[)";
    for (const auto& record : _llsData)
    {
      lls.append(R"({")");
      lls.append(record.first);
      lls.append(R"(":")");
      lls.append(record.second.mid);
      lls.append(R"(")");
      if (_llsData[record.first].llsNetworkTime != LLS_TIMEOUT)
        lls.append(R"(,"net":)" + std::to_string(_llsData[record.first].llsNetworkTime));
      lls.append(R"(},)");
    }
    lls.pop_back();
    lls.push_back(']');
    strStream << lls;
  }

  strStream << "}";
}

std::string
JsonMessage::generateConversationId()
{
  return _cid + "." + _id;
}

std::string
JsonMessage::generateMessageId(const std::string& serviceName)
{
  std::string llsMid = generateId();
  LlsData data;
  data.mid = llsMid;
  _llsData[serviceName] = data;
  return llsMid;
}

void
JsonMessage::setLlsNetworkTime(const std::string& serviceName, const int64_t netTime)
{
  const auto iter = _llsData.find(serviceName);
  if (iter != _llsData.end())
    iter->second.llsNetworkTime = netTime;
}

void
JsonMessage::generateTimestamp(const std::string& timeZoneOffset)
{
  _timestamp = boost::posix_time::to_iso_extended_string(_startDate); // YYYY-MM-DDTHH:MM:SS.ssssss
  _timestamp.erase(23); // YYYY-MM-DDTHH:MM:SS.sss
  _timestamp.append(timeZoneOffset); // YYYY-MM-DDTHH:MM:SS.sss+XXXX
}

void
JsonMessage::calculateTotalTime()
{
  boost::posix_time::time_period transactionPeriod(_startDate, _endDate);
  _totaltime = transactionPeriod.length().total_milliseconds();
  if (!_hurryUpEndDate.isEmptyDate())
  {
    boost::posix_time::time_period postHurryUpPeriod(_hurryUpEndDate, _endDate);
    if (!postHurryUpPeriod.is_null())
      _postHurryUpPeriod = postHurryUpPeriod.length().total_milliseconds();
  }
}
}
}
