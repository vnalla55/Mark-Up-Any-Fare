//----------------------------------------------------------------------------
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Util/CompressUtil.h"

#include "Common/Logger.h"

#include <string>

#include <bzlib.h>
#include <zlib.h>

namespace tse
{
bool
CompressUtil::compress(std::string& buff)
{
  std::vector<char> tmp(buff.begin(), buff.end());
  const bool res = compress(tmp);

  if (res == false)
  {
    return false;
  }

  buff.assign(tmp.begin(), tmp.end());

  return true;
}

bool
CompressUtil::compress(const std::string& input, std::string& output)
{
  std::vector<char> tmp(input.begin(), input.end());
  const bool res = compress(tmp);

  if (res == false)
  {
    return false;
  }

  output.assign(tmp.begin(), tmp.end());

  return true;
}

bool
CompressUtil::compress(std::vector<char>& buf)
{
  // destination buffer must be size of input buffer + 0.1% + 12
  // according to zlib documentation
  std::vector<char> dst(12 + buf.size() + buf.size() / 1000);

  uLongf len = dst.size();
  const Bytef* const from = reinterpret_cast<const Bytef*>(&buf[0]);
  Bytef* const to = reinterpret_cast<Bytef*>(&dst[0]);
  const int res = ::compress(to, &len, from, buf.size());
  switch (res)
  {
  case Z_OK:
    dst.resize(len);
    buf.swap(dst);
    return true;

  case Z_BUF_ERROR:
  {
    log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
    LOG4CXX_FATAL(_logger, "Error compressing data: buffer not big enough\n");
  }
    return false;

  case Z_MEM_ERROR:
  {
    log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
    LOG4CXX_FATAL(_logger, "Error compressing data: memory error\n");
    return false;
  }

  default:
  {
    log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
    LOG4CXX_FATAL(_logger, "Error compressing data: unknown error " << res << "\n");
    return false;
  }
  }
}

bool
CompressUtil::decompress(std::string& buff)
{
  std::vector<char> tmp(buff.begin(), buff.end());

  const bool res = decompress(tmp);
  if (res == false)
  {
    return false;
  }

  buff.assign(tmp.begin(), tmp.end());

  return true;
}

bool
CompressUtil::decompress(const std::string& input, std::string& output)
{
  std::vector<char> tmp(input.begin(), input.end());

  const bool res = decompress(tmp);
  if (res == false)
  {
    return false;
  }

  output.assign(tmp.begin(), tmp.end());

  return true;
}

bool
CompressUtil::decompress(std::vector<char>& buf)
{
  std::vector<char> dst(10 * buf.size());
  for (;;)
  {
    dst.resize(dst.capacity());
    uLongf len = dst.size();
    const Bytef* const from = reinterpret_cast<const Bytef*>(&buf[0]);
    Bytef* const to = reinterpret_cast<Bytef*>(&dst[0]);
    const int res = ::uncompress(to, &len, from, buf.size());
    switch (res)
    {
    case Z_OK:
      dst.resize(len);
      buf.swap(dst);
      return true;

    case Z_BUF_ERROR:
      dst.resize(dst.size() * 2);
      break;

    case Z_MEM_ERROR:
    {
      log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
      LOG4CXX_FATAL(_logger, "Error decompressing data: memory error\n");
      return false;
    }

    case Z_DATA_ERROR:
    {
      log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
      LOG4CXX_FATAL(_logger, "Error decompressing data: data is corrupt\n");
      return false;
    }
    default:
    {
      log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
      LOG4CXX_FATAL(_logger, "Error decompressing data: unknown error\n");
      return false;
    }
    }
  }
}

const char*
CompressUtil::xlateBz2StatusCode(int rcode)
{
  switch (rcode)
  {
  case BZ_OK:
    return "OK";
  case BZ_RUN_OK:
    return "RUN OK";
  case BZ_FLUSH_OK:
    return "FLUSH OK";
  case BZ_FINISH_OK:
    return "FINISH OK";
  case BZ_STREAM_END:
    return "STREAM END";
  case BZ_SEQUENCE_ERROR:
    return "SEQUENCE ERROR";
  case BZ_PARAM_ERROR:
    return "PARAM ERROR";
  case BZ_MEM_ERROR:
    return "MEM ERROR";
  case BZ_DATA_ERROR:
    return "DATA ERROR";
  case BZ_DATA_ERROR_MAGIC:
    return "DATA ERROR MAGIC";
  case BZ_IO_ERROR:
    return "IO ERROR";
  case BZ_UNEXPECTED_EOF:
    return "UNEXPECTED EOF";
  case BZ_OUTBUFF_FULL:
    return "OUTBUFF FULL";
  case BZ_CONFIG_ERROR:
    return "CONFIG ERROR";
  default:
    return "UNKNOWN";
  }
}

bool
CompressUtil::compressBz2(std::vector<char>& inVec)
{
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
  LOG4CXX_INFO(_logger, "compressBz2 start, " << inVec.size() << " bytes");

  if (inVec.size() == 0)
    return false;

  std::vector<char> outVec(2 * inVec.size());

  bz_stream strm;
  strm.bzalloc = nullptr;
  strm.bzfree = nullptr;
  strm.opaque = nullptr;
  BZ2_bzCompressInit(&strm, 9, 0, 0);

  strm.next_in = reinterpret_cast<char*>(&inVec[0]);
  strm.avail_in = unsigned(inVec.size());
  strm.next_out = reinterpret_cast<char*>(&outVec[0]);
  strm.avail_out = unsigned(outVec.size());

  int rcode1 = BZ2_bzCompress(&strm, BZ_FINISH);
  int rcode2 = BZ2_bzCompressEnd(&strm);
  if (rcode1 == BZ_STREAM_END && rcode2 == BZ_OK)
  {
    outVec.resize(strm.total_out_lo32);
    inVec.swap(outVec);
    LOG4CXX_INFO(_logger, "compressBz2 complete, " << strm.total_out_lo32 << " bytes");
    return true;
  }

  LOG4CXX_FATAL(_logger,
                "Unexpected BZIP2 status: " << xlateBz2StatusCode(rcode1) << ", "
                                            << xlateBz2StatusCode(rcode2));
  return false;
}

bool
CompressUtil::compressBz2(std::string& buf)
{
  std::vector<char> tmp(buf.begin(), buf.end());
  const bool res = compressBz2(tmp);

  if (res == false)
  {
    return false;
  }

  buf.assign(tmp.begin(), tmp.end());

  return true;
}

bool
CompressUtil::decompressBz2(std::vector<char>& inVec)
{
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("atseintl.Common.CompressUtil"));
  LOG4CXX_INFO(_logger, "decompressBz2 start, " << inVec.size() << " bytes");

