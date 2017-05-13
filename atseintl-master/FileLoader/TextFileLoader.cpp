//-------------------------------------------------------------------
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "FileLoader/TextFileLoader.h"

#include "Common/Logger.h"
#include "Common/TseUtil.h"
#include "FileLoader/BFUtils.h"
#include "FileLoader/GZStream.h"

namespace tse
{

static Logger
logger("atseintl.FileLoader.TextFileLoader");

TextFileLoader::TextFileLoader(const std::string& fileName, const std::string& desc)
  : _fileName(fileName),
    _desc(desc),
    _gzStream(new GZStream(fileName)),
    _badEntriesThreshold(BFUtils::getBadEntriesThreshold())
{
  LOG4CXX_INFO(logger, "TextFileLoader:fileName:" << fileName);
}

TextFileLoader::~TextFileLoader() { delete _gzStream; }

bool
TextFileLoader::parse()
{
  if (!parseStarting())
    return false;

  const std::streamsize bufferSz(1024000);
  char buffer[bufferSz];
  std::streamsize bytesRead(0);
  size_t lineNumber(0);
  size_t off(0);
  int numberOfBadEntries = 0;

  while ((bytesRead = _gzStream->read(buffer + off, bufferSz - off)) > 0)
  {
    const char* pBuffer = buffer;
    const char* const pBufferEnd = buffer + bytesRead + off;
    const char* pEndOfLine = pBufferEnd;
    while (pBuffer < pBufferEnd &&
           (pEndOfLine = std::find(pBuffer, pBufferEnd, '\n')) != pBufferEnd)
    {
      // process line
      if (!parseLine(pBuffer, pEndOfLine - pBuffer))
      {
        std::ostringstream msg;
        msg << _desc << "::parse:line #" << lineNumber << ":bad entry:";
        msg.write(pBuffer, pEndOfLine - pBuffer);
        LOG4CXX_ERROR(logger, msg.str());
        pBuffer = pEndOfLine + 1;
        ++lineNumber;
        ++numberOfBadEntries;
        if (numberOfBadEntries >= _badEntriesThreshold)
        {
          LOG4CXX_ERROR(logger, _desc << "Too many bad entries, load aborted!");
          TseUtil::alert("Too many bad entries, load aborted!");
          return false;
        }
        continue;
      }
      ++lineNumber;
      pBuffer = pEndOfLine + 1;
    }
    off = pBufferEnd - pBuffer;
    memcpy(buffer, pBuffer, off);
  }
  LOG4CXX_INFO(logger, _desc << "::parse:parsed " << lineNumber << " lines");

  if (!parseFinished())
    return false;

  return true;

} // end of parse

bool
TextFileLoader::parseStarting()
{
  return true;
}

bool
TextFileLoader::parseFinished()
{
  return true;
}

} // end namespace
