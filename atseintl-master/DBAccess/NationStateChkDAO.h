//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseWrappers.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class NationStateHistIsCurrChk;
class DeleteList;

////////////////////////////////// Nation Checks //////////////////////////////////
/////// Current ///////
typedef HashKey<NationCode, LocCode> NationInAreaKey;
typedef BoolWrapper NationInAreaInfo;
class NationInAreaDAO : public DataAccessObject<NationInAreaKey, NationInAreaInfo>
{
public:
  static NationInAreaDAO& instance();

  bool isNationInArea(const NationCode& nation, const LocCode& area);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationInAreaDAO>;
  static DAOHelper<NationInAreaDAO> _helper;
  NationInAreaDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationInAreaKey, NationInAreaInfo>(cacheSize, cacheType)
  {
  }
  NationInAreaInfo* create(NationInAreaKey key) override;
  void destroy(NationInAreaKey key, NationInAreaInfo* rec) override;

private:
  static NationInAreaDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NationInAreaDAO

/////// Historical ///////
typedef HashKey<NationCode, LocCode, DateTime, DateTime> NationInAreaHistoricalKey;
class NationInAreaHistoricalDAO
    : public HistoricalDataAccessObject<NationInAreaHistoricalKey,
                                        std::vector<NationStateHistIsCurrChk*> >
{
public:
  static NationInAreaHistoricalDAO& instance();

  bool isNationInArea(const NationCode& nation,
                      const LocCode& area,
                      DeleteList& del,
                      const DateTime& tktDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationInAreaHistoricalDAO>;
  static DAOHelper<NationInAreaHistoricalDAO> _helper;
  NationInAreaHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NationInAreaHistoricalKey,
                                 std::vector<NationStateHistIsCurrChk*> >(cacheSize, cacheType)
  {
  }
  std::vector<NationStateHistIsCurrChk*>* create(NationInAreaHistoricalKey key) override;
  void
  destroy(NationInAreaHistoricalKey key, std::vector<NationStateHistIsCurrChk*>* recs) override;

private:
  static NationInAreaHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NationInAreaHistoricalDAO

/////// Current ///////
typedef HashKey<NationCode, LocCode> NationInSubAreaKey;
class NationInSubAreaDAO : public DataAccessObject<NationInSubAreaKey, NationInAreaInfo>
{
public:
  static NationInSubAreaDAO& instance();

  bool isNationInSubArea(const NationCode& nation, const LocCode& subArea);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationInSubAreaDAO>;
  static DAOHelper<NationInSubAreaDAO> _helper;
  NationInSubAreaDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationInSubAreaKey, NationInAreaInfo>(cacheSize, cacheType)
  {
  }
  NationInAreaInfo* create(NationInSubAreaKey key) override;
  void destroy(NationInSubAreaKey key, NationInAreaInfo* rec) override;

private:
  static NationInSubAreaDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NationInSubAreaDAO

/////// Historical ///////
typedef HashKey<NationCode, LocCode, DateTime, DateTime> NationInSubAreaHistoricalKey;
class NationInSubAreaHistoricalDAO
    : public HistoricalDataAccessObject<NationInSubAreaHistoricalKey,
                                        std::vector<NationStateHistIsCurrChk*> >
{
public:
  static NationInSubAreaHistoricalDAO& instance();

  bool isNationInSubArea(const NationCode& nation,
                         const LocCode& subArea,
                         DeleteList& del,
                         const DateTime& tktDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationInSubAreaHistoricalDAO>;
  static DAOHelper<NationInSubAreaHistoricalDAO> _helper;
  NationInSubAreaHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NationInSubAreaHistoricalKey,
                                 std::vector<NationStateHistIsCurrChk*> >(cacheSize, cacheType)
  {
  }
  std::vector<NationStateHistIsCurrChk*>* create(NationInSubAreaHistoricalKey key) override;
  void
  destroy(NationInSubAreaHistoricalKey key, std::vector<NationStateHistIsCurrChk*>* recs) override;

private:
  static NationInSubAreaHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NationInSubAreaHistoricalDAO

/////// Current ///////
typedef HashKey<VendorCode, int, char, NationCode> NationInZoneKey;
class NationInZoneDAO : public DataAccessObject<NationInZoneKey, NationInAreaInfo>
{
public:
  static NationInZoneDAO& instance();

  bool isNationInZone(const VendorCode& vendor,
                      int zone,
                      const ZoneType zoneType,
                      const NationCode& nation);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationInZoneDAO>;
  static DAOHelper<NationInZoneDAO> _helper;
  NationInZoneDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationInZoneKey, NationInAreaInfo>(cacheSize, cacheType)
  {
  }
  NationInAreaInfo* create(NationInZoneKey key) override;
  void destroy(NationInZoneKey key, NationInAreaInfo* rec) override;

private:
  static NationInZoneDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NationInZoneDAO

/////// Historical ///////
typedef HashKey<VendorCode, int, char, NationCode, DateTime, DateTime> NationInZoneHistoricalKey;
class NationInZoneHistoricalDAO
    : public HistoricalDataAccessObject<NationInZoneHistoricalKey,
                                        std::vector<NationStateHistIsCurrChk*> >
{
public:
  static NationInZoneHistoricalDAO& instance();

  bool isNationInZone(const VendorCode& vendor,
                      int zone,
                      const ZoneType zoneType,
                      const NationCode& nation,
                      DeleteList& del,
                      const DateTime& tktDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NationInZoneHistoricalDAO>;
  static DAOHelper<NationInZoneHistoricalDAO> _helper;
  NationInZoneHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NationInZoneHistoricalKey,
                                 std::vector<NationStateHistIsCurrChk*> >(cacheSize, cacheType)
  {
  }
  std::vector<NationStateHistIsCurrChk*>* create(NationInZoneHistoricalKey key) override;
  void
  destroy(NationInZoneHistoricalKey key, std::vector<NationStateHistIsCurrChk*>* recs) override;

private:
  static NationInZoneHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NationInZoneHistDAO

////////////////////////////////// State Checks //////////////////////////////////
/////// Current ///////
typedef HashKey<NationCode, StateCode, LocCode> StateInAreaKey;
class StateInAreaDAO : public DataAccessObject<StateInAreaKey, NationInAreaInfo>
{
public:
  static StateInAreaDAO& instance();

  bool isStateInArea(const NationCode& nation, const StateCode& state, const LocCode& area);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StateInAreaDAO>;
  static DAOHelper<StateInAreaDAO> _helper;
  StateInAreaDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<StateInAreaKey, NationInAreaInfo>(cacheSize, cacheType)
  {
  }
  NationInAreaInfo* create(StateInAreaKey key) override;
  void destroy(StateInAreaKey key, NationInAreaInfo* rec) override;

private:
  static StateInAreaDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateInAreaDAO

/////// Historical ///////
typedef HashKey<NationCode, StateCode, LocCode, DateTime, DateTime> StateInAreaHistoricalKey;
class StateInAreaHistoricalDAO
    : public HistoricalDataAccessObject<StateInAreaHistoricalKey,
                                        std::vector<NationStateHistIsCurrChk*> >
{
public:
  static StateInAreaHistoricalDAO& instance();

  bool isStateInArea(const NationCode& nation,
                     const StateCode& state,
                     const LocCode& area,
                     DeleteList& del,
                     const DateTime& tktDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StateInAreaHistoricalDAO>;
  static DAOHelper<StateInAreaHistoricalDAO> _helper;
  StateInAreaHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<StateInAreaHistoricalKey, std::vector<NationStateHistIsCurrChk*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<NationStateHistIsCurrChk*>* create(StateInAreaHistoricalKey key) override;
  void destroy(StateInAreaHistoricalKey key, std::vector<NationStateHistIsCurrChk*>* recs) override;

private:
  static StateInAreaHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateInAreaHistoricalDAO

/////// Current ///////
typedef HashKey<NationCode, StateCode, LocCode> StateInSubAreaKey;
class StateInSubAreaDAO : public DataAccessObject<StateInSubAreaKey, NationInAreaInfo>
{
public:
  static StateInSubAreaDAO& instance();

  bool isStateInSubArea(const NationCode& nation, const StateCode& state, const LocCode& subArea);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StateInSubAreaDAO>;
  static DAOHelper<StateInSubAreaDAO> _helper;
  StateInSubAreaDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<StateInSubAreaKey, NationInAreaInfo>(cacheSize, cacheType)
  {
  }
  NationInAreaInfo* create(StateInSubAreaKey key) override;
  void destroy(StateInSubAreaKey key, NationInAreaInfo* rec) override;

private:
  static StateInSubAreaDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateInSubAreaDAO

/////// Historical ///////
typedef HashKey<NationCode, StateCode, LocCode, DateTime, DateTime> StateInSubAreaHistoricalKey;
class StateInSubAreaHistoricalDAO
    : public HistoricalDataAccessObject<StateInSubAreaHistoricalKey,
                                        std::vector<NationStateHistIsCurrChk*> >
{
public:
  static StateInSubAreaHistoricalDAO& instance();

  bool isStateInSubArea(const NationCode& nation,
                        const StateCode& state,
                        const LocCode& subArea,
                        DeleteList& del,
                        const DateTime& tktDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StateInSubAreaHistoricalDAO>;
  static DAOHelper<StateInSubAreaHistoricalDAO> _helper;
  StateInSubAreaHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<StateInSubAreaHistoricalKey,
                                 std::vector<NationStateHistIsCurrChk*> >(cacheSize, cacheType)
  {
  }
  std::vector<NationStateHistIsCurrChk*>* create(StateInSubAreaHistoricalKey key) override;
  void
  destroy(StateInSubAreaHistoricalKey key, std::vector<NationStateHistIsCurrChk*>* recs) override;

private:
  static StateInSubAreaHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateInSubAreaHistoricalDAO

/////// Current ///////
typedef HashKey<VendorCode, int, char, NationCode, StateCode> StateInZoneKey;
class StateInZoneDAO : public DataAccessObject<StateInZoneKey, NationInAreaInfo>
{
public:
  static StateInZoneDAO& instance();
  bool isStateInZone(const VendorCode& vendor,
                     int zone,
                     const ZoneType zoneType,
                     const NationCode& nation,
                     const StateCode& state);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<StateInZoneDAO>;
  static DAOHelper<StateInZoneDAO> _helper;

  StateInZoneDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<StateInZoneKey, NationInAreaInfo>(cacheSize, cacheType)
  {
  }

  NationInAreaInfo* create(StateInZoneKey key) override;
  void destroy(StateInZoneKey key, NationInAreaInfo* rec) override;

private:
  static StateInZoneDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateInZoneDAO

/////// Historical ///////
typedef HashKey<VendorCode, int, char, NationCode, StateCode, DateTime, DateTime>
StateInZoneHistoricalKey;
class StateInZoneHistoricalDAO
    : public HistoricalDataAccessObject<StateInZoneHistoricalKey,
                                        std::vector<NationStateHistIsCurrChk*> >
{
public:
  static StateInZoneHistoricalDAO& instance();

  bool isStateInZone(const VendorCode& vendor,
                     int zone,
                     const ZoneType zoneType,
                     const NationCode& nation,
                     const StateCode& state,
                     DeleteList& del,
                     const DateTime& ticketDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<StateInZoneHistoricalDAO>;
  static DAOHelper<StateInZoneHistoricalDAO> _helper;

  StateInZoneHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<StateInZoneHistoricalKey, std::vector<NationStateHistIsCurrChk*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<NationStateHistIsCurrChk*>* create(StateInZoneHistoricalKey key) override;
  void destroy(StateInZoneHistoricalKey key, std::vector<NationStateHistIsCurrChk*>* recs) override;

private:
  static StateInZoneHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateInZoneHistoricalDAO
} // namespace tse
