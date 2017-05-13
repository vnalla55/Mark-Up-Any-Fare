//----------------------------------------------------------------------------
//          File:           QueryGetFDSGlobalDirSQLStatement.h
//          Description:    QueryGetFDSGlobalDirSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFDSGlobalDir.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFDSGlobalDirSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFDSGlobalDirSQLStatement() {};
  virtual ~QueryGetFDSGlobalDirSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    SSGGROUPNO,
    FAREDISPLAYTYPE,
    DOMINTLAPPL,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    ORDERNO,
    GLOBALDIR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,FAREDISPLAYTYPE,"
        " DOMINTLAPPL,VERSIONDATE,SEQNO,CREATEDATE,ORDERNO,GLOBALDIR");
    this->From("=FAREDISPSORTGBL");
    this->Where("USERAPPLTYPE = %1q "
                "   and USERAPPL = %2q "
                "   and PSEUDOCITYTYPE = %3q "
                "   and PSEUDOCITY = %4q "
                "   and SSGGROUPNO = %5n ");

    if (DataManager::forceSortOrder())
      this->OrderBy("SEQNO, "
                    "SSGGROUPNO,FAREDISPLAYTYPE,DOMINTLAPPL,SSGGROUPNO,VERSIONDATE,CREATEDATE,"
                    "ORDERNO,GLOBALDIR");
    else
      this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FDSGlobalDir* mapRowToFDSGlobalDir(Row* row)
  {
    FDSGlobalDir* info = new FDSGlobalDir;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->pseudoCity() = row->getString(PSEUDOCITY);
    info->ssgGroupNo() = row->getInt(SSGGROUPNO);
    info->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);
    info->domIntlAppl() = row->getChar(DOMINTLAPPL);
    info->versionDate() = row->getDate(VERSIONDATE);
    info->seqno() = row->getLong(SEQNO);
    info->createDate() = row->getDate(CREATEDATE);
    info->orderNo() = row->getInt(ORDERNO);
    std::string gdir = row->getString(GLOBALDIR);
    strToGlobalDirection(info->globalDir(), gdir);

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
class QueryGetAllFDSGlobalDirSQLStatement : public QueryGetFDSGlobalDirSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SEQNO,SSGGROUPNO,"
                    "FAREDISPLAYTYPE,DOMINTLAPPL,VERSIONDATE,CREATEDATE,ORDERNO,GLOBALDIR");
    else
      this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, SEQNO, SSGGROUPNO");
  }; // adjustBaseSQL()
}; // class QueryGetAllFDSGlobalDirSQLStatement
}

