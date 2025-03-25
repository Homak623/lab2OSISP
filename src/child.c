#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Name: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    FILE *env_file = fopen("env", "r");
    if (!env_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    printf("Environment variables:\n");
    char line[256];
    while (fgets(line, sizeof(line), env_file)){
        line[strcspn(line, "\n")] = '\0';
        char *value = getenv(line);
        if (value) {
            printf("%s=%s\n", line, value);
        }
    }
    fclose(env_file);

    return 0;
}
