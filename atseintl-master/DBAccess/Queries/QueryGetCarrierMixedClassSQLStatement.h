//----------------------------------------------------------------------------
//          File:           QueryGetCarrierMixedClassSQLStatement.h
//          Description:    QueryGetCarrierMixedClassSQLStatement
//          Created:        10/29/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierMixedClass.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCarrierMixedClassSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierMixedClassSQLStatement() {};
  virtual ~QueryGetCarrierMixedClassSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    HIERARCHY,
    BKGCD,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(" select cx.CARRIER,cx.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  "        HIERARCHY,BKGCD");
    this->From("=CARRIERMIXEDCLASS cx, =CARRIERMIXEDCLASSSEG cxs");
    this->Where("cx.CARRIER = %1q "
                "    and cx.carrier = cxs.carrier"
                "    and cx.createdate = cxs.createdate"
                "    and %cd <= EXPIREDATE ");
    this->OrderBy("carrier, createdate, hierarchy, bkgcd");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CarrierMixedClass*
  mapRowToCarrierMixedClass(Row* row, CarrierMixedClass* carrierMixedClassPrev, DateTime& prevDate)
  {
    CarrierCode carrier = row->getString(CARRIER);
    DateTime createDate = row->getDate(CREATEDATE);

    // Parent not changed, just have a new child row
    if (carrierMixedClassPrev != nullptr && carrierMixedClassPrev->carrier() == carrier &&
        createDate == prevDate)
    {
      CarrierMixedClassSeg* seg = mapRowToCarrierMixedClassSeg(row);
      carrierMixedClassPrev->segs().push_back(seg);
      return carrierMixedClassPrev;
    }
    else
    {
      tse::CarrierMixedClass* carrierMixedClass = new tse::CarrierMixedClass;
      carrierMixedClass->carrier() = carrier;
      prevDate = createDate;
      carrierMixedClass->expireDate() = row->getDate(EXPIREDATE);
      carrierMixedClass->effDate() = row->getDate(EFFDATE);
      carrierMixedClass->discDate() = row->getDate(DISCDATE);
      CarrierMixedClassSeg* seg = mapRowToCarrierMixedClassSeg(row);
      carrierMixedClass->segs().push_back(seg);
      return carrierMixedClass;
    }
  } // mapRowToCarrierMixedClass()

  static tse::CarrierMixedClassSeg* mapRowToCarrierMixedClassSeg(Row* row)
  {
    tse::CarrierMixedClassSeg* carrierMixedClassSeg = new tse::CarrierMixedClassSeg;

    carrierMixedClassSeg->hierarchy() = row->getChar(HIERARCHY);
    carrierMixedClassSeg->bkgcd() = row->getString(BKGCD);

    return carrierMixedClassSeg;
  } // mapRowToCarrierMixedClassSeg()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCarrierMixedClassHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCarrierMixedClassHistoricalSQLStatement
    : public QueryGetCarrierMixedClassSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("cx.CARRIER = %1q "
                "    and cx.carrier = cxs.carrier"
                "    and cx.createdate = cxs.createdate");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCarrierMixedClass
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCarrierMixedClassSQLStatement
    : public QueryGetCarrierMixedClassSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("cx.carrier = cxs.carrier"
                "    and cx.createdate = cxs.createdate"
                "    and %cd <= EXPIREDATE ");
    this->OrderBy("carrier, createdate, hierarchy, bkgcd");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCarrierMixedClassHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCarrierMixedClassHistoricalSQLStatement
    : public QueryGetCarrierMixedClassSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("cx.carrier = cxs.carrier"
                "    and cx.createdate = cxs.createdate");
    this->OrderBy("carrier, createdate, hierarchy, bkgcd");
  };
};

} // tse
