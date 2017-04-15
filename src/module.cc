#include <nan.h>

using namespace v8;

NAN_METHOD(localGroupsList);
NAN_METHOD(localGroupComment);
NAN_METHOD(localGroupMembers);

void init(Handle<Object> exports) {
  exports->Set(Nan::New<String>("list").ToLocalChecked(),
               Nan::New<FunctionTemplate>(localGroupsList)->GetFunction());

  exports->Set(Nan::New<String>("getComment").ToLocalChecked(),
               Nan::New<FunctionTemplate>(localGroupComment)->GetFunction());

  exports->Set(Nan::New<String>("getMembers").ToLocalChecked(),
               Nan::New<FunctionTemplate>(localGroupMembers)->GetFunction());
}

NODE_MODULE(localgroups, init);
