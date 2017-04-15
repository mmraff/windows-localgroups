#ifndef __ARGSRESOLV_H__
#define __ARGSRESOLV_H__

class LGArgsResolver {
  public:
    explicit LGArgsResolver(const Nan::FunctionCallbackInfo<v8::Value>&);
    explicit LGArgsResolver(const Nan::FunctionCallbackInfo<v8::Value>&, bool);
    inline ~LGArgsResolver() {}
    inline bool HasHostName() { return _hostIdx != -1; }
    inline bool HasCallback() { return _cbIdx != -1; }
    inline bool HasError() { return _error->IsNull() == false; }
    v8::Local<v8::String> GetGroupName();
    v8::Local<v8::String> GetHostName();
    Nan::Callback* GetCallback();
    inline v8::Local<v8::Value> GetError() { return _error; }

  private:
    const Nan::FunctionCallbackInfo<v8::Value>& _info;
    v8::Local<v8::Value> _error;
    bool _hasGroup;
    int _hostIdx;
    int _cbIdx;

    void init(bool);

    // Prevent copy constructing - not in our best interest
    LGArgsResolver(const LGArgsResolver&);
    void operator=(const LGArgsResolver&);
};

#endif
