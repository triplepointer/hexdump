#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <richedit.h>
#include "transform.h"

const TCHAR g_szClassName[] = _T("EgorkaWindowClass");

long long offset_low_left = 0;
long long offset_high_left = 0;
long long offset_low_right = 0;
long long offset_high_right = 0;

OVERLAPPED olf_left = { 0 };
OVERLAPPED olf_right = { 0 };

LARGE_INTEGER li_left = { 0 };
LARGE_INTEGER li_right = { 0 };

HMENU hMenu;
HWND hEdit_left;
HWND hEdit_right;
HWND hCompare;
HWND button_next;
HWND button_back;

DWORD FileSize_left = 0;
DWORD FileSize_right = 0;

int left_menu_id;
int right_menu_id;

TCHAR saved_filename_left[100];
TCHAR saved_filename_right[100];

char isEnd_left = 0;
char isEnd_right = 0;

long long chunk_read_left = 0;
long long chunk_read_right = 0;

void AddMenus(HWND hwnd)
{
    hMenu = CreateMenu();

    HMENU hFileMenu = CreateMenu();
    HMENU hSubMenu = CreateMenu();

    AppendMenu(hSubMenu, MF_STRING, OPEN_FIRST_FILE, _T("First File"));
    AppendMenu(hSubMenu, MF_STRING, OPEN_SECOND_FILE, _T("Second File"));

    AppendMenu(hFileMenu, MF_POPUP, (UINT_PTR)hSubMenu,_T("Open"));
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_COMPARE, _T("Compare Files"));
    AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_EXIT, _T("Exit"));

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, _T("File"));

    SetMenu(hwnd,hMenu);
}

HWND CreateRichEdit(HWND hwndOwner,        // Dialog box handle.
    int x, int y,          // Location.
    int width, int height,
    HMENU hmenu, // Dimensions.
    HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibrary(TEXT("riched20.dll"));

    HWND hwndEdit = CreateWindowEx(0, RICHEDIT_CLASS, _T(""),
        ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VSCROLL | ES_READONLY,
        x, y, width, height,
        hwndOwner, hmenu, hinst, NULL);

    return hwndEdit;
}

void AddControls(HWND hwnd)
{
    hEdit_left = CreateRichEdit(hwnd, 0, 0, 550, 550, (HMENU)left_menu_id, NULL);
    hEdit_right = CreateRichEdit(hwnd, 550, 0, 550, 550, (HMENU)right_menu_id, NULL);

    button_back = CreateWindow(_T("Button"),_T("BACK"), WS_CHILD | WS_VISIBLE | WS_BORDER,
        0, 600, 120, 30, hwnd, (HMENU)BACK, NULL, NULL);
    button_next = CreateWindow(_T("Button"), _T("NEXT"), WS_CHILD | WS_VISIBLE | WS_BORDER,
        980, 600, 120, 30, hwnd, (HMENU)NEXT, NULL, NULL);
}

void display_file_left(PTCHAR path, HWND hwnd)
{
    TCHAR Buffer_1[CHUNK_SIZE];
    memset(Buffer_1, 0, sizeof(Buffer_1));
    TCHAR Buffer_2[LINES_PER_CHUNK * 16 + 1];
    memset(Buffer_2, 0, sizeof(Buffer_2));

    if (path[0] == '\0')
        ;
    else {
        
        HANDLE hFile_left = NULL;

        hFile_left = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile_left  == INVALID_HANDLE_VALUE)
            MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);

        olf_left.Offset = li_left.LowPart;
        olf_left.OffsetHigh = li_left.HighPart;

        DWORD iNumRead = 0;

        ReadFile(hFile_left, Buffer_2, LINES_PER_CHUNK * 16, &iNumRead, &olf_left);

        FileSize_left = GetFileSize(hFile_left, NULL);
        if (FileSize_left == INVALID_FILE_SIZE) {
        MessageBox(hwnd, _T("GetFileSize failed"), _T("Error!"), NULL);
        CloseHandle(hFile_left);
        return NULL;
        }

//        olf_left.Offset += iNumRead;

        buffer_write_left(Buffer_1, Buffer_2, &FileSize_left, chunk_read_left, 1);

        if (Buffer_1[1065] == 0 && FileSize_left < LINES_PER_CHUNK * 16) isEnd_left = TRUE;

        SetWindowText(hEdit_left, Buffer_1);
        CloseHandle(hFile_left);
    }
}

