#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <richedit.h>
#include "transform.h"

char isEnd_left = FALSE;
char isEnd_right = FALSE;

long long addr_left = 0;
long long addr_right = 0;

char last_counter_left = 0;
char last_counter_right = 0;

void Highlight_hEdit_left(UINT uStartPos, UINT uEndPos, COLORREF color) {
    CHARRANGE cr;
    CHARRANGE save;
    cr.cpMin = uStartPos;
    cr.cpMax = uEndPos;
    SendMessage(hEdit_left, EM_EXGETSEL, 0, (LPARAM)&save);// запомнить
    SendMessage(hEdit_left, EM_EXSETSEL, 0, (LPARAM)&cr);  // новая позиция
    CHARFORMAT cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0; // add this line
    cf.crTextColor = color;
    SendMessage(hEdit_left, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hEdit_left, EM_EXSETSEL, 0, (LPARAM)&save); // вернуть позицию
}

void Highlight_hEdit_right(UINT uStartPos, UINT uEndPos, COLORREF color) {
    CHARRANGE cr;
    CHARRANGE save;
    cr.cpMin = uStartPos;
    cr.cpMax = uEndPos;
    SendMessage(hEdit_right, EM_EXGETSEL, 0, (LPARAM)&save);// запомнить
    SendMessage(hEdit_right, EM_EXSETSEL, 0, (LPARAM)&cr);  // новая позиция
    CHARFORMAT cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0; // add this line
    cf.crTextColor = color;
    SendMessage(hEdit_right, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SendMessage(hEdit_right, EM_EXSETSEL, 0, (LPARAM)&save); // вернуть позицию
}

/* readline - This routine reads in the next 16 bytes from the file and
              places them in the array named 'row', setting 'size' to
              be the number of bytes read in.  Size will be less than
              16 if EOF was encountered, and may possibly be 0.  */

int readline(PTCHAR buf, long long mark_src) {
    int size = 0;
    while (buf[mark_src++] != '\0') {
        size = size + 1;
        if (buf[mark_src] == '\0') break;
        if (size == 16) break;
    }
    return size;
}

/* putbyt - This routine is passed a byte (i.e., an integer < 256) which
          it displays as 2 hex characters.  If passed a number out of that
          range, it outputs nothing.  */

void putbyt_buf(PTCHAR buf1, PTCHAR buf2, long long* mark_dst, long long* mark_src)
{
    int i;
    if (((buf2[*mark_src]) >= 0) && ((buf2[*mark_src]) <= 255)) {
        i = (buf2[(*mark_src)] & 0x000000f0) >> 4;
        if (i < 10) {
            buf1[++ * mark_dst] = ('0' + i);
        }
        else {
            buf1[++ * mark_dst] = ('A' + i - 10);
        }
        i = (buf2[(*mark_src)] & 0x0000000f) >> 0;
        if (i < 10) {
            buf1[++(*mark_dst)] = ('0' + i);
        }
        else {
            buf1[++(*mark_dst)] = ('A' + i - 10);
        }
    }
}

/* putbyt - This routine is passed a byte (i.e., an integer < 256) which
          it displays as 2 hex characters.  If passed a number out of that
          range, it outputs nothing.  */
void putbyt_addr(int c, PTCHAR Buffer_1, long long* mark_dst)
{
    int i;
    if ((c >= 0) && (c <= 255)) {
        i = (c & 0x000000f0) >> 4;
        if (i < 10) {
            Buffer_1[*mark_dst] = ('0' + i);
        }
        else {
            Buffer_1[*mark_dst] = ('A' + i - 10);
        }
        i = (c & 0x0000000f) >> 0;
        if (i < 10) {
            Buffer_1[++ * mark_dst] = ('0' + i);
        }
        else {
            Buffer_1[++ * mark_dst] = ('A' + i - 10);
        }
    }
}

/* putlong - This routine is passed an integer, which it displays as 8
             hex digits.  */
void putlong(PTCHAR Buffer_1, long long addr, long long* mark_dst, char *first_time)
{
    if (addr >= 16 && *first_time) {
        *mark_dst += 1;
        *first_time = FALSE;
    }
    else
        *mark_dst += 0;
    putbyt_addr(((addr >> 24) & 0x000000ff), Buffer_1, mark_dst);
    *mark_dst += 1;
    putbyt_addr(((addr >> 16) & 0x000000ff), Buffer_1, mark_dst);
    *mark_dst += 1;
    putbyt_addr(((addr >> 8) & 0x000000ff), Buffer_1, mark_dst);
    *mark_dst += 1;
    putbyt_addr(((addr >> 0) & 0x000000ff), Buffer_1, mark_dst);
}

/* printline - This routine prints the current 'row'.  */
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
                if (buf2[*mark_src] == '\0') return;
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

void buffer_write_left(PTCHAR Buffer_1, PTCHAR Buffer_2, long long chunk_read, char *first_time)
{
    long long size = 0;
    char counter = 0;

    long long mark_dst = 0;
    long long mark_src = chunk_read* LINES_PER_CHUNK * 16;

    size = readline(Buffer_2, mark_src);

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
        if ((size = readline(Buffer_2, mark_src)) == 0)
            break;
    }
}

