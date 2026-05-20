#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include<stdio.h>

#define PID_FILE  "/tmp/.monitor_pid"
#define FIFO_PATH "/tmp/.monitor_fifo"

volatile sig_atomic_t keep_running = 1;

/* fd-ul FIFO — global ca să fie accesibil în signal handler */
static int fifo_fd = -1;

void handle_sigint(int sig) {
    keep_running = 0;
}

void handle_sigusr1(int sig) {
    const char msg[] = "MSG:Notification received. A new report has been added\n";
    int out = (fifo_fd != -1) ? fifo_fd : STDOUT_FILENO;
    write(out, msg, sizeof(msg) - 1);
}

int main(void) {
    /* ── Verifică dacă există deja un monitor ── */
    int check_fd = open(PID_FILE, O_RDONLY);
    if (check_fd != -1) {
        char existing_pid[16];
        int bytes = read(check_fd, existing_pid, sizeof(existing_pid) - 1);
        close(check_fd);
        if (bytes > 0) {
            existing_pid[bytes] = '\0';
            pid_t pid = (pid_t)atoi(existing_pid);

            if (kill(pid, 0) == 0 || errno == EPERM) {
                /* scrie eroarea în FIFO dacă există, altfel stdout */
                int efd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
                int out = (efd != -1) ? efd : STDOUT_FILENO;

                write(out, "ERROR:Monitor already running with PID ", 39);
                write(out, existing_pid, strlen(existing_pid));
                write(out, "\n", 1);
                if (efd != -1) close(efd);
                return 1;
            } else {
                unlink(PID_FILE);
            }
        } else {
            unlink(PID_FILE);
        }
    }

    /* ── Deschide FIFO pentru scriere (non-blocking) ──
       Dacă FIFO-ul nu există (monitor pornit standalone),
       scriem la stdout normal                           */
    fifo_fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
    /* fifo_fd == -1 înseamnă că nu există FIFO — e ok */

    /* Determină unde scriem mesajele */
    int out = (fifo_fd != -1) ? fifo_fd : STDOUT_FILENO;

    /* ── Scrie PID-ul în fișier ── */
    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        write(out, "ERROR:Cannot create PID file\n", 29);
        return 1;
    }
    char pid_str[16];
    int pid_len = snprintf(pid_str, sizeof(pid_str), "%d", (int)getpid());
    write(fd, pid_str, pid_len);
    close(fd);

    /* ── Anunță hub_mon că a pornit ── */
    write(out, "MSG:Monitor started with PID ", 29);
    write(out, pid_str, pid_len);
    write(out, "\n", 1);

    /* ── Signal handlers ── */
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        unlink(PID_FILE);
        return 1;
    }

    struct sigaction sa_usr1;
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        unlink(PID_FILE);
        return 1;
    }

    /* ── Bucla principală ── */
    while (keep_running) {
        pause();
    }

    /* ── Cleanup ── */
    unlink(PID_FILE);
    write(out, "\nEND:Monitor finished. Goodbye.\n", 31);

    if (fifo_fd != -1) close(fifo_fd);

    return 0;
}