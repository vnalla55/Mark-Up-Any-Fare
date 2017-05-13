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
class BaseFareRule;
class DeleteList;

typedef HashKey<int> BaseFareRuleKey;

class BaseFareRuleDAO : public DataAccessObject<BaseFareRuleKey, std::vector<const BaseFareRule*>>
{
public:
    BaseFareRuleDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }
    static BaseFareRuleDAO& instance();
    const std::vector<const BaseFareRule*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                int itemNo,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

    BaseFareRuleKey createKey( const BaseFareRule * info ) ;

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData *
      compress (const std::vector<const BaseFareRule*> *vect) const;

    virtual std::vector<const BaseFareRule*> *
      uncompress (const sfc::CompressedData &compressed) const;

protected:

    struct isEffective;
    static std::string _name;
    static std::string _cacheClass;
    void load() ;
    std::vector<const BaseFareRule*>* create(BaseFareRuleKey key);
    void destroy(BaseFareRuleKey key, std::vector<const BaseFareRule*>* t);

private:
    static BaseFareRuleDAO* _instance;
};
} // namespace tse
