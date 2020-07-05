#include "helping_funs.h"


unsigned int get_number_of_digits(int num)
{
    if (num == 0)
    {
        return 1;
    }
    else
    {
        return floor(log10(abs(num))) + 1;   //https://stackoverflow.com/questions/3068397/finding-the-length-of-an-integer-in-c
    }
}


FILE * open_file(char * fn)
{
    FILE* random_file;
    random_file = fopen(fn, "r");

    if (!random_file)
    {
        fprintf(stderr, "-! ERROR - docfile does not exist !-\n");
        return NULL;
    }

    return random_file;
}


unsigned int get_number_of_lines(FILE * fp)
{
    unsigned int lines = 0;
    int ch ;

    while(!feof(fp))
    {
      ch = fgetc(fp);
      if(ch == '\n')
      {
        lines++;
      }
    }

    rewind(fp);
    return lines ;
}


char * read_str(unsigned int str_len)
{ //reads and returns string of unknown length
    char * str = (char *) malloc((str_len) * sizeof(char));
    int ch;
    unsigned int i = 0;

    while(((ch = getchar()) != '\n') && (ch != EOF))
    {
        str[i] = ch;
        i++;
        if(i == str_len)
        {
            str_len++;
            str = (char *) realloc(str, str_len * sizeof(char));
        }
    }

    str[i] = '\0';  // add \0 at the end of the string
    return str;
}


char ** strip_line_into_words(char * cmd, unsigned int & cmd_nwords)
{//returns a char * array which contains each cmd's word
    char ** cmd_words = NULL;
    char * word = strtok (cmd, " \t");

    unsigned int word_counter = 0;

    while (word != NULL)
    { //for each cmd's word
        cmd_words = (char **) realloc(cmd_words, (word_counter + 1) * sizeof(char *));
        cmd_words[word_counter] = (char *) malloc((strlen(word) + 1) * sizeof(char));  // plus one for \0
        strcpy(cmd_words[word_counter], word);
        word_counter++;
        word = strtok (NULL, " \t");
    }

    cmd_nwords = word_counter;
    return cmd_words;
}


void delete_str_array(char ** str_array, unsigned int array_size)
{
    for(unsigned int i = 0; i < array_size; i++)
    {
        free(str_array[i]);
    }

    free(str_array);
}


bool parse_docs(char **docs, FILE * fp)
{// storing docfile's lines into docs array - checking docfile for errors
    char * line = NULL;
    size_t len = 0;
    ssize_t nchars;
    int line_index = 0;

    while ((nchars = getline(&line, &len, fp)) != -1)
    {
        if ( line[nchars - 1] == '\n')
        { // removing newline character from string
            line[nchars - 1] = '\0';
            nchars--;
        }

        DIR * dir = opendir(line);
        if (dir)
        {
            /* Directory exists. */
            closedir(dir);
        }
        else if (ENOENT == errno)
        {
            /* Directory does not exist. */
            cout<<"-! ERROR !- Directory with path:'"<<line<<"' does not exist."<<endl;
            free(line);
            return 1;
        }
        else
        {
            /* opendir() failed for some other reason. */
            cout<<"-! ERROR !- Directory with path:'"<<line<<"' cannot be opened."<<endl;
            free(line);
            return 1;
        }

        docs[line_index] = (char *) malloc((strlen(line) + 1) * sizeof(char));  //plus one for \0
        strcpy(docs[line_index], line);  //storing docfile's line into doc array
        line_index++;
    }

    free(line);
    return 0;
}


char ** parse_text_file(FILE * fp, unsigned int & n_lines, unsigned int & n_chars)
{// storing docfile's lines into docs array
    char * line = NULL;
    size_t len = 0;
    ssize_t nchars;
    n_lines = 0;
    n_chars = 0;

    char ** lines = NULL;

    while ((nchars = getline(&line, &len, fp)) != -1)
    {
        if ( line[nchars - 1] == '\n')
        { // removing newline character from string
            line[nchars - 1] = '\0';
            nchars--;
        }

        n_chars += nchars + 1;

        lines = (char **) realloc(lines, (n_lines + 1) * sizeof(char *));
        lines[n_lines] = (char *) malloc((strlen(line) + 1) * sizeof(char));  //plus one for \0
        strcpy(lines[n_lines], line);  //storing docfile's line into doc array
        n_lines++;
    }

    free(line);

    if (n_chars > 0)
    {
        n_chars--;
    }

    return lines;
}


