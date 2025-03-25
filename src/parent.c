#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <locale.h>

extern char **environ;

#define MAX_ENV_VARS 100
#define CHILD_NAME_FORMAT "child_%02d"

int env_comparator(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void print_sorted_env() {
    char *env_vars[MAX_ENV_VARS];
    int count = 0;

    for (char **env = environ; *env != NULL && count < MAX_ENV_VARS; env++) {
        env_vars[count++] = *env;
    }

    qsort(env_vars, count, sizeof(char *), env_comparator);

    for (int i = 0; i < count; i++) {
        puts(env_vars[i]);
    }
}

char **create_child_env() {
    FILE *env_file = fopen("env", "r");
    if (!env_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char *env_vars[MAX_ENV_VARS];
    int count = 0;
    char line[256];

    while (fgets(line, sizeof(line), env_file) != NULL && count < MAX_ENV_VARS) {
        line[strcspn(line, "\n")] = '\0';
        char *value = getenv(line);
        if (!value) {
            fprintf(stderr, "Warning: Variable '%s' not found\n", line);
            continue;
        }
        env_vars[count] = malloc(strlen(line) + strlen(value) + 2);
        if (!env_vars[count]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        sprintf(env_vars[count++], "%s=%s", line, value);
    }

    env_vars[count] = NULL;
    fclose(env_file);

    char **result = malloc((count + 1) * sizeof(char *));
    if (!result) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(result, env_vars, (count + 1) * sizeof(char *));
    return result;
}

void launch_child(char **env, const char *mode_name) {
    static int child_count = 0;
    char child_name[16];
    sprintf(child_name, CHILD_NAME_FORMAT, child_count++);

    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {child_name, NULL};
        char *child_path = NULL;
        
        if (*mode_name == '+') {
            child_path = getenv("CHILD_PATH");
            printf("Using getenv() to find CHILD_PATH\n");
        } else if (*mode_name == '*') {
            // Ищем CHILD_PATH в переданном окружении envp
            extern char **environ;
            for (int i = 0; environ[i] != NULL; i++) {
                if (strncmp(environ[i], "CHILD_PATH=", 11) == 0) {
                    child_path = environ[i] + 11;
                    printf("Found CHILD_PATH in envp: %s\n", child_path);
                    break;
                }
            }
        } else if (*mode_name == '&') {
            // Ищем CHILD_PATH в глобальном environ
            for (int i = 0; environ[i] != NULL; i++) {
                if (strncmp(environ[i], "CHILD_PATH=", 11) == 0) {
                    child_path = environ[i] + 11;
                    printf("Found CHILD_PATH in environ: %s\n", child_path);
                    break;
                }
            }
        }

        if (!child_path) {
            fprintf(stderr, "Error: CHILD_PATH not found\n");
            exit(EXIT_FAILURE);
        }

        if (access(child_path, X_OK) == -1) {
            perror("access");
            exit(EXIT_FAILURE);
        }

        execve(child_path, argv, env);
        perror("execve");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        wait(NULL);
    }
}

int main(int argc, char *argv[], char *envp[]) {
    setlocale(LC_ALL, "C");
    print_sorted_env();
    char **child_env = create_child_env();

    char input;
    while (scanf(" %c", &input) == 1 && input != 'q') {
        switch (input) {
            case '+': 
            case '*': 
            case '&':
                launch_child(child_env, &input);
                break;
            default:
                printf("Unknown command\n");
                break;
        }
    }

    for (char **env = child_env; *env != NULL; env++) {
        free(*env);
    }
    free(child_env);
    return 0;
}
