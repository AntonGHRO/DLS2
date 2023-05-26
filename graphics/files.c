#include "./graphics.h"

char *string_from_filename(const char *filename) {
    FILE *file = fopen(filename, "rb");

    if(!file) debug_send("Files", "File '%s' not found. Expect further errors...", filename);

    char *string = NULL;
    unsigned length = 0;

    fseek(file, 0L, SEEK_END);
    length = ftell(file);
    fseek(file, 0L, SEEK_SET);

    string = malloc((length + 1) * sizeof(char));

    for(unsigned i=0; i<length; i++)
    {
        string[i] = fgetc(file);
    }

    string[length] = '\0';

    fclose(file);
    return string;
}

char *string_from_file(FILE *file) {
    char *string = NULL;
    unsigned length = 0;

    fseek(file, 0L, SEEK_SET);
    fseek(file, 0L, SEEK_END);
    length = ftell(file);
    fseek(file, 0L, SEEK_SET);

    for(unsigned i=0; i<length; i++)
    {
        string[i] = fgetc(file);
    }

    string[length] = '\0';

    return string;
}
