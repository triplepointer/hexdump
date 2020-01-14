#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <richedit.h>
#include "transform.h"

long long addr_low_left = 0;
long long addr_low_right = 0;
long long addr_high_left = 0;
long long addr_high_right = 0;

long long FileSize_restoration_left = 0;
long long FileSize_restoration_right = 0;

char IsInGotoPrevChunk = FALSE;
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

void putlong_high(PTCHAR Buffer_1, long long addr, long long* mark_dst)
{
    *mark_dst += 0;
    putbyt_addr(((addr >> 24) & 0x000000ff), Buffer_1, mark_dst); // 3rd byte
    *mark_dst += 1;
    putbyt_addr(((addr >> 16) & 0x000000ff), Buffer_1, mark_dst); // 2nd byte
    *mark_dst += 1;
    putbyt_addr(((addr >> 8) & 0x000000ff), Buffer_1, mark_dst); // 1st byte
    *mark_dst += 1;
    putbyt_addr(((addr >> 0) & 0x000000ff), Buffer_1, mark_dst); // 0th byte
}

void putlong_low(PTCHAR Buffer_1, long long addr, long long* mark_dst)
{
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

void buffer_write_left(PTCHAR Buffer_1, PTCHAR Buffer_2)
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
        putlong_high(Buffer_1, addr_high_left, &mark_dst);
        putlong_low(Buffer_1, addr_low_left, &mark_dst);

        /* add :  after every address passed*/
        mark_dst += 1;
        Buffer_1[mark_dst] = ':';
        Buffer_1[++mark_dst] = ' ';
        printline(size, &mark_dst, &mark_src, Buffer_1, Buffer_2);

        if (addr_low_left < 0xFFFFFFFF - 16)
            addr_low_left += 16;
        else {
            addr_high_left += 1;
            addr_low_left = 16 - (0xFFFFFFFF - addr_low_left);
        }
        counter++;

        last_counter_left = counter;

        if ((size = readline(Buffer_2, mark_src, &file_size)) == 0)
            break;
    }
    if (FileSize_left < 464 && file_size == 0 && !isEnd_left && !IsInGotoPrevChunk) {
        FileSize_restoration_left = FileSize_left;
        FileSize_left = file_size;
    }
    else {
        if (IsInGotoPrevChunk)
            ;
        else
            FileSize_left = file_size;
    }
        if (file_size == 0 && !IsInGotoPrevChunk) isEnd_left = TRUE;
}

void buffer_write_right(PTCHAR Buffer_1, PTCHAR Buffer_2)
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
        putlong_high(Buffer_1, addr_high_right, &mark_dst);
        putlong_low(Buffer_1, addr_low_right, &mark_dst);

        /* add :  after every address passed*/
        mark_dst += 1;
        Buffer_1[mark_dst] = ':';
        Buffer_1[++mark_dst] = ' ';
        printline(size, &mark_dst, &mark_src, Buffer_1, Buffer_2);

        if (addr_low_right < 0xFFFFFFFF - 16)
            addr_low_right += 16;
        else {
            addr_high_right += 1;
            addr_low_right = 16 - (0xFFFFFFFF - addr_low_right);
        }
        counter++;

        last_counter_right = counter;

        if ((size = readline(Buffer_2, mark_src, &file_size)) == 0)
            break;
    }
    if (FileSize_right < 464 && file_size == 0 && !isEnd_right && !IsInGotoPrevChunk) {
        FileSize_restoration_right = FileSize_right;
        FileSize_right = file_size;
    }
    else {
        if (IsInGotoPrevChunk)
            ;
        else
            FileSize_right = file_size;
    }
    if (file_size == 0 && !IsInGotoPrevChunk) isEnd_right = TRUE;
}

void compare(HWND hwnd) 
{
    if (saved_filename_left[0] == 0 || saved_filename_right[0] == 0) {
        MessageBox(hwnd, _T("You should open two files!"), _T("Error!"), NULL);
        return;
    }
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
    IsInGotoPrevChunk = FALSE;
    if (saved_filename_left[0] == 0 || saved_filename_right[0] == 0) {
        MessageBox(hwnd, _T("You should open two files!"), _T("Error!"), NULL);
        return;
    }
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

    if (olf_left.Offset < (0xFFFFFFFF - LINES_PER_CHUNK * 16))
        olf_left.Offset += LINES_PER_CHUNK * 16;
    else {
        olf_left.OffsetHigh += 1;
        olf_left.Offset = LINES_PER_CHUNK * 16 - (0xFFFFFFFF - olf_left.Offset);
    }

    ReadFile(hFile_left, buffer_left_2, LINES_PER_CHUNK * 16, &iNumRead_left, &olf_left);

    chunk_read_left += 1;
    buffer_write_left(buffer_left_1, buffer_left_2);
    SetWindowText(hEdit_left, buffer_left_1);
    CloseHandle(hFile_left);

right:

    if (isEnd_right) return;



    HANDLE hFile_right = NULL;
    if (saved_filename_right[0] == 0) return;
    hFile_right = CreateFile(saved_filename_right, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile_right == INVALID_HANDLE_VALUE)
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);

    DWORD iNumRead_right = 0;

    if (olf_right.Offset < (0xFFFFFFFF - LINES_PER_CHUNK * 16))
        olf_right.Offset += LINES_PER_CHUNK * 16;
    else {
        olf_right.OffsetHigh += 1;
        olf_right.Offset = LINES_PER_CHUNK*16 - (0xFFFFFFFF - olf_right.Offset);
    }

    ReadFile(hFile_right, buffer_right_2, LINES_PER_CHUNK * 16, &iNumRead_right, &olf_right);

    chunk_read_right += 1;
    buffer_write_right(buffer_right_1, buffer_right_2);
    SetWindowText(hEdit_right, buffer_right_1);
    CloseHandle(hFile_right);
}

