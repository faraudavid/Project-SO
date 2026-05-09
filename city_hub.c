#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdio.h>

#define BUFF_SIZE  1024
#define FIFO_PATH  "/tmp/.monitor_fifo"

int read_line(int fd, char *buf, int maxlen) {
    int i = 0;
    char c;
    int n;
    while (i < maxlen - 1) {
        n = read(fd, &c, 1);
        if (n <= 0) break;
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    return i;
}

void handle_scorer(int argc_count, char **args) {
    if (argc_count < 2) {
        write(STDOUT_FILENO, "Error: no districts provided\n", 29);
        return;
    }

    for (int i = 1; i < argc_count; i++) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(pipefd[0]);
            close(pipefd[1]);
            continue;
        }

        if (pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            execlp("./scorer", "scorer", args[i], (char *)NULL);
            perror("execlp scorer");
            exit(2);
        } else {
            close(pipefd[1]);
            char out_buff[1024];
            int n;
            while ((n = read(pipefd[0], out_buff, sizeof(out_buff) - 1)) > 0) {
                out_buff[n] = '\0';
                write(STDOUT_FILENO, out_buff, n);
            }
            close(pipefd[0]);
            waitpid(pid, NULL, 0);
            write(STDOUT_FILENO, "------------------------\n", 25);
        }
    }
}

void run_hub_mon(void) {
    /* hub_mon ignoră SIGINT — monitorul face cleanup */
    signal(SIGINT, SIG_IGN);

    /* ── Creează FIFO dacă nu există ── */
    mkfifo(FIFO_PATH, 0644);

    /* ── Fork monitor ── */
    pid_t monitor_pid = fork();
    if (monitor_pid < 0) {
        perror("fork monitor");
        exit(2);
    }

    if (monitor_pid == 0) {
        /* proces monitor — rulează standalone, va deschide FIFO singur */
        execlp("./monitor", "monitor", (char *)NULL);
        perror("execlp monitor_reports");
        exit(2);
    }

    /* ── hub_mon deschide FIFO pentru citire (blocking) ──
       Blochează până când monitorul deschide capătul de scriere */
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open FIFO");
        waitpid(monitor_pid, NULL, 0);
        exit(2);
    }

    /* ── Citește mesaje linie cu linie ── */
    char line[512];
    int ended = 0;

    while (read_line(fifo_fd, line, sizeof(line)) > 0) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        if (strncmp(line, "ERROR:", 6) == 0) {
            write(STDOUT_FILENO, "\n[HUB_MON] Error: ", 18);
            write(STDOUT_FILENO, line + 6, strlen(line + 6));
            write(STDOUT_FILENO, "\n", 1);
            ended = 1;
            break;
        } else if (strncmp(line, "END:", 4) == 0) {
            write(STDOUT_FILENO, "\n[HUB_MON] Monitor has ended. You can start a new monitor.\n", 59);
            ended = 1;
            break;
        } else if (strncmp(line, "MSG:", 4) == 0) {
            write(STDOUT_FILENO, "\n[HUB_MON] ", 11);
            write(STDOUT_FILENO, line + 4, strlen(line + 4));
            write(STDOUT_FILENO, "\n", 1);
        } else {
            write(STDOUT_FILENO, "\n[HUB_MON] ", 11);
            write(STDOUT_FILENO, line, strlen(line));
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    if (!ended) {
        write(STDOUT_FILENO, "\n[HUB_MON] Monitor terminated unexpectedly.\n", 44);
    }

    close(fifo_fd);
    unlink(FIFO_PATH); /* curăță FIFO după ce monitorul s-a terminat */
    waitpid(monitor_pid, NULL, 0);
    exit(0);
}

int main(int argc, const char **argv) {
    signal(SIGCHLD, SIG_IGN);

    char buffer[BUFF_SIZE];
    int n;

    while (1) {
        write(STDOUT_FILENO, "\ncity_hub > ", 12);

        n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;

        buffer[n] = '\0';
        if (buffer[n - 1] == '\n') buffer[--n] = '\0';

        char *args[16];
        int arg_count = 0;
        args[arg_count] = strtok(buffer, " ");
        while (args[arg_count] != NULL && arg_count < 15) {
            arg_count++;
            args[arg_count] = strtok(NULL, " ");
        }

        if (arg_count == 0 || args[0] == NULL) continue;

        if (strcmp(args[0], "start_monitor") == 0) {
            pid_t hub_mon_pid = fork();
            if (hub_mon_pid < 0) {
                perror("fork hub_mon");
            } else if (hub_mon_pid == 0) {
                run_hub_mon();
            }

        } else if (strcmp(args[0], "calculate_scores") == 0) {
            handle_scorer(arg_count, args);
        }
    }

    return 0;
}