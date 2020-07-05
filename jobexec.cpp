
#include "helping_funs.h"
#include "jobexec.h"
#include "worker.h"
#include <poll.h>

int * workers_pids = NULL;
unsigned int workers_pids_size;

bool * alive_workers = NULL;
volatile sig_atomic_t  some_worker_died = 0;
volatile sig_atomic_t  timeout = 0;


void job_executor(char ** boss_worker_ffs, char ** worker_boss_ffs, char ** dirs, unsigned int N, unsigned int n_dirs)
{
    struct sigaction sa;            //for handling signals
    sa.sa_handler = worker_died;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    struct sigaction sa2;
    sa2.sa_handler = SIG_IGN;
    sigemptyset (&sa2.sa_mask);
    sa2.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sa2, NULL);

    struct sigaction sa3;
    sa3.sa_handler = alarm_handler;
    sigemptyset (&sa3.sa_mask);
    sa3.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sa3, NULL);

    int jobexec_to_worker_fds[N]; //storing fifos' fds
    int worker_to_jobexec_fds[N];

    set_alive_workers(N);
    open_fifos(boss_worker_ffs, worker_boss_ffs, jobexec_to_worker_fds, worker_to_jobexec_fds, N, 1);

    unsigned int dirs_per_worker = n_dirs / N;
    unsigned int additional_dirs = n_dirs % N;
    unsigned int dirs_index = 0;

    int ** dirs_indexes_per_worker = (int **)malloc(N * sizeof(int *));
    for(unsigned int i = 0; i < N; i++)
    {//creating 2d array for storing dirs' indexes (will be used for respawning dead workers)
        dirs_indexes_per_worker[i] = (int *)malloc((dirs_per_worker + 1) * sizeof(int));
    }

    for(unsigned int i = 0; i < N; i++)
    {//distributing dirs equally to workers
        for(unsigned int j = 0; j < dirs_per_worker; j++)
        {
            if(alive_workers[i] == 1)
            {
                send_msg(dirs[dirs_index], jobexec_to_worker_fds[i]);
            }
            dirs_indexes_per_worker[i][j] = dirs_index;
            dirs_index++;
        }

        if (additional_dirs > 0)
        {
            if(alive_workers[i] == 1)
            {
                send_msg(dirs[dirs_index], jobexec_to_worker_fds[i]);
            }
            dirs_indexes_per_worker[i][dirs_per_worker] = dirs_index;
            additional_dirs--;
            dirs_index++;
        }
        else
        {
            dirs_indexes_per_worker[i][dirs_per_worker] = -1;
        }

        if(alive_workers[i] == 1)
        {//indicates that all dirs have been sent
            send_msg("-!-dirs_have_been_sent-!-", jobexec_to_worker_fds[i]);
        }
    }

    for(unsigned int i = 0; i < N; i++)
    {//make sure every worker has finished preprocessing (creating maps and trie)
        if(alive_workers[i] == 1)
        {
            char * inc_msg = read_msg(worker_to_jobexec_fds[i]);
            free(inc_msg);
        }
    }

    while(1)
    {//starting interaction
        if(some_worker_died == 1)
        {
            respawn_workers(dirs, n_dirs, dirs_indexes_per_worker, dirs_per_worker,
                            jobexec_to_worker_fds, worker_to_jobexec_fds,
                            boss_worker_ffs, worker_boss_ffs, N);
        }

        cout<<"-> Please type a command.. ";
        char * cmd = read_str(1); //read user's command
        cout<<endl;

        if (strlen(cmd) == 0)
        {
            free(cmd);
            continue;
        }

        if (strcmp("/exit", cmd) == 0)
        {
            for(unsigned int i = 0; i < N; i++)
            {//notify every worker to exit
                if (alive_workers[i] == 1)
                {
                    send_msg(cmd, jobexec_to_worker_fds[i]);
                }
            }

            for(unsigned int i = 0; i < N; i++)
            {//receiving matched strings info from every worker
                if(alive_workers[i] == 1)
                {//worker still running
                    int matched_strings = read_int_number(worker_to_jobexec_fds[i]);

                    if(matched_strings == -1)
                    {
                        cout<<"--> No searching queries had been given to worker with pid: '"<<workers_pids[i]<<"'"<<endl;
                    }
                    else
                    {
                        cout<<"--> Worker's (pid: '"<<workers_pids[i]<<"') number of matched searching queries: '"<<matched_strings<<"'"<<endl;
                    }
                    cout<<"--> Worker with pid: '"<<workers_pids[i]<<"' will exit."<<endl;
                }
                else
                {
                    cout<<"--> Worker with pid: '"<<workers_pids[i]<<"' has died"<<endl;
                }
                sep_term_line();
            }

            for(unsigned int i = 0; i < N; i++)
            {//make sure every worker will finish (in case some have already finished)
                if (alive_workers[i] == 1)
                {
                    send_msg("exit", jobexec_to_worker_fds[i]);
                }
            }

            free(cmd);

            while (wait(NULL) > 0);  //wait for every worker(child) to finish first
            break;
        }
        else
        {
            char * cmd_cpy = (char *)malloc((strlen(cmd) + 1) * sizeof(char));
            strcpy(cmd_cpy, cmd);

            unsigned int cmd_nwords;
            char ** cmd_words = strip_line_into_words(cmd_cpy, cmd_nwords); //get arguments from trie command
            free(cmd_cpy);

            if (strcmp("/wc", cmd_words[0]) == 0)
            {
                if (cmd_nwords == 1)
                {
                    for(unsigned int i = 0; i < N; i++)
                    {//informing every worker for wc command
                        if (alive_workers[i] == 1)
                        {
                            send_msg(cmd, jobexec_to_worker_fds[i]);
                        }
                    }

                    int total_lines = 0;
                    int total_words = 0;
                    int total_chars = 0;

                    get_total_stats(worker_to_jobexec_fds, total_lines, total_words, total_chars, N);

                    cout<<"--> Printing total stast <--"<<endl;
                    if(some_worker_died == 0)
                    {//all workers have to answer to sum total stats
                        cout<<"-> Total lines: "<<total_lines<<endl;
                        cout<<"-> Total words: "<<total_words<<endl;
                        cout<<"-> Total characters: "<<total_chars<<endl;
                    }
                    else
                    {
                        cout<<"->Cannot print stats. Some workers died. Please repeat /wc command"<<endl;
                    }
                    cout<<endl;
                }
                else
                {
                    cout<<"-! ERROR - too many arguments for /wc command !-"<<endl<<endl;
                }

            }
            else if (strcmp("/maxcount", cmd_words[0]) == 0)
            {
                if (cmd_nwords == 2)
                {
                    for(unsigned int i = 0; i < N; i++)
                    {//informing every worker for maxcount command
                        if (alive_workers[i] == 1)
                        {
                            send_msg(cmd, jobexec_to_worker_fds[i]);
                        }
                    }

                    int maxcount;
                    char * max_txt_file_name = get_maxcount_txt_file_name(worker_to_jobexec_fds, maxcount, N);
                    if(some_worker_died == 0)
                    {//all workers have to answer to get maxcount result
                        if(max_txt_file_name != NULL)
                        {
                            cout<<"--> /maxcount '"<<cmd_words[1]<<"' result: '"<<max_txt_file_name<<" ("<<maxcount<<")"<<endl;
                        }
                        else
                        {
                            cout<<"--> /maxcount '"<<cmd_words[1]<<"' result: 'WORD NOT FOUND'"<<endl;
                        }
                    }
                    else
                    {
                        cout<<"->Cannot print /maxcount result. Some workers died. Please repeat /macount 'keyword' command"<<endl;
                    }
                    cout<<endl;
                    free(max_txt_file_name);

                }
                else if (cmd_nwords == 1)
                {
                    cout<<"-! ERROR - you must enter a keyword !-"<<endl<<endl;
                }
                else
                {
                    cout<<"-! ERROR - too many arguments for /maxcount command !-"<<endl<<endl;
                }

            }
            else if (strcmp("/mincount", cmd_words[0]) == 0)
            {
                if (cmd_nwords == 2)
                {
                    for(unsigned int i = 0; i < N; i++)
                    {//informing every worker for mincount command
                        if (alive_workers[i] == 1)
                        {
                            send_msg(cmd, jobexec_to_worker_fds[i]);
                        }
                    }

                    int mincount;
                    char * min_txt_file_name = get_mincount_txt_file_name(worker_to_jobexec_fds, mincount, N);
                    if(some_worker_died == 0)
                    {//all workers have to answer to get mincount result
                        if(min_txt_file_name != NULL)
                        {
                            cout<<"--> /mincount '"<<cmd_words[1]<<"' result: '"<<min_txt_file_name<<" ("<<mincount<<")"<<endl;
                        }
                        else
                        {
                            cout<<"--> /mincount '"<<cmd_words[1]<<"' result: 'WORD NOT FOUND'"<<endl;
                        }
                    }
                    else
                    {
                        cout<<"->Cannot print /mincount result. Some workers died. Please repeat /minount 'keyword' command"<<endl;
                    }
                    cout<<endl;
                    free(min_txt_file_name);

                }
                else if (cmd_nwords == 1)
                {
                    cout<<"-! ERROR - you must enter a keyword !-"<<endl<<endl;
                }
                else
                {
                    cout<<"-! ERROR - too many arguments for /mincount command !-"<<endl<<endl;
                }

            }
            else if (strcmp("/search", cmd_words[0]) == 0)
            {
                if (cmd_nwords < 4)
                {
                    cout<<"-! ERROR - wrong number of arguments for /search command !-"<<endl<<endl;
                }
                else
                {
                    if(strcmp(cmd_words[cmd_nwords - 2], "-d") != 0)
                    {
                        cout<<"-! ERROR - you must enter '-d' parameter at the penultimate position of the query !-"<<endl<<endl;
                    }
                    else
                    {
                        if( !is_unsigned_int_number(cmd_words[cmd_nwords - 1]) )
                        {
                            cout<<"-! ERROR - last argument must be a number in the /search command !-"<<endl<<endl;
                        }
                        else
                        {
                            unsigned int deadline = atoi(cmd_words[cmd_nwords - 1]);

                            //checking for duplicate queries . only keeping unique ones
                            int unq_search_queries_size;
                            char ** unq_search_queries = get_unq_search_queries(cmd_words, cmd_nwords, unq_search_queries_size);

                            for(unsigned int i = 0; i < N; i++)
                            {//send to each worker the number of expected queries and the queries themselves
                                if(alive_workers[i] == 1)
                                {
                                    send_msg(cmd_words[0], jobexec_to_worker_fds[i]);
                                }

                                if(alive_workers[i] == 1)
                                {
                                    send_int_number(jobexec_to_worker_fds[i], unq_search_queries_size);
                                }

                                for(int j = 0; j < unq_search_queries_size; j++)
                                {
                                    if(alive_workers[i] == 1)
                                    {
                                        send_msg(unq_search_queries[j], jobexec_to_worker_fds[i]);
                                    }
                                }
                            }

                            read_search_answers(deadline, N, worker_to_jobexec_fds);
                            free(unq_search_queries);
                        }
                    }
                }
            }
            else
            {
                cout<<"-! ERROR - command not found !-"<<endl<<endl;
            }

            delete_str_array(cmd_words, cmd_nwords);
            free(cmd);
        }
    }

    //cleaning allocated memory
    free(alive_workers);

    for(unsigned int i = 0; i < N; i++)
    {
        free(dirs_indexes_per_worker[i]);
    }
    free(dirs_indexes_per_worker);

    for(unsigned int i = 0; i < N; i++)
    {
        close(jobexec_to_worker_fds[i]);
        close(worker_to_jobexec_fds[i]);

        unlink(boss_worker_ffs[i]);
        unlink(worker_boss_ffs[i]);
    }

}


