//
// Created by Mariotaku on 2021/08/25.
//
#include "indev_sdl.h"

#if __WIN32__

#include <hidusage.h>
#include <hidsdi.h>
#include <hidpi.h>

#pragma comment(lib, "hid.lib")

#include <SDL_syswm.h>

int isTouchingInWindow = 0, isScrolling = 0;

#if HAS_SCROLL_EVENT
void handleTouchpadEvent(const SDL_SysWMmsg *wmmsg);
#endif

void _indev_handle_syswm_setup_win(SDL_SysWMinfo *info) {
#if HAS_SCROLL_EVENT
    HWND hWnd = info->info.win.window;
    RAWINPUTDEVICE rawdev = {
            .usUsagePage = HID_USAGE_PAGE_DIGITIZER,
            .usUsage = HID_USAGE_DIGITIZER_TOUCH_PAD,
            .dwFlags = RIDEV_INPUTSINK,
            .hwndTarget = hWnd,
    };
    RegisterRawInputDevices(&rawdev, 1, sizeof(rawdev));
#endif
}

void _indev_handle_syswm_evt_win(SDL_SysWMEvent *event) {
#if HAS_SCROLL_EVENT
    SDL_SysWMmsg *wmmsg = event->msg;
    UINT msg = wmmsg->msg.win.msg;
    switch (msg) {
        case WM_INPUT: {
            WPARAM code = GET_RAWINPUT_CODE_WPARAM(wmmsg->msg.win.wParam);
            if (code != RIM_INPUT) {
                break;
            }
            handleTouchpadEvent(wmmsg);
            DefWindowProc(wmmsg->msg.win.hwnd, wmmsg->msg.win.msg, wmmsg->msg.win.wParam,
                          wmmsg->msg.win.lParam);
            break;
        }
        case WM_MOUSEWHEEL: {
            if (isTouchingInWindow) {
                isScrolling = 1;

                SDL_Event smooth_scroll;
                SDL_memset(&smooth_scroll, 0, sizeof(smooth_scroll));
                smooth_scroll.type = INDEV_SCROLL;
                int x, y;
                SDL_GetMouseState(&x, &y);
                smooth_scroll.motion.state = isTouchingInWindow ? SDL_PRESSED : SDL_RELEASED;
                smooth_scroll.motion.x = x;
                smooth_scroll.motion.y = y;
                smooth_scroll.motion.yrel = GET_WHEEL_DELTA_WPARAM(wmmsg->msg.win.wParam);
                SDL_PushEvent(&smooth_scroll);
            }
            break;
        }
        case WM_MOUSEHWHEEL: {
            if (isTouchingInWindow) {
                isScrolling = 1;

                SDL_Event smooth_scroll;
                SDL_memset(&smooth_scroll, 0, sizeof(smooth_scroll));
                smooth_scroll.type = INDEV_SCROLL;
                int x, y;
                SDL_GetMouseState(&x, &y);
                smooth_scroll.motion.state = isTouchingInWindow ? SDL_PRESSED : SDL_RELEASED;
                smooth_scroll.motion.x = x;
                smooth_scroll.motion.y = y;
                smooth_scroll.motion.xrel = -GET_WHEEL_DELTA_WPARAM(wmmsg->msg.win.wParam);
                SDL_PushEvent(&smooth_scroll);
            }
            break;
        }
        default: {
            break;
        }
    }
#endif
}

#if HAS_SCROLL_EVENT
void handleTouchpadEvent(const SDL_SysWMmsg *wmmsg) {
    HRAWINPUT hRawInput = (HRAWINPUT) wmmsg->msg.win.lParam;
    UINT pcbSize = 0;
    GetRawInputData(hRawInput, RID_INPUT, NULL, &pcbSize, sizeof(RAWINPUTHEADER));
    if (!pcbSize) {
        return;
    }
    RAWINPUT *pData = SDL_malloc(pcbSize);
    GetRawInputData(hRawInput, RID_INPUT, pData, &pcbSize, sizeof(RAWINPUTHEADER));
    if (pData->header.dwType != RIM_TYPEHID) {
        goto free_pData;
    }
    pcbSize = 0;
    GetRawInputDeviceInfo(pData->header.hDevice, RIDI_PREPARSEDDATA, NULL, &pcbSize);
    if (!pcbSize) {
        return;
    }
    PHIDP_PREPARSED_DATA pPreparsedData = SDL_malloc(pcbSize);
    GetRawInputDeviceInfo(pData->header.hDevice, RIDI_PREPARSEDDATA, pPreparsedData, &pcbSize);
    HIDP_CAPS caps;
    HidP_GetCaps(pPreparsedData, &caps);
    PHIDP_VALUE_CAPS valueCaps = SDL_malloc(sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps);
    USHORT numValueCaps = caps.NumberInputValueCaps;
    HidP_GetValueCaps(HidP_Input, valueCaps, &numValueCaps, pPreparsedData);

    ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, pPreparsedData);
    USAGE *buttonUsageArray = (USAGE *) SDL_malloc(sizeof(USAGE) * maxNumButtons);
    HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, valueCaps->LinkCollection, buttonUsageArray,
                   &maxNumButtons, pPreparsedData, (PCHAR) pData->data.hid.bRawData, pData->data.hid.dwSizeHid);
    int isContactOnSurface = 0;

    for (ULONG usageIdx = 0; usageIdx < maxNumButtons; usageIdx++) {
        if (buttonUsageArray[usageIdx] == HID_USAGE_DIGITIZER_TIP_SWITCH) {
            isContactOnSurface = 1;
            break;
        }
    }

    if (!isContactOnSurface && isScrolling) {
        // Send scroll end event
        SDL_Event smooth_scroll;
        SDL_memset(&smooth_scroll, 0, sizeof(smooth_scroll));
        smooth_scroll.type = INDEV_SCROLL;
        int x, y;
        SDL_GetMouseState(&x, &y);
        smooth_scroll.motion.state = SDL_RELEASED;
        smooth_scroll.motion.x = x;
        smooth_scroll.motion.y = y;
        SDL_PushEvent(&smooth_scroll);
        isScrolling = 0;
    }
    if (isTouchingInWindow != isContactOnSurface) {
//        SDL_Log("%10d touch state changed: %d\n", SDL_GetTicks(), isContactOnSurface);
    }
    isTouchingInWindow = isContactOnSurface;

    SDL_free(valueCaps);
    SDL_free(buttonUsageArray);
    free_pPreparsedData:
    SDL_free(pPreparsedData);
    free_pData:
    SDL_free(pData);
}
#endif

#endif