//----------------------------------------------------------------------------
//     � 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/Queries/QueryGetSectorDetail.h"
#include "DBAccess/Row.h"
#include "DBAccess/SectorDetailInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetSectorDetailSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSectorDetailSQLStatement() {};

  virtual ~QueryGetSectorDetailSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    APPLTAG,
    FAREBASISTKTDESIGNATOR,
    EQUIPMENTCODE,
    CABINCODE,
    RBD1,
    RBD2,
    RBD3,
    FARETYPE,
    FARETARIFFIND,
    FARETARIFF,
    RULE,
    FAREOWNINGCXR,
    TKTCODE,
    VERSIONNBR
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,LOCKDATE,EXPIREDATE,LASTMODDATE,"
                  "APPLTAG,FAREBASISTKTDESIGNATOR,EQUIPMENTCODE,CABINCODE,RBD1,RBD2,RBD3,"
                  "FARETYPE,FARETARIFFIND,FARETARIFF,RULE,FAREOWNINGCXR,TKTCODE,VERSIONNBR");

    this->From("=SECTORDETAIL");

    adjustBaseSQL();
    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const tse::SectorDetailInfo* mapRowToSectorDetailInfo(const Row* row)
  {
    tse::SectorDetailInfo* serviceInfo = new tse::SectorDetailInfo;

    serviceInfo->vendor() = row->getString(VENDOR);
    serviceInfo->itemNo() = row->getInt(ITEMNO);
    serviceInfo->seqNo() = row->getLong(SEQNO);
    serviceInfo->createDate() = row->getDate(CREATEDATE);
    serviceInfo->lockDate() = row->getDate(LOCKDATE);
    serviceInfo->expireDate() = row->getDate(EXPIREDATE);
    serviceInfo->lastModificationDate() = row->getDate(LASTMODDATE);
    serviceInfo->applyTag() = row->getChar(APPLTAG);
    serviceInfo->fareBasisTktDesignator() = row->getString(FAREBASISTKTDESIGNATOR);
    serviceInfo->equipmentCode() = row->getString(EQUIPMENTCODE);
    serviceInfo->cabinCode() = row->getChar(CABINCODE);
    serviceInfo->rbd1() = row->getChar(RBD1);
    serviceInfo->rbd2() = row->getChar(RBD2);
    serviceInfo->rbd3() = row->getChar(RBD3);
    serviceInfo->fareType() = row->getString(FARETYPE);
    serviceInfo->fareTariffInd() = row->getString(FARETARIFFIND);
    serviceInfo->fareTariff() = row->getInt(FARETARIFF);
    serviceInfo->rule() = row->getString(RULE);
    serviceInfo->fareOwningCxr() = row->getString(FAREOWNINGCXR);
    serviceInfo->tktCode() = row->getString(TKTCODE);
    serviceInfo->versionNbr() = row->getInt(VERSIONNBR);
    return serviceInfo;
  }

private:
  virtual void adjustBaseSQL()
  {
    this->Where("VENDOR = %1q "
                "and ITEMNO = %2n "
                "and %cd <= EXPIREDATE "
                "and VALIDITYIND = 'Y'");
  }
}; // class QueryGetSectorDetailSQLStatement

template <class QUERYCLASS>
class QueryGetSectorDetailHistoricalSQLStatement
    : public QueryGetSectorDetailSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetSectorDetailHistoricalSQLStatement

} // tse

