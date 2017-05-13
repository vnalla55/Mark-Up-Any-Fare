//----------------------------------------------------------------------------
//          File:           QueryGetCombCtrlSQLStatement.h
//          Description:    QueryGetCombCtrlSQLStatement
//          Created:        10/30/2007
//          Authors:        Mike Lillis
//                          Simon Li
//
//          Updates:
//
//     (C) 2010, Sabre Inc.  All rights reserved. This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Utils/CommonUtils.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCombCtrl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCombCtrlSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCombCtrlSQLStatement() {};
  virtual ~QueryGetCombCtrlSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    CATEGORY,
    SEQNO,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    CREATEDATE,
    SEGCNT,
    JOINTCARRIERTBLITEMNO,
    SAMEPOINTSTBLITEMNO,
    DOJGENERALRULETARIFF,
    CT2GENERALRULETARIFF,
    CT2PLUSGENERALRULETARIFF,
    EOEGENERALRULETARIFF,
    ARBGENERALRULETARIFF,
    CREATORID,
    CREATORBUSINESSUNIT,
    VALIDITYIND,
    INHIBIT,
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
    SOJIND,
    SOJORIGINDESTIND,
    DOJIND,
    DOJCARRIERRESTIND,
    DOJTARIFFRULERESTIND,
    DOJFARECLASSTYPERESTIND,
    DOJGENERALRULE,
    DOJGENERALRULEAPPL,
    CT2IND,
    CT2CARRIERRESTIND,
    CT2TARIFFRULERESTIND,
    CT2FARECLASSTYPERESTIND,
    CT2GENERALRULE,
    CT2GENERALRULEAPPL,
    CT2PLUSIND,
    CT2PLUSCARRIERRESTIND,
    CT2PLUSTARIFFRULERESTIND,
    CT2PLUSFARECLASSTYPERESTIND,
    CT2PLUSGENERALRULE,
    CT2PLUSGENERALRULEAPPL,
    EOEIND,
    EOECARRIERRESTIND,
    EOETARIFFRULERESTIND,
    EOEFARECLASSTYPERESTIND,
    EOEGENERALRULE,
    EOEGENERALRULEAPPL,
    ARBIND,
    ARBCARRIERRESTIND,
    ARBTARIFFRULERESTIND,
    ARBFARECLASSTYPERESTIND,
    ARBGENERALRULE,
    ARBGENERALRULEAPPL,
    ORDERNO,
    ITEMNO,
    RELATIONALIND,
    INOUTIND,
    DIRECTIONALITY,
    TEXTONLYIND,
    EOERVALUEIND,
    EOEALLSEGIND,
    SAMECARRIERIND,
    SAMERULETARIFFIND,
    SAMEFAREIND,
    ITEMCAT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select cat10.VENDOR,cat10.CARRIER,cat10.RULETARIFF,cat10.RULE,cat10.CATEGORY,"
                  " cat10.SEQNO,EXPIREDATE,EFFDATE,DISCDATE,cat10.CREATEDATE,SEGCNT,"
                  " JOINTCARRIERTBLITEMNO,SAMEPOINTSTBLITEMNO,DOJGENERALRULETARIFF,"
                  " CT2GENERALRULETARIFF,CT2PLUSGENERALRULETARIFF,EOEGENERALRULETARIFF,"
                  " ARBGENERALRULETARIFF,CREATORID,CREATORBUSINESSUNIT,VALIDITYIND,INHIBIT,"
                  " LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,ROUTING,"
                  " FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,SOJIND,"
                  " SOJORIGINDESTIND,DOJIND,DOJCARRIERRESTIND,DOJTARIFFRULERESTIND,"
                  " DOJFARECLASSTYPERESTIND,DOJGENERALRULE,DOJGENERALRULEAPPL,CT2IND,"
                  " CT2CARRIERRESTIND,CT2TARIFFRULERESTIND,CT2FARECLASSTYPERESTIND,"
                  " CT2GENERALRULE,CT2GENERALRULEAPPL,CT2PLUSIND,CT2PLUSCARRIERRESTIND,"
                  " CT2PLUSTARIFFRULERESTIND,CT2PLUSFARECLASSTYPERESTIND,CT2PLUSGENERALRULE,"
                  " CT2PLUSGENERALRULEAPPL,EOEIND,EOECARRIERRESTIND,EOETARIFFRULERESTIND,"
                  " EOEFARECLASSTYPERESTIND,EOEGENERALRULE,EOEGENERALRULEAPPL,ARBIND,"
                  " ARBCARRIERRESTIND,ARBTARIFFRULERESTIND,ARBFARECLASSTYPERESTIND,"
                  " ARBGENERALRULE,ARBGENERALRULEAPPL,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
                  " DIRECTIONALITY,TEXTONLYIND,EOERVALUEIND,EOEALLSEGIND,SAMECARRIERIND,"
                  " SAMERULETARIFFIND,SAMEFAREIND,ITEMCAT");

    //		        this->From("=COMBCATCNTL cat10 LEFT OUTER JOIN =CONDITIONALRULEEXPCC cat10s"
    //		                   "    USING (VENDOR, CARRIER, RULETARIFF, RULE, SEQNO, CREATEDATE) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(6);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=COMBCATCNTL",
                             "cat10",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPCC",
                             "cat10s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("cat10.VENDOR = %1q"
                " and cat10.CARRIER = %2q "
                " and cat10.RULETARIFF = %3n"
                " and cat10.RULE = %4q "
                " and cat10.CATEGORY = %5n "
                " and cat10.OVERRIDEIND = '' "
                " and %cd <= EXPIREDATE "
                " and INHIBIT = 'N'");
    this->OrderBy("SEQNO, CREATEDATE, ORDERNO ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static bool mapRowToCombinationRuleItem(const Row* row, tse::CombinabilityRuleItemInfo& combCtrlSeg)
  {
    if (UNLIKELY(row->isNull(ORDERNO)))
      return false;

    combCtrlSeg.setOrderNo(row->getInt(ORDERNO));
    combCtrlSeg.setItemNo(row->getInt(ITEMNO));

    const char relationalInd = row->getChar(RELATIONALIND);
    combCtrlSeg.setRelationalInd(logicalOpFromChar(relationalInd));

    combCtrlSeg.setInOutInd(row->getChar(INOUTIND));
    combCtrlSeg.setDirectionality(row->getChar(DIRECTIONALITY));
    combCtrlSeg.setTextonlyInd(row->getChar(TEXTONLYIND));
    combCtrlSeg.setEoervalueInd(row->getChar(EOERVALUEIND));
    combCtrlSeg.setEoeallsegInd(row->getChar(EOEALLSEGIND));
    combCtrlSeg.setSameCarrierInd(row->getChar(SAMECARRIERIND));
    combCtrlSeg.setSameRuleTariffInd(row->getChar(SAMERULETARIFFIND));
    combCtrlSeg.setSameFareInd(row->getChar(SAMEFAREIND));
    combCtrlSeg.setItemcat(row->getInt(ITEMCAT));
    return true;
  }

  ////
  static tse::CombinabilityRuleInfo* mapRowToCombination(Row* row,
                                                         CombinabilityRuleInfo* combCtrlPrev,
                                                         std::map<int, tse::ScoreSummary>& scoreSum,
                                                         uint16_t& dataSetNumber)
  {
    SequenceNumberLong seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    tse::CombinabilityRuleItemInfoSet* segSets = nullptr;

    // Parent not changed, just have a new child row
    if (combCtrlPrev != nullptr && seqNo == combCtrlPrev->sequenceNumber() &&
        createDate == combCtrlPrev->createDate())
    {
      tse::CombinabilityRuleItemInfo ctrlSeg;
      const bool created = mapRowToCombinationRuleItem(row, ctrlSeg);
      if (LIKELY(created))
      {
        if (ctrlSeg.relationalInd() == CategoryRuleItemInfo::THEN ||
            ctrlSeg.relationalInd() == CategoryRuleItemInfo::ELSE)
        {
          dataSetNumber++;
          segSets = tools::new_reserved<CombinabilityRuleItemInfoSet>(
              combCtrlPrev->segCnt());
          combCtrlPrev->addItemInfoSetNosync(segSets);
        }
        else
        {
          segSets = combCtrlPrev->categoryRuleItemInfoSet().back();
        }
        accumulateScore(&ctrlSeg, scoreSum, dataSetNumber);

        segSets->push_back(ctrlSeg);
      }
      return combCtrlPrev;
    }
    else
    {
      if (combCtrlPrev != nullptr)
      {
        mapScoreSumToComb(combCtrlPrev, scoreSum, dataSetNumber);
      }

      dataSetNumber = 1;

      tse::CombinabilityRuleInfo* combCtrl = new tse::CombinabilityRuleInfo;
      combCtrl->createDate() = row->getDate(CREATEDATE);
      combCtrl->expireDate() = row->getDate(EXPIREDATE);
      combCtrl->vendorCode() = row->getString(VENDOR);
      combCtrl->carrierCode() = row->getString(CARRIER);
      combCtrl->tariffNumber() = row->getInt(RULETARIFF);
      combCtrl->ruleNumber() = row->getString(RULE);
      combCtrl->categoryNumber() = row->getInt(CATEGORY);
      combCtrl->sequenceNumber() = row->getLong(SEQNO);
      combCtrl->effDate() = row->getDate(EFFDATE);
      combCtrl->discDate() = row->getDate(DISCDATE);
      combCtrl->segCnt() = row->getInt(SEGCNT);
      combCtrl->jointCarrierTblItemNo() = row->getInt(JOINTCARRIERTBLITEMNO);
      combCtrl->samepointstblItemNo() = row->getInt(SAMEPOINTSTBLITEMNO);
      combCtrl->dojGeneralRuleTariff() = row->getInt(DOJGENERALRULETARIFF);
      combCtrl->ct2GeneralRuleTariff() = row->getInt(CT2GENERALRULETARIFF);
      combCtrl->ct2plusGeneralRuleTariff() = row->getInt(CT2PLUSGENERALRULETARIFF);
      combCtrl->eoeGeneralRuleTariff() = row->getInt(EOEGENERALRULETARIFF);
      combCtrl->arbGeneralRuleTariff() = row->getInt(ARBGENERALRULETARIFF);
      combCtrl->validityInd() = row->getChar(VALIDITYIND);
      combCtrl->inhibit() = row->getChar(INHIBIT);
      combCtrl->loc1().locType() = row->getChar(LOC1TYPE);
      combCtrl->loc1().loc() = row->getString(LOC1);
      combCtrl->loc2().locType() = row->getChar(LOC2TYPE);
      combCtrl->loc2().loc() = row->getString(LOC2);
      combCtrl->locKey1().locType() = row->getChar(LOC1TYPE);
      combCtrl->locKey1().loc() = row->getString(LOC1);
      combCtrl->locKey2().locType() = row->getChar(LOC2TYPE);
      combCtrl->locKey2().loc() = row->getString(LOC2);
      combCtrl->fareClass() = row->getString(FARECLASS);
      combCtrl->owrt() = row->getChar(OWRT);
      combCtrl->routingAppl() = row->getChar(ROUTINGAPPL);
      combCtrl->routing() = row->getString(ROUTING);
      combCtrl->footNote1() = row->getString(FOOTNOTE1);
      combCtrl->footNote2() = row->getString(FOOTNOTE2);
      combCtrl->fareType() = row->getString(FARETYPE);
      combCtrl->seasonType() = row->getChar(SEASONTYPE);
      combCtrl->dowType() = row->getChar(DOWTYPE);
      combCtrl->applInd() = row->getChar(APPLIND);
      combCtrl->batchci() = row->getString(BATCHCI);
      combCtrl->sojInd() = row->getChar(SOJIND);
      combCtrl->sojorigIndestInd() = row->getChar(SOJORIGINDESTIND);
      combCtrl->dojInd() = row->getChar(DOJIND);
      combCtrl->dojCarrierRestInd() = row->getChar(DOJCARRIERRESTIND);
      combCtrl->dojTariffRuleRestInd() = row->getChar(DOJTARIFFRULERESTIND);
      combCtrl->dojFareClassTypeRestInd() = row->getChar(DOJFARECLASSTYPERESTIND);
      combCtrl->dojGeneralRule() = row->getString(DOJGENERALRULE);
      combCtrl->dojGeneralRuleAppl() = row->getChar(DOJGENERALRULEAPPL);
      combCtrl->ct2Ind() = row->getChar(CT2IND);
      combCtrl->ct2CarrierRestInd() = row->getChar(CT2CARRIERRESTIND);
      combCtrl->ct2TariffRuleRestInd() = row->getChar(CT2TARIFFRULERESTIND);
      combCtrl->ct2FareClassTypeRestInd() = row->getChar(CT2FARECLASSTYPERESTIND);
      combCtrl->ct2GeneralRule() = row->getString(CT2GENERALRULE);
      combCtrl->ct2GeneralRuleAppl() = row->getChar(CT2GENERALRULEAPPL);
      combCtrl->ct2plusInd() = row->getChar(CT2PLUSIND);
      combCtrl->ct2plusCarrierRestInd() = row->getChar(CT2PLUSCARRIERRESTIND);
      combCtrl->ct2plusTariffRuleRestInd() = row->getChar(CT2PLUSTARIFFRULERESTIND);
      combCtrl->ct2plusFareClassTypeRestInd() = row->getChar(CT2PLUSFARECLASSTYPERESTIND);
      combCtrl->ct2plusGeneralRule() = row->getString(CT2PLUSGENERALRULE);
      combCtrl->ct2plusGeneralRuleAppl() = row->getChar(CT2PLUSGENERALRULEAPPL);
      combCtrl->eoeInd() = row->getChar(EOEIND);
      combCtrl->eoeCarrierRestInd() = row->getChar(EOECARRIERRESTIND);
      combCtrl->eoeTariffRuleRestInd() = row->getChar(EOETARIFFRULERESTIND);
      combCtrl->eoeFareClassTypeRestInd() = row->getChar(EOEFARECLASSTYPERESTIND);
      combCtrl->eoeGeneralRule() = row->getString(EOEGENERALRULE);
      combCtrl->eoeGeneralRuleAppl() = row->getChar(EOEGENERALRULEAPPL);
      combCtrl->arbInd() = row->getChar(ARBIND);
      combCtrl->arbCarrierRestInd() = row->getChar(ARBCARRIERRESTIND);
      combCtrl->arbTariffRuleRestInd() = row->getChar(ARBTARIFFRULERESTIND);
      combCtrl->arbFareClassTypeRestInd() = row->getChar(ARBFARECLASSTYPERESTIND);
      combCtrl->arbGeneralRule() = row->getString(ARBGENERALRULE);
      combCtrl->arbGeneralRuleAppl() = row->getChar(ARBGENERALRULEAPPL);

      if (combCtrl->segCnt() > 0)
      {
        tse::CombinabilityRuleItemInfo ctrlSeg;
        const bool created = mapRowToCombinationRuleItem(row, ctrlSeg);
        if (LIKELY(created))
        {
          accumulateScore(&ctrlSeg, scoreSum, dataSetNumber);

          segSets = tools::new_reserved<CombinabilityRuleItemInfoSet>(
              combCtrl->segCnt());
          segSets->push_back(ctrlSeg);
          combCtrl->addItemInfoSetNosync(segSets);
        }
      }
      return combCtrl;
    } // else
  } // mapRowToCombination()

  //-------------------------------------------------------------------------
  // accumulateScore maintains a map of each major cat 10 subcategory
  // and its subcategory 'same' indicators.  When we encounter a major
  // subcategory we simply add an item to the map.
  // when we encounter a minor subcategory we check to see if the value
  // in the indicators has changed
  //-------------------------------------------------------------------------
  static void accumulateScore(const CombinabilityRuleItemInfo* ctrlSeg,
                              std::map<int, tse::ScoreSummary>& score,
                              uint16_t dataSetNumber)
  {
    // if major category then add item to map
    // if minor category then get all major categories and update their score
    int cat = ctrlSeg->itemcat();
    if (cat == 101 || cat == 102 || cat == 103 || cat == 104)
    {
      score[cat].majorSetCnt++; // will create a blank one if not exist
      score[cat].dataSetNumber = dataSetNumber;
    }
    else
    {
      scoreMajor(101, score, cat, ctrlSeg, dataSetNumber);
      scoreMajor(102, score, cat, ctrlSeg, dataSetNumber);
      scoreMajor(103, score, cat, ctrlSeg, dataSetNumber);
      scoreMajor(104, score, cat, ctrlSeg, dataSetNumber);
    }
  } // accumulateScore()

  //-----------------------------------------------------------------------
  // This function tallys each score summary for each of the subcategories
  // of cat 10 by examining the sub category values that pertain to each
  // major category (101-104) that we have encountered so far
  //-----------------------------------------------------------------------
  static void scoreMajor(int majorCat,
                         std::map<int, tse::ScoreSummary>& score,
                         int minorCat,
                         const CombinabilityRuleItemInfo* ctrlSeg,
                         uint16_t dataSetNumber)
  {
    if (score.count(majorCat) > 0 && score[majorCat].dataSetNumber == dataSetNumber)
    {
      switch (minorCat)
      {
      case 106:
        score[majorCat].setWith106Cnt++;
        score[majorCat].sameCarrierInd106 =
            setSameInd(dataSetNumber,
                       score[majorCat].sameCarrierInd106,
                       ctrlSeg->sameCarrierInd(),
                       score[majorCat].majorSetCnt == score[majorCat].setWith106Cnt);
        break;
      case 107:
        score[majorCat].setWith107Cnt++;
        score[majorCat].sameRuleTariffInd107 =
            setSameInd(dataSetNumber,
                       score[majorCat].sameRuleTariffInd107,
                       ctrlSeg->sameRuleTariffInd(),
                       score[majorCat].majorSetCnt == score[majorCat].setWith107Cnt);
        break;
      case 108:
        score[majorCat].setWith108Cnt++;
        score[majorCat].sameFareInd108 =
            setSameInd(dataSetNumber,
                       score[majorCat].sameFareInd108,
                       ctrlSeg->sameFareInd(),
                       score[majorCat].majorSetCnt == score[majorCat].setWith108Cnt);
        break;
      }
    }
  } // scoreMajor()

  //------------------------------------------------------------------------
  // setSameInd sets the initial value encountered for a scoreboard summary
  // (from 'X' to initial value).  If the value remains the same as each
  // previous occurence, it is unaltered.  If it changes it is set to ' '
  //  which indicates some difference in the rec 2 cat 10 data strings
  //------------------------------------------------------------------------
  static char
  setSameInd(uint16_t dataSetNumber, char scoreValue, char segValue, bool allSetHasThisMinor)
  {
    if (!allSetHasThisMinor)
    {
      scoreValue = ' ';
    }
    else if (scoreValue == 'X')
    {
      scoreValue = segValue; // set initial value
      /*
      if (dataSetNumber == 1)
      {
          scoreValue = segValue;   // set initial value
      }
      else
      {
          // a previous THEN or ELSE didn't have this subcat
          scoreValue = ' ';
      }
      */
    }
    else
    {
      if (scoreValue != segValue)
        // a subcat with a different value was already found
        scoreValue = ' ';
    }

    return scoreValue;
  } // setSameInd()

  static void applyMinorToMajor(std::map<int, tse::ScoreSummary>& score,
                                int majorCat,
                                char& sameCarrierInd,
                                char& sameRuleTariffInd,
                                char& sameFareInd)
  {
    sameCarrierInd = score[majorCat].sameCarrierInd106 == 'X' ||
                             score[majorCat].majorSetCnt != score[majorCat].setWith106Cnt
                         ? ' '
                         : score[majorCat].sameCarrierInd106;
    sameRuleTariffInd = score[majorCat].sameRuleTariffInd107 == 'X' ||
                                score[majorCat].majorSetCnt != score[majorCat].setWith107Cnt
                            ? ' '
                            : score[majorCat].sameRuleTariffInd107;
    sameFareInd = score[majorCat].sameFareInd108 == 'X' ||
                          score[majorCat].majorSetCnt != score[majorCat].setWith108Cnt
                      ? ' '
                      : score[majorCat].sameFareInd108;
  }

  //-----------------------------------------------------------------------
  // mapScoreSumToComb sets the values accumulated from the rec 2 cat 10
  // string tables to the rec2 cat 10 scoreboard (CombinabilityRuleInfo )
  // If we have occurences of the majors then we set the same indicators
  // based on what is in the accumulated map.
  //-----------------------------------------------------------------------
  static void mapScoreSumToComb(CombinabilityRuleInfo* comb,
                                std::map<int, tse::ScoreSummary>& score,
                                uint16_t dataSetNumber)
  {
    if (score.count(101) > 0)
    {
      applyMinorToMajor(score,
                        101,
                        comb->dojSameCarrierInd(),
                        comb->dojSameRuleTariffInd(),
                        comb->dojSameFareInd());
    }
    if (score.count(102) > 0)
    {
      applyMinorToMajor(score,
                        102,
                        comb->ct2SameCarrierInd(),
                        comb->ct2SameRuleTariffInd(),
                        comb->ct2SameFareInd());
    }
    if (score.count(103) > 0)
    {
      applyMinorToMajor(score,
                        103,
                        comb->ct2plusSameCarrierInd(),
                        comb->ct2plusSameRuleTariffInd(),
                        comb->ct2plusSameFareInd());
    }
    if (score.count(104) > 0)
    {
      applyMinorToMajor(score,
                        104,
                        comb->eoeSameCarrierInd(),
                        comb->eoeSameRuleTariffInd(),
                        comb->eoeSameFareInd());
    }

    score.clear();

  } // mapScoreSumToComb()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCombCtrlHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCombCtrlHistoricalSQLStatement : public QueryGetCombCtrlSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(6);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    partialStatement.Command(
        "("
        "select cat10h.VENDOR,cat10h.CARRIER,cat10h.RULETARIFF,cat10h.RULE,cat10h.CATEGORY,"
        " cat10h.SEQNO,EXPIREDATE,EFFDATE,DISCDATE,cat10h.CREATEDATE,SEGCNT,"
        " JOINTCARRIERTBLITEMNO,SAMEPOINTSTBLITEMNO,DOJGENERALRULETARIFF,"
        " CT2GENERALRULETARIFF,CT2PLUSGENERALRULETARIFF,EOEGENERALRULETARIFF,"
        " ARBGENERALRULETARIFF,CREATORID,CREATORBUSINESSUNIT,VALIDITYIND,INHIBIT,"
        " LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,ROUTING,"
        " FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,SOJIND,"
        " SOJORIGINDESTIND,DOJIND,DOJCARRIERRESTIND,DOJTARIFFRULERESTIND,"
        " DOJFARECLASSTYPERESTIND,DOJGENERALRULE,DOJGENERALRULEAPPL,CT2IND,"
        " CT2CARRIERRESTIND,CT2TARIFFRULERESTIND,CT2FARECLASSTYPERESTIND,"
        " CT2GENERALRULE,CT2GENERALRULEAPPL,CT2PLUSIND,CT2PLUSCARRIERRESTIND,"
        " CT2PLUSTARIFFRULERESTIND,CT2PLUSFARECLASSTYPERESTIND,CT2PLUSGENERALRULE,"
        " CT2PLUSGENERALRULEAPPL,EOEIND,EOECARRIERRESTIND,EOETARIFFRULERESTIND,"
        " EOEFARECLASSTYPERESTIND,EOEGENERALRULE,EOEGENERALRULEAPPL,ARBIND,"
        " ARBCARRIERRESTIND,ARBTARIFFRULERESTIND,ARBFARECLASSTYPERESTIND,"
        " ARBGENERALRULE,ARBGENERALRULEAPPL,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,TEXTONLYIND,EOERVALUEIND,EOEALLSEGIND,SAMECARRIERIND,"
        " SAMERULETARIFFIND,SAMEFAREIND,ITEMCAT");
    // partialStatement.From("=COMBCATCNTLH cat10h LEFT OUTER JOIN =CONDITIONALRULEEXPCCH cat10sh"
    //                      "   USING (VENDOR, CARRIER, RULETARIFF, RULE, SEQNO, CREATEDATE) ");
    this->generateJoinString(partialStatement,
                             "=COMBCATCNTLH",
                             "cat10h",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPCCH",
                             "cat10sh",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("cat10h.VENDOR = %1q"
                           " and cat10h.CARRIER = %2q "
                           " and cat10h.RULETARIFF = %3n"
                           " and cat10h.RULE = %4q "
                           " and cat10h.CATEGORY = %5n "
                           " and cat10h.OVERRIDEIND = '' "
                           " and %6n <= cat10h.EXPIREDATE"
                           " and %7n >= cat10h.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select cat10.VENDOR,cat10.CARRIER,cat10.RULETARIFF,cat10.RULE,cat10.CATEGORY,"
        " cat10.SEQNO,EXPIREDATE,EFFDATE,DISCDATE,cat10.CREATEDATE,SEGCNT,"
        " JOINTCARRIERTBLITEMNO,SAMEPOINTSTBLITEMNO,DOJGENERALRULETARIFF,"
        " CT2GENERALRULETARIFF,CT2PLUSGENERALRULETARIFF,EOEGENERALRULETARIFF,"
        " ARBGENERALRULETARIFF,CREATORID,CREATORBUSINESSUNIT,VALIDITYIND,INHIBIT,"
        " LOC1TYPE,LOC1,LOC2TYPE,LOC2,FARECLASS,OWRT,ROUTINGAPPL,ROUTING,"
        " FOOTNOTE1,FOOTNOTE2,FARETYPE,SEASONTYPE,DOWTYPE,APPLIND,BATCHCI,SOJIND,"
        " SOJORIGINDESTIND,DOJIND,DOJCARRIERRESTIND,DOJTARIFFRULERESTIND,"
        " DOJFARECLASSTYPERESTIND,DOJGENERALRULE,DOJGENERALRULEAPPL,CT2IND,"
        " CT2CARRIERRESTIND,CT2TARIFFRULERESTIND,CT2FARECLASSTYPERESTIND,"
        " CT2GENERALRULE,CT2GENERALRULEAPPL,CT2PLUSIND,CT2PLUSCARRIERRESTIND,"
        " CT2PLUSTARIFFRULERESTIND,CT2PLUSFARECLASSTYPERESTIND,CT2PLUSGENERALRULE,"
        " CT2PLUSGENERALRULEAPPL,EOEIND,EOECARRIERRESTIND,EOETARIFFRULERESTIND,"
        " EOEFARECLASSTYPERESTIND,EOEGENERALRULE,EOEGENERALRULEAPPL,ARBIND,"
        " ARBCARRIERRESTIND,ARBTARIFFRULERESTIND,ARBFARECLASSTYPERESTIND,"
        " ARBGENERALRULE,ARBGENERALRULEAPPL,ORDERNO,ITEMNO,RELATIONALIND,INOUTIND,"
        " DIRECTIONALITY,TEXTONLYIND,EOERVALUEIND,EOEALLSEGIND,SAMECARRIERIND,"
        " SAMERULETARIFFIND,SAMEFAREIND,ITEMCAT");
    // partialStatement.From("=COMBCATCNTL cat10 LEFT OUTER JOIN =CONDITIONALRULEEXPCC cat10s"
    //                      "   USING (VENDOR, CARRIER, RULETARIFF, RULE, SEQNO, CREATEDATE) ");
    this->generateJoinString(partialStatement,
                             "=COMBCATCNTL",
                             "cat10",
                             "LEFT OUTER JOIN",
                             "=CONDITIONALRULEEXPCC",
                             "cat10s",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("cat10.VENDOR = %8q"
                           " and cat10.CARRIER = %9q "
                           " and cat10.RULETARIFF = %10n"
                           " and cat10.RULE = %11q "
                           " and cat10.CATEGORY = %12n "
                           " and cat10.OVERRIDEIND = '' "
                           " and %13n <= cat10.EXPIREDATE"
                           " and %14n >= cat10.CREATEDATE"
                           ")");
    partialStatement.OrderBy("SEQNO, CREATEDATE desc, ORDERNO ");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->OrderBy("");
    this->Where("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetCombCtrlHistoricalSQLStatement
} // tse
