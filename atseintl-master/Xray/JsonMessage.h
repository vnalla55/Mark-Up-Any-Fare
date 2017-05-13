//----------------------------------------------------------------------------
//
//  File:               JsonMessage.h
//  Description:        Single JSON message for Xray
//
//  Copyright Sabre 2016
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"

#include <cstdint>
#include <map>
#include <sstream>
#include <string>

namespace tse
{
namespace xray
{
constexpr int64_t MILLISECONDS_IN_SECOND = 1000;
constexpr int64_t LLS_TIMEOUT = -1;
class JsonMessage
{
  struct LlsData
  {
    std::string mid;
    int64_t llsNetworkTime = LLS_TIMEOUT;
  };

public:
  JsonMessage(const std::string& mid, const std::string& cid);
  void setId(const std::string& id) { _id = id; }
  void setInstance(const std::string& instance) { _instance = instance; }
  void setService(const std::string& service) { _service = service; }
  void setError(const std::string& error) { _error = error; }
  void setHostname(const std::string& hostname) { _hostname = hostname; }
  void setStartDate(const DateTime& startDate);
  void setStartDate(const DateTime& startDate, const std::string& timeZoneOffset);
  void setEndDate(const DateTime& endDate);
  void setHurryUpEndDate(const DateTime& hurryUpEndDate) { _hurryUpEndDate = hurryUpEndDate; }
  std::string getMid() const { return _mid; }
  std::string getCid() const { return _cid; }
  std::string getId() const { return _id; }
  void appendStringStream(std::ostringstream& strStream);
  std::string generateConversationId();
  std::string generateMessageId(const std::string& serviceName);
  void setLlsNetworkTime(const std::string& serviceName, const int64_t netTime);

private:
  std::map<std::string, LlsData> _llsData;
  const std::string _mid;
  const std::string _cid;
  std::string _id;
  std::string _error;
  std::string _service;
  std::string _instance;
  uint64_t _totaltime = 0;
  uint64_t _postHurryUpPeriod = 0;
  std::string _timestamp;
  std::string _hostname;
  DateTime _startDate;
  DateTime _endDate;
  DateTime _hurryUpEndDate;

  void generateTimestamp(const std::string& timeZoneOffset);
  void calculateTotalTime();
};

using JsonMessagePtr = std::unique_ptr<JsonMessage>;
}
}
