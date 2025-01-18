#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>

#define size_of_attribute(struct, attribute) sizeof(((struct *)0)->attribute)
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

// defining constants

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);

// TYPEDEFS
typedef enum
{

    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND

} MetaCommandResult;
typedef enum
{

    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_COMMAND

} PrepareResult;
typedef struct
{
    char *buffer;
    size_t buffer_length;
    int input_Length;

} InputBuffer;
typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT

} StatementType;
typedef struct
{
    int id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];

} Row;
typedef struct
{
    StatementType type;
    Row row_to_insert;
} Statement;

// FUNCTIONS
// :Related to input
InputBuffer *new_input_buffer()
{
    InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_Length = 0;
    return input_buffer;
}
void print_prompt()
{
    printf("db >");
}
void read_input(InputBuffer *input_buffer)
{

    int bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if (bytes_read <= 0)
    {
        printf("Error reading input. \n");
        exit(EXIT_FAILURE);
    }
    input_buffer->input_Length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}
// void close_input_buffer(InputBuffer *input_buffer)
// {

//     free(input_buffer->buffer);

//     free(input_buffer);
// }
// related to statements
MetaCommandResult do_meta_command(InputBuffer *Buffer)
{
    if (strcmp(Buffer->buffer, ".exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}
PrepareResult prepare_statement(InputBuffer *buffer, Statement *statement)
{

    if (strcmp(buffer->buffer, "insert") == 0)
    {
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email);
        if (args_assigned < 3)
        {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strcmp(buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_COMMAND;
}
void execute_statement(Statement *statement)

{

    switch (statement->type)
    {

    case (STATEMENT_INSERT):
        printf("This is where we will insert \n");
        break;

    case (STATEMENT_SELECT):
        printf("This is where we will Select\n");
        break;
    }
}
int main()
{

    InputBuffer *input_buffer = new_input_buffer();

    while (1)
    {

        print_prompt();
        read_input(input_buffer);
        { // Closing buffer for memory
          // if (strcmp(input_buffer->buffer, ".exit") == 0)
          // {

            //     close_input_buffer(input_buffer);
            //     exit(EXIT_SUCCESS);
            // }
            // else
            // {
            //     printf("Unrecognized command '%s'.\n", input_buffer->buffer);
            // }
        }

        if (input_buffer->buffer[0] == '.')
        {

            switch (do_meta_command(input_buffer))

            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Unrecognized command.\n");
                continue;
            }
        }

        Statement Statement;
        switch (prepare_statement(input_buffer, &Statement))
        {
        case PREPARE_SUCCESS:
            break;

        case PREPARE_UNRECOGNIZED_COMMAND:
            printf("Unrecognized Command.\n");
            continue;
        }
        execute_statement(&Statement);
        printf("Executed\n");
    }
    return 0;
}
