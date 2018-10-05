#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "argsresolv.h"
#include "lgapi-utils.h"
#include "lgmemblist.h"
#include "lgrperrs.h"

using namespace v8;

Local<Array> transformMemberData(LGMemberList* pList)
{
  assert(pList != NULL && "transformMemberData: arg is NULL");

  Local<Array> results = Nan::New<Array>();
  size_t count = pList->Length();

  for (size_t i = 0; i < count; i++)
  {
    const struct LocGrpMemberInfo& currMember = (*pList)[i];
    Local<Object> groupObj = Nan::New<Object>();
      
    groupObj->Set(Nan::New<String>("sid").ToLocalChecked(),
                  Nan::New<String>(currMember.sid).ToLocalChecked());

    groupObj->Set(Nan::New<String>("accountType").ToLocalChecked(),
                  Nan::New<String>(currMember.accountType).ToLocalChecked());
      
    groupObj->Set(Nan::New<String>("domainAndName").ToLocalChecked(),
                  Nan::New<String>(currMember.domainAndName).ToLocalChecked());

    (*results)->Set(i, groupObj);
  }

  return results;
}

class MemberInfoWorker : public Nan::AsyncWorker {
  public:
    MemberInfoWorker(LGArgsResolver& inArgs)
      : Nan::AsyncWorker(inArgs.GetCallback()), _pGrpNameW(NULL), _pSrvNameW(NULL),
        _pList(NULL), _errCode(0)
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

    ~MemberInfoWorker() {}

    void Execute() {
      if (_errCode != 0) return;

      try { _pList = getMemberList(_pGrpNameW, _pSrvNameW); }
      catch (WinLGrpsError& er) {
        _errCode = er.code();
        SetErrorMessage(er.what());
      }
      free(_pGrpNameW);
      if (_pSrvNameW) free(_pSrvNameW);
    }

    void HandleErrorCallback()
    {
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
        Nan::Null(), transformMemberData(_pList)
      };
      delete _pList; _pList = NULL;
      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pGrpNameW;
    wchar_t* _pSrvNameW;
    LGMemberList* _pList;
    unsigned long _errCode;
};

NAN_METHOD(localGroupMembers) {

  LGArgsResolver args(info, true);
  if (args.HasError()) return Nan::ThrowError(args.GetError());

  if (args.HasCallback())
  {
    MemberInfoWorker* pWorker = new MemberInfoWorker(args);
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

    LGMemberList* pList = NULL;
    try {
      pList = getMemberList(pGrpNameW, NULL);
      assert(pList != NULL && "getMemberList() returned NULL!");
    }
    catch (WinLGrpsError& er)
    {
      exc = (er.what() == NULL) ?
        Nan::ErrnoException(er.code(), NULL, "Unknown error (ref:B)") :
        Nan::Error(er.what());
    }

    free(pGrpNameW);
    if (!exc->IsNull()) return Nan::ThrowError(exc);

    info.GetReturnValue().Set(transformMemberData(pList));
    delete pList;
  }
}
