//----------------------------------------------------------------------------
//          File:           QueryGetSurfaceSectorExemptionInfoSQLStatement.h
//          Description:    QueryGetNoPNROptionsSQLStatement
//          Created:        1/13/2009
//          Author:        Marcin Augustyniak
//
//
//     � 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSurfaceSectorExemptionInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSurfaceSectorExemptionInfoSQLStatement() {};
  virtual ~QueryGetSurfaceSectorExemptionInfoSQLStatement() {};

  enum ColumnIndexes
  {
    // SURFACESECTOREXEMPTION
    VALIDATINGCARRIER = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    USERAPPLTYPE,
    USERAPPL,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    POSLOCEXCEPTION,
    POSLOCTYPE,
    POSLOC,
    LOCEXCEPTION,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    EXCEPTMARKETINGCARRIERS,
    EXCEPTOPERATINGCARRIERS,
    EXCEPTPASSENGERTYPES,
    MARKETINGCARRIER,
    OPERATINGCARRIER,
    PSGTYPE
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("SELECT c.VALIDATINGCARRIER, c.VERSIONDATE, c.SEQNO, c.CREATEDATE,"
                  "       c.USERAPPLTYPE, c.USERAPPL,"
                  "       c.EFFDATE,"
                  "       c.DISCDATE, c.EXPIREDATE, c.POSLOCEXCEPTION,"
                  "       c.POSLOCTYPE, c.POSLOC, c.LOCEXCEPTION,"
                  "       c.LOC1TYPE, c.LOC1, c.LOC2TYPE,"
                  "       c.LOC2, c.EXCEPTMARKETINGCARRIERS, c.EXCEPTOPERATINGCARRIERS,"
                  "       c.EXCEPTPASSENGERTYPES,"
                  "       cs.MARKETINGCARRIER,"
                  "       ct.OPERATINGCARRIER, cw.PSGTYPE");
    // this->From( "=SURFACESECTOREXEMPTION c"
    //            " LEFT OUTER JOIN =SURFACESECEXEMPTMKTGCXR cs"
    //            " USING (VALIDATINGCARRIER, VERSIONDATE, SEQNO, CREATEDATE)"
    //            " LEFT OUTER JOIN =SURFACESECEXEMPTOPERCXR ct"
    //            " USING (VALIDATINGCARRIER, VERSIONDATE, SEQNO, CREATEDATE)"
    //            " LEFT OUTER JOIN =SURFACESECEXEMPTPSGTYPE cw"
    //            " USING (VALIDATINGCARRIER, VERSIONDATE, SEQNO, CREATEDATE)");

    this->From("=SURFACESECTOREXEMPTION c"
               " LEFT OUTER JOIN =SURFACESECEXEMPTMKTGCXR cs"
               "   on c.VALIDATINGCARRIER = cs.VALIDATINGCARRIER "
               "   and c.VERSIONDATE = cs.VERSIONDATE "
               "   and c.SEQNO = cs.SEQNO "
               "   and c.CREATEDATE = cs.CREATEDATE "
               " LEFT OUTER JOIN =SURFACESECEXEMPTOPERCXR ct"
               "   on c.VALIDATINGCARRIER = ct.VALIDATINGCARRIER "
               "   and c.VERSIONDATE = ct.VERSIONDATE "
               "   and c.SEQNO = ct.SEQNO "
               "   and c.CREATEDATE = ct.CREATEDATE "
               " LEFT OUTER JOIN =SURFACESECEXEMPTPSGTYPE cw"
               "   on c.VALIDATINGCARRIER = cw.VALIDATINGCARRIER "
               "   and c.VERSIONDATE = cw.VERSIONDATE "
               "   and c.SEQNO = cw.SEQNO "
               "   and c.CREATEDATE = cw.CREATEDATE ");

    this->Where("c.VALIDATINGCARRIER = %1q AND %cd <= c.EXPIREDATE");
    this->OrderBy("c.VALIDATINGCARRIER, c.SEQNO, c.VERSIONDATE, c.CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SurfaceSectorExemptionInfo*
  mapRowToSurfaceSectorExemptionInfo(Row* row, SurfaceSectorExemptionInfo* info)
  {
    CarrierCode validatingCarrier = row->getString(VALIDATINGCARRIER);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    SurfaceSectorExemptionInfo* result;

    // If info object already exists, just add member of vector.
    if (info != nullptr && info->validatingCarrier() == validatingCarrier &&
        info->versionDate() == versionDate && info->seqNo() == seqNo &&
        info->createDate() == createDate)
    {
      result = info;
    }
    else
    { // info object does not exists so we have to create a new one.
      result = new SurfaceSectorExemptionInfo;

      result->validatingCarrier() = row->getString(VALIDATINGCARRIER);
      result->versionDate() = row->getDate(VERSIONDATE);
      result->seqNo() = row->getLong(SEQNO);
      result->createDate() = row->getDate(CREATEDATE);
      result->userApplType() = row->getChar(USERAPPLTYPE);
      result->userAppl() = row->getString(USERAPPL);
      result->effDate() = row->getDate(EFFDATE);
      result->discDate() = row->getDate(DISCDATE);
      result->expireDate() = row->getDate(EXPIREDATE);
      result->posLocException() = row->getChar(POSLOCEXCEPTION);
      result->posLocType() = row->getChar(POSLOCTYPE);
      result->posLoc() = row->getString(POSLOC);
      result->locException() = row->getChar(LOCEXCEPTION);
      result->loc1Type() = row->getChar(LOC1TYPE);
      result->loc1() = row->getString(LOC1);
      result->loc2Type() = row->getChar(LOC2TYPE);
      result->loc2() = row->getString(LOC2);
      result->exceptMarketingCarriers() = row->getChar(EXCEPTMARKETINGCARRIERS);
      result->exceptOperatingCarriers() = row->getChar(EXCEPTOPERATINGCARRIERS);
      result->exceptPassengersTypes() = row->getChar(EXCEPTPASSENGERTYPES);

      if (result->userAppl() == AXESS_USER)
        result->crs() = AXESS_MULTIHOST_ID;
      else if (result->userAppl() == ABACUS_USER)
        result->crs() = ABACUS_MULTIHOST_ID;
      else if (result->userAppl() == INFINI_USER)
        result->crs() = INFINI_MULTIHOST_ID;
      else if (result->userAppl() == SABRE_USER)
        result->crs() = SABRE_MULTIHOST_ID;
      else
        result->crs() = "";
    }

    // Load up vector members
    if (!row->isNull(MARKETINGCARRIER))
    {
      result->marketingCarriers().insert(row->getString(MARKETINGCARRIER));
    }

    if (!row->isNull(OPERATINGCARRIER))
    {
      result->operatingCarriers().insert(row->getString(OPERATINGCARRIER));
    }

    if (!row->isNull(PSGTYPE))
    {
      result->paxTypes().insert(row->getString(PSGTYPE));
    }

    return result;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetSurfaceSectorExemptionInfoHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSurfaceSectorExemptionInfoHistoricalSQLStatement
    : public QueryGetSurfaceSectorExemptionInfoSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("c.VALIDATINGCARRIER = %1q"); }
};
} // tse
