//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesFareIdSQLStatement.h
//          Description:    QueryGetSvcFeesFareIdSQLStatement
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
#include "DBAccess/Queries/QueryGetSvcFeesFareId.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetSvcFeesFareIdSQLStatement : public DBAccess::SQLStatement
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
    FAREAPPLIND,
    OWRT,
    RULETARIFF,
    RULETARIFFIND,
    RULE,
    FARECLASS,
    FARETYPE,
    PSGTYPE,
    ROUTING,
    BOOKINGCODE1,
    BOOKINGCODE2,
    SOURCE,
    MINFAREAMT1,
    MAXFAREAMT1,
    CUR1,
    NODEC1,
    MINFAREAMT2,
    MAXFAREAMT2,
    CUR2,
    NODEC2,
    NUMBEROFCOLUMNS
  };
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, VALIDITYIND,"
            "FAREAPPLIND, OWRT, RULETARIFF, RULETARIFFIND, RULE, FARECLASS,"
            "FARETYPE, PSGTYPE, ROUTING, BOOKINGCODE1, BOOKINGCODE2, SOURCE,"
            "MINFAREAMT1, MAXFAREAMT1, CUR1, NODEC1,"
            "MINFAREAMT2, MAXFAREAMT2, CUR2, NODEC2");
    From("=SVCFEESFAREID");
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

  static SvcFeesFareIdInfo* mapRowToSvcFeesFareId(Row* row)
  {
    SvcFeesFareIdInfo* info(new SvcFeesFareIdInfo);
    info->vendor() = row->getString(VENDOR);
    info->itemNo() = row->getLongLong(ITEMNO);
    info->seqNo() = row->getLong(SEQNO);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(VALIDITYIND))
    {
      info->validityInd() = row->getChar(VALIDITYIND);
    }
    if (!row->isNull(FAREAPPLIND))
    {
      info->fareApplInd() = row->getChar(FAREAPPLIND);
    }
    if (!row->isNull(OWRT))
    {
      info->owrt() = row->getChar(OWRT);
    }
    if (!row->isNull(RULETARIFF))
    {
      info->ruleTariff() = row->getInt(RULETARIFF);
    }
    info->ruleTariffInd() = row->getString(RULETARIFFIND);
    info->rule() = row->getString(RULE);
    info->fareClass() = row->getString(FARECLASS);
    info->fareType() = row->getString(FARETYPE);
    info->paxType() = row->getString(PSGTYPE);
    if (!row->isNull(ROUTING))
    {
      info->routing() = row->getInt(ROUTING);
    }
    info->bookingCode1() = row->getString(BOOKINGCODE1);
    info->bookingCode2() = row->getString(BOOKINGCODE2);
    if (!row->isNull(SOURCE))
    {
      info->source() = row->getChar(SOURCE);
    }
    if (!row->isNull(NODEC1))
    {
      info->noDec1() = row->getInt(NODEC1);
    }
    if (!row->isNull(MINFAREAMT1))
    {
      info->minFareAmt1() = QUERYCLASS::adjustDecimal(row->getInt(MINFAREAMT1), info->noDec1());
    }
    if (!row->isNull(MAXFAREAMT1))
    {
      info->maxFareAmt1() = QUERYCLASS::adjustDecimal(row->getInt(MAXFAREAMT1), info->noDec1());
    }
    info->cur1() = row->getString(CUR1);
    if (!row->isNull(NODEC2))
    {
      info->noDec2() = row->getInt(NODEC2);
    }
    if (!row->isNull(MINFAREAMT2))
    {
      info->minFareAmt2() = QUERYCLASS::adjustDecimal(row->getInt(MINFAREAMT2), info->noDec2());
    }
    if (!row->isNull(MAXFAREAMT2))
    {
      info->maxFareAmt2() = QUERYCLASS::adjustDecimal(row->getInt(MAXFAREAMT2), info->noDec2());
    }
    info->cur2() = row->getString(CUR2);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetSvcFeesFareIdHistoricalSQLStatement
    : public QueryGetSvcFeesFareIdSQLStatement<QUERYCLASS>
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
class QueryGetAllSvcFeesFareIdSQLStatement : public QueryGetSvcFeesFareIdSQLStatement<QUERYCLASS>
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
