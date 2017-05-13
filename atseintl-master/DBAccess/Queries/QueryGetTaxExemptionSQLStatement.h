// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxExemption.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
template <class QUERYCLASS>
class QueryGetTaxExemptionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxExemptionSQLStatement() = default;
  virtual ~QueryGetTaxExemptionSQLStatement() = default;

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VENDOR,
    CHANNELTYPE,
    CHANNELID,
    CREATEDATE,

    TAXEXEMPTIONSEGID,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    IATASTATIONNUMBER,

    LASTMODDATE,
    LOCKDATE,
    VALIDITYIND,
    VALIDATINGCARRIER,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select tr.TAXCODE,tr.VENDOR,tr.CHANNELTYPE,tr.CHANNELID,tr.CREATEDATE,"
                  "tr.TAXEXEMPTIONSEGID,tr.EFFDATE,tr.DISCDATE,tr.EXPIREDATE,tr.IATASTATIONNUMBER,"
                  "tr.LASTMODDATE,tr.LOCKDATE,tr.VALIDITYIND,trs.VALIDATINGCARRIER");

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(1);
    joinFields.push_back("TAXEXEMPTIONSEGID");

    this->generateJoinString(
        "=TAXEXEMPTION", "tr", "left outer join", "=TAXEXEMPTIONSEGMENT", "trs", joinFields, from);
    this->From(from);

    this->Where("tr.TAXCODE = %1q"
                " and tr.CHANNELID = %2q"
                " and %cd <= tr.EXPIREDATE"
                " and tr.VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }


  static tse::TaxExemption* mapRowToTaxExemption(Row* row, TaxExemption* prev)
  { // Get the Key Info
    TaxCode taxCode = row->getString(TAXCODE);
    DateTime createDate = row->getDate(CREATEDATE);
    PseudoCityCode channelId = row->getString(CHANNELID);
    ChannelType channelType = row->getString(CHANNELTYPE);
    VendorCode vendor = row->getString(VENDOR);
    long long int exemptionSegId = row->getLongLong(TAXEXEMPTIONSEGID);

    TaxExemption* tr = nullptr;

    if (prev != nullptr && prev->exemptionSegId() == exemptionSegId &&
        prev->taxCode() == taxCode && prev->channelId() == channelId &&
        prev->createDate() == createDate && prev->channelType() == channelType &&
        prev->vendor() == vendor)
    {
      tr = prev;
    }
    else
    {
      tr = new TaxExemption();
      tr->taxCode() = taxCode;
      tr->expireDate() = row->getDate(EXPIREDATE);
      tr->effDate() = row->getDate(EFFDATE);
      tr->discDate() = row->getDate(DISCDATE);
      tr->createDate() = createDate;
      tr->channelId() = channelId;
      tr->channelType() = channelType;
      tr->officeStationCode() = row->getString(IATASTATIONNUMBER);
      tr->vendor() = vendor;
      tr->exemptionSegId() = exemptionSegId;
    }

    if (!row->isNull(VALIDATINGCARRIER))
      tr->validationCxr().push_back(row->getString(VALIDATINGCARRIER));

    return tr;
  } // mapRowToTaxExemption()


private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxExemptionHistoricalSQLStatement :
  public QueryGetTaxExemptionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("tr.TAXCODE = %1q"
                " and tr.CHANNELID = %2q"
                " and tr.VALIDITYIND = 'Y'");
  }
};

} //namespace tse
