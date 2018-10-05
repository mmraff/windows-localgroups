#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "argsresolv.h"
#include "lgapi-utils.h"
#include "lgrplist.h"
#include "lgrperrs.h"

using namespace v8;

Local<Array> transformLGData(LocalGroupList* pList)
{
  Local<Array> results = Nan::New<Array>();
  size_t count = pList->Length();

  for (size_t i = 0; i < count; i++)
  {
    const struct LocalGroupInfo& currGrp = (*pList)[i];
    Local<Object> groupObj = Nan::New<Object>();

    groupObj->Set(Nan::New<String>("name").ToLocalChecked(),
                  Nan::New<String>(currGrp.name).ToLocalChecked());
      
    groupObj->Set(Nan::New<String>("comment").ToLocalChecked(),
                  currGrp.comment == NULL ? Nan::Null() :
                    Nan::New<String>(currGrp.comment).ToLocalChecked());

    (*results)->Set(i, groupObj);
  }

  return results;
}

class GrpEnumWorker : public Nan::AsyncWorker {
  public:
    GrpEnumWorker(LGArgsResolver& inArgs)
      : Nan::AsyncWorker(inArgs.GetCallback()), _pSrvNameW(NULL),
        _pList(NULL), _errCode(0)
    {
      if (inArgs.HasHostName())
      {
        Nan::Utf8String* pName = new Nan::Utf8String(inArgs.GetHostName());
        try { 
          _pSrvNameW = getWideStrCopy(*(*pName));
        }
        catch (WinLGrpsError& er) {
          _errCode = er.code();
          SetErrorMessage(er.what());
        }
        delete pName;
      }
    }

    ~GrpEnumWorker() {}

    void Execute()
    {
      if (_errCode != 0) return;

      try { _pList = getLocalGroupList(_pSrvNameW); }
      catch (WinLGrpsError& er) {
        _errCode = er.code();
        SetErrorMessage(er.what());
      }
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
        Nan::Null(), transformLGData(_pList)
      };
      delete _pList; _pList = NULL;
      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pSrvNameW;
    LocalGroupList* _pList;
    unsigned long _errCode;
};

NAN_METHOD(localGroupsList) {

  LGArgsResolver args(info);
  if (args.HasError()) return Nan::ThrowError(args.GetError());

  if (args.HasCallback())
  {
    GrpEnumWorker* pWorker = new GrpEnumWorker(args);
    Nan::AsyncQueueWorker(pWorker);
  }
  else
  {
    LocalGroupList* pList = NULL;
    try { pList = getLocalGroupList(NULL); }
    catch (WinLGrpsError& er)
    {
      Local<Value> exc = (er.what() == NULL) ?
        Nan::ErrnoException(er.code(), NULL, "Unknown error") :
        Nan::Error(er.what());
      return Nan::ThrowError(exc);
    }

    info.GetReturnValue().Set(transformLGData(pList));
    delete pList;
  }
}