void buffer_write_right(PTCHAR Buffer_1, PTCHAR Buffer_2, long long chunk_read, char *first_time)
{
    long long size = 0;
    char counter = 0;

    long long mark_dst = 0;
    long long mark_src = chunk_read * LINES_PER_CHUNK * 16;

    size = readline(Buffer_2, mark_src);

    while (counter < LINES_PER_CHUNK && size != 0) {

        putlong(Buffer_1, addr_right, &mark_dst, &first_time);

        /* add :  after every address passed*/
        mark_dst += 1;
        Buffer_1[mark_dst] = ':';
        Buffer_1[++mark_dst] = ' ';
        printline(size, &mark_dst, &mark_src, Buffer_1, Buffer_2);

        addr_right += 16;
        if ((size = readline(Buffer_2, mark_src)) == 0)
            break;
        counter++;
        last_counter_right = counter;
    }
    
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
                Highlight_hEdit_left(i - j, i-j+1, RED_COLOR);
                Highlight_hEdit_right(i - j, i-j+1, RED_COLOR);
            }
        }
        i++;
    }
}

void goto_next_chunk(HWND hwnd)
{
    TCHAR buffer_left[CHUNK_SIZE];
    memset(buffer_left, 0, sizeof(buffer_left));
    TCHAR buffer_right[CHUNK_SIZE];
    memset(buffer_right, 0, sizeof(buffer_right));

    if (isEnd_left) goto right;
    

    if ((mapping_left = fileMappingCreate(saved_filename_left,hwnd)) == NULL)
        MessageBox(hwnd, _T("You have to reopen first file for comparison!"), _T("Error!"), NULL);
    else {
        chunk_read_left += 1;
        buffer_write_left(buffer_left, mapping_left->dataPtr, chunk_read_left, 0);
        if (buffer_left[0] == 0) isEnd_left = TRUE;
        SetWindowText(hEdit_left, buffer_left);
        fileMappingClose(mapping_left);
    }

right:

    if (isEnd_right) return;

    if ((mapping_right = fileMappingCreate(saved_filename_right, hwnd)) == NULL)
        MessageBox(hwnd, _T("You have to reopen second file for comparison!"), _T("Error!"), NULL);
    else {
        chunk_read_right += 1;
        buffer_write_right(buffer_right, mapping_right->dataPtr, chunk_read_right, 0);
        if (buffer_right[0] == 0) isEnd_right = TRUE;
        SetWindowText(hEdit_right, buffer_right);
        fileMappingClose(mapping_right);
    }
}

void goto_previous_chunk(HWND hwnd)
{
    TCHAR buffer_left[CHUNK_SIZE];
    memset(buffer_left, 0, sizeof(buffer_left));
    TCHAR buffer_right[CHUNK_SIZE];
    memset(buffer_right, 0, sizeof(buffer_right));

    if (chunk_read_left == 0 || (chunk_read_left < chunk_read_right)) goto right;
    
    if ((mapping_left = fileMappingCreate(saved_filename_left, hwnd)) == NULL)
        MessageBox(hwnd, _T("You have to reopen first file for comparison!"), _T("Error!"), NULL);
    else {
        chunk_read_left -= 1;

        addr_left -= (last_counter_left * 16 + LINES_PER_CHUNK * 16);
        isEnd_left = FALSE;
        if (addr_left != 0)
            buffer_write_left(buffer_left, mapping_left->dataPtr, chunk_read_left, 0);
        else
            buffer_write_left(buffer_left, mapping_left->dataPtr, chunk_read_left, 1);
        SetWindowText(hEdit_left, buffer_left);
        fileMappingClose(mapping_left);
    }

right:

    if (chunk_read_right == 0 || (chunk_read_right < chunk_read_left)) return;

    if ((mapping_right = fileMappingCreate(saved_filename_right, hwnd)) == NULL)
        MessageBox(hwnd, _T("You have to reopen second file for comparison!"), _T("Error!"), NULL);
    else {
        chunk_read_right -= 1;

        addr_right -= (last_counter_right * 16 + LINES_PER_CHUNK * 16);
        isEnd_right = FALSE;
        if (addr_right != 0)
            buffer_write_right(buffer_right, mapping_right->dataPtr, chunk_read_right, 0);
        else
            buffer_write_right(buffer_right, mapping_right->dataPtr, chunk_read_right, 1);
        SetWindowText(hEdit_right, buffer_right);   
        fileMappingClose(mapping_right);
    }
}