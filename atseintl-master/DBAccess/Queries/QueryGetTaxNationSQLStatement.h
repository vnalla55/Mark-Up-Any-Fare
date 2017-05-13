//----------------------------------------------------------------------------
//          File:           QueryGetTaxNationSQLStatement.h
//          Description:    QueryGetTaxNationSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include <utility>
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxNation.h"
#include "DBAccess/SQLStatement.h"
#include "DBAccess/TaxOrderTktIssue.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTaxOrderTktIssueSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxOrderTktIssueSQLStatement() {}
  virtual ~QueryGetTaxOrderTktIssueSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    TAXTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE, TAXTYPE");
    this->From("=TAXORDERTKTISSUE");
    this->Where("NATION = %1q and CREATEDATE = %2n");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRow(const Row& row, TaxOrderTktIssue& taxOrderTktIssue)
  {
    taxOrderTktIssue.taxCode() = row.getString(TAXCODE);
    taxOrderTktIssue.taxType() = row.isNull(TAXTYPE) ? "000" : row.getString(TAXTYPE);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxNationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxNationSQLStatement() {}
  virtual ~QueryGetTaxNationSQLStatement() {}

  enum ColumnIndexes
  {
    NATION = 0,
    CREATEDATE,
    EXPIREDATE,
    LOCKDATE,
    EFFDATE,
    DISCDATE,
    MEMONO,
    ROUNDINGUNITNODEC,
    ROUNDINGUNIT,
    CREATORBUSINESSUNIT,
    CREATORID,
    ROUNDINGRULE,
    TAXROUNDINGOVERRIDEIND,
    TAXCOLLECTIONIND,
    TAXFAREQUOTEIND,
    COLLECTIONNATION1,
    COLLECTIONNATION2,
    MESSAGE,
    SEQNO,
    TAXCODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select taxN.NATION,taxN.CREATEDATE,EXPIREDATE,LOCKDATE,EFFDATE,DISCDATE,"
                  "MEMONO,ROUNDINGUNITNODEC,ROUNDINGUNIT,CREATORBUSINESSUNIT,"
                  "CREATORID,ROUNDINGRULE,TAXROUNDINGOVERRIDEIND,TAXCOLLECTIONIND,"
                  "TAXFAREQUOTEIND,COLLECTIONNATION1,COLLECTIONNATION2,MESSAGE,"
                  "taxO.SEQNO,taxO.TAXCODE");

    //		        this->From("=TAXNATION taxN LEFT OUTER JOIN =TAXCODEORDER taxO"
    //		                  " USING (NATION,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(2);
    joinFields.push_back("NATION");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=TAXNATION", "taxN", "LEFT OUTER JOIN", "=TAXCODEORDER", "taxO", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("taxN.NATION = %1q"
                " and %cd <= EXPIREDATE");
    this->OrderBy("taxN.NATION,taxN.CREATEDATE,taxO.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxNation* mapRowToTaxNation(Row* row, TaxNation* prevTaxN)
  {
    tse::TaxNation* pTaxNation;

    NationCode nation = row->getString(NATION);
    DateTime createDate = row->getDate(CREATEDATE);

    // If the TaxNation has not changed then let we just have a new TAXCodeOrder
    if (prevTaxN != nullptr && nation == prevTaxN->nation() && createDate == prevTaxN->createDate())
    {
      pTaxNation = prevTaxN;
    }
    else
    {
      pTaxNation = new TaxNation();
      pTaxNation->nation() = nation;
      pTaxNation->createDate() = createDate;
      pTaxNation->expireDate() = row->getDate(EXPIREDATE);
      pTaxNation->effDate() = row->getDate(EFFDATE);
      pTaxNation->discDate() = row->getDate(DISCDATE);
      pTaxNation->roundingUnitNodec() = row->getInt(ROUNDINGUNITNODEC);
      pTaxNation->roundingUnit() =
          QUERYCLASS::adjustDecimal(row->getInt(ROUNDINGUNIT), pTaxNation->roundingUnitNodec());
      std::string roundRule = row->getString(ROUNDINGRULE);
      if (roundRule == "D")
        pTaxNation->roundingRule() = DOWN;
      else if (roundRule == "U")
        pTaxNation->roundingRule() = UP;
      else if (roundRule == "N")
        pTaxNation->roundingRule() = NEAREST;
      else
        pTaxNation->roundingRule() = NONE;

      pTaxNation->taxRoundingOverrideInd() = row->getChar(TAXROUNDINGOVERRIDEIND);
      pTaxNation->taxCollectionInd() = row->getChar(TAXCOLLECTIONIND);
      pTaxNation->taxFarequoteInd() = row->getChar(TAXFAREQUOTEIND);
      pTaxNation->collectionNation1() = row->getString(COLLECTIONNATION1);
      pTaxNation->collectionNation2() = row->getString(COLLECTIONNATION2);
      pTaxNation->message() = row->getString(MESSAGE);
    }

    if (!row->isNull(TAXCODE))
      pTaxNation->taxCodeOrder().push_back(row->getString(TAXCODE));

    return pTaxNation;
  } // mapRowToTaxNation()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxNRoundSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxNRoundSQLStatement() {}
  virtual ~QueryGetTaxNRoundSQLStatement() {}

  enum ColumnIndexes
  {
    EXCEPTNATION = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select EXCEPTNATION");
    this->From("=TAXNATIONROUND");
    this->Where("NATION = %1q and CREATEDATE = %2n");
    if (DataManager::forceSortOrder())
      this->OrderBy("NATION,CREATEDATE,EXCEPTNATION");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToExceptNation(Row* row) { return row->getString(EXCEPTNATION); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxNCollectSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxNCollectSQLStatement() {}
  virtual ~QueryGetTaxNCollectSQLStatement() {}

  enum ColumnIndexes
  {
    COLLECTNATION = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select COLLECTNATION");
    this->From("=TAXNATIONCOLLECT");
    this->Where("NATION = %1q and CREATEDATE = %2n");
    if (DataManager::forceSortOrder())
      this->OrderBy("NATION,CREATEDATE,COLLECTNATION");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToCollectNation(Row* row) { return row->getString(COLLECTNATION); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template to adjust the SQL for QueryGetTaxNationHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetTaxNationHistoricalSQLStatement : public QueryGetTaxNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {

    this->Where("taxN.NATION = %1q"
                "    and %2n <= taxN.EXPIREDATE"
                "    and (%3n >= taxN.CREATEDATE"
                "     or %4n >= taxN.EFFDATE)");
  }
};
}

