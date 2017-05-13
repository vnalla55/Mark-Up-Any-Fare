//----------------------------------------------------------------------------
//          File:           QueryGetSectorSurchargeSQLStatement.h
//          Description:    QueryGetSectorSurchargeSQLStatement
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
#include "DBAccess/Queries/QueryGetSectorSurcharge.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSectorSurchargeBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSectorSurchargeBaseSQLStatement() {};
  virtual ~QueryGetSectorSurchargeBaseSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FIRSTTVLDATE,
    LASTTVLDATE,
    STARTTIME,
    STOPTIME,
    SURCHARGENODEC1,
    SURCHARGEAMT1,
    SURCHARGENODEC2,
    SURCHARGEAMT2,
    SURCHARGEPERCENTNODEC,
    SURCHARGEPERCENT,
    USERAPPLTYPE,
    USERAPPL,
    DIRECTIONALIND,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    SURCHARGEAPPL,
    POSLOCTYPE,
    POSLOC,
    POILOCTYPE,
    POILOC,
    EXCEPTTKTGCARRIER,
    DOW,
    SURCHARGETYPE,
    SURCHARGECUR1,
    SURCHARGECUR2,
    EXCLPSGTYPE,
    PSGTYPECHILD,
    PSGTYPEINFANT,
    PSGTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ss.CARRIER,ss.VERSIONDATE,ss.SEQNO,ss.CREATEDATE,EXPIREDATE,"
                  "       EFFDATE,DISCDATE,FIRSTTVLDATE,LASTTVLDATE,STARTTIME,STOPTIME,"
                  "       SURCHARGENODEC1,SURCHARGEAMT1,SURCHARGENODEC2,SURCHARGEAMT2,"
                  "       SURCHARGEPERCENTNODEC,SURCHARGEPERCENT,USERAPPLTYPE,USERAPPL,"
                  "       DIRECTIONALIND,LOC1TYPE,LOC1,LOC2TYPE,LOC2,SURCHARGEAPPL,"
                  "       POSLOCTYPE,POSLOC,POILOCTYPE,POILOC,EXCEPTTKTGCARRIER,DOW,"
                  "       SURCHARGETYPE,SURCHARGECUR1,SURCHARGECUR2,EXCLPSGTYPE,"
                  "       PSGTYPECHILD,PSGTYPEINFANT,pt.PSGTYPE");

    //		        this->From("=SECTORSURCHARGE ss LEFT OUTER JOIN =SECTORSURCHPSGTYPE pt"
    //		                   "                            USING
    //(CARRIER,VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("CARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=SECTORSURCHARGE", "ss", "LEFT OUTER JOIN", "=SECTORSURCHPSGTYPE", "pt", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("ss.CARRIER = %1q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,VERSIONDATE,SEQNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SectorSurcharge* mapRowToSectorSurchBase(Row* row, SectorSurcharge* ssPrev)
  { // Load up Parent Determinant Fields
    CarrierCode carrier = row->getString(CARRIER);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    SectorSurcharge* ss;

    // If Parent hasn't changed, add to Children
    if (ssPrev != nullptr && ssPrev->carrier() == carrier && ssPrev->versionDate() == versionDate &&
        ssPrev->seqNo() == seqNo && ssPrev->createDate() == createDate)
    { // Just add to Prev
      ss = ssPrev;
    }
    else
    { // Time for a new Parent
      ss = new tse::SectorSurcharge;
      ss->carrier() = carrier;
      ss->versionDate() = versionDate;
      ss->seqNo() = seqNo;
      ss->createDate() = createDate;

      ss->expireDate() = row->getDate(EXPIREDATE);
      ss->effDate() = row->getDate(EFFDATE);
      ss->discDate() = row->getDate(DISCDATE);
      ss->firstTvlDate() = row->getDate(FIRSTTVLDATE);
      ss->lastTvlDate() = row->getDate(LASTTVLDATE);
      ss->startTime() = row->getInt(STARTTIME);
      ss->stopTime() = row->getInt(STOPTIME);

      ss->surchargeNoDec1() = row->getInt(SURCHARGENODEC1);
      ss->surchargeAmt1() =
          QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEAMT1), ss->surchargeNoDec1());

      ss->surchargeNoDec2() = row->getInt(SURCHARGENODEC2);
      ss->surchargeAmt2() =
          QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEAMT2), ss->surchargeNoDec2());

      ss->surchargePercentNoDec() = row->getInt(SURCHARGEPERCENTNODEC);
      ss->surchargePercent() =
          QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEPERCENT), ss->surchargePercentNoDec());

      ss->userApplType() = row->getChar(USERAPPLTYPE);
      ss->userAppl() = row->getString(USERAPPL);

      std::string dir = row->getString(DIRECTIONALIND);
      if (dir == "F")
        ss->directionalInd() = FROM;
      else if (dir == "W")
        ss->directionalInd() = WITHIN;
      else if (dir == "O")
        ss->directionalInd() = ORIGIN;
      else if (dir == "X")
        ss->directionalInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        ss->directionalInd() = BETWEEN;

      LocKey* loc = &ss->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &ss->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      ss->surchargeAppl() = row->getChar(SURCHARGEAPPL);

      loc = &ss->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);

      loc = &ss->poiLoc();
      loc->locType() = row->getChar(POILOCTYPE);
      loc->loc() = row->getString(POILOC);

      ss->exceptTktgCarrier() = row->getChar(EXCEPTTKTGCARRIER);
      ss->dow() = row->getString(DOW);
      ss->surchargeType() = row->getChar(SURCHARGETYPE);
      ss->surchargeCur1() = row->getString(SURCHARGECUR1);
      ss->surchargeCur2() = row->getString(SURCHARGECUR2);
      ss->exclPsgType() = row->getChar(EXCLPSGTYPE);
      ss->psgTypeChild() = row->getChar(PSGTYPECHILD);
      ss->psgTypeInfant() = row->getChar(PSGTYPEINFANT);
    } // New Parent

    // Add new CopCarrier & return
    if (!row->isNull(PSGTYPE))
    {
      PaxTypeCode pt = row->getString(PSGTYPE);
      ss->psgTypes().push_back(pt);
    }
    return ss;
  } // mapRowToSectorSurchBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for Historical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSectorSurchargeBaseHistoricalSQLStatement
    : public QueryGetSectorSurchargeBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where(" ss.CARRIER = %1q ");
    this->OrderBy("CARRIER,VERSIONDATE,SEQNO,CREATEDATE");
  }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSectorSurchargeBase
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSectorSurchargeBaseSQLStatement
    : public QueryGetSectorSurchargeBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for Historical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSectorSurchargeBaseHistoricalSQLStatement
    : public QueryGetSectorSurchargeBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("CARRIER,VERSIONDATE,SEQNO,CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
// QueryGetSectSurchTktgCxrs
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSectSurchTktgCxrsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSectSurchTktgCxrsSQLStatement() {};
  virtual ~QueryGetSectSurchTktgCxrsSQLStatement() {};

  enum ColumnIndexes
  {
    TKTGCARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TKTGCARRIER");
    this->From("=SECTORSURCHTKTGCXR");
    this->Where("CARRIER = %1q"
                "   and VERSIONDATE = %2n"
                "   and SEQNO = %3n"
                "   and CREATEDATE = %4n");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER, VERSIONDATE, SEQNO, CREATEDATE, TKTGCARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const CarrierCode mapRowToCarrier(Row* row) { return row->getString(TKTGCARRIER); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

} // tse
