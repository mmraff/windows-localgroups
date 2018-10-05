#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "argsresolv.h"
#include "lgapi-utils.h"
#include "lgrperrs.h"

using namespace v8;

// lgrpinfo.cc is not complicated enough to justify a header file, so:
char* getLocalGroupComment(const wchar_t*, const wchar_t*);

class GrpInfoWorker : public Nan::AsyncWorker {
  public:
    GrpInfoWorker(LGArgsResolver& inArgs)
      : Nan::AsyncWorker(inArgs.GetCallback()), _pGrpNameW(NULL), _pSrvNameW(NULL),
        _pComment(NULL), _errCode(0)
    {
      Nan::Utf8String* pName = new Nan::Utf8String(inArgs.GetGroupName());
      try { _pGrpNameW = getWideStrCopy(*(*pName)); }
      catch (WinLGrpsError& er) {
        _errCode = er.code();
        SetErrorMessage(er.what());
      }
      delete pName;
      if (_errCode) return;

      if (inArgs.HasHostName())
      {
        pName = new Nan::Utf8String(inArgs.GetHostName());
        try { _pSrvNameW = getWideStrCopy(*(*pName)); }
        catch (WinLGrpsError& er) {
          _errCode = er.code();
          SetErrorMessage(er.what());
        }
        delete pName;
        if (_errCode) {
          free(_pGrpNameW); _pGrpNameW = NULL;
        }
      }
    }

    ~GrpInfoWorker() {}

    void Execute() {
      if (_errCode != 0) return;

      try { _pComment = getLocalGroupComment(_pGrpNameW, _pSrvNameW); }
      catch (WinLGrpsError& er) {
        _errCode = er.code();
        SetErrorMessage(er.what());
      }
      free(_pGrpNameW);
      if (_pSrvNameW) free(_pSrvNameW);
    }

    void HandleErrorCallback() {
      const unsigned argc = 1;
      Local<Value> exc = (this->ErrorMessage() == NULL) ?
        Nan::ErrnoException(_errCode, NULL, "Unknown error") :
        Nan::Error(this->ErrorMessage());
      Local<Value> argv[argc] = { exc };
      callback->Call(argc, argv);
    }

    void HandleOKCallback()
    {
      const unsigned argc = 2;
      Local<Value> argv[argc] = {
        Nan::Null(),
        _pComment == NULL ? Nan::Null()
                          : Nan::New<String>(_pComment).ToLocalChecked()
      };
      if (_pComment) delete _pComment;
      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pGrpNameW;
    wchar_t* _pSrvNameW;
    char* _pComment;
    unsigned long _errCode;
};

NAN_METHOD(localGroupComment) { // I'm thinking of calling it 'description' instead...

  LGArgsResolver args(info, true);
  if (args.HasError()) return Nan::ThrowError(args.GetError());

  if (args.HasCallback())
  {
    GrpInfoWorker* pWorker = new GrpInfoWorker(args);
    Nan::AsyncQueueWorker(pWorker);
  }
  else
  {
    wchar_t* pGrpNameW = NULL;
    Local<Value> exc = Nan::Null();
    try {
      pGrpNameW = getWideStrCopy(*(Nan::Utf8String(args.GetGroupName())));
    }
    catch (WinLGrpsError& er)
    {
      exc = (er.what() == NULL) ?
        Nan::ErrnoException(er.code(), NULL, "Unknown error (ref:A)") :
        Nan::Error(er.what());
      return Nan::ThrowError(exc);
    }

    char* pComment = NULL;
    try {
      pComment = getLocalGroupComment(pGrpNameW, NULL);
    }
    catch (WinLGrpsError& er)
    {
      exc = (er.what() == NULL) ?
        Nan::ErrnoException(er.code(), NULL, "Unknown error (ref:B)") :
        Nan::Error(er.what());
    }

    free(pGrpNameW);
    if (!exc->IsNull()) return Nan::ThrowError(exc);

    info.GetReturnValue().Set(Nan::New<String>(pComment).ToLocalChecked());
    delete pComment;
  }
}