void goto_previous_chunk(HWND hwnd)
{
    IsInGotoPrevChunk = TRUE;
    if (saved_filename_left[0] == 0 || saved_filename_right[0] == 0) {
        MessageBox(hwnd, _T("You should open two files!"), _T("Error!"), NULL);
        return;
    }
    TCHAR buffer_left_1[CHUNK_SIZE];
    memset(buffer_left_1, 0, sizeof(buffer_left_1));
    TCHAR buffer_left_2[LINES_PER_CHUNK * 16 + 1];
    memset(buffer_left_2, 0, sizeof(buffer_left_2));

    TCHAR buffer_right_1[CHUNK_SIZE];
    memset(buffer_right_1, 0, sizeof(buffer_right_1));
    TCHAR buffer_right_2[LINES_PER_CHUNK * 16 + 1];
    memset(buffer_right_2, 0, sizeof(buffer_right_2));

    if (chunk_read_left == 0 || (chunk_read_left < chunk_read_right)) goto right;


    //if (olf_right.Offset < (0xFFFFFFFF - LINES_PER_CHUNK * 16))
    //    olf_right.Offset += LINES_PER_CHUNK * 16;
    //else {
    //    olf_right.OffsetHigh += 1;
    //    olf_right.Offset = LINES_PER_CHUNK * 16 - (0xFFFFFFFF - olf_right.Offset);
    //}

    if (olf_left.OffsetHigh > 0 && olf_left.Offset > 0) {
        if (olf_left.Offset >= LINES_PER_CHUNK * 16) {
            olf_left.OffsetHigh -= 1;
            olf_left.Offset -= LINES_PER_CHUNK * 16;
        }
        else if (olf_left.Offset < LINES_PER_CHUNK * 16) {
            olf_left.OffsetHigh -= 1;
            olf_left.Offset = 1 + (0xFFFFFFFF - LINES_PER_CHUNK * 16) + olf_left.Offset;
        }
    }
    else if (olf_left.OffsetHigh > 0 && olf_left.Offset == 0) {
        olf_left.OffsetHigh -= 1;
        olf_left.Offset = 1 + (0xFFFFFFFF - LINES_PER_CHUNK * 16);
    }
    else
    {
        olf_left.Offset -= LINES_PER_CHUNK * 16;
    }

    HANDLE hFile_left = NULL;
    if (saved_filename_left[0] == 0) return;
    hFile_left = CreateFile(saved_filename_left, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile_left == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);
        return;
    }
    else {
                chunk_read_left -= 1;
                if (isEnd_left)
                    FileSize_left += FileSize_restoration_left;
                else
                    FileSize_left += 464;

                addr_low_left -= (last_counter_left * LINE_LENGTH + LINES_PER_CHUNK* LINE_LENGTH);
                isEnd_left = FALSE;

                DWORD iNumRead_left = 0;

                ReadFile(hFile_left, buffer_left_2, LINES_PER_CHUNK * 16, &iNumRead_left, &olf_left);
                
                if (addr_low_left != 0)
                    buffer_write_left(buffer_left_1, buffer_left_2);
                else
                    buffer_write_left(buffer_left_1, buffer_left_2);
                SetWindowText(hEdit_left, buffer_left_1);
                CloseHandle(hFile_left);
    }
    
right:

    if (chunk_read_right == 0 || (chunk_read_right <= chunk_read_left)) return;

    if (olf_right.OffsetHigh > 0 && olf_right.Offset > 0) {
        if (olf_right.Offset >= LINES_PER_CHUNK * 16) {
            olf_right.OffsetHigh -= 1;
            olf_right.Offset -= LINES_PER_CHUNK * 16;
        }
        else if (olf_right.Offset < LINES_PER_CHUNK * 16) {
            olf_right.OffsetHigh -= 1;
            olf_right.Offset = 1 + (0xFFFFFFFF - LINES_PER_CHUNK * 16) + olf_right.Offset;
        }
    }
    else if (olf_right.OffsetHigh > 0 && olf_right.Offset == 0) {
        olf_right.OffsetHigh -= 1;
        olf_right.Offset = 1 + (0xFFFFFFFF - LINES_PER_CHUNK * 16);
    }
    else
    {
        olf_right.Offset -= LINES_PER_CHUNK * 16;
    }

    HANDLE hFile_right = NULL;
    if (saved_filename_right[0] == 0) return;
    hFile_right = CreateFile(saved_filename_right, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile_right == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, _T("Can't create file!"), _T("Error!"), NULL);
        return;
    }
    else {
        chunk_read_right -= 1;

        if (FileSize_right < 464)
            FileSize_right += FileSize_restoration_right + 464;
        else
            FileSize_right += 464;

        addr_low_right -= (last_counter_right * LINE_LENGTH + LINES_PER_CHUNK * LINE_LENGTH);
        isEnd_right = FALSE;

        DWORD iNumRead_right = 0;

        ReadFile(hFile_right, buffer_right_2, LINES_PER_CHUNK * 16, &iNumRead_right, &olf_right);
        
        if (addr_low_right != 0)
            buffer_write_right(buffer_right_1, buffer_right_2);
        else
            buffer_write_right(buffer_right_1, buffer_right_2);

        if (FileSize_right < 464)
            FileSize_right += FileSize_restoration_right + 464;
        else
            FileSize_right += 464;
        SetWindowText(hEdit_right, buffer_right_1);
        CloseHandle(hFile_right);
    }
}