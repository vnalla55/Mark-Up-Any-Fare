#pragma once

#include "FileLoader/FileLoaderBase.h"

namespace tse
{
class FileLoader : public FileLoaderBase
{
public:
  FileLoader(const std::string& url, BoundFareCache* cache);
  virtual ~FileLoader();

  virtual void parse() override;

private:
  typedef void (FileLoader::*ONNEWMARKET)(const FareKey&);
  static class FareInfo* parseLine(const char* pLine, size_t length);
  bool onNewFare(const FareInfo* pFareInfo, FareKey& prvKey);
  static int createBindings(Record2ReferenceVector& bindings, const char* pBuffer, size_t length);
  static const char* createRecord1Info(const char* pBuffer,
                                       const char* pBufferEnd,
                                       class AdditionalInfoContainer* pContainer);

  void insertEntry(const FareKey& key);
  /*
      enum
      {
        MARKET1
        , MARKET2
        , CARRIER
        , FARECLASS
        , FARETARIFF
        , VENDOR
        , CURRENCY
        , LINKNO
        , SEQNO
        , CREATEDATE
        , EXPIREDATE
        , EFFDATE
        , DISCONTINUEDATE
        , FAREAMT
        , ADJUSTEDFAREAMT
        , NODECIMALPOINTS
        , FOOTNOTE1
        , FOOTNOTE2
        , ROUTING
        , RULE
        , DIRECTIONALITY
        , GLOBALDIR
        , OWRT
        , TRAVELOCITYWEBFARE // T | ''
        , EXPEDIAWEBFARE     // E | ''
        , FARETYPE           // P | R
        , ALLBOOKINGCODES
        , RECORD2
        , TOKENCOUNT
      };
  */
  enum
  {
    MARKET1,
    MARKET2,
    CARRIER,
    FARECLASS,
    FARETARIFF,
    VENDOR,
    CURRENCY,
    LINKNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCONTINUEDATE,
    FAREAMT,
    ADJUSTEDFAREAMT,
    NODECIMALPOINTS,
    FOOTNOTE1,
    FOOTNOTE2,
    ROUTING,
    RULE,
    DIRECTIONALITY,
    GLOBALDIR,
    OWRT,
    RULETARIFF // FareClassAppInfo::_ruleTariff, from TARIFFCROSSREFERENCE, TariffNumber
    ,
    ROUTINGTARIFF // from TARIFFCROSSREFERENCE.ROUTINGTARIFF1, TariffNumber
    ,
    FARETYPE // 'EU'
    ,
    ROUTING3RESTRICTIONNEGVIAPPL // ROUTING 3 NEGATIVE APPLICATION INDICATOR ?
    ,
    ROUTING3RESTRICTIONNONSTOPDIRECTIND // ROUTING 3 RESTRICTION NON STOP/DIRECT INDICATOR ?
    ,
    SAMECARRIER102 // SAME CARRIER 102
    ,
    SAMECARRIER103 // SAME CARRIER RESTRICTION COMBINABILITY CIRCLE TRIP
    ,
    SAMECARRIER104 // SAME CARRIER RESTRICTION COMBINABILITY END-ON-END
    ,
    TRAVELOCITYWEBFARE // T | ''
    ,
    EXPEDIAWEBFARE // E | ''
    ,
    TARIFFTYPE // P | R previously called FARETYPE, from TARIFFCROSSREFERENCE, tariff category
    ,
    DOMINTINDICATOR // 'D'
    ,
    ALLBOOKINGCODES,
    RECORD1INFO // "<"seqno,faretype,seasonal,dow,(flip indicator)">", (added as last thing after
                // all the bound rules)
    ,
    RECORD2,
    TOKENCOUNT // 37
  };
  /*
    "{" Rule Category Number |
        Rule Sequence number |
        FLIP_INDICATOR='F'|'T' |
        ReferenceType='0|1|2|3'
    "}"
  */

  enum
  {
    RECORD1SEQNO,
    RECORD1FARETYPE,
    RECORD1SEASONAL,
    RECORD1DOW,
    RECORD1FLIPINDICATOR
  };

  ONNEWMARKET _onNewMarket;
  // not implemented
  FileLoader(const FileLoader&);
  FileLoader& operator=(const FileLoader&);
};
}
