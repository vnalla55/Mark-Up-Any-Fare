//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/SQLStatement.h"
#include "DBAccess/SubCodeInfo.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSubCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSubCodeSQLStatement() {}
  virtual ~QueryGetSubCodeSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    SERVICETYPECODE,
    SERVICESUBTYPECODE,
    CREATEDATE,
    EXPIREDATE,
    FLTTKTMERCHIND,
    EFFDATE,
    DISCDATE,
    INDUSTRYCARRIERIND,
    SVCGROUP,
    SVCSUBGROUP,
    DESCRIPTION1,
    DESCRIPTION2,
    CONCUR,
    RFICODE,
    SSRCODE,
    DISPLAYCAT,
    SSIMCODE,
    EMDTYPE,
    COMMERCIALNAME,
    TAXTEXTTBLITEMNO,
    PICTURENO,
    BOOKINGIND,
    CONSUMPTIONIND
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, CARRIER, SERVICETYPECODE, SERVICESUBTYPECODE, CREATEDATE, "
                  "       EXPIREDATE, FLTTKTMERCHIND, EFFDATE, DISCDATE, INDUSTRYCARRIERIND, "
                  "       SVCGROUP, SVCSUBGROUP, DESCRIPTION1, DESCRIPTION2, CONCUR, RFICODE, "
                  "       SSRCODE, DISPLAYCAT, SSIMCODE, EMDTYPE, COMMERCIALNAME, "
                  "       TAXTEXTTBLITEMNO, PICTURENO, BOOKINGIND, CONSUMPTIONIND");

    this->From("=SUBCODE");

    this->Where("VENDOR = %1q  "
                " and CARRIER= %2q "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    this->OrderBy("SERVICESUBTYPECODE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SubCodeInfo* mapRowToSubCodeInfo(Row* row)
  {
    SubCodeInfo* subCode = new SubCodeInfo;

    subCode->vendor() = row->getString(VENDOR);
    subCode->carrier() = row->getString(CARRIER);
    subCode->serviceTypeCode() = row->getString(SERVICETYPECODE);
    subCode->serviceSubTypeCode() = row->getString(SERVICESUBTYPECODE);
    subCode->createDate() = row->getDate(CREATEDATE);

    if (!row->isNull(EXPIREDATE))
      subCode->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(FLTTKTMERCHIND))
      subCode->fltTktMerchInd() = row->getChar(FLTTKTMERCHIND);
    if (!row->isNull(EFFDATE))
      subCode->effDate() = row->getDate(EFFDATE);
    if (!row->isNull(DISCDATE))
      subCode->discDate() = row->getDate(DISCDATE);
    if (!row->isNull(INDUSTRYCARRIERIND))
      subCode->industryCarrierInd() = row->getChar(INDUSTRYCARRIERIND);
    if (!row->isNull(SVCGROUP))
      subCode->serviceGroup() = row->getString(SVCGROUP);
    if (!row->isNull(SVCSUBGROUP))
      subCode->serviceSubGroup() = row->getString(SVCSUBGROUP);
    if (!row->isNull(DESCRIPTION1))
      subCode->description1() = row->getString(DESCRIPTION1);
    if (!row->isNull(DESCRIPTION2))
      subCode->description2() = row->getString(DESCRIPTION2);
    if (!row->isNull(CONCUR))
      subCode->concur() = row->getChar(CONCUR);
    if (!row->isNull(RFICODE))
      subCode->rfiCode() = row->getChar(RFICODE);
    if (!row->isNull(SSRCODE))
      subCode->ssrCode() = row->getString(SSRCODE);
    if (!row->isNull(DISPLAYCAT))
      subCode->displayCat() = row->getString(DISPLAYCAT);
    if (!row->isNull(SSIMCODE))
      subCode->ssimCode() = row->getChar(SSIMCODE);
    if (!row->isNull(EMDTYPE))
      subCode->emdType() = row->getChar(EMDTYPE);
    if (!row->isNull(COMMERCIALNAME))
      subCode->commercialName() = row->getString(COMMERCIALNAME);
    if (!row->isNull(TAXTEXTTBLITEMNO))
      subCode->taxTextTblItemNo() = row->getInt(TAXTEXTTBLITEMNO);
    if (!row->isNull(PICTURENO))
      subCode->pictureNo() = row->getInt(PICTURENO);
    if (!row->isNull(BOOKINGIND))
      subCode->bookingInd() = row->getString(BOOKINGIND);
    if (!row->isNull(CONSUMPTIONIND))
      subCode->consumptionInd() = row->getChar(CONSUMPTIONIND);

    return subCode;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSubCodeHistoricalSQLStatement : public QueryGetSubCodeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and CARRIER= %2q "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("SERVICESUBTYPECODE, CREATEDATE");
  }
};
} // tse
