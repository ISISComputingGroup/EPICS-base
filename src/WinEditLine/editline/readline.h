/*

readline.h

is part of:

WinEditLine (formerly MinGWEditLine)
Copyright 2010-2014 Paolo Tosco <paolo.tosco@unito.it>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of WinEditLine (formerly MinGWEditLine) nor the
    name of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef _READLINE_H_
#define _READLINE_H_

#include <windows.h>

#include <shareLib.h>

/*
these defines may be changed
*/
#define _EL_BUF_LEN      4096  /* maximum line buffer size */
#define DEFAULT_HISTORY_SIZE    200  /* default number of history entries */

/*
these defines should not be mangled
*/
#define _EL_CONSOLE_BUF_LEN    8
#define _EL_MIN_HIST_SIZE    1
#define _EL_ENV_BUF_LEN      64
#define _EL_MAX_FILE_BREAK_CHARACTERS  64
#define _EL_BASIC_FILE_BREAK_CHARACTERS  _T(" \"\t\n=><|")
#define _EL_BASIC_FILE_QUOTE_CHARACTERS  _T(" $;=&")

/*
these are included because for some reason
they are missing from MinGW gcc 4.5.0
*/
#ifndef ENABLE_EXTENDED_FLAGS
#define ENABLE_EXTENDED_FLAGS    0x0080
#endif
#ifndef ENABLE_INSERT_MODE
#define ENABLE_INSERT_MODE    0x0020
#endif
#ifndef ENABLE_QUICK_EDIT_MODE
#define ENABLE_QUICK_EDIT_MODE    0x0040
#endif


typedef char **rl_completion_func_t(const char *, int, int);
typedef char *rl_compentry_func_t(const char *, int);
typedef void rl_compentryfree_func_t(void *);
/*
this is ignored by WinEditLine
*/
typedef void *histdata_t;

typedef struct _hist_entry {
  char *line;
  char *timestamp;
  histdata_t data;
} HIST_ENTRY;

typedef struct _hist_state {
  HIST_ENTRY **entries;
  int offset;
  int length;
  int size;
  int flags;
} HISTORY_STATE;


#ifdef __cplusplus
extern "C" {
#endif
/*
prototypes of internal functions
*/
char **_el_alloc_array(int n, int size);
wchar_t *_el_mb2w(char *mb, wchar_t **w);
char *_el_w2mb(wchar_t *w, char **mb);
void _el_free_array(void *array);
void _el_clean_exit();
int _el_insert_char(wchar_t *buf, int n);
int _el_delete_char(UINT32 vk, int n);
void _el_add_char(wchar_t *string, wchar_t c, int n);
int _el_pad(CONSOLE_SCREEN_BUFFER_INFO *sbInfo, wchar_t *_el_temp_print);
int _el_print_string(wchar_t *string);
int _el_move_cursor(UINT32 vk, UINT32 ctrl);
int _el_set_cursor(int offset);
void _el_remove_tail_spaces(char *line);
wchar_t *_el_get_compl_text(int *start, int *end);
int _el_check_root_identity(wchar_t *root, wchar_t *entry_name);
int _el_fn_qsort_string_compare(const void *i1, const void *i2);
void _el_display_history();
int _el_find_history_file(const char *filename, wchar_t **name);
int _el_display_prev_hist();
int _el_display_next_hist();
int _el_display_first_hist();
int _el_display_last_hist();
BOOL _el_signal_handler(DWORD fdwCtrlType);
BOOL _el_replace_char(wchar_t *string, wchar_t src, wchar_t dest);

/*
prototypes of functions which may be
called by the user
*/
epicsShareFunc void  source_editrc();
epicsShareFunc char*  readline(const char *prompt);
epicsShareFunc char**  rl_completion_matches(const char *text, char *entry_func(const char *, int));
epicsShareFunc char*  rl_filename_completion_function(const char *text, int state);
epicsShareFunc void  rl_free(void *mem);
epicsShareFunc int  using_history();
epicsShareFunc void  free_history();
epicsShareFunc void  free_history_entry(HIST_ENTRY *entry);
epicsShareFunc void  clear_history();
epicsShareFunc char*  add_history(char *line);
epicsShareFunc HIST_ENTRY*  remove_history(int i);
epicsShareFunc HIST_ENTRY*  replace_history_entry(int i, char *line, histdata_t dummy);
epicsShareFunc HIST_ENTRY**  history_list();
epicsShareFunc int  where_history();
epicsShareFunc int  history_length();
epicsShareFunc HIST_ENTRY*  current_history();
epicsShareFunc HIST_ENTRY*  history_get(int offset);
epicsShareFunc int  history_set_pos(int i);
epicsShareFunc HIST_ENTRY*  previous_history();
epicsShareFunc HIST_ENTRY*  next_history();
epicsShareFunc int  read_history(const char *filename);
epicsShareFunc int  write_history(const char *filename);

/*
extern variables
*/
epicsShareExtern char *rl_line_buffer;
epicsShareExtern wchar_t *_el_line_buffer;
epicsShareExtern wchar_t *_el_print;
epicsShareExtern wchar_t *_el_temp_print;
epicsShareExtern wchar_t *_el_next_compl;
epicsShareExtern wchar_t *_el_file_name;
epicsShareExtern wchar_t *_el_dir_name;
epicsShareExtern wchar_t *_el_old_arg;
epicsShareExtern wchar_t *_el_wide;
epicsShareExtern wchar_t *_el_text;
epicsShareExtern char *_el_text_mb;
epicsShareExtern char *rl_prompt;
epicsShareExtern wchar_t *_el_prompt;
epicsShareExtern wchar_t **_el_compl_array;
epicsShareExtern wchar_t *_el_basic_word_break_characters;
epicsShareExtern wchar_t *_el_completer_word_break_characters;
epicsShareExtern wchar_t _el_basic_file_break_characters[_EL_MAX_FILE_BREAK_CHARACTERS];
epicsShareExtern int _el_ctrl_c_pressed;
epicsShareExtern int rl_point;
epicsShareExtern int rl_attempted_completion_over;
epicsShareExtern int rl_completion_append_character;
epicsShareExtern const char rl_basic_word_break_characters[];
epicsShareExtern const char *rl_completer_word_break_characters;
epicsShareExtern int _el_compl_index;
epicsShareExtern int _el_n_compl;
epicsShareExtern int _el_prompt_len;
epicsShareExtern const char *rl_readline_name;
epicsShareExtern rl_completion_func_t *rl_attempted_completion_function;
epicsShareExtern rl_compentry_func_t *rl_completion_entry_function;
epicsShareExtern rl_compentryfree_func_t *rl_user_completion_entry_free_function;
epicsShareExtern BOOL _el_prev_in_cm_saved;
epicsShareExtern BOOL _el_prev_out_cm_saved;
epicsShareExtern  DWORD _el_prev_in_cm;
epicsShareExtern  DWORD _el_prev_out_cm;
epicsShareExtern  HANDLE _el_h_in;
epicsShareExtern  HANDLE _el_h_out;
epicsShareExtern  COORD _el_prev_size;

#ifdef __cplusplus
}
#endif

#endif /* _READLINE_H_ */
