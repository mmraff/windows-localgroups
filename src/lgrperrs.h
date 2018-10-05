#ifndef __LGRPERRS_2_H__
#define __LGRPERRS_2_H__

#include <stdexcept>

class WinLGrpsError : public std::exception {
  public:
    WinLGrpsError(unsigned long hcode) noexcept : _code(hcode) {}
    WinLGrpsError& operator=(const WinLGrpsError& other) noexcept {
      _code = other._code;
      return *this;
    }
    unsigned long code() const noexcept { return _code; }
    virtual const char* what() const noexcept { return ""; }
  protected:
    unsigned long _code;
};

class APIError : public WinLGrpsError {
  public:
    APIError(unsigned long hcode) : WinLGrpsError(hcode) {}
    virtual const char* what() const noexcept;
};

class SysError : public WinLGrpsError {
  public:
    SysError(unsigned long hcode) : WinLGrpsError(hcode) {}
    virtual const char* what() const noexcept;
};

class UsageError : public WinLGrpsError {
  public:
    UsageError(unsigned long hcode) : WinLGrpsError(hcode) {}
    virtual const char* what() const noexcept;
};

#endif
