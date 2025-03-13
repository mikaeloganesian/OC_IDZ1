#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 5000

void find_sequence(const char *input, int n, char *result) {
    int len = strlen(input);
    for (int i = 0; i <= len - n; i++) {
        int is_sequence = 1;
        for (int j = 1; j < n; j++) {
            if (input[i + j] >= input[i + j - 1]) {
                is_sequence = 0;
                break;
            }
        }
        if (is_sequence) {
            strncpy(result, input + i, n);
            result[n] = '\0';
            return;
        }
    }
    strcpy(result, "Последовательность не найдена");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Использование: %s <входной_файл> <выходной_файл> <N>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    int n = atoi(argv[3]);

    if (n <= 0) {
        printf("Ошибка: N должно быть положительным числом.\n");
        return 1;
    }

    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Ошибка создания канала");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка создания процесса");
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(pipe1[0], buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка чтения из канала");
            exit(1);
        }
        buffer[bytes_read] = '\0';
        close(pipe1[0]);

        char result[BUFFER_SIZE];
        find_sequence(buffer, n, result);

        write(pipe2[1], result, strlen(result) + 1);
        close(pipe2[1]);
        exit(0);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        int fd = open(input_file, O_RDONLY);
        if (fd == -1) {
            perror("Ошибка открытия входного файла");
            return 1;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == -1) {
            perror("Ошибка чтения из файла");
            return 1;
        }
        buffer[bytes_read] = '\0';
        close(fd);

        write(pipe1[1], buffer, bytes_read + 1);
        close(pipe1[1]);

        char result[BUFFER_SIZE];
        bytes_read = read(pipe2[0], result, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка чтения из канала");
            return 1;
        }
        close(pipe2[0]);

        fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Ошибка открытия выходного файла");
            return 1;
        }

        write(fd, result, bytes_read);
        close(fd);
    }

    return 0;
}