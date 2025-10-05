#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#pragma GCC diagnostic pop

#include "nbt.h"
#include "nbt_parse.h"
#include "nbt_traverse.h"

#define traverse(root) traverse(root, 0)

int main(int argc, char* argv[]) {
    static const char* default_file = "./nbt/bigtest.nbt";

    const char* filename = argc > 1 ? argv[1] : default_file;

    int inputfd;

    if (strcmp(filename, "-") == 0) {
        inputfd = STDIN_FILENO;
    } else {

        inputfd = open(filename, O_RDONLY);

        if (inputfd == -1) {
            perror("open");
            return errno;
        }
    }

    // {read_file, write_file}
    int writepipe[2]; // to child
    int readpipe[2]; // to parent

    pid_t childpid = -1;

    if (pipe(writepipe) < 0 || pipe(readpipe) < 0) {
        // an error occurred
        perror("pipe");
        return errno;
    }

    dup2(inputfd, writepipe[0]);
    close(inputfd);

    if ((childpid = fork()) < 0) {
        // Failure to fork
        perror("fork");
    } else if (childpid == 0) {
        // Child process
        close(writepipe[1]);
        close(readpipe[0]);
        
        dup2(writepipe[0], STDIN_FILENO);
        close(writepipe[0]);
        dup2(readpipe[1], STDOUT_FILENO);
        close(readpipe[1]);

        execl("./zpipe", "zpipe", "-d", NULL);

    } else {
        // Parent process
        close(writepipe[0]);
        close(readpipe[1]);

        FILE* decompressed_stream = fdopen(readpipe[0], "rb");

        NamedTag* tag;

        tag = parse_named_tag(decompressed_stream);

        fclose(decompressed_stream);

        if (!tag) {
            return 1;
        }

        traverse(tag);

        NamedTag_free(tag);

        return 0;

    }

}

#undef traverse