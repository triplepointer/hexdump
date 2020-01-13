﻿#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <richedit.h>
#include "transform.h"

long long addr_left = 0;
long long addr_right = 0;

char FileSize_restoration_left = 0;
char FileSize_restoration_right = 0;

char last_counter_left = 0;
char last_counter_right = 0;

void Highlight_hEdit_left(UINT uStartPos, UINT uEndPos, COLORREF color) {
    CHARRANGE cr;
    CHARRANGE save;
    cr.cpMin = uStartPos;
    cr.cpMax = uEndPos;
    SendMessage(hEdit_left, EM_EXGETSEL, 0, (LPARAM)&save);// save old position
    SendMessage(hEdit_left, EM_EXSETSEL, 0, (LPARAM)&cr);  // new position
    CHARFORMAT cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0; // add this line
    cf.crTextColor = color;
    SendMessage(hEdit_left, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hEdit_left, EM_EXSETSEL, 0, (LPARAM)&save); // go back to saved position
}

void Highlight_hEdit_right(UINT uStartPos, UINT uEndPos, COLORREF color) {
    CHARRANGE cr;
    CHARRANGE save;
    cr.cpMin = uStartPos;
    cr.cpMax = uEndPos;
    SendMessage(hEdit_right, EM_EXGETSEL, 0, (LPARAM)&save);// save old position
    SendMessage(hEdit_right, EM_EXSETSEL, 0, (LPARAM)&cr);  // new position
    CHARFORMAT cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0; // add this line
    cf.crTextColor = color;
    SendMessage(hEdit_right, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hEdit_right, EM_EXSETSEL, 0, (LPARAM)&save); // go back to saved position
}

int readline(PTCHAR buf, long long mark_src, DWORD *file_size) {
    int size = 0;

    if ((*file_size) == 0) return size;

    while (((*file_size)--) != 0) {
        size = size + 1;
        if ((*file_size) == 0) break;
        if (size == 16) break;
    }
    return size;
}

void putbyt_buf(PTCHAR buf1, PTCHAR buf2, long long* mark_dst, long long* mark_src)
{
    int i;
    if (((buf2[*mark_src]) >= 0) && ((buf2[*mark_src]) <= 255)) {
        i = (buf2[(*mark_src)] & 0x000000f0) >> 4;
        if (i < 10) {
            buf1[++ * mark_dst] = ('0' + i); // 0 1 2 3 4 5 6 7 8 9
        }
        else {
            buf1[++ * mark_dst] = ('A' + i - 10); // 10(A) 11(B) 12(C) 13(D) 14(E) 15(F)
        }
        i = (buf2[(*mark_src)] & 0x0000000f) >> 0;
        if (i < 10) {
            buf1[++(*mark_dst)] = ('0' + i);// 0 1 2 3 4 5 6 7 8 9
        }
        else {
            buf1[++(*mark_dst)] = ('A' + i - 10);// 10(A) 11(B) 12(C) 13(D) 14(E) 15(F)
        }
    }
    else if ((buf2[*mark_src]) < 0) {
        i = ((unsigned)buf2[(*mark_src)] & 0x000000f0) >> 4;
        if (i < 10) {
            buf1[++ * mark_dst] = ('0' + i); // 0 1 2 3 4 5 6 7 8 9
        }
        else {
            buf1[++ * mark_dst] = ('A' + i - 10); // 10(A) 11(B) 12(C) 13(D) 14(E) 15(F)
        }
        i = ((unsigned)buf2[(*mark_src)] & 0x0000000f) >> 0;
        if (i < 10) {
            buf1[++(*mark_dst)] = ('0' + i);// 0 1 2 3 4 5 6 7 8 9
        }
        else {
            buf1[++(*mark_dst)] = ('A' + i - 10);// 10(A) 11(B) 12(C) 13(D) 14(E) 15(F)
        }
    }
}