char * get_maxcount_txt_file_name(int * worker_to_jobexec_fds, int & maxcount, unsigned int N)
{
    char * max_txt_file_name = NULL;
    maxcount = 0;

    for(unsigned int i = 0; i < N; i++)
    {//take every worker's maxcount text file compare it with the others and keep the overall
        char * text_file_name = NULL;
        if(alive_workers[i] == 1)
        {
            text_file_name = read_msg(worker_to_jobexec_fds[i]);
        }

        int counter = 0;
        if(alive_workers[i] == 1)
        {
            counter = read_int_number(worker_to_jobexec_fds[i]);
        }

        if((counter > 0) && (some_worker_died == 0))
        {//no point checking for maxcount file name if at least one work has died
            if(counter > maxcount)
            {//found new temp overall
                maxcount = counter;
                free(max_txt_file_name);
                max_txt_file_name = text_file_name;
            }
            else if(counter == maxcount)
            {//comparing text files alphabetically
                if(strcmp(max_txt_file_name, text_file_name) > 0)
                {//found new temp overall
                    free(max_txt_file_name);
                    max_txt_file_name = text_file_name;
                }
                else
                {//keep old one - dump current
                   free(text_file_name);
                }
            }
            else
            {
                free(text_file_name);
            }
        }
        else
        {
            free(text_file_name);
        }
    }

    return max_txt_file_name;
}


