
#include <Windows.h>
#include <iostream>
#include <windowsx.h>
#include <GdiPlus.h>
#include <string>
#include <CommCtrl.h>
#include <math.h>
#include <fstream>
#pragma comment( lib, "gdiplus" )
#include <math.h>
#pragma comment( lib, "comctl32.lib")

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")
void unhookMouse(HHOOK mouseHook)
{
	UnhookWindowsHookEx(mouseHook);
	exit(0);
}
bool LBUTTONDOWN = false;
int i = 0;
int nTime = 0;
auto  new_x = 0, new_y = 0, old_x = 0, old_y = 0, count = 0;
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    using namespace Gdiplus;
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }
    free(pImageCodecInfo);
    return 0;
}
void take_picture(int& i)
{
    using namespace Gdiplus;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    HDC scrdc, memdc;
    HBITMAP membit;
    RECT rc;
    HWND eos = GetForegroundWindow();
    GetClientRect(eos, &rc);
    scrdc = ::GetDC(eos);
    memdc = CreateCompatibleDC(scrdc);
    int Width = rc.right - rc.left;
    int Height = rc.bottom - rc.top - 150;
   
    membit = CreateCompatibleBitmap(scrdc, Width, Height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);
    BitBlt(memdc,0,0, Width, Height, scrdc,0, 125, SRCCOPY);

    Gdiplus::Bitmap bitmap(membit, NULL);
    CLSID clsid;
    EncoderParameters encoderParameters;
    ULONG             quality;
    Status            stat;

    GetEncoderClsid(L"image/png", &clsid);
    encoderParameters.Count = 1;
    encoderParameters.Parameter[0].Guid = EncoderQuality;
    encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoderParameters.Parameter[0].NumberOfValues = 1;
    quality = 100;//25;
    encoderParameters.Parameter[0].Value = &quality;
    std::string file;

 
    const char* x2 = ".png";
    file = "cap" + std::to_string(i) + x2;
    std::wstring wfile(file.begin(), file.end());
    bitmap.Save(wfile.c_str(), &clsid, &encoderParameters);

    SelectObject(memdc, hOldBitmap);
    DeleteObject(memdc);

    DeleteObject(membit);

    ::ReleaseDC(0, scrdc);
    i++;
}
HWND m_hWndTip;
void create_tooltip()
{
    #ifndef TTF_TRACK
    #define TTF_TRACK 0x0020
    #define TTF_ABSOLUTE 0x0080
    #define TTM_SETMAXTIPWIDTH (WM_USER+24)
    #define TTM_TRACKPOSITION (WM_USER+18)
    #define TTM_TRACKACTIVATE (WM_USER+17)
    #endif

    TOOLINFO ti;
    POINT  pt;

    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
    ti.hwnd = NULL;
    ti.hinst = NULL;
    ti.uId = 0;
    ti.lpszText = (LPSTR)"test";
    ti.rect.left = ti.rect.top = ti.rect.right = ti.rect.bottom = 0;

    
    GetCursorPos(&pt);
    pt.x += 16;
    pt.y += 16;

   
    if (!m_hWndTip)
    {
        m_hWndTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);

        RECT dtw;
        GetWindowRect(GetDesktopWindow(), &dtw);

        SendMessage(m_hWndTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        SendMessage(m_hWndTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)dtw.right);
    }
    else
    {
        SendMessage(m_hWndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
    }
    SendMessage(m_hWndTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x, pt.y));
    SendMessage(m_hWndTip, TTM_TRACKACTIVATE, true, (LPARAM)&ti);

    SetWindowDisplayAffinity(m_hWndTip, WDA_EXCLUDEFROMCAPTURE);

}
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    
    POINT p;
    MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
    if (nCode == HC_ACTION && pMouseStruct != NULL)
    {
        if (wParam == WM_RBUTTONDOWN)
        {
            GetCursorPos(&p);
            nTime = 0;
            nTime = GetTickCount();
            old_x = p.x;
            old_y = p.y;
            //printf("DOWN x : %d, y:%d\n", old_x, old_y);
      
        }
        if (wParam == WM_RBUTTONUP)
        {
            if (GetTickCount() - nTime > 250)
            {
                
                GetCursorPos(&p);
                new_x = p.x;
                new_y = p.y;
                if (old_x - new_x >= 45 && abs(old_y - new_y) <= 150)
                {
                    printf("Huong sang trai\n");
                    create_tooltip();
                    printf("%d\n", GetLastError());
                }
                else if (new_x - old_x >= 45 && abs(old_y - new_y) <= 150) printf("Huong sang phai\n");
                else if (old_y - new_y >= 45 && abs(old_x - new_x) <= 150) printf("Huong len tren\n");
                else if (new_y - old_y >= 45 && abs(old_x - new_x) <= 150) printf("Huong xuong duoi\n");
                printf("-----------------------\n");
            }
            nTime = 0;
            
        }
    }
    return 0;
}
int main()
{
   HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), NULL);
   
   MSG Msg;
   
   while (GetMessage(&Msg, NULL, 0, 0) > 0)
   {
       TranslateMessage(&Msg);
       DispatchMessage(&Msg);
   }
   unhookMouse(mouseHook);

   return 0;
}
