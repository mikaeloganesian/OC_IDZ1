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

int main() {
    const char *fifo1 = "/tmp/fifo1";
    const char *fifo2 = "/tmp/fifo2";

    // Получение данных через fifo1
    int fifo_fd = open(fifo1, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Ошибка открытия fifo1");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fifo_fd, buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("Ошибка чтения из fifo1");
        return 1;
    }
    buffer[bytes_read] = '\0';
    close(fifo_fd);

    // Разделение данных и N
    char *data = strtok(buffer, "\n");
    char *n_str = strtok(NULL, "\n");
    int n = atoi(n_str);

    // Обработка данных
    char result[BUFFER_SIZE];
    find_sequence(data, n, result);

    // Передача результата через fifo2
    fifo_fd = open(fifo2, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Ошибка открытия fifo2");
        return 1;
    }

    write(fifo_fd, result, strlen(result) + 1);
    close(fifo_fd);

    return 0;
}