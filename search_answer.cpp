#include "search_answer.h"
#include "helping_funs.h"

search_answer::search_answer(unsigned int dir_index, unsigned int text_file_index, char * text_file_path, char * text_file_name)
{
    lines_size = 0;
    lines = NULL;
    lines_indexes = NULL;

    this->dir_index = dir_index;
    this->text_file_index = text_file_index;
    this->text_file_name = text_file_name;
    this->text_file_path = text_file_path;
}

search_answer::~search_answer()
{
    free(lines);
    free(lines_indexes);
}

void search_answer::print()
{
    cout<<"Text file name: '"<<text_file_name<<"'"<<endl;
    cout<<"Text file's path: '"<<text_file_path<<"'"<<endl;
    cout<<dir_index<<"_"<<text_file_index<<endl;
    cout<<"-->Printing text file's lines<--"<<endl;
    for(unsigned int i = 0; i < lines_size; i++)
    {
        cout<<"line index:'"<<lines_indexes[i]<<" - '"<<lines[i]<<"'"<<endl;
    }
    cout<<"------------------------------------------------"<<endl;
}

bool search_answer::line_exists(unsigned int line_index)
{
    for(unsigned int i = 0; i < lines_size; i++)
    {
        if(lines_indexes[i] == line_index)
        {
            return 1;
        }
    }

    return 0;
}

unsigned int search_answer::get_n_lines()
{
    return lines_size;
}

unsigned int search_answer::get_line_id(unsigned int line_index)
{
    return lines_indexes[line_index];
}

char * search_answer::get_line(unsigned int line_index)
{
    return lines[line_index];
}


char * search_answer::get_full_file_name_path()
{
    return make_full_file_path(text_file_path, text_file_name);
}


bool search_answer::cmp_doc_id(unsigned int dir_index, unsigned text_file_index)
{
    if((dir_index == this->dir_index) && (text_file_index == this->text_file_index))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void search_answer::add_line(unsigned int line_index, char * line)
{
    lines_size++;
    lines_indexes = (unsigned int*) realloc(lines_indexes, lines_size * sizeof(unsigned int));
    lines = (char **) realloc(lines, lines_size * sizeof(char *));

    lines_indexes[lines_size - 1] = line_index;
    lines[lines_size - 1] = line;
}

void search_answer::update(unsigned int line_index, char * line)
{
    if(line_exists(line_index) == 0)
    {
        add_line(line_index, line);
    }
}


search_answer ** update_search_answers(search_answer ** sa_array, unsigned int & sa_array_size,
        unsigned int dir_index, unsigned int text_file_index, unsigned int line_index, char * line,
        char * text_file_path, char * text_file_name)
{
    for(unsigned int i = 0; i < sa_array_size; i++)
    {
        if(sa_array[i]->cmp_doc_id(dir_index, text_file_index) == 1)
        {
            sa_array[i]->update(line_index, line);
            return sa_array;
        }
    }

    sa_array_size++;
    sa_array = (search_answer **) realloc(sa_array, sa_array_size * sizeof(search_answer *));
    sa_array[sa_array_size - 1] = new search_answer(dir_index, text_file_index, text_file_path, text_file_name);
    sa_array[sa_array_size - 1]->update(line_index, line);

    return sa_array;
}


void print_search_answers(search_answer ** sa_array, unsigned int sa_array_size)
{
    for(unsigned int i = 0; i < sa_array_size; i++)
    {
        sa_array[i]->print();
    }
}


void delete_search_answers(search_answer ** sa_array, unsigned int sa_array_size)
{
    for(unsigned int i = 0; i < sa_array_size; i++)
    {
        delete(sa_array[i]);
    }
    free(sa_array);
}


unsigned int get_total_n_lines(search_answer ** sa_array, unsigned int sa_array_size)
{
    unsigned int total_n_lines = 0;

    for(unsigned int i = 0; i < sa_array_size; i++)
    {
        total_n_lines += sa_array[i]->get_n_lines();
    }

    return total_n_lines;
}



