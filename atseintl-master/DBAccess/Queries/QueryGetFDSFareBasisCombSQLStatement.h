//----------------------------------------------------------------------------
//          File:           QueryGetFDSFareBasisCombSQLStatement.h
//          Description:    QueryGetFDSFareBasisCombSQLStatement
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
#include "DBAccess/Queries/QueryGetFDSFareBasisComb.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFDSFareBasisCombSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFDSFareBasisCombSQLStatement() {};
  virtual ~QueryGetFDSFareBasisCombSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    SSGGROUPNO,
    FAREDISPLAYTYPE,
    DOMINTLAPPL,
    SEQNO,
    ORDERNO,
    FAREBASISCHARCOMB,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,FAREDISPLAYTYPE,"
        " DOMINTLAPPL,SEQNO,ORDERNO,FAREBASISCHARCOMB");
    this->From("=FAREDISPSORTCHAR");
    this->Where("USERAPPLTYPE = %1q "
                " and USERAPPL = %2q "
                " and PSEUDOCITYTYPE = %3q "
                " and PSEUDOCITY = %4q "
                " and SSGGROUPNO = %5n ");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FDSFareBasisComb* mapRowToFDSFareBasisComb(Row* row)
  {
    FDSFareBasisComb* info = new FDSFareBasisComb;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->pseudoCity() = row->getString(PSEUDOCITY);
    info->ssgGroupNo() = row->getInt(SSGGROUPNO);
    info->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);
    info->domIntlAppl() = row->getChar(DOMINTLAPPL);
    info->seqno() = row->getLong(SEQNO);
    info->orderNo() = row->getInt(ORDERNO);
    info->fareBasisCharComb() = row->getString(FAREBASISCHARCOMB);

    return info;
  }; // mapRowToFDSFareBasisComb()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFDSFareBasisCombSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFDSFareBasisCombSQLStatement
    : public QueryGetFDSFareBasisCombSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, SEQNO, SSGGROUPNO");
  };
}; // class QueryGetAllFDSFareBasisCombSQLStatement
}

