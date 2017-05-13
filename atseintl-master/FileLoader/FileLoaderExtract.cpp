#include "FileLoader/FileLoaderExtract.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/BoundFareAdditionalInfo.h"
#include "DBAccess/Cache.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/HashKey.h"
#include "FileLoader/GZStream.h"

#include <set>

#include <errno.h>
#include <time.h>

using std::cerr;
#define cerr                                                                                       \
  if (false)                                                                                       \
  cerr

const std::string
_futureStr("9999-12-31 00:00:00.0"),
    _pastStr("2005-09-26 00:00:00.0");
namespace tse
{
DateTime _future, _past;

namespace
{
log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.BFFileLoader"));

inline long
atoi(const char* buffer, size_t sz)
{
  long result(0);
  for (size_t i = 0; i < sz; ++i)
  {
    long res((result << 3) + (result << 1));
    result = res + (buffer[i] - static_cast<int>('0'));
  }
  /*
    cerr << "result=" << result << ",buffer:";
    cerr.write(buffer, sz);
    cerr << std::endl;
  */
  return result;
}
inline size_t
trim(const char* pString, size_t sz)
{
  size_t newSz(sz);
  while (newSz > 0 && ' ' == *(pString + newSz - 1))
  {
    --newSz;
  }
  return newSz;
}
}

FileLoaderExtract::FileLoaderExtract(const std::string& url, BoundFareCache* cache)
  : FileLoaderBase(url, cache)
{
  _onNewMarket = &FileLoaderExtract::insertEntry;
  parseDateTime(_futureStr.c_str(), _futureStr.length(), _future);
  parseDateTime(_pastStr.c_str(), _pastStr.length(), _past);
  LOG4CXX_INFO(_logger, "FileLoaderExtract:url:" << url);
}

FileLoaderExtract::~FileLoaderExtract() {}

void
FileLoaderExtract::insertEntry(const FareKey& key)
{
  if (_pVector != nullptr)
  {
    _cache->put(key, _pVector, true);
  }
  _pVector = nullptr;
}

bool
FileLoaderExtract::onNewFare(const FareInfo* pFareInfo, FareKey& prvKey)
{
  FareKey key(pFareInfo->_market1, pFareInfo->_market2, pFareInfo->_carrier);
  if (!prvKey.initialized && nullptr == _pVector)
  {
    _pVector = new FareInfoVec;
    prvKey = key;
    cerr << "FileLoaderExtract::onNewFare:created first vector" << std::endl;
  }
  else if (!(key == prvKey))
  {
    (this->*_onNewMarket)(prvKey);
    _pVector = new FareInfoVec;
    prvKey = key;
  }
  _pVector->push_back(pFareInfo);
  return true;
}
/*
      MARKET1
      , MARKET2
      , CARRIER
      , OWRT
      , FARETYPE // overloaded, 'P' | 'R'| 'W' | 'J'
      , NONSTOPDIRECTIND
      , NEGAPPLMARKET
      , FARECLASS
      , VENDOR
      , RULE
      , SAMECARRIERCT
      , SAMECARRIEREOE
      , ROUTING
      , ROUTINGTARIFF
      , RULETARIFF
      , FARETARIFF
      , FOOTNOTE1
      , FOOTNOTE2
      , FAREAMT
      , NUMBERADVPURCHASEDAYS
      , ALLBOOKINGCODES
      , RECORD2
      , TOKENCOUNT
 */
/*
  private enum Columns {
      Orig,
      Dest,
      Carrier,
      Owrt,
      FareType,
      NonstopDirectInd,
      NegApplMarket,
      FareClass,
      Vendor,
      Rule,
      sameCarrierCT,
      sameCarrierEOE,
      Routing,
      RoutingTariff,
      RuleTariff,
      FareTariff,
      Footnote1,
      Footnote2,
      FareAmt,
      numberAdvPurchaseDays,
      BookingCodes,
      RuleCategorySequences,
      ColumnCount
  }
*/
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
FareInfo*
FileLoaderExtract::parseLine(const char* pLine, size_t length)
{
  FareInfo* pFareInfo = new FareInfo;
  AdditionalInfoContainer* pContainer = new AdditionalInfoContainer;
  pFareInfo->_inhibit = 'N';
  const char* pBuffer = pLine, *const pBufferEnd = pLine + length, *pDelim = pBufferEnd;
  int itemNumber(0);
  long amount(0);
  while (pBuffer < pBufferEnd && (pDelim = std::find(pBuffer, pBufferEnd, ',')) != pBufferEnd)
  {
    // process it
    size_t sz(pDelim - pBuffer);
    switch (itemNumber)
    {
    case MARKET1:
      pFareInfo->_market1.assign(pBuffer, trim(pBuffer, sz));
      break;
    case MARKET2:
      pFareInfo->_market2.assign(pBuffer, trim(pBuffer, sz));
      break;
    case CARRIER:
      pFareInfo->_carrier.assign(pBuffer, trim(pBuffer, sz));
      break;
    case OWRT:
      if (sz > 0)
      {
        pFareInfo->_owrt = *pBuffer;
      }
      break;
    case FARETYPE:
      cerr << "FARETYPE=";
      cerr.write(pBuffer, sz);
      cerr << std::endl;
      if (1 == sz)
      {
        if ('J' == *pBuffer)
        {
          pContainer->_paxType = "JCB";
          pContainer->_tariffType = 'R';
          // std::cout << pContainer->_paxType << std::endl;
        }
        else
        {
          pContainer->_tariffType = *pBuffer;
        }
      }
      break;
    case NONSTOPDIRECTIND:
      if (1 == sz)
      {
        pContainer->_nonstopDirectInd = *pBuffer;
        // if (*pBuffer != ' ')
        //{
        //  std::cout << "pContainer->_nonstopDirectInd=" << *pBuffer << std::endl;// 'N'|'E'|' '
        //}
      }
      break;
    case NEGAPPLMARKET:
      if (1 == sz)
      {
        pContainer->_negViaAppl = *pBuffer;
        // if (*pBuffer != ' ')
        //{
        //  std::cout << "pContainer->_negViaAppl=" << *pBuffer << std::endl;
        //}
      }
      break;
    case FARECLASS:
      pFareInfo->_fareClass.assign(pBuffer, trim(pBuffer, sz));
      break;
    case VENDOR:
      pFareInfo->_vendor.assign(pBuffer, trim(pBuffer, sz));
      break;
    case RULE:
      pFareInfo->_ruleNumber.assign(pBuffer, trim(pBuffer, sz));
      // cerr << "pFareInfo->_ruleNumber=" << pFareInfo->_ruleNumber << std::endl;
      break;
    case SAMECARRIERRT:
      break;
    case SAMECARRIERCT:
      if (1 == sz)
      {
        pContainer->_sameCarrier103 = 'T' == *pBuffer;
        // if (' ' != *pBuffer)
        //{
        //  std::cout << "SAMECARRIER103:" << *pBuffer << std::endl;
        //}
      }
      break;
    case SAMECARRIEREOE:
      if (1 == sz)
      {
        pContainer->_sameCarrier104 = 'T' == *pBuffer;
        // if (' ' != *pBuffer)
        //{
        //  std::cout << "SAMECARRIER104:" << *pBuffer << std::endl;
        //}
      }
      break;
    case ROUTING:
    {
      size_t numberOfLeadingZeros(4 - sz);
      std::string paddedRouting(numberOfLeadingZeros, '0');
      paddedRouting.append(pBuffer, sz);
      pFareInfo->_routingNumber = paddedRouting;
      // std::cout << pFareInfo->_routingNumber << std::endl;
    }
    break;
    case ROUTINGTARIFF:
      pContainer->_routingTariff = atoi(pBuffer, sz);
      break;
    case RULETARIFF:
      pContainer->_ruleTariff = atoi(pBuffer, sz);
      break;
    case FARETARIFF:
      pFareInfo->_fareTariff = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_fareTariff=" << pFareInfo->_fareTariff << std::endl;
      // cerr.write(pBuffer, sz);
      // cerr << std::endl;
      break;
    case FOOTNOTE1:
      pFareInfo->_footnote1.assign(pBuffer, trim(pBuffer, sz));
      // cerr << "pFareInfo->_footnote1=" << pFareInfo->_footnote1 << std::endl;
      break;
    case FOOTNOTE2:
      pFareInfo->_footnote2.assign(pBuffer, trim(pBuffer, sz));
      // cerr << "pFareInfo->_footnote2=" << pFareInfo->_footnote2 << std::endl;
      break;
    case FAREAMT:
      amount = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_fareAmount=" << pFareInfo->_fareAmount << std::endl;
      break;
    case NUMBERADVPURCHASEDAYS:
      break;
    case ALLBOOKINGCODES:
      // std::cout << "ALLBOOKINGCODES=";
      // std::cout.write(pBuffer, sz);
      // std::cout << std::endl;
      pContainer->_bookingCodes.assign(pBuffer, pBuffer + sz);
      // std::cout << "sz=" << sz << ",bookingCodes.size()=" << bookingCodes.size() << std::endl;
      break;
      /*
  case LINKNO:
    break;
  case SEQNO:
      */
    }
    ++itemNumber;
    pBuffer = pDelim + 1;
  }
  pFareInfo->_currency = "USD";
  pFareInfo->_effInterval.createDate() = _past;
  pFareInfo->_effInterval.expireDate() = _future;
  pFareInfo->_effInterval.effDate() = _past;
  pFareInfo->_effInterval.discDate() = _future;
  pFareInfo->_noDec = 2;
  pFareInfo->_directionality = BOTH;
  pFareInfo->_globalDirection = GlobalDirection::ZZ;
  pContainer->_travelocityWebfare = 'W' == pContainer->_tariffType;

  pContainer->_fareType = "EU";

  pFareInfo->_fareAmount = adjustDecimal(amount, pFareInfo->_noDec);
  pFareInfo->_originalFareAmount = pFareInfo->_fareAmount;
  if (pFareInfo->_owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    pFareInfo->_fareAmount /= 2;
  }
  if (pContainer->_bookingCodes.empty() && !pFareInfo->_fareClass.empty())
  {
    pContainer->_bookingCodes.push_back(BookingCode(pFareInfo->_fareClass[0]));
  }
  if (pBuffer < pBufferEnd && TOKENCOUNT - 1 == itemNumber) // bindings
  {
    Record2ReferenceVector& references = pContainer->_references;
    references.reserve(_ESTIMATEDNUMBEROFBINDINGS);
    // int numberBindings =
    createBindings(references, pBuffer, pBufferEnd - pBuffer);
    // std::sort(references.begin(), references.end());
    // cerr << "created " << numberBindings << " bindings" << endl;
  }
  // else pBufferEnd == pBuffer - empty bindings
  if (TOKENCOUNT != ++itemNumber) // bad entry
  {
    delete pContainer;
    delete pFareInfo;
    pFareInfo = nullptr;
  }
  else
  {
    pFareInfo->_pAdditionalInfoContainer = pContainer;
  }
  return pFareInfo;
}

int
FileLoaderExtract::createBindings(Record2ReferenceVector& bindings,
                                  const char* substr,
                                  size_t length)
{
  int numberOfBindings(0);
  const char* pBuffer = substr + 1;
  const char* const pBufferEnd = substr + length;
  const char* pEndOfBindings = pBufferEnd;
  while (pBuffer < pBufferEnd &&
         (pEndOfBindings = std::find(pBuffer, pBufferEnd, '}')) != pBufferEnd)
  {
    // std::string bindingsStr(pBuffer, pEndOfBindings - pBuffer);
    // cerr << bindingsStr << endl;
    const char* pBind = pBuffer;
    const char* pPipe = nullptr;
    uint16_t catNumber(0);
    uint32_t sequenceNumber(0);
    MATCHTYPE matchType(MATCHNONE);
    char directionality('0');
    if ((pPipe = std::find(pBind, pEndOfBindings, '|')) != pEndOfBindings)
    {
      catNumber = static_cast<uint16_t>(atoi(pBind, pPipe - pBind));
      pBind = pPipe + 1;
      // cerr << "catNumber=" << catNumber << std::endl;
    }
    if (pBind < pEndOfBindings && (pPipe = std::find(pBind, pEndOfBindings, '|')) != pEndOfBindings)
    {
      sequenceNumber = static_cast<uint32_t>(atoi(pBind, pPipe - pBind));
      pBind = pPipe + 1;
      // cerr << "sequenceNumber=" << sequenceNumber << std::endl;
    }
    if (pBind < pEndOfBindings && (pPipe = std::find(pBind, pEndOfBindings, '|')) != pEndOfBindings)
    {
      directionality = *pBind;
      pBind = pPipe + 1;
      cerr << "directionality=" << directionality << std::endl;
    }
    if ('f' == directionality || 'b' == directionality)
    {
      matchType = FOOTNOTE;
    }
    else
    {
      matchType = FARERULE;
    }
    Record2Reference reference(catNumber, sequenceNumber, directionality, matchType);
    bindings.push_back(reference);
    ++numberOfBindings;
    pBuffer = pEndOfBindings + 2; // "}{"
  }
  return numberOfBindings;
}

void
FileLoaderExtract::parse()
{
  const std::streamsize bufferSz(512000);
  char buffer[bufferSz];
  std::streamsize bytesRead(0);
  size_t lineNumber(0);
  size_t off(0);
  FareKey prvKey;
  while ((bytesRead = _gzStream->read(buffer + off, bufferSz - off)) > 0)
  {
    const char* pBuffer = buffer;
    const char* const pBufferEnd = buffer + bytesRead + off;
    const char* pEndOfLine = pBufferEnd;
    while (pBuffer < pBufferEnd &&
           (pEndOfLine = std::find(pBuffer, pBufferEnd, '\n')) != pBufferEnd)
    {
// check line
#if 0
        size_t lineLength(pEndOfLine - pBuffer);
        for (size_t ci = 0; ci < lineLength; ++ci)
        {
          char ch(*(pBuffer + ci));
          if (ch < 32 || ch > 126)
          {
            std::cout << "FileLoaderExtract::parse:line #" << lineNumber << ":invalid character:" << int(ch) << std::endl;
          }
        }
#endif // 0
      // process line
      FareInfo* pFareInfo = nullptr;
      if (nullptr == (pFareInfo = parseLine(pBuffer, pEndOfLine - pBuffer)))
      {
        std::ostringstream msg;
        msg << "FileLoaderExtract::parse:line #" << lineNumber << ":bad entry:";
        msg.write(pBuffer, pEndOfLine - pBuffer);
        LOG4CXX_WARN(_logger, msg.str());
        pBuffer = pEndOfLine + 1;
        continue;
      }
#if 0
        if (lineNumber < 50)
        {
          std::cout.write(pBuffer, pEndOfLine - pBuffer);
          std::cout << std::endl;
        }
#endif // 0
      onNewFare(pFareInfo, prvKey);
      ++lineNumber;
      pBuffer = pEndOfLine + 1;
    }
    off = pBufferEnd - pBuffer;
    memcpy(buffer, pBuffer, off);
  }
  (this->*_onNewMarket)(prvKey);
  LOG4CXX_INFO(_logger,
               "FileLoaderExtract::parse:parsed " << lineNumber
                                                  << " lines,_cache->size()=" << _cache->size());
#if 0
    if (_logger->isInfoEnabled())
    {
      std::shared_ptr<std::vector<FareKey> > allKeys = _cache->keys();
      size_t numberFares(0);
      if (allKeys)
      {
        size_t numberKeys(allKeys->size());
        for (size_t i = 0; i < numberKeys; ++i)
        {
          std::shared_ptr<const FareInfoVec> fares(_cache->getIfResident((*allKeys)[i]));
          numberFares += fares->size();
        }
      }
      LOG4CXX_INFO(_logger, "FileLoaderExtract::parse:total fares:" << numberFares);
    }
#endif // 0
}

} // tse namespace
