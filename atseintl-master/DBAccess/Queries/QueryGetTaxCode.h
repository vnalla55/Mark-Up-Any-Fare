//----------------------------------------------------------------------------
//          File:           QueryGetTaxCode.h
//          Description:    QueryGetTaxCode
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//      2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#ifndef QUERYGETTAXCODE_H
#define QUERYGETTAXCODE_H

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxCodeReg.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTaxCodeGenTextSeq : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCodeGenTextSeq(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxCodeGenTextSeq() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getCodeTextSeq(TaxCode txCd,
                      std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                      std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxCodeGenTextSeq& operator=(const QueryGetTaxCodeGenTextSeq& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxCodeGenTextSeq& operator=(const std::string& Another)
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
}; // class QueryGetTaxCodeGenTextSeq

class QueryGetTaxCodeGenTextSeqHistorical : public QueryGetTaxCodeGenTextSeq
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCodeGenTextSeqHistorical(DBAdapter* dbAdapt) : QueryGetTaxCodeGenTextSeq(dbAdapt) {}
  virtual ~QueryGetTaxCodeGenTextSeqHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getCodeTextSeq(TaxCode txCd,
                      std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                      std::vector<tse::TaxCodeReg*>& vecTCR,
                      const DateTime& startDate,
                      const DateTime& endDate);

  static void initialize();
  const QueryGetTaxCodeGenTextSeqHistorical&
  operator=(const QueryGetTaxCodeGenTextSeqHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxCodeGenTextSeqHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxCodeGenTextSeqHistorical

class QueryGetTaxCodeGenText : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCodeGenText(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxCodeGenText() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void findTaxCodeGenText(std::vector<tse::TaxCodeGenText*>& taxTexts, const TaxCode& taxCode);
  static void initialize();
  const QueryGetTaxCodeGenText& operator=(const QueryGetTaxCodeGenText& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxCodeGenText& operator=(const std::string& Another)
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
}; // class QueryGetTaxCodeGenText

class QueryGetTaxRestrValCxr : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrValCxr(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxRestrValCxr() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getRestrValCxrs(TaxCode txCd,
                       std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                       std::vector<tse::TaxCodeReg*>& vecTCR);
  static void initialize();
  const QueryGetTaxRestrValCxr& operator=(const QueryGetTaxRestrValCxr& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrValCxr& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrValCxr

class QueryGetTaxRestrPsgr : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrPsgr(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxRestrPsgr() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getRestrPsgrs(TaxCode txCd,
                     std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                     std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxRestrPsgr& operator=(const QueryGetTaxRestrPsgr& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrPsgr& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrPsgr

class QueryGetTaxRestrFareType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrFareType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxRestrFareType() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getRestrFareTypes(TaxCode txCd,
                         std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                         std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxRestrFareType& operator=(const QueryGetTaxRestrFareType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrFareType& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrFareType

class QueryGetTaxRestrFareClass : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrFareClass(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxRestrFareClass() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getRestrFareClasses(TaxCode txCd,
                           std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                           std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxRestrFareClass& operator=(const QueryGetTaxRestrFareClass& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrFareClass& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrFareClass

class QueryGetTaxExempEquip : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxExempEquip(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxExempEquip() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getExempEquips(TaxCode txCd,
                      std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                      std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxExempEquip& operator=(const QueryGetTaxExempEquip& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxExempEquip& operator=(const std::string& Another)
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
}; // class QueryGetTaxExempEquip

class QueryGetTaxExempCxr : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxExempCxr(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxExempCxr() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getExempCxrs(TaxCode txCd,
                    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                    std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxExempCxr& operator=(const QueryGetTaxExempCxr& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxExempCxr& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

  static int checkFlightWildCard(const char* fltStr);
  static int stringToInteger(const char* stringVal, int lineNumber);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxExempCxr

class QueryGetTaxOnTax : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxOnTax(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxOnTax() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getTaxOnTaxes(TaxCode txCd,
                     std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                     std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxOnTax& operator=(const QueryGetTaxOnTax& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxOnTax& operator=(const std::string& Another)
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
}; // class QueryGetTaxOnTax

class QueryGetTaxRestrTransit : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrTransit(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxRestrTransit() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getRestrTransits(TaxCode txCd,
                        std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                        std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxRestrTransit& operator=(const QueryGetTaxRestrTransit& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrTransit& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrTransit

class QueryGetTaxRestrTktDsg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrTktDsg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxRestrTktDsg() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getRestrTktDsgs(TaxCode txCd,
                       std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                       std::vector<tse::TaxCodeReg*>& vecTCR);

  static void initialize();
  const QueryGetTaxRestrTktDsg& operator=(const QueryGetTaxRestrTktDsg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrTktDsg& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrTktDsg

class QueryGetTaxCode : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxCode() {}

  virtual const char* getQueryName() const override;

  void findTaxCodeReg(std::vector<tse::TaxCodeReg*>& taxC, const TaxCode& code);

  static void initialize();
  const QueryGetTaxCode& operator=(const QueryGetTaxCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxCode& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

  static int checkFlightWildCard(const char* fltStr);
  static int stringToInteger(const char* stringVal, int lineNumber);
  void buildTaxCodeRegChildren(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxCode

class QueryGetAllTaxCodeRegs : public QueryGetTaxCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxCodeRegs(DBAdapter* dbAdapt) : QueryGetTaxCode(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg) { findAllTaxCodeReg(vecTaxCodeReg); }

  void findAllTaxCodeReg(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxCodeRegs

class QueryGetTaxCodeGenTextHistorical : public QueryGetTaxCodeGenText
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCodeGenTextHistorical(DBAdapter* dbAdapt) : QueryGetTaxCodeGenText(dbAdapt) {}
  virtual ~QueryGetTaxCodeGenTextHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void findTaxCodeGenText(std::vector<tse::TaxCodeGenText*>& taxTexts,
                          const TaxCode& taxCode,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();
  const QueryGetTaxCodeGenTextHistorical& operator=(const QueryGetTaxCodeGenTextHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxCodeGenTextHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxCodeGenTextHistorical

class QueryGetTaxRestrValCxrHistorical : public QueryGetTaxRestrValCxr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrValCxrHistorical(DBAdapter* dbAdapt) : QueryGetTaxRestrValCxr(dbAdapt) {}

  virtual ~QueryGetTaxRestrValCxrHistorical() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getRestrValCxrs(TaxCode txCd,
                       std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                       std::vector<tse::TaxCodeReg*>& vecTCR,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();
  const QueryGetTaxRestrValCxrHistorical& operator=(const QueryGetTaxRestrValCxrHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrValCxrHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrValCxrHistorical

class QueryGetTaxRestrPsgrHistorical : public QueryGetTaxRestrPsgr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrPsgrHistorical(DBAdapter* dbAdapt) : QueryGetTaxRestrPsgr(dbAdapt) {}
  virtual ~QueryGetTaxRestrPsgrHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getRestrPsgrs(TaxCode txCd,
                     std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                     std::vector<tse::TaxCodeReg*>& vecTCR,
                     const DateTime& startDate,
                     const DateTime& endDate);

  static void initialize();
  const QueryGetTaxRestrPsgrHistorical& operator=(const QueryGetTaxRestrPsgrHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrPsgrHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrPsgrHistorical

class QueryGetTaxRestrFareTypeHistorical : public QueryGetTaxRestrFareType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrFareTypeHistorical(DBAdapter* dbAdapt) : QueryGetTaxRestrFareType(dbAdapt) {}

  virtual ~QueryGetTaxRestrFareTypeHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getRestrFareTypes(TaxCode txCd,
                         std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                         std::vector<tse::TaxCodeReg*>& vecTCR,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();
  const QueryGetTaxRestrFareTypeHistorical&
  operator=(const QueryGetTaxRestrFareTypeHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrFareTypeHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrFareTypeHistorical

class QueryGetTaxRestrFareClassHistorical : public QueryGetTaxRestrFareClass
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrFareClassHistorical(DBAdapter* dbAdapt) : QueryGetTaxRestrFareClass(dbAdapt) {}

  virtual ~QueryGetTaxRestrFareClassHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getRestrFareClasses(TaxCode txCd,
                           std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                           std::vector<tse::TaxCodeReg*>& vecTCR,
                           const DateTime& startDate,
                           const DateTime& endDate);

  static void initialize();
  const QueryGetTaxRestrFareClassHistorical&
  operator=(const QueryGetTaxRestrFareClassHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrFareClassHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrFareClassHistorical

class QueryGetTaxExempEquipHistorical : public QueryGetTaxExempEquip
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxExempEquipHistorical(DBAdapter* dbAdapt) : QueryGetTaxExempEquip(dbAdapt) {}

  virtual ~QueryGetTaxExempEquipHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getExempEquips(TaxCode txCd,
                      std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                      std::vector<tse::TaxCodeReg*>& vecTCR,
                      const DateTime& startDate,
                      const DateTime& endDate);

  static void initialize();
  const QueryGetTaxExempEquipHistorical& operator=(const QueryGetTaxExempEquipHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxExempEquipHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxExempEquipHistorical

class QueryGetTaxExempCxrHistorical : public QueryGetTaxExempCxr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxExempCxrHistorical(DBAdapter* dbAdapt) : QueryGetTaxExempCxr(dbAdapt) {}

  virtual ~QueryGetTaxExempCxrHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getExempCxrs(TaxCode txCd,
                    std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                    std::vector<tse::TaxCodeReg*>& vecTCR,
                    const DateTime& startDate,
                    const DateTime& endDate);

  static void initialize();
  const QueryGetTaxExempCxrHistorical& operator=(const QueryGetTaxExempCxrHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxExempCxrHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxExempCxrHistorical

class QueryGetTaxOnTaxHistorical : public QueryGetTaxOnTax
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxOnTaxHistorical(DBAdapter* dbAdapt) : QueryGetTaxOnTax(dbAdapt) {}

  virtual ~QueryGetTaxOnTaxHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getTaxOnTaxes(TaxCode txCd,
                     std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                     std::vector<tse::TaxCodeReg*>& vecTCR,
                     const DateTime& startDate,
                     const DateTime& endDate);

  static void initialize();
  const QueryGetTaxOnTaxHistorical& operator=(const QueryGetTaxOnTaxHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxOnTaxHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxOnTaxHistorical

class QueryGetTaxRestrTransitHistorical : public QueryGetTaxRestrTransit
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrTransitHistorical(DBAdapter* dbAdapt) : QueryGetTaxRestrTransit(dbAdapt) {}

  virtual ~QueryGetTaxRestrTransitHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getRestrTransits(TaxCode txCd,
                        std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                        std::vector<tse::TaxCodeReg*>& vecTCR,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();
  const QueryGetTaxRestrTransitHistorical&
  operator=(const QueryGetTaxRestrTransitHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrTransitHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrTransitHistorical

class QueryGetTaxRestrTktDsgHistorical : public QueryGetTaxRestrTktDsg
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxRestrTktDsgHistorical(DBAdapter* dbAdapt) : QueryGetTaxRestrTktDsg(dbAdapt) {}
  virtual ~QueryGetTaxRestrTktDsgHistorical() {}
  void resetSQL() { *this = _baseSQL; }

  const char* getQueryName() const override;

  void getRestrTktDsgs(TaxCode txCd,
                       std::vector<tse::TaxCodeReg*>::iterator& itTCR,
                       std::vector<tse::TaxCodeReg*>& vecTCR,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();
  const QueryGetTaxRestrTktDsgHistorical& operator=(const QueryGetTaxRestrTktDsgHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxRestrTktDsgHistorical& operator=(const std::string& Another)
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
}; // class QueryGetTaxRestrTktDsgHistorical

class QueryGetTaxCodeHistorical : public QueryGetTaxCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxCodeHistorical(DBAdapter* dbAdapt) : QueryGetTaxCode(dbAdapt, _baseSQL) {}
  QueryGetTaxCodeHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetTaxCode(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxCodeHistorical() {}

  virtual const char* getQueryName() const override;

  void findTaxCodeRegHistorical(std::vector<tse::TaxCodeReg*>& taxC,
                                const TaxCode& code,
                                const DateTime& startDate,
                                const DateTime& endDate);

  static void initialize();

  void buildTaxCodeRegChildren(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg,
                               const DateTime& startDate,
                               const DateTime& endDate);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxCodeHistorical

class QueryGetAllTaxCodeRegsHistorical : public QueryGetTaxCodeHistorical
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxCodeRegsHistorical(DBAdapter* dbAdapt)
    : QueryGetTaxCodeHistorical(dbAdapt, _baseSQL)
  {
  }

  virtual const char* getQueryName() const override;

  void findAllTaxCodeReg(std::vector<tse::TaxCodeReg*>& vecTaxCodeReg,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxCodeRegsHistorical

} // namespace tse

#endif // QUERYGETTAXCODE_H
