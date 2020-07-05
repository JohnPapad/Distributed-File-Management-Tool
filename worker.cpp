
#include "helping_funs.h"
#include "worker.h"

void worker(unsigned int i, char * boss_worker_ffs, char * worker_boss_ffs)
{
    int jobexec_to_worker = open(boss_worker_ffs, O_RDONLY);
    if(jobexec_to_worker < 0)
    {
        perror("problem in opening fifo");
        exit(3);
    }

    int worker_to_jobexec = open(worker_boss_ffs, O_WRONLY);
    if(worker_to_jobexec < 0)
    {
        perror("problem in opening fifo");
        exit(3);
    }

    int matched_strings = -1;
    my_trie * my_trie_ptr = new my_trie();
    dir ** dirs = NULL; //array of maps (one for each dir)
    unsigned int n_dirs = 0;

    while(1)
    {
        char * inc_msg = read_msg(jobexec_to_worker);  //reading dirs' paths

        if (strcmp("-!-dirs_have_been_sent-!-", inc_msg) == 0)
        {//all dirs have been sent. proceed
            free(inc_msg);
            break;
        }

        //creating all the necessary data structures
        dirs = (dir **) realloc(dirs, (n_dirs + 1) * sizeof(dir *));
        dirs[n_dirs] = new dir(inc_msg, my_trie_ptr, n_dirs);
        n_dirs++;

        free(inc_msg);
    }

    int pid = getpid();
    char pid_str[get_number_of_digits(pid) + 1];
    sprintf(pid_str, "%d", pid);

    unsigned int log_file_size = strlen(pid_str) + sizeof("log/Worker_") + 1;
    char log_file[log_file_size];
    strcpy(log_file, "log/Worker_");
    strcat(log_file, pid_str);    //creating worker's log file name

    send_msg("finished_preprocessing", worker_to_jobexec);  //inform job executor that the worker is ready for receiving commands

    while(1)
    {
        char * cmd = read_msg(jobexec_to_worker);  //reading user's command
        char * timestamp = get_timestamp();

        if (strcmp("/exit", cmd) == 0)
        {
            send_int_number(worker_to_jobexec, matched_strings);
            free(cmd);

            for(unsigned int i = 0; i < n_dirs; i++)
            {
                delete(dirs[i]);
            }
            free(dirs);

            delete(my_trie_ptr);

            char * inc_msg = read_msg(jobexec_to_worker);
            free(inc_msg);

            close(jobexec_to_worker);
            close(worker_to_jobexec);
            return;
        }
        else
        {
            unsigned int cmd_nwords;
            char ** cmd_words = strip_line_into_words(cmd, cmd_nwords); //get arguments from user's command

            if (strcmp("/wc", cmd_words[0]) == 0)
            {
                //calculating stats of every text file in worker's assigned dirs
                int total_lines = get_total_lines(dirs, n_dirs);
                int total_words = get_total_words(dirs, n_dirs);
                int total_chars = get_total_chars(dirs, n_dirs);

                write_wc_in_log_file(log_file, timestamp, total_lines, total_words, total_chars);

                //sending stats to job executor
                send_int_number(worker_to_jobexec, total_lines);
                send_int_number(worker_to_jobexec, total_words);
                send_int_number(worker_to_jobexec, total_chars);
            }
            else if (strcmp("/maxcount", cmd_words[0]) == 0)
            {
                unsigned int maxcount = 0;
                char * max_txt_file_name = NULL;
                posting_list * pl_str = my_trie_ptr->search_word(cmd_words[1]); //searching for keyword in trie
                if(pl_str != NULL)
                {//keyword found
                    max_txt_file_name = pl_str->get_maxcount_keyword(dirs, maxcount);
                    write_maxcount_in_log_file(log_file, timestamp, cmd_words[1], max_txt_file_name, maxcount);
                }
                else
                {//keyword not found
                    max_txt_file_name = (char *)malloc((strlen("NULL") + 1) * sizeof(char));
                    strcpy(max_txt_file_name, "NULL");
                    write_maxcount_in_log_file(log_file, timestamp, cmd_words[1], "[KEYWORD_NOT_FOUND]", 0);
                }

                send_msg(max_txt_file_name, worker_to_jobexec);  //send answer back to job exec
                free(max_txt_file_name);
                send_int_number(worker_to_jobexec, maxcount);
            }
            else if (strcmp("/mincount", cmd_words[0]) == 0)
            {
                unsigned int mincount = 0;
                char * min_txt_file_name = NULL;
                posting_list * pl_str = my_trie_ptr->search_word(cmd_words[1]);
                if(pl_str != NULL)
                {
                    min_txt_file_name = pl_str->get_mincount_keyword(dirs, mincount);
                    write_mincount_in_log_file(log_file, timestamp, cmd_words[1], min_txt_file_name, mincount);
                }
                else
                {
                    min_txt_file_name = (char *)malloc((strlen("NULL") + 1) * sizeof(char));
                    strcpy(min_txt_file_name, "NULL");
                    write_mincount_in_log_file(log_file, timestamp, cmd_words[1], "[KEYWORD_NOT_FOUND]", 0);
                }

                send_msg(min_txt_file_name, worker_to_jobexec);
                free(min_txt_file_name);
                send_int_number(worker_to_jobexec, mincount);
            }
            else if (strcmp("/search", cmd_words[0]) == 0)
            {
                if (matched_strings == -1)
                {
                    matched_strings = 0;
                }

                int n_queries = read_int_number(jobexec_to_worker);
                char ** queries = get_queries(n_queries, jobexec_to_worker);

                search_answer ** sa_array = NULL;
                unsigned int sa_array_size = 0;

                for(int i = 0; i < n_queries; i++)
                {//for each word query
                    posting_list * pl_ptr = my_trie_ptr->search_word(queries[i]); //search trie
                    if(pl_ptr != NULL)
                    {//word found
                        matched_strings++;
                        sa_array = pl_ptr->get_search_answer(dirs, sa_array, sa_array_size); //gather answer lines
                        char ** unq_txt_files_names = NULL;
                        unsigned int unq_txt_files_names_size = 0;
                        //gathering the text files' names in which the query is contained (without keeping duplicates)
                        unq_txt_files_names = pl_ptr->get_unq_txt_files_names(dirs, unq_txt_files_names, unq_txt_files_names_size);
                        write_search_query_in_log_file(log_file, timestamp, queries[i], unq_txt_files_names, unq_txt_files_names_size);
                        delete_str_array(unq_txt_files_names, unq_txt_files_names_size);
                    }
                    else
                    {//word not found
                        write_search_query_in_log_file(log_file, timestamp, queries[i], NULL, 0);
                    }
                }

                int n_answers = get_total_n_lines(sa_array, sa_array_size);
                send_int_number(worker_to_jobexec, n_answers);

                //send answers to job exec
                for(unsigned int i = 0; i < sa_array_size; i++)
                {
                    char * full_file_name_path = sa_array[i]->get_full_file_name_path();

                    for(unsigned int j = 0; j < sa_array[i]->get_n_lines(); j++)
                    {
                        send_msg(full_file_name_path, worker_to_jobexec);
                        int line_index = sa_array[i]->get_line_id(j);
                        send_int_number(worker_to_jobexec, line_index);
                        send_msg(sa_array[i]->get_line(j), worker_to_jobexec);
                    }

                    free(full_file_name_path);
                }

                delete_search_answers(sa_array, sa_array_size);
                delete_str_array(queries, n_queries);
            }

            delete_str_array(cmd_words, cmd_nwords);
            free(cmd);
        }
    }
}


