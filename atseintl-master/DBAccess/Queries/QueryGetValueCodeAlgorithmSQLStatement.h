//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLStatement.h"
#include "DBAccess/ValueCodeAlgorithm.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetValueCodeAlgorithmSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetValueCodeAlgorithmSQLStatement() {}
  virtual ~QueryGetValueCodeAlgorithmSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    ALGORITHMNAME,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    PREFIX,
    SUFFIX,
    DIGIT1CHAR,
    DIGIT2CHAR,
    DIGIT3CHAR,
    DIGIT4CHAR,
    DIGIT5CHAR,
    DIGIT6CHAR,
    DIGIT7CHAR,
    DIGIT8CHAR,
    DIGIT9CHAR,
    DIGIT0CHAR,
    DECIMALCHAR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select VENDOR, CARRIER, ALGORITHMNAME, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
        "       PREFIX, SUFFIX, DIGIT1CHAR, DIGIT2CHAR, DIGIT3CHAR, DIGIT4CHAR, DIGIT5CHAR,"
        "       DIGIT6CHAR, DIGIT7CHAR, DIGIT8CHAR, DIGIT9CHAR, DIGIT0CHAR, DECIMALCHAR");

    this->From("=VALUECODEALGORITHM");

    this->Where("VENDOR = %1q  "
                " and CARRIER= %2q "
                " and ALGORITHMNAME= %3q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("VERSION");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static ValueCodeAlgorithm* mapRowToValueCodeAlgorithm(Row* row)
  {
    ValueCodeAlgorithm* algorithm = new ValueCodeAlgorithm;

    algorithm->vendor() = row->getString(VENDOR);
    algorithm->carrier() = row->getString(CARRIER);
    algorithm->algorithmName() = row->getString(ALGORITHMNAME);

    algorithm->createDate() = row->getDate(CREATEDATE);
    algorithm->expireDate() = row->getDate(EXPIREDATE);
    algorithm->effDate() = row->getDate(EFFDATE);
    algorithm->discDate() = row->getDate(DISCDATE);

    if (!row->isNull(PREFIX))
      algorithm->prefix() = row->getString(PREFIX);
    if (!row->isNull(SUFFIX))
      algorithm->suffix() = row->getString(SUFFIX);
    if (!row->isNull(DIGIT1CHAR))
      algorithm->digitToChar()[1] = row->getChar(DIGIT1CHAR);
    if (!row->isNull(DIGIT2CHAR))
      algorithm->digitToChar()[2] = row->getChar(DIGIT2CHAR);
    if (!row->isNull(DIGIT3CHAR))
      algorithm->digitToChar()[3] = row->getChar(DIGIT3CHAR);
    if (!row->isNull(DIGIT4CHAR))
      algorithm->digitToChar()[4] = row->getChar(DIGIT4CHAR);
    if (!row->isNull(DIGIT5CHAR))
      algorithm->digitToChar()[5] = row->getChar(DIGIT5CHAR);
    if (!row->isNull(DIGIT6CHAR))
      algorithm->digitToChar()[6] = row->getChar(DIGIT6CHAR);
    if (!row->isNull(DIGIT7CHAR))
      algorithm->digitToChar()[7] = row->getChar(DIGIT7CHAR);
    if (!row->isNull(DIGIT8CHAR))
      algorithm->digitToChar()[8] = row->getChar(DIGIT8CHAR);
    if (!row->isNull(DIGIT9CHAR))
      algorithm->digitToChar()[9] = row->getChar(DIGIT9CHAR);
    if (!row->isNull(DIGIT0CHAR))
      algorithm->digitToChar()[0] = row->getChar(DIGIT0CHAR);
    if (!row->isNull(DECIMALCHAR))
      algorithm->decimalChar() = row->getChar(DECIMALCHAR);

    return algorithm;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetValueCodeAlgorithmHistoricalSQLStatement
    : public QueryGetValueCodeAlgorithmSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and CARRIER= %2q "
                " and ALGORITHMNAME= %3q "
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
    this->OrderBy("VERSION");
  }
};
} // tse
