#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxSpecConfig.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetTaxSpecConfigSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxSpecConfigSQLStatement() {}
  virtual ~QueryGetTaxSpecConfigSQLStatement() {}

  enum ColumnIndexes
  { SPECCONFIGNAME = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DESCRIPTION,
    SEQNO,
    PARAMNAME,
    PARAMVALUE,
    NUMBEROFCOLUMNS }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select c.SPECCONFIGNAME, c.CREATEDATE, c.EXPIREDATE, c.EFFDATE,"
                  " c.DISCDATE, c.DESCRIPTION, s.SEQNO, s.PARAMNAME, s.PARAMVALUE");

    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(2);
    joinFields.push_back("SPECCONFIGNAME");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=TAXSPECCONFIG", "c", "INNER JOIN", "=TAXSPECCONFIGSEQ", "s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where(" c.SPECCONFIGNAME = %1q"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("c.SPECCONFIGNAME, s.SEQNO, c.CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxSpecConfigReg* mapRowToTaxSpecConfigReg(Row* row, tse::TaxSpecConfigReg* prev)
  {
    TaxSpecConfigName name = row->getString(SPECCONFIGNAME);
    DateTime createDate = row->getDate(CREATEDATE);
    DateTime expireDate = row->getDate(EXPIREDATE);
    DateTime discDate = row->getDate(DISCDATE);

    tse::TaxSpecConfigReg* tcr;

    if (prev != nullptr && name == prev->taxSpecConfigName() && createDate == prev->createDate() &&
        expireDate == prev->expireDate() && discDate == prev->discDate())
    {
      tcr = prev;
    }
    else
    {
      tcr = new tse::TaxSpecConfigReg;
      tcr->taxSpecConfigName() = row->getString(SPECCONFIGNAME);
      tcr->createDate() = row->getDate(CREATEDATE);
      tcr->expireDate() = row->getDate(EXPIREDATE);
      tcr->effDate() = row->getDate(EFFDATE);
      tcr->discDate() = row->getDate(DISCDATE);
      tcr->setDescription(row->getString(DESCRIPTION));
    }

    tse::TaxSpecConfigReg::TaxSpecConfigRegSeq* tcrs =
        new tse::TaxSpecConfigReg::TaxSpecConfigRegSeq();

    tcrs->seqNo() = row->getLong(SEQNO);
    tcrs->paramName() = row->getString(PARAMNAME);
    tcrs->paramValue() = row->getString(PARAMVALUE);

    tcr->seqs().push_back(tcrs);

    return tcr;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetAllTaxSpecConfigRegsSQLStatement
    : public QueryGetTaxSpecConfigSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};

template <class QUERYCLASS>
class QueryGetTaxSpecConfigHistoricalSQLStatement
    : public QueryGetTaxSpecConfigSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" c.SPECCONFIGNAME = %1q"
                " and %2n <= c.EXPIREDATE"
                " and %3n >= c.CREATEDATE");
  }
};
}
