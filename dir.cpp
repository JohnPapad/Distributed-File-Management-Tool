#include "dir.h"
#include "helping_funs.h"

dir::dir(char * dir_path, my_trie * my_trie_ptr, unsigned int dir_index)
{
    this->path = (char *)malloc((strlen(dir_path) + 1) * sizeof(char));
    strcpy(this->path, dir_path);

    DIR * fd = opendir(this->path);
    struct dirent * in_dir;
    n_files = 0;
    text_files = NULL;

    while ((in_dir = readdir(fd)))
    {//find every text file contained in the dir
        if (!strcmp (in_dir->d_name, "."))
            continue;
        if (!strcmp (in_dir->d_name, ".."))
            continue;

        text_files = (text_file **) realloc(text_files, (n_files + 1) * sizeof(text_file *));
        text_files[n_files] = new text_file(this->path, in_dir->d_name, my_trie_ptr, dir_index, n_files);
        n_files++;
    }

    closedir(fd);
}


dir::~dir()
{
    free(path);
    for(unsigned int i = 0; i < n_files; i++)
    {
        delete(text_files[i]);
    }
    free(text_files);
}


void dir::print(bool full_print_flag)
{
    cout<<"===>Printing all directories' info<==="<<endl;
    cout<<"Directory path: '"<<path<<"'"<<endl;
    cout<<"Directory contains: "<<n_files<<" text files."<<endl;

    if(full_print_flag)
    {
        cout<<"--->Printing directory's text files<---"<<endl;
        for(unsigned int i = 0; i < n_files; i++)
        {
            text_files[i]->print(0);
        }
    }
    cout<<"======================================="<<endl<<endl;
}


char * dir::get_text_file_line(unsigned int text_file_index, unsigned int line_index)
{
    return text_files[text_file_index]->get_line(line_index);
}


char * dir::get_text_file_name(unsigned int text_file_index)
{
    return text_files[text_file_index]->get_name();
}


unsigned int dir::get_text_files_n_words()
{
    unsigned int total_words = 0;

    for(unsigned int i = 0; i < n_files; i++)
    {
        total_words += text_files[i]->get_n_words();
    }

    return total_words;
}


unsigned int dir::get_text_files_n_lines()
{
    unsigned int total_lines = 0;

    for(unsigned int i = 0; i < n_files; i++)
    {
        total_lines += text_files[i]->get_n_lines();
    }

    return total_lines;
}


unsigned int dir::get_text_files_n_chars()
{
    unsigned int total_chars = 0;

    for(unsigned int i = 0; i < n_files; i++)
    {
        total_chars += text_files[i]->get_n_chars();
    }

    return total_chars;
}

char * dir::get_path() { return path; }

