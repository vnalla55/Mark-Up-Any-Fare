#include "FileLoader/FileLoader.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DBAccess/BoundFareAdditionalInfo.h"
#include "DBAccess/Cache.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/HashKey.h"
#include "FileLoader/FareSortKey.h"
#include "FileLoader/GZStream.h"
#include "FileLoader/VctrKey.h"

#include <set>

#include <errno.h>
#include <time.h>

using std::cerr;
#define cerr                                                                                       \
  if (false)                                                                                       \
  cerr

namespace tse
{
extern const char* globalDirectionItems[];

namespace
{
inline bool
translateGlobalDirection(GlobalDirection& dst, const char* src)
{
  bool bResult(false);
  for (int i = 0; i < GlobalDirection::ZZ; ++i)
  {
    const char* globDir = globalDirectionItems[i];
    if (src[1] == globDir[1] && src[0] == globDir[0])
    {
      dst = static_cast<tse::GlobalDirection>(i + 1);
      bResult = true;
      break;
    }
  }
  return bResult;
}

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
}

std::ostream& operator<<(std::ostream& os, const FareInfo& fareInfo)
{
  os << fareInfo._vendor << ',' << fareInfo._carrier << ',' << fareInfo._market1 << ','
     << fareInfo._market2 << ',' << fareInfo._originalFareAmount << ',' << fareInfo._fareAmount
     << ',' << fareInfo._lastModDate << ',' << fareInfo._fareClass << ',' << fareInfo._fareTariff
     << ',' << fareInfo._linkNumber << ',' << fareInfo._sequenceNumber << ',' << fareInfo._noDec
     << ',' << fareInfo._currency << ',' << fareInfo._footnote1 << ',' << fareInfo._footnote2 << ','
     << fareInfo._owrt << ',' << fareInfo._directionality << ',' << fareInfo._ruleNumber << ','
     << fareInfo._routingNumber << ',' << fareInfo._globalDirection << ';';
  if (fareInfo._pAdditionalInfoContainer != nullptr)
  {
    os << *(fareInfo._pAdditionalInfoContainer);
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const FareInfo* fareInfo)
{
  return operator<<(os, *fareInfo);
}

FileLoader::FileLoader(const std::string& url, BoundFareCache* cache) : FileLoaderBase(url, cache)
{
  _onNewMarket = &FileLoader::insertEntry;
  LOG4CXX_INFO(_logger, "FileLoader:url:" << url);
}

FileLoader::~FileLoader() {}

void
FileLoader::insertEntry(const FareKey& key)
{
  if (_pVector != nullptr)
  {
    _cache->put(key, _pVector, false);
  }
  _pVector = nullptr;
}

bool
FileLoader::onNewFare(const FareInfo* pFareInfo, FareKey& prvKey)
{
  FareKey key(pFareInfo->_market1, pFareInfo->_market2, pFareInfo->_carrier);
  if (!prvKey.initialized && nullptr == _pVector)
  {
    _pVector = new FareInfoVec;
    prvKey = key;
    cerr << "FileLoader::onNewFare:created first vector" << std::endl;
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

FareInfo*
FileLoader::parseLine(const char* pLine, size_t length)
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
      pFareInfo->_market1.assign(pBuffer, sz);
      break;
    case MARKET2:
      pFareInfo->_market2.assign(pBuffer, sz);
      break;
    case CARRIER:
      pFareInfo->_carrier.assign(pBuffer, sz);
      break;
    case FARECLASS:
      pFareInfo->_fareClass.assign(pBuffer, sz);
      break;
    case FARETARIFF:
      pFareInfo->_fareTariff = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_fareTariff=" << pFareInfo->_fareTariff << std::endl;
      // cerr.write(pBuffer, sz);
      // cerr << std::endl;
      break;
    case VENDOR:
      pFareInfo->_vendor.assign(pBuffer, sz);
      break;
    case CURRENCY:
      pFareInfo->_currency.assign(pBuffer, sz);
      break;
    case LINKNO:
      pFareInfo->_linkNumber = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_linkNumber=" << pFareInfo->_linkNumber << std::endl;
      break;
    case SEQNO:
      pFareInfo->_sequenceNumber = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_sequenceNumber=" << pFareInfo->_sequenceNumber << std::endl;
      break;
    case CREATEDATE:
      parseDateTime(pBuffer, sz, pFareInfo->_effInterval.createDate());
      break;
    case EXPIREDATE:
      parseDateTime(pBuffer, sz, pFareInfo->_effInterval.expireDate());
      break;
    case EFFDATE:
      parseDateTime(pBuffer, sz, pFareInfo->_effInterval.effDate());
      break;
    case DISCONTINUEDATE:
      parseDateTime(pBuffer, sz, pFareInfo->_effInterval.discDate());
      break;
    case FAREAMT:
      amount = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_fareAmount=" << pFareInfo->_fareAmount << std::endl;
      break;
    case ADJUSTEDFAREAMT:
      break;
    case NODECIMALPOINTS:
      pFareInfo->_noDec = atoi(pBuffer, sz);
      // cerr << "pFareInfo->_noDec=" << pFareInfo->_noDec << std::endl;
      break;
    case FOOTNOTE1:
      pFareInfo->_footnote1.assign(pBuffer, sz);
      // cerr << "pFareInfo->_footnote1=" << pFareInfo->_footnote1 << std::endl;
      break;
    case FOOTNOTE2:
      pFareInfo->_footnote2.assign(pBuffer, sz);
      // cerr << "pFareInfo->_footnote2=" << pFareInfo->_footnote2 << std::endl;
      break;
    case ROUTING:
      pFareInfo->_routingNumber.assign(pBuffer, sz);
      break;
    case RULE:
      pFareInfo->_ruleNumber.assign(pBuffer, sz);
      // cerr << "pFareInfo->_ruleNumber=" << pFareInfo->_ruleNumber << std::endl;
      break;
    case DIRECTIONALITY:
      if (1 == sz)
      {
        char dir(*pBuffer);
        if ('F' == dir)
        {
          pFareInfo->_directionality = FROM;
        }
        else if ('T' == dir)
        {
          pFareInfo->_directionality = TO;
        }
        else if (' ' == dir || 'B' == dir)
        {
          pFareInfo->_directionality = BOTH;
        }
      }
      else if (0 == sz)
      {
        pFareInfo->_directionality = BOTH;
      }
      // cerr << "pFareInfo->_directionality=" << pFareInfo->_directionality << ",*pBuffer=" <<
      // int(*pBuffer) << std::endl;
      break;
    case GLOBALDIR:
      if (0 == sz)
      {
        pFareInfo->_globalDirection = GlobalDirection::ZZ;
      }
      else if (2 == sz)
      {
        if (!translateGlobalDirection(pFareInfo->_globalDirection, pBuffer))
        {
          char src[] = { *pBuffer, *(pBuffer + 1), 0 };
          LOG4CXX_WARN(_logger, "failed GLOBALDIR,sz=" << sz << ",src:" << src);
        }
      }
      // cerr << "pFareInfo->_globalDirection=" << pFareInfo->_globalDirection << std::endl;
      break;
    case OWRT:
      if (sz > 0)
      {
        pFareInfo->_owrt = *pBuffer;
      }
      break;
    case RULETARIFF:
      pContainer->_ruleTariff = atoi(pBuffer, sz);
      break;
    case ROUTINGTARIFF:
      pContainer->_routingTariff = atoi(pBuffer, sz);
      break;
    case FARETYPE: // 'EU'
      pContainer->_fareType.assign(pBuffer, sz);
      break;
    case ROUTING3RESTRICTIONNEGVIAPPL:
      if (1 == sz)
      {
        pContainer->_negViaAppl = *pBuffer;
        // if (*pBuffer != ' ')
        //{
        //  std::cout << "pContainer->_negViaAppl=" << *pBuffer << std::endl;
        //}
      }
      break;
    case ROUTING3RESTRICTIONNONSTOPDIRECTIND:
      if (1 == sz)
      {
        pContainer->_nonstopDirectInd = *pBuffer;
        // if (*pBuffer != ' ')
        //{
        //  std::cout << "pContainer->_nonstopDirectInd=" << *pBuffer << std::endl;// 'N'|'E'|' '
        //}
      }
      break;
    case SAMECARRIER102:
      if (1 == sz)
      {
        pContainer->_sameCarrier102 = ('1' == *pBuffer);
        // if (' ' != *pBuffer)
        //{
        //  std::cout << "SAMECARRIER102:" << *pBuffer << std::endl;
        //}
      }
      break;
    case SAMECARRIER103:
      if (1 == sz)
      {
        pContainer->_sameCarrier103 = ('1' == *pBuffer);
        // if (' ' != *pBuffer)
        //{
        //  std::cout << "SAMECARRIER103:" << *pBuffer << std::endl;
        //}
      }
      break;
    case SAMECARRIER104:
      if (1 == sz)
      {
        pContainer->_sameCarrier104 = ('1' == *pBuffer);
        // if (' ' != *pBuffer)
        //{
        //  std::cout << "SAMECARRIER104:" << *pBuffer << std::endl;
        //}
      }
      break;
    case TRAVELOCITYWEBFARE:
      cerr << "TRAVELOCITYWEBFARE=";
      cerr.write(pBuffer, sz);
      cerr << std::endl;
      if (1 == sz)
      {
        pContainer->_travelocityWebfare = 'T' == *pBuffer;
      }
      break;
    case EXPEDIAWEBFARE:
      cerr << "EXPEDIAWEBFARE=";
      cerr.write(pBuffer, sz);
      cerr << std::endl;
      if (1 == sz)
      {
        pContainer->_expediaWebfare = 'E' == *pBuffer;
      }
      break;
    case TARIFFTYPE: // previous FARETYPE, 'P' | 'R'
      cerr << "FARETYPE=";
      cerr.write(pBuffer, sz);
      cerr << std::endl;
      if (1 == sz)
      {
        if ('J' == *pBuffer)
        {
          pContainer->_paxType = "JCB";
          pContainer->_tariffType = 'R';
          cerr << pContainer->_paxType << std::endl;
        }
        else
        {
          pContainer->_tariffType = *pBuffer;
        }
      }
      break;
    case DOMINTINDICATOR:
      if (1 == sz)
      {
        pContainer->_domInternInd = *pBuffer;
      }
      break;
    case ALLBOOKINGCODES:
      // std::cout << "ALLBOOKINGCODES=";
      // std::cout.write(pBuffer, sz);
      // std::cout << std::endl;
      pContainer->_bookingCodes.assign(pBuffer, pBuffer + sz);
      // std::cout << "sz=" << sz << ",bookingCodes.size()=" << bookingCodes.size() << std::endl;
      break;
    }
    pBuffer = pDelim + 1;
    if (ALLBOOKINGCODES == itemNumber++)
    {
      break;
    }
  }
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
  pBuffer = createRecord1Info(pBuffer, pBufferEnd, pContainer);
  if (pBuffer != nullptr)
  {
    ++itemNumber;
  }
  if (pBuffer != nullptr && pBuffer < pBufferEnd && TOKENCOUNT - 1 == itemNumber) // bindings
  {
    Record2ReferenceVector& references = pContainer->_references;
    references.reserve(_ESTIMATEDNUMBEROFBINDINGS);
    // int numberBindings =
    createBindings(references, pBuffer, pBufferEnd - pBuffer);
    // std::sort(references.begin(), references.end());
    // cerr << "created " << numberBindings << " bindings" << endl;
  }
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
//<1000000,PCF, , ,F>,
const char*
FileLoader::createRecord1Info(const char* pBuffer,
                              const char* pBufferEnd,
                              AdditionalInfoContainer* pContainer)
{
  const char* pNext = nullptr;
  // expired format does not have '<' or '>'
  if (pBuffer < pBufferEnd && ',' == *pBuffer)
  {
    return pBuffer + 1;
  }
  if (pBuffer < pBufferEnd && '<' == *pBuffer)
  {
    const char* pInfoEnd = std::find(pBuffer, pBufferEnd, '>');
    if (pInfoEnd != pBufferEnd && ++pInfoEnd < pBufferEnd && ',' == *pInfoEnd)
    {
      // std::cout.write(pBuffer, pInfoEnd - pBuffer);
      // std::cout << std::endl;
      pNext = pInfoEnd + 1;
      int itemNumber(0);
      const char* pField = pBuffer + 1;
      const char* pDelim = nullptr;
      while ((pDelim = std::find(pField, pInfoEnd, ',')) != pInfoEnd)
      {
        size_t sz(pDelim - pField);
        switch (itemNumber)
        {
        case RECORD1SEQNO:
          // std::cout.write(pField, sz);
          // std::cout << std::endl;
          atoi(pField, sz);
          break;
        case RECORD1FARETYPE:
          // std::cout.write(pField, sz);
          // std::cout << std::endl;
          // assign(pField, pDelim - pField);
          break;
        case RECORD1SEASONAL:
          // std::cout.write(pField, sz);// H | L
          // std::cout << std::endl;
          if (1 == sz)
          {
            //*pField;
          }
          break;
        case RECORD1DOW:
          // std::cout.write(pField, sz);// W | X
          // std::cout << std::endl;
          if (1 == sz)
          {
            //*pField;
          }
          break;
        }
        ++itemNumber;
        pField = pDelim + 1;
      }
      if (pField < pInfoEnd - 1) // RECORD1FLIPINDICATOR ?
      {
        size_t sz(pInfoEnd - 1 - pField);
        if (1 == sz)
        {
          //*pField;
        }
      }
    }
  }
  return pNext;
}

int
FileLoader::createBindings(Record2ReferenceVector& bindings, const char* substr, size_t length)
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
    if (pBind < pEndOfBindings)
    {
      int refInt(atoi(pBind, 1));
      if (refInt > BINDINGSNONE && refInt < NUMBERBINDINGTYPES)
      {
        switch (refInt)
        {
        case BINDINGSFOOTNOTE1:
        case BINDINGSFOOTNOTE2:
          matchType = FOOTNOTE;
          break;
        case BINDINGSFARERULE:
        case BINDINGSGENERALRULE:
          matchType = FARERULE;
          break;
        }
      }
      else
      {
        LOG4CXX_WARN(_logger, "createBindings:unsupported reference type:" << *pBind);
      }
      // cerr << "referenceType=" << referenceType << std::endl;
    }
    Record2Reference reference(catNumber, sequenceNumber, directionality, matchType);
    bindings.push_back(reference);
    ++numberOfBindings;
    pBuffer = pEndOfBindings + 2; // "}{"
  }
  return numberOfBindings;
}

