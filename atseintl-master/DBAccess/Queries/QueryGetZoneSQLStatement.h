//----------------------------------------------------------------------------
//          File:           QueryGetZoneSQLStatement.h
//          Description:    QueryGetZoneSQLStatement
//          Created:        10/5/2007
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

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetZone.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetZoneSQLStatement() {}
  virtual ~QueryGetZoneSQLStatement() {}

  enum ColumnIndexes
  { VENDOR = 0,
    ZONENO,
    ZONETYPE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DESCRIPTION,
    ORDERNO,
    LOCTYPE,
    SETNO,
    DIRECTIONALQUALIFIER,
    INCLEXCLIND,
    LOC,
    NUMBEROFCOLUMNS }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select z.VENDOR,z.ZONENO,z.ZONETYPE,z.CREATEDATE,EXPIREDATE,EFFDATE,"
        "       DISCDATE,DESCRIPTION,ORDERNO,LOCTYPE,SETNO,DIRECTIONALQUALIFIER,INCLEXCLIND,LOC");
    this->From("=ZONE z, =ZONESEQ s");
    this->Where(" z.VENDOR = s.VENDOR"
                "    and z.ZONENO = s.ZONENO"
                "    and z.ZONETYPE = s.ZONETYPE"
                "    and z.CREATEDATE = s.CREATEDATE"
                "    and z.VENDOR = %1q "
                "    and z.ZONENO = %2q "
                "    and z.ZONETYPE = %3q ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ZONENO,ZONETYPE,CREATEDATE,SETNO,ORDERNO,LOC");
    else
      this->OrderBy("z.VENDOR, z.ZONENO, z.ZONETYPE, z.CREATEDATE, s.SETNO, s.ORDERNO ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static ZoneInfo* mapRowToZone(Row* row, ZoneInfo* prevZone, DateTime& prevDate, int& prevSet)
  {
    ZoneInfo* zoneInfo = nullptr;

    VendorCode vendor = row->getString(VENDOR);
    Zone zone = row->getString(ZONENO);
    Indicator zoneType = row->getChar(ZONETYPE);
    DateTime createDate = row->getDate(CREATEDATE);
    if (prevZone == nullptr || prevZone->vendor() != vendor || prevZone->zone() != zone ||
        prevZone->zoneType() != zoneType || prevDate != createDate)
    {
      zoneInfo = new ZoneInfo;
      zoneInfo->vendor() = vendor;
      zoneInfo->zone() = zone;
      zoneInfo->zoneType() = zoneType;
      prevDate = createDate;
      zoneInfo->expireDate() = row->getDate(EXPIREDATE);
      zoneInfo->effDate() = row->getDate(EFFDATE);
      zoneInfo->discDate() = row->getDate(DISCDATE);
      zoneInfo->setDescription(row->getString(DESCRIPTION));
      zoneInfo->setUniform();
      prevSet = 0;
    }
    else
    {
      zoneInfo = prevZone;
    }

    int set = row->getInt(SETNO);
    if (prevSet != set)
    {
      prevSet = set;
      zoneInfo->sets().resize(zoneInfo->sets().size() + 1);
    }
    std::vector<ZoneInfo::ZoneSeg>& segs = zoneInfo->sets().back();
    segs.resize(segs.size() + 1);
    ZoneInfo::ZoneSeg& zoneSeg = segs.back();
    zoneSeg.locType() = row->getChar(LOCTYPE);
    zoneSeg.loc() = row->getString(LOC);
    zoneSeg.inclExclInd() = row->getChar(INCLEXCLIND);
    zoneSeg.directionalQualifier() = row->getChar(DIRECTIONALQUALIFIER);
    if (zoneInfo->isUniform())
    {
      if ((zoneSeg.inclExclInd() != 'I') || (zoneSeg.locType() == LOCTYPE_ZONE) ||
          (segs.size() > 1 && zoneSeg.locType() != (segs.end()-2)->locType()))
      {
        zoneInfo->setUniform(false);
      }
      else if (zoneInfo->sets().size() > 1 &&
               zoneSeg.locType() != (zoneInfo->sets().end()-2)->back().locType())
      {
        zoneInfo->setUniform(false);
      }
    }
    return zoneInfo;
  } // mapRowToZone()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetZoneHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetZoneHistoricalSQLStatement : public QueryGetZoneSQLStatement<QUERYCLASS>

{
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select z.VENDOR as VENDOR,z.ZONENO as ZONENO,"
                             " z.ZONETYPE as ZONETYPE,"
                             " z.CREATEDATE as CREATEDATE,EXPIREDATE,EFFDATE,"
                             " DISCDATE,DESCRIPTION,ORDERNO as ORDERNO,LOCTYPE,"
                             " SETNO as SETNO,DIRECTIONALQUALIFIER,"
                             " INCLEXCLIND,LOC");
    partialStatement.From("=ZONE z, =ZONESEQ s");
    partialStatement.Where(" z.VENDOR = s.VENDOR"
                           " and z.ZONENO = s.ZONENO"
                           " and z.ZONETYPE = s.ZONETYPE"
                           " and z.CREATEDATE = s.CREATEDATE"
                           " and z.VENDOR = %1q "
                           " and z.ZONENO = %2q "
                           " and z.ZONETYPE = %3q "
                           " and %4n <= z.EXPIREDATE"
                           " and %5n >= z.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             " select n.VENDOR,n.ZONENO,n.ZONETYPE,"
                             "  n.CREATEDATE,EXPIREDATE,EFFDATE,"
                             "  DISCDATE,DESCRIPTION,ORDERNO,LOCTYPE,SETNO,"
                             "  DIRECTIONALQUALIFIER,INCLEXCLIND,LOC ");
    partialStatement.From(" =ZONEH n, =ZONESEQH q ");
    partialStatement.Where(" n.VENDOR = q.VENDOR"
                           " and n.ZONENO = q.ZONENO"
                           " and n.ZONETYPE = q.ZONETYPE"
                           " and n.CREATEDATE = q.CREATEDATE"
                           " and n.VENDOR = %6q "
                           " and n.ZONENO = %7q "
                           " and n.ZONETYPE = %8q "
                           " and %9n <= n.EXPIREDATE"
                           " and %10n >= n.CREATEDATE"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("VENDOR, ZONENO, ZONETYPE, CREATEDATE, SETNO, ORDERNO");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};
}

