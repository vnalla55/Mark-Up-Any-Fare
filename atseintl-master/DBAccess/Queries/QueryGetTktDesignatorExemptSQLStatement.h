//----------------------------------------------------------------------------
//          File:           QueryGetTktDesignatorExempt.h
//          Description:    QueryGetTktDesignatorExempt
//          Created:        12/12/2013
//
//     (C) 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTktDesignatorExempt.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetTktDesignatorExemptSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    CARRIER,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    VALIDITYIND,
    DESCRIPTION,
    EXEMPTIONTYPE,
    TKTDESIG,
    PSGRTYPE,
    TAXEXEMPT,
    PFCEXEMPT,
    RULETARIFF,
    RULENBR1,
    RULERELATIONAL,
    RULENBR2,
    DIR,
    LOCTYPE1,
    LOC1,
    LOCTYPE2,
    LOC2
    // end of TKTDESIGNATOREXEMPT
    ,
    CARRIERTAXA,
    VERSIONDATETAXA,
    SEQNOTAXA,
    CREATEDATETAXA,
    TAXCODE,
    TAXNATION
    // end of TKTDESIGNATOREXEMPTTAXA
    ,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.CARRIER, p.VERSIONDATE, p.SEQNO, p.CREATEDATE, EFFDATE, DISCDATE, EXPIREDATE,"
            "VALIDITYIND, DESCRIPTION, EXEMPTIONTYPE, TKTDESIG, PSGRTYPE, TAXEXEMPT,"
            "PFCEXEMPT, RULETARIFF, RULENBR1, RULERELATIONAL, RULENBR2, DIR,"
            "LOCTYPE1, LOC1, LOCTYPE2, LOC2, taxa.CARRIER, taxa.VERSIONDATE, taxa.SEQNO, "
            "taxa.CREATEDATE,"
            "TAXCODE, TAXNATION");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("CARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    std::string from;
    this->generateJoinString("=TKTDESIGNATOREXEMPT",
                             "p",
                             "LEFT OUTER JOIN",
                             "=TKTDESIGNATOREXEMPTTAXA",
                             "taxa",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.CARRIER = %1q"
                " and %cd <= EXPIREDATE"
                " and VALIDITYIND = 'Y'");
    OrderBy("p.VERSIONDATE, p.SEQNO, p.CREATEDATE");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TktDesignatorExemptInfo* mapRow(Row* row, TktDesignatorExemptInfo* infoPrev)
  {
    // get the OrderBy fields to determine new parent
    CarrierCode carrier(row->getString(CARRIER));
    DateTime versionDate(row->getDate(VERSIONDATE));
    SequenceNumberLong sequenceNumber(row->getLong(SEQNO));
    DateTime createDate(row->getDate(CREATEDATE));

    TktDesignatorExemptInfo* info(nullptr);
    if (infoPrev && carrier == infoPrev->carrier() && versionDate == infoPrev->versionDate() &&
        sequenceNumber == infoPrev->sequenceNumber() && createDate == infoPrev->createDate())
    {
      info = infoPrev;
    }
    else
    {
      info = new TktDesignatorExemptInfo;
      info->carrier() = carrier;
      info->versionDate() = versionDate;
      info->sequenceNumber() = sequenceNumber;
      info->createDate() = createDate;
      info->effDate() = row->getDate(EFFDATE);
      info->discDate() = row->getDate(DISCDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
      if (!row->isNull(VALIDITYIND))
      {
        info->validityInd() = row->getChar(VALIDITYIND);
      }
      info->description() = row->getString(DESCRIPTION);
      if (!row->isNull(EXEMPTIONTYPE))
      {
        info->exemptionType() = row->getChar(EXEMPTIONTYPE);
      }
      info->ticketDesignator() = row->getString(TKTDESIG);
      info->paxType() = row->getString(PSGRTYPE);
      if (!row->isNull(TAXEXEMPT))
      {
        info->taxExempt() = row->getChar(TAXEXEMPT);
      }
      if (!row->isNull(PFCEXEMPT))
      {
        info->pfcExempt() = row->getChar(PFCEXEMPT);
      }
      if (!row->isNull(RULETARIFF))
      {
        info->ruleTariff() = row->getInt(RULETARIFF);
      }
      info->ruleNumber1() = row->getString(RULENBR1);
      if (!row->isNull(RULERELATIONAL))
      {
        info->ruleRelational() = row->getChar(RULERELATIONAL);
      }
      info->ruleNumber2() = row->getString(RULENBR2);
      if (!row->isNull(DIR))
      {
        info->directionality() = static_cast<Directionality>(row->getInt(DIR));
      }
      if (!row->isNull(LOCTYPE1))
      {
        info->locType1() = row->getInt(LOCTYPE1);
      }
      info->loc1() = row->getString(LOC1);
      if (!row->isNull(LOCTYPE2))
      {
        info->locType2() = row->getInt(LOCTYPE2);
      }
      info->loc2() = row->getString(LOC2);
    }
    TaxCode taxCode(row->getString(TAXCODE));
    NationCode taxNation(row->getString(TAXNATION));
    if (!taxCode.empty() || !taxNation.empty())
    {
      TktDesignatorExemptTaxAInfo* child(new TktDesignatorExemptTaxAInfo);
      child->taxCode() = taxCode;
      child->taxNation() = taxNation;
      info->tktDesignatorExemptTaxA().push_back(child);
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetTktDesignatorExemptHistoricalSQLStatement
    : public QueryGetTktDesignatorExemptSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string from;
    std::vector<std::string> joinFields;

    joinFields.push_back("CARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    partialStatement.Command(
        "("
        "select ph.CARRIER, ph.VERSIONDATE, ph.SEQNO, ph.CREATEDATE, EFFDATE, DISCDATE, EXPIREDATE,"
        "VALIDITYIND, DESCRIPTION, EXEMPTIONTYPE, TKTDESIG, PSGRTYPE, TAXEXEMPT,"
        "PFCEXEMPT, RULETARIFF, RULENBR1, RULERELATIONAL, RULENBR2, DIR,"
        "LOCTYPE1, LOC1, LOCTYPE2, LOC2, ch.CARRIER, ch.VERSIONDATE, ch.SEQNO, ch.CREATEDATE,"
        "TAXCODE, TAXNATION");
    this->generateJoinString(partialStatement,
                             "=TKTDESIGNATOREXEMPTH",
                             "ph",
                             "LEFT OUTER JOIN",
                             "=TKTDESIGNATOREXEMPTTAXAH",
                             "ch",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("ph.CARRIER = %1q"
                           " and VALIDITYIND = 'Y'"
                           " and %2n <= EXPIREDATE"
                           " and %3n >= ph.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select p.CARRIER, p.VERSIONDATE, p.SEQNO, p.CREATEDATE, EFFDATE, DISCDATE, EXPIREDATE,"
        "VALIDITYIND, DESCRIPTION, EXEMPTIONTYPE, TKTDESIG, PSGRTYPE, TAXEXEMPT,"
        "PFCEXEMPT, RULETARIFF, RULENBR1, RULERELATIONAL, RULENBR2, DIR,"
        "LOCTYPE1, LOC1, LOCTYPE2, LOC2, c.CARRIER, c.VERSIONDATE, c.SEQNO, c.CREATEDATE,"
        "TAXCODE, TAXNATION");
    this->generateJoinString(partialStatement,
                             "=TKTDESIGNATOREXEMPT",
                             "p",
                             "LEFT OUTER JOIN",
                             "=TKTDESIGNATOREXEMPTTAXA",
                             "c",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("p.CARRIER = %4q"
                           " and %cd <= EXPIREDATE"
                           " and VALIDITYIND = 'Y'"
                           ")");
    partialStatement.OrderBy("");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("1,2,3,4");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllTktDesignatorExemptSQLStatement
    : public QueryGetTktDesignatorExemptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= p.CREATEDATE"
                " and %cd <= EXPIREDATE"
                " and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("p.CARRIER, p.VERSIONDATE, p.SEQNO, p.CREATEDATE");
    else
      this->OrderBy("");
  }
};

} // tse

