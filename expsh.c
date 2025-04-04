#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <readline/readline.h>

#define MAX_ARG_LEN 256
#define INIT_CAP      8

typedef struct {
    const char** args;
    size_t count;
    size_t cap;
} Args_t;

int parse_args(const char** args, size_t count);

void args_alloc(Args_t* a, size_t wanted_cap);

void args_append(Args_t* a, const char* arg);
void args_append_many(Args_t* a, const char** args, size_t count);
#define args_append_more(a, ...) {args_vappend((a), __VA_ARGS__, NULL)}
void args_vappend(Args_t* a, ...);

void args_print(Args_t args);

Args_t get_args(char* line);
char* sdup(const char* str);

int main(int argc, char* argv[]) {
    int quit = 0;

    Args_t args = {0};
    size_t count;
    const char* prompt = "$ ";
    while (!quit && !feof(stdin)) {
        char* buff = readline(prompt);
        args = get_args(buff);
        args_print(args);
        quit = parse_args(args.args, args.count);
    }
    return 0;
}

int parse_args(const char** args, size_t count) {

}

Args_t get_args(char* line) {
    Args_t args = {0};
    const char* delim = " \n\r\t\b\a";
    char* tmp = sdup(line);
    char* token = strtok(tmp, delim);
    while (token != NULL) {
        args_append(&args, token);
        token = strtok(NULL, delim);
    }
    free(tmp);

    return args;
}

void args_append(Args_t* a, const char* arg) {
    args_alloc(a, a->count+1);
    memcpy(a->args[a->count++], arg, strlen(arg) + 1);
}

void args_alloc(Args_t* a, size_t wanted_cap) {
    if (wanted_cap >= a->cap) {
        if (a->cap == 0) a->cap = INIT_CAP;

        while (wanted_cap > a->cap) a->cap *= 2;

        a->args = realloc(a->args, a->cap * sizeof(char*));
        for (int i = 0; i < a->cap; ++i) {
            a->args[i] = realloc(a->args[i], MAX_ARG_LEN);
        }
    }
}

void args_append_many(Args_t* a, const char** args, size_t count) {
    args_alloc(a, a->count+count);
    for (size_t i = 0; i < count; ++i) {
        memcpy(a->args[a->count+i], args[i], strlen(args[i]) + 1);
    }
    a->count += count;
}

void args_vappend(Args_t* a, ...) {
    va_list ap;
    va_start(ap, a);
    char* arg; 
    while ((arg = va_arg(ap, char*))) {
        args_alloc(a, a->count+1);
        memcpy(a->args[a->count++], arg, strlen(arg) + 1);
    }
    va_end(ap);
}

void args_print(Args_t args) {
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