int scan_dir_for_files(char * in_dir)
{//checking that text files at every dir exist
    DIR * FD;
    struct dirent* in_file;

    /* Scanning directory */
    if (NULL == (FD = opendir(in_dir)))
    {
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
        return 1;
    }

    while ((in_file = readdir(FD)))
    {
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))
            continue;

        FILE * in_dir_file = open_file(in_file->d_name);
        if (in_dir_file == NULL)
        {
            fprintf(stderr, "Error : Failed to open directory file - %s\n", strerror(errno));
            return 1;
        }

        fclose(in_dir_file);
    }

    return 0;
}


bool is_number(const char *str)
{
    unsigned int dot_counter = 0;
    for(unsigned int i=0; i < strlen(str); i++)
    { //iterating the whole string character by character

        if(str[i] == '.')
        {
            dot_counter++;
            if((i == 0) || (i == strlen(str) - 1))
            {
                return 0;
            }
            else
            {
                if(dot_counter == 2)
                {
                    return 0;
                }
            }
        }
        else if(!isdigit(str[i]))
        {
            return 0;
        }
    }

    return 1;
}


bool is_unsigned_int_number(const char *str)
{
    for(unsigned int i=0; i < strlen(str); i++)
    { //iterating the whole string character by character
        if(!isdigit(str[i]))
        {// and checking if every character is a number
            return 0;
        }
    }

    return 1;
}


char * make_full_file_path(char * path, char * text_file_name)
{
    char * full_file_path = (char *)malloc((strlen(text_file_name) + strlen(path) + 1 + 1) * sizeof(char));
    strcpy(full_file_path, path);
    strcat(full_file_path, "/");
    strcat(full_file_path, text_file_name);
    return full_file_path;
}


int send_msg(const char * msg, unsigned int fd)
{//send a string of uknown legth through fifo
    int buf_size = strlen(msg) + 1;
    if (write(fd, &buf_size, sizeof(int)) < 0)
    {
        perror("problem in writing fifo");
        exit(4);
    }

    if (write(fd, msg, buf_size) < 0)
    {
        perror("problem in writing fifo");
        exit(4);
    }

    return 0;
}


char * read_msg(unsigned int fd)
{//read a string of uknown legth from fifo
    int buf_size;
    if (read(fd, &buf_size, sizeof(int)) < 0)
    {
        perror("problem in reading fifo");
        exit(5);
    }

    char * inc_msg = (char *)malloc(buf_size);
    if (read(fd, inc_msg, buf_size) < 0)
    {
        perror("problem in reading fifo");
        exit(5);
    }

    return inc_msg;
}


int send_int_number(unsigned int fd, int number)
{
    if (write(fd, &number, sizeof(int)) < 0)
    {
        perror("problem in writing fifo");
        exit(4);
    }

    return 0;
}


int read_int_number(unsigned int fd)
{
    int number;
    if (read(fd, &number, sizeof(int)) < 0)
    {
        perror("problem in reading fifo");
        exit(5);
    }

    return number;
}


char * check_program_args(char ** argv, int argc, unsigned int & N)
{
    if (argc != 5)
    {
        cout<<"-! ERROR - Wrong number of arguments is given !-"<<endl;
        exit(1);
    }

    char * docfile = NULL;
    bool N_found = 0;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-d") == 0)
        {// if -d parameter found the next argument must be docfile's name
            if((i + 1) == argc)
            {
                cout<<"-! ERROR - Docfile's name not found !-"<<endl;
                exit(1);
            }
            else
            {
                docfile = argv[i + 1];
                i++;
            }
        }
        else if(strcmp(argv[i], "-w") == 0)
        {
            if((i + 1) == argc)
            {
                cout<<"-! ERROR - w's value not found !-"<<endl;
                exit(1);
            }
            else
            {
                if(!is_unsigned_int_number(argv[i + 1]))
                {
                    cout<<"-> ERROR - w's value is not an unsigned int number !-"<<endl;
                    exit(1);
                }
                else
                {
                    N = atoi(argv[i + 1]);
                    N_found = 1;
                    i++;
                }
            }
        }
        else
        {
            cout<<"-! ERROR - invalid parameters given !-"<<endl;
            exit(1);
        }
    }

    if(docfile == NULL)
    {
        cout<<"-! ERROR - Docfile's -d parameter not found !-"<<endl;
        exit(1);
    }

    if(N_found == 0)
    {
        cout<<"-! ERROR - numWorkers' -w parameter not found !-"<<endl;
        exit(1);
    }
    else
    {
        if (N < 1)
        {
            cout<<"-> ERROR - numWorkers' value is 0 !-"<<endl;
            exit(1);
        }
    }

    return docfile;
}


char * get_timestamp()
{
    time_t now = time(NULL);
    char* dt = ctime(&now);
    dt[strlen(dt) - 1] = '\0';
    return dt;
}


void sep_term_line()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    unsigned int width = w.ws_col;
    for(unsigned int i = 0; i < width; i++)
    {
        cout<<"-";
    }
    cout<<endl;
}



