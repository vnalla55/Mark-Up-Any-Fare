//----------------------------------------------------------------------------
//          File:           QueryGetFDSSortingSQLStatement.h
//          Description:    QueryGetFDSSortingSQLStatement
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
#include "DBAccess/Queries/QueryGetFDSSorting.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFDSSortingSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFDSSortingSQLStatement() {};
  virtual ~QueryGetFDSSortingSQLStatement() {};

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
    SORTFAREBASISCHAR1,
    FAREBASISCHAR1,
    SORTFAREBASISCHAR2,
    FAREBASISCHAR2,
    SORTFAREBASISCHAR3,
    FAREBASISCHAR3,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,FAREDISPLAYTYPE,"
        " DOMINTLAPPL,SEQNO,SORTFAREBASISCHAR1,FAREBASISCHAR1,SORTFAREBASISCHAR2,"
        " FAREBASISCHAR2,SORTFAREBASISCHAR3,FAREBASISCHAR3");
    this->From("=FAREDISPSORTFB");
    this->Where("USERAPPLTYPE = %1q "
                "   and USERAPPL = %2q "
                "   and PSEUDOCITYTYPE = %3q "
                "   and PSEUDOCITY = %4q "
                "   and SSGGROUPNO = %5n ");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FDSSorting* mapRowToFDSSorting(Row* row)
  {
    FDSSorting* info = new FDSSorting;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->pseudoCity() = row->getString(PSEUDOCITY);
    info->ssgGroupNo() = row->getInt(SSGGROUPNO);
    info->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);
    info->domIntlAppl() = row->getChar(DOMINTLAPPL);
    info->seqno() = row->getLong(SEQNO);
    info->sortFareBasisChar1() = row->getChar(SORTFAREBASISCHAR1);
    info->fareBasisChar1() = row->getString(FAREBASISCHAR1);
    info->sortFareBasisChar2() = row->getChar(SORTFAREBASISCHAR2);
    info->fareBasisChar2() = row->getString(FAREBASISCHAR2);
    info->sortFareBasisChar3() = row->getChar(SORTFAREBASISCHAR3);
    info->fareBasisChar3() = row->getString(FAREBASISCHAR3);

    return info;
  }; // mapRowToFDSSorting()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFDSSortingSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFDSSortingSQLStatement : public QueryGetFDSSortingSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, SEQNO, SSGGROUPNO");
  }; // adjustBaseSQL()
}; // class QueryGetAllFDSSortingSQLStatement
}

