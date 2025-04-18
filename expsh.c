#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include <readline/readline.h>
#include <readline/history.h>

#define HIST_FILENAME ".expsh_history"
#define MAX_ARG_LEN   256
#define INIT_CAP      8

typedef struct {
    char** args;
    size_t count;
    size_t cap;
} args_t;

int parse_args(char** args, size_t count);

void args_alloc(args_t* a, size_t wanted_cap);
void args_append(args_t* a, const char* arg);
void args_append_many(args_t* a, const char** args, size_t count);
#define args_append_more(a, ...) {args_vappend((a), __VA_ARGS__, NULL)}
void args_vappend(args_t* a, ...);
void free_args(args_t args);

int print_cmd_history(const char* filename);

void args_print(args_t args);
args_t get_args(char* line);

char* sdup(const char* str);

int main(int argc, char* argv[]) {
    int quit = 0;

    args_t args = {0};
    const char* prompt = "$ ";

    read_history(HIST_FILENAME);

    while (!quit) {
        char* buff = readline(prompt);
        if (buff == NULL) return 1;
        args = get_args(buff);
        if (args.count == 0) continue;
        quit = parse_args(args.args, args.count);
        add_history(buff);
    }
    write_history(HIST_FILENAME);
    free_args(args);
    return 0;
}

int parse_args(char** args, size_t count) {
    int res;
    if (!strcmp(args[0], "exit") || !strcmp(args[0], "quit")) {
        return 1;
    } else if (!strcmp(args[0], "hist")) {
        return print_cmd_history(HIST_FILENAME);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("Cannot fork a process");
    } else if (pid == 0) {
        if (execvp(args[0], (char* const*)args) == -1) {
            perror("Cannot execute a process");
        }
    }
    waitpid(-1, &res, 0);
    if (WIFEXITED(res)) {
        fprintf(stdout, "Exit status of child process was %d\n", WEXITSTATUS(res));
    }
    return 0;
}

args_t get_args(char* line) {
    args_t args = {0};
    const char* delim = " \r\t\b\a";
    char* tmp = sdup(line);
    char* token = strtok(tmp, delim);
    while (token != NULL) {
        args_append(&args, token);
        token = strtok(NULL, delim);
    }
    free(tmp);
    args_alloc(&args, 1);
    args.args[args.count] = NULL;
    return args;
}

void args_append(args_t* a, const char* arg) {
    args_alloc(a, a->count+1);
    memcpy(a->args[a->count++], arg, strlen(arg) + 1);
}

void args_alloc(args_t* a, size_t wanted_cap) {
    if (wanted_cap >= a->cap) {
        if (a->cap == 0) a->cap = INIT_CAP;

        while (wanted_cap > a->cap) a->cap *= 2;

        a->args = realloc(a->args, a->cap * sizeof(char*));
        for (int i = 0; i < a->cap; ++i) {
            a->args[i] = realloc(a->args[i], MAX_ARG_LEN);
        }
    }
}

void args_append_many(args_t* a, const char** args, size_t count) {
    args_alloc(a, a->count+count);
    for (size_t i = 0; i < count; ++i) {
        memcpy(a->args[a->count+i], args[i], strlen(args[i]) + 1);
    }
    a->count += count;
}

void args_vappend(args_t* a, ...) {
    va_list ap;
    va_start(ap, a);
    char* arg; 
    while ((arg = va_arg(ap, char*)) != NULL) {
        args_alloc(a, a->count+1);
        memcpy(a->args[a->count++], arg, strlen(arg) + 1);
    }
    va_end(ap);
}

void free_args(args_t args) {
    for (int i = 0; i < args.count; ++i) {
        free(args.args[i]);
    }
    free(args.args);
}

int print_cmd_history(const char* filename) {
    FILE* hist_file = fopen(filename, "r");
    if (hist_file == NULL) {
        fprintf(stderr, "Cannot open file '%s': %s!\n", filename, strerror(errno));
        return 1;
    }
    char line[1024];
    while (fgets(line, sizeof(line), hist_file) != NULL) {
        fprintf(stdout, "%s", line);
    }
    return 0;
}

void args_print(args_t args) {
    printf("[");
    for (int i = 0; i < args.count; ++i) {
        if (i == args.count-1) printf("%s", args.args[i]);
        else printf("%s, ", args.args[i]);
    }
    printf("]\n");
}


char* sdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    assert(new_str != NULL && "Allocation Failed");
    memcpy(new_str, str, len);
    return new_str;
}
