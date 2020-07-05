#ifndef JOBEXEC_H
#define JOBEXEC_H

extern int *  workers_pids;
extern unsigned int workers_pids_size;

void job_executor(char ** boss_worker_ffs, char ** worker_boss_ffs, char ** dirs, unsigned int N, unsigned int n_dirs);
char * get_maxcount_txt_file_name(int * worker_to_jobexec_fds, int & maxcount, unsigned int N);
char * get_mincount_txt_file_name(int * worker_to_jobexec_fds, int & mincount, unsigned int N);
void get_total_stats(int * worker_to_jobexec_fds, int & total_lines, int & total_words, int & total_chars, unsigned int N);
char ** get_unq_search_queries(char ** cmd_words, unsigned int cmd_words_size, int & unq_search_queries_size);
void set_alive_workers(unsigned int N);
void reset_alive_workers(unsigned int N);
void worker_died(int);
void alarm_handler(int);
void open_fifos(char ** boss_worker_ffs, char ** worker_boss_ffs, int * jobexec_to_worker_fds, int * worker_to_jobexec_fds, unsigned int N, bool check_for_alives);
void respawn_workers(char ** dirs, unsigned int n_dirs, int ** dirs_indexes_per_worker, unsigned int dirs_per_worker, int * jobexec_to_worker_fds, int * worker_to_jobexec_fds, char ** boss_worker_ffs, char ** worker_boss_ffs, unsigned int N);
void read_search_answers(unsigned int deadline, unsigned int N, int * worker_to_jobexec_fds);



#endif // JOBEXEC_H
