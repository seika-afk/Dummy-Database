// started Creatin:g in jan-2025
/*
git add .
git commit -m  "second commit"
git push -u origin master
*/
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define PAGE_SIZE 4096
#define TABLE_MAX_PAGES 100
#define ROW_SIZE 256
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)

typedef struct {
    int file_descriptor;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    Pager* pager;
    uint32_t num_rows;
} Table;

typedef struct {
    Table *table;
    uint32_t row_num;
    bool end_of_table;
} Cursor;

/* 🔹 Execute "refresh" */
void execute_refresh(Table* table) {
    table->num_rows = 0;
    printf("Database cleared.\n");
}

Cursor* table_start(Table* table) {
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->row_num = 0;
    cursor->end_of_table = (table->num_rows == 0);
    return cursor;
}

Cursor* table_end(Table* table) {
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->row_num = table->num_rows;
    cursor->end_of_table = true;
    return cursor;
}

void cursor_advance(Cursor* cursor) {
    cursor->row_num += 1;
    if (cursor->row_num >= cursor->table->num_rows) {
        cursor->end_of_table = true;
    }
}

/* 🔹 Open database file */
Pager* pager_open(const char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);
    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }
    return pager;
}

/* 🔹 Open database and load existing rows */
Table* db_open(const char* filename) {
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_length / ROW_SIZE;
    Table* table = malloc(sizeof(Table));
    table->pager = pager;
    table->num_rows = num_rows;
    return table;
}

/* 🔹 Load page into memory if needed */
void* get_page(Pager* pager, uint32_t page_num) {
    if (page_num >= TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds.\n");
        exit(EXIT_FAILURE);
    }

    if (pager->pages[page_num] == NULL) {
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE) {
            num_pages += 1;
        }

        if (page_num < num_pages) {
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
            if (bytes_read == -1) {
                printf("Error reading file.\n");
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;
    }
    return pager->pages[page_num];
}

/* 🔹 Get row slot */
void* row_slot(Table* table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = get_page(table->pager, page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    return page + (row_offset * ROW_SIZE);
}

/* 🔹 Flush pages to disk */
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
    if (pager->pages[page_num] == NULL) {
        printf("Tried to flush null page.\n");
        exit(EXIT_FAILURE);
    }

    lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);
    if (bytes_written == -1) {
        printf("Error writing file.\n");
        exit(EXIT_FAILURE);
    }
    fsync(pager->file_descriptor);
}

/* 🔹 Close database */
void db_close(Table* table) {
    Pager* pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < num_full_pages; i++) {
        if (pager->pages[i] != NULL) {
            pager_flush(pager, i, PAGE_SIZE);
            free(pager->pages[i]);
            pager->pages[i] = NULL;
        }
    }

    close(pager->file_descriptor);
    free(pager);
    free(table);
}

/* 🔹 Execute "insert" */
void execute_insert(Table* table, uint32_t id, char* name, char* email) {
    if (table->num_rows >= TABLE_MAX_PAGES * ROWS_PER_PAGE) {
        printf("Error: Table full.\n");
        return;
    }

    void* slot = row_slot(table, table->num_rows);
    sprintf(slot, "%d %s %s", id, name, email);
    table->num_rows++;

    pager_flush(table->pager, (table->num_rows - 1) / ROWS_PER_PAGE, ROW_SIZE);
    printf("Executed.\n");
}

/* 🔹 Execute "select" */
void execute_select(Table* table) {
    for (uint32_t i = 0; i < table->num_rows; i++) {
        void* slot = row_slot(table, i);
        printf("%s\n", (char*)slot);
    }
    printf("Executed.\n");
}

/* 🔹 Main */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <database_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Table* table = db_open(argv[1]);

    char input[256];
    while (1) {
        printf("db > ");
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "insert", 6) == 0) {
            uint32_t id;
            char name[32], email[64];
            sscanf(input, "insert %d %s %s", &id, name, email);
            execute_insert(table, id, name, email);
        } else if (strncmp(input, "select", 6) == 0) {
            execute_select(table);
        } else if (strncmp(input, "refresh", 7) == 0) {
            execute_refresh(table);
        } else if (strncmp(input, ".exit", 5) == 0) {
            db_close(table);
            exit(0);
        } else {
            printf("Unrecognized command.\n");
        }
    }
}

