//----------------------------------------------------------------------------
//
//     File:           ParameterSubstitutor.cpp
//     Description:    ParameterSubstitutor
//     Created:        07/01/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright 2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/ParameterSubstitutor.h"

#include "Common/Thread/TSELockGuards.h"
#include "DBAccess/ORACLEBoundParameterTypes.h"
#include "DBAccess/ParameterSubstitutorImpl.h"
#include "DBAccess/SQLQueryHelper.h"

using namespace DBAccess;

TSEFastMutex ParameterSubstitutor::_mutex;

ParameterSubstitutor::~ParameterSubstitutor() { clearBoundParameters(); }

void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const char* parm,
                                          int32_t index,
                                          bool forceLiteral)
{
  impl().substituteParameter(*this, sql, parm, index, forceLiteral);
}

void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const std::string& parm,
                                          int32_t index,
                                          bool forceLiteral)
{
  impl().substituteParameter(*this, sql, parm, index, forceLiteral);
}

void
ParameterSubstitutor::substituteParameter(std::string& sql, int64_t parm, int32_t index)
{
  impl().substituteParameter(*this, sql, parm, index);
}

void
ParameterSubstitutor::substituteParameter(std::string& sql, int parm, int32_t index)
{
  impl().substituteParameter(*this, sql, parm, index);
}

void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const tse::DateTime& parm,
                                          int32_t index,
                                          bool dateOnly)
{
  impl().substituteParameter(*this, sql, parm, index, dateOnly);
}

void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const std::vector<tse::CarrierCode>& parm,
                                          int32_t index)
{
  impl().substituteParameter(*this, sql, parm, index);
}

void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const std::vector<int64_t>& parm,
                                          int32_t index)
{
  impl().substituteParameter(*this, sql, parm, index);
}

/*
void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const std::vector<PaxTypeCode>& parm,
                                          int32_t index)
{
  impl().substituteParameter(*this, sql, parm, index);
}
*/

void
ParameterSubstitutor::substituteParameter(std::string& sql,
                                          const tse::DateTime& parm,
                                          const char* placeHolder,
                                          bool dateOnly)
{
  impl().substituteParameter(*this, sql, parm, placeHolder, dateOnly);
}

void
ParameterSubstitutor::bindAllParameters(const ParameterBinder& binder) const
{
  int32_t indexCtr = 1;

  BoundParameterCollection::const_iterator iter = _boundParameters.begin();
  BoundParameterCollection::const_iterator iterEnd = _boundParameters.end();

  for (; iter != iterEnd; ++iter, ++indexCtr)
  {
    (*iter)->index(indexCtr);
    (*iter)->bind(binder);
  }
}

const BoundParameterCollection&
ParameterSubstitutor::getBoundParameters() const
{
  return _boundParameters;
}

void
ParameterSubstitutor::clearBoundParameters()
{
  // Clean up the bound parameter collection
  //
  for (const auto elem : _boundParameters)
  {
    delete elem;
  }
  _boundParameters.clear();
}

const ParameterSubstitutorImpl&
ParameterSubstitutor::impl()
{
  if (!_impl)
  {
    // Paranoia inspired mutex lock.
    //
    const TSEGuard guard(_mutex);
    if (LIKELY(!_impl))
    {
      _impl = &(SQLQueryHelper::getSQLQueryHelper().getParameterSubstitutorImpl());
    }
  }
  return *_impl;
}