char * get_mincount_txt_file_name(int * worker_to_jobexec_fds, int & mincount, unsigned int N)
{//accordingly as above at the maxcount function
    char * min_txt_file_name = NULL;
    mincount = numeric_limits<int>::max();

    for(unsigned int i = 0; i < N; i++)
    {
        char * text_file_name = NULL;
        if(alive_workers[i] == 1)
        {
            text_file_name = read_msg(worker_to_jobexec_fds[i]);
        }

        int counter = 0;
        if(alive_workers[i] == 1)
        {
            counter = read_int_number(worker_to_jobexec_fds[i]);
        }

        if((counter > 0) && (some_worker_died == 0))
        {
            if(counter < mincount)
            {
                mincount = counter;
                free(min_txt_file_name);
                min_txt_file_name = text_file_name;
            }
            else if(counter == mincount)
            {
                if(strcmp(min_txt_file_name, text_file_name) > 0)
                {
                    free(min_txt_file_name);
                    min_txt_file_name = text_file_name;
                }
                else
                {
                   free(text_file_name);
                }
            }
            else
            {
                free(text_file_name);
            }
        }
        else
        {
            free(text_file_name);
        }
    }

    return min_txt_file_name;
}


void get_total_stats(int * worker_to_jobexec_fds, int & total_lines, int & total_words, int & total_chars, unsigned int N)
{
    //take and sum stats of every worker
    for(unsigned int i = 0; i < N; i++)
    {//only read stats if worker is still running
        if(alive_workers[i] == 1)
        {
            int n_lines = read_int_number(worker_to_jobexec_fds[i]);
            total_lines += n_lines;
        }

        if(alive_workers[i] == 1)
        {
            int n_words = read_int_number(worker_to_jobexec_fds[i]);
            total_words += n_words;
        }

        if(alive_workers[i] == 1)
        {
            int n_chars = read_int_number(worker_to_jobexec_fds[i]);
            total_chars += n_chars;
        }
    }
}


