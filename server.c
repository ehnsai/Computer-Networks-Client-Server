#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

// دریافت دقیق n بایت
int recv_all(int sock, void *buffer, int length) 
{
    int total = 0, n;
    while (total < length) 
    {
        n = recv(sock, buffer + total, length - total, 0);
        if (n <= 0) 
        {
            return -1;
        }
        total += n;
    }
    return total;
}

int main(int argc, char *argv[]) 
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    if (argc != 2) 
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    printf("Listening on port %d...\n", port);

    addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    while (1) 
    {
        printf("$ ");
        fflush(stdout);

        fgets(command, BUFFER_SIZE, stdin);
        send(client_fd, command, strlen(command), 0);

        // دریافت طول داده
        int data_len;
        if (recv_all(client_fd, &data_len, sizeof(int)) <= 0) 
        {
            break;
        }

        data_len = ntohl(data_len);

        int received = 0;
        while (received < data_len) 
        {
            int chunk = (data_len - received > BUFFER_SIZE) ? BUFFER_SIZE : (data_len - received);
            int n = recv(client_fd, buffer, chunk, 0);
            if (n <= 0) 
            {
                break;
            }
            fwrite(buffer, 1, n, stdout);
            received += n;
        }
        printf("\n");
    }
    close(client_fd);
    close(server_fd);
    return 0;
}
