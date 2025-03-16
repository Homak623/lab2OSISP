#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]) {
    printf("Name: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    if (argc > 1 && strcmp(argv[1], "env") == 0) {
        printf("Reading env file...\n");
        FILE *env_file = fopen("env", "r");
        if (!env_file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char line[256];
        while (fgets(line, sizeof(line), env_file)) {
            line[strcspn(line, "\n")] = 0;
            char *value = getenv(line);
            if (value) {
                printf("%s=%s\n", line, value);
            }
        }
        fclose(env_file);
    } else {
        printf("Scanning environment...\n");
        for (char **env = envp; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    }

    return 0;
}
