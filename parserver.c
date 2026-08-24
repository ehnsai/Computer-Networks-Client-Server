#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUF 4096
#define MAX 30

typedef struct 
{
    int fd;
    int waiting;
} cl;

cl c[MAX];
int ncl = 0;

int send_all(int s, void *b, int l) 
{
    int t = 0, n;
    while (t < l) 
    {
        n = send(s, (char*)b + t, l - t, 0);
        if (n <= 0) 
        {
            return -1;
        }
        t += n;
    }
    return 0;
}

int recv_all(int s, void *b, int l) 
{
    int t = 0, n;
    while (t < l) 
    {
        n = recv(s, (char*)b + t, l - t, 0);
        if (n <= 0) return -1;
        t += n;
    }
    return 0;
}

void add(int fd) 
{
    c[ncl].fd = fd;
    c[ncl].waiting = 0;
    printf("[+] Client %d connected.\n", ncl);
    ncl++;
}

void broadcast(char *cmd) 
{
    for (int i = 0; i < ncl; i++) 
    {
        send_all(c[i].fd, cmd, strlen(cmd));
        c[i].waiting = 1;
    }
}

void send_one(int id, char *cmd) 
{
    if (id < 0 || id >= ncl) 
    {
        printf("invalid client id.\n");
        return;
    }
    send_all(c[id].fd, cmd, strlen(cmd));
    c[id].waiting = 1;
}

void list_clients() 
{
    printf("\n**** Clients ****\n");
    for (int i = 0; i < ncl; i++) 
    {
        printf("client%d -> fd:%d %s\n",i, c[i].fd,c[i].waiting ? "(waiting)" : "");
    }
}

void process(int i) 
{
    int len;
    if (recv_all(c[i].fd, &len, 4) < 0) 
    {
        return;
    }

    len = ntohl(len);

    char *buf = malloc(len + 1);

    if (len > 0)
    {
        recv_all(c[i].fd, buf, len);
    }

    buf[len] = 0;
    printf("\n**** Client %d ****\n%s\n", i, buf);
    free(buf);
    c[i].waiting = 0;
}

void menu() 
{
    printf("\n---------- MAIN MENU ----------\n");
    printf("1. broadcast command\n");
    printf("2. send to one client\n");
    printf("3. list clients\n");
    printf("4. exit\n");
    printf("---------------------------------\n");
    printf("choice: ");
}

int main(int argc, char *argv[]) 
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons(atoi(argv[1]));
    a.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr*)&a, sizeof(a));
    listen(s, 10);

    printf("server running...\n");

    fd_set f;

    while (1) 
    {
        FD_ZERO(&f);
        FD_SET(s, &f);
        FD_SET(0, &f);

        int mx = s;

        for (int i = 0; i < ncl; i++) 
        {
            FD_SET(c[i].fd, &f);
            if (c[i].fd > mx) mx = c[i].fd;
        }
        select(mx + 1, &f, NULL, NULL, NULL);
        if (FD_ISSET(s, &f)) 
        {
            int fd = accept(s, NULL, NULL);
            add(fd);
        }
        if (FD_ISSET(0, &f)) 
        {
            menu();
            char input[BUF];
            fgets(input, BUF, stdin);
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "4") == 0) 
            {
                printf("back to loop...\n");
                continue;
            }
            if (strcmp(input, "3") == 0) 
            {
                list_clients();
                continue;
            }
            if (strcmp(input, "1") == 0) 
            {
                char cmd[BUF];
                printf("cmd> ");
                fgets(cmd, BUF, stdin);
                cmd[strcspn(cmd, "\n")] = 0;
                broadcast(cmd);
                continue;
            }
            if (strcmp(input, "2") == 0) 
            {
                char line[BUF];
                printf("format:client_id command\n");
                printf("> ");
                fgets(line, BUF, stdin);
                line[strcspn(line, "\n")] = 0;
                int id;
                char cmd[BUF];
                if (sscanf(line, "%d %[^\n]", &id, cmd) == 2) 
                {
                    send_one(id, cmd);
                } 
                else 
                {
                    printf("bad format\n");
                }
                continue;
            }
        }
        for (int i = 0; i < ncl; i++) 
        {
            if (FD_ISSET(c[i].fd, &f)) 
            {
                process(i);
            }
        }
    }
}