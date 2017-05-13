//----------------------------------------------------------------------------
//
//     File:           ORACLEBindingParameterSubstitutor.cpp
//     Description:
//     Created:        11/17/2009
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

#include "DBAccess/ORACLEBindingParameterSubstitutor.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DBAccess/BoundParameter.h"
#include "DBAccess/ORACLEBoundParameterTypes.h"
#include "DBAccess/ParameterSubstitutor.h"
#include "DBAccess/SQLStatementHelper.h"

#include <stdexcept>

namespace DBAccess
{

log4cxx::LoggerPtr
ORACLEBindingParameterSubstitutor::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEBindingParameterSubstitutor"));

ORACLEBindingParameterSubstitutor::ORACLEBindingParameterSubstitutor()
{
  LOG4CXX_INFO(log4cxx::Logger::getLogger("atseintl.Server.TseServer"),
               "ORACLEBindingParameterSubstitutor enabled.")
}

ORACLEBindingParameterSubstitutor::~ORACLEBindingParameterSubstitutor() {}

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       const char* parm,
                                                       int32_t index,
                                                       bool forceLiteral) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);
  size_t pos = sql.find(srch, 0);

  if (UNLIKELY(pos == std::string::npos))
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  if (LIKELY(!forceLiteral))
  {
    char bindVarLabel[16];
    snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

    sql.replace(pos, strlen(srch) + 1, bindVarLabel);
    substitutor.addBoundParameter<ORACLEBoundString>(-1, pos, parm);
  }
  else
  {
    if (sql[pos + strlen(srch)] == 'q')
    {
      if (strlen(parm) != 0)
      {
        sql.replace(pos, strlen(srch) + 1, "''");
        sql.insert(pos + 1, parm);
      }
      else
      {
        sql.replace(pos, strlen(srch) + 1, "' '");
      }
    }
    else
    {
      sql.replace(pos, strlen(srch) + 1, parm);
    }
  }
}

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       const std::string& parm,
                                                       int32_t index,
                                                       bool forceLiteral) const
{
  substituteParameter(substitutor, sql, parm.c_str(), index, forceLiteral);
}

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       int64_t parm,
                                                       int32_t index) const
{
  // Search for each %(number):(q)(n) next
  char srch[21];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (pos == std::string::npos)
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[21];
  snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

  sql.replace(pos, strlen(srch) + 1, bindVarLabel);

  substitutor.addBoundParameter<ORACLEBoundLong>(-1, pos, parm);
}

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       int parm,
                                                       int32_t index) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (UNLIKELY(pos == std::string::npos))
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[16];
  snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

  sql.replace(pos, strlen(srch) + 1, bindVarLabel);
  substitutor.addBoundParameter<ORACLEBoundInteger>(-1, pos, parm);
}

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       const tse::DateTime& parm,
                                                       int32_t index,
                                                       bool dateOnly) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (UNLIKELY(pos == std::string::npos))
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[16];
  snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

  sql.replace(pos, strlen(srch) + 1, bindVarLabel);
  if (UNLIKELY(dateOnly))
  {
    substitutor.addBoundParameter<ORACLEBoundDate>(-1, pos, parm);
  }
  else
  {
    substitutor.addBoundParameter<ORACLEBoundDateTime>(-1, pos, parm);
  }
}

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       const std::vector<tse::CarrierCode>& parm,
                                                       int32_t index) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (UNLIKELY(pos == std::string::npos))
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  if (LIKELY(parm.size() == 1))
  {
    char bindVarLabel[20];
    snprintf(bindVarLabel, sizeof(bindVarLabel), "= :%d", index);

    sql.replace(pos, strlen(srch) + 1, bindVarLabel);
    substitutor.addBoundParameter<ORACLEBoundString>(-1, pos, parm.front().c_str());
  }
  else
  {
    char bindVarLabel[8192];
    char* ptr = bindVarLabel;
    int numChars = 0;
    size_t offset = 0;
    numChars = snprintf(ptr, sizeof(bindVarLabel), "IN ( ");
    ptr += numChars;
    offset += numChars;

    bool prefixComma = false;
    int bindCtr = 1;

    std::vector<tse::CarrierCode>::const_iterator iter = parm.begin();
    std::vector<tse::CarrierCode>::const_iterator iterEnd = parm.end();
    for (; iter != iterEnd; ++iter, ++bindCtr)
    {
      // We could calculate the exact length of the bind variable list
      //  by taking the base 10 log of bindCtr, but it's not worth the
      //  extra overhead. Instead, we'll make a conservative (and safe)
      //  "close enough" calculation.
      //
      //  ORACLE IN close can contain up to 1000 items
      if (offset > sizeof(bindVarLabel) - 20 || parm.size() >= 1000)
      {
        LOG4CXX_ERROR(logger(),
                      "Query parameter "
                          << index
                          << " carrier vector size exceeds buffer size for SQL: " << sql.c_str());
        throw std::runtime_error("SQL bind variable error.");
      }

      if (prefixComma)
      {
        numChars = snprintf(ptr, sizeof(bindVarLabel) - offset, ", :cxr%d", bindCtr);
      }
      else
      {
        numChars = snprintf(ptr, sizeof(bindVarLabel) - offset, ":cxr%d", bindCtr);
        prefixComma = true;
      }
      substitutor.addBoundParameter<ORACLEBoundString>(-1, pos + offset, (*iter).c_str());

      ptr += numChars;
      offset += numChars;
    }
    snprintf(ptr, sizeof(bindVarLabel) - offset, " ) ");

    sql.replace(pos, strlen(srch) + 1, bindVarLabel);
  }
}

