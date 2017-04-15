#include <nan.h>
#include "argsresolv.h"

using namespace v8;

LGArgsResolver::LGArgsResolver(
  const Nan::FunctionCallbackInfo<Value>& callerInfo
) : _info(callerInfo), _hasGroup(false), _hostIdx(-1), _cbIdx(-1), _error(Nan::Null())
{
  init(false);
}

LGArgsResolver::LGArgsResolver(
  const Nan::FunctionCallbackInfo<Value>& callerInfo,
  bool requireGroupName
) : _info(callerInfo), _hasGroup(false), _hostIdx(-1), _cbIdx(-1), _error(Nan::Null())
{
  init(requireGroupName);
}

void LGArgsResolver::init(bool requireGroupName)
{
  int currIdx = 0;

  if (requireGroupName)
  {
    if (_info.Length() == 0 || _info[0]->IsUndefined() || _info[0]->IsNull()) {
      _error = Nan::SyntaxError("Must provide name of local group");
      return;
    }

    if (!(_info[0]->IsString() || _info[0]->IsStringObject())) {
      _error = Nan::TypeError("Invalid type for group name argument");
      return;
    }

    if (_info[0]->ToString()->Length() == 0) {
      _error = Nan::SyntaxError("Group name cannot be empty");
      return;
    }

    _hasGroup = true;
    currIdx++;
  }

  if (_info.Length() > currIdx)
  {
    if (_info[currIdx]->IsFunction())
    {
      // Saw callback first, so no hostname
      //fprintf(stderr, "Arg %d is a function.\n", currIdx + 1);
      _cbIdx = currIdx;
    }
    else if (_info[currIdx]->IsString() || _info[currIdx]->IsStringObject())
    {
      if (_info[currIdx]->ToString()->Length() != 0)
      {
        _hostIdx = currIdx;
      }
      // else this arg is empty, so we leave hostname arg NULL.
      //else
      //  fprintf(stderr, "Arg %d is an empty string.\n", currIdx + 1);
    }
    else if (!(_info[currIdx]->IsUndefined() || _info[currIdx]->IsNull()))
    {
      if (currIdx == 0)
        _error = Nan::TypeError("Invalid type for first argument");
      else
        _error = Nan::TypeError("Invalid type for second argument");
      return;
    }
    currIdx++;

    if (_info.Length() > currIdx && _cbIdx == -1)
    {
      if (_info[currIdx]->IsFunction())
      {
        //fprintf(stderr, "Arg %d is a function.\n", currIdx + 1);
        _cbIdx = currIdx;
      }
      // Else? I'm considering throwing an error here, different
      // from missing callback: "too many arguments" or "bad argument list"
      else
        fprintf(stderr, "Warning: ignoring extra argument(s)");
    }

    // It's only OK to leave out the callback arg when hostname arg is empty.
    if (_hostIdx != -1 && _cbIdx == -1)
      _error = Nan::SyntaxError("Must provide callback when giving a hostname");
  }
}

Local<String> LGArgsResolver::GetGroupName()
{
  assert(_hasGroup && "Attempt to GetGroupName() when there is none!");

  return _info[0].As<String>();
}

Local<String> LGArgsResolver::GetHostName()
{
  assert(_hostIdx != -1 && "Attempt to GetHostName() when there is none!");

  return _info[_hostIdx].As<String>();
}

Nan::Callback* LGArgsResolver::GetCallback()
{
  assert(_cbIdx != -1 && "Attempt to GetCallback() when there is none!");

  return new Nan::Callback(_info[_cbIdx].As<Function>());
}
