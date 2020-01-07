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

#define FILE_MENU_OPEN 1
#define FILE_MENU_COMPARE 2
#define FILE_MENU_EXIT 3
#define OPEN_FIRST_FILE 4
#define OPEN_SECOND_FILE 5
#define NEXT 6
#define BACK 7

#define LINES_PER_CHUNK 14
#define LINE_LENGTH 16

#define CHUNK_SIZE 1200
#define RED_COLOR 0x000000FF

extern HMENU hMenu;
extern HWND hEdit_left;
extern HWND hEdit_right;
struct FileMapping {
    HANDLE hFile;
    HANDLE hMapping;
    size_t fsize;
    PTCHAR dataPtr;
};
extern struct FileMapping* mapping_left;
extern struct FileMapping* mapping_right;
extern long long chunk_read_left;
extern long long chunk_read_right;

extern TCHAR saved_filename_left[100];
extern TCHAR saved_filename_right[100];

extern char isEnd_left;
extern char isEnd_right;

extern char last_counter_left;
extern char last_counter_right;

extern long long addr_left;
extern long long addr_right;

extern int left_menu_id;
extern int right_menu_id;

void Highlight_hEdit_left(UINT uStartPos, UINT uEndPos, COLORREF color);
void Highlight_hEdit_right(UINT uStartPos, UINT uEndPos, COLORREF color);
int readline(PTCHAR buf, long long mark_src);
void putbyt_buf(PTCHAR buf1, PTCHAR buf2, long long* mark_dst, long long* mark_src);
void putbyt_addr(int c, PTCHAR Buffer_1, long long* mark_dst);
void putlong(PTCHAR Buffer_1, long long addr, long long* mark_dst, char *first_time);
void printline(long long size, long long* mark_dst, long long* mark_src, PTCHAR buf1, PTCHAR buf2);
void buffer_write_left(PTCHAR Buffer_1, PTCHAR Buffer_2, long long chunk_read, char *first_time);
void buffer_write_right(PTCHAR Buffer_1, PTCHAR Buffer_2, long long chunk_read, char *first_time);
void compare();
void goto_next_chunk(HWND hwnd);
void goto_previous_chunk(HWND hwnd);
struct FileMapping* fileMappingCreate(PTCHAR path, HWND hwnd);
void fileMappingClose(struct FileMapping* mapping);
void display_file_left(PTCHAR path, HWND hwnd);
void display_file_right(PTCHAR path, HWND hwnd);
void open_file_left(HWND hwnd);
void open_file_right(HWND hwnd);

#endif