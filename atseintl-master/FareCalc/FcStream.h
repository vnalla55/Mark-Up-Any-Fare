//----------------------------------------------------------------------------
//  File:        FcStream.h
//  Authors:     Quan Ta
//  Created:     Feb 20, 2006
//
//  Description: Fare Calc Line buffer
//
//  Updates:
//          date - initials - description.
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
#pragma once

#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TicketStock.h"
#include "FareCalc/FareCalcConsts.h"

#include <cstring>
#include <sstream>
#include <streambuf>
#include <string>

namespace tse
{
namespace FareCalc
{

class Margin;
class Group;

class FcStreamBuf : public std::streambuf
{
  friend class FareCalc::Margin;
  friend class FareCalc::Group;

public:
  explicit FcStreamBuf(PricingTrx* trx, int width, int im)
    : std::streambuf(), _width(width), _im(im), _im_str(_im, ' '), _trx(trx)
  {
    _im_enable = (_im > 0);

    if (trx)
    {
      _isTicketEntry = trx->getRequest()->isTicketEntry();
      _isLowFareRequested = trx->getRequest()->isLowFareRequested();

      if (_isTicketEntry)
      {
        if (trx->getRequest()->ticketingAgent() &&
            !trx->getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
            trx->getRequest()->electronicTicket())
        {
          return;
        }
        int16_t ticketStk = trx->getOptions()->ticketStock();
        if (trx->getOptions()->ticketStock() >= FareCalcConsts::BSP_MIN_TICKET_STOCK_NUMBER &&
            trx->getOptions()->ticketStock() <= FareCalcConsts::BSP_MAX_TICKET_STOCK_NUMBER)
        {
          ticketStk = FareCalcConsts::BSP_TICKET_STOCK_NUMBER;
        }

        if ((!ticketStk) && trx->getRequest()->electronicTicket())
          ticketStk = FareCalcConsts::WORLD_PRICING_TICKET_STOCK_NUMBER;

        const std::vector<TicketStock*>& ticketStock =
            trx->dataHandle().getTicketStock(ticketStk, trx->getRequest()->ticketingDT());

        if (!ticketStock.empty() && ticketStock.front())
        {
          _ticketStock = ticketStock.front();
          _ticketStockLength = _ticketStock->totalCharacters();

          if (trx->getOptions()->ticketStock() >= FareCalcConsts::ATB_MIN_TICKET_STOCK_NUMBER &&
              trx->getOptions()->ticketStock() <= FareCalcConsts::ATB_MAX_TICKET_STOCK_NUMBER &&
              trx->getRequest()->lengthATBFareCalc() > 0)
          {
            _ticketStockLength = trx->getRequest()->lengthATBFareCalc();
          }
        }
      }
    }
  }

  std::string str() const { return _os.str() + _nos.str(); }
  std::string ufstr() const { return _ufos.str() + _nos.str(); }
  int lineLength() const { return _len; } // current line's length
  char lastChar() const { return _last; }

  void clear()
  {
    _os.str("");
    _nos.str("");
    _ufos.str("");
    _len = 0;
    _last = 0;
  }

protected:
  // NOTE: the only reason for this function is to get around a bug
  // in libstdc++ streambuf in handling of std::endl and numeric.
  int overflow(int c) override
  {
    if (UNLIKELY(_isInGroup))
    {
      _group << (char)c;
      _last = (char)c;
      return c;
    }

    if (c == '\n')
    {
      const std::string& nosStr = _nos.str();
      if (nosStr.length() > 0)
      {
        save(nosStr.data(), nosStr.length());
        _nos.str("");
      }
      newline();
    }
    else if (UNLIKELY(c == '\r'))
    {
      return c;
    }
    else
    {
      // only digits get here, store it in the digit bucket
      _last = (char)c;
      _nos << (char)c;
    }
    return c;
  }

