#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

namespace
{

typedef boost::tokenizer<boost::char_separator<char> > Tokens;
boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);

void parseString(std::vector<char>& vect,
                 const std::string& source)
{
  Tokens tokens(source, sep);
  for (const auto& tok : tokens)
  {
    if (!tok.empty())
    {
      vect.push_back(tok[0]);
    }
  }
}

template <typename T> void parseString(std::vector<T>& vect,
                                       const std::string& source)
{
  Tokens tokens(source, sep);
  for (const auto& tok : tokens)
  {
    if (!tok.empty())
    {
      vect.emplace_back(tok);
    }
  }
}

}// namespace

template <typename QUERYCLASS>
class QueryGetCommissionRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR
    , COMMISSIONRULEID
    , CREATEDATETIME
    , EFFDATETIME
    , DISCDATETIME
    , EXPIREDATETIME
    , PROGRAMID
    , COSLASTUSER
    , COSLASTMODIFYDATETIME
    , COMMISSIONRULEDESCRIPTION
    , CURRENCY
    , COMMISSIONVALUE
    , NODEC
    , COMMISSIONTYPEID
    , INHIBIT
    , VALIDITYIND
    , FAREBASISCODEINCL
    , FAREBASISCODEEXCL
    , CLASSOFSERVICEINCL
    , CLASSOFSERVICEEXCL
    , OPERATINGCARRIERINCL
    , OPERATINGCARRIEREXCL
    , MARKETINGCARRIERINCL
    , MARKETINGCARRIEREXCL
    , TICKETINGCARRIERINCL
    , TICKETINGCARRIEREXCL
    , INTERLINECNXREQUIRED
    , ROUNDTRIP
    , FAREAMOUNTMIN
    , FAREAMOUNTMINNODEC
    , FAREAMOUNTMAX
    , FAREAMOUNTMAXNODEC
    , FAREAMOUNTCUR
    , FBCFRAGMENTINCL
    , FBCFRAGMENTEXCL
    , REQUIREDTKTDESIG
    , EXCLUDEDTKTDESIG
    , REQUIREDCNXAIRPORTCODES
    , EXCLUDEDCNXAIRPORTCODES
    , REQUIREDMKTGOVCXR
    , EXCLUDEDMKTGOVCXR
    , REQUIREDPAXTYPE
    , REQUIREDOPERATINGGOVCXR
    , EXCLUDEDOPERATINGGOVCXR
    , REQUIREDNONSTOP
    , REQUIREDCABINTYPE
    , EXCLUDEDCABINTYPE
    , EXCLUDEDTOURCODE
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select * from COMMISSIONRULE");

    this->Where("VENDOR = %1q and PROGRAMID = %2n and INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and EXPIREDATETIME >= %cd and DISCDATETIME >= %cd and EFFDATETIME <= DISCDATETIME");

    this->OrderBy("VENDOR, COMMISSIONRULEID");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CommissionRuleInfo* mapRow(Row* row)
  {
    CommissionRuleInfo* info(new CommissionRuleInfo);
    info->vendor() = row->getString(VENDOR);
    info->commissionId() = row->getLong(COMMISSIONRULEID);
    info->createDate() = row->getDate(CREATEDATETIME);
    if (!row->isNull(EFFDATETIME))
    {
      info->effDate() = row->getDate(EFFDATETIME);
    }
    if (!row->isNull(DISCDATETIME))
    {
      info->discDate() = row->getDate(DISCDATETIME);
    }
    if (!row->isNull(EXPIREDATETIME))
    {
      info->expireDate() = row->getDate(EXPIREDATETIME);
    }
    if (!row->isNull(PROGRAMID))
    {
      info->programId() = row->getLong(PROGRAMID);
    }
    if (!row->isNull(COMMISSIONRULEDESCRIPTION))
    {
      info->description() = row->getString(COMMISSIONRULEDESCRIPTION);
    }
    if (!row->isNull(CURRENCY))
    {
      info->currency() = row->getString(CURRENCY);
    }
    if (!row->isNull(COMMISSIONVALUE) && !row->isNull(NODEC))
    {
      info->commissionValue() = QUERYCLASS::adjustDecimal(row->getLong(COMMISSIONVALUE), row->getLong(NODEC));
    }
    if (!row->isNull(COMMISSIONTYPEID))
    {
      info->commissionTypeId() = row->getLong(COMMISSIONTYPEID);
    }
    if (!row->isNull(INHIBIT))
    {
      info->inhibit() = row->getChar(INHIBIT);
    }
    if (!row->isNull(VALIDITYIND))
    {
      info->validityIndicator() = row->getChar(VALIDITYIND);
    }
    if (!row->isNull(FAREBASISCODEINCL))
    {
      parseString(info->fareBasisCodeIncl(), row->getString(FAREBASISCODEINCL));
    }
    if (!row->isNull(FAREBASISCODEEXCL))
    {
      parseString(info->fareBasisCodeExcl(), row->getString(FAREBASISCODEEXCL));
    }
    if (!row->isNull(CLASSOFSERVICEINCL))
    {
      parseString(info->classOfServiceIncl(), row->getString(CLASSOFSERVICEINCL));
    }
    if (!row->isNull(CLASSOFSERVICEEXCL))
    {
      parseString(info->classOfServiceExcl(), row->getString(CLASSOFSERVICEEXCL));
    }
    if (!row->isNull(OPERATINGCARRIERINCL))
    {
      parseString(info->operatingCarrierIncl(), row->getString(OPERATINGCARRIERINCL));
    }
    if (!row->isNull(OPERATINGCARRIEREXCL))
    {
      parseString(info->operatingCarrierExcl(), row->getString(OPERATINGCARRIEREXCL));
    }
    if (!row->isNull(MARKETINGCARRIERINCL))
    {
      parseString(info->marketingCarrierIncl(), row->getString(MARKETINGCARRIERINCL));
    }
    if (!row->isNull(MARKETINGCARRIEREXCL))
    {
      parseString(info->marketingCarrierExcl(), row->getString(MARKETINGCARRIEREXCL));
    }
    if (!row->isNull(TICKETINGCARRIERINCL))
    {
      parseString(info->ticketingCarrierIncl(), row->getString(TICKETINGCARRIERINCL));
    }
    if (!row->isNull(TICKETINGCARRIEREXCL))
    {
      parseString(info->ticketingCarrierExcl(), row->getString(TICKETINGCARRIEREXCL));
    }
    if (!row->isNull(INTERLINECNXREQUIRED))
    {
      info->interlineConnectionRequired() = row->getChar(INTERLINECNXREQUIRED);
    }
    if (!row->isNull(ROUNDTRIP))
    {
      info->roundTrip() = row->getChar(ROUNDTRIP);
    }
    if (!row->isNull(FAREAMOUNTMIN) && !row->isNull(FAREAMOUNTMINNODEC))
    {
      info->fareAmountMin() = QUERYCLASS::adjustDecimal(row->getLong(FAREAMOUNTMIN), row->getInt(FAREAMOUNTMINNODEC));

    }
    if (!row->isNull(FAREAMOUNTMAX) && !row->isNull(FAREAMOUNTMAXNODEC))
    {
      info->fareAmountMax() = QUERYCLASS::adjustDecimal(row->getLong(FAREAMOUNTMAX), row->getInt(FAREAMOUNTMAXNODEC));
    }
    if (!row->isNull(FAREAMOUNTCUR))
    {
      info->fareAmountCurrency() = row->getString(FAREAMOUNTCUR);
    }
    if (!row->isNull(FBCFRAGMENTINCL))
    {
      parseString(info->fbcFragmentIncl(), row->getString(FBCFRAGMENTINCL));
    }
    if (!row->isNull(FBCFRAGMENTEXCL))
    {
      parseString(info->fbcFragmentExcl(), row->getString(FBCFRAGMENTEXCL));
    }
    if (!row->isNull(REQUIREDTKTDESIG))
    {
      std::vector<std::string> strVector;
      parseString(strVector, row->getString(REQUIREDTKTDESIG));
      for (const auto& str : strVector)
      {
        info->requiredTktDesig().emplace_back(str.c_str(), str.size());
      }
    }
    if (!row->isNull(EXCLUDEDTKTDESIG))
    {
      std::vector<std::string> strVector;
      parseString(strVector, row->getString(EXCLUDEDTKTDESIG));
      for (const auto& str : strVector)
      {
        info->excludedTktDesig().emplace_back(str.c_str(), str.size());
      }
    }
    if (!row->isNull(REQUIREDCNXAIRPORTCODES))
    {
      parseString(info->requiredCnxAirPCodes(), row->getString(REQUIREDCNXAIRPORTCODES));
    }
    if (!row->isNull(EXCLUDEDCNXAIRPORTCODES))
    {
      parseString(info->excludedCnxAirPCodes(), row->getString(EXCLUDEDCNXAIRPORTCODES));
    }
    if (!row->isNull(REQUIREDMKTGOVCXR))
    {
      parseString(info->requiredMktGovCxr(), row->getString(REQUIREDMKTGOVCXR));
    }
    if (!row->isNull(EXCLUDEDMKTGOVCXR))
    {
      parseString(info->excludedMktGovCxr(), row->getString(EXCLUDEDMKTGOVCXR));
    }
    if (!row->isNull(REQUIREDPAXTYPE))
    {
      parseString(info->requiredPaxType(), row->getString(REQUIREDPAXTYPE));
    }
    if (!row->isNull(REQUIREDOPERATINGGOVCXR))
    {
      parseString(info->requiredOperGovCxr(), row->getString(REQUIREDOPERATINGGOVCXR));
    }
    if (!row->isNull(EXCLUDEDOPERATINGGOVCXR))
    {
      parseString(info->excludedOperGovCxr(), row->getString(EXCLUDEDOPERATINGGOVCXR));
    }
    if (!row->isNull(REQUIREDNONSTOP))
    {
      std::vector<std::string> odVector;
      parseString(odVector, row->getString(REQUIREDNONSTOP));
      for (const std::string& od : odVector)
      {
        if (6 == od.size())
        {
          info->requiredNonStop().emplace_back(od.substr(0, 3), od.substr(3));
        }
      }
    }
    if (!row->isNull(REQUIREDCABINTYPE))
    {
      parseString(info->requiredCabinType(), row->getString(REQUIREDCABINTYPE));
    }
    if (!row->isNull(EXCLUDEDCABINTYPE))
    {
      parseString(info->excludedCabinType(), row->getString(EXCLUDEDCABINTYPE));
    }
    if (!row->isNull(EXCLUDEDTOURCODE))
    {
      parseString(info->excludedTourCode(), row->getString(EXCLUDEDTOURCODE));
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetCommissionRuleHistoricalSQLStatement
    : public QueryGetCommissionRuleSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" VENDOR = %1q and PROGRAMID = %2n and INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATETIME"
                " and %4n >= CREATEDATETIME");

    this->OrderBy("VENDOR, COMMISSIONRULEID");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllCommissionRuleSQLStatement
    : public QueryGetCommissionRuleSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and %cd >= CREATEDATETIME"
                " and %cd <= EXPIREDATETIME");

    this->OrderBy("VENDOR, COMMISSIONRULEID");
  }
};

} // tse
