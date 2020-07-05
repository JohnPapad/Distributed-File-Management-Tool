#ifndef SEARCH_ANSWER_H
#define SEARCH_ANSWER_H


class search_answer
{
    public:
        search_answer(unsigned int dir_index, unsigned int text_file_index, char * text_file_path, char * text_file_name);
        ~search_answer();

        void print();
        bool cmp_doc_id(unsigned int dir_index, unsigned text_file_index);
        void update(unsigned int line_index, char * line);
        unsigned int get_n_lines();
        char * get_full_file_name_path();
        unsigned int get_line_id(unsigned int line_index);
        char * get_line(unsigned int line_index);


    private:
        unsigned int dir_index;
        unsigned int text_file_index;
        char * text_file_path;
        char * text_file_name;

        unsigned int * lines_indexes;
        char ** lines;
        unsigned int lines_size;

        bool line_exists(unsigned int line_index);
        void add_line(unsigned int line_index, char * line);

};


search_answer ** update_search_answers(search_answer ** sa_array, unsigned int & sa_array_size,
        unsigned int dir_index, unsigned int text_file_index, unsigned int line_index, char * line,
        char * text_file_path, char * text_file_name);

void print_search_answers(search_answer ** sa_array, unsigned int sa_array_size);
void delete_search_answers(search_answer ** sa_array, unsigned int sa_array_size);
unsigned int get_total_n_lines(search_answer ** sa_array, unsigned int sa_array_size);




#endif // SEARCH_ANSWER_H
