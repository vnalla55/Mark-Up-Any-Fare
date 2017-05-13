//----------------------------------------------------------------------------
//
//     File:           SQLStatement.h
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

#pragma once

#include "DBAccess/DataManager.h"
#include "DBAccess/Row.h"
#include "DBAccess/SQLStatementHelper.h"

#include <list>
#include <string>

namespace DBAccess
{

//////////////////////////////////////////////////////////////////////
//    Class to represent a SQL statement
//////////////////////////////////////////////////////////////////////

class SQLStatement : public std::string
{
public:
  SQLStatement();
  SQLStatement(const SQLStatement& another);
  SQLStatement(const std::string& SQLString);
  virtual ~SQLStatement();

  void Reset();

  const SQLStatement& operator=(const SQLStatement& another);

  // Various clauses for the statement

  void Command(const std::string& sqlCommand);
  const std::string& Command() const { return _Command; }

  void From(const std::string& sqlFrom);
  const std::string& From() const { return _From; }

  void Where(const std::string& sqlWhere);
  const std::string& Where() const { return _Where; }

  void OrderBy(const std::string& sqlOrderBy);
  const std::string& OrderBy() const { return _OrderBy; }

  void GroupBy(const std::string& sqlGroupBy);
  const std::string& GroupBy() const { return _GroupBy; }

  void Limit(const std::string& sqlLimit);
  const std::string& Limit() const { return _Limit; }

  // Function to combine the clauses into a complete SQL statement
  const std::string& ConstructSQL();

  virtual const std::string& RegisterColumnsAndBaseSQL(const char* queryName)
  {
    return *this;
  };

protected:
  // Helper functions to generate parts of the SQL statement
  // formatted appropriately for the database type in use.
  //
  void generateJoinString(const std::string& table1,
                          const std::string& alias1,
                          const std::string& joinType,
                          const std::string& table2,
                          const std::string& alias2,
                          const std::vector<std::string>& joinFields,
                          std::string& str) const;

  void generateJoinString(SQLStatement& target,
                          const std::string& table1,
                          const std::string& alias1,
                          const std::string& joinType,
                          const std::string& table2,
                          const std::string& alias2,
                          const std::vector<std::string>& joinFields,
                          std::string& str) const
  {
    target.generateJoinString(table1, alias1, joinType, table2, alias2, joinFields, str);
  }

  void generateLimitString(uint32_t limit, std::string& str) const;
  void generateLimitString(const std::string& limit, std::string& str) const;

  void generateLimitString(SQLStatement& target, uint32_t limit, std::string& str) const
  {
    target.generateLimitString(limit, str);
  }

  void generateLimitString(SQLStatement& target, const std::string& limit, std::string& str) const
  {
    target.generateLimitString(limit, str);
  }

private:
  std::string _Command;
  std::string _From;
  std::string _Where;
  std::string _GroupBy;
  std::string _OrderBy;
  std::string _Limit;

  SQLStatementHelper _sqlStatementHelper;

  void trimAll(std::string& result, const std::string& str);
};

// Compound SQL statement forms by combining others.
class CompoundSQLStatement : public std::list<SQLStatement>
{
public:
  CompoundSQLStatement() {};
  ~CompoundSQLStatement() {};

  // Function to combine the clauses into a complete SQL statement
  const std::string& ConstructSQL();

private:
  std::string _ConstructedSQL;
};
}

