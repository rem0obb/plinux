#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main() {
    pid_t pid = getpid();

    while (1) {
        // Aloca memória para uma string
        char *msg = (char *)malloc(64);

        if (msg == NULL) {
            fprintf(stderr, "Erro ao alocar memória\n");
            return 1;
        }

        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        // Escreve na memória alocada
        snprintf(msg, 64, "PID: %d | Agora: %02d:%02d:%02d | malloc: %p\n",
			 pid, t->tm_hour, t->tm_min, t->tm_sec, &malloc);

        // Imprime a mensagem
        printf("%s", msg);

        // Libera a memória
        free(msg);

        sleep(1);
    }

    return 0;
}