namespace
{

void collectParameters(const ParameterSubstitutor& parameterSubstitutor,
                       std::vector<std::string>& parameters)
{
  std::ostringstream oss;
  const BoundParameterCollection& boundParameters(parameterSubstitutor.getBoundParameters());
  for (const auto parm : boundParameters)
  {
    oss.str("");
    ORACLEBoundString* boundString(nullptr);
    ORACLEBoundInteger* boundInteger(nullptr);
    ORACLEBoundLong* boundLong(nullptr);
    ORACLEBoundDate* boundDate(nullptr);
    ORACLEBoundDateTime* boundDateTime(nullptr);
    if ((boundString = dynamic_cast<ORACLEBoundString*>(parm)))
    {
      oss << boundString->getBindBuffer();
    }
    else if ((boundInteger = dynamic_cast<ORACLEBoundInteger*>(parm)))
    {
      oss << *boundInteger->getBindBuffer();
    }
    else if ((boundLong = dynamic_cast<ORACLEBoundLong*>(parm)))
    {
      oss << *boundLong->getBindBuffer();
    }
    else if ((boundDate = dynamic_cast<ORACLEBoundDate*>(parm)))
    {
      char* bindBuffer = boundDate->getBindBuffer();
      int year = (bindBuffer[0] - 100) * 100 + (bindBuffer[1] - 100);
      oss << year << '-' << std::setw(2) << std::setfill('0') << ((int)bindBuffer[2])
          << '-' << std::setw(2) << std::setfill('0') << ((int)bindBuffer[3])
          << ' ' << std::setw(2) << std::setfill('0')
          << ((int)bindBuffer[4]) - 1 << ":" << std::setw(2) << std::setfill('0')
          << ((int)bindBuffer[5]) - 1 << ":" << std::setw(2) << std::setfill('0')
          << ((int)bindBuffer[5]) - 1;
    }
    else if ((boundDateTime = dynamic_cast<ORACLEBoundDateTime*>(parm)))
    {
      tse::DateTime& value(boundDateTime->getValue());
      int year(0);
      int month(0), day(0), hour(0), min(0), sec(0);
      long fsec(0);
      if (value.isInfinity())
      {
        year = 9999;
        month = 12;
        day = 31;
        hour = 23;
        min = 59;
        sec = 59;
        fsec = 999000000;
      }
      else if (value.isNegInfinity())
      {
        year = -4712;
        month = 1;
        day = 1;
      }
      else
      {
        year = value.year();
        month = value.month();
        day = value.day();
        hour = value.hours();
        min = value.minutes();
        sec = value.seconds();
        fsec = value.fractionalSeconds() * 1000;
      }
      char buf[128];
      snprintf(buf,
               sizeof(buf),
               "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.6ld",
               year,
               month,
               day,
               hour,
               min,
               sec,
               fsec / 1000);
      oss << buf;
    }
    std::string parameter(oss.str());
    parameters.push_back(parameter);
  }
}

}

void ParameterSubstitutor::fillParameterString(std::string& displayString) const
{
  std::ostringstream oss;
  std::vector<std::string> parameterStrings;
  collectParameters(*this, parameterStrings);
  int index(1);
  for (std::vector<std::string>::const_iterator it(parameterStrings.begin()),
                                                itend(parameterStrings.end());
       it != itend;
       ++it)
  {
    oss.str("");
    oss << index++;
    displayString += " (" + oss.str() + ") " + *it + ';';
  }
}

void ParameterSubstitutor::getSQLString(const std::string& orig, std::string& result) const
{
  std::vector<std::string> parameterStrings;
  collectParameters(*this, parameterStrings);
  size_t prevPos(0), pos(0);
  for (std::vector<std::string>::const_iterator it(parameterStrings.begin()),
                                                itend(parameterStrings.end());
       it != itend;
       ++it)
  {
    pos = orig.find(':', pos);
    if (std::string::npos == pos
        || pos < prevPos
        || pos >= orig.size() - 1)
    {
      return;
    }
    size_t length(++pos - prevPos);
    if (length > 0)
    {
      --length;
    }
    result += orig.substr(prevPos, length) + *it;
    prevPos = orig.find_first_of(" \n\t,()<=>;", pos + 1);
    if (std::string::npos == prevPos)
    {
      prevPos = pos;
    }
  }
  if (pos < orig.size())
  {
    pos = orig.find_first_of(" \n\t,()<=>;", pos);
    if (pos != std::string::npos)
    {
      result += orig.substr(pos);
    }
  }
}
