#include "clipboard_listener.h"
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>

#pragma comment(lib, "Version.lib")

static HWND hNextClipboardViewer = NULL;
static HWND hwnd = NULL;
static napi_threadsafe_function tsfn = NULL;
static std::thread clipboardThread;

void Cleanup(void *arg);
void CallJs(napi_env env, napi_value jsCallback, void *context, void *data);
void ClipboardWatcherThread();
std::string WideStringToUTF8(const std::wstring &wideStr);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DRAWCLIPBOARD:
    {

        if (OpenClipboard(NULL))
        {
            HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
            if (hClipboardData)
            {
                wchar_t *pText = static_cast<wchar_t *>(GlobalLock(hClipboardData));
                if (pText)
                {
                    std::wstring clipboardContent(pText);
                    GlobalUnlock(hClipboardData);

                    napi_call_threadsafe_function(tsfn, new std::wstring(clipboardContent), napi_tsfn_blocking);
                }
            }
            CloseClipboard();
        }

        if (hNextClipboardViewer)
            SendMessage(hNextClipboardViewer, uMsg, wParam, lParam);
        break;
    }
    case WM_CHANGECBCHAIN:
    {

        if ((HWND)wParam == hNextClipboardViewer)
        {
            hNextClipboardViewer = (HWND)lParam;
        }
        else if (hNextClipboardViewer)
        {
            SendMessage(hNextClipboardViewer, uMsg, wParam, lParam);
        }
        break;
    }
    case WM_DESTROY:
    {
        ChangeClipboardChain(hWnd, hNextClipboardViewer);
        PostQuitMessage(0);
        break;
    }
    default:
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    }
    return 0;
}

void ClipboardWatcherThread()
{
    const char CLASS_NAME[] = "ClipboardWatcher";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
    hwnd = CreateWindowEx(
        0, CLASS_NAME, "Clipboard Watcher", 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

    hNextClipboardViewer = SetClipboardViewer(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hwnd);
    UnregisterClass(CLASS_NAME, GetModuleHandle(NULL));
}

void CallJs(napi_env env, napi_value jsCallback, void *context, void *data)
{
    std::wstring *content = static_cast<std::wstring *>(data);

    std::string utf8Content = WideStringToUTF8(*content);
    delete content;

    napi_value result;
    napi_create_string_utf8(env, utf8Content.c_str(), utf8Content.length(), &result);

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    napi_call_function(env, undefined, jsCallback, 1, &result, NULL);
}

napi_value SetClipboardListener(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    if (argc < 2)
    {
        napi_throw_error(env, NULL, "Expected 2 arguments: callback, options");
        return NULL;
    }

    napi_value resource_name;
    napi_create_string_utf8(env, "Clipboard Update", NAPI_AUTO_LENGTH, &resource_name);

    napi_status status = napi_create_threadsafe_function(
        env,
        args[1],
        NULL,
        resource_name,
        0,
        1,
        NULL,
        NULL,
        NULL,
        CallJs,
        &tsfn);

    clipboardThread = std::thread(ClipboardWatcherThread);
    napi_add_env_cleanup_hook(env, Cleanup, NULL);

    return NULL;
}

void Cleanup(void *arg)
{
    if (hwnd)
    {
        PostMessage(hwnd, WM_DESTROY, 0, 0);
    }
    if (clipboardThread.joinable())
    {
        clipboardThread.join();
    }
    if (tsfn)
    {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    }
}

napi_value RemoveClipboardListener(napi_env env, napi_callback_info info)
{
    Cleanup(NULL);
    return NULL;
}

std::string WideStringToUTF8(const std::wstring &wideStr)
{
    if (wideStr.empty())
        return "";

    int utf8Size = WideCharToMultiByte(
        CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size == 0)
        return "";

    std::string utf8Str(utf8Size, '\0');
    WideCharToMultiByte(
        CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Size, nullptr, nullptr);
    utf8Str.resize(utf8Size - 1);
    return utf8Str;
}