#include "posting_list.h"
#include "helping_funs.h"
#include "dir.h"

posting_list::posting_list()
{
    first = NULL;
    word = NULL;
    pl_size = 0;
}


void posting_list::print(dir ** dirs, bool full_print)
{

    cout<<"--->Printing posting list of word:'"<<word<<"'"<<endl;
    cout<<"The word is contained in '"<<pl_size<<"'"<<" different text files"<<endl;

    if (full_print == 0)
    {
        return;
    }

    node * temp = first;
    if (first == NULL)
    {
        cout<<"Posting list is empty"<<endl;
        return;
    }

    while(temp != NULL) //print whole posting list
    {
        char * dir_path = dirs[temp->doc_id.dir_index]->get_path();
        char * text_file_name = dirs[temp->doc_id.dir_index]->get_text_file_name(temp->doc_id.text_file_index);
        cout<<"[Dir_path, Text_file_name] - freq: "<<"["<<dir_path<<", "<<text_file_name<<"]"<<" - "<<temp->freq<<endl;
        cout<<"The word appears in '"<<temp->lines_indexes_size<<"' different lines in this text file"<<endl;
        cout<<"->Printing the lines' indexes that the word appears"<<endl;
        cout<<"[";
        for(unsigned int i = 0; i < temp->lines_indexes_size; i++)
        {
            if ( i != temp->lines_indexes_size - 1)
            {
                cout<<temp->lines_indexes[i]<<", ";
            }
            else
            {
                cout<<temp->lines_indexes[i]<<"]";
            }
        }

        cout<<endl<<"->Printing whole lines<-"<<endl;
        for(unsigned int i = 0; i < temp->lines_indexes_size; i++)
        {
            cout<<dirs[temp->doc_id.dir_index]->get_text_file_line(temp->doc_id.text_file_index, temp->lines_indexes[i])<<endl;
        }

        sep_term_line();
        temp = temp->next;
    }

    cout<<endl;
}


unsigned int posting_list::get_size()
{
    return pl_size;
}


posting_list::~posting_list()
{
    node * temp = first;
    node * temp2;

    while(temp != NULL) //iterate list
    {
        temp2 = temp->next;
        free(temp->lines_indexes);
        delete temp;
        temp = temp2;
        pl_size--;
    }

    free(word);
}


posting_list::node * posting_list::create_node(unsigned int dir_index, unsigned int text_file_index, unsigned int line_index)
{
    node * new_node = new node;

    new_node->doc_id.dir_index = dir_index;
    new_node->doc_id.text_file_index = text_file_index;

    new_node->freq = 1;

    new_node->lines_indexes = (unsigned int *)malloc(sizeof(unsigned int));
    new_node->lines_indexes[0] = line_index;
    new_node->lines_indexes_size = 1;

    new_node->next = NULL ;
    return new_node;
}