void
FileLoader::parse()
{
  const std::streamsize bufferSz(512000);
  char buffer[bufferSz];
  std::streamsize bytesRead(0);
  size_t lineNumber(0);
  int numberOfBadEntries(0);
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
            std::cout << "FileLoader::parse:line #" << lineNumber << ":invalid character:" << int(ch) << std::endl;
          }
        }
#endif // 0
      // process line
      FareInfo* pFareInfo = nullptr;
      if (nullptr == (pFareInfo = parseLine(pBuffer, pEndOfLine - pBuffer)))
      {
        std::ostringstream msg;
        msg << "FileLoader::parse:line #" << lineNumber << ":bad entry:";
        msg.write(pBuffer, pEndOfLine - pBuffer);
        LOG4CXX_ERROR(_logger, msg.str());
        ++numberOfBadEntries;
        if (numberOfBadEntries >= _badEntriesThreshold)
        {
          LOG4CXX_ERROR(_logger, "Too many bad entries, load aborted!");
          TseUtil::alert("Too many bad entries, load aborted!");
          return;
        }
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
               "FileLoader::parse:parsed " << lineNumber
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
      LOG4CXX_INFO(_logger, "FileLoader::parse:total fares:" << numberFares);
    }
#endif // 0
}

} // tse namespace
