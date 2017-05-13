#pragma once

#include "Common/FallbackUtil.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionContract.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackAMC2SrcTgtPCCChange);

template <typename QUERYCLASS>
class QueryGetCommissionContractSQLStatement : public DBAccess::SQLStatement
{
public:
  enum CI2
  {
    VENDOR2=0
    , CARRIER2
    , PSEUDOCITY2
    , CONTRACTID2
    , EFFDATETIME2
    , EXPIREDATE2
    , CONTRACTCREATEDATE2
    , SECURITYCREATEDATE2
    , CONTRACTDESCRIPTION2
    , CONTRACTTYPE2
    , INHIBIT2
    , VALIDITYIND2
    , SECURITYEFFDATETIME2
    , SECURITYEXPIREDATE2
    , CONTRACTEFFDATETIME2
    , CONTRACTEXPIREDATE2
    , NUMBEROFCOLUMNS2
  };

  enum CI
  {
    VENDOR
    , CARRIER
    , SOURCEPCC
    , TARGETPCC
    , CONTRACTID
    , EFFDATETIME
    , EXPIREDATE
    , CONTRACTCREATEDATE
    , SECURITYCREATEDATE
    , CONTRACTDESCRIPTION
    , CONTRACTTYPE
    , INHIBIT
    , VALIDITYIND
    , SECURITYEFFDATETIME
    , SECURITYEXPIREDATE
    , CONTRACTEFFDATETIME
    , CONTRACTEXPIREDATE
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    if (!fallback::fixed::fallbackAMC2SrcTgtPCCChange())
    {
      Command("select VENDOR, CARRIER, SOURCEPCC, TARGETPCC, CONTRACTID, EFFDATETIME, EXPIREDATE,"
          " CONTRACTCREATEDATE, SECURITYCREATEDATE, CONTRACTDESCRIPTION, CONTRACTTYPE,"
          " INHIBIT, VALIDITYIND, SECURITYEFFDATETIME, SECURITYEXPIREDATE,"
          " CONTRACTEFFDATETIME, CONTRACTEXPIREDATE"
          " from V_COMMISSIONCONTRACTMAPPING_V2");

      this->Where("VENDOR = %1q and CARRIER = %2q and TARGETPCC = %3q"
          " and INHIBIT = 'N' and VALIDITYIND = 'Y'"
          " and EXPIREDATE >= %cd"
          " and EFFDATETIME <= %4n");
    }
    else
    {
      Command("select VENDOR, CARRIER, PSEUDOCITY, CONTRACTID, EFFDATETIME, EXPIREDATE,"
          " CONTRACTCREATEDATE, SECURITYCREATEDATE, CONTRACTDESCRIPTION, CONTRACTTYPE,"
          " INHIBIT, VALIDITYIND, SECURITYEFFDATETIME, SECURITYEXPIREDATE,"
          " CONTRACTEFFDATETIME, CONTRACTEXPIREDATE"
          " from V_COMMISSIONCONTRACTMAPPING");

      this->Where("VENDOR = %1q and CARRIER = %2q and PSEUDOCITY = %3q"
          " and INHIBIT = 'N' and VALIDITYIND = 'Y'"
          " and EXPIREDATE >= %cd"
          " and EFFDATETIME <= %4n");
    }

    this->OrderBy("VENDOR, CONTRACTID");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CommissionContractInfo* mapRow(Row* row)
  {
    CommissionContractInfo* info(new CommissionContractInfo);
    if (!fallback::fixed::fallbackAMC2SrcTgtPCCChange() && info)
    {
      if (!row->isNull(CI::VENDOR))
        info->vendor() = row->getString(CI::VENDOR);
      if (!row->isNull(CI::CARRIER))
        info->carrier() = row->getString(CI::CARRIER);

      if (!row->isNull(CI::SOURCEPCC))
        info->sourcePCC() = row->getString(CI::SOURCEPCC);
      if (!row->isNull(CI::TARGETPCC))
        info->pcc() = row->getString(CI::TARGETPCC);

      info->contractId() = row->getLong(CI::CONTRACTID);
      if (!row->isNull(CI::EFFDATETIME))
        info->effDateTime() = row->getDate(CI::EFFDATETIME);
      if (!row->isNull(CI::EXPIREDATE))
        info->expireDate() = row->getDate(CI::EXPIREDATE);
      if (!row->isNull(CI::CONTRACTDESCRIPTION))
        info->description() = row->getString(CI::CONTRACTDESCRIPTION);
      if (!row->isNull(CI::CONTRACTTYPE))
        info->contractType() = row->getString(CI::CONTRACTTYPE);
      if (!row->isNull(CI::INHIBIT))
        info->inhibit() = row->getChar(CI::INHIBIT);
      if (!row->isNull(CI::VALIDITYIND))
        info->validityInd() = row->getChar(CI::VALIDITYIND);
    }
    else
    {
      if (!row->isNull(CI2::VENDOR2))
        info->vendor() = row->getString(CI2::VENDOR2);
      if (!row->isNull(CI2::CARRIER2))
        info->carrier() = row->getString(CI2::CARRIER2);
      if (!row->isNull(CI2::PSEUDOCITY2))
        info->pcc() = row->getString(CI2::PSEUDOCITY2);
      info->contractId() = row->getLong(CI2::CONTRACTID2);
      if (!row->isNull(CI2::EFFDATETIME2))
        info->effDateTime() = row->getDate(CI2::EFFDATETIME2);
      if (!row->isNull(CI2::EXPIREDATE2))
        info->expireDate() = row->getDate(CI2::EXPIREDATE2);
      if (!row->isNull(CI2::CONTRACTDESCRIPTION2))
        info->description() = row->getString(CI2::CONTRACTDESCRIPTION2);
      if (!row->isNull(CI2::CONTRACTTYPE2))
        info->contractType() = row->getString(CI2::CONTRACTTYPE2);
      if (!row->isNull(CI2::INHIBIT2))
        info->inhibit() = row->getChar(CI2::INHIBIT2);
      if (!row->isNull(CI2::VALIDITYIND2))
        info->validityInd() = row->getChar(CI2::VALIDITYIND2);
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetCommissionContractHistoricalSQLStatement
    : public QueryGetCommissionContractSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    if (!fallback::fixed::fallbackAMC2SrcTgtPCCChange())
    {
      this->Where(" VENDOR = %1q and CARRIER = %2q and TARGETPCC = %3q"
          " and INHIBIT = 'N' and VALIDITYIND = 'Y'"
          " and %4n <= EXPIREDATE"
          " and %5n >= EFFDATETIME");
    }
    else
    {
      this->Where(" VENDOR = %1q and CARRIER = %2q and PSEUDOCITY = %3q"
          " and INHIBIT = 'N' and VALIDITYIND = 'Y'"
          " and %4n <= EXPIREDATE"
          " and %5n >= EFFDATETIME");
    }

    this->OrderBy("VENDOR, CONTRACTID");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllCommissionContractSQLStatement
    : public QueryGetCommissionContractSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and %cd >= CREATEDATETIME"
                " and %cd <= EXPIREDATETIME");

    this->OrderBy("VENDOR, CONTRACTID");
  }
};

} // tse
