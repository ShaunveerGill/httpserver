#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <ctype.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "bind.h"

int GET_HEAD_RESPONSE(int client, char *URI, bool head) {
    struct stat sz;
    char buf[4096];
    char info[4096];
    int con_len = 0;
    int fd;
    char *ptr_URI;
    int bytes_wrote = 0;
    int bytes_read = 0;
    int status_code = 200;
    char *status_phrase = "OK";
    if (status_code == 200) {
        ptr_URI = URI + 1;
        fd = open(ptr_URI, O_RDWR);
        if (fd == -1) {
            if (errno == EISDIR) {
                status_code = 403;
                status_phrase = "Forbidden";
            } else if (errno == EACCES) {
                status_code = 403;
                status_phrase = "Forbidden";
            } else if (errno == ENOENT) {
                status_code = 404;
                status_phrase = "Not Found";
            } else {
                sprintf(info, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                              "16\r\n\r\nInternal Server Error\n");
                bytes_wrote = write(client, info, strlen(info));
                memset(buf, '\0', 4096);
                memset(info, '\0', 4096);
                return 0;
            }
        }
    }
    if (status_code == 200) {
        stat(ptr_URI, &sz);
        con_len = sz.st_size;
        sprintf(info, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n", status_code, status_phrase,
            con_len);
        bytes_wrote = write(client, info, strlen(info));
    } else {
        con_len = strlen(status_phrase) + 1;
        sprintf(info, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s\n", status_code,
            status_phrase, con_len, status_phrase);
        bytes_wrote = write(client, info, strlen(info));
        memset(buf, '\0', 4096);
        memset(info, '\0', 4096);
        return 0;
    }
    while (con_len > 0 && head == 0) {
        bytes_read = read(fd, buf, 4096);
        bytes_wrote = write(client, buf, bytes_read);
        con_len -= bytes_wrote;
    }
    close(fd);
    memset(buf, '\0', 4096);
    memset(info, '\0', 4096);
    return 0;
}

