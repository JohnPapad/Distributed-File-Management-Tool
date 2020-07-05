
#ifndef WORKER_H
#define WORKER_H

#include "dir.h"


void worker(unsigned int i, char * boss_worker_ffs, char * worker_boss_ffs);
unsigned int get_total_words(dir ** dirs, unsigned n_dirs);
unsigned int get_total_lines(dir ** dirs, unsigned n_dirs);
unsigned int get_total_chars(dir ** dirs, unsigned n_dirs);
void print_dirs(dir ** dirs, unsigned int n_dirs);
char ** get_queries(int queries_size, unsigned int fd);
void write_wc_in_log_file(char * log_file, char * timestamp, unsigned int n_lines, unsigned int n_words, unsigned int n_chars);
void write_maxcount_in_log_file(char * log_file, char * timestamp, char * keyword, const char * max_txt_file_name, unsigned int maxcount);
void write_mincount_in_log_file(char * log_file, char * timestamp, char * keyword, const char * min_txt_file_name, unsigned int mincount);
void write_search_query_in_log_file(char * log_file, char * timestamp, char * query, char ** unq_txt_files_names, unsigned int unq_txt_files_names_size);



#endif // WORKER_H
