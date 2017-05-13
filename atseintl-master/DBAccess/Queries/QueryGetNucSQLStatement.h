//----------------------------------------------------------------------------
//          File:           QueryGetNucSQLStatement.h
//          Description:    QueryGetNucSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNuc.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetOneNucSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOneNucSQLStatement() {};
  virtual ~QueryGetOneNucSQLStatement() {};

  enum ColumnIndexes
  {
    CUR = 0,
    CARRIER,
    EXPIREDATE,
    EFFDATE,
    CREATEDATE,
    DISCDATE,
    NUCFACTOR,
    ROUNDINGFACTOR,
    NUCFACTORNODEC,
    ROUNDINGFACTORNODEC,
    ROUNDINGRULE,
    VENDOR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CUR,CARRIER,EXPIREDATE,EFFDATE,CREATEDATE,DISCDATE,"
                  "       NUCFACTOR,ROUNDINGFACTOR,NUCFACTORNODEC,ROUNDINGFACTORNODEC,"
                  "       ROUNDINGRULE,VENDOR");
    this->From("=NUC ");
    this->Where("CUR = %1q "
                "    and CARRIER = %2q "
                "    and EXPIREDATE >= %cd");

    if (DataManager::forceSortOrder())
      this->OrderBy("CUR,CARRIER,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NUCInfo* mapRowToNUCInfo(Row* row)
  {
    tse::NUCInfo* pNUC = new tse::NUCInfo;

    pNUC->_cur = row->getString(CUR);
    pNUC->_carrier = row->getString(CARRIER);
    pNUC->_discDate = row->getDate(DISCDATE);
    pNUC->_effDate = row->getDate(EFFDATE);
    pNUC->_createDate = row->getDate(CREATEDATE);
    pNUC->_expireDate = row->getDate(EXPIREDATE);

    pNUC->_nucFactorNodec = row->getInt(NUCFACTORNODEC);
    pNUC->_nucFactor = QUERYCLASS::adjustDecimal(row->getInt(NUCFACTOR), pNUC->_nucFactorNodec);
    pNUC->_roundingFactorNodec = row->getInt(ROUNDINGFACTORNODEC);
    pNUC->_roundingFactor =
        QUERYCLASS::adjustDecimal(row->getInt(ROUNDINGFACTOR), pNUC->_roundingFactorNodec);
    std::string roundRule = row->getString(ROUNDINGRULE);
    if (roundRule == "D")
      pNUC->_roundingRule = DOWN;
    else if (roundRule == "U")
      pNUC->_roundingRule = UP;
    else if (roundRule == "N")
      pNUC->_roundingRule = NEAREST;
    else
      pNUC->_roundingRule = NONE;

    return pNUC;
  } // mapRowToNUCInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllNucsSQLStatement : public QueryGetOneNucSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("EXPIREDATE >= %cd");
    this->OrderBy(" CUR, CARRIER, CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace From and Where clauses
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetNucHistoricalSQLStatement : public QueryGetOneNucSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select CUR,CARRIER,EXPIREDATE,EFFDATE,CREATEDATE,DISCDATE,"
                             " NUCFACTOR,ROUNDINGFACTOR,NUCFACTORNODEC,ROUNDINGFACTORNODEC,"
                             " ROUNDINGRULE,VENDOR");
    partialStatement.From("=NUCH ");
    partialStatement.Where("CUR = %1q "
                           "and CARRIER = %2q "
                           "and (CREATEDATE <= %3n "
                           "or   EFFDATE <= %4n) "
                           "and EXPIREDATE >= %5n "
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select CUR,CARRIER,EXPIREDATE,EFFDATE,CREATEDATE,DISCDATE,"
                             " NUCFACTOR,ROUNDINGFACTOR,NUCFACTORNODEC,ROUNDINGFACTORNODEC,"
                             " ROUNDINGRULE,VENDOR");
    partialStatement.From("=NUC ");
    partialStatement.Where("CUR = %6q "
                           "and CARRIER = %7q "
                           "and (CREATEDATE <= %8n "
                           "or   EFFDATE <= %9n) "
                           "and EXPIREDATE >= %10n "
                           ")");
    partialStatement.OrderBy(" CREATEDATE");

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

template <class QUERYCLASS>
class QueryGetOneNucAllCarriersSQLStatement : public QueryGetOneNucSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("CUR = %1q "
                "    and EXPIREDATE >= %cd");
    this->OrderBy(" CARRIER, CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetAllNucsAllCarriersSQLStatement
    : public QueryGetOneNucAllCarriersSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("EXPIREDATE >= %cd");
    this->OrderBy(" CUR, CARRIER, CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetNucAllCarriersHistoricalSQLStatement : public QueryGetOneNucSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select CUR,CARRIER,EXPIREDATE,EFFDATE,CREATEDATE,DISCDATE,"
                             " NUCFACTOR,ROUNDINGFACTOR,NUCFACTORNODEC,ROUNDINGFACTORNODEC,"
                             " ROUNDINGRULE,VENDOR");
    partialStatement.From("=NUCH ");
    partialStatement.Where("CUR = %1q "
                           "   and DISCDATE > EFFDATE "
                           "   and EXPIREDATE > EFFDATE "
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select CUR,CARRIER,EXPIREDATE,EFFDATE,CREATEDATE,DISCDATE,"
                             " NUCFACTOR,ROUNDINGFACTOR,NUCFACTORNODEC,ROUNDINGFACTORNODEC,"
                             " ROUNDINGRULE,VENDOR");
    partialStatement.From("=NUC ");
    partialStatement.Where("CUR = %2q "
                           "   and DISCDATE > EFFDATE "
                           "   and EXPIREDATE > EFFDATE "
                           ")");
    partialStatement.OrderBy("CARRIER, CREATEDATE");

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
}

