//----------------------------------------------------------------------------
//          File:           QueryGetNonCombCatCtrlSQLStatement.h
//          Description:    QueryGetNonCombCatCtrlSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Utils/CommonUtils.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNonCombCatCtrl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class Statement>
void mapRowToCategoryRuleItem(const Row* row, tse::CategoryRuleItemInfo& catCtrlSeg)
{
  catCtrlSeg.setOrderNo(row->getInt(Statement::ORDERNO));
  catCtrlSeg.setItemNo(row->getInt(Statement::ITEMNO));

  const char relationalInd = row->getChar(Statement::RELATIONALIND);
  catCtrlSeg.setRelationalInd(logicalOpFromChar(relationalInd));

  catCtrlSeg.setInOutInd(row->getChar(Statement::INOUTIND));
  catCtrlSeg.setDirectionality(row->getChar(Statement::DIRECTIONALITY));
  catCtrlSeg.setItemcat(row->getInt(Statement::ITEMCAT));
}

template <class QUERYCLASS>
class QueryGetNonCombCatCtrlSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNonCombCatCtrlSQLStatement() {};
  virtual ~QueryGetNonCombCatCtrlSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    CATEGORY,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    ORDERNO,
    ITEMNO,
    RELATIONALIND,
    INOUTIND,
    DIRECTIONALITY,
    ITEMCAT,
    MEMONO,
    MCN,
    BATCHNO,
    SEGCOUNT,
    JOINTCARRIERTBLITEMNO,
    GENERALRULETARIFF,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    FARECLASS,
    OWRT,
    ROUTINGAPPL,
    ROUTING,
    FOOTNOTE1,
    FOOTNOTE2,
    FARETYPE,
    SEASONTYPE,
    DOWTYPE,
    APPLIND,
    BATCHCI,
    GENERALRULE,
    GENERALRULEAPPL,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    R2SCATEGORY,
    R2SSEQNO,
    INHIBIT,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select r2.VENDOR,r2.CARRIER,r2.RULETARIFF,r2.RULE,r2.CATEGORY,r2.SEQNO,r2.CREATEDATE,"
        " EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,ITEMCAT,MEMONO,MCN,BATCHNO,SEGCOUNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,"
        " ROUTING,FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,"
        " GENERALRULE,GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " r2s.CATEGORY R2SCATEGORY,r2s.SEQNO R2SSEQNO,INHIBIT");
    // this->From("=NONCOMBCATCNTL r2 LEFT OUTER JOIN =CONDITIONALRULEEXPNC r2s"
    //           "       USING (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE)");

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    this->generateJoinString("=NONCOMBCATCNTL",
                             "r2",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPNC",
                             "r2s",
                             joinFields,
                             from);
    this->From(from);
    this->Where("r2.VENDOR = %1q"
                " and r2.CARRIER = %2q"
                " and r2.RULETARIFF = %3n"
                " and r2.RULE = %4q"
                " and r2.CATEGORY = %5n "
                " and r2.OVERRIDEIND = '' "
                " and %cd <= r2.DISCDATE"
                " and %cd <= r2.EXPIREDATE"
                " and r2.EFFDATE <= r2.DISCDATE");
    this->OrderBy("SEQNO, CREATEDATE, ORDERNO ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::GeneralFareRuleInfo*
  mapRowToGeneralFareRule(Row* row, GeneralFareRuleInfo* catCtrlPrev)
  {
    SequenceNumberLong seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    tse::CategoryRuleItemInfoSet* segSets = nullptr;

    // Parent not changed, just have a new child row
    if (catCtrlPrev != nullptr && seqNo == catCtrlPrev->sequenceNumber() &&
        createDate == catCtrlPrev->createDate())
    {
      if (LIKELY(!row->isNull(ORDERNO)))
      {
        CategoryRuleItemInfo ctrlSeg;
        mapRowToCategoryRuleItem<QueryGetNonCombCatCtrlSQLStatement>(row, ctrlSeg);
        if (ctrlSeg.relationalInd() == CategoryRuleItemInfo::THEN ||
            ctrlSeg.relationalInd() == CategoryRuleItemInfo::ELSE)
        {
          segSets = tools::new_reserved<CategoryRuleItemInfoSet>(
              catCtrlPrev->segcount());
          catCtrlPrev->addItemInfoSetNosync(segSets);
        }
        else
        {
          segSets = catCtrlPrev->categoryRuleItemInfoSet().back();
        }
        segSets->push_back(ctrlSeg);
      }

      return catCtrlPrev;
    }
    else
    {
      tse::GeneralFareRuleInfo* catCtrl = new tse::GeneralFareRuleInfo;

      catCtrl->vendorCode() = row->getString(VENDOR);
      catCtrl->carrierCode() = row->getString(CARRIER);
      catCtrl->tariffNumber() = row->getInt(RULETARIFF);
      catCtrl->ruleNumber() = row->getString(RULE);
      catCtrl->categoryNumber() = row->getInt(CATEGORY);
      catCtrl->sequenceNumber() = row->getLong(SEQNO);
      catCtrl->createDate() = row->getDate(CREATEDATE);
      catCtrl->expireDate() = row->getDate(EXPIREDATE);
      catCtrl->effDate() = row->getDate(EFFDATE);
      catCtrl->discDate() = row->getDate(DISCDATE);
      catCtrl->segcount() = row->getInt(SEGCOUNT);
      catCtrl->jointCarrierTblItemNo() = row->getInt(JOINTCARRIERTBLITEMNO);
      catCtrl->generalRuleTariff() = row->getInt(GENERALRULETARIFF);
      catCtrl->loc1().locType() = row->getChar(LOC1TYPE);
      catCtrl->loc1().loc() = row->getString(LOC1);
      catCtrl->loc2().locType() = row->getChar(LOC2TYPE);
      catCtrl->loc2().loc() = row->getString(LOC2);
      catCtrl->fareClass() = row->getString(FARECLASS);
      catCtrl->owrt() = row->getChar(OWRT);
      catCtrl->routingAppl() = row->getChar(ROUTINGAPPL);
      catCtrl->routing() = row->getString(ROUTING);
      catCtrl->footNote1() = row->getString(FOOTNOTE1);
      catCtrl->footNote2() = row->getString(FOOTNOTE2);
      catCtrl->fareType() = row->getString(FARETYPE);
      catCtrl->seasonType() = row->getChar(SEASONTYPE);
      catCtrl->dowType() = row->getChar(DOWTYPE);
      catCtrl->applInd() = row->getChar(APPLIND);
      catCtrl->generalRule() = row->getString(GENERALRULE);
      catCtrl->generalRuleAppl() = row->getChar(GENERALRULEAPPL);
      catCtrl->inhibit() = row->getChar(INHIBIT);

      if (!row->isNull(ORDERNO))
      {
        tse::CategoryRuleItemInfo ctrlSeg;
        mapRowToCategoryRuleItem<QueryGetNonCombCatCtrlSQLStatement>(row, ctrlSeg);
        segSets = tools::new_reserved<CategoryRuleItemInfoSet>(
            catCtrl->segcount());
        segSets->push_back(ctrlSeg);
        catCtrl->addItemInfoSetNosync(segSets);
      }

      return catCtrl;
    }
  } // mapRowToGeneralFareRule()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNonCombCatCtrlHistoricalSQLStatement
    : public QueryGetNonCombCatCtrlSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string from;
    std::vector<std::string> joinFields;

    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    partialStatement.Command(
        "("
        "select r2h.VENDOR,r2h.CARRIER,r2h.RULETARIFF,r2h.RULE,r2h.CATEGORY,r2h.SEQNO,"
        " r2h.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,ITEMCAT,MEMONO,MCN,BATCHNO,SEGCOUNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,"
        " ROUTING,FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,"
        " GENERALRULE,GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " r2sh.CATEGORY R2SCATEGORY,r2sh.SEQNO R2SSEQNO,INHIBIT");
    // partialStatement.From("=NONCOMBCATCNTLH r2h LEFT OUTER JOIN =CONDITIONALRULEEXPNCH r2sh"
    //                      "       USING
    // (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=NONCOMBCATCNTLH",
                             "r2h",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPNCH",
                             "r2sh",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2h.VENDOR = %1q"
                           " and r2h.CARRIER = %2q"
                           " and r2h.RULETARIFF = %3n"
                           " and r2h.RULE = %4q"
                           " and r2h.CATEGORY = %5n "
                           " and r2h.OVERRIDEIND = '' "
                           " and %6n <= r2h.EXPIREDATE"
                           " and %7n >= r2h.CREATEDATE"
                           " and r2h.EFFDATE <= r2h.DISCDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select r2.VENDOR,r2.CARRIER,r2.RULETARIFF,r2.RULE,r2.CATEGORY,r2.SEQNO,"
        " r2.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,ITEMCAT,MEMONO,MCN,BATCHNO,SEGCOUNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,"
        " ROUTING,FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,"
        " GENERALRULE,GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " r2s.CATEGORY R2SCATEGORY,r2s.SEQNO R2SSEQNO,INHIBIT");
    // partialStatement.From("=NONCOMBCATCNTL r2 LEFT OUTER JOIN =CONDITIONALRULEEXPNC r2s"
    //                     "       USING
    // (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=NONCOMBCATCNTL",
                             "r2",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPNC",
                             "r2s",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2.VENDOR = %8q"
                           " and r2.CARRIER = %9q"
                           " and r2.RULETARIFF = %10n"
                           " and r2.RULE = %11q"
                           " and r2.CATEGORY = %12n "
                           " and r2.OVERRIDEIND = '' "
                           " and %13n <= r2.EXPIREDATE"
                           " and %14n >= r2.CREATEDATE"
                           " and r2.EFFDATE <= r2.DISCDATE"
                           ")");
    partialStatement.OrderBy("SEQNO, EFFDATE desc, CREATEDATE desc, ORDERNO ");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNonCombCtrlBackDatingSQLStatement
    : public QueryGetNonCombCatCtrlSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string from;
    std::vector<std::string> joinFields;

    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    partialStatement.Command(
        "("
        "select r2h.VENDOR,r2h.CARRIER,r2h.RULETARIFF,r2h.RULE,r2h.CATEGORY,r2h.SEQNO,"
        " r2h.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,ITEMCAT,MEMONO,MCN,BATCHNO,SEGCOUNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,"
        " ROUTING,FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,"
        " GENERALRULE,GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " r2sh.CATEGORY R2SCATEGORY,r2sh.SEQNO R2SSEQNO,INHIBIT");
    this->generateJoinString(partialStatement,
                             "=NONCOMBCATCNTLH",
                             "r2h",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPNCH",
                             "r2sh",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2h.VENDOR = %1q"
                           " and r2h.CARRIER = %2q"
                           " and r2h.RULETARIFF = %3n"
                           " and r2h.RULE = %4q"
                           " and r2h.CATEGORY = %5n "
                           " and r2h.OVERRIDEIND = '' "
                           " and r2h.DISCDATE >= %6n "
                           " and r2h.EFFDATE <= %7n "
                           " and r2h.EXPIREDATE >= %8n "
                           " and r2h.EFFDATE <= r2h.DISCDATE "
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select r2.VENDOR,r2.CARRIER,r2.RULETARIFF,r2.RULE,r2.CATEGORY,r2.SEQNO,"
        " r2.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,ITEMCAT,MEMONO,MCN,BATCHNO,SEGCOUNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,"
        " ROUTING,FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,"
        " GENERALRULE,GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " r2s.CATEGORY R2SCATEGORY,r2s.SEQNO R2SSEQNO,INHIBIT");
    // partialStatement.From("=NONCOMBCATCNTL r2 LEFT OUTER JOIN =CONDITIONALRULEEXPNC r2s"
    //                     "       USING
    // (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=NONCOMBCATCNTL",
                             "r2",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPNC",
                             "r2s",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2.VENDOR = %9q"
                           " and r2.CARRIER = %10q"
                           " and r2.RULETARIFF = %11n"
                           " and r2.RULE = %12q"
                           " and r2.CATEGORY = %13n "
                           " and r2.OVERRIDEIND = '' "
                           " and r2.DISCDATE >= %14n "
                           " and r2.EFFDATE <= %15n "
                           " and r2.EXPIREDATE >= %16n "
                           " and r2.EFFDATE <= r2.DISCDATE "
                           ")");
    partialStatement.OrderBy("SEQNO, EFFDATE desc, CREATEDATE desc, ORDERNO ");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and remove OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllNonCombCatCtrlSQLStatement : public QueryGetNonCombCatCtrlSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("r2.OVERRIDEIND = '' "
                "    and %cd <= r2.DISCDATE"
                "    and %cd <= r2.EXPIREDATE"
                "    and r2.EFFDATE <= r2.DISCDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("r2.VENDOR,r2.CARRIER,r2.RULETARIFF,r2.RULE,r2.CATEGORY");
    else
      this->OrderBy("");
  }
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetFBRCtrl
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFBRCtrlSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFBRCtrlSQLStatement() {};
  virtual ~QueryGetFBRCtrlSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    CATEGORY,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    ORDERNO,
    ITEMNO,
    RELATIONALIND,
    INOUTIND,
    DIRECTIONALITY,
    ITEMCAT,
    SEGCNT,
    JOINTCARRIERTBLITEMNO,
    GENERALRULETARIFF,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    APPLIND,
    GENERALRULE,
    GENERALRULEAPPL,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    INHIBIT,
    LOC1ZONETBLITEMNO,
    LOC2ZONETBLITEMNO,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r2.VENDOR,r2.CARRIER,r2.RULETARIFF,r2.RULE,r2.CATEGORY,r2.SEQNO,"
                  "       r2.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,"
                  "       INOUTIND,DIRECTIONALITY,ITEMCAT,SEGCNT,JOINTCARRIERTBLITEMNO,"
                  "       GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,APPLIND,GENERALRULE,"
                  "       GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
                  "       INHIBIT,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO");
    // this->From("=FAREBYRULECNTL r2 LEFT OUTER JOIN =CONDRULEEXPFBR r2s "
    //           "        USING (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE) ");

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=FAREBYRULECNTL", "r2", "LEFT OUTER JOIN", "=CONDRULEEXPFBR", "r2s", joinFields, from);
    this->From(from);
    this->Where("r2.VENDOR = %1q"
                "    and r2.CARRIER = %2q"
                "    and r2.RULETARIFF = %3n"
                "    and r2.RULE = %4q"
                "    and %cd <= r2.EXPIREDATE");
    this->OrderBy("SEQNO, CREATEDATE, ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareByRuleCtrlInfo* mapRowToFareByRuleCtrlInfo(Row* row, FareByRuleCtrlInfo* catCtrl)
  {
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);
    tse::CategoryRuleItemInfoSet* segSets = nullptr;

    // Parent has changed
    if (catCtrl == nullptr || seqNo != catCtrl->sequenceNumber() || createDate != catCtrl->createDate())
    {
      catCtrl = new tse::FareByRuleCtrlInfo;

      catCtrl->categoryNumber() = row->getInt(CATEGORY);
      catCtrl->vendorCode() = row->getString(VENDOR);
      catCtrl->carrierCode() = row->getString(CARRIER);
      catCtrl->tariffNumber() = row->getInt(RULETARIFF);
      catCtrl->ruleNumber() = row->getString(RULE);
      catCtrl->segCnt() = row->getInt(SEGCNT);
      catCtrl->sequenceNumber() = seqNo;
      catCtrl->createDate() = createDate;
      catCtrl->expireDate() = row->getDate(EXPIREDATE);
      catCtrl->effDate() = row->getDate(EFFDATE);
      catCtrl->discDate() = row->getDate(DISCDATE);
      catCtrl->jointCarrierTblItemNo() = row->getInt(JOINTCARRIERTBLITEMNO);
      catCtrl->generalRuleTariff() = row->getInt(GENERALRULETARIFF);
      catCtrl->loc1().locType() = row->getChar(LOC1TYPE);
      catCtrl->loc1().loc() = row->getString(LOC1);
      catCtrl->loc2().locType() = row->getChar(LOC2TYPE);
      catCtrl->loc2().loc() = row->getString(LOC2);

      catCtrl->inhibit() = row->getChar(INHIBIT);
      catCtrl->applInd() = row->getChar(APPLIND);
      catCtrl->generalRule() = row->getString(GENERALRULE);
      catCtrl->generalRuleAppl() = row->getChar(GENERALRULEAPPL);
      catCtrl->loc1zoneTblItemNo() = row->getString(LOC1ZONETBLITEMNO);
      catCtrl->loc2zoneTblItemNo() = row->getString(LOC2ZONETBLITEMNO);
    }

    if (!row->isNull(ORDERNO))
    {
      tse::CategoryRuleItemInfo ctrlSeg;
      mapRowToCategoryRuleItem<QueryGetFBRCtrlSQLStatement>(row, ctrlSeg);
      if (catCtrl->categoryRuleItemInfoSet().empty() || ctrlSeg.relationalInd() == CategoryRuleItemInfo::THEN ||
          ctrlSeg.relationalInd() == CategoryRuleItemInfo::ELSE)
      {
        segSets = tools::new_reserved<CategoryRuleItemInfoSet>(
            catCtrl->segCnt());
        catCtrl->addItemInfoSetNosync(segSets);
      }
      else
      {
        segSets = catCtrl->categoryRuleItemInfoSet().back();
      }
      segSets->push_back(ctrlSeg);
    }

    return catCtrl;
  } // mapRowToFareByRuleCtrlInfo()



private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFBRCtrlHistoricalSQLStatement : public QueryGetFBRCtrlSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    partialStatement.Command(
        "("
        "select r2h.VENDOR,r2h.CARRIER,r2h.RULETARIFF,r2h.RULE,r2h.CATEGORY,r2h.SEQNO,"
        " r2h.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,"
        " INOUTIND,DIRECTIONALITY,ITEMCAT,SEGCNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,APPLIND,GENERALRULE,"
        " GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " INHIBIT,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO");
    // partialStatement.From("=FAREBYRULECNTLH r2h LEFT OUTER JOIN =CONDRULEEXPFBRH r2sh "
    //                      "   USING (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE) ");
    this->generateJoinString(partialStatement,
                             "=FAREBYRULECNTLH",
                             "r2h",
                             "LEFT OUTER JOIN",
                             "=CONDRULEEXPFBRH",
                             "r2sh",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2h.VENDOR = %1q"
                           " and r2h.CARRIER = %2q"
                           " and r2h.RULETARIFF = %3n"
                           " and r2h.RULE = %4q"
                           " and %5n <= r2h.EXPIREDATE"
                           " and (   %6n >=  r2h.CREATEDATE"
                           "      or %7n >= r2h.EFFDATE)"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select r2.VENDOR,r2.CARRIER,r2.RULETARIFF,r2.RULE,r2.CATEGORY,r2.SEQNO,"
        " r2.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,RELATIONALIND,"
        " INOUTIND,DIRECTIONALITY,ITEMCAT,SEGCNT,JOINTCARRIERTBLITEMNO,"
        " GENERALRULETARIFF,LOC1TYPE,LOC1,LOC2TYPE,LOC2,APPLIND,GENERALRULE,"
        " GENERALRULEAPPL,VERSIONINHERITEDIND,VERSIONDISPLAYIND,"
        " INHIBIT,LOC1ZONETBLITEMNO,LOC2ZONETBLITEMNO");
    // partialStatement.From("=FAREBYRULECNTL r2 LEFT OUTER JOIN =CONDRULEEXPFBR r2s "
    //                     "   USING (VENDOR,CARRIER,RULETARIFF,RULE,CATEGORY,SEQNO,CREATEDATE) ");
    this->generateJoinString(partialStatement,
                             "=FAREBYRULECNTL",
                             "r2",
                             "LEFT OUTER JOIN",
                             "=CONDRULEEXPFBR",
                             "r2s",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2.VENDOR = %8q"
                           " and r2.CARRIER = %9q"
                           " and r2.RULETARIFF = %10n"
                           " and r2.RULE = %11q"
                           " and %12n <= r2.EXPIREDATE"
                           " and (   %13n >=  r2.CREATEDATE"
                           "      or %14n >= r2.EFFDATE)"
                           ")");
    partialStatement.OrderBy("SEQNO, EFFDATE desc, CREATEDATE desc, ORDERNO ");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetFBRCtrlHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetFootNoteCtrl
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFootNoteCtrlSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFootNoteCtrlSQLStatement() {};
  virtual ~QueryGetFootNoteCtrlSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    FARETARIFF,
    FOOTNOTE,
    CATEGORY,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    ORDERNO,
    ITEMNO,
    RELATIONALIND,
    INOUTIND,
    DIRECTIONALITY,
    ITEMCAT,
    SEGCNT,
    JOINTCARRIERTBLITEMNO,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    FARECLASSCODE,
    OWRT,
    ROUTINGAPPL,
    ROUTING,
    APPLIND,
    INHIBIT,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("FARETARIFF");
    joinFields.push_back("FOOTNOTE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    this->Command("select r2.VENDOR,r2.CARRIER,r2.FARETARIFF,r2.FOOTNOTE,r2.CATEGORY,"
                  " r2.SEQNO,r2.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,"
                  " RELATIONALIND,INOUTIND,DIRECTIONALITY,ITEMCAT,SEGCNT,"
                  " JOINTCARRIERTBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASSCODE,"
                  " OWRT,ROUTINGAPPL,ROUTING,APPLIND,INHIBIT");
    // this->From("=FTNTCNTL r2 LEFT OUTER JOIN =CONDRULEEXPFTNT r2s"
    //           "     USING (VENDOR,CARRIER,FARETARIFF,FOOTNOTE,CATEGORY,SEQNO,CREATEDATE)");
    this->generateJoinString(
        "=FTNTCNTL", "r2", "LEFT OUTER JOIN", "=CONDRULEEXPFTNT", "r2s", joinFields, from);
    this->From(from);
    this->Where("r2.VENDOR = %1q"
                " and r2.CARRIER = %2q"
                " and r2.FARETARIFF = %3n"
                " and r2.FOOTNOTE = %4q"
                " and r2.CATEGORY = %5n"
                " and %cd <= r2.EXPIREDATE"
                " and VALIDITYIND = 'Y'");
    this->OrderBy("SEQNO, CREATEDATE, ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FootNoteCtrlInfo* mapRowToFootNoteCtrlInfo(Row* row, FootNoteCtrlInfo* catCtrlPrev)
  {
    SequenceNumberLong seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    tse::CategoryRuleItemInfoSet* segSets = nullptr;

    // Parent not changed, just have a new child row
    if (catCtrlPrev != nullptr && seqNo == catCtrlPrev->sequenceNumber() &&
        row->getString(VENDOR) == catCtrlPrev->vendorCode() &&
        row->getString(CARRIER) == catCtrlPrev->carrierCode() &&
        row->getInt(FARETARIFF) == catCtrlPrev->fareTariff() &&
        row->getString(FOOTNOTE) == catCtrlPrev->footNote() &&
        row->getInt(CATEGORY) == catCtrlPrev->categoryNumber() &&
        createDate == catCtrlPrev->createDate())
    {
      if (!row->isNull(ORDERNO))
      {
        tse::CategoryRuleItemInfo ctrlSeg;
        mapRowToCategoryRuleItem<QueryGetFootNoteCtrlSQLStatement>(row, ctrlSeg);
        if (ctrlSeg.relationalInd() == CategoryRuleItemInfo::THEN ||
            ctrlSeg.relationalInd() == CategoryRuleItemInfo::ELSE)
        {
          segSets = tools::new_reserved<CategoryRuleItemInfoSet>(
              catCtrlPrev->segcount());
          catCtrlPrev->addItemInfoSetNosync(segSets);
        }
        else
        {
          segSets = catCtrlPrev->categoryRuleItemInfoSet().back();
        }
        segSets->push_back(ctrlSeg);
      } // if (got Child)

      return catCtrlPrev;
    }
    else
    {
      tse::FootNoteCtrlInfo* catCtrl = new tse::FootNoteCtrlInfo;

      catCtrl->vendorCode() = row->getString(VENDOR);

      catCtrl->carrierCode() = row->getString(CARRIER);

      catCtrl->tariffNumber() = -1; // These columns don't exist in the query or the table it uses
      catCtrl->ruleNumber() = ""; // These columns don't exist in the query or the table it uses

      catCtrl->footNote() = row->getString(FOOTNOTE);
      catCtrl->categoryNumber() = row->getInt(CATEGORY);
      catCtrl->sequenceNumber() = row->getLong(SEQNO);

      catCtrl->createDate() = row->getDate(CREATEDATE);
      catCtrl->expireDate() = row->getDate(EXPIREDATE);

      catCtrl->effDate() = row->getDate(EFFDATE);
      catCtrl->discDate() = row->getDate(DISCDATE);

      catCtrl->segcount() = row->getInt(SEGCNT);
      catCtrl->jointCarrierTblItemNo() = row->getInt(JOINTCARRIERTBLITEMNO);
      catCtrl->fareTariff() = row->getInt(FARETARIFF);
      catCtrl->loc1().locType() = row->getChar(LOC1TYPE);
      catCtrl->loc1().loc() = row->getString(LOC1);
      catCtrl->loc2().locType() = row->getChar(LOC2TYPE);
      catCtrl->loc2().loc() = row->getString(LOC2);
      catCtrl->fareClass() = row->getString(FARECLASSCODE);
      catCtrl->owrt() = row->getChar(OWRT);
      catCtrl->routingAppl() = row->getChar(ROUTINGAPPL);
      catCtrl->routing() = row->getString(ROUTING);
      catCtrl->applInd() = row->getChar(APPLIND);
      catCtrl->inhibit() = row->getChar(INHIBIT);

      if (!row->isNull(ORDERNO))
      {
        tse::CategoryRuleItemInfo ctrlSeg;
        mapRowToCategoryRuleItem<QueryGetFootNoteCtrlSQLStatement>(row, ctrlSeg);
        segSets = tools::new_reserved<CategoryRuleItemInfoSet>(
            catCtrl->segcount());
        segSets->push_back(ctrlSeg);
        catCtrl->addItemInfoSetNosync(segSets);
      }

      return catCtrl;
    }
  } // mapRowToFootNoteCtrlInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetFootNoteCtrlSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFootNoteCtrlHistoricalSQLStatement
    : public QueryGetFootNoteCtrlSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("FARETARIFF");
    joinFields.push_back("FOOTNOTE");
    joinFields.push_back("CATEGORY");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    partialStatement.Command(
        "("
        "select r2h.VENDOR,r2h.CARRIER,r2h.FARETARIFF,r2h.FOOTNOTE,r2h.CATEGORY,"
        " r2h.SEQNO,r2h.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,"
        " RELATIONALIND,INOUTIND,DIRECTIONALITY,ITEMCAT,SEGCNT,"
        " JOINTCARRIERTBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASSCODE,"
        " OWRT,ROUTINGAPPL,ROUTING,APPLIND,INHIBIT");
    // partialStatement.From("=FTNTCNTLH r2h LEFT OUTER JOIN =CONDRULEEXPFTNTH r2sh"
    //                      "   USING
    // (VENDOR,CARRIER,FARETARIFF,FOOTNOTE,CATEGORY,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=FTNTCNTLH",
                             "r2h",
                             "LEFT OUTER JOIN",
                             "=CONDRULEEXPFTNTH",
                             "r2sh",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2h.VENDOR = %1q"
                           " and r2h.CARRIER = %2q"
                           " and r2h.FARETARIFF = %3n"
                           " and r2h.FOOTNOTE = %4q"
                           " and r2h.CATEGORY = %5n"
                           " and %6n <= r2h.EXPIREDATE"
                           " and %7n >= r2h.CREATEDATE"
                           " and VALIDITYIND = 'Y'"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(" union all"
                             " ("
                             "select r2.VENDOR,r2.CARRIER,r2.FARETARIFF,r2.FOOTNOTE,r2.CATEGORY,"
                             " r2.SEQNO,r2.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ORDERNO,ITEMNO,"
                             " RELATIONALIND,INOUTIND,DIRECTIONALITY,ITEMCAT,SEGCNT,"
                             " JOINTCARRIERTBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASSCODE,"
                             " OWRT,ROUTINGAPPL,ROUTING,APPLIND,INHIBIT");
    // partialStatement.From("=FTNTCNTL r2 LEFT OUTER JOIN =CONDRULEEXPFTNT r2s"
    //                      "   USING
    // (VENDOR,CARRIER,FARETARIFF,FOOTNOTE,CATEGORY,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=FTNTCNTL",
                             "r2",
                             "LEFT OUTER JOIN",
                             "=CONDRULEEXPFTNT",
                             "r2s",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r2.VENDOR = %8q"
                           " and r2.CARRIER = %9q"
                           " and r2.FARETARIFF = %10n"
                           " and r2.FOOTNOTE = %11q"
                           " and r2.CATEGORY = %12n"
                           " and %13n <= r2.EXPIREDATE"
                           " and %14n >=  r2.CREATEDATE"
                           " and VALIDITYIND = 'Y'"
                           ")");
    partialStatement.OrderBy("SEQNO, EFFDATE desc, CREATEDATE desc, ORDERNO ");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetFootNoteCtrlHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFootNoteCtrlSQLStatement : public QueryGetFootNoteCtrlSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= r2.EXPIREDATE"
                "    and VALIDITYIND = 'Y'");
    this->OrderBy("VENDOR,CARRIER,FARETARIFF,FOOTNOTE,CATEGORY,SEQNO,CREATEDATE,ORDERNO");
  }
};
} // tse
