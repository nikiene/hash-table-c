#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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

int main(void)
{
    const char *filename = "t8.shakespeare.txt";
    String_Builder buffer = {0};

    if (!read_file(filename, &buffer))
        return EXIT_FAILURE;

    printf("Size of %s : %zu bytes", filename, buffer.count);

    String_View content = {
        .data = buffer.data,
        .count = buffer.count,
    };

    for (size_t i = 0; i < 100 && content.count > 0; i++)
    {
        content = trim_left(content);
        String_View token = split_by_space(content);
        printf("\nToken %zu: %.*s", i + 1, (int)token.count, token.data);
        content.data += token.count;
        content.count -= token.count;
    }
    printf("\n");

    return EXIT_SUCCESS;
}