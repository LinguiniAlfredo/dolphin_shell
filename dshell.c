#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFERSIZE 1024
#define TOKENSIZE 64
#define DELIMITER " \t\r\n\a"

char *shReadLine()
{
    int buffersize = BUFFERSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffersize);
    int c;

    if (!buffer) {
        fprintf(stderr, "sh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= buffersize) {
            buffersize += BUFFERSIZE;
            buffer = realloc(buffer, buffersize); 
            if (!buffer) {
                fprintf(stderr, "sh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **shSplitLine(char *line)
{
    int buffersize = TOKENSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char*) * buffersize);
    char *token;

    if (!tokens) {
        fprintf(stderr, "sh: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIMITER);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffersize) {
            buffersize += TOKENSIZE;
            tokens = realloc(tokens, sizeof(char*) * buffersize);
            if (!tokens) {
                fprintf(stderr, "sh: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}

int shLaunch(char **args) 
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("sh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int shCd(char **args);
int shHelp(char **args);
int shExit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &shCd,
    &shHelp,
    &shExit
};

int shNumBuiltIns() {
    return sizeof(builtin_str) / sizeof(char*);
}

int shCd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
}

int shHelp(char **args)
{
    int i;
    printf("Dolphin Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < shNumBuiltIns(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int shExit(char ** args)
{
    return 0;
}

int shExecute(char **args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < shNumBuiltIns(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    
    return shLaunch(args);
}

void shLoop()
{
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = shReadLine();
        args = shSplitLine(line);
        status = shExecute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) 
{
    shLoop();

    return EXIT_SUCCESS;
}
