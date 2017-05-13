//----------------------------------------------------------------------------
//
//     File:           NonBindingParameterSubstitutor.cpp
//     Description:    NonBindingParameterSubstitutor
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

#include "DBAccess/NonBindingParameterSubstitutor.h"

#include "Common/Logger.h"
#include "DBAccess/SQLStatementHelper.h"

#include <stdexcept>

namespace DBAccess
{
log4cxx::LoggerPtr
NonBindingParameterSubstitutor::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NonBindingParameterSubstitutor"));

NonBindingParameterSubstitutor::NonBindingParameterSubstitutor()
{
  LOG4CXX_INFO(log4cxx::Logger::getLogger("atseintl.Server.TseServer"),
               "NonBindingParameterSubstitutor enabled.")
}

NonBindingParameterSubstitutor::~NonBindingParameterSubstitutor() {}

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
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
  if (!parm)
  {
    LOG4CXX_ERROR(logger(), "Illegal parameter [" << index << "] null argument. ");
    throw std::runtime_error("Illegal substituteParameter NULL argument param.");
  }

  if (sql[pos + strlen(srch)] == 'q')
  {
    /*
    ** If in the future the application wants to ignore leading spaces, then...
        // skip leading blanks
        while ( *parm && *parm == ' ' )
          parm++ ;
    */
    size_t length(strlen(parm));
    // will only copy from first non blank to last non-blank character.
    while (length && parm[length - 1] == ' ')
      length--;
    if (length == 0) // all white space
      sql.replace(pos, strlen(srch) + 1, "' '");
    else
    {
      sql.replace(pos, strlen(srch) + 1, "''");
      sql.insert(pos + 1, parm, length);
    }
  }
  else
  {
    sql.replace(pos, strlen(srch) + 1, parm);
  }
}

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    const std::string& parm,
                                                    int32_t index,
                                                    bool forceLiteral) const
{
  substituteParameter(substitutor, sql, parm.c_str(), index, forceLiteral);
}

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    int parm,
                                                    int32_t index) const
{
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", parm);
  substituteParameter(substitutor, sql, buf, index);
}

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    int64_t parm,
                                                    int32_t index) const
{
  char buf[32];
  snprintf(buf, sizeof(buf), "%ld", parm);
  substituteParameter(substitutor, sql, buf, index);
}

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    const tse::DateTime& parm,
                                                    int32_t index,
                                                    bool dateOnly) const
{
  std::string s;

  SQLStatementHelper sqlHelper;
  if (dateOnly)
    sqlHelper.formatDateString(parm, s);
  else
    sqlHelper.formatDateTimeString(parm, s);

  substituteParameter(substitutor, sql, s.c_str(), index);
}

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    const std::vector<tse::CarrierCode>& parm,
                                                    int32_t index) const
{
  std::string s;
  if (parm.size() == 1)
  {
    s = "= '" + parm.front() + "'";
  }
  else
  {
    s = "IN (";
    bool prefixComma = false;

    std::vector<tse::CarrierCode>::const_iterator iter = parm.begin();
    std::vector<tse::CarrierCode>::const_iterator iterEnd = parm.end();
    for (; iter != iterEnd; ++iter)
    {
      if (prefixComma)
        s += ",";
      else
        prefixComma = true;

      s += "'" + *iter + "'";
    }
    s += ")";
  }
  substituteParameter(substitutor, sql, s.c_str(), index, true);
}

// Precondition: the sql query has been built with IN ( :d )
// Create 1 bound parameter for each item in the vector parm.
void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    const std::vector<int64_t>& parm,
                                                    int32_t index) const
{
  std::string s;
  bool prefixComma = false;

  std::vector<int64_t>::const_iterator i = parm.begin();
  std::vector<int64_t>::const_iterator e = parm.end();
  for (; i != e; ++i)
  {
    if (prefixComma)
      s += ",";
    else
      prefixComma = true;
    char buffer[32];
    sprintf(buffer, "%ld", *i);
    s += buffer;
  }
  substituteParameter(substitutor, sql, s.c_str(), index, true);
}

/*
void
NonBindingParameterSubstitutor::
substituteParameter(ParameterSubstitutor& substitutor,
                    std::string& sql,
                    const std::vector<PaxTypeCode>& parm,
                    int32_t index) const
{
}
*/

void
NonBindingParameterSubstitutor::substituteParameter(ParameterSubstitutor& substitutor,
                                                    std::string& sql,
                                                    const tse::DateTime& parm,
                                                    const char* placeHolder,
                                                    bool dateOnly) const
{
  std::string s;

  SQLStatementHelper sqlHelper;
  if (dateOnly)
    sqlHelper.formatDateString(parm, s);
  else
    sqlHelper.formatDateTimeString(parm, s);

  std::string::size_type pos = 0;
  std::string::size_type posLen = s.length();
  while ((pos = sql.find(placeHolder, pos)) != std::string::npos)
  {
    sql.erase(pos, strlen(placeHolder));
    sql.insert(pos, s);
    // Skip over the string we just inserted
    pos += posLen;
  }
}
}
