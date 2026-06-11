#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80
#define HISTORY_SIZE 10

char history[HISTORY_SIZE][MAX_LINE];
int history_count = 0;

void add_history(char command[]) {
    int index = history_count % HISTORY_SIZE;
    strcpy(history[index], command);
    history_count++;
}

void show_history() {
    int start;
    int i;
    int index;

    if (history_count == 0) {
        printf("Nenhum comando salvo no histório\n");
        return;
    }

    if (history_count < HISTORY_SIZE) {
        start = 1;
    } else {
        start = history_count - HISTORY_SIZE + 1;
    }
    
    for (i = history_count; i >= start; i--) {
        index = (i - 1) % HISTORY_SIZE;
        printf("%d %s\n", i, history[index]);
    }
}

int parse_command(char input[], char *args[]) {
    int i = 0;
    int background = 0;
    char *token = strtok(input, " ");
 
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
 
    args[i] = NULL;
 
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1;
        args[i - 1] = NULL;
    }
 
    return background;
}

void execute_command(char *args[], int background) {
    pid_t pid;
 
    pid = fork();
 
    if (pid < 0) {
        printf("Erro ao criar processo filho\n");
    }
    else if (pid == 0) {
        execvp(args[0], args);
        printf("erro: comando inválido\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("[Processo filho - PID: %d]\n", pid);

        if (!background) {
            wait(NULL);
        }
    }
}

int main(void) {
    char input[MAX_LINE];
    char input_copy[MAX_LINE];
    int should_run = 1;    

    while (should_run) {
        printf("osh> ");
        fflush(stdout); 

        if (fgets(input, MAX_LINE, stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        if (strcmp(input, "!!") == 0) {
            if (history_count == 0) {
                printf("Nenhum comando salvo no histórico\n");
                continue;
            }
            int index = (history_count - 1) % HISTORY_SIZE;
            strcpy(input, history[index]);
            printf("%s\n", input);
        } 
        else if (input[0] == '!') {
            int n = atoi(&input[1]);
            int start = (history_count < HISTORY_SIZE) ? 1 : history_count - HISTORY_SIZE + 1;
            
            if (n < start || n > history_count || n == 0) {
                printf("Este comando não foi encontrado no histórico\n");
                continue;
            }
            int index = (n - 1) % HISTORY_SIZE;
            strcpy(input, history[index]);
            printf("%s\n", input);
        }

        add_history(input);

        strcpy(input_copy, input);

        char *args[MAX_LINE / 2 + 1];
        int background = parse_command(input_copy, args);

        if (args[0] == NULL) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        } else if (strcmp(args[0], "history") == 0) {
            show_history();
            continue;
        } else if (strcmp(args[0], "ajuda") == 0) {
            printf("Comandos:\n");
            printf("-> history: exibe os ultimos comandos digitados\n");
            printf("-> ajuda: exibe os comandos suportados\n");
            printf("-> exit: encerra o mini-shell\n");
            continue;
        }

        execute_command(args, background);
    }

    return 0;
}