void putbyt_addr(int c, PTCHAR Buffer_1, long long* mark_dst)
{
    int i;
    if ((c >= 0) && (c <= 255)) {
        i = (c & 0x000000f0) >> 4;
        if (i < 10) {
            Buffer_1[*mark_dst] = ('0' + i);// 0 1 2 3 4 5 6 7 8 9
        }
        else {
            Buffer_1[*mark_dst] = ('A' + i - 10);// 10(A) 11(B) 12(C) 13(D) 14(E) 15(F)
        }
        i = (c & 0x0000000f) >> 0;
        if (i < 10) {
            Buffer_1[++ * mark_dst] = ('0' + i);// 0 1 2 3 4 5 6 7 8 9
        }
        else {
            Buffer_1[++ * mark_dst] = ('A' + i - 10);// 10(A) 11(B) 12(C) 13(D) 14(E) 15(F)
        }
    }
}

void putlong(PTCHAR Buffer_1, long long addr, long long* mark_dst, char *first_time)
{
    if (addr >= 16 && *first_time) {
        //*mark_dst += 1;
        //*first_time = FALSE;
    }
    else
        *mark_dst += 0;
    putbyt_addr(((addr >> 24) & 0x000000ff), Buffer_1, mark_dst); // 3rd byte
    *mark_dst += 1;
    putbyt_addr(((addr >> 16) & 0x000000ff), Buffer_1, mark_dst); // 2nd byte
    *mark_dst += 1;
    putbyt_addr(((addr >> 8) & 0x000000ff), Buffer_1, mark_dst); // 1st byte
    *mark_dst += 1;
    putbyt_addr(((addr >> 0) & 0x000000ff), Buffer_1, mark_dst); // 0th byte
}

void printline(long long size, long long* mark_dst, long long* mark_src, PTCHAR buf1, PTCHAR buf2)
{
    int i, c;
    if (size > 0) {
        i = 0;
        while (i < 16) {
            if (i < size) {
                putbyt_buf(buf1, buf2, mark_dst, mark_src);
                *mark_src += 1;
                buf1[++(*mark_dst)] = ' ';
 //               if (buf2[*mark_src] == '\0') return;
            }
            else {
                buf1[++(*mark_dst)] = '0';
                buf1[++(*mark_dst)] = '0';
                buf1[++(*mark_dst)] = ' ';
            }
            i++;
        }
    }
    buf1[++(*mark_dst)] = '|';
    if (size != 16)
        *mark_src -= size;
    else
        *mark_src -= 16;
    for (i = 0; i < size; i++) {
        c = buf2[(*mark_src)++];
        if ((c >= ' ') && (c <= '~')) {
            buf1[++(*mark_dst)] = c;
        }
        else {
            buf1[++(*mark_dst)] = '.';
        }
    }
    buf1[++(*mark_dst)] = '\r';
    buf1[++(*mark_dst)] = '\n';
}

void buffer_write_left(PTCHAR Buffer_1, PTCHAR Buffer_2, char *first_time)
{
    long long size = 0;
    char counter = 0;

    long long mark_dst = 0;
    long long mark_src = 0;

    DWORD file_size = FileSize_left;

    size = readline(Buffer_2, mark_src, &file_size);

    if (size == 0) {
        last_counter_left = 0;
    }
    
    while (counter < LINES_PER_CHUNK && size != 0) {

        putlong(Buffer_1, addr_left, &mark_dst, &first_time);

        /* add :  after every address passed*/
        mark_dst += 1;
        Buffer_1[mark_dst] = ':';
        Buffer_1[++mark_dst] = ' ';
        printline(size, &mark_dst, &mark_src, Buffer_1, Buffer_2);

        addr_left += 16;
        counter++;

        last_counter_left = counter;

        if ((size = readline(Buffer_2, mark_src, &file_size)) == 0)
            break;
    }
    if (FileSize_left < 240) {
        isEnd_left = TRUE;
        FileSize_restoration_left = 240 - FileSize_left;
    }
}