unsigned int get_total_words(dir ** dirs, unsigned n_dirs)
{
    unsigned int total_words = 0;

    for(unsigned int i = 0; i < n_dirs; i++)
    {
        total_words += dirs[i]->get_text_files_n_words();
    }

    return total_words;
}


unsigned int get_total_lines(dir ** dirs, unsigned n_dirs)
{
    unsigned int total_lines = 0;

    for(unsigned int i = 0; i < n_dirs; i++)
    {
        total_lines += dirs[i]->get_text_files_n_lines();
    }

    return total_lines;
}


unsigned int get_total_chars(dir ** dirs, unsigned n_dirs)
{
    unsigned int total_chars = 0;

    for(unsigned int i = 0; i < n_dirs; i++)
    {
        total_chars += dirs[i]->get_text_files_n_chars();
    }

    return total_chars;
}


void print_dirs(dir ** dirs, unsigned int n_dirs)
{
    for(unsigned int i = 0; i < n_dirs; i++)
    {
        dirs[i]->print(1);
    }
}


char ** get_queries(int queries_size, unsigned int fd)
{
    char ** queries = (char **)malloc(queries_size * sizeof(char *));

    for(int i = 0; i < queries_size; i++)
    {
        char * query = read_msg(fd);
        queries[i] = query;
    }

    return queries;
}


void write_wc_in_log_file(char * log_file, char * timestamp, unsigned int n_lines, unsigned int n_words, unsigned int n_chars)
{
    ofstream outfile (log_file, ios_base::app | ios_base::out);
    outfile<<timestamp<<" : wc : "<<n_chars<<" : "<<n_words<<" : "<<n_lines<<endl;
}

void write_maxcount_in_log_file(char * log_file, char * timestamp, char * keyword, const char * max_txt_file_name, unsigned int maxcount)
{
    ofstream outfile (log_file, ios_base::app | ios_base::out);
    outfile<<timestamp<<" : maxcount : '"<<keyword<<"' : '"<<max_txt_file_name<<"' ("<<maxcount<<")"<<endl;
}

void write_mincount_in_log_file(char * log_file, char * timestamp, char * keyword, const char * min_txt_file_name, unsigned int mincount)
{
    ofstream outfile (log_file, ios_base::app | ios_base::out);
    outfile<<timestamp<<" : mincount : '"<<keyword<<"' : '"<<min_txt_file_name<<"' ("<<mincount<<")"<<endl;
}


void write_search_query_in_log_file(char * log_file, char * timestamp, char * query, char ** unq_txt_files_names, unsigned int unq_txt_files_names_size)
{
    ofstream outfile (log_file, ios_base::app | ios_base::out);

    outfile<<timestamp<<" : search : '"<<query<<"' : ";
    if(unq_txt_files_names_size == 0)
    {
        outfile<<"[WORD_NOT_FOUND]"<<endl;
        return;
    }

    outfile<<"'";
    for(unsigned int i = 0; i < unq_txt_files_names_size; i++)
    {
        outfile<<unq_txt_files_names[i];
        if(i < unq_txt_files_names_size - 1)
        {
            outfile<<"' : '";
        }
        else
        {
            outfile<<"'"<<endl;
        }
    }
}