bool posting_list::cmp_doc_ids(posting_list::node::docID & d_id, unsigned int dir_index, unsigned int text_file_index)
{//both dir index and text file index must be checked
    if( (d_id.dir_index == dir_index) && (d_id.text_file_index == text_file_index) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void posting_list::update_node(posting_list::node * n, unsigned int line_index)
{
    n->freq++;

    if( n->lines_indexes[n->lines_indexes_size - 1] != line_index )
    {
        n->lines_indexes = (unsigned int *)realloc(n->lines_indexes, (n->lines_indexes_size + 1) * sizeof(unsigned int));
        n->lines_indexes[n->lines_indexes_size] = line_index;
        n->lines_indexes_size++;
    }
}


void posting_list::update(char * word, unsigned int dir_index, unsigned int text_file_index, unsigned int line_index)
{ //create new list node if doc id (dir index and text file index) is not contained in the list or update its lines
    node * temp = first;
    node * prev_temp;
    node * n;

    if(first == NULL)
    { // list is empty
        n = create_node(dir_index, text_file_index, line_index);
        first = n;
        this->word = (char *) malloc((strlen(word) + 1) * sizeof(char));
        strcpy(this->word, word);
        pl_size++;
    }
    else
    { // list already exists
        while(1)
        {
            if(temp == NULL)
            {// we have iterated the whole list and doc id does not exist. so it will be added at the end of the list
                n = create_node(dir_index, text_file_index, line_index);
                pl_size++;
                prev_temp->next = n;
                break;
            }

            if (cmp_doc_ids(temp->doc_id, dir_index, text_file_index))
            {//found it
                update_node(temp, line_index);
                break;
            }
            else
            {//keep searching
                prev_temp = temp;
                temp = temp->next;
            }
        }
    }
}

char * posting_list::get_maxcount_keyword(dir ** dirs, unsigned int & maxcount)
{
    node * temp = first;
    unsigned int dir_index;
    unsigned int text_file_index;
    maxcount = 0;

    while(temp != NULL) //iterating posting list
    {
        if(temp->freq > maxcount)
        {//new temp maxcount
            maxcount = temp->freq;
            dir_index = temp->doc_id.dir_index;
            text_file_index = temp->doc_id.text_file_index;
        }
        else if(temp->freq == maxcount)
        {//same appearences must check text file names alphabetically
            char * prev_full_file_path = make_full_file_path(dirs[dir_index]->get_path(), dirs[dir_index]->get_text_file_name(text_file_index));

            char * curr_dir_path = dirs[temp->doc_id.dir_index]->get_path();
            char * curr_text_file_name = dirs[temp->doc_id.dir_index]->get_text_file_name(temp->doc_id.text_file_index);
            char * curr_full_file_path = make_full_file_path(curr_dir_path, curr_text_file_name);

            if(strcmp(prev_full_file_path, curr_full_file_path) > 0)
            {
                dir_index = temp->doc_id.dir_index;
                text_file_index = temp->doc_id.text_file_index;
            }

            free(prev_full_file_path);
            free(curr_full_file_path);
        }

        temp = temp->next;
    }

    char * dir_path = dirs[dir_index]->get_path();
    char * text_file_name = dirs[dir_index]->get_text_file_name(text_file_index);
    char * full_file_path = make_full_file_path(dir_path, text_file_name);
    return full_file_path;

}


char * posting_list::get_mincount_keyword(dir ** dirs, unsigned int & mincount)
{//indetical as above
    node * temp = first;
    unsigned int dir_index;
    unsigned int text_file_index;
    mincount = numeric_limits<unsigned int>::max();

    while(temp != NULL) //iterating posting list
    {
        if(temp->freq < mincount)
        {
            mincount = temp->freq;
            dir_index = temp->doc_id.dir_index;
            text_file_index = temp->doc_id.text_file_index;
        }
        else if(temp->freq == mincount)
        {
            char * prev_full_file_path = make_full_file_path(dirs[dir_index]->get_path(), dirs[dir_index]->get_text_file_name(text_file_index));

            char * curr_dir_path = dirs[temp->doc_id.dir_index]->get_path();
            char * curr_text_file_name = dirs[temp->doc_id.dir_index]->get_text_file_name(temp->doc_id.text_file_index);
            char * curr_full_file_path = make_full_file_path(curr_dir_path, curr_text_file_name);

            if(strcmp(prev_full_file_path, curr_full_file_path) > 0)
            {
                dir_index = temp->doc_id.dir_index;
                text_file_index = temp->doc_id.text_file_index;
            }

            free(prev_full_file_path);
            free(curr_full_file_path);
        }

        temp = temp->next;
    }

    char * dir_path = dirs[dir_index]->get_path();
    char * text_file_name = dirs[dir_index]->get_text_file_name(text_file_index);
    char * full_file_path = make_full_file_path(dir_path, text_file_name);
    return full_file_path;

}


char ** posting_list::get_unq_txt_files_names(dir ** dirs, char ** unq_txt_files_names, unsigned int & unq_txt_files_names_size)
{// adding posting list's text files names to an array. if a posting list's text file is already contained in the array do not add it again
    node * temp = first;
    while(temp != NULL) //iterating posting list
    {
        for(unsigned int i = 0; i < temp->lines_indexes_size; i++)
        {
            char * txt_file_name = make_full_file_path(dirs[temp->doc_id.dir_index]->get_path(), dirs[temp->doc_id.dir_index]->get_text_file_name(temp->doc_id.text_file_index));

            bool txt_file_exists = 0;
            for(unsigned int j = 0; j < unq_txt_files_names_size; j++)
            { // checking if current text file name is already contained in the array
                if (strcmp(unq_txt_files_names[j], txt_file_name) == 0)
                {
                    txt_file_exists = 1;  // already exists
                    break;
                }
            }

            if(txt_file_exists == 0)
            { //if current text file name is not already contained in the array add it now
                unq_txt_files_names_size++;
                unq_txt_files_names = (char **) realloc(unq_txt_files_names, unq_txt_files_names_size * sizeof(char *));
                unq_txt_files_names[unq_txt_files_names_size - 1] = txt_file_name;
            }
            else
            {
                free(txt_file_name);
            }
        }

        temp = temp->next;
    }

    return unq_txt_files_names;
}


search_answer ** posting_list::get_search_answer(dir ** dirs, search_answer ** search_answers, unsigned int & search_answers_size)
{// adding posting list's text file's lines to an array. if already contained in the array do not add it again
    node * temp = first;
    while(temp != NULL) //iterating posting list
    {
        for(unsigned int i = 0; i < temp->lines_indexes_size; i++)
        {
            search_answers = update_search_answers(search_answers, search_answers_size,
                             temp->doc_id.dir_index, temp->doc_id.text_file_index, temp->lines_indexes[i],
                             dirs[temp->doc_id.dir_index]->get_text_file_line(temp->doc_id.text_file_index, temp->lines_indexes[i]),
                             dirs[temp->doc_id.dir_index]->get_path(),
                             dirs[temp->doc_id.dir_index]->get_text_file_name(temp->doc_id.text_file_index));
        }

        temp = temp->next;
    }

    return search_answers;
}