void buffer_write_right(PTCHAR Buffer_1, PTCHAR Buffer_2, long long chunk_read, char *first_time)
{
    long long size = 0;
    char counter = 0;

    long long mark_dst = 0;
    long long mark_src = 0;

    DWORD file_size = FileSize_right;

    size = readline(Buffer_2, mark_src, &file_size);

    if (size == 0) {
        last_counter_right = 0;
    }
    
    while (counter < LINES_PER_CHUNK && size != 0) {

        putlong(Buffer_1, addr_right, &mark_dst, &first_time);

        /* add :  after every address passed*/
        mark_dst += 1;
        Buffer_1[mark_dst] = ':';
        Buffer_1[++mark_dst] = ' ';
        printline(size, &mark_dst, &mark_src, Buffer_1, Buffer_2);

        addr_right += 16;
        counter++;
        last_counter_right = counter;
        if ((size = readline(Buffer_2, mark_src, &file_size)) == 0)
            break;
    }
    
    if (FileSize_right < 240) isEnd_right = TRUE;

    if (FileSize_right < 240) {
        FileSize_restoration_right = 240 - FileSize_right;
    }
    else
        FileSize_right = file_size;
}

void compare() 
{
    TCHAR Buffer_1[CHUNK_SIZE];
    TCHAR Buffer_2[CHUNK_SIZE];

    GetWindowText(hEdit_left, Buffer_1, sizeof(Buffer_1));
    GetWindowText(hEdit_right, Buffer_2, sizeof(Buffer_2));

    long long i = 0;
    long long j = 0;
    
    while (i != CHUNK_SIZE - 1) {
        if (Buffer_1[i] == '\n') {
            j += 1;
        }
        if (Buffer_1[i] != Buffer_2[i]) {
            if (j == 0) {
                Highlight_hEdit_left(i, i + 1, RED_COLOR);
                Highlight_hEdit_right(i, i + 1, RED_COLOR);
            }
            else {
                Highlight_hEdit_left(i - j, i - j + 1, RED_COLOR);
                Highlight_hEdit_right(i - j, i - j + 1, RED_COLOR);
            }
        }
        i++;
    }
}

void goto_next_chunk(HWND hwnd)
{
    TCHAR buffer_left_1[CHUNK_SIZE];
    memset(buffer_left_1, 0, sizeof(buffer_left_1));
    TCHAR buffer_left_2[LINES_PER_CHUNK * 16 + 1];
    memset(buffer_left_2, 0, sizeof(buffer_left_2));

    TCHAR buffer_right_1[CHUNK_SIZE];
    memset(buffer_right_1, 0, sizeof(buffer_right_1));
    TCHAR buffer_right_2[LINES_PER_CHUNK * 16 + 1];
    memset(buffer_right_2, 0, sizeof(buffer_right_2));

    if (isEnd_left) goto right;
    
    if (olf_left.Offset < sizeof(DWORD))
        ;

    HANDLE hFile_left = NULL;
    if (saved_filename_left[0] == 0) return;
    hFile_left = CreateFile(saved_filename_left, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile_left == INVALID_HANDLE_VALUE)
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);

    DWORD iNumRead_left = 0;

    olf_left.Offset += LINES_PER_CHUNK * 16;

    ReadFile(hFile_left, buffer_left_2, LINES_PER_CHUNK * 16, &iNumRead_left, &olf_left);

    chunk_read_left += 1;
    buffer_write_left(buffer_left_1, buffer_left_2, FileSize_left, chunk_read_left, 0);
    if (FileSize_left < 240) isEnd_left = TRUE;
    SetWindowText(hEdit_left, buffer_left_1);
    CloseHandle(hFile_left);

right:

    if (isEnd_right) return;

    if (olf_right.Offset < sizeof(DWORD))
        ;

    HANDLE hFile_right = NULL;
    if (saved_filename_right[0] == 0) return;
    hFile_right = CreateFile(saved_filename_right, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile_right == INVALID_HANDLE_VALUE)
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);

    DWORD iNumRead_right = 0;

    olf_right.Offset += LINES_PER_CHUNK * 16;

    ReadFile(hFile_right, buffer_right_2, LINES_PER_CHUNK * 16, &iNumRead_right, &olf_right);

    chunk_read_right += 1;
    buffer_write_right(buffer_right_1, buffer_right_2, FileSize_right, chunk_read_right, 0);
    if (FileSize_right < 240) isEnd_right = TRUE;
    SetWindowText(hEdit_right, buffer_right_1);
    CloseHandle(hFile_right);
}

