#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

void route_get(int client_sock_fd, char *uri);
void route_post(int client_sock_fd, char *uri);

void start_server(int *server_sock_fd, struct sockaddr_in *address, socklen_t *addrlen)
{
    int opt = 1;

    if((*server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Server socket failure");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(*server_sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failure");
        exit(EXIT_FAILURE);
    }
    address -> sin_family = AF_INET;
    address -> sin_addr.s_addr = INADDR_ANY;
    address -> sin_port = htons(PORT);

    if(bind(*server_sock_fd, (struct sockaddr*)address, sizeof(*address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(*server_sock_fd, 100) < 0) {
        perror("Listen failure");
        exit(EXIT_FAILURE);
    }
}

void parse_request(int client_sock_fd)
{
    char buffer[1024], *method, *uri, *prot, *temp;
    recv(client_sock_fd, buffer, sizeof(buffer) - 1, 0);

    printf("%s\n", buffer);

    method = strtok(buffer,  " \t\r\n");
    uri = strtok(NULL, " \t");
    prot = strtok(NULL, " \t\r\n");

    /*for(int i = 0; i < 14; ++i)*/
    /*{*/
    /*    temp = strtok(NULL, "\n");*/
    /*    printf("%s\n", temp);*/
    /*}*/


    printf("%s %s\n\n", method, uri);

    if(!strcmp(method, "POST"))
    {
        route_post(client_sock_fd, uri);
    }
    else
    {
        route_get(client_sock_fd, uri);
    }
}

void accept_requests_server(int server_sock_fd, struct sockaddr_in address, socklen_t addrlen)
{
    int client_sock_fd;

    printf("Server started %shttp://127.0.0.1:%d%s\n\n", "\033[92m", PORT, "\033[0m");

    while(1)
    {
        if((client_sock_fd = accept(server_sock_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Accept failure");
            exit(EXIT_FAILURE);
        }

        parse_request(client_sock_fd);

        close(client_sock_fd);
    }
}

void send_file(int client_sock_fd, char *file_name)
{
    int file_buff_size = 10240;
    char file_buff[file_buff_size];
    FILE *ptr_file;
    size_t bytes_read;

    if((ptr_file = fopen(file_name, "rb")) == NULL){
        perror("Cant open file");
        exit(EXIT_FAILURE);
    }

    while((bytes_read = fread(file_buff, 1, file_buff_size, ptr_file )) > 0)
    {
        send(client_sock_fd, file_buff, bytes_read, 0);
    }
}

