#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class SpanishReferenceFareInfo;

class QueryGetSpanishReferenceFare : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSpanishReferenceFare (DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetSpanishReferenceFare (DBAdapter* dbAdapt, const std::string& sqlStatement)
      : SQLQuery(dbAdapt, sqlStatement) {};

  virtual const char* getQueryName() const override;

  void
  findSpanishReferenceFare (std::vector<SpanishReferenceFareInfo*>& lst,
                            const CarrierCode& tktCarrier,
                            const CarrierCode& fareCarrier,
                            const LocCode& sourceLoc, const LocCode& destLoc);

  static void initialize();

  const QueryGetSpanishReferenceFare& operator=(const QueryGetSpanishReferenceFare& Another) = delete;

  const QueryGetSpanishReferenceFare& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetSpanishReferenceFareHistorical : public QueryGetSpanishReferenceFare
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSpanishReferenceFareHistorical(DBAdapter* dbAdapt) :
      QueryGetSpanishReferenceFare(dbAdapt, _baseSQL)
  {}
  virtual const char* getQueryName() const override;

  void
  findSpanishReferenceFare(std::vector<SpanishReferenceFareInfo*>& lst,
                           const CarrierCode& tktCarrier, const CarrierCode& fareCarrier,
                           const LocCode& sourceLoc, const LocCode& destLoc,
                           const DateTime& startDate, const DateTime& endDate);

  static void initialize();

private:
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse


