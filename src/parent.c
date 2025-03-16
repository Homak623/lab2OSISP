#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ; // Объявление переменной environ

#define MAX_ENV_VARS 100
#define CHILD_NAME_FORMAT "child_%02d"

void print_sorted_env() {
    char *env_vars[MAX_ENV_VARS];
    int count = 0;

    for (char **env = environ; *env != NULL && count < MAX_ENV_VARS; env++) {
        env_vars[count++] = *env;
    }

    qsort(env_vars, count, sizeof(char*), (int (*)(const void*, const void*))strcmp);

    for (int i = 0; i < count; i++) {
        printf("%s\n", env_vars[i]);
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

    while (fgets(line, sizeof(line), env_file)) {
        line[strcspn(line, "\n")] = 0; // Удаляем символ новой строки
        char *value = getenv(line);
        if (value) {
            char *env_entry = malloc(strlen(line) + strlen(value) + 2); // +2 для '=' и '\0'
            sprintf(env_entry, "%s=%s", line, value);
            env_vars[count++] = env_entry;
        }
    }

    env_vars[count] = NULL; // Завершаем массив NULL
    fclose(env_file);

    // Копируем массив в динамически выделенную память
    char **result = malloc((count + 1) * sizeof(char *));
    for (int i = 0; i <= count; i++) {
        result[i] = env_vars[i];
    }

    return result;
}

void launch_child(char **env, int use_env_file) {
    static int child_count = 0;
    char child_name[16];
    sprintf(child_name, CHILD_NAME_FORMAT, child_count++);

    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {child_name, use_env_file ? "env" : NULL, NULL};
        execve(getenv("CHILD_PATH"), argv, use_env_file ? env : environ);
        perror("execve");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

int main() {
    print_sorted_env();
    char **child_env = create_child_env();

    char input;
    while (1) {
        scanf(" %c", &input);
        if (input == '+') {
            launch_child(child_env, 1);
        } else if (input == '*') {
            launch_child(child_env, 0);
        } else if (input == 'q') {
            break;
        }
    }

    for (char **env = child_env; *env != NULL; env++) {
        free(*env);
    }
    free(child_env);

    return 0;
}