char ** get_unq_search_queries(char ** cmd_words, unsigned int cmd_words_size, int & unq_search_queries_size)
{//get rid of duplicate searching queries
    char ** unq_search_queries = NULL;
    unq_search_queries_size = 0;

    for(unsigned int i = 1; i < cmd_words_size - 2; i++)
    {
        bool already_exists = 0;
        for(int j = 0; j < unq_search_queries_size; j++)
        {
            if(strcmp(cmd_words[i], unq_search_queries[j]) == 0)
            {
                already_exists = 1;
                break;
            }
        }

        if(!already_exists)
        {
            unq_search_queries = (char **)realloc(unq_search_queries, (unq_search_queries_size + 1) * sizeof(char *));
            unq_search_queries[unq_search_queries_size] = cmd_words[i];
            unq_search_queries_size++;
        }
    }

    return unq_search_queries;
}


void set_alive_workers(unsigned int N)
{//at first all workers are alive
    alive_workers = (bool *)malloc(N * sizeof(bool));

    for(unsigned int i = 0; i < N; i++)
    {
        alive_workers[i] = 1;
    }
}


void reset_alive_workers(unsigned int N)
{
    for(unsigned int i = 0; i < N; i++)
    {
        alive_workers[i] = 1;
    }
}


void open_fifos(char ** boss_worker_ffs, char ** worker_boss_ffs, int * jobexec_to_worker_fds, int * worker_to_jobexec_fds, unsigned int N, bool check_for_alives)
{//if check for alives flag is 1 then open fifos of alive workers
    for(unsigned int i = 0; i < N; i++)
    {
        bool open_ff = 1;
        if(check_for_alives == 1)
        {
            if(alive_workers[i] == 0)
            {
                open_ff = 0;
            }
        }
        else
        {
            if(alive_workers[i] == 1)
            {
                open_ff = 0;
            }
        }

        if(open_ff == 1)
        {
            jobexec_to_worker_fds[i] = open(boss_worker_ffs[i], O_WRONLY);
            if (jobexec_to_worker_fds[i] < 0)
            {
                perror("problem in opening fifo");
                exit(3);
            }

            worker_to_jobexec_fds[i] = open(worker_boss_ffs[i], O_RDONLY);
            if (worker_to_jobexec_fds[i] < 0)
            {
                perror("problem in opening fifo");
                exit(3);
            }
        }
    }
}