void goto_previous_chunk(HWND hwnd)
{
    TCHAR buffer_left_1[CHUNK_SIZE];
    memset(buffer_left_1, 0, sizeof(buffer_left_1));
    TCHAR buffer_left_2[LINES_PER_CHUNK * 16 + 1];
    memset(buffer_left_2, 0, sizeof(buffer_left_2));

    TCHAR buffer_right_1[CHUNK_SIZE];
    memset(buffer_right_1, 0, sizeof(buffer_right_1));
    TCHAR buffer_right_2[LINES_PER_CHUNK * 16 + 1];
    memset(buffer_right_2, 0, sizeof(buffer_right_2));

    if (chunk_read_left == 0 || (chunk_read_left < chunk_read_right)) goto right;

    olf_left.Offset -= LINES_PER_CHUNK * 16;

    HANDLE hFile_left = NULL;
    if (saved_filename_left[0] == 0) return;
    hFile_left = CreateFile(saved_filename_left, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile_left == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);
        return;
    }
    else {
                chunk_read_left -= 1;
                
                if (FileSize_left < LINES_PER_CHUNK * 16)
                    FileSize_left += FileSize_restoration_left + LINES_PER_CHUNK * 16;
                else
                    FileSize_left += LINES_PER_CHUNK * 16;

                addr_left -= (last_counter_left * LINE_LENGTH + LINES_PER_CHUNK* LINE_LENGTH);
                isEnd_left = FALSE;

                DWORD iNumRead_left = 0;

                ReadFile(hFile_left, buffer_left_2, LINES_PER_CHUNK * 16, &iNumRead_left, &olf_left);
                
                if (addr_left != 0)
                    buffer_write_left(buffer_left_1, buffer_left_2, FileSize_left, chunk_read_left, 0);
                else
                    buffer_write_left(buffer_left_1, buffer_left_2, FileSize_left, chunk_read_left, 1);

                if (FileSize_left < LINES_PER_CHUNK * 16)
                    FileSize_left += FileSize_restoration_left + LINES_PER_CHUNK * 16;
                else
                    FileSize_left += LINES_PER_CHUNK * 16;
                SetWindowText(hEdit_left, buffer_left_1);
                CloseHandle(hFile_left);
    }
    
right:

    if (chunk_read_right == 0 || (chunk_read_right <= chunk_read_left)) return;

    olf_right.Offset -= LINES_PER_CHUNK * 16;

    HANDLE hFile_right = NULL;
    if (saved_filename_right[0] == 0) return;
    hFile_right = CreateFile(saved_filename_right, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile_right == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);
        return;
    }
    else {
        chunk_read_right -= 1;

        if (FileSize_right < LINES_PER_CHUNK * 16)
            FileSize_right += FileSize_restoration_right + LINES_PER_CHUNK * 16;
        else
            FileSize_right += LINES_PER_CHUNK * 16;

        addr_right -= (last_counter_right * LINE_LENGTH + LINES_PER_CHUNK * LINE_LENGTH);
        isEnd_right = FALSE;

        DWORD iNumRead_right = 0;

        ReadFile(hFile_right, buffer_right_2, LINES_PER_CHUNK * 16, &iNumRead_right, &olf_right);
        
        if (addr_right != 0)
            buffer_write_right(buffer_right_1, buffer_right_2, FileSize_right, chunk_read_right, 0);
        else
            buffer_write_right(buffer_right_1, buffer_right_2, FileSize_right, chunk_read_right, 1);

        if (FileSize_right < LINES_PER_CHUNK * 16)
            FileSize_right += FileSize_restoration_right + LINES_PER_CHUNK * 16;
        else
            FileSize_right += LINES_PER_CHUNK * 16;
        SetWindowText(hEdit_right, buffer_right_1);
        CloseHandle(hFile_right);
    }
}