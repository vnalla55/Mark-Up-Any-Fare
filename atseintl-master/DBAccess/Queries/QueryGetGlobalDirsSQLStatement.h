//----------------------------------------------------------------------------
//          File:           QueryGetGlobalDirsSQLStatement.h
//          Description:    QueryGetGlobalDirsSQLStatement
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

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGlobalDirs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetGlobalDirsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGlobalDirsSQLStatement() {};
  virtual ~QueryGetGlobalDirsSQLStatement() {};

  enum ColumnIndexes
  {
    GLOBALDIR = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DISPLAYONLYIND,
    DESCRIPTION,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    ALLTVLONCARRIER,
    SALEEFFDATE,
    SALEDISCDATE,
    FLIGHTTRACKINGIND,
    MUSTBEVIALOCIND,
    MUSTBEVIALOCTYPE,
    MUSTBEVIALOC,
    VIAINTERLOC1TYPE,
    VIAINTERLOC1,
    VIAINTERLOC2TYPE,
    VIAINTERLOC2,
    MUSTNOTBEVIALOCIND,
    MUSTNOTBEVIALOCTYPE,
    MUSTNOTBEVIALOC,
    NOTVIAINTERLOC1TYPE,
    NOTVIAINTERLOC1,
    NOTVIAINTERLOC2TYPE,
    NOTVIAINTERLOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select gdir.GLOBALDIR,gdir.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  " DISPLAYONLYIND,DESCRIPTION,DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,"
                  " ALLTVLONCARRIER,SALEEFFDATE,SALEDISCDATE,FLIGHTTRACKINGIND,"
                  " MUSTBEVIALOCIND,MUSTBEVIALOCTYPE,MUSTBEVIALOC,VIAINTERLOC1TYPE,"
                  " VIAINTERLOC1,VIAINTERLOC2TYPE,VIAINTERLOC2,MUSTNOTBEVIALOCIND,"
                  " MUSTNOTBEVIALOCTYPE,MUSTNOTBEVIALOC,NOTVIAINTERLOC1TYPE,NOTVIAINTERLOC1,"
                  " NOTVIAINTERLOC2TYPE,NOTVIAINTERLOC2");
    this->From("=GLOBALDIR gdir, =GLOBALDIRSEG gdirseg");
    this->Where("gdir.GLOBALDIR = gdirseg.GLOBALDIR"
                " and gdir.CREATEDATE = gdirseg.CREATEDATE"
                " and %cd <= gdir.EXPIREDATE");
    this->OrderBy("gdir.GLOBALDIR, gdir.CREATEDATE, DIRECTIONALITY, LOC1TYPE, LOC1, LOC2TYPE, "
                  "LOC2, ALLTVLONCARRIER, SALEEFFDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::GlobalDir* mapRowToGlobalDir(Row* row)
  {
    tse::GlobalDir* pMap = new tse::GlobalDir;

    strToGlobalDirection(pMap->globalDir(), row->getString(GLOBALDIR));
    pMap->createDate() = row->getDate(CREATEDATE);
    pMap->expireDate() = row->getDate(EXPIREDATE);
    pMap->effDate() = row->getDate(EFFDATE);
    pMap->discDate() = row->getDate(DISCDATE);
    pMap->displayOnlyInd() = row->getChar(DISPLAYONLYIND);
    pMap->description() = row->getString(DESCRIPTION);

    return pMap;
  }; // mapRowToGlobalDir()

  static tse::GlobalDirSeg* mapRowToGlobalDirSeg(Row* row)
  {
    tse::GlobalDirSeg* pMap = new tse::GlobalDirSeg;

    pMap->globalDir() = row->getString(GLOBALDIR);
    pMap->directionality() = row->getChar(DIRECTIONALITY);
    pMap->loc1Type() = LocType(row->getString(LOC1TYPE)[0]);
    pMap->loc1() = row->getString(LOC1);
    pMap->loc2Type() = LocType(row->getString(LOC2TYPE)[0]);
    pMap->loc2() = row->getString(LOC2);
    pMap->allTvlOnCarrier() = row->getString(ALLTVLONCARRIER);
    pMap->saleEffDate() = row->getDate(SALEEFFDATE);
    pMap->saleDiscDate() = row->getDate(SALEDISCDATE);
    pMap->flightTrackingInd() = row->getChar(FLIGHTTRACKINGIND);
    pMap->mustBeViaLocInd() = row->getChar(MUSTBEVIALOCIND);
    pMap->mustBeViaLocType() = LocType(row->getString(MUSTBEVIALOCTYPE)[0]);
    pMap->mustBeViaLoc() = row->getString(MUSTBEVIALOC);
    pMap->viaInterLoc1Type() = LocType(row->getString(VIAINTERLOC1TYPE)[0]);
    pMap->viaInterLoc1() = row->getString(VIAINTERLOC1);
    pMap->viaInterLoc2Type() = LocType(row->getString(VIAINTERLOC2TYPE)[0]);
    pMap->viaInterLoc2() = row->getString(VIAINTERLOC2);
    pMap->mustNotBeViaLocInd() = row->getChar(MUSTNOTBEVIALOCIND);
    pMap->mustNotBeViaLocType() = LocType(row->getString(MUSTNOTBEVIALOCTYPE)[0]);
    pMap->mustNotBeViaLoc() = row->getString(MUSTNOTBEVIALOC);
    pMap->notViaInterLoc1Type() = LocType(row->getString(NOTVIAINTERLOC1TYPE)[0]);
    pMap->notViaInterLoc1() = row->getString(NOTVIAINTERLOC1);
    pMap->notViaInterLoc2Type() = LocType(row->getString(NOTVIAINTERLOC2TYPE)[0]);
    pMap->notViaInterLoc2() = row->getString(NOTVIAINTERLOC2);

    return pMap;
  }; // mapRowToGlobalDirSeg()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetGlobalDirsSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGlobalDirsHistoricalSQLStatement : public QueryGetGlobalDirsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("gdir.GLOBALDIR = gdirseg.GLOBALDIR and gdir.CREATEDATE = gdirseg.CREATEDATE");
    this->OrderBy("gdir.GLOBALDIR, gdir.CREATEDATE, DIRECTIONALITY, LOC1TYPE, LOC1, LOC2TYPE, "
                  "LOC2, ALLTVLONCARRIER, SALEEFFDATE");
  }; // adjustBaseSQL()
}; // class QueryGetGlobalDirsHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllGlobalDirsHistoricalSQLStatement
    : public QueryGetGlobalDirsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("gdir.GLOBALDIR = gdirseg.GLOBALDIR and gdir.CREATEDATE = gdirseg.CREATEDATE");
    this->OrderBy("gdir.GLOBALDIR, gdir.CREATEDATE, DIRECTIONALITY, LOC1TYPE, LOC1, LOC2TYPE, "
                  "LOC2, ALLTVLONCARRIER, SALEEFFDATE");
  }; // adjustBaseSQL()
}; // class QueryGetAllGlobalDirsHistoricalSQLStatement
}

