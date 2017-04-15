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
        _pList(NULL), _pSnag(NULL)
    {
      if (inArgs.HasHostName())
      {
        Nan::Utf8String* pName = new Nan::Utf8String(inArgs.GetHostName());
        try { 
          _pSrvNameW = getWideStrCopy(*(*pName));
        }
        catch (Snag* pS) {
          _pSnag = pS;
          SetErrorMessage("ERROR");
        }
        delete pName;
      }
    }

    ~GrpEnumWorker()
    {
      if (_pSrvNameW) free(_pSrvNameW);
      if (_pList) delete _pList;
      if (_pSnag) delete _pSnag;
    }

    void Execute()
    {
      if (_pSnag != NULL) {
        //fprintf(stderr, "Error was set in constructor, but Execute gets called anyway!?\n");
        return;
      }

      try { _pList = getLocalGroupList(_pSrvNameW); }
      catch (Snag* pS)
      {
        _pSnag = pS;
        SetErrorMessage("OUCH");
      }
    }

    void HandleErrorCallback()
    {
      const unsigned argc = 1;
      Local<Value> exc = (_pSnag->message() == NULL) ?
        Nan::ErrnoException(_pSnag->code(), NULL, "Unknown error") :
        Nan::Error(_pSnag->message());
      Local<Value> argv[argc] = { exc };
      // FAILED EXPERIMENT (tried arrow too):
      //argv[0].Set(Nan::New<String>("code").ToLocalChecked(),
      //             Nan::New<String>("EIEIO").ToLocalChecked());
      callback->Call(argc, argv);
    }

    void HandleOKCallback()
    {
      const unsigned argc = 2;
      Local<Value> argv[argc] = {
        Nan::Undefined(), Nan::New(transformLGData(_pList))
      };
      delete _pList; _pList = NULL;
      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pSrvNameW;
    LocalGroupList* _pList;
    Snag* _pSnag;
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
    catch (Snag* pSnag)
    {
      Local<Value> exc = (pSnag->message() == NULL) ?
        Nan::ErrnoException(pSnag->code(), NULL, "Unknown error") :
        Nan::Error(pSnag->message());
      delete pSnag;
      return Nan::ThrowError(exc);
    }

    info.GetReturnValue().Set(transformLGData(pList));
    delete pList;
  }
}

