#pragma once
#include <node_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

    napi_value SetClipboardListener(napi_env env, napi_callback_info info);

    napi_value RemoveClipboardListener(napi_env env, napi_callback_info info);

#ifdef __cplusplus
}
#endif