#ifndef QUERYGETTAXCODE_H
#define QUERYGETTAXCODE_H

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxSpecConfigReg.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxSpecConfig : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxSpecConfig(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxSpecConfig(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxSpecConfig() {}

  virtual const char* getQueryName() const override;

  void
  findTaxSpecConfigReg(std::vector<tse::TaxSpecConfigReg*>& taxC, const TaxSpecConfigName& name);

  static void initialize();
  const QueryGetTaxSpecConfig& operator=(const QueryGetTaxSpecConfig& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxSpecConfig& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxSpecConfig

class QueryGetAllTaxSpecConfigRegs : public QueryGetTaxSpecConfig
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxSpecConfigRegs(DBAdapter* dbAdapt) : QueryGetTaxSpecConfig(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::TaxSpecConfigReg*>& vecTaxSpecConfigReg)
  {
    findAllTaxSpecConfigReg(vecTaxSpecConfigReg);
  }

  void findAllTaxSpecConfigReg(std::vector<tse::TaxSpecConfigReg*>& vecTaxSpecConfigReg);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxSpecConfigRegs

class QueryGetTaxSpecConfigHistorical : public QueryGetTaxSpecConfig
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxSpecConfigHistorical(DBAdapter* dbAdapt) : QueryGetTaxSpecConfig(dbAdapt, _baseSQL) {}
  QueryGetTaxSpecConfigHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetTaxSpecConfig(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxSpecConfigHistorical() {}

  virtual const char* getQueryName() const override;

  void findTaxSpecConfigRegHistorical(std::vector<tse::TaxSpecConfigReg*>& taxC,
                                      const TaxSpecConfigName& name,
                                      const DateTime& startDate,
                                      const DateTime& endDate);

  static void initialize();

  //    void buildTaxSpecConfigRegChildren(std::vector<tse::TaxSpecConfigReg *>
  // &vecTaxSpecConfigReg,
  //                                 const DateTime &startDate, const DateTime &endDate);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxSpecConfigHistorical

} // namespace tse

#endif // QUERYGETTAXCODE_H
