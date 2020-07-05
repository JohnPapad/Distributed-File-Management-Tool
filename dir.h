#ifndef DIR_H
#define DIR_H

#include "text_file.h"

class dir
{
    public:
        dir(char * dir_path, my_trie * my_trie_ptr, unsigned int dir_index);
        ~dir();

        char * get_path();
        char * get_text_file_line(unsigned int text_file_index, unsigned int line_index);
        char * get_text_file_name(unsigned int text_file_index);
        unsigned int get_text_files_n_words();
        unsigned int get_text_files_n_lines();
        unsigned int get_text_files_n_chars();

        void print(bool full_print_flag);


    private:
        unsigned int n_files;
        text_file ** text_files;
        char * path;
};

#endif // DIR_H
