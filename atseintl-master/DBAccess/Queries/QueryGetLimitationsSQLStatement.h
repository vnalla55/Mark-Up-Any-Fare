//----------------------------------------------------------------------------
//          File:           QueryGetLimitationsSQLStatement.h
//          Description:    QueryGetLimitationsSQLStatement
//          Created:        11/01/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     [C] 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetLimitations.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetLimitJrnyBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitJrnyBaseSQLStatement() {};
  virtual ~QueryGetLimitJrnyBaseSQLStatement() {};

  enum ColumnIndexes
  {
    VERSIONDATE = 0,
    SEQNO,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    USERAPPL,
    LIMITATIONAPPL,
    EXCEPTTKTGCXRIND,
    TKTGCARRIER,
    WHOLLYWITHINAPPL,
    WHOLLYWITHINLOCTYPE,
    WHOLLYWITHINLOC,
    ORIGINTVLAPPL,
    ORIGINLOCTYPE,
    ORIGINLOC,
    FARETYPE,
    INTLDEPARTMAXNO,
    INTLARRIVALMAXNO,
    MAXRETRANSITALLOWED,
    RETRANSITPOINTAPPL,
    RETRANSITLOCTYPE,
    RETRANSITLOC,
    MAXSTOPSATRETRANSIT,
    VIAPOINTSTOPOVERAPPL,
    STOPOVERLOCTYPE,
    STOPOVERLOC,
    POSLOCTYPE,
    POSLOC,
    POTLOCTYPE,
    POTLOC,
    SEPARATETKTIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select l.VERSIONDATE,l.SEQNO,l.CREATEDATE,EFFDATE,EXPIREDATE,DISCDATE,"
                  "  USERAPPL,LIMITATIONAPPL,EXCEPTTKTGCXRIND,TKTGCARRIER,WHOLLYWITHINAPPL,"
                  "  WHOLLYWITHINLOCTYPE,WHOLLYWITHINLOC,ORIGINTVLAPPL,ORIGINLOCTYPE,"
                  "  ORIGINLOC,FARETYPE,INTLDEPARTMAXNO,INTLARRIVALMAXNO,MAXRETRANSITALLOWED,"
                  "  RETRANSITPOINTAPPL,RETRANSITLOCTYPE,RETRANSITLOC,MAXSTOPSATRETRANSIT,"
                  "  VIAPOINTSTOPOVERAPPL,STOPOVERLOCTYPE,STOPOVERLOC,POSLOCTYPE,POSLOC,"
                  "  POTLOCTYPE,POTLOC,SEPARATETKTIND");

    //		        this->From("=LIMITATIONS l LEFT OUTER JOIN =LIMITTICKETINGCXR tc"
    //		                   "                       USING (VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=LIMITATIONS", "l", "LEFT OUTER JOIN", "=LIMITTICKETINGCXR", "tc", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("l.LIMITATIONAPPL = 'J'"
                " and USERAPPL = %1q "
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPL,l.SEQNO, l.VERSIONDATE,l.CREATEDATE");
    else
      this->OrderBy("l.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static LimitationJrny* mapRowToLimitJrnyBase(Row* row, LimitationJrny* limJrnyPrev)
  { // Load up Parent Determinant Fields
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    LimitationJrny* lj;

    // If Parent hasn't changed, add to Children (tktgCarriers) of Prev
    if (limJrnyPrev != nullptr && limJrnyPrev->versionDate() == versionDate &&
        limJrnyPrev->seqNo() == seqNo && limJrnyPrev->createDate() == createDate)
    { // Just add to Prev
      lj = limJrnyPrev;
    }
    else
    { // Time for a new Parent
      lj = new tse::LimitationJrny;
      lj->versionDate() = versionDate;
      lj->setSeqNo(seqNo);
      lj->createDate() = createDate;
      lj->effDate() = row->getDate(EFFDATE);
      lj->expireDate() = row->getDate(EXPIREDATE);
      lj->discDate() = row->getDate(DISCDATE);
      lj->userAppl() = row->getString(USERAPPL);
      lj->limitationAppl() = row->getChar(LIMITATIONAPPL);
      lj->exceptTktgCxrInd() = row->getChar(EXCEPTTKTGCXRIND);
      lj->whollyWithinAppl() = row->getChar(WHOLLYWITHINAPPL);

      LocKey* loc = &lj->whollyWithinLoc();
      loc->locType() = row->getChar(WHOLLYWITHINLOCTYPE);
      loc->loc() = row->getString(WHOLLYWITHINLOC);

      lj->originTvlAppl() = row->getChar(ORIGINTVLAPPL);

      loc = &lj->originLoc();
      loc->locType() = row->getChar(ORIGINLOCTYPE);
      loc->loc() = row->getString(ORIGINLOC);

      lj->fareType() = row->getString(FARETYPE);
      lj->intlDepartMaxNo() = QUERYCLASS::charToInt(row->getString(INTLDEPARTMAXNO));
      lj->intlArrivalMaxNo() = QUERYCLASS::charToInt(row->getString(INTLARRIVALMAXNO));
      lj->maxRetransitAllowed() = QUERYCLASS::charToInt(row->getString(MAXRETRANSITALLOWED));
      lj->retransitPointAppl() = row->getChar(RETRANSITPOINTAPPL);

      loc = &lj->retransitLoc();
      loc->locType() = row->getChar(RETRANSITLOCTYPE);
      loc->loc() = row->getString(RETRANSITLOC);

      lj->maxStopsAtRetransit() = QUERYCLASS::charToInt(row->getString(MAXSTOPSATRETRANSIT));
      lj->viaPointStopoverAppl() = row->getChar(VIAPOINTSTOPOVERAPPL);

      loc = &lj->stopoverLoc();
      loc->locType() = row->getChar(STOPOVERLOCTYPE);
      loc->loc() = row->getString(STOPOVERLOC);

      loc = &lj->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);

      loc = &lj->potLoc();
      loc->locType() = row->getChar(POTLOCTYPE);
      loc->loc() = row->getString(POTLOC);

      lj->separateTktInd() = row->getChar(SEPARATETKTIND);
    } // New Parent

    // Add new TktgCarrier & return
    if (!row->isNull(TKTGCARRIER))
    {
      CarrierCode newTC = row->getString(TKTGCARRIER);
      lj->tktgCarriers().push_back(newTC);
    }
    return lj;
  } // mapRowToLimitJrnyBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetLimitJrnyBaseSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitJrnyBaseHistoricalSQLStatement
    : public QueryGetLimitJrnyBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("l.LIMITATIONAPPL = 'J'"
                " and USERAPPL = %1q");
  }
}; // class QueryGetLimitJrnyBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllLimitJrnyBaseSQLStatement : public QueryGetLimitJrnyBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("l.LIMITATIONAPPL = 'J'"
                "  and %cd <= EXPIREDATE");
    if (!DataManager::forceSortOrder())
      this->OrderBy("USERAPPL, l.SEQNO");
  };
}; // class QueryGetAllLimitJrnyBaseSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllLimitJrnyBaseHistoricalSQLStatement
    : public QueryGetLimitJrnyBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("l.LIMITATIONAPPL = 'J'");
    if (!DataManager::forceSortOrder())
      this->OrderBy("USERAPPL, l.SEQNO");
  };
}; // class QueryGetAllLimitJrnyBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetLimitJrnyTxtMsg
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitJrnyTxtMsgSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitJrnyTxtMsgSQLStatement() {};
  virtual ~QueryGetLimitJrnyTxtMsgSQLStatement() {};

  enum ColumnIndexes
  {
    TEXT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select fts.TEXT");
    this->From("=LIMITATIONS l,=LIMITTEXTMSG tm,=FREETEXTSEG fts");
    this->Where("l.VERSIONDATE = %1n"
                "    and l.SEQNO = %2n"
                "    and l.CREATEDATE = %3n"
                "    and l.VERSIONDATE = tm.VERSIONDATE"
                "    and l.SEQNO       = tm.SEQNO"
                "    and l.CREATEDATE = tm.CREATEDATE"
                "    and tm.MESSAGETYPE = fts.MESSAGETYPE"
                "    and tm.MSGITEMNO   = fts.ITEMNO");
    if (DataManager::forceSortOrder())
      this->OrderBy("l.VERSIONDATE , l.SEQNO, l.CREATEDATE, tm.MESSAGETYPE, tm.MSGITEMNO, "
                    "tm.USERAPPLTYPE, tm.USERAPPL");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToText(Row* row) { return row->getString(TEXT); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetLimitationPU
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitationPUSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitationPUSQLStatement() {};
  virtual ~QueryGetLimitationPUSQLStatement() {};

  enum ColumnIndexes
  {
    VERSIONDATE = 0,
    SEQNO,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    USERAPPL,
    LIMITATIONAPPL,
    EXCEPTTKTGCXRIND,
    TKTGCARRIER,
    WHOLLYWITHINAPPL,
    WHOLLYWITHINLOCTYPE,
    WHOLLYWITHINLOC,
    ORIGINTVLAPPL,
    ORIGINLOCTYPE,
    ORIGINLOC,
    FARETYPE,
    INTLDEPARTMAXNO,
    INTLARRIVALMAXNO,
    MAXRETRANSITALLOWED,
    RETRANSITPOINTAPPL,
    RETRANSITLOCTYPE,
    RETRANSITLOC,
    MAXSTOPSATRETRANSIT,
    VIAPOINTSTOPOVERAPPL,
    STOPOVERLOCTYPE,
    STOPOVERLOC,
    POSLOCTYPE,
    POSLOC,
    POTLOCTYPE,
    POTLOC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select l.VERSIONDATE,l.SEQNO,l.CREATEDATE,EFFDATE,EXPIREDATE,DISCDATE,"
                  "   USERAPPL,LIMITATIONAPPL,EXCEPTTKTGCXRIND,TKTGCARRIER,WHOLLYWITHINAPPL,"
                  "   WHOLLYWITHINLOCTYPE,WHOLLYWITHINLOC,ORIGINTVLAPPL,ORIGINLOCTYPE,"
                  "   ORIGINLOC,FARETYPE,INTLDEPARTMAXNO,INTLARRIVALMAXNO,MAXRETRANSITALLOWED,"
                  "   RETRANSITPOINTAPPL,RETRANSITLOCTYPE,RETRANSITLOC,MAXSTOPSATRETRANSIT,"
                  "   VIAPOINTSTOPOVERAPPL,STOPOVERLOCTYPE,STOPOVERLOC,POSLOCTYPE,POSLOC,"
                  "   POTLOCTYPE,POTLOC");

    //		        this->From("=LIMITATIONS l LEFT OUTER JOIN =LIMITTICKETINGCXR tc"
    //		                   "                       USING (VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=LIMITATIONS", "l", "LEFT OUTER JOIN", "=LIMITTICKETINGCXR", "tc", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("l.LIMITATIONAPPL = 'P'"
                " and USERAPPL = %1q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("l.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::LimitationCmn* mapRowToLimitationPU(Row* row, LimitationCmn* limPuPrev)
  { // Load up Parent Determinant Fields
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    LimitationCmn* lp;

    // If Parent hasn't changed, add to Children (tktgCarriers)
    if (limPuPrev != nullptr && limPuPrev->versionDate() == versionDate && limPuPrev->seqNo() == seqNo &&
        limPuPrev->createDate() == createDate)
    { // Just add to Prev
      lp = limPuPrev;
    } // Previous Parent
    else
    { // Time for a new Parent
      lp = new tse::LimitationCmn;
      lp->versionDate() = versionDate;
      lp->setSeqNo(seqNo);
      lp->createDate() = createDate;

      lp->effDate() = row->getDate(EFFDATE);
      lp->expireDate() = row->getDate(EXPIREDATE);
      lp->discDate() = row->getDate(DISCDATE);

      lp->userAppl() = row->getString(USERAPPL);
      lp->limitationAppl() = row->getChar(LIMITATIONAPPL);
      lp->exceptTktgCxrInd() = row->getChar(EXCEPTTKTGCXRIND);
      lp->whollyWithinAppl() = row->getChar(WHOLLYWITHINAPPL);

      LocKey* loc = &lp->whollyWithinLoc();
      loc->locType() = row->getChar(WHOLLYWITHINLOCTYPE);
      loc->loc() = row->getString(WHOLLYWITHINLOC);

      lp->originTvlAppl() = row->getChar(ORIGINTVLAPPL);

      loc = &lp->originLoc();
      loc->locType() = row->getChar(ORIGINLOCTYPE);
      loc->loc() = row->getString(ORIGINLOC);

      lp->fareType() = row->getString(FARETYPE);
      lp->intlDepartMaxNo() = QUERYCLASS::charToInt(row->getString(INTLDEPARTMAXNO));
      lp->intlArrivalMaxNo() = QUERYCLASS::charToInt(row->getString(INTLARRIVALMAXNO));
      lp->maxRetransitAllowed() = QUERYCLASS::charToInt(row->getString(MAXRETRANSITALLOWED));
      lp->retransitPointAppl() = row->getChar(RETRANSITPOINTAPPL);

      loc = &lp->retransitLoc();
      loc->locType() = row->getChar(RETRANSITLOCTYPE);
      loc->loc() = row->getString(RETRANSITLOC);

      lp->maxStopsAtRetransit() = QUERYCLASS::charToInt(row->getString(MAXSTOPSATRETRANSIT));
      lp->viaPointStopoverAppl() = row->getChar(VIAPOINTSTOPOVERAPPL);

      loc = &lp->stopoverLoc();
      loc->locType() = row->getChar(STOPOVERLOCTYPE);
      loc->loc() = row->getString(STOPOVERLOC);

      loc = &lp->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);

      loc = &lp->potLoc();
      loc->locType() = row->getChar(POTLOCTYPE);
      loc->loc() = row->getString(POTLOC);
    } // New Parent

    // Add new TktgCarrier & return
    if (!row->isNull(TKTGCARRIER))
    {
      CarrierCode newTC = row->getString(TKTGCARRIER);
      lp->tktgCarriers().push_back(newTC);
    }
    return lp;
  } // mapRowToLimitationPU()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetLimitationPUSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitationPUHistoricalSQLStatement
    : public QueryGetLimitationPUSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("l.LIMITATIONAPPL = 'P'"
                " and USERAPPL = %1q ");

    if (DataManager::forceSortOrder())
    {
      this->OrderBy("l.SEQNO,l.CREATEDATE,tc.TKTGCARRIER");
    }
    else
    {
      this->OrderBy("l.SEQNO");
    }
  };
}; // class QueryGetLimitationPUHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllLimitationPUSQLStatement : public QueryGetLimitationPUSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("l.LIMITATIONAPPL = 'P'"
                " and %cd <= EXPIREDATE");
    this->OrderBy("USERAPPL, l.SEQNO");
  };
}; // class QueryGetAllLimitationPUSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllLimitationPUHistoricalSQLStatement
    : public QueryGetLimitationPUSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("l.LIMITATIONAPPL = 'P'");
    this->OrderBy("USERAPPL, l.SEQNO");
  };
}; // class QueryGetAllLimitationPUHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetLimitFareBase
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitFareBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitFareBaseSQLStatement() {};
  virtual ~QueryGetLimitFareBaseSQLStatement() {};

  enum ColumnIndexes
  {
    VERSIONDATE = 0,
    SEQNO,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    USERAPPL,
    LIMITATIONAPPL,
    EXCEPTTKTGCXRIND,
    TKTGCARRIER,
    WHOLLYWITHINAPPL,
    WHOLLYWITHINLOCTYPE,
    WHOLLYWITHINLOC,
    ORIGINTVLAPPL,
    ORIGINLOCTYPE,
    ORIGINLOC,
    FARETYPE,
    INTLDEPARTMAXNO,
    INTLARRIVALMAXNO,
    MAXRETRANSITALLOWED,
    RETRANSITPOINTAPPL,
    RETRANSITLOCTYPE,
    RETRANSITLOC,
    MAXSTOPSATRETRANSIT,
    VIAPOINTSTOPOVERAPPL,
    STOPOVERLOCTYPE,
    STOPOVERLOC,
    POSLOCTYPE,
    POSLOC,
    POTLOCTYPE,
    POTLOC,
    GOVCARRIERAPPL,
    DIRECTIONALITY,
    FARECOMPONENTAPPL,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALDIR,
    MUSTNOTVIAHIP,
    CONFIRMEDAPPL,
    MAXDOMSEGMENTS,
    EXCEPTVIACXRLOCIND,
    NOTVIALOCTYPE,
    NOTVIALOC,
    VIAGOVCARRIERIND,
    NOTVIATOFROMLOCTYPE,
    NOTVIATOFROMLOC,
    RETRANSITGOVCXRAPPL,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select l.VERSIONDATE,l.SEQNO,l.CREATEDATE,EFFDATE,EXPIREDATE,DISCDATE,"
                  "  USERAPPL,LIMITATIONAPPL,EXCEPTTKTGCXRIND,TKTGCARRIER,WHOLLYWITHINAPPL,"
                  "  WHOLLYWITHINLOCTYPE,WHOLLYWITHINLOC,ORIGINTVLAPPL,ORIGINLOCTYPE,"
                  "  ORIGINLOC,FARETYPE,INTLDEPARTMAXNO,INTLARRIVALMAXNO,MAXRETRANSITALLOWED,"
                  "  RETRANSITPOINTAPPL,RETRANSITLOCTYPE,RETRANSITLOC,MAXSTOPSATRETRANSIT,"
                  "  VIAPOINTSTOPOVERAPPL,STOPOVERLOCTYPE,STOPOVERLOC,POSLOCTYPE,POSLOC,"
                  "  POTLOCTYPE,POTLOC,GOVCARRIERAPPL,DIRECTIONALITY,FARECOMPONENTAPPL,"
                  "  LOC1TYPE,LOC1,LOC2TYPE,LOC2,GLOBALDIR,MUSTNOTVIAHIP,CONFIRMEDAPPL,"
                  "  MAXDOMSEGMENTS,EXCEPTVIACXRLOCIND,NOTVIALOCTYPE,NOTVIALOC,"
                  "  VIAGOVCARRIERIND,NOTVIATOFROMLOCTYPE,NOTVIATOFROMLOC,RETRANSITGOVCXRAPPL");

    //		        this->From("=LIMITATIONS l LEFT OUTER JOIN =LIMITTICKETINGCXR tc"
    //		                   "                       USING (VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=LIMITATIONS", "l", "LEFT OUTER JOIN", "=LIMITTICKETINGCXR", "tc", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    // mazam: test it
    /*this->Where("l.LIMITATIONAPPL = 'F'"
                "  and USERAPPL = %1q "
                "  and EXPIREDATE > 0"
                "  and %cd <= EXPIREDATE"); */

    this->Where("l.LIMITATIONAPPL = 'F'"
                "  and USERAPPL = %1q "
                "  and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPL,l.SEQNO,l.VERSIONDATE,l.CREATEDATE");
    else
      this->OrderBy("l.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static LimitationFare* mapRowToLimitFareBase(Row* row, LimitationFare* limFarePrev)
  { // Load up Parent Determinant Fields
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    LimitationFare* lf;

    // If Parent hasn't changed, add to Children (tktgCarriers)
    if (limFarePrev != nullptr && limFarePrev->versionDate() == versionDate &&
        limFarePrev->seqNo() == seqNo && limFarePrev->createDate() == createDate)
    { // Just add to Prev
      lf = limFarePrev;
    }
    else
    { // Time for a new Parent
      lf = new tse::LimitationFare;
      lf->versionDate() = versionDate;
      lf->setSeqNo(seqNo);
      lf->createDate() = createDate;
      lf->effDate() = row->getDate(EFFDATE);
      lf->expireDate() = row->getDate(EXPIREDATE);
      lf->discDate() = row->getDate(DISCDATE);
      lf->userAppl() = row->getString(USERAPPL);
      lf->limitationAppl() = row->getChar(LIMITATIONAPPL);
      lf->exceptTktgCxrInd() = row->getChar(EXCEPTTKTGCXRIND);
      lf->whollyWithinAppl() = row->getChar(WHOLLYWITHINAPPL);

      LocKey* loc = &lf->whollyWithinLoc();
      loc->locType() = row->getChar(WHOLLYWITHINLOCTYPE);
      loc->loc() = row->getString(WHOLLYWITHINLOC);

      lf->originTvlAppl() = row->getChar(ORIGINTVLAPPL);

      loc = &lf->originLoc();
      loc->locType() = row->getChar(ORIGINLOCTYPE);
      loc->loc() = row->getString(ORIGINLOC);

      lf->fareType() = row->getString(FARETYPE);
      lf->intlDepartMaxNo() = QUERYCLASS::charToInt(row->getString(INTLDEPARTMAXNO));
      lf->intlArrivalMaxNo() = QUERYCLASS::charToInt(row->getString(INTLARRIVALMAXNO));
      lf->maxRetransitAllowed() = QUERYCLASS::charToInt(row->getString(MAXRETRANSITALLOWED));
      lf->retransitPointAppl() = row->getChar(RETRANSITPOINTAPPL);

      loc = &lf->retransitLoc();
      loc->locType() = row->getChar(RETRANSITLOCTYPE);
      loc->loc() = row->getString(RETRANSITLOC);

      lf->maxStopsAtRetransit() = QUERYCLASS::charToInt(row->getString(MAXSTOPSATRETRANSIT));
      lf->viaPointStopoverAppl() = row->getChar(VIAPOINTSTOPOVERAPPL);

      loc = &lf->stopoverLoc();
      loc->locType() = row->getChar(STOPOVERLOCTYPE);
      loc->loc() = row->getString(STOPOVERLOC);

      loc = &lf->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);

      loc = &lf->potLoc();
      loc->locType() = row->getChar(POTLOCTYPE);
      loc->loc() = row->getString(POTLOC);

      lf->govCarrierAppl() = row->getChar(GOVCARRIERAPPL);
      lf->directionality() = row->getChar(DIRECTIONALITY);
      lf->fareComponentAppl() = row->getChar(FARECOMPONENTAPPL);

      loc = &lf->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &lf->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      std::string gd = row->getString(GLOBALDIR);
      strToGlobalDirection(lf->globalDir(), gd);

      lf->mustNotViaHip() = row->getChar(MUSTNOTVIAHIP);
      lf->confirmedAppl() = row->getChar(CONFIRMEDAPPL);
      lf->maxDomSegments() = row->getChar(MAXDOMSEGMENTS);
      lf->exceptViaCxrLocInd() = row->getChar(EXCEPTVIACXRLOCIND);

      loc = &lf->notViaLoc();
      loc->locType() = row->getChar(NOTVIALOCTYPE);
      loc->loc() = row->getString(NOTVIALOC);

      lf->viaGovCarrierInd() = row->getChar(VIAGOVCARRIERIND);

      loc = &lf->notViaToFromLoc();
      loc->locType() = row->getChar(NOTVIATOFROMLOCTYPE);
      loc->loc() = row->getString(NOTVIATOFROMLOC);

      lf->retransitGovCxrAppl() = row->getChar(RETRANSITGOVCXRAPPL);
    } // New Parent

    // Load up new TktgCarrier & return
    if (!row->isNull(TKTGCARRIER))
    {
      CarrierCode newTC = row->getString(TKTGCARRIER);
      lf->tktgCarriers().push_back(newTC);
    }
    return lf;
  } // mapRowToLimitFareBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetLimitFareBaseSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitFareBaseHistoricalSQLStatement
    : public QueryGetLimitFareBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    // mazam: test it
    /*this->Where("l.LIMITATIONAPPL = 'F'"
                "  and USERAPPL = %1q "
                "  and EXPIREDATE > 0");*/
    this->Where("l.LIMITATIONAPPL = 'F'"
                "  and USERAPPL = %1q");
    this->OrderBy("USERAPPL, l.SEQNO, VERSIONDATE, CREATEDATE, TKTGCARRIER");
  };
}; // class QueryGetLimitFareBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllLimitFareBaseSQLStatement : public QueryGetLimitFareBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    // mazam: test it
    /*this->Where("l.LIMITATIONAPPL = 'F'"
                "  and EXPIREDATE > 0"
                "  and %cd <= EXPIREDATE");*/

    this->Where("l.LIMITATIONAPPL = 'F'"
                "  and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPL,l.SEQNO,l.VERSIONDATE,l.CREATEDATE");
    else
      this->OrderBy("USERAPPL, l.SEQNO");
  };
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllLimitFareBaseHistoricalSQLStatement
    : public QueryGetLimitFareBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    // mazam: test it
    /*this->Where("l.LIMITATIONAPPL = 'F'"
                "  and EXPIREDATE > 0");*/
    this->Where("l.LIMITATIONAPPL = 'F'");
    this->OrderBy("USERAPPL, l.SEQNO, VERSIONDATE, CREATEDATE, TKTGCARRIER");
  };
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetLimitFareGovCxrs
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitFareGovCxrsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitFareGovCxrsSQLStatement() {};
  virtual ~QueryGetLimitFareGovCxrsSQLStatement() {};

  enum ColumnIndexes
  {
    GOVCARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select GOVCARRIER");
    this->From("=LIMITGOVCARRIER");
    this->Where("VERSIONDATE = %1n"
                "  and SEQNO = %2n"
                "  and CREATEDATE = %3n");
    if (DataManager::forceSortOrder())
      this->OrderBy("GOVCARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const CarrierCode mapRowToGovCarrier(Row* row) { return row->getString(GOVCARRIER); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetLimitFareCxrLocs
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitFareCxrLocsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitFareCxrLocsSQLStatement() {};
  virtual ~QueryGetLimitFareCxrLocsSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,LOC1TYPE,LOC1,LOC2TYPE,LOC2");
    this->From("=LIMITVIACXRLOC");
    this->Where("VERSIONDATE = %1n"
                "   and SEQNO = %2n"
                "   and CREATEDATE = %3n");

    if (DataManager::forceSortOrder())
      this->OrderBy("VERSIONDATE, SEQNO, CREATEDATE, CARRIER, LOC1TYPE, LOC1, LOC2TYPE, LOC2");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static LimitFareCxrLoc* mapRowToLimitFareCxrLoc(Row* row)
  {
    LimitFareCxrLoc* newCxrLoc = new LimitFareCxrLoc;
    newCxrLoc->carrier() = row->getString(CARRIER);

    LocKey* loc = &newCxrLoc->loc1();
    loc->locType() = row->getChar(LOC1TYPE);
    loc->loc() = row->getString(LOC1);

    loc = &newCxrLoc->loc2();
    loc->locType() = row->getChar(LOC2TYPE);
    loc->loc() = row->getString(LOC2);

    return newCxrLoc;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetLimitFareRoutings
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetLimitFareRoutingsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLimitFareRoutingsSQLStatement() {};
  virtual ~QueryGetLimitFareRoutingsSQLStatement() {};

  enum ColumnIndexes
  {
    ROUTING = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ROUTING");
    this->From("=LIMITNOTVIAROUTING");
    this->Where("VERSIONDATE = %1n"
                "  and SEQNO = %2n"
                "  and CREATEDATE = %3n");

    if (DataManager::forceSortOrder())
      this->OrderBy("VERSIONDATE, SEQNO, CREATEDATE, ROUTING");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const RoutingNumber mapRowToRouting(Row* row) { return row->getString(ROUTING); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