// Precondition: the sql query has been built with IN ( :d )
// Create 1 bound parameter for each item in the vector parm.
void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       const std::vector<int64_t>& parm,
                                                       int32_t index) const
{
  // Find ':index' [ e.g. :1, :2 ]
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (pos == std::string::npos)
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[1024];
  char* ptr = bindVarLabel;
  int numChars = 0;
  size_t offset = 0;
  int bindCtr = 1;

  std::vector<long>::const_iterator i = parm.begin();
  std::vector<long>::const_iterator e = parm.end();
  bool first = true;
  for (; i != e; ++i, ++bindCtr)
  {
    char label[32];
    int len = 0;
    if (first)
    {
      len = sprintf(label, ":l%d", bindCtr);
      first = false;
    }
    else
    {
      len = sprintf(label, ", :l%d", bindCtr);
    }

    if (UNLIKELY(offset > sizeof(bindVarLabel) - (len + 2)))
    {
      LOG4CXX_ERROR(logger(),
                    "Query parameter "
                        << index << " vector size exceeds buffer size for SQL: " << sql.c_str());
      throw std::runtime_error("SQL bind variable error.");
    }

    numChars = snprintf(ptr, sizeof(bindVarLabel) - offset, "%s", label);
    // Why are these all using -1 for the index? Don't we know enough at this point
    // to assign the index of this item?
    substitutor.addBoundParameter<ORACLEBoundLong>(-1, pos + offset, *i);

    ptr += numChars;
    offset += numChars;
  }
  sql.replace(pos, strlen(srch) + 1, bindVarLabel);
}

/*
void
ORACLEBindingParameterSubstitutor::
substituteParameter(ParameterSubstitutor& substitutor,
                    std::string& sql,
                    const std::vector<PaxTypeCode>& parm,
                    int32_t index) const
{
}
*/

void
ORACLEBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                       std::string& sql,
                                                       const tse::DateTime& parm,
                                                       const char* placeHolder,
                                                       bool dateOnly) const
{
  if (UNLIKELY(strlen(placeHolder) > 16))
  {
    LOG4CXX_ERROR(logger(),
                  "Parameter placeholder is too long: "
                      << placeHolder << " Please exercise some restraint and "
                      << " use a placeholder less than 16 characters in length."
                      << " SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter placeholder exceeds max length.");
  }

  int32_t ctr = 0;
  std::string::size_type pos = 0;
  while ((pos = sql.find(placeHolder, pos)) != std::string::npos)
  {
    ++ctr;
    char bindVarLabel[32];
    snprintf(bindVarLabel, sizeof(bindVarLabel), ":%s%d", placeHolder + 1, ctr);

    if (LIKELY(dateOnly))
    {
      substitutor.addBoundParameter<ORACLEBoundDate>(-1, pos, parm);
    }
    else
    {
      substitutor.addBoundParameter<ORACLEBoundDateTime>(-1, pos, parm);
    }

    sql.erase(pos, strlen(placeHolder));
    sql.insert(pos, bindVarLabel);

    // Skip over the string we just inserted
    pos += strlen(bindVarLabel);
  }
}
}
