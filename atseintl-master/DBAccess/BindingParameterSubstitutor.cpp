//----------------------------------------------------------------------------
//
//     File:           BindingParameterSubstitutor.cpp
//     Description:    BindingParameterSubstitutor
//     Created:        07/02/2009
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

#include "DBAccess/BindingParameterSubstitutor.h"

#include "Common/Logger.h"
#include "DBAccess/BoundParameter.h"
#include "DBAccess/BoundParameterTypes.h"
#include "DBAccess/ParameterSubstitutor.h"
#include "DBAccess/SQLStatementHelper.h"

#include <stdexcept>

namespace DBAccess
{
log4cxx::LoggerPtr
BindingParameterSubstitutor::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BindingParameterSubstitutor"));

BindingParameterSubstitutor::BindingParameterSubstitutor()
{
  LOG4CXX_INFO(log4cxx::Logger::getLogger("atseintl.Server.TseServer"),
               "BindingParameterSubstitutor enabled.")
}

BindingParameterSubstitutor::~BindingParameterSubstitutor() {}

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 const char* parm,
                                                 int32_t index,
                                                 bool forceLiteral) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);
  size_t pos = sql.find(srch, 0);

  if (pos == std::string::npos)
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  if (!forceLiteral)
  {
    char bindVarLabel[16];
    snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

    sql.replace(pos, strlen(srch) + 1, bindVarLabel);
    substitutor.addBoundParameter<BoundString>(-1, pos, parm);
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
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 const std::string& parm,
                                                 int32_t index,
                                                 bool forceLiteral) const
{
  substituteParameter(substitutor, sql, parm.c_str(), index, forceLiteral);
}

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 int64_t parm,
                                                 int32_t index) const
{
  // Search for each %(number):(q)(n) next
  char srch[22];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (pos == std::string::npos)
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[16];
  snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

  sql.replace(pos, strlen(srch) + 1, bindVarLabel);
  substitutor.addBoundParameter<BoundLong>(-1, pos, parm);
}

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 int parm,
                                                 int32_t index) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (pos == std::string::npos)
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[16];
  snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

  sql.replace(pos, strlen(srch) + 1, bindVarLabel);
  substitutor.addBoundParameter<BoundInteger>(-1, pos, parm);
}

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 const tse::DateTime& parm,
                                                 int32_t index,
                                                 bool dateOnly) const
{
  // Search for each %(number):(q)(n) next
  char srch[16];
  snprintf(srch, sizeof(srch), "%%%d", index);

  size_t pos = sql.find(srch, 0);
  if (pos == std::string::npos)
  {
    LOG4CXX_ERROR(logger(), "Parameter " << index << " not found in SQL string: " << sql.c_str());
    throw std::runtime_error("Parameter not found in SQL string.");
  }

  char bindVarLabel[16];
  snprintf(bindVarLabel, sizeof(bindVarLabel), ":%d", index);

  sql.replace(pos, strlen(srch) + 1, bindVarLabel);
  if (dateOnly)
  {
    substitutor.addBoundParameter<BoundDate>(-1, pos, parm);
  }
  else
  {
    substitutor.addBoundParameter<BoundDateTime>(-1, pos, parm);
  }

}

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 const std::vector<tse::CarrierCode>& parm,
                                                 int32_t index) const
{
  // TODO: Implement
}

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 const std::vector<int64_t>& parm,
                                                 int32_t index) const
{
}

/*
void
BindingParameterSubstitutor::
substituteParameter(ParameterSubstitutor& substitutor,
                    std::string& sql,
                    const std::vector<PaxTypeCode>& parm,
                    int32_t index) const
{
}
*/

void
BindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                 std::string& sql,
                                                 const tse::DateTime& parm,
                                                 const char* placeHolder,
                                                 bool dateOnly) const
{
  if (strlen(placeHolder) > 16)
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

    if (dateOnly)
    {
      substitutor.addBoundParameter<BoundDate>(-1, pos, parm);
    }
    else
    {
      substitutor.addBoundParameter<BoundDateTime>(-1, pos, parm);
    }

    sql.erase(pos, strlen(placeHolder));
    sql.insert(pos, bindVarLabel);

    // Skip over the string we just inserted
    pos += strlen(bindVarLabel);
  }
}
}
