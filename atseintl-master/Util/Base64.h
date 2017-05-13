
//*********************************************************************
//* C_Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*********************************************************************

#pragma once

#include <stdexcept>
#include <string>

namespace tse
{
class Base64
{
public:
  static std::string encode(const std::string& data);
  static std::string decode(const std::string& data);

  class DecodeError : public std::runtime_error
  {
  public:
    DecodeError();
  };
};
}

