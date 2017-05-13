//----------------------------------------------------------------------------
//  � 2013, Sabre Inc. All rights reserved. This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
class SectorDetailInfo;

class QueryGetSectorDetail : public SQLQuery
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetSectorDetail(DBAdapter* dbAdapt);

  QueryGetSectorDetail(DBAdapter* dbAdapt, const std::string& sqlStatement);

  virtual ~QueryGetSectorDetail();

  virtual const char* getQueryName() const override;

  void findSectorDetailInfo(std::vector<const SectorDetailInfo*>& data,
                            const VendorCode& vendor,
                            int itemNumber);

  static void initialize();

  const QueryGetSectorDetail& operator=(const QueryGetSectorDetail& another);

  const QueryGetSectorDetail& operator=(const std::string& another);

private:
  static void deinitialize() { _isInitialized = false; }

  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;

}; // class QueryGetSectorDetail

class QueryGetSectorDetailHistorical : public QueryGetSectorDetail
{
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;

public:
  QueryGetSectorDetailHistorical(DBAdapter* dbAdapt);
  virtual ~QueryGetSectorDetailHistorical();

  virtual const char* getQueryName() const override;

  void findSectorDetailInfo(std::vector<const SectorDetailInfo*>& data,
                            const VendorCode& vendor,
                            int itemNumber,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static void deinitialize() { _isInitialized = false; }

  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSectorDetailHistorical

} // namespace tse

