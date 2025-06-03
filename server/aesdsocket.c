#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

volatile sig_atomic_t running = 1;
int server_fd = -1;
int client_fd = -1;

void signal_handler(int sig) {
    syslog(LOG_INFO, "Caught signal %d, exiting", sig);
    running = 0;
    if (server_fd != -1) close(server_fd);
    if (client_fd != -1) close(client_fd);
    unlink("/var/tmp/aesdsocketdata");
    closelog();
    exit(0);
}
int main(int argc, char *argv[]) {
    int daemon_mode = 0;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    openlog("aesdsocket", LOG_PID, LOG_USER);

    // 設置信號處理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 創建套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        syslog(LOG_ERR, "Failed to create socket");
        return -1;
    }

    // 設置端口可重用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        syslog(LOG_ERR, "Failed to set socket options");
        close(server_fd);
        return -1;
    }

    // 配置服務器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9000);

    // 綁定端口
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        syslog(LOG_ERR, "Failed to bind to port 9000");
        close(server_fd);
        return -1;
    }

    // 監聽
    if (listen(server_fd, 5) == -1) {
        syslog(LOG_ERR, "Failed to listen");
        close(server_fd);
        return -1;
    }

    syslog(LOG_INFO, "Server listening on port 9000");

    // 守護進程模式
    if (daemon_mode) {
        pid_t pid = fork();
        if (pid == -1) {
            syslog(LOG_ERR, "Failed to fork");
            close(server_fd);
            return -1;
        }
        if (pid > 0) {
            exit(0); // 父進程退出
        }
        setsid();
        chdir("/");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    // 主循環
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (running) syslog(LOG_ERR, "Failed to accept connection");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // 打開數據文件
        int fd = open("/var/tmp/aesdsocketdata", O_CREAT | O_APPEND | O_RDWR, 0644);
        if (fd == -1) {
            syslog(LOG_ERR, "Failed to open /var/tmp/aesdsocketdata");
            close(client_fd);
            continue;
        }

        char buffer[1024];
        char *packet = NULL;
        size_t packet_size = 0;
        ssize_t bytes_received;

        while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            char *newline = strchr(buffer, '\n');
            if (newline) {
                size_t packet_len = newline - buffer + 1;
                packet = realloc(packet, packet_size + packet_len + 1);
                if (!packet) {
                    syslog(LOG_ERR, "Memory allocation failed");
                    close(fd);
                    close(client_fd);
                    break;
                }
                memcpy(packet + packet_size, buffer, packet_len);
                packet_size += packet_len;
                packet[packet_size] = '\0';

                // 寫入文件
                if (write(fd, packet, packet_size) == -1) {
                    syslog(LOG_ERR, "Failed to write to file");
                    free(packet);
                    close(fd);
                    close(client_fd);
                    break;
                }

                // 回傳文件內容
                lseek(fd, 0, SEEK_SET);
                char read_buffer[1024];
                ssize_t bytes_read;
                while ((bytes_read = read(fd, read_buffer, sizeof(read_buffer))) > 0) {
                    if (send(client_fd, read_buffer, bytes_read, 0) == -1) {
                        syslog(LOG_ERR, "Failed to send data to client");
                        break;
                    }
                }

                free(packet);
                packet = NULL;
                packet_size = 0;
            } else {
                packet = realloc(packet, packet_size + bytes_received + 1);
                if (!packet) {
                    syslog(LOG_ERR, "Memory allocation failed");
                    close(fd);
                    close(client_fd);
                    break;
                }
                memcpy(packet + packet_size, buffer, bytes_received);
                packet_size += bytes_received;
                packet[packet_size] = '\0';
            }
        }

        if (bytes_received == -1) {
            syslog(LOG_ERR, "Failed to receive data");
        }

        free(packet);
        close(fd);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
        close(client_fd);
    }

    // 清理
    if (server_fd != -1) close(server_fd);
    unlink("/var/tmp/aesdsocketdata");
    closelog();
    return 0;
}
