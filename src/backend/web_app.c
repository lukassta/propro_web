#include "headers.h"

int main()
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int server_sock_fd;

    initiate_db();
    start_server(&server_sock_fd, &address, &addrlen);

    constantly_accept_requests(server_sock_fd, address, addrlen);

    return 0;
}


void route_get(SSL *client_ssl, char *uri)
{
    char *path;
    path = strtok(uri, "/0");


    if(path ==  NULL)
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/index.html");
    }
    else if(!strcmp(path, "upload"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/upload.html");
    }
    else if(!strcmp(path, "snippets"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        render_snippets_page(client_ssl);
    }
    else if(!strcmp(path, "secret-admin"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        render_admin_snippets_page(client_ssl);
    }
    else if(!strcmp(path, "about"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/about.html");
    }
    else if(!strcmp(path, "favicon.png"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:image/png\r\n\r\n", 42);

        send_file(client_ssl, "./public/favicon.png");
    }
    else if(!strcmp(path, "snipper_logo.png"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./public/snipper_logo.png");
    }
    else if(!strcmp(path, "styles.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/styles.css");
    }
    else if(!strcmp(path, "index.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/index.css");
    }
    else if(!strcmp(path, "upload.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/upload.css");
    }
    else if(!strcmp(path, "snippets.css"))
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/css\r\n\r\n", 41);

        send_file(client_ssl, "./src/frontend/snippets.css");
    }
    else // Not found
    {
        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        SSL_write(client_ssl, "<h1>404</h1>\n", 14);
    }
}


void route_post(SSL *client_ssl, char *uri, dict *body)
{
    int id;
    char *path;
    path = strtok(uri, "/0");


    if(path == NULL)
    {
        SSL_write(client_ssl, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
    }
    else if(!strcmp(path, "submit-snippet"))
    {
        int error = 0;

        char *team_name = get_value_by_key(body, "team_name");
        char *title = get_value_by_key(body, "title");
        char *language = get_value_by_key(body, "language");
        char *code = get_value_by_key(body, "code");
        char *description = get_value_by_key(body, "description");

        if(!team_name || !title || !language || !code || !description)
        {
            bad_request_body(client_ssl);
            return;
        }

        error = create_snippet(team_name, title, language, code, description);

        if(error)
        {
            perror("Error writing to database");
            exit(EXIT_FAILURE);
        }

        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        send_file(client_ssl, "./src/frontend/index.html");
    }
    else if(!strcmp(path, "secret-delete-snippet"))
    {
        path = strtok(NULL, "/0");

        id = atoi(path);

        if(id == 0)
        {
            SSL_write(client_ssl, "HTTP/1.1 404 Not Found\r\n\r\n", 26);

            return;
        }

        if(delete_snippet(id) != 0)
        {
            SSL_write(client_ssl, "HTTP/1.1 500 Internal Server Error\r\n\r\n", 38);

            return;
        }

        SSL_write(client_ssl, "HTTP/1.1 200 OK\nContent-Type:text/html\r\n\r\n", 42);

        render_admin_snippets_page(client_ssl);
    }
    else
    {
        SSL_write(client_ssl, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
    }
}


void bad_request_body(SSL *client_ssl)
{
    SSL_write(client_ssl, "HTTP/1.1 400 Bad request\r\n\r\n", 28);
}