void respawn_workers(char ** dirs, unsigned int n_dirs, int ** dirs_indexes_per_worker, unsigned int dirs_per_worker, int * jobexec_to_worker_fds, int * worker_to_jobexec_fds, char ** boss_worker_ffs, char ** worker_boss_ffs, unsigned int N)
{//searching for workers that have died and respawning them (by forking new ones and sending them the old info)
    pid_t pid;

    sleep(1);
    for(unsigned int i = 0; i < N; i++)
    {
        if(alive_workers[i] == 0)
        {
            unlink(boss_worker_ffs[i]); //flush old fifos
            unlink(worker_boss_ffs[i]);

            if (mkfifo(boss_worker_ffs[i], 0666) == -1)
            {
                perror("problem in making fifo");
                exit(2);
            }
            if (mkfifo(worker_boss_ffs[i], 0666) == -1)
            {
                perror("problem in making fifo");
                exit(2);
            }

            if ((pid = fork()) == -1)
            {
                perror("fork failed");
                exit(1);
            }
            else if (pid == 0)
            {
                worker(i, boss_worker_ffs[i], worker_boss_ffs[i]);
                delete_str_array(dirs, n_dirs);
                delete_str_array(boss_worker_ffs, N);
                delete_str_array(worker_boss_ffs, N);

                free(workers_pids);
                free(alive_workers);

                for(unsigned int i = 0; i < N; i++)
                {
                    free(dirs_indexes_per_worker[i]);
                }
                free(dirs_indexes_per_worker);

                exit(0);
            }
            else
            {
                workers_pids[i] = pid; //updating worker's pid
            }
        }
    }

    //open newly created fifos (the ones that correspond to the dead workes)
    open_fifos(boss_worker_ffs, worker_boss_ffs, jobexec_to_worker_fds, worker_to_jobexec_fds, N, 0);

    for(unsigned int i = 0; i < N; i++)
    {//send lost dirs to respawned workers
        if(alive_workers[i] == 0)
        {
            for(unsigned int j = 0; j < dirs_per_worker + 1; j++)
            {//unsing stored dirs indexes to send the right dirs to the right workers
                if(j == dirs_per_worker)
                {
                    if(dirs_indexes_per_worker[i][j] != -1)
                    {
                        send_msg(dirs[dirs_indexes_per_worker[i][j]], jobexec_to_worker_fds[i]);
                    }
                }
                else
                {
                    send_msg(dirs[dirs_indexes_per_worker[i][j]], jobexec_to_worker_fds[i]);
                }
            }

            send_msg("-!-dirs_have_been_sent-!-", jobexec_to_worker_fds[i]);
        }
    }

    for(unsigned int i = 0; i < N; i++)
    {//waiting for confirmation that every respawned worker has finished preprocessing and is ready for taking commands
        if(alive_workers[i] == 0)
        {
            char * inc_msg = read_msg(worker_to_jobexec_fds[i]);
            free(inc_msg);
        }
    }

    some_worker_died = 0;
    reset_alive_workers(N);
}