  std::streamsize xsputn(const char* ptr, std::streamsize count) override
  {
    if (_isInGroup)
    {
      _group.write(ptr, count);
      _last = ptr[count - 1];
      return count;
    }

    const std::string& nosStr = _nos.str();

    if (!nosStr.empty())
    {
      save(nosStr.data(), nosStr.length());
      _nos.str("");
    }

    char* p = nullptr;
    if ((p = (char*)memchr(ptr, '\n', count)) != nullptr)
    {
      for (int n = count; n > 0; n -= (p - ptr + 1), ptr = (p + 1), p = (char*)memchr(ptr, '\n', n))
      {
        if (p == nullptr)
        {
          save(ptr, n);
          break;
        }
        if (LIKELY((p - ptr) >= 0))
        {
          save(ptr, (p - ptr));
          save("\n", 1); // should keep the nl separate to preserve
          // the wrapping logic
        }
      }
    }
    else
    {
      save(ptr, count);
    }

    return count;
  }

  void save(const char* ptr, std::streamsize count)
  {
    // std::cout << "\n_len: " << _len
    //          << "\n         1         2         3         4         5         6   "
    //          << "\n123456789012345678901234567890123456789012345678901234567890123"
    //          << "\n" << _os.str()
    //          << "\nptr: [";
    // std::cout.write(ptr, count);
    // std::cout << "]\ncount: " << count << std::endl;

    if (count <= 0)
      return;

    saveTktFcl(ptr, count);

    if (UNLIKELY(count == 1 && *ptr == '\r'))
      return;

    if (count == 1 && *ptr == '\n')
      return newline();

    if (_len > _im && _len + count > _width)
      newline();

    if (indent() && _len == 1 && *ptr == ' ')
    {
      _os.write(++ptr, --count);
      if (count > 0)
        _last = ptr[count - 1];
    }
    else
    {
      _os.write(ptr, count);
      _last = ptr[count - 1];
    }
    _len += count;
  }

  void newline()
  {
    if (_len == 0)
      _os << "  \n"; // empty-line need at least a space
    else
      _os << '\n';

    _len = 0;
    _last = 0;
  }

  bool indent()
  {
    if (_im_enable && _len == 0 && _os.str().length() > 0)
    {
      _os << _im_str;
      _last = _im_str[_im_str.length() - 1];
      _len = _im;
      return true;
    }
    return false;
  }

private:
  void saveTktFcl(const char* ptr, std::streamsize count)
  {
    // Do not want that leading space for ticket.
    if (_tktCurLen == 0 && count > 0 && *ptr == ' ')
    {
      return saveTktFcl(ptr + 1, count - 1);
    }

    unsigned width = 9999;

    if (UNLIKELY(_trx && _isTicketEntry && _ticketStock && !_isLowFareRequested))
    {
      if ((_tktCurLen + count) >= _ticketStockLength)
      {
        return;
      }
      width = (unsigned)_ticketStock->lineLength(_tktLineNo);
    }

    if (LIKELY((_tktCurLen + count) < width))
    {
      _ufos.write(ptr, count);
      _tktCurLen += count;
    }
    else
    {
      for (int i = 0, n = (width - _tktCurLen); i < n; i++)
      {
        _ufos << ' ';
      }

      _ufos.write(ptr, count);
      _tktCurLen = count;
      _tktLineNo++;
    }
  }

private:
  void enableInternalMargin(int im)
  {
    _im = im;
    _im_enable = (_im > 0);
    _im_str.assign(_im, ' ');
  }
  void enableInternalMargin(const char* im)
  {
    _im_str = im;
    _im = _im_str.length();
    _im_enable = (_im > 0);
  }
  void disableInternalMargin() { _im_enable = false; }

private:
  void startGroup()
  {
    _group.str("");
    _isInGroup = true;
  }
  void endGroup()
  {
    _isInGroup = false;
    const std::string& groupStr = _group.str();
    if (LIKELY(!groupStr.empty()))
    {
      const char* ptr = groupStr.data();
      unsigned length = groupStr.length();
      for (; length > 0 && *ptr == ' '; ++ptr, --length)
      {
        xsputn(" ", 1);
      }
      if (LIKELY(length > 0))
        xsputn(ptr, length);
      _group.str("");
    }
  }
  const char* group() { return _group.str().data(); }

private:
  unsigned _width;
  unsigned _len = 0;
  char _last = 0;
  unsigned _im; // internal left margin when wrapping
  bool _im_enable;
  std::string _im_str;
  std::ostringstream _os;
  std::ostringstream _ufos; // the unformatted version for fareCalculationLine
  std::ostringstream _nos; // streambuf's bug work around - digit bucket

