//----------------------------------------------------------------------------
//
//     File:           SQLStatement.cpp
//     Description:    SQLStatement
//     Created:        10/11/2007
//     Authors:        Mike Lillis
//
//     Updates:
//
//     Copyright @2007-2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/SQLStatement.h"

#include "DBAccess/SQLStatementHelper.h"

#include <list>
#include <string>

#include <ctype.h>

namespace DBAccess
{

SQLStatement::SQLStatement() {};

SQLStatement::SQLStatement(const SQLStatement& another)
  : std::string(static_cast<const std::string&>(another)),
    _Command(another._Command),
    _From(another._From),
    _Where(another._Where),
    _GroupBy(another._GroupBy),
    _OrderBy(another._OrderBy),
    _sqlStatementHelper(another._sqlStatementHelper) {};

SQLStatement::SQLStatement(const std::string& SQLString)
{
  std::string& str = *this;
  str = SQLString;
};

SQLStatement::~SQLStatement() {}

void
SQLStatement::Reset()
{
  *(static_cast<std::string*>(this)) = "";
  _Command = "";
  _From = "";
  _Where = "";
  _GroupBy = "";
  _OrderBy = "";
  _sqlStatementHelper.resetState();
};

const SQLStatement&
SQLStatement::
operator=(const SQLStatement& another)
{
  if (&another != this)
  {
    _Command = another._Command;
    _From = another._From;
    _Where = another._Where;
    _GroupBy = another._GroupBy;
    _OrderBy = another._OrderBy;
    _sqlStatementHelper = another._sqlStatementHelper;
    *this = static_cast<const std::string>(another);
  }
  return *this;
}

// Various clauses for the statement

void
SQLStatement::Command(const std::string& sqlCommand)
{
  _Command = "";
  trimAll(_Command, sqlCommand);
  if (_Command.empty())
  {
    _sqlStatementHelper.hasCommand(false);
  }
  else
  {
    _sqlStatementHelper.hasCommand(true);
  }
}

void
SQLStatement::From(const std::string& sqlFrom)
{
  _From = "";
  trimAll(_From, sqlFrom);
  if (_From.empty())
  {
    _sqlStatementHelper.hasFrom(false);
  }
  else
  {
    _From = " from " + _From;
    _sqlStatementHelper.hasFrom(true);
  }
}

void
SQLStatement::Where(const std::string& sqlWhere)
{
  _Where = "";
  trimAll(_Where, sqlWhere);
  if (_Where.empty())
  {
    _sqlStatementHelper.hasWhere(false);
  }
  else
  {
    _Where = " where " + _Where;
    _sqlStatementHelper.hasWhere(true);
  }
}

void
SQLStatement::OrderBy(const std::string& sqlOrderBy)
{
  _OrderBy = "";
  trimAll(_OrderBy, sqlOrderBy);
  if (_OrderBy.empty())
  {
    _sqlStatementHelper.hasOrderBy(false);
  }
  else
  {
    _OrderBy = " order by " + _OrderBy;
    _sqlStatementHelper.hasOrderBy(true);
  }
}

void
SQLStatement::GroupBy(const std::string& sqlGroupBy)
{
  _GroupBy = "";
  trimAll(_GroupBy, sqlGroupBy);
  if (_GroupBy.empty())
  {
    _sqlStatementHelper.hasGroupBy(false);
  }
  else
  {
    _GroupBy = " group by " + _GroupBy;
    _sqlStatementHelper.hasGroupBy(true);
  }
}

void
SQLStatement::Limit(const std::string& sqlLimit)
{
  _Limit = "";
  trimAll(_Limit, sqlLimit);
  if (_Limit.empty())
  {
    _sqlStatementHelper.hasLimit(false);
  }
  else
  {
    _Limit = " " + _Limit;
    _sqlStatementHelper.hasLimit(true);
  }
}

// Function to combine the clauses into a complete SQL statement
const std::string&
SQLStatement::ConstructSQL()
{
  std::string& str = *this;
  _sqlStatementHelper.constructStatement(_Command, _From, _Where, _GroupBy, _OrderBy, _Limit, str);
  return *this;
}

// Helper function to generate JOIN clause formatted appropriately
// for the database type in use.
void
SQLStatement::generateJoinString(const std::string& table1,
                                 const std::string& alias1,
                                 const std::string& joinType,
                                 const std::string& table2,
                                 const std::string& alias2,
                                 const std::vector<std::string>& joinFields,
                                 std::string& result) const
{
  _sqlStatementHelper.generateJoinString(
      table1, alias1, joinType, table2, alias2, joinFields, result);
}

// Helper function to generate LIMIT clause formatted appropriately
// for the database type in use.

void
SQLStatement::generateLimitString(uint32_t limit, std::string& result) const
{
  _sqlStatementHelper.generateLimitString(limit, result);
}

void
SQLStatement::generateLimitString(const std::string& limit, std::string& result) const
{
  _sqlStatementHelper.generateLimitString(limit, result);
}

//
// append str to result while removing extra white space
// removes leading and trailing white space
// converts " ," or ", " into "," and strings of blanks/tabs to a space
//
void
SQLStatement::trimAll(std::string& result, const std::string& str)
{
  std::string::size_type pos = str.find_last_not_of(" \t\n");
  std::string::const_iterator end = str.end();
  if (pos != std::string::npos)
  {
    end = str.begin() + pos + 1;
  }
  std::string::const_iterator ptr = str.begin();
  if (ptr != end)
  {
    while (isspace(*ptr))
      ptr++;
  }
  while (ptr != end)
  {
    if (isspace(*ptr))
    {
      while (isspace(*ptr))
        ptr++;
      if (*ptr != ',')
        result.push_back(' ');
    }
    if (*ptr == ',')
    {
      result.push_back(',');
      if (++ptr == end)
        break;
      while (isspace(*ptr))
        ptr++;
    }
    else
    {
      result.push_back(*ptr++);
    }
  }
}

const std::string&
CompoundSQLStatement::ConstructSQL()
{
  _ConstructedSQL = "";

  std::list<SQLStatement>::iterator iter = this->begin();

  for (; iter != this->end(); ++iter)
  {
    if (iter != this->begin())
      _ConstructedSQL += " ";
    (*iter).ConstructSQL();
    _ConstructedSQL += *iter;
  }

  return _ConstructedSQL;
}

} // DBAccess
