#include "text_file.h"
#include "helping_funs.h"

text_file::text_file(char * path, char * name, my_trie * my_trie_ptr,
                     unsigned int dir_index, unsigned int text_file_index)
{
    this->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
    strcpy(this->name, name);

    char * full_file_path = make_full_file_path(path, name);
    FILE * fp = open_file(full_file_path);
    free(full_file_path);
    lines = parse_text_file(fp, n_lines, n_chars); //get text file's lines and calc stats
    fclose(fp);

    n_words = my_trie_ptr->insert_docs(lines, n_lines, dir_index, text_file_index); //insert all text file's words into trie
}


text_file::~text_file()
{
    free(name);
    for(unsigned int i = 0; i < n_lines; i++)
    {
        free(lines[i]);
    }
    free(lines);
}


void text_file::print(bool full_print_flag)
{
    cout<<"Text file's name: '"<<name<<"'"<<endl;
    cout<<"Number of lines: "<<n_lines<<endl;
    cout<<"Number of words: "<<n_words<<endl;
    cout<<"Number of characters: "<<n_chars<<endl;

    if(full_print_flag)
    {
        cout<<"--->Printing whole text file<---"<<endl;
        for(unsigned int i = 0; i < n_lines; i++)
        {
            cout<<"'"<<lines[i]<<"'"<<endl;
        }
    }
    sep_term_line();
}


char * text_file::get_line(unsigned int line_index)
{
    return lines[line_index];
}


unsigned int text_file::get_n_words() { return n_words; }

unsigned int text_file::get_n_chars() { return n_chars; }

unsigned int text_file::get_n_lines() { return n_lines; }

char * text_file::get_name() { return name; }



