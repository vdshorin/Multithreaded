#include "windows.h"
#include <string>
#include <iostream>
#include <vector>
#include <commctrl.h>
#include <process.h>

#define BTN_STOP_IMAGE 101
#define BTN_STOP_LINE 102
#define ID_STATUSBAR 103

using namespace std;

BOOL RegClass(WNDPROC, LPCTSTR, UINT);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void SwitchState(int);

DWORD WINAPI ThreadImage(LPVOID);
DWORD WINAPI ThreadRunnigLine(LPVOID);

HWND hwnd;
HINSTANCE hInstance;

char szMainClass[] = "MainClass";
char szTitle[] = "Lab7";

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

    hInstance = hInst;

    if (!RegClass(WndProc, szMainClass, COLOR_WINDOW)) { return FALSE; }

    hwnd = CreateWindow(szMainClass,
                        szTitle,
                        WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        500, 400,
                        0, 0, hInstance, NULL);

    if (!hwnd) { return FALSE; }

    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

BOOL RegClass(WNDPROC Proc, LPCTSTR szName, UINT brBackground)
{
    WNDCLASS wc;
    wc.style = 0;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.lpfnWndProc = Proc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(brBackground + 1);
    wc.lpszMenuName = (LPCTSTR)NULL;
    wc.lpszClassName = szName;
    return (RegisterClass(&wc) != 0);
}

HWND hwndBtnStopImage, hwndBtnStopLine;
HWND hwndStatusBar;
HWND hwndImage;

HANDLE hThreadImage;
HANDLE hThreadRunnigLine;

enum actions {startImage, stopImage,
        startString, stopString};
enum states { state_1, state_2};

actions getAction[2][2] = {
        {startImage, stopImage},
        {startString, stopString}
};

int states[2] = {0, 0};

int row;

void SwitchState(int sig)
{
    actions curAct = getAction[sig][states[sig]];
    switch (curAct)
    {
        case startImage:
        {
            ResumeThread(hThreadImage);
            states[sig] = 1;
            break;
        }
        case stopImage:
        {
            SuspendThread(hThreadImage);
            states[sig] = 0;
            break;
        }
        case startString:
        {
            ResumeThread(hThreadRunnigLine);
            states[sig] = 1;
            break;
        }
        case stopString:
        {
            SuspendThread(hThreadRunnigLine);
            states[sig] = 0;
            break;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
        //region WM_CREATE
        case WM_CREATE:
        {
            hwndBtnStopImage = CreateWindow("button", "IMAGE",
                    WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                    20, 20,
                    100, 30,
                    hWnd, (HMENU) BTN_STOP_IMAGE, hInstance, NULL);

            hwndBtnStopLine = CreateWindow("button", "LINE",
                    WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                    20, 70,
                    100, 30,
                    hWnd, (HMENU) BTN_STOP_LINE, hInstance, NULL);

            hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
                    "My Status Bar",
                    hWnd,
                    ID_STATUSBAR);

            hThreadImage = CreateThread(NULL, 0, ThreadImage, 0, CREATE_SUSPENDED, 0);
            if(hThreadImage == NULL)
            {
                MessageBox(hWnd, "CreateThread hThreadImage failed", "Err", MB_OK);
            }

            hThreadRunnigLine = CreateThread(NULL, 0, ThreadRunnigLine, 0, CREATE_SUSPENDED, 0);
            if(hThreadRunnigLine == NULL)
            {
                MessageBox(hWnd, "CreateThread hThreadRunnigLine failed", "Err", MB_OK);
            }

            return 0;
        }
            //endregion

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case BTN_STOP_IMAGE:
                {
                    row = 0;
                    break;
                }

                case BTN_STOP_LINE:
                {
                    row = 1;
                    break;
                }
            }
            SwitchState(row);
            break;
        }

        case WM_DESTROY:
        {

            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

PAINTSTRUCT ps;
HDC hdc, hCompatibleDC;
HANDLE hBitMap, hOldBitMap;
RECT rect;
BITMAP bitMap;
int curImg = 0;
vector<string> images = {"img1.bmp", "img2.bmp", "img3.bmp", "img4.bmp", "img5.bmp"};


DWORD WINAPI ThreadImage(LPVOID lpParameter)
{
    while (states[0] == 1)
    {
        hdc = GetDC(hwnd);
        hBitMap = LoadImage(NULL, images[curImg].c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        GetObject(hBitMap, sizeof(BITMAP), &bitMap);
        hCompatibleDC = CreateCompatibleDC(hdc);
        hOldBitMap = SelectObject(hCompatibleDC, hBitMap);
        GetClientRect(hwnd, &rect);
        StretchBlt(hdc, 120, 10, rect.right, rect.bottom,
                hCompatibleDC, -5, -5, 50, 50,
                SRCCOPY);
        SelectObject(hCompatibleDC, hOldBitMap);
        DeleteObject(hBitMap);
        DeleteDC(hCompatibleDC);
        ReleaseDC(hwnd, hdc);

        curImg++;
        if(curImg == 3)
        {
            curImg = 0;
        }
        Sleep(1000);
    }

    return 0;
}

string strName = "My Status Bar";
string str;
int countSpaces = 0;
DWORD WINAPI ThreadRunnigLine(LPVOID lpParameter)
{
    while (states[1] == 1)
    {
        str = "";
        for(int i = 0; i < countSpaces; i++)
        {
            str.insert(0, " ");
        }
        str += strName;
        countSpaces++;

        if(countSpaces == 165)
        {
            str = "";
            countSpaces = 0;
        }
        SetWindowText(hwndStatusBar, str.c_str());
        Sleep(100);
    }

    return 0;
}

