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
        _pList(NULL), _pSnag(NULL)
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

    ~MemberInfoWorker()
    {
      if (_pGrpNameW) free(_pGrpNameW);
      if (_pSrvNameW) free(_pSrvNameW);
      if (_pList) delete _pList;
      if (_pSnag) delete _pSnag;
    }

    void Execute() {
      if (_pSnag != NULL) return;

      try { _pList = getMemberList(_pGrpNameW, _pSrvNameW); }
      catch (Snag* pS)
      {
        _pSnag = pS;
        SetErrorMessage("ERROR");
      }
    }

    void HandleErrorCallback()
    {
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
        Nan::Null(), transformMemberData(_pList)
      };
      delete _pList; _pList = NULL;
      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pGrpNameW;
    wchar_t* _pSrvNameW;
    LGMemberList* _pList;
    Snag* _pSnag;
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

    LGMemberList* pList = NULL;
    Local<Value> err = Nan::Null();
    try {
      pList = getMemberList(pGrpNameW, NULL);
      assert(pList != NULL && "getMemberList() returned NULL!");
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

    info.GetReturnValue().Set(transformMemberData(pList));
    delete pList;
  }
}
