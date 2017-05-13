#pragma once

#include "Common/FallbackUtil.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackFFRaddFocusCodeInFFR);

template <typename QUERYCLASS>
class QueryGetFareFocusRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    FAREFOCUSRULEID
    , FAREFOCUSRULEFAMILYID
    , STATUSCD
    , PROCESSSTATECD
    , SOURCEPSEUDOCITY
    , SECURITYITEMNO
    , VENDOR
    , CARRIER
    , RULECD
    , RULETARIFF
    , FARECLASSITEMNO
    , FARETYPE
    , BOOKINGCDITEMNO
    , OWRT
    , DISPLAYTYPE
    , LOC1
    , LOC1TYPE
    , LOC2
    , LOC2TYPE
    , DIRECTIONALITY
    , PUBLICPRIVATEIND
    , EFFDATETIME
    , DISCDATETIME
    , EXPIREDATE
    , CREATEDATE
    , LASTMODDATE
    , ACTIVATIONDATETIME
    , INHIBITCD
    , EFFECTIVENOWIND
    , DEACTIVATENOWIND
    , GLOBALDIR
    , PSGTYPEITEMNO
    , ACCOUNTCDITEMNO
    , ROUTINGITEMNO
    , RETAILERCD
    , POSDAYTIMEAPPLITEMNO
    , TRAVELDAYTIMEAPPLITEMNO
    , LOCATIONPAIREXCLUDEITEMNO
    , FARECLASSEXCLUDEITEMNO
    , FARETYPEEXCLUDEITEMNO
    , DISPLAYTYPEEXCLUDEITEMNO
    , PSGTYPEEXCLUDEITEMNO
    , ACCOUNTCDEXCLUDEITEMNO
    , ROUTINGEXCLUDEITEMNO
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    if (fallback::fixed::fallbackFFRaddFocusCodeInFFR())
    {
      Command("select FAREFOCUSRULEID, FAREFOCUSRULEFAMILYID, STATUSCD, PROCESSSTATECD,"
              "SOURCEPSEUDOCITY, SECURITYITEMNO, VENDOR, CARRIER, RULECD, RULETARIFF,"
              "FARECLASSITEMNO, FARETYPE, BOOKINGCDITEMNO, OWRT, DISPLAYTYPE, LOC1,"
              "LOC1TYPE, LOC2, LOC2TYPE, DIRECTIONALITY, PUBLICPRIVATEIND, EFFDATETIME,"
              "DISCDATETIME, EXPIREDATE, CREATEDATE, LASTMODDATE, ACTIVATIONDATETIME, INHIBITCD "
              "from FAREFOCUSRULE "
              "where FAREFOCUSRULEID = %1n and STATUSCD = 'A' and INHIBITCD = 'N' "
              "and %2n <= DISCDATETIME and %3n <= EXPIREDATE and EFFDATETIME <= DISCDATETIME");
    }
    else
    {
      Command("select FAREFOCUSRULEID, FAREFOCUSRULEFAMILYID, STATUSCD, PROCESSSTATECD,"
              "SOURCEPSEUDOCITY, SECURITYITEMNO, VENDOR, CARRIER, RULECD, RULETARIFF,"
              "FARECLASSITEMNO, FARETYPE, BOOKINGCDITEMNO, OWRT, DISPLAYTYPE, LOC1,"
              "LOC1TYPE, LOC2, LOC2TYPE, DIRECTIONALITY, PUBLICPRIVATEIND, EFFDATETIME,"
              "DISCDATETIME, EXPIREDATE, CREATEDATE, LASTMODDATE, ACTIVATIONDATETIME, INHIBITCD,"
	      "EFFECTIVENOWIND, DEACTIVATENOWIND, GLOBALDIR, PSGTYPEITEMNO, ACCOUNTCDITEMNO,"
	      "ROUTINGITEMNO, RETAILERCD, POSDAYTIMEAPPLITEMNO, TRAVELDAYTIMEAPPLITEMNO,"
              "LOCATIONPAIREXCLUDEITEMNO, FARECLASSEXCLUDEITEMNO, FARECLASSEXCLUDEITEMNO,"
              "DISPLAYTYPEEXCLUDEITEMNO, PSGTYPEEXCLUDEITEMNO, ACCOUNTCDEXCLUDEITEMNO, ROUTINGEXCLUDEITEMNO "
              "from FAREFOCUSRULE "
              "where FAREFOCUSRULEID = %1n and STATUSCD = 'A' and INHIBITCD = 'N' "
              "and %2n <= DISCDATETIME and %3n <= EXPIREDATE and EFFDATETIME <= DISCDATETIME");
	}  

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusRuleInfo* mapRow(Row* row, FareFocusRuleInfo*)
  {
    FareFocusRuleInfo* info(new FareFocusRuleInfo);
    info->fareFocusRuleId() = row->getLong(FAREFOCUSRULEID);
    info->fareFocusRuleFamilyId() = row->getLong(FAREFOCUSRULEFAMILYID);
    info->statusCode() = row->getChar(STATUSCD);
    info->processStateCode() = row->getChar(PROCESSSTATECD);
    info->sourcePCC() = row->getString(SOURCEPSEUDOCITY);
    info->securityItemNo() = row->getLong(SECURITYITEMNO);
    info->vendor() = row->getString(VENDOR);
    info->carrier() = row->getString(CARRIER);
    info->ruleCode() = row->getString(RULECD);
    if (!row->isNull(RULETARIFF))
    {
      info->ruleTariff() = row->getInt(RULETARIFF);
    }
    if (!row->isNull(FARECLASSITEMNO))
    {
      info->fareClassItemNo() = row->getLong(FARECLASSITEMNO);
    }
    if (!row->isNull(FARETYPE))
    {
      info->fareType() = row->getString(FARETYPE);
    }
    if (!row->isNull(BOOKINGCDITEMNO))
    {
      info->bookingCodeItemNo() = row->getLong(BOOKINGCDITEMNO);
    }
    if (!row->isNull(OWRT))
    {
      info->owrt() = row->getChar(OWRT);
    }
    if (!row->isNull(DISPLAYTYPE))
    {
      info->displayType() = row->getChar(DISPLAYTYPE);
    }
    info->loc1().loc() = row->getString(LOC1);
    if (!row->isNull(LOC1TYPE))
    {
      info->loc1().locType() = row->getChar(LOC1TYPE);
    }
    info->loc2().loc() = row->getString(LOC2);
    if (!row->isNull(LOC2TYPE))
    {
      info->loc2().locType() = row->getChar(LOC2TYPE);
    }
    if (!row->isNull(DIRECTIONALITY))
    {
      info->directionality() = row->getChar(DIRECTIONALITY);
    }
    if (!row->isNull(PUBLICPRIVATEIND))
    {
      info->publicPrivateIndicator() = row->getChar(PUBLICPRIVATEIND);
    }
    info->createDate() = row->getDate(CREATEDATE);
    info->effDate() = row->getDate(EFFDATETIME);
    info->discDate() = row->getDate(DISCDATETIME);
    info->expireDate() = row->getDate(EXPIREDATE);
    info->lastModDate() = row->getDate(LASTMODDATE);
    info->activationDateTime() = row->getDate(ACTIVATIONDATETIME);
    info->inhibitCD() = row->getChar(INHIBITCD);

    if (!fallback::fixed::fallbackFFRaddFocusCodeInFFR())
    {
      if (!row->isNull(EFFECTIVENOWIND))
        info->effectiveNowInd() = row->getChar(EFFECTIVENOWIND);
      if (!row->isNull(DEACTIVATENOWIND))
        info->deactivateNowInd() = row->getChar(DEACTIVATENOWIND);
      if (!row->isNull(GLOBALDIR))
        strToGlobalDirection(info->globalDir(), row->getString(GLOBALDIR));
      if (!row->isNull(PSGTYPEITEMNO))
        info->psgTypeItemNo() = row->getLong(PSGTYPEITEMNO);
      if (!row->isNull(ACCOUNTCDITEMNO))
        info->accountCdItemNo() = row->getLong(ACCOUNTCDITEMNO);
      if (!row->isNull(ROUTINGITEMNO))
        info->routingItemNo() = row->getLong(ROUTINGITEMNO);
      if (!row->isNull(RETAILERCD))
        info->retailerCode() = row->getString(RETAILERCD);
      if (!row->isNull(POSDAYTIMEAPPLITEMNO))
        info->posDayTimeApplItemNo() = row->getLong(POSDAYTIMEAPPLITEMNO);
      if (!row->isNull(TRAVELDAYTIMEAPPLITEMNO))
        info->travelDayTimeApplItemNo() = row->getLong(TRAVELDAYTIMEAPPLITEMNO);
      if (!row->isNull(LOCATIONPAIREXCLUDEITEMNO))
        info->locationPairExcludeItemNo() = row->getLong(LOCATIONPAIREXCLUDEITEMNO);
      if (!row->isNull(FARECLASSEXCLUDEITEMNO))
        info->fareClassExcludeItemNo() = row->getLong(FARECLASSEXCLUDEITEMNO);
      if (!row->isNull(FARETYPEEXCLUDEITEMNO))
        info->fareTypeExcludeItemNo() = row->getLong(FARETYPEEXCLUDEITEMNO);
      if (!row->isNull(DISPLAYTYPEEXCLUDEITEMNO))
        info->displayTypeExcludeItemNo() = row->getLong(DISPLAYTYPEEXCLUDEITEMNO);
      if (!row->isNull(PSGTYPEEXCLUDEITEMNO))
        info->psgTypeExcludeItemNo() = row->getLong(PSGTYPEEXCLUDEITEMNO);
      if (!row->isNull(ACCOUNTCDEXCLUDEITEMNO))
        info->accountCdExcludeItemNo() = row->getLong(ACCOUNTCDEXCLUDEITEMNO);
      if (!row->isNull(ROUTINGEXCLUDEITEMNO))
        info->routingExcludeItemNo() = row->getLong(ROUTINGEXCLUDEITEMNO);
    }

    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusRuleHistoricalSQLStatement
    : public QueryGetFareFocusRuleSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    if (fallback::fixed::fallbackFFRaddFocusCodeInFFR())
    {
      this->Command("select FAREFOCUSRULEID, "
                    "FAREFOCUSRULEFAMILYID, STATUSCD, PROCESSSTATECD,"
                    "SOURCEPSEUDOCITY, SECURITYITEMNO, VENDOR, CARRIER, RULECD, RULETARIFF,"
                    "FARECLASSITEMNO, FARETYPE, BOOKINGCDITEMNO, OWRT, DISPLAYTYPE, LOC1,"
                    "LOC1TYPE, LOC2, LOC2TYPE, DIRECTIONALITY, PUBLICPRIVATEIND, EFFDATETIME,"
                    "DISCDATETIME, EXPIREDATE, CREATEDATE, LASTMODDATE, ACTIVATIONDATETIME, INHIBITCD "
                    "from FAREFOCUSRULEH "
                    "where FAREFOCUSRULEID = %1n and %2n <= EXPIREDATE and %3n >= CREATEDATE "
                    "and STATUSCD != 'P' and INHIBITCD = 'N' "
                    "union all "
                    "select FAREFOCUSRULEID, FAREFOCUSRULEFAMILYID, STATUSCD, PROCESSSTATECD,"
                    "SOURCEPSEUDOCITY, SECURITYITEMNO, VENDOR, CARRIER, RULECD, RULETARIFF,"
                    "FARECLASSITEMNO, FARETYPE, BOOKINGCDITEMNO, OWRT, DISPLAYTYPE, LOC1,"
                    "LOC1TYPE, LOC2, LOC2TYPE, DIRECTIONALITY, PUBLICPRIVATEIND, EFFDATETIME,"
                    "DISCDATETIME, EXPIREDATE, CREATEDATE, LASTMODDATE, ACTIVATIONDATETIME, INHIBITCD "
                    "from FAREFOCUSRULE "
                    "where FAREFOCUSRULEID = %4n and %5n <= EXPIREDATE and %6n >= CREATEDATE "
                    "and STATUSCD != 'P' and INHIBITCD = 'N' ");
    }
 
  else
    {
      this->Command("select FAREFOCUSRULEID, "
                    "FAREFOCUSRULEFAMILYID, STATUSCD, PROCESSSTATECD,"
                    "SOURCEPSEUDOCITY, SECURITYITEMNO, VENDOR, CARRIER, RULECD, RULETARIFF,"
                    "FARECLASSITEMNO, FARETYPE, BOOKINGCDITEMNO, OWRT, DISPLAYTYPE, LOC1,"
                    "LOC1TYPE, LOC2, LOC2TYPE, DIRECTIONALITY, PUBLICPRIVATEIND, EFFDATETIME,"
                    "DISCDATETIME, EXPIREDATE, CREATEDATE, LASTMODDATE, ACTIVATIONDATETIME, INHIBITCD,"
		    "EFFECTIVENOWIND, DEACTIVATENOWIND, GLOBALDIR, POSDAYTIMEAPPLITEMNO,"
		    "TRAVELDAYTIMEAPPLITEMNO, LOCATIONPAIREXCLUDEITEMNO,"
		    "PSGTYPEITEMNO, ACCOUNTCDITEMNO, ROUTINGITEMNO, RETAILERCD,"
	            "FARECLASSEXCLUDEITEMNO, FARETYPEEXCLUDEITEMNO, DISPLAYTYPEEXCLUDEITEMNO,"
	            "PSGTYPEEXCLUDEITEMNO, ACCOUNTCDEXCLUDEITEMNO, ROUTINGEXCLUDEITEMNO "
                    "from FAREFOCUSRULEH "
                    "where FAREFOCUSRULEID = %1n and %2n <= EXPIREDATE and %3n >= CREATEDATE "
                    "and STATUSCD != 'P' and INHIBITCD = 'N' "
                    "union all "
                    "select FAREFOCUSRULEID, FAREFOCUSRULEFAMILYID, STATUSCD, PROCESSSTATECD,"
                    "SOURCEPSEUDOCITY, SECURITYITEMNO, VENDOR, CARRIER, RULECD, RULETARIFF,"
                    "FARECLASSITEMNO, FARETYPE, BOOKINGCDITEMNO, OWRT, DISPLAYTYPE, LOC1,"
                    "LOC1TYPE, LOC2, LOC2TYPE, DIRECTIONALITY, PUBLICPRIVATEIND, EFFDATETIME,"
                    "DISCDATETIME, EXPIREDATE, CREATEDATE, LASTMODDATE, ACTIVATIONDATETIME, INHIBITCD,"
		    "EFFECTIVENOWIND, DEACTIVATENOWIND, GLOBALDIR, POSDAYTIMEAPPLITEMNO,"
		    "TRAVELDAYTIMEAPPLITEMNO, LOCATIONPAIREXCLUDEITEMNO, "
		    "PSGTYPEITEMNO, ACCOUNTCDITEMNO, ROUTINGITEMNO, RETAILERCD, "
	            "FARECLASSEXCLUDEITEMNO, FARETYPEEXCLUDEITEMNO, DISPLAYTYPEEXCLUDEITEMNO,"
	            "PSGTYPEEXCLUDEITEMNO, ACCOUNTCDEXCLUDEITEMNO, ROUTINGEXCLUDEITEMNO "
                    "from FAREFOCUSRULE "
                    "where FAREFOCUSRULEID = %4n and %5n <= EXPIREDATE and %6n >= CREATEDATE "
                    "and STATUSCD != 'P' and INHIBITCD = 'N' ");
     } 
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusRuleSQLStatement
    : public QueryGetFareFocusRuleSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE"
                " and STATUSCD = 'A'");
  }
};

} // tse

