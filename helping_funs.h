
#ifndef HELPING_FUNS_H
#define HELPING_FUNS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <ctype.h>
#include <limits>
#include <time.h>
#include <signal.h>

#include <fstream>

#define BUFSIZE 5024

using namespace std;

char * check_program_args(char ** argv, int argc, unsigned int & N);
unsigned int get_number_of_digits(int num);
void upper_string(char s[]);

FILE * open_file(char * fn);
unsigned int get_number_of_lines(FILE * fp);
char * read_str(unsigned int str_len);
void delete_str_array(char ** str_array, unsigned int array_size);
bool parse_docs(char **docs, FILE * fp);
char ** parse_text_file(FILE * fp, unsigned int & n_lines, unsigned int & n_chars);
char * compress_line_id(unsigned int dir_id, unsigned int text_file_id, unsigned int line_id);
unsigned int extract_line_id(char * cnct_str, unsigned int & dir_id, unsigned int & text_file_id);
char * make_full_file_path(char * path, char * text_file_name);
bool is_number(const char *str);
bool is_unsigned_int_number(const char *str);
char ** strip_line_into_words(char * cmd, unsigned int & cmd_nwords);
char * get_timestamp();
void sep_term_line();

int send_msg(const char * msg, unsigned int fd);
char * read_msg(unsigned int fd);

int send_int_number(unsigned int fd, int number);
int read_int_number(unsigned int fd);



#endif

