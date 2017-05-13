//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DataAccessObject.h"
//#include "HistoricalDataAccessObject.h"
//#include "DBAccess/DAOHelper.h"
#include "TseTypes.h"
//#include "DBAccess/DeleteList.h"
#include "AddonCombFareClassInfo.h"

namespace tse
{
//class AddonCombFareClassInfo;
class DeleteList;
/*
typedef HashKey<VendorCode, TariffNumber, CarrierCode> AddOnCombFareClassKey;

class AddOnCombFareClassDAO : public DataAccessObject<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false>
{
public:
    static AddOnCombFareClassDAO& instance();

    const AddonFareClassCombMultiMap & get(DeleteList& del,
                                           const VendorCode& vendor,
                                           const TariffNumber& fareTariff,
                                           const CarrierCode& carrier);

    bool translateKey(const ObjectKey& objectKey, AddOnCombFareClassKey& key) const
    {
        return key.initialized =    objectKey.getValue("VENDOR", key._a)
                                 && objectKey.getValue("FARETARIFF", key._b)
                                 && objectKey.getValue("CARRIER", key._c);
    }


    AddOnCombFareClassKey createKey( AddonCombFareClassInfo * info ) ;

    void translateKey( const AddOnCombFareClassKey & key, ObjectKey & objectKey ) const
    {
        objectKey.setValue("VENDOR",key._a);
        objectKey.setValue("FARETARIFF", key._b);
        objectKey.setValue("CARRIER",key._c);
    }

    bool insertDummyObject( std::string & flatKey, ObjectKey & objectKey )
    {
      return DummyObjectInserter<AddonCombFareClassInfo,AddOnCombFareClassDAO>( flatKey, objectKey ).success() ;
    }


    log4cxx::LoggerPtr & getLogger()
    {
      return _logger ;
    }

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

protected:

    static std::string _name;
    static std::string _cacheClass;
    friend class DAOHelper<AddOnCombFareClassDAO>;
    static DAOHelper<AddOnCombFareClassDAO> _helper;
    AddOnCombFareClassDAO(int cacheSize = 0, const std::string& cacheType="")
      :   DataAccessObject<AddOnCombFareClassKey, AddonFareClassCombMultiMap, false>(cacheSize,cacheType, 2)
    {
    }
    virtual void load();
    AddonFareClassCombMultiMap * create(AddOnCombFareClassKey key);
    void destroy(AddOnCombFareClassKey key, AddonFareClassCombMultiMap * t);

private:
    struct groupByKey;
    static AddOnCombFareClassDAO* _instance;
    static log4cxx::LoggerPtr _logger;
}; // class AddOnCombFareClassDAO
*/
// Historical Stuff ///////////////////////////////////////////////////////////////////////////////////////////////
//typedef HashKey<VendorCode,TariffNumber,CarrierCode,DateTime,DateTime> AddOnCombFareClassHistoricalKey;

typedef HashKey<int> AddOnCombFareClassHistoricalKey;

class AddOnCombFareClassHistoricalDAO : public DataAccessObject<AddOnCombFareClassHistoricalKey, std::vector<AddonCombFareClassInfo*>>
{
public:
    static AddOnCombFareClassHistoricalDAO& instance();
    AddOnCombFareClassHistoricalDAO(int cacheSize = 0, const std::string& cacheType="")
      //:   HistoricalDataAccessObject<AddOnCombFareClassHistoricalKey, std::vector<AddonCombFareClassInfo*>>(cacheSize,cacheType, 3)
    {
    }

    const std::vector<AddonCombFareClassInfo*>& get(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const TariffNumber& fareTariff,
                                                    const CarrierCode& carrier,
                                                    const DateTime& ticketDate);
    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData *
      compress (const std::vector<AddonCombFareClassInfo*> *vect) const;

    virtual std::vector<AddonCombFareClassInfo*> *
      uncompress (const sfc::CompressedData &compressed) const;

protected:
    static std::string _name;
    static std::string _cacheClass;
    //friend class DAOHelper<AddOnCombFareClassHistoricalDAO>;
    //static DAOHelper<AddOnCombFareClassHistoricalDAO> _helper;
    std::vector<AddonCombFareClassInfo*>* create(AddOnCombFareClassHistoricalKey key);
    void destroy(AddOnCombFareClassHistoricalKey key, std::vector<AddonCombFareClassInfo*>* t);

private:
    static AddOnCombFareClassHistoricalDAO* _instance;
    //static log4cxx::LoggerPtr _logger;
}; // class AddOnCombFareClassHistoricalDAO
} // namespace tse

