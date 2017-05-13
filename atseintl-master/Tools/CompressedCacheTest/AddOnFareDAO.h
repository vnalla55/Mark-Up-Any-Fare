//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
class AddonFareInfo;
class DeleteList;

typedef HashKey<int> AddOnFareKey;

class AddOnFareDAO : public DataAccessObject<AddOnFareKey, std::vector<AddonFareInfo*>>
{
public:
    static AddOnFareDAO& instance();

    AddOnFareDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }

    const std::vector<AddonFareInfo*>& get(DeleteList& del,
                                           const LocCode& interiorMarket,
                                           const CarrierCode& carrier,
                                           const DateTime& date,
                                           const DateTime& ticketDate,
                                           bool isFareDisplay);

    AddOnFareKey createKey( AddonFareInfo * info ) ;

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData*
      compress(const std::vector<AddonFareInfo*>* vect) const;

    virtual std::vector<AddonFareInfo*>*
      uncompress(const sfc::CompressedData& compressed) const;
protected:
    static std::string _name;
    static std::string _cacheClass;
    void load() ;
    std::vector<AddonFareInfo*>* create(AddOnFareKey key);
    void destroy(AddOnFareKey key, std::vector<AddonFareInfo*>* t);

private:
    struct isEffective;
    struct groupByKey;
    static AddOnFareDAO* _instance;
}; // class AddOnFareDAO

} // namespace tse
