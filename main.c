#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Code yoinked from https://github.com/tsoding/musializer/blob/master/src/nob.h
typedef struct
{
    char *data;
    size_t count;
    size_t capacity;
} String_Builder;

// Code yoinked from https://github.com/tsoding/musializer/blob/master/src/nob.h
typedef struct
{
    size_t count;
    char *data;
} String_View;

typedef struct
{
    String_View key;
    size_t value;
} Hash_Table_Entry;

typedef struct
{
    Hash_Table_Entry *entries;
    size_t count;
    size_t capacity;
} Hash_Table;

// Code yoinked from https://github.com/tsoding/musializer/blob/master/src/nob.h
bool read_file(const char *file_path, String_Builder *builder)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        perror("fopen");
        return false;
    }

    if (fseek(file, 0, SEEK_END) < 0)
    {
        perror("fseek");
        fclose(file);
        return false;
    }

    long file_size = ftell(file);
    if (file_size < 0)
    {
        perror("ftell");
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) < 0)
    {
        perror("fseek");
        fclose(file);
        return false;
    }

    size_t new_count = builder->count + file_size;
    if (new_count > builder->capacity)
    {
        builder->data = realloc(builder->data, new_count);
        if (!builder->data)
        {
            perror("realloc");
            fclose(file);
            return false;
        }

        builder->capacity = new_count;
    }

    fread(builder->data + builder->count, 1, file_size, file);
    if (ferror(file))
    {
        perror("fread");
        fclose(file);
        return false;
    }

    builder->count = new_count;
    fclose(file);
    return true;
}

// Code yoinked from https://github.com/tsoding/musializer/blob/master/src/nob.h
String_View trim_left(String_View string)
{
    size_t i = 0;
    while (i < string.count && isspace(string.data[i]))
    {
        i += 1;
    }

    String_View trimmed = {
        .data = string.data + i,
        .count = string.count - i,
    };

    return trimmed;
}

// Code yoinked from https://github.com/tsoding/musializer/blob/master/src/nob.h
String_View split_by_space(String_View string)
{
    size_t i = 0;
    while (i < string.count && !isspace(string.data[i]))
    {
        i += 1;
    }

    String_View trimmed = {
        .data = string.data,
        .count = i,
    };

    if (i < string.count)
    {
        string.count -= i + 1;
        string.data += i + 1;
    }
    else
    {
        string.count -= i;
        string.data += i;
    }

    return trimmed;
}

Hash_Table_Entry *hash_table_find(Hash_Table table, String_View key)
{
    for (size_t i = 0; i < table.count; i++)
    {
        String_View entry_key = table.entries[i].key;
        if (entry_key.count == key.count && strncmp(entry_key.data, key.data, key.count) == 0)
        {
            return &table.entries[i];
        }
    }

    return NULL;
}

int compare_entries_count(const void *a, const void *b)
{
    Hash_Table_Entry *entry_a = (Hash_Table_Entry *)a;
    Hash_Table_Entry *entry_b = (Hash_Table_Entry *)b;

    return (int)entry_b->value - (int)entry_a->value;
}

int main(int argc, char **argv)
{
    assert(argc > 0);
    char *result = *argv;
    (argv) += 1;
    (argc) -= 1;

    const char *program = result;

    if (argc <= 0)
    {
        printf("No filename provided\n");
        printf("Usage: %s <filename.txt>\n", program);
        return EXIT_FAILURE;
    }

    result = *argv;

    const char *filename = result;
    String_Builder buffer = {0};

    if (!read_file(filename, &buffer))
        return EXIT_FAILURE;

    printf("Size of %s : %zu bytes", filename, buffer.count);

    String_View content = {
        .data = buffer.data,
        .count = buffer.count,
    };

    Hash_Table table = {0};

    clock_t start_time, end_time;
    double time_spent;

    start_time = clock();

    size_t count = 0;
    for (; content.count > 0; count++)
    {
        content = trim_left(content);
        String_View token = split_by_space(content);
        Hash_Table_Entry *entry = hash_table_find(table, token);

        if (entry != NULL)
        {
            entry->value += 1;
        }
        else
        {
            if (table.count >= table.capacity)
            {
                table.capacity = table.capacity == 0 ? 1 : table.capacity * 2;
                table.entries = realloc(table.entries, table.capacity * sizeof(*table.entries));
            }

            Hash_Table_Entry new_entry = {
                .key = token,
                .value = 1,
            };

            table.entries[table.count++] = new_entry;
        }

        content.data += token.count;
        content.count -= token.count;
    }

    end_time = clock();
    time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    qsort(table.entries, table.count, sizeof(*table.entries), compare_entries_count);

    printf("\nTop 10 most common words in %s", filename);
    for (size_t i = 0; i < table.count && i < 10; i++)
    {
        printf("\n%.*s : %zu", (int)table.entries[i].key.count, table.entries[i].key.data, table.entries[i].value);
    }

    printf("\n");

    printf("\nTime spent on the linear search: %f seconds\n", time_spent);

    return EXIT_SUCCESS;
}