void read_search_answers(unsigned int deadline, unsigned int N, int * worker_to_jobexec_fds)
{
    alarm(deadline);

    unsigned int success_answers = 0;
    bool none_query_found = 1;
    cout<<"--> Printing search command's results <--"<<endl;
    sep_term_line();
    for(unsigned int i = 0; i < N; i++)
    {
        bool success_answer = 0;
        int n_answers = -1;
        if(alive_workers[i] == 1)
        {
            n_answers = read_int_number(worker_to_jobexec_fds[i]);
        }

        if(n_answers > 0)
        {//one worker at least answered
            none_query_found = 0;
        }

        if((n_answers <= 0) && (timeout == 0))
        {//no answers received from worker but managed to answer on time
            success_answer = 1;
        }

        for(int j = 0; j < n_answers; j++)
        {//reading all answers from worker
            char * answer_full_file_name_path = NULL;
            if(alive_workers[i] == 1)
            {
                answer_full_file_name_path = read_msg(worker_to_jobexec_fds[i]);
            }

            int answer_line_index = -1;
            if(alive_workers[i] == 1)
            {
                answer_line_index = read_int_number(worker_to_jobexec_fds[i]);
            }

            char * answer_line = NULL;
            if(alive_workers[i] == 1)
            {
                answer_line = read_msg(worker_to_jobexec_fds[i]);
            }

            if((answer_line != NULL) && (timeout == 0) && (some_worker_died == 0))
            {//in order to print answer, worker must not have died during transmission of the answer and timeout threshold must not have been exceeded
                success_answer = 1;
                cout<<"-> text file's full path: '"<<answer_full_file_name_path<<"' - line index: '"<<answer_line_index<<"'"<<endl;
                cout<<"-> line's content: '"<<answer_line<<"'"<<endl;
                sep_term_line();
            }

            free(answer_full_file_name_path);
            free(answer_line);
        }

        success_answers += success_answer;
    }

    alarm(0);    //reseting alarm
    timeout = 0;

    if(none_query_found == 1)
    {
        cout<<"--> None of the words given was found <--"<<endl;
    }
    cout<<"--> Number of workers managed to answer on time: '"<<success_answers<<"'"<<endl;
    cout<<"--> Total number of workers (N): '"<<N<<"'"<<endl;
    cout<<"--> Deadline given (-d): '"<<deadline<<"'"<<endl;
    sep_term_line();
    cout<<endl;

}


void worker_died(int)
{//credits to https://stackoverflow.com/questions/8398298/handling-multiple-sigchld
    while(1)
    {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);   //detecting dead child by its pid
        if (pid <= 0)
        {
            break;
        }

        for(unsigned int i = 0; i < workers_pids_size; i++)
        {//searching for dead child's pid in order to set its corresponding flag to 0 (marked as dead)
            if(workers_pids[i] == pid)
            {
                alive_workers[i] = 0;
                break;
            }
        }
    }

    some_worker_died = 1;
}

void alarm_handler(int)
{
    timeout = 1;
}