void display_file_right(PTCHAR path, HWND hwnd)
{
    TCHAR Buffer_1[CHUNK_SIZE];
    memset(Buffer_1, 0, sizeof(Buffer_1));
    TCHAR Buffer_2[LINES_PER_CHUNK * 16 + 1];
    memset(Buffer_2, 0, sizeof(Buffer_2));

    HANDLE hFile_right = NULL;

    if (path[0] == '\0')
        ;
    else {

        hFile_right = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile_right == INVALID_HANDLE_VALUE)
            MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);

        olf_right.Offset = li_right.LowPart;
        olf_right.OffsetHigh = li_right.HighPart;

        DWORD iNumRead = 0;

        ReadFile(hFile_right, Buffer_2, LINES_PER_CHUNK * 16, &iNumRead, &olf_right);

        FileSize_right = GetFileSize(hFile_right, NULL);
        if (FileSize_right == INVALID_FILE_SIZE) {
            MessageBox(hwnd, _T("GetFileSize failed"), _T("Error!"), NULL);
            CloseHandle(hFile_right);
            return NULL;
        }

        buffer_write_right(Buffer_1, Buffer_2, FileSize_right, chunk_read_right, 1);

        if (Buffer_1[1065] == 0 && FileSize_right < LINES_PER_CHUNK*16) isEnd_right = TRUE;

        SetWindowText(hEdit_right, Buffer_1);
        CloseHandle(hFile_right);
    }
}

void open_file_left(HWND hwnd)
{
    OPENFILENAME ofn_left = { 0 };

    TCHAR file_name[100];
    ZeroMemory(&ofn_left, sizeof(OPENFILENAME));
    ofn_left.lStructSize = sizeof(OPENFILENAME);
    ofn_left.hwndOwner = hwnd;
    ofn_left.lpstrFile = file_name;
    ofn_left.lpstrFile[0] = '\0';
    ofn_left.nMaxFile = 100;
    ofn_left.lpstrFilter = _T("All files\0*.*\0Text Files\0*.txt\0");
    ofn_left.nFilterIndex = 1;

    GetOpenFileName(&ofn_left);

    strcpy_s(saved_filename_left, sizeof(saved_filename_left), ofn_left.lpstrFile);
   
    display_file_left(ofn_left.lpstrFile, hwnd);
}

void open_file_right(HWND hwnd)
{
    OPENFILENAME ofn_right = { 0 };

    TCHAR file_name[100];
    ZeroMemory(&ofn_right, sizeof(OPENFILENAME));
    ofn_right.lStructSize = sizeof(OPENFILENAME);
    ofn_right.hwndOwner = hwnd;
    ofn_right.lpstrFile = file_name;
    ofn_right.lpstrFile[0] = '\0';
    ofn_right.nMaxFile = 100;
    ofn_right.lpstrFilter = _T("All files\0*.*\0Text Files\0*.txt\0");
    ofn_right.nFilterIndex = 1;

    GetOpenFileName(&ofn_right);

    strcpy_s(saved_filename_right, sizeof(saved_filename_right) ,ofn_right.lpstrFile);

    display_file_right(ofn_right.lpstrFile, hwnd);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_COMMAND:
            switch (wParam)
            {
            case OPEN_FIRST_FILE:
                last_counter_left = 0;
                addr_left = 0;
                chunk_read_left = 0;
                isEnd_left = FALSE;
                open_file_left(hwnd);
                break;
            case OPEN_SECOND_FILE:
                last_counter_right = 0;
                addr_right = 0;
                chunk_read_right = 0;
                isEnd_right = FALSE;
                open_file_right(hwnd);
                break;
            case FILE_MENU_COMPARE:
                compare();
                break;
            case NEXT:
                goto_next_chunk(hwnd);
                break;
            case BACK:
                goto_previous_chunk(hwnd);
                break;
            case FILE_MENU_EXIT:
                DestroyWindow(hwnd);
            break;
            }
            break;
        case WM_CREATE:
            AddMenus(hwnd);
            AddControls(hwnd);
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    HWND hwnd;
    MSG Msg;

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = g_szClassName;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"),
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindow(
        wc.lpszClassName,
        TEXT("TheWindow"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 1130, 1000,
        NULL, NULL, hInstance, NULL);
    
    if (hwnd == NULL)
    {
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"),
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}