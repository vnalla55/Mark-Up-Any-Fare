//----------------------------------------------------------------------------
//  File:           QueryGetNonCombCatCtrl.h
//  Description:    QueryGetNonCombCatCtrl
//  Created:        3/2/2006
//  Authors:         Mike Lillis
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//  ----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetNonCombCatCtrl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNonCombCatCtrl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetNonCombCatCtrl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetNonCombCatCtrl() {};

  virtual const char* getQueryName() const override;

  void findGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           int ruleTariff,
                           const RuleNumber& ruleNo,
                           int cat);

  static void initialize();

  const QueryGetNonCombCatCtrl& operator=(const QueryGetNonCombCatCtrl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetNonCombCatCtrl& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNonCombCatCtrl

class QueryGetNonCombCatCtrlHistorical : public QueryGetNonCombCatCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNonCombCatCtrlHistorical(DBAdapter* dbAdapt)
    : QueryGetNonCombCatCtrl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetNonCombCatCtrlHistorical() {};
  virtual const char* getQueryName() const override;

  void findGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           int ruleTariff,
                           const RuleNumber& ruleNo,
                           int cat,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNonCombCatCtrlHistorical

class QueryGetNonCombCtrlBackDating : public QueryGetNonCombCatCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNonCombCtrlBackDating(DBAdapter* dbAdapt) : QueryGetNonCombCatCtrl(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;

  void findGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           int ruleTariff,
                           const RuleNumber& ruleNo,
                           int cat,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetNonCombCtrlBackDating

class QueryGetAllNonCombCatCtrl : public QueryGetNonCombCatCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllNonCombCatCtrl(DBAdapter* dbAdapt) : QueryGetNonCombCatCtrl(dbAdapt, _baseSQL) {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::GeneralFareRuleInfo*>& combs) { findAllGeneralFareRule(combs); }

  void findAllGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllNonCombCatCtrl

class QueryGetFBRCtrl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFBRCtrl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFBRCtrl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFBRCtrl() {};

  virtual const char* getQueryName() const override;

  void findFareByRuleCtrlInfo(std::vector<tse::FareByRuleCtrlInfo*>& fbrs,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              int ruleTariff,
                              const RuleNumber& ruleNo);

  static void initialize();

  const QueryGetFBRCtrl& operator=(const QueryGetFBRCtrl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFBRCtrl& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFBRCtrl

class QueryGetFBRCtrlHistorical : public QueryGetFBRCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFBRCtrlHistorical(DBAdapter* dbAdapt) : QueryGetFBRCtrl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFBRCtrlHistorical() {};
  virtual const char* getQueryName() const override;

  void findFareByRuleCtrlInfo(std::vector<tse::FareByRuleCtrlInfo*>& fbrs,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              int ruleTariff,
                              const RuleNumber& ruleNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFBRCtrlHist

class QueryGetFootNoteCtrl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFootNoteCtrl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFootNoteCtrl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFootNoteCtrl() {};

  virtual const char* getQueryName() const override;

  void findFootNoteCtrlInfo(std::vector<tse::FootNoteCtrlInfo*>& foots,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            int fareTariff,
                            const Footnote& footNote,
                            int cat);

  static void initialize();

  const QueryGetFootNoteCtrl& operator=(const QueryGetFootNoteCtrl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFootNoteCtrl& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFootNoteCtrl

class QueryGetFootNoteCtrlHistorical : public QueryGetFootNoteCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFootNoteCtrlHistorical(DBAdapter* dbAdapt) : QueryGetFootNoteCtrl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetFootNoteCtrlHistorical() {};
  virtual const char* getQueryName() const override;

  void findFootNoteCtrlInfo(std::vector<tse::FootNoteCtrlInfo*>& foots,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            int fareTariff,
                            const Footnote& footNote,
                            int cat,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFootNoteCtrlHistorical

class QueryGetAllFootNoteCtrl : public QueryGetFootNoteCtrl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFootNoteCtrl(DBAdapter* dbAdapt) : QueryGetFootNoteCtrl(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllFootNoteCtrl() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FootNoteCtrlInfo*>& foots) { findAllFootNoteCtrlInfo(foots); }

  void findAllFootNoteCtrlInfo(std::vector<tse::FootNoteCtrlInfo*>& foots);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFootNoteCtrl
} // namespace tse
