//----------------------------------------------------------------------------
//          File:           QueryGetFDSPsgTypeSQLStatement.h
//          Description:    QueryGetFDSPsgTypeSQLStatement
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
#include "DBAccess/Queries/QueryGetFDSPsgType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFDSPsgTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFDSPsgTypeSQLStatement() {};
  virtual ~QueryGetFDSPsgTypeSQLStatement() {};

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
    PSGTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,FAREDISPLAYTYPE,"
        " DOMINTLAPPL,SEQNO,ORDERNO,PSGTYPE");
    this->From("=FAREDISPSORTPSG");
    this->Where(" USERAPPLTYPE = %1q "
                "    and USERAPPL = %2q "
                "    and PSEUDOCITYTYPE = %3q "
                "    and PSEUDOCITY = %4q "
                "    and SSGGROUPNO = %5n ");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FDSPsgType* mapRowToFDSPsgType(Row* row)
  {
    FDSPsgType* info = new FDSPsgType;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->pseudoCity() = row->getString(PSEUDOCITY);
    info->ssgGroupNo() = row->getInt(SSGGROUPNO);
    info->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);
    info->domIntlAppl() = row->getChar(DOMINTLAPPL);
    info->seqno() = row->getLong(SEQNO);
    info->orderNo() = row->getInt(ORDERNO);
    info->psgType() = row->getString(PSGTYPE);

    return info;
  }; // mapRowToFDSGlobalDir()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFDSGlobalDirSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFDSPsgTypeSQLStatement : public QueryGetFDSPsgTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, SEQNO, SSGGROUPNO");
  }; // adjustBaseSQL()
}; // class QueryGetAllFDSPsgTypeSQLStatement
}

