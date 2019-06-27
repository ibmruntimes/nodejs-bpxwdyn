/*
 * Licensed Materials - Property of IBM
 * (C) Copyright IBM Corp. 2019. All Rights Reserved.
 * US Government Users Restricted Rights - Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if defined(__MVS__)
#include <_Nascii.h>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#else
#error This addon is for ZOS only
#endif

#if ' ' == 0x40
#error Not compiled with -qascii
#endif

class __ae_runmode {
  int mode;

public:
  __ae_runmode(int new_mode) { mode = __ae_thread_swapmode(new_mode); }
  ~__ae_runmode() { __ae_thread_swapmode(mode); }
};

class bulk_allocator {
private:
  char *p = 0;
  int allocated_size;
  int offset;

public:
  bulk_allocator(size_t bulksize) : allocated_size(bulksize), offset(0) {
    p = (char *)__malloc31(bulksize);
    memset(p, 0, bulksize);
  }
  void reset(void) {
    offset = 0;
    memset(p, 0, allocated_size);
  }
  ~bulk_allocator() { free(p); }
  void *alloc(int size) {
    int sz = ((7 + size) >> 3) << 3; // round up
    int available = allocated_size - offset;
    if (available < sz)
      return 0;
    char *res = p + offset;
    offset += sz;
    return res;
  }
};

std::mutex access_lock;
bulk_allocator ptr32(1024 * 1024 * 4);

typedef struct vchar {
  unsigned short len;
  char str[1];
} vchar_t;

static vchar_t *make_vchar(const char *str, int specified_len) {
  int len = (specified_len) ? specified_len : strlen(str);
  vchar_t *res = (vchar_t *)ptr32.alloc(sizeof(vchar_t) + len);
  res->len = len;
  int cpy_len = (strlen(str) + 1);
  if (cpy_len > len)
    cpy_len = len;
  memcpy(res->str, str, cpy_len);
  __a2e_l(res->str, cpy_len);
  return res;
}

static char *make_char(const char *str, int specified_len) {
  int len = (specified_len) ? specified_len : strlen(str);
  int cpy_len = (strlen(str) + 1);
  char *res = (char *)ptr32.alloc(len + 1);
  if (cpy_len > len)
    cpy_len = len;
  memcpy(res, str, cpy_len);
  __a2e_l(res, cpy_len);
  return res;
}

static int svc6(void *reg15, void *reg1, void *dsa) {
  __ae_runmode rm(__AE_EBCDIC_MODE);
  // program can issue EBCDIC message if diag(n) is used
  __asm(" sam31 \n"
        " svc 6\n"
        " sam64 \n"
        : "+NR:r15"(reg15)
        : "NR:r1"(reg1), "NR:r13"(dsa), "NR:r0"(0)
        :);
  return (int)(unsigned long)reg15;
}

typedef struct workarea {
  char *__ptr32 pgm;
  unsigned int dsa[18];
} workarea_t;

void *make_plist(int argc, ...) {
  va_list argp;
  int i;
  if (argc < 1)
    return 0;
  void *__ptr32 *__ptr32 plist =
      (void *__ptr32 *__ptr32)ptr32.alloc((argc) * sizeof(void *__ptr32));
  va_start(argp, argc);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
  for (i = 0; i < argc; ++i) {
    plist[i] = (void *__ptr32)va_arg(argp, int);
  }
#pragma clang diagnostic pop
  va_end(argp);
  *((unsigned int *)&(plist[i - 1])) |= 0x80000000;
  return (void *)plist;
}

int bpxwdyn(const std::string &str, std::string &ddn, std::string &dsn,
            std::string &vol, std::string &msg, int return_vars) {
  __ae_runmode rm(__AE_ASCII_MODE);
  std::lock_guard<std::mutex> guard(access_lock);
  ptr32.reset();
  int rc = 0;
  workarea_t *wa = (workarea_t *)ptr32.alloc(sizeof(workarea_t));
  wa->pgm = make_char("BPXWDYN ", 8);
  void *__ptr32 parm_string_addr = make_char(str.c_str(), 0);
  void *__ptr32 ddname;
  void *__ptr32 dsname;
  void *__ptr32 volume;
  if (return_vars) {
    ddname = make_vchar("RTDDN", 9);
    dsname = make_vchar("RTDSN", 55);
    volume = make_vchar("RTVOL", 9);
  }
  void *__ptr32 msgtxt[6];
  for (int i = 0; i < 6; ++i) {
    char msg[20];
    if (i == 0) {
      snprintf(msg, 20, "MSG");
    } else {
      snprintf(msg, 20, "MSG.%d", i);
    }
    msgtxt[i] = make_vchar(msg, 258);
  }
  void *r1;
  if (return_vars) {
    r1 = make_plist(9, parm_string_addr, ddname, dsname, volume, msgtxt[0],
                    msgtxt[1], msgtxt[2], msgtxt[3], msgtxt[4]);
  } else {
    r1 = make_plist(6, parm_string_addr, msgtxt[0], msgtxt[1], msgtxt[2],
                    msgtxt[3], msgtxt[4]);
  }
  rc = svc6(&(wa->pgm), r1, wa->dsa);
  if (rc == 0) {
    if (return_vars) {
      int len;
      char *ddn_a = (((char *)ddname) + 2);
      len = strlen(ddn_a);
      if (len < 9) {
        __e2a_l(ddn_a, len);
        ddn = ddn_a;
      }
      char *dsn_a = (((char *)dsname) + 2);
      len = strlen(dsn_a);
      if (len < 54) {
        __e2a_l(dsn_a, len);
        dsn = dsn_a;
      }
      char *vol_a = (((char *)volume) + 2);
      len = strlen(vol_a);
      if (len < 9) {
        __e2a_l(vol_a, len);
        vol = vol_a;
      }
    }
  } else {
    unsigned short *size = (unsigned short *)msgtxt[0];
    if (*size > 257)
      return rc;
    char *tmp = (2 + (char *)msgtxt[0]);
    __e2a_l(tmp, *size);
    int cnt = atoi(tmp);
    if (cnt < 4) {
      for (int i = 0; i < cnt; ++i) {
        tmp = (((char *)msgtxt[i + 1]) + 2);
        unsigned short len = *((unsigned short *)msgtxt[i + 1]);
        if (len < 258)
          __e2a_l(tmp, len);
        msg.append(tmp);
      }
    }
  }
  return rc;
}

#if defined(UNIT_TEST)
int main(int argc, char **argv) {
  std::string ddname;
  std::string dsname;
  std::string volser;
  std::string msg;
  if (argc < 2) {
    fprintf(stderr, "need alloc string\n");
    return -1;
  }
  int rc = bpxwdyn(argv[1], ddname, dsname, volser, msg);
  printf("rc = %d\n", rc);
  if (rc == 0) {
    printf("DDNAME: %s\n", ddname.c_str());
    printf("DSNAME: %s\n", dsname.c_str());
    printf("VOLSER: %s\n", volser.c_str());
  } else {
    printf("%s \n", msg.c_str());
  }
  return 0;
}
#else
#include <napi.h>
class ErrorAsyncWorker : public Napi::AsyncWorker {
public:
  ErrorAsyncWorker(const Napi::Function &callback, Napi::Error error)
      : Napi::AsyncWorker(callback), error(error) {}

protected:
  void Execute() override {}
  void OnOK() override {
    Napi::Env env = Env();

    Callback().MakeCallback(Receiver().Value(),
                            {error.Value(), env.Undefined()});
  }

  void OnError(const Napi::Error &e) override {
    Napi::Env env = Env();

    Callback().MakeCallback(Receiver().Value(), {e.Value(), env.Undefined()});
  }

private:
  Napi::Error error;
};

class BpxwdynAsyncWorker : public Napi::AsyncWorker {

public:
  BpxwdynAsyncWorker(const Napi::Function &callback,
                     const std::string &alloc_str, int return_vars)
      : Napi::AsyncWorker(callback), return_variables(return_vars) {
    command = alloc_str;
  }
  ~BpxwdynAsyncWorker() {}

protected:
  void Execute() override {
    rc = bpxwdyn(command, ddname, dsname, volser, msg, return_variables);
    if (0 != rc) {
      SetError(msg);
    }
  }

  void OnOK() override {
    Napi::Env env = Env();
    Callback().MakeCallback(
        Receiver().Value(),
        {env.Null(), Napi::Number::New(env, rc), Napi::String::New(env, ddname),
         Napi::String::New(env, dsname), Napi::String::New(env, volser)});
  }

  void OnError(const Napi::Error &e) override {
    Napi::Env env = Env();
    Callback().MakeCallback(Receiver().Value(),
                            {e.Value(), Napi::Number::New(env, rc),
                             env.Undefined(), env.Undefined(),
                             env.Undefined()});
  }

private:
  bool return_variables;
  std::string command;
  std::string ddname;
  std::string dsname;
  std::string volser;
  std::string msg;
  int rc;
};

void BpxwdynAsyncCallbackArgsCheck(const Napi::CallbackInfo &info,
                                   bool return_variables) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::Error::New(
        env, "BPXWDYN needs a TEXT string as argument and a call back function")
        .ThrowAsJavaScriptException();
    return;
  }
  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Second argument is not a call back function")
        .ThrowAsJavaScriptException();
    return;
  }
  Napi::Function cb = info[1].As<Napi::Function>();
  if (!info[0].IsString()) {
    (new ErrorAsyncWorker(
         cb, Napi::TypeError::New(env, "First argument is not a string")))
        ->Queue();
    return;
  }

  (new BpxwdynAsyncWorker(cb, info[0].As<Napi::String>(), return_variables))
      ->Queue();
  return;
}

void BpxwdynAsyncCallbackReturnVars(const Napi::CallbackInfo &info) {
  BpxwdynAsyncCallbackArgsCheck(info, true);
}

void BpxwdynAsyncCallback(const Napi::CallbackInfo &info) {
  BpxwdynAsyncCallbackArgsCheck(info, false);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("execute", Napi::Function::New(env, BpxwdynAsyncCallback));
  exports.Set("execute_rtvars",
              Napi::Function::New(env, BpxwdynAsyncCallbackReturnVars));
  return exports;
}
NODE_API_MODULE(bpxwdyn, Init)
#endif
