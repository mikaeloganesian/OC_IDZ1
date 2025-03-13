#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 5000

// Функция для поиска последовательности
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

    // Создание именованных каналов
    const char *fifo1 = "/tmp/fifo1";
    const char *fifo2 = "/tmp/fifo2";
    mkfifo(fifo1, 0666);
    mkfifo(fifo2, 0666);

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка создания процесса");
        return 1;
    }

    if (pid == 0) {
        // Второй процесс: чтение из fifo1, обработка, запись в fifo2
        int fifo_fd = open(fifo1, O_RDONLY);
        if (fifo_fd == -1) {
            perror("Ошибка открытия fifo1");
            exit(1);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(fifo_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка чтения из fifo1");
            exit(1);
        }
        buffer[bytes_read] = '\0';
        close(fifo_fd);

        char result[BUFFER_SIZE];
        find_sequence(buffer, n, result);

        fifo_fd = open(fifo2, O_WRONLY);
        if (fifo_fd == -1) {
            perror("Ошибка открытия fifo2");
            exit(1);
        }

        write(fifo_fd, result, strlen(result) + 1);
        close(fifo_fd);
        exit(0);
    } else {
        // Первый процесс: чтение из файла, запись в fifo1, чтение из fifo2, запись в файл
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

        int fifo_fd = open(fifo1, O_WRONLY);
        if (fifo_fd == -1) {
            perror("Ошибка открытия fifo1");
            return 1;
        }

        write(fifo_fd, buffer, bytes_read + 1);
        close(fifo_fd);

        fifo_fd = open(fifo2, O_RDONLY);
        if (fifo_fd == -1) {
            perror("Ошибка открытия fifo2");
            return 1;
        }

        char result[BUFFER_SIZE];
        bytes_read = read(fifo_fd, result, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Ошибка чтения из fifo2");
            return 1;
        }
        close(fifo_fd);

        fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Ошибка открытия выходного файла");
            return 1;
        }

        write(fd, result, bytes_read);
        close(fd);
    }

    // Удаление именованных каналов
    unlink(fifo1);
    unlink(fifo2);

    return 0;
}