  if (inVec.size() == 0)
    return false;

  std::vector<char> outVec(15 * inVec.size());

  bz_stream strm;
  strm.bzalloc = nullptr;
  strm.bzfree = nullptr;
  strm.opaque = nullptr;
  BZ2_bzDecompressInit(&strm, 0, 0);

  strm.next_in = reinterpret_cast<char*>(&inVec[0]);
  strm.avail_in = unsigned(inVec.size());
  strm.next_out = reinterpret_cast<char*>(&outVec[0]);
  strm.avail_out = unsigned(outVec.size());

  int rcode1;
  do
  {
    rcode1 = BZ2_bzDecompress(&strm);
    if (rcode1 == BZ_OK && strm.avail_in > 0 && strm.avail_out == 0)
    {
      size_t oldSize = outVec.size();
      outVec.resize(oldSize * 2);
      strm.next_out = reinterpret_cast<char*>(&outVec[0] + strm.total_out_lo32);
      strm.avail_out = unsigned(outVec.size() - strm.total_out_lo32);
    }
  } while (rcode1 == BZ_OK);

  int rcode2 = BZ2_bzDecompressEnd(&strm);
  if (rcode1 == BZ_STREAM_END && rcode2 == BZ_OK)
  {
    outVec.resize(strm.total_out_lo32);
    inVec.swap(outVec);
    LOG4CXX_INFO(_logger, "decompressBz2 complete, " << strm.total_out_lo32 << " bytes");
    return true;
  }
  LOG4CXX_FATAL(_logger,
                "Unexpected BZIP2 status: " << xlateBz2StatusCode(rcode1) << ", "
                                            << xlateBz2StatusCode(rcode2));
  return false;
}

bool
CompressUtil::decompressBz2(std::string& buf)
{
  std::vector<char> tmp(buf.begin(), buf.end());
  const bool res = decompressBz2(tmp);

  if (res == false)
  {
    return false;
  }

  buf.assign(tmp.begin(), tmp.end());

  return true;
}
}