  bool _isInGroup = false;
  std::ostringstream _group; // holding place for unbreakable group of item

  unsigned _tktLineNo = 1;
  PricingTrx* _trx;
  TicketStock* _ticketStock = nullptr;
  unsigned _ticketStockLength = 0;
  unsigned _tktCurLen = 0;
  bool _isTicketEntry = false;
  bool _isLowFareRequested = false;
};

class FcStream_InitBuf
{
protected:
  FcStreamBuf _buf;

  explicit FcStream_InitBuf(PricingTrx* trx, int width, int im) : _buf(trx, width, im) {}
};

class FcStream : private FcStream_InitBuf, public std::ostream
{
  friend class FareCalc::Margin;
  friend class FareCalc::Group;

public:
  explicit FcStream(PricingTrx* trx = nullptr,
                    int width = FareCalcConsts::FCL_MAX_LINE_LENGTH,
                    int im = 0)
    : FcStream_InitBuf(trx, width, im), std::ostream(&_buf)
  {
  }

  std::string str() const { return _buf.str(); }
  std::string ufstr() const { return _buf.ufstr(); }
  int lineLength() const { return _buf.lineLength(); }
  char lastChar() const { return _buf.lastChar(); }

  bool lastCharAlpha() { return isalpha(lastChar()); }
  bool lastCharDigit() { return isdigit(lastChar()); }
  bool lastCharSpace() { return isspace(lastChar()); }
  bool lastCharSpecial()
  {
    char c = lastChar();
    return (c == '*' || c == '(' || c == ')');
  }

  void clear() { _buf.clear(); }

  int split(std::vector<std::string>& lines)
  {
    std::string tmpBuf(str());
    const char* ptr = tmpBuf.data();
    unsigned int count = tmpBuf.length();
    char* p = nullptr;

    if ((p = (char*)memchr(ptr, '\n', count)) != nullptr)
    {
      for (int n = count; n > 0; n -= (p - ptr + 1), ptr = (p + 1), p = (char*)memchr(ptr, '\n', n))
      {
        if (p == nullptr)
        {
          lines.push_back(std::string(ptr, n));
          break;
        }
        if ((p - ptr) >= 0)
        {
          lines.push_back(std::string(ptr, (p - ptr)));
        }
      }
    }
    else
    {
      lines.push_back(std::string(ptr, count));
    }
    return lines.size();
  }

  void displayMessage(const std::string& str) { displayMessage(str.data()); }

  void displayMessage(const char* ptr)
  {
    int count = strlen(ptr);
    char* p = nullptr;
    if ((p = (char*)memchr(ptr, ' ', count)) != nullptr)
    {
      for (int n = count; n > 0; n -= (p - ptr + 1), ptr = p + 1, p = (char*)memchr(ptr, ' ', n))
      {
        if (p == nullptr)
        {
          this->write(ptr, n);
          break;
        }
        if (LIKELY((p - ptr) >= 0))
        {
          this->write(ptr, p - ptr);
          (*this) << ' ';
        }
      }
    }
    else
    {
      this->write(ptr, count);
    }
  }

private:
  FcStreamBuf* operator->() { return &_buf; }
};

class Margin
{
public:
  Margin(FcStream& stream, int size) : _stream(stream) { _stream->enableInternalMargin(size); }
  Margin(FcStream& stream, const char* margin) : _stream(stream)
  {
    _stream->enableInternalMargin(margin);
  }
  ~Margin() { _stream->disableInternalMargin(); }
  void setMargin(int size) { _stream->enableInternalMargin(size); }
  void setMargin(const char* margin) { _stream->enableInternalMargin(margin); }

private:
  FcStream& _stream;
};

class Group
{
public:
  Group(FcStream& stream, bool enable = false) : _stream(stream), _enable(enable)
  {
    if (_enable)
      _stream->startGroup();
  }
  ~Group()
  {
    if (_enable)
      _stream->endGroup();
  }

  void startGroup()
  {
    _enable = true;
    _stream->startGroup();
  }
  void endGroup()
  {
    _enable = false;
    _stream->endGroup();
  }
  const char* group() { return _stream->group(); }

private:
  FcStream& _stream;
  bool _enable;
};
}
} // namespace tse::FareCalc
