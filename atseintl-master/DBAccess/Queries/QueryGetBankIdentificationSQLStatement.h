// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBankIdentification.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetBankIdentificationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBankIdentificationSQLStatement() {};
  virtual ~QueryGetBankIdentificationSQLStatement() {};

  enum ColumnIndexes
  {
    BANKIDENTIFICATIONNBR = 0,
    EFFDATE,
    DISCDATE,
    CARDTYPE,
    CREATEDATE,
    EXPIREDATE,
    LASTMODDATE,
    INHIBIT,
    VALIDITYIND,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select BANKIDENTIFICATIONNBR, EFFDATE, DISCDATE, CARDTYPE, "
                  "       CREATEDATE, EXPIREDATE, LASTMODDATE, INHIBIT, VALIDITYIND");

    this->From("=BANKIDENTIFICATION");

    this->Where("BANKIDENTIFICATIONNBR = %1q  "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BankIdentificationInfo* mapRowToBankIdentificationInfo(Row* row)
  {
    tse::BankIdentificationInfo* binInfo = new tse::BankIdentificationInfo;

    binInfo->binNumber() = row->getInt(BANKIDENTIFICATIONNBR);
    binInfo->effDate() = row->getDate(EFFDATE);
    binInfo->discDate() = row->getDate(DISCDATE);
    binInfo->cardType() = row->getChar(CARDTYPE);
    binInfo->createDate() = row->getDate(CREATEDATE);
    binInfo->expireDate() = row->getDate(EXPIREDATE);
    binInfo->lastModDate() = row->getDate(LASTMODDATE);
    binInfo->inhibit() = row->getChar(INHIBIT);
    binInfo->validityInd() = row->getChar(VALIDITYIND);

    return binInfo;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetBankIdentificationHistoricalSQLStatement
    : public QueryGetBankIdentificationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("BANKIDENTIFICATIONNBR = %1q  "
                " and VALIDITYIND = 'Y'"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  };
};
}

