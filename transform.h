#pragma once
#ifndef TRANSFORM
#define TRANSFORM

#ifdef _UNICODE 
#define _T(c) L##c
#else 
#define _T(c) c
#endif

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 3000

#define FILE_MENU_OPEN 1
#define FILE_MENU_COMPARE 2
#define FILE_MENU_EXIT 3
#define OPEN_FIRST_FILE 4
#define OPEN_SECOND_FILE 5
#define NEXT 6
#define BACK 7

#define LINES_PER_CHUNK 28
#define LINE_LENGTH 16

#define CHUNK_SIZE 3000
#define SRC_SIZE 2400

#define RED_COLOR 0x000000FF


extern OVERLAPPED olf_left;
extern OVERLAPPED olf_right;

extern LARGE_INTEGER li_left;
extern LARGE_INTEGER li_right;

extern HMENU hMenu;
extern HWND hEdit_left;
extern HWND hEdit_right;

extern long long chunk_read_left;
extern long long chunk_read_right;

extern TCHAR saved_filename_left[100];
extern TCHAR saved_filename_right[100];

extern char isEnd_left;
extern char isEnd_right;

extern long long FileSize_restoration_left;
extern long long FileSize_restoration_right;

extern char last_counter_left;
extern char last_counter_right;

extern long long offset_low_left;
extern long long offset_high_left;
extern long long offset_low_right;
extern long long offset_high_right;

extern char IsInGotoPrevChunk;

extern long long addr_low_left;
extern long long addr_low_right;
extern long long addr_high_left;
extern long long addr_high_right;

extern DWORD FileSize_left;
extern DWORD FileSize_right;

extern int left_menu_id;
extern int right_menu_id;

void Highlight_hEdit_left(UINT uStartPos, UINT uEndPos, COLORREF color);
void Highlight_hEdit_right(UINT uStartPos, UINT uEndPos, COLORREF color);
int readline(PTCHAR buf, long long mark_src, DWORD* file_size);
void putbyt_buf(PTCHAR buf1, PTCHAR buf2, long long* mark_dst, long long* mark_src);
void putbyt_addr(int c, PTCHAR Buffer_1, long long* mark_dst);
void putlong_low(PTCHAR Buffer_1, long long addr, long long* mark_dst);
void putlong_high(PTCHAR Buffer_1, long long addr, long long* mark_dst);
void printline(long long size, long long* mark_dst, long long* mark_src, PTCHAR buf1, PTCHAR buf2);
void buffer_write_left(PTCHAR Buffer_1, PTCHAR Buffer_2);
void buffer_write_right(PTCHAR Buffer_1, PTCHAR Buffer_2);
void compare(HWND hwnd);
void goto_next_chunk(HWND hwnd);
void goto_previous_chunk(HWND hwnd);
void display_file_left(PTCHAR path, HWND hwnd);
void display_file_right(PTCHAR path, HWND hwnd);
void open_file_left(HWND hwnd);
void open_file_right(HWND hwnd);

#endif