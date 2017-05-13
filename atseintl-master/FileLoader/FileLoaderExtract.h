#pragma once

#include "FileLoader/FileLoaderBase.h"

#include <string>

namespace tse
{
class FareInfo;

class FileLoaderExtract : public FileLoaderBase
{
public:
  FileLoaderExtract(const std::string& url, BoundFareCache* cache);
  virtual ~FileLoaderExtract();

  virtual void parse() override;

private:
  typedef void (FileLoaderExtract::*ONNEWMARKET)(const FareKey&);
  static class FareInfo* parseLine(const char* pLine, size_t length);
  bool onNewFare(const FareInfo* pFareInfo, FareKey& prvKey);
  static int createBindings(Record2ReferenceVector& bindings, const char* pBuffer, size_t length);

  void insertEntry(const FareKey& key);

  enum
  {
    MARKET1,
    MARKET2,
    CARRIER,
    OWRT,
    FARETYPE // overloaded, 'P' | 'R'| 'W' | 'J'
    ,
    NONSTOPDIRECTIND,
    NEGAPPLMARKET,
    FARECLASS,
    VENDOR,
    RULE,
    SAMECARRIERRT // (round trip) 102 bool
    ,
    SAMECARRIERCT // (circle trip) 103 bool
    ,
    SAMECARRIEREOE // (end on end) 104 bool
    ,
    ROUTING,
    ROUTINGTARIFF,
    RULETARIFF,
    FARETARIFF,
    FOOTNOTE1,
    FOOTNOTE2,
    FAREAMT,
    NUMBERADVPURCHASEDAYS,
    ALLBOOKINGCODES,
    RECORD2,
    TOKENCOUNT
  };
  /*
ABE,            // MARKET1
MSY,            // MARKET2
UA ,            // CARRIER
2,              // OWRT
P,              // FARETYPE overloaded, 'P' | 'R'| 'W' | 'J'
,               // NONSTOPDIRECTIND char
 ,              // NEGAPPLMARKET char
WRA14ON7,       // FareClass
ATP ,           // vendor
2320,           // Rule
0,              // sameCarrierRT (round trip) 102 bool
0,              // sameCarrierCT (circle trip) 103 bool
0,              // sameCarrierEOE (end on end) 104 bool
524,            // Routing
99,             // RoutingTariff
11,             // RuleTariff
0,              // FareTariff
63,             // Footnote1
  ,             // Footnote2
16651,          // FareAmt
14,             // numberAdvPurchaseDays
WW,             // BookingCodes
{14|100000|f|}{8|1000000|F|}{2|1325000|F|}{11|1907000|F|}{15|9000000|F|}
*/
  /*
    "{" Rule Category Number |
        Rule Sequence number |
        FLIP_INDICATOR='F'|'B' |
    "}"
  */
  ONNEWMARKET _onNewMarket;
  // not implemented
  FileLoaderExtract(const FileLoaderExtract&);
  FileLoaderExtract& operator=(const FileLoaderExtract&);
};
}
