#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    printf("awesome chall\n");
    fprintf(stderr, "this is stderr\n");
    printf("who are you?\n");
    fflush(stdout);

    char* line = NULL;
    size_t num_allocated;
    size_t num_read = getline(&line, &num_allocated, stdin);

    if (num_read > 0) {
        line[num_read - 1] = '\0';

        if (*line == '/') {
            char buf[1024];
            FILE* input = fopen(line, "r");
            if (input != NULL) {
                ssize_t num_file_read = fread(buf, 1, sizeof(buf), input);
                if (num_file_read > 0) {
                    buf[num_file_read - 1] = '\0';
                    printf("%s\n", buf);
                }

                fclose(input);
            }
        } else {
            printf("you said %s\n", line);
        }
    }

    fflush(stdout);
    free(line);
    return 0;
}
