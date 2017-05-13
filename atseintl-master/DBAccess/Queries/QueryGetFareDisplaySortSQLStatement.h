//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplaySortSQLStatement.h
//          Description:    QueryGetFareDisplaySortSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetFareDisplaySort.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDisplaySortSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDisplaySortSQLStatement() {};
  virtual ~QueryGetFareDisplaySortSQLStatement() {};

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
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NEWSEQNO,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    SORTBYGLOBALDIR,
    SORTBYROUTING,
    SORTBYNMLSPECIAL,
    SORTBYOWRT,
    SORTBYFAREBASIS,
    SORTBYFAREBASISCHARCOMB,
    SORTBYPSGTYPE,
    SORTBYEXPIREDATE,
    SORTBYPUBLICPRIVATE,
    SORTBYAMOUNT,
    DOUBLEOWFARES,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,FAREDISPLAYTYPE,"
        "       DOMINTLAPPL,VERSIONDATE,SEQNO,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NEWSEQNO,"
        "       VERSIONINHERITEDIND,VERSIONDISPLAYIND,SORTBYGLOBALDIR,SORTBYROUTING,"
        "       SORTBYNMLSPECIAL,SORTBYOWRT,SORTBYFAREBASIS,SORTBYFAREBASISCHARCOMB,"
        "       SORTBYPSGTYPE,SORTBYEXPIREDATE,SORTBYPUBLICPRIVATE,SORTBYAMOUNT,"
        "       DOUBLEOWFARES");
    this->From("=FAREDISPLAYSORT ");
    this->Where("USERAPPLTYPE = %1q "
                "   and USERAPPL = %2q "
                "   and PSEUDOCITYTYPE = %3q "
                "   and PSEUDOCITY = %4q "
                "   and SSGGROUPNO = %5n "
                "   and %cd <= EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("SEQNO,SSGGROUPNO,FAREDISPLAYTYPE,DOMINTLAPPL,VERSIONDATE");
    else
      this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareDisplaySort* mapRowToFareDisplaySort(Row* row)
  {
    FareDisplaySort* info = new FareDisplaySort;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->pseudoCity() = row->getString(PSEUDOCITY);
    info->ssgGroupNo() = row->getInt(SSGGROUPNO);
    info->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);
    info->domIntlAppl() = row->getChar(DOMINTLAPPL);
    info->versionDate() = row->getDate(VERSIONDATE);
    info->seqno() = row->getLong(SEQNO);
    info->newSeqno() = row->getLong(NEWSEQNO);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);
    info->effDate() = row->getDate(EFFDATE);
    info->discDate() = row->getDate(DISCDATE);
    info->versionInheritedInd() = row->getChar(VERSIONINHERITEDIND);
    info->versionDisplayInd() = row->getChar(VERSIONDISPLAYIND);
    info->sortByGlobalDir() = row->getChar(SORTBYGLOBALDIR);
    info->sortByRouting() = row->getChar(SORTBYROUTING);
    info->sortByNMLSpecial() = row->getChar(SORTBYNMLSPECIAL);
    info->sortByOWRT() = row->getChar(SORTBYOWRT);
    info->sortByFareBasis() = row->getChar(SORTBYFAREBASIS);
    info->sortByFareBasisCharComb() = row->getChar(SORTBYFAREBASISCHARCOMB);
    info->sortByPsgType() = row->getChar(SORTBYPSGTYPE);
    info->sortByExpireDate() = row->getChar(SORTBYEXPIREDATE);
    info->sortByPublicPrivate() = row->getChar(SORTBYPUBLICPRIVATE);
    info->sortByAmount() = row->getChar(SORTBYAMOUNT);
    info->doubleOWFares() = row->getChar(DOUBLEOWFARES);

    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDisplaySort
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDisplaySortSQLStatement
    : public QueryGetFareDisplaySortSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SEQNO,SSGGROUPNO,"
                    "FAREDISPLAYTYPE,DOMINTLAPPL,VERSIONDATE");
    else
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SEQNO");
  }
};
} // tse
