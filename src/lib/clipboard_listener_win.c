#include "clipboard_listener.h"
#include <windows.h>


typedef struct {
    napi_threadsafe_function tsFunc;
    WNDPROC oldWndProc;
} ClipboardListenerData;

static ClipboardListenerData listenerData = { NULL, NULL };

LRESULT CALLBACK ClipboardWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CLIPBOARDUPDATE && listenerData.tsFunc) {
        napi_call_threadsafe_function(listenerData.tsFunc, NULL, napi_tsfn_nonblocking);
    }
    return CallWindowProc(listenerData.oldWndProc, hwnd, msg, wParam, lParam);
}

napi_value SetClipboardListener(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    if (argc < 2) return NULL;

    // 解析 hWnd
    void* hWndData;
    size_t dataSize;
    napi_get_buffer_info(env, args[0], &hWndData, &dataSize);
    HWND hwnd = *(HWND*)hWndData;

    // 先创建 napi_value 作为资源名称
    napi_value resource_name;
    napi_create_string_utf8(env, "ClipboardCallback", NAPI_AUTO_LENGTH, &resource_name);

    // 创建线程安全回调
    napi_create_threadsafe_function(env, args[1], NULL, resource_name,
                                    0, 1, NULL, NULL, NULL, NULL, &listenerData.tsFunc);

    AddClipboardFormatListener(hwnd);
    listenerData.oldWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ClipboardWndProc);

    return NULL;
}

napi_value RemoveClipboardListener(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    if (argc < 1) return NULL;

    void* hWndData;
    size_t dataSize;
    napi_get_buffer_info(env, args[0], &hWndData, &dataSize);
    HWND hwnd = *(HWND*)hWndData;

    RemoveClipboardFormatListener(hwnd);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)listenerData.oldWndProc);
    listenerData.oldWndProc = NULL;

    if (listenerData.tsFunc) {
        napi_release_threadsafe_function(listenerData.tsFunc, napi_tsfn_release);
        listenerData.tsFunc = NULL;
    }

    return NULL;
}