int PUT_RESPONSE(int client, char *URI, char *ptr, int con_len) {
    char buf[4096];
    char info[4096];
    char *ptr_URI = URI + 1;
    int status_code = 200;
    char *status_phrase = "OK";
    int phrase_len = strlen(status_phrase) + 1;
    int bytes_read = 1;
    int bytes_wrote = 0;
    int fd = open(ptr_URI, O_TRUNC | O_WRONLY);
    if (fd == -1) {
        if (errno == EISDIR || errno == EACCES) {
            sprintf(info, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n");
            bytes_wrote = write(client, info, strlen(info));
            memset(buf, '\0', 4096);
            memset(info, '\0', 4096);
            return 0;
        } else if (errno == ENOENT) {
            fd = open(ptr_URI, O_WRONLY | O_CREAT, 0664);
            status_code = 201;
            status_phrase = "Created";
        } else {
            sprintf(info, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                          "16\r\n\r\nInternal Server Error\n");
            bytes_wrote = write(client, info, strlen(info));
            memset(buf, '\0', 4096);
            memset(info, '\0', 4096);
            return 0;
        }
    }
    if (con_len == 0) {
        phrase_len = strlen(status_phrase) + 1;
        sprintf(info, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s\n", status_code,
            status_phrase, phrase_len, status_phrase);
        bytes_wrote = write(client, info, strlen(info));
        memset(buf, '\0', 4096);
        memset(info, '\0', 4096);
        return 0;
    }
    ptr = ptr + 4;
    int ptr_len = strlen(ptr);
    if (con_len < ptr_len) {
        bytes_wrote = write(fd, ptr, con_len);
    } else {
        bytes_wrote = write(fd, ptr, strlen(ptr));
    }
    con_len -= bytes_wrote;
    while (bytes_read > 0 && con_len > 0) {
        bytes_read = read(client, buf, 4096);
        if (con_len < bytes_read) {
            bytes_wrote = write(fd, buf, con_len);
        } else {
            bytes_wrote = write(fd, buf, bytes_read);
        }
        con_len -= bytes_wrote;
    }
    phrase_len = strlen(status_phrase) + 1;
    sprintf(info, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s\n", status_code, status_phrase,
        phrase_len, status_phrase);
    bytes_wrote = write(client, info, strlen(info));
    close(fd);
    memset(buf, '\0', 4096);
    memset(info, '\0', 4096);
    return 0;
}

int VALID_REQUEST(int client, char *URI, char *verison, char *method) {
    char info[4096];
    int bytes_wrote = 0;
    for (size_t i = 0; i < strlen(method); i++) {
        if (isalpha(method[i]) == 0) {
            sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
            bytes_wrote = write(client, info, strlen(info));
            return 1;
        }
    }
    if ((strcmp(method, "PUT") != 0) && (strcmp(method, "HEAD") != 0)
        && (strcmp(method, "GET") != 0) && strlen(method) <= 8) {
        sprintf(
            info, "%s 501 Not Implemented\r\nContent-Length: 19\r\n\r\nNot Implemented\n", verison);
        bytes_wrote = write(client, info, strlen(info));
        return 1;
    }
    if (strlen(method) > 8) {
        sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
        bytes_wrote = write(client, info, strlen(info));
        return 1;
    }

    if (strcmp(verison, "HTTP/1.1") != 0 || strlen(URI) > 20) {
        sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
        bytes_wrote = write(client, info, strlen(info));
        return 1;
    }
    for (size_t i = 0; i < strlen(URI); i++) {
        if (isdigit(URI[i]) == 0 && isalpha(URI[i]) == 0 && URI[i] != '.' && URI[i] != '_'
            && URI[i] != '/') {
            sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
            bytes_wrote = write(client, info, strlen(info));
            return 1;
        }
    }
    memset(info, '\0', 4096);
    return 0;
}

int VALID_HEADERS(int acc_soc, char *read_buf, char *head, char *req_line) {
    int byte_offset = 0;
    int bytes_wrote = 0;
    char info[4096];
    bool value = false;
    int line_count = 0;
    int read_buf_size = strlen(read_buf);
    for (int i = 0; i < (read_buf_size); i++) {
        if (read_buf[i + 1] == ' ' && read_buf[i] == ' ') {
            sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
            bytes_wrote = write(acc_soc, info, strlen(info));
            return 1;
        }
        if (read_buf[i] == '\r' && read_buf[i + 1] == '\n') {
            byte_offset += 2;
            break;
        }
        req_line[i] = read_buf[i];
        byte_offset += 1;
    }
    int j = 0;
    for (int i = byte_offset; i < read_buf_size; i++) {
        if (j == 0) {
            if (isascii(read_buf[0]) != 0) {
                head[j] = read_buf[i];
                j++;
                continue;
            } else {
                sprintf(
                    info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
                bytes_wrote = write(acc_soc, info, strlen(info));
                return 1;
            }
        }
        if (read_buf[i] == '\r' || read_buf[i] == '\n') {
            line_count += 1;
            continue;
        }
        if (line_count == 4) {
            break;
        }
        if (line_count == 2) {
            value = false;
        }
        if (isascii(read_buf[i] != 0)) {
            if (value == false && read_buf[i] == ' ' && read_buf[i - 1] != ':') {
                sprintf(
                    info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
                bytes_wrote = write(acc_soc, info, strlen(info));
                return 1;
            } else if (read_buf[i] == ' ' && read_buf[i - 1] == ':') {
                value = true;
            } else if (read_buf[i] == ' ' && value != true) {
                sprintf(
                    info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
                bytes_wrote = write(acc_soc, info, strlen(info));
                return 1;
            }
        } else {
            sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
            bytes_wrote = write(acc_soc, info, strlen(info));
            return 1;
        }
        line_count = 0;
        head[j] = read_buf[i];
        j++;
    }
    /*    if (value != true) {
        sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
        bytes_wrote = write(acc_soc, info, strlen(info));
        return 1;
    }*/
    return 0;
}

int main(int argc, char *argv[]) {
    char *ptr = NULL;
    char method[10];
    char URI[21];
    char verison[10];
    char read_buf[4096];
    char info[4096];
    char head[2098];
    char req_line[2098];
    char *mes_ptr = NULL;
    char *num_ptr = NULL;
    int con_len = 0;
    int result = 0;
    int bytes_wrote = 0;
    int acc_soc = 0;
    int bytes_read = 0;
    int scan_bytes = 0;
    //may break when given not integer string not number
    if (argc <= 1) {
        warnx("wrong arguments: ./httpserver port_num\nusage: ./httpserver <port>");
        return 1;
    }
    for (size_t i = 0; i < strlen(argv[1]); i++) {
        if (isdigit(argv[1][i]) == 0) {
            warnx("invalid port number: %s\n", argv[1]);
            return 1;
        }
    }
    uint16_t port_num = strtol(argv[1], &num_ptr, 10);
    int soc = create_listen_socket(port_num);
    if (soc <= 0) {
        warnx("%s", strerror(errno));
        return 1;
    }
    while (1) {
        acc_soc = accept(soc, NULL, NULL);
        if (acc_soc < 0) {
            sprintf(info, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n");
            bytes_wrote = write(acc_soc, info, strlen(info));
            close(acc_soc);
            memset(read_buf, '\0', 4096);
            memset(head, '\0', 2098);
            memset(req_line, '\0', 2098);
            memset(info, '\0', 4096);
            continue;
        }
        bytes_read = read(acc_soc, read_buf, 4096);
        if (bytes_read > 0) {
            result = VALID_HEADERS(acc_soc, read_buf, head, req_line);
        }
        scan_bytes = sscanf(read_buf, "%s %s %s %n", method, URI, verison, &scan_bytes);
        if (result == 0 && 0 < scan_bytes && VALID_REQUEST(acc_soc, URI, verison, method) == 0) {
            if (!(strcmp(method, "HEAD"))) {
                GET_HEAD_RESPONSE(acc_soc, URI, true);
            } else if (!(strcmp(method, "GET"))) {
                GET_HEAD_RESPONSE(acc_soc, URI, false);
            } else if (!(strcmp(method, "PUT"))) {
                ptr = strstr(read_buf, "Content-Length");
                if (ptr == NULL) {
                    sprintf(info,
                        "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
                    bytes_wrote = write(acc_soc, info, strlen(info));
                } else {
                    ptr = ptr + 16;
                    for (size_t i = 0; ptr[i] != '\r'; i++) {
                        if (isdigit(ptr[i]) == 0) {
                            sprintf(info, "HTTP/1.1 400 Bad Request\r\nContent-Length: "
                                          "12\r\n\r\nBad Request\n");
                            bytes_wrote = write(acc_soc, info, strlen(info));
                            close(acc_soc);
                            memset(read_buf, '\0', 4096);
                            memset(head, '\0', 2098);
                            memset(req_line, '\0', 2098);
                            memset(info, '\0', 4096);
                            continue;
                        }
                    }
                    num_ptr = NULL;
                    con_len = strtol(ptr, &num_ptr, 10);
                    mes_ptr = strstr(read_buf, "\r\n\r\n");
                    PUT_RESPONSE(acc_soc, URI, mes_ptr, con_len);
                }
            }
        }
        memset(read_buf, '\0', 4096);
        memset(head, '\0', 2098);
        memset(req_line, '\0', 2098);
        memset(info, '\0', 4096);
        close(acc_soc);
    }
    return 0;
}
