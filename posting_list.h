#ifndef POSTING_LIST_H
#define POSTING_LIST_H

#include "search_answer.h"

class dir;

class posting_list
{
    public:
        posting_list();
        ~posting_list();
        void update(char * word, unsigned int dir_index, unsigned int text_file_index, unsigned int line_index);
        void print(dir ** dirs, bool full_print);
        unsigned int get_size();
        char * get_maxcount_keyword(dir ** dirs, unsigned int & maxcount);
        char * get_mincount_keyword(dir ** dirs, unsigned int & mincount);
        char ** get_unq_txt_files_names(dir ** dirs, char ** unq_txt_files_names, unsigned int & unq_txt_files_names_size);
        search_answer ** get_search_answer(dir ** dirs, search_answer ** search_answers, unsigned int & search_answers_size);


    private:
        struct node
        {
            struct docID
            {
                unsigned int dir_index;
                unsigned int text_file_index;
            } doc_id;

            unsigned int freq;

            unsigned int * lines_indexes;
            unsigned int lines_indexes_size;

            node * next;
        };

        unsigned int pl_size;
        node * first;
        void destroy();
        char * word;
        node * create_node(unsigned int dir_index, unsigned int text_file_index, unsigned int line_index);
        bool cmp_doc_ids(posting_list::node::docID & d_id, unsigned int dir_index, unsigned int text_file_index);
        void update_node(posting_list::node * n, unsigned int line_index);

};

#endif // POSTING_LIST_H
