//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/FareProperties.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetFarePropertiesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFarePropertiesSQLStatement() {}
  virtual ~QueryGetFarePropertiesSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    CREATEDATE,
    EXPIREDATE,
    FARESOURCE,
    VALUECODEALGORITHMNAME,
    EXCLUDEQSURCHARGE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, CARRIER, RULETARIFF, RULE, CREATEDATE, EXPIREDATE,"
                  "       FARESOURCE, VALUECODEALGORITHMNAME, EXCLUDEQSURCHARGE");

    this->From("=FAREPROPERTIES");

    this->Where("VENDOR= %1q  "
                " and CARRIER= %2q "
                " and RULETARIFF= %3n "
                " and RULE= %4q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("VERSION");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareProperties* mapRowToFareProperties(Row* row)
  {
    FareProperties* fareProperties = new FareProperties;

    fareProperties->vendor() = row->getString(VENDOR);
    fareProperties->carrier() = row->getString(CARRIER);
    fareProperties->ruleTariff() = row->getInt(RULETARIFF);
    fareProperties->rule() = row->getString(RULE);

    fareProperties->createDate() = row->getDate(CREATEDATE);
    fareProperties->expireDate() = row->getDate(EXPIREDATE);
    fareProperties->fareSource() = row->getString(FARESOURCE);

    if (!row->isNull(VALUECODEALGORITHMNAME))
      fareProperties->valueCodeAlgorithmName() = row->getString(VALUECODEALGORITHMNAME);
    if (!row->isNull(EXCLUDEQSURCHARGE))
      fareProperties->excludeQSurchargeInd() = row->getChar(EXCLUDEQSURCHARGE);

    return fareProperties;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFarePropertiesHistoricalSQLStatement
    : public QueryGetFarePropertiesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR, CARRIER, RULETARIFF, RULE, CREATEDATE, EXPIREDATE,"
                             "       FARESOURCE, VALUECODEALGORITHMNAME, EXCLUDEQSURCHARGE");
    partialStatement.From("=FAREPROPERTIESH");
    partialStatement.Where("VENDOR= %1q  "
                           " and CARRIER= %2q "
                           " and RULETARIFF= %3n "
                           " and RULE= %4q "
                           " and %5n <= EXPIREDATE"
                           " and %6n >= CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select VENDOR, CARRIER, RULETARIFF, RULE, CREATEDATE, EXPIREDATE,"
                             "       FARESOURCE, VALUECODEALGORITHMNAME, EXCLUDEQSURCHARGE");
    partialStatement.From("=FAREPROPERTIES");
    partialStatement.Where("VENDOR= %7q  "
                           " and CARRIER= %8q "
                           " and RULETARIFF= %9n "
                           " and RULE= %10q "
                           " and %11n <= EXPIREDATE"
                           " and %12n >= CREATEDATE"
                           ")");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};
} // tse
