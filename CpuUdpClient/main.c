#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    double *data;
    int count;
    int ready;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} SharedData;

SharedData shared;

typedef struct {
    char ip[16];
    int port;
} NetArgs;

void* udp_sender_thread(void *arg) {
    NetArgs *args = (NetArgs*)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(args->port) };
    inet_aton(args->ip, &addr.sin_addr);

    double *local_buffer = malloc(sizeof(double) * shared.count);

    while (1) {
        pthread_mutex_lock(&shared.mtx);
        while (!shared.ready) {
            pthread_cond_wait(&shared.cond, &shared.mtx);
        }

        memcpy(local_buffer, shared.data, sizeof(double) * shared.count);
        shared.ready = 0;
        pthread_mutex_unlock(&shared.mtx);

        sendto(sock, local_buffer, sizeof(double) * shared.count, 0,
               (struct sockaddr *)&addr, sizeof(addr));
    }
    return NULL;
}

void parse_cpu(char *line, unsigned long long *t, unsigned long long *i) {
    char *p = line; while (*p != ' ' && *p) p++;
    unsigned long long v[10];
    for (int j = 0; j < 10; j++) v[j] = strtoull(p, &p, 10);
    *i = v[3] + v[4];
    *t = v[0]+v[1]+v[2]+v[3]+v[4]+v[5]+v[6]+v[7];
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
	printf("Usage: %s <IP> <Port>\n", argv[0]);
	return 1;
    }

    int n_cores = sysconf(_SC_NPROCESSORS_ONLN) + 1;
    shared.count = n_cores;
    shared.data = calloc(n_cores, sizeof(double));
    shared.ready = 0;
    pthread_mutex_init(&shared.mtx, NULL);
    pthread_cond_init(&shared.cond, NULL);

    NetArgs n_args;
    strncpy(n_args.ip, argv[1], 15);
    n_args.port = atoi(argv[2]);

    pthread_t tid;
    pthread_create(&tid, NULL, udp_sender_thread, &n_args);

    int fd = open("/proc/stat", O_RDONLY);
    char buf[16384];
    typedef struct { unsigned long long t, i; } Snapshot;
    Snapshot *prev = calloc(n_cores, sizeof(Snapshot));
    Snapshot curr;

    while (1) {
        ssize_t n = pread(fd, buf, sizeof(buf)-1, 0);
        buf[n] = '\0';
        char *line = buf;

        pthread_mutex_lock(&shared.mtx);
        for (int j = 0; j < n_cores; j++) {
            parse_cpu(line, &curr.t, &curr.i);
            unsigned long long dt = curr.t - prev[j].t;
            unsigned long long di = curr.i - prev[j].i;
            shared.data[j] = (dt > 0) ? (double)(dt - di) / dt * 100.0 : 0.0;
            prev[j] = curr;
            line = strchr(line, '\n'); if (line) line++; else break;
        }
        shared.ready = 1;
        pthread_cond_signal(&shared.cond);
        pthread_mutex_unlock(&shared.mtx);

        //printf("Main: данные обновлены и переданы в сеть\n");
        sleep(1);
    }
    return 0;
}
