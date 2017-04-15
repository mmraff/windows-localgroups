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
        _pComment(NULL), _pSnag(NULL)
    {
      Nan::Utf8String* pName = new Nan::Utf8String(inArgs.GetGroupName());
      try { _pGrpNameW = getWideStrCopy(*(*pName)); }
      catch (Snag* pS) {
        _pSnag = pS;
        SetErrorMessage("ERROR");
      }
      delete pName;
      if (_pSnag) return;

      if (inArgs.HasHostName())
      {
        pName = new Nan::Utf8String(inArgs.GetHostName());
        try { _pSrvNameW = getWideStrCopy(*(*pName)); }
        catch (Snag* pS) {
          _pSnag = pS;
          SetErrorMessage("ERROR");
        }
        delete pName;
        if (_pSnag) {
          free(_pGrpNameW); _pGrpNameW = NULL;
        }
      }
    }

    ~GrpInfoWorker()
    {
      if (_pGrpNameW) free(_pGrpNameW);
      if (_pSrvNameW) free(_pSrvNameW);
      if (_pComment) delete _pComment;
      if (_pSnag) delete _pSnag;
    }

    void Execute() {
      if (_pSnag != NULL) return;

      try { _pComment = getLocalGroupComment(_pGrpNameW, _pSrvNameW); }
      catch (Snag* pS)
      {
        _pSnag = pS;
        SetErrorMessage("ERROR");
      }
    }

    void HandleErrorCallback() {
      const unsigned argc = 1;
      Local<Value> exc = (_pSnag->message() == NULL) ?
        Nan::ErrnoException(_pSnag->code(), NULL, "Unknown error") :
        Nan::Error(_pSnag->message());
      Local<Value> argv[argc] = { exc };
      callback->Call(argc, argv);
    }

    void HandleOKCallback()
    {
      const unsigned argc = 2;
      Local<Value> argv[argc] = {
        Nan::Undefined(),
        _pComment == NULL ? Nan::Null()
                          : Nan::New<String>(_pComment).ToLocalChecked()
      };
      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pGrpNameW;
    wchar_t* _pSrvNameW;
    char* _pComment;
    Snag* _pSnag;
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
    try {
      pGrpNameW = getWideStrCopy(*(Nan::Utf8String(args.GetGroupName())));
    }
    catch (Snag* pS)
    {
      Local<Value> err = (pS->message() != NULL) ?
        Nan::Error(pS->message()) :
        Nan::ErrnoException(pS->code(), NULL, "Unknown error from getWideStrCopy");
      delete pS;
      return Nan::ThrowError(err);
    }

    char* pComment = NULL;
    Local<Value> err = Nan::Null();
    try {
      pComment = getLocalGroupComment(pGrpNameW, NULL);
    }
    catch (Snag* pS)
    {
      err = (pS->message() != NULL) ?
        Nan::Error(pS->message()) :
        Nan::ErrnoException(pS->code(), NULL, "Unknown error from getMemberList");
      delete pS;
    }

    free(pGrpNameW);
    if (!err->IsNull()) return Nan::ThrowError(err);

    info.GetReturnValue().Set(Nan::New<String>(pComment).ToLocalChecked());
    delete pComment;
  }
}

