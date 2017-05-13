//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/PrintOption.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetPrintOptionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPrintOptionSQLStatement() {}
  virtual ~QueryGetPrintOptionSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    FARESOURCE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    PRINTOPTION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, FARESOURCE, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
                  "       PRINTOPTION");

    this->From("=PRINTOPTION");

    this->Where("VENDOR = %1q  "
                " and FARESOURCE= %2q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("VERSION");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static PrintOption* mapRowToPrintOption(Row* row)
  {
    PrintOption* algorithm = new PrintOption;

    algorithm->vendor() = row->getString(VENDOR);
    algorithm->fareSource() = row->getString(FARESOURCE);

    algorithm->createDate() = row->getDate(CREATEDATE);
    algorithm->expireDate() = row->getDate(EXPIREDATE);
    algorithm->effDate() = row->getDate(EFFDATE);
    algorithm->discDate() = row->getDate(DISCDATE);

    if (!row->isNull(PRINTOPTION))
      algorithm->printOption() = row->getChar(PRINTOPTION);

    return algorithm;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetPrintOptionHistoricalSQLStatement : public QueryGetPrintOptionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and FARESOURCE= %2q "
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("VERSION");
  }
};
} // tse
