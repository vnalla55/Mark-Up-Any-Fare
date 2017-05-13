//----------------------------------------------------------------------------
//          File:           QueryGetFareTypeMatrixsSQLStatement.h
//          Description:    QueryGetFareTypeMatrixsSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareTypeMatrixs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareTypeMatrixsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareTypeMatrixsSQLStatement() {};
  virtual ~QueryGetFareTypeMatrixsSQLStatement() {};

  enum ColumnIndexes
  {
    FARETYPE = 0,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    CABIN,
    FARETYPEDESIG,
    FARETYPEAPPL,
    RESTRIND,
    FARETYPEDISPLAY,
    DESCRIPTION,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARETYPE,SEQNO,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  "CABIN,FARETYPEDESIG,FARETYPEAPPL,RESTRIND,FARETYPEDISPLAY,"
                  "DESCRIPTION,VERSIONINHERITEDIND,VERSIONDISPLAYIND");
    this->From("=FARETYPEMATRIX");
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("CABIN,SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FareTypeMatrix* mapRowToFareTypeMatrix(Row* row)
  {
    tse::FareTypeMatrix* FareTypeMatrix = new tse::FareTypeMatrix;

    FareTypeMatrix->fareType() = row->getString(FARETYPE);
    FareTypeMatrix->seqNo() = row->getLong(SEQNO);
    FareTypeMatrix->createDate() = row->getDate(CREATEDATE);
    FareTypeMatrix->expireDate() = row->getDate(EXPIREDATE);
    FareTypeMatrix->effDate() = row->getDate(EFFDATE);
    FareTypeMatrix->discDate() = row->getDate(DISCDATE);
    Indicator cabin = row->getChar(CABIN);
    FareTypeMatrix->cabin().setClass(cabin);
    int ftd = row->getInt(FARETYPEDESIG);
    FareTypeMatrix->fareTypeDesig().setFareTypeDesignator(ftd);
    FareTypeMatrix->fareTypeAppl() = row->getChar(FARETYPEAPPL);
    FareTypeMatrix->restrInd() = row->getChar(RESTRIND);
    FareTypeMatrix->fareTypeDisplay() = row->getChar(FARETYPEDISPLAY);
    FareTypeMatrix->description() = row->getString(DESCRIPTION);
    FareTypeMatrix->versioninheritedInd() = row->getChar(VERSIONINHERITEDIND);
    FareTypeMatrix->versionDisplayInd() = row->getChar(VERSIONDISPLAYIND);

    return FareTypeMatrix;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

template <class QUERYCLASS>
class QueryGetFareTypeMatrixsHistoricalSQLStatement
    : public QueryGetFareTypeMatrixsSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("CABIN,SEQNO,CREATEDATE");
  }
};
}

