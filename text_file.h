#ifndef TEXT_FILE_H
#define TEXT_FILE_H

#include "trie.h"

class text_file
{
    public:
        text_file(char * path, char * name, my_trie * my_trie_ptr,
                  unsigned int dir_index, unsigned int text_file_index);
        ~text_file();

        unsigned int get_n_lines();
        char * get_name();
        unsigned int get_n_words();
        unsigned int get_n_chars();
        char * get_line(unsigned int line_index);

        void print(bool full_print_flag);


    private:
        unsigned int n_lines;
        char ** lines;
        char * name;
        unsigned int n_words;
        unsigned int n_chars;
};

#endif // TEXT_FILE_H
