//----------------------------------------------------------------------------
//          File:           QueryGetMarriedCabinSQLStatement.h
//          Description:    QueryGetMarriedCabinSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetMarriedCabin.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMarriedCabinSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMarriedCabinSQLStatement() {};
  virtual ~QueryGetMarriedCabinSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    PREMIUMCABIN,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    LOCKDATE,
    NEWSEQNO,
    EFFDATE,
    DISCDATE,
    MEMONO,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    CREATORID,
    CREATORBUSINESSUNIT,
    DIRECTIONALITY,
    VERSIONDISPLAYIND,
    VERSIONINHERITEDIND,
    MARRIEDCABIN,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select m.CARRIER,m.PREMIUMCABIN,m.VERSIONDATE,m.SEQNO,m.CREATEDATE,"
                  "       m.EXPIREDATE,m.LOCKDATE,m.NEWSEQNO,m.EFFDATE,m.DISCDATE,"
                  "       m.MEMONO,m.LOC1TYPE,m.LOC1,m.LOC2TYPE,m.LOC2,m.CREATORID,"
                  "       m.CREATORBUSINESSUNIT,m.DIRECTIONALITY,m.VERSIONDISPLAYIND,"
                  "       m.VERSIONINHERITEDIND, ms.MARRIEDCABIN");
    this->From("=MARRIEDCABIN m left outer join =MARRIEDCABINSEG ms"
               "                          on m.CARRIER = ms.CARRIER"
               "                         and m.PREMIUMCABIN = ms.PREMIUMCABIN"
               "                         and m.VERSIONDATE = ms.VERSIONDATE"
               "                         and m.SEQNO = ms.SEQNO"
               "                         and m.CREATEDATE = ms.CREATEDATE");
    this->Where("%cd <= m.EXPIREDATE"
                "    and m.CARRIER = %1q"
                "    and m.PREMIUMCABIN = %2q");
    this->OrderBy("m.CARRIER, m.PREMIUMCABIN, m.SEQNO, ms.MARRIEDCABIN");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MarriedCabin* mapRowToMarriedCabin(Row* row, tse::MarriedCabin* prev)
  {
    CarrierCode carrier = row->getString(CARRIER);
    BookingCode premiumCabin = row->getString(PREMIUMCABIN);
    BookingCode marriedCabin = row->getString(MARRIEDCABIN);

    tse::MarriedCabin* mc = nullptr;

    if (prev != nullptr && prev->carrier() == carrier && prev->premiumCabin() == premiumCabin &&
        prev->marriedCabin() == marriedCabin)
    {
      mc = prev;
    }
    else
    {
      mc = new MarriedCabin;
      mc->carrier() = carrier;
      mc->premiumCabin() = premiumCabin;
      mc->marriedCabin() = marriedCabin;

      mc->versionDate() = row->getDate(VERSIONDATE);
      mc->lockDate() = row->getDate(LOCKDATE);
      mc->createDate() = row->getDate(CREATEDATE);
      mc->expireDate() = row->getDate(EXPIREDATE);
      mc->effDate() = row->getDate(EFFDATE);
      mc->discDate() = row->getDate(DISCDATE);
      mc->creatorId() = row->getString(CREATORID);
      mc->seqNo() = row->getLong(SEQNO);
      mc->newSeqNo() = row->getLong(NEWSEQNO);
      mc->memoNo() = row->getInt(MEMONO);
      mc->loc1().loc() = row->getString(LOC1);
      mc->loc1().locType() = row->getChar(LOC1TYPE);
      mc->loc2().loc() = row->getString(LOC2);
      mc->loc2().locType() = row->getChar(LOC2TYPE);
      mc->directionality() = row->getChar(DIRECTIONALITY);
      mc->versionDisplayInd() = row->getChar(VERSIONDISPLAYIND);
      mc->versionInheritedInd() = row->getChar(VERSIONINHERITEDIND);
    }

    return mc;
  } // mapRowToMarriedCabin()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetAllMarriedCabinHistoricalSQLStatement
    : public QueryGetMarriedCabinSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("1 = 1"); }
};

template <class QUERYCLASS>
class QueryGetMarriedCabinHistoricalSQLStatement
    : public QueryGetMarriedCabinSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("    m.CARRIER = %1q"
                "    and m.PREMIUMCABIN = %2q");
  }
};

} // tse
