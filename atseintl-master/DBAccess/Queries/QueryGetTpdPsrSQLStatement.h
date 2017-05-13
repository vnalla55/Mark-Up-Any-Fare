//----------------------------------------------------------------------------
//          File:           QueryGetTpdPsrSQLStatement.h
//          Description:    QueryGetTpdPsrSQLStatement
//          Created:        10/6/2007
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
#include "DBAccess/Queries/QueryGetTpdPsr.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTpdPsrBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTpdPsrBaseSQLStatement() {}
  virtual ~QueryGetTpdPsrBaseSQLStatement() {}

  enum ColumnIndexes
  {
    APPLIND = 0,
    CARRIER,
    AREA1,
    AREA2,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    EFFTVLDATE,
    DISCTVLDATE,
    TPMDEDUCTION,
    GLOBALDIR,
    ISICODE,
    FARETYPEAPPL,
    PSRHIP,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    THISCARRIERRESTR,
    THRUVIALOCRESTR,
    STOPOVERCNT,
    THRUVIAMKTSAMECXR,
    THRUMKTCARRIEREXCEPT,
    TPDTHRUVIAMKTONLYIND,
    THRUMKTCARRIER,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select t.APPLIND,t.CARRIER,t.AREA1,t.AREA2,t.VERSIONDATE,t.SEQNO,"
                  "t.CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,EFFTVLDATE,"
                  "DISCTVLDATE,TPMDEDUCTION,GLOBALDIR,ISICODE,FARETYPEAPPL,"
                  "PSRHIP,LOC1TYPE,LOC1,LOC2TYPE,LOC2,THISCARRIERRESTR,"
                  "THRUVIALOCRESTR,STOPOVERCNT,THRUVIAMKTSAMECXR,"
                  "THRUMKTCARRIEREXCEPT,TPDTHRUVIAMKTONLYIND,c.THRUMKTCARRIER");

    //		        this->From("=TPDPSR t LEFT OUTER JOIN =TPDPSRTHRUMKTCXR c"
    //		                   " USING (APPLIND,CARRIER,AREA1,AREA2,VERSIONDATE,"
    //		                   " SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("APPLIND");
    joinFields.push_back("CARRIER");
    joinFields.push_back("AREA1");
    joinFields.push_back("AREA2");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=TPDPSR", "t", "LEFT OUTER JOIN", "=TPDPSRTHRUMKTCXR", "c", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("%cd <= EXPIREDATE"
                "  and t.APPLIND = %1q"
                "  and t.CARRIER = %2q"
                "  and t.AREA1 = %3q"
                "  and t.AREA2 = %4q");
    if (DataManager::forceSortOrder())
      this->OrderBy("t.APPLIND, t.CARRIER, t.AREA1, t.AREA2, t.VERSIONDATE, t.SEQNO, t.CREATEDATE, "
                    "c.THRUMKTCARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TpdPsr* mapRowToTpdPsrBase(Row* row, TpdPsr* tpPrev)
  {
    Indicator applInd = row->getChar(APPLIND);
    CarrierCode carrier = row->getString(CARRIER);
    Indicator area1 = row->getChar(AREA1);
    Indicator area2 = row->getChar(AREA2);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    TpdPsr* tp;

    // If Parent hasn't changed, add to Child (segs)
    if (tpPrev != nullptr && tpPrev->applInd() == applInd && tpPrev->carrier() == carrier &&
        tpPrev->area1() == area1 && tpPrev->area2() == area2 &&
        tpPrev->versionDate() == versionDate && tpPrev->seqNo() == seqNo &&
        tpPrev->createDate() == createDate)
    { // Add to Prev
      tp = tpPrev;
    } // Previous Parent
    else
    { // Time for a new Parent
      tp = new tse::TpdPsr;
      tp->applInd() = applInd;
      tp->carrier() = carrier;
      tp->area1() = area1;
      tp->area2() = area2;
      tp->versionDate() = versionDate;
      tp->seqNo() = seqNo;
      tp->createDate() = createDate;
      tp->effDate() = row->getDate(EFFDATE);
      tp->expireDate() = row->getDate(EXPIREDATE);
      tp->discDate() = row->getDate(DISCDATE);
      tp->effTvlDate() = row->getDate(EFFTVLDATE);
      tp->discTvlDate() = row->getDate(DISCTVLDATE);
      tp->tpmDeduction() = row->getInt(TPMDEDUCTION);

      std::string gd = row->getString(GLOBALDIR);
      strToGlobalDirection(tp->globalDir(), gd);

      tp->isiCode() = row->getString(ISICODE);
      tp->fareTypeAppl() = row->getChar(FARETYPEAPPL);
      tp->psrHip() = row->getChar(PSRHIP);

      LocKey* loc = &tp->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &tp->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      tp->thisCarrierRestr() = row->getChar(THISCARRIERRESTR);
      tp->thruViaLocRestr() = row->getChar(THRUVIALOCRESTR);
      tp->stopoverCnt() = QUERYCLASS::charToInt(row->getString(STOPOVERCNT));
      tp->thruViaMktSameCxr() = row->getChar(THRUVIAMKTSAMECXR);
      tp->thruMktCarrierExcept() = row->getChar(THRUMKTCARRIEREXCEPT);
      tp->tpdThruViaMktOnlyInd() = row->getChar(TPDTHRUVIAMKTONLYIND);
    } // else (New Parent)

    // Check for Child
    if (!row->isNull(THRUMKTCARRIER))
    {
      CarrierCode tmc = row->getString(THRUMKTCARRIER);
      tp->thruMktCxrs().push_back(tmc);
    }

    return tp;
  } // mapRowToTpdPsrBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTpdPsrBaseHistoricalSQLStatement : public QueryGetTpdPsrBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("t.APPLIND = %1q"
                "  and t.CARRIER = %2q"
                "  and t.AREA1 = %3q"
                "  and t.AREA2 = %4q"
                "  and %5n <= t.EXPIREDATE"
                "  and %6n >= t.CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllTpdPsrBaseSQLStatement : public QueryGetTpdPsrBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("t.APPLIND, t.CARRIER, t.AREA1, t.AREA2,t.VERSIONDATE,t.SEQNO,t.CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTpdPsrViaGeoLocsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTpdPsrViaGeoLocsSQLStatement() {}
  virtual ~QueryGetTpdPsrViaGeoLocsSQLStatement() {}

  enum ColumnIndexes
  {
    SETNO = 0,
    ORDERNO,
    LOCTYPE,
    LOC,
    RELATIONALIND,
    STOPOVERNOTALLOWED,
    NOSTOPATVIABETWVIALOC1,
    DIRECTSVCREQBETWVIALOC1,
    NOSTOPATVIABETWVIALOC2,
    DIRECTSVCREQBETWVIALOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SETNO,ORDERNO,LOCTYPE,LOC,RELATIONALIND,STOPOVERNOTALLOWED, "
                  "NOSTOPATVIABETWVIALOC1,                       DIRECTSVCREQBETWVIALOC1, "
                  "NOSTOPATVIABETWVIALOC2,DIRECTSVCREQBETWVIALOC2");

    this->From("=TPDPSRVIAGEOLOC");
    this->Where("APPLIND = %1q"
                " and CARRIER = %2q"
                " and AREA1 = %3q"
                " and AREA2 = %4q"
                " and VERSIONDATE = %5n"
                " and SEQNO = %6n"
                " and CREATEDATE = %7n");

    if (DataManager::forceSortOrder())
      this->OrderBy(
          "APPLIND, CARRIER, AREA1, AREA2, VERSIONDATE, SEQNO, CREATEDATE, SETNO, ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToTpdPsrViaGeoLoc(Row* row, TpdPsrViaGeoLoc* vgl)
  {
    vgl->setNo() = row->getInt(SETNO);
    vgl->orderNo() = row->getInt(ORDERNO);
    vgl->loc().locType() = row->getChar(LOCTYPE);
    vgl->loc().loc() = row->getString(LOC);
    vgl->relationalInd() = row->getChar(RELATIONALIND);
    vgl->stopoverNotAllowed() = row->getChar(STOPOVERNOTALLOWED);
    vgl->noStopBtwViaAndLoc1() = row->getChar(NOSTOPATVIABETWVIALOC1);
    vgl->reqDirectSvcBtwViaAndLoc1() = row->getChar(DIRECTSVCREQBETWVIALOC1);
    vgl->noStopBtwViaAndLoc2() = row->getChar(NOSTOPATVIABETWVIALOC2);
    vgl->reqDirectSvcBtwViaAndLoc2() = row->getChar(DIRECTSVCREQBETWVIALOC2);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTpdPsrViaExceptsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTpdPsrViaExceptsSQLStatement() {}
  virtual ~QueryGetTpdPsrViaExceptsSQLStatement() {}

  enum ColumnIndexes
  {
    VIACARRIER = 0,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VIACARRIER,LOC1TYPE,LOC1,LOC2TYPE,LOC2");
    this->From("=TPDPSRVIAEXCEPT");
    this->Where("APPLIND = %1q"
                "    and CARRIER = %2q"
                "    and AREA1 = %3q"
                "    and AREA2 = %4q"
                "    and VERSIONDATE = %5n"
                "    and SEQNO = %6n"
                "    and CREATEDATE = %7n");

    if (DataManager::forceSortOrder())
      this->OrderBy("APPLIND,CARRIER,AREA1,AREA2,VERSIONDATE,SEQNO,CREATEDATE,VIACARRIER,LOC1TYPE,"
                    "LOC1,LOC2TYPE,LOC2");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToTpdPsrViaExcept(Row* row, TpdPsrViaExcept* ve)
  {
    ve->viaCarrier() = row->getString(VIACARRIER);
    ve->loc1().locType() = row->getChar(LOC1TYPE);
    ve->loc1().loc() = row->getString(LOC1);
    ve->loc2().locType() = row->getChar(LOC2TYPE);
    ve->loc2().loc() = row->getString(LOC2);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTpdPsrViaCxrLocsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTpdPsrViaCxrLocsSQLStatement() {}
  virtual ~QueryGetTpdPsrViaCxrLocsSQLStatement() {}

  enum ColumnIndexes
  {
    VIACARRIER = 0,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VIACARRIER,LOC1TYPE,LOC1,LOC2TYPE,LOC2");
    this->From("=TPDPSRVIACXRLOC");
    this->Where("APPLIND = %1q"
                " and CARRIER = %2q"
                " and AREA1 = %3q"
                " and AREA2 = %4q"
                " and VERSIONDATE = %5n"
                " and SEQNO = %6n"
                " and CREATEDATE = %7n");

    if (DataManager::forceSortOrder())
      this->OrderBy("APPLIND,CARRIER,AREA1,AREA2,VERSIONDATE,SEQNO,CREATEDATE,VIACARRIER,LOC1TYPE,"
                    "LOC1,LOC2TYPE,LOC2");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToTpdPsrViaCxrLoc(Row* row, TpdPsrViaCxrLoc* vcl)
  {
    vcl->viaCarrier() = row->getString(VIACARRIER);
    vcl->loc1().locType() = row->getChar(LOC1TYPE);
    vcl->loc1().loc() = row->getString(LOC1);
    vcl->loc2().locType() = row->getChar(LOC2TYPE);
    vcl->loc2().loc() = row->getString(LOC2);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
}
