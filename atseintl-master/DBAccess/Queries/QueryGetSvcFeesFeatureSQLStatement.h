//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesFeatureSQLStatement.h
//          Description:    QueryGetSvcFeesFeatureSQLStatement
//          Created:        04/08/2013
//
//     (C) 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesFeature.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetSvcFeesFeatureSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    CARRIER,
    SERVICETYPECODE,
    SERVICESUBTYPECODE,
    FLTTKTMERCHIND,
    SEGMENTAPPL1,
    SEGMENTAPPL2,
    SEGMENTAPPL3,
    SEGMENTAPPL4,
    SEGMENTAPPL5,
    SEGMENTAPPL6,
    SEGMENTAPPL7,
    SEGMENTAPPL8,
    SEGMENTAPPL9,
    SEGMENTAPPL10,
    NUMBEROFCOLUMNS
  };
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, VALIDITYIND"
            ", CARRIER, SERVICETYPECODE, SERVICESUBTYPECODE, FLTTKTMERCHIND"
            ", SEGMENTAPPL1, SEGMENTAPPL2, SEGMENTAPPL3, SEGMENTAPPL4, SEGMENTAPPL5"
            ", SEGMENTAPPL6, SEGMENTAPPL7, SEGMENTAPPL8, SEGMENTAPPL9, SEGMENTAPPL10");
    From("=SVCFEESFEATURE");
    Where("VENDOR = %1q"
          " and ITEMNO = %2n"
          " and %cd <= EXPIREDATE"
          " and VALIDITYIND = 'Y'");
    OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SvcFeesFeatureInfo* mapRowToSvcFeesFeature(Row* row)
  {
    SvcFeesFeatureInfo* info(new SvcFeesFeatureInfo);
    info->vendor() = row->getString(VENDOR);
    info->itemNo() = row->getLongLong(ITEMNO);
    info->seqNo() = row->getLong(SEQNO);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(VALIDITYIND))
    {
      info->validityInd() = row->getChar(VALIDITYIND);
    }
    info->carrier() = row->getString(CARRIER);
    info->serviceTypeCode() = row->getString(SERVICETYPECODE);
    info->serviceSubTypeCode() = row->getString(SERVICESUBTYPECODE);
    if (!row->isNull(FLTTKTMERCHIND))
    {
      info->fltTktMerchInd() = row->getChar(FLTTKTMERCHIND);
    }
    if (!row->isNull(SEGMENTAPPL1))
    {
      info->segmentAppl1() = row->getChar(SEGMENTAPPL1);
    }
    if (!row->isNull(SEGMENTAPPL2))
    {
      info->segmentAppl2() = row->getChar(SEGMENTAPPL2);
    }
    if (!row->isNull(SEGMENTAPPL3))
    {
      info->segmentAppl3() = row->getChar(SEGMENTAPPL3);
    }
    if (!row->isNull(SEGMENTAPPL4))
    {
      info->segmentAppl4() = row->getChar(SEGMENTAPPL4);
    }
    if (!row->isNull(SEGMENTAPPL5))
    {
      info->segmentAppl5() = row->getChar(SEGMENTAPPL5);
    }
    if (!row->isNull(SEGMENTAPPL6))
    {
      info->segmentAppl6() = row->getChar(SEGMENTAPPL6);
    }
    if (!row->isNull(SEGMENTAPPL7))
    {
      info->segmentAppl7() = row->getChar(SEGMENTAPPL7);
    }
    if (!row->isNull(SEGMENTAPPL8))
    {
      info->segmentAppl8() = row->getChar(SEGMENTAPPL8);
    }
    if (!row->isNull(SEGMENTAPPL9))
    {
      info->segmentAppl9() = row->getChar(SEGMENTAPPL9);
    }
    if (!row->isNull(SEGMENTAPPL10))
    {
      info->segmentAppl10() = row->getChar(SEGMENTAPPL10);
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetSvcFeesFeatureHistoricalSQLStatement
    : public QueryGetSvcFeesFeatureSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");
    else
      this->OrderBy("");
  }
};

template <class QUERYCLASS>
class QueryGetAllSvcFeesFeatureSQLStatement : public QueryGetSvcFeesFeatureSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE"
                " and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");
    else
      this->OrderBy("");
  }
};
} // tse
