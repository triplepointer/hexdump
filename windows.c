#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <richedit.h>
#include "transform.h"

const TCHAR g_szClassName[] = _T("EgorkaWindowClass");

HMENU hMenu;
HWND hEdit_left;
HWND hEdit_right;
HWND hCompare;
HWND button_next;
HWND button_back;

int left_menu_id;
int right_menu_id;

TCHAR saved_filename_left[100];
TCHAR saved_filename_right[100];

long long chunk_read_left = 0;
long long chunk_read_right = 0;

struct FileMapping* mapping_left = { 0 };
struct FileMapping* mapping_right = { 0 };

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

struct FileMapping* fileMappingCreate(PTCHAR path, HWND hwnd) {
    HANDLE hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, _T("CreateFile failed"), _T("Error!"), NULL);
        return NULL;
    }

    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE) {
        MessageBox(hwnd, _T("GetFileSize failed"), _T("Error!"), NULL);
        CloseHandle(hFile);
        return NULL;
    }

    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapping == NULL) {
        MessageBox(hwnd, _T("CreateFileMapping failed"), _T("Error!"), NULL);
        CloseHandle(hFile);
        return NULL;
    }

    PTCHAR dataPtr = (PTCHAR)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwFileSize);
    if (dataPtr == NULL) {
        MessageBox(hwnd, _T("MapViewOfFile failed"), _T("Error!"), NULL);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return NULL;
    }

    struct FileMapping* mapping = (struct FileMapping*)malloc(sizeof(struct FileMapping));
    if (mapping == NULL) {
        MessageBox(hwnd, _T("MapViewOfFile failed"), _T("Error!"), NULL);
        UnmapViewOfFile(dataPtr);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return NULL;
    }

    mapping->hFile = hFile;
    mapping->hMapping = hMapping;
    mapping->dataPtr = dataPtr;
    mapping->fsize = (size_t)dwFileSize;

    return mapping;
}

void fileMappingClose(struct FileMapping* mapping) {
    UnmapViewOfFile(mapping->dataPtr);
    CloseHandle(mapping->hMapping);
    CloseHandle(mapping->hFile);
    free(mapping);
}

void display_file_left(PTCHAR path, HWND hwnd)
{
    TCHAR Buffer[CHUNK_SIZE];
    memset(Buffer, 0, sizeof(Buffer));

    if (path == NULL)
        ;
    else {
        mapping_left = fileMappingCreate(path, hwnd);
        if (mapping_right != NULL && chunk_read_right > 0) {
            MessageBox(hwnd, _T("You have to open second file for comparison!"), _T("Attention!"), NULL);
            SetWindowText(hEdit_right, _T(""));
        }
        buffer_write(Buffer, mapping_left->dataPtr, chunk_read_left);
        SetWindowText(hEdit_left, Buffer);
        fileMappingClose(mapping_left);
    }
}

void display_file_right(PTCHAR path, HWND hwnd)
{
    TCHAR Buffer[CHUNK_SIZE];
    memset(Buffer, 0, sizeof(Buffer));

    if (path == NULL)
        ;
    else {
        mapping_right = fileMappingCreate(path, hwnd);
        if (mapping_left != NULL && chunk_read_left > 0) {
            MessageBox(hwnd, _T("You have to open first file for comparison!"), _T("Attention!"), NULL);
            SetWindowText(hEdit_left, _T(""));
        }
        buffer_write(Buffer, mapping_right->dataPtr, chunk_read_right);
        SetWindowText(hEdit_right, Buffer);
        fileMappingClose(mapping_right);
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
                chunk_read_left = 0;
                isEnd_right = FALSE;
                isEnd_left = FALSE;
                open_file_left(hwnd);
                break;
            case OPEN_SECOND_FILE:
                chunk_read_right = 0;
                isEnd_right = FALSE;
                isEnd_left = FALSE;
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