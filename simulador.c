#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int occupied;   // 0 = vazio, 1 = ocupado
    int pid;        // PID do processo dono da página
    int page;       // número da página
    int R;          // referenced bit (0 ou 1)
} Frame;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <algoritmo> <arquivo_config> <arquivo_acessos>\n", argv[0]);
        fprintf(stderr, "Algoritmo deve ser: fifo ou clock\n");
        return 1;
    }

    char *algoritmo = argv[1];
    char *arquivo_config = argv[2];
    char *arquivo_acessos = argv[3];

    int usa_fifo = 0;
    int usa_clock = 0;

    if (strcmp(algoritmo, "fifo") == 0) {
        usa_fifo = 1;
    } else if (strcmp(algoritmo, "clock") == 0) {
        usa_clock = 1;
    } else {
        fprintf(stderr, "Algoritmo inválido. Use 'fifo' ou 'clock'.\n");
        return 1;
    }

    // --- Leitura do arquivo de configuração ---
    FILE *fc = fopen(arquivo_config, "r");
    if (!fc) {
        perror("Erro ao abrir arquivo de configuração");
        return 1;
    }

    int num_frames;
    int tamanho_pagina;
    int num_processos;

    if (fscanf(fc, "%d", &num_frames) != 1) {
        fprintf(stderr, "Erro ao ler numero_de_frames\n");
        fclose(fc);
        return 1;
    }
    if (fscanf(fc, "%d", &tamanho_pagina) != 1) {
        fprintf(stderr, "Erro ao ler tamanho_da_pagina\n");
        fclose(fc);
        return 1;
    }
    if (fscanf(fc, "%d", &num_processos) != 1) {
        fprintf(stderr, "Erro ao ler numero_de_processos\n");
        fclose(fc);
        return 1;
    }

    // Lê linhas de PID e tamanho da memória virtual
    for (int i = 0; i < num_processos; i++) {
        int pid_cfg, tam_virtual;
        if (fscanf(fc, "%d %d", &pid_cfg, &tam_virtual) != 2) {
            fprintf(stderr, "Erro ao ler linha de processo\n");
            fclose(fc);
            return 1;
        }
        // Aqui poderíamos calcular o número de páginas virtuais, se quisessemos
    }

    fclose(fc);

    // --- Inicializa os frames de memória física ---
    Frame *frames = (Frame *) malloc(sizeof(Frame) * num_frames);
    if (!frames) {
        fprintf(stderr, "Erro de alocação de memória para frames\n");
        return 1;
    }

    for (int i = 0; i < num_frames; i++) {
        frames[i].occupied = 0;
        frames[i].pid = -1;
        frames[i].page = -1;
        frames[i].R = 0;
    }

    int frames_usados = 0;     // Quantos frames já foram ocupados
    int fifo_index = 0;        // Ponteiro para vítima no FIFO
    int clock_hand = 0;        // Ponteiro (relógio) para o algoritmo de Clock

    // --- Leitura do arquivo de acessos ---
    FILE *fa = fopen(arquivo_acessos, "r");
    if (!fa) {
        perror("Erro ao abrir arquivo de acessos");
        free(frames);
        return 1;
    }

    int total_acessos = 0;
    int total_page_faults = 0;

    int pid, endereco;

    while (fscanf(fa, "%d %d", &pid, &endereco) == 2) {
        total_acessos++;

        int pagina = endereco / tamanho_pagina;
        int deslocamento = endereco % tamanho_pagina;

        // Verifica HIT
        int frame_hit = -1;
        for (int i = 0; i < num_frames; i++) {
            if (frames[i].occupied &&
                frames[i].pid == pid &&
                frames[i].page == pagina) {
                frame_hit = i;
                break;
            }
        }

        if (frame_hit != -1) {
            // --- HIT ---
            frames[frame_hit].R = 1; // referenced bit é setado a 1 em todo acesso

            printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> HIT: Página %d (PID %d) já está no Frame %d\n",
                   pid, endereco, pagina, deslocamento, pagina, pid, frame_hit);
        } else {
            // --- PAGE FAULT ---
            total_page_faults++;

            // Caso 2a: ainda existe frame livre
            if (frames_usados < num_frames) {
                int idx = frames_usados;
                frames_usados++;

                frames[idx].occupied = 1;
                frames[idx].pid = pid;
                frames[idx].page = pagina;
                frames[idx].R = 1; // referenced bit = 1 ao carregar a página

                printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Página %d (PID %d) alocada no Frame livre %d\n",
                       pid, endereco, pagina, deslocamento, pagina, pid, idx);
            } else {
                // Caso 2b: memória cheia -> aplicar algoritmo de substituição
                int idx_vitima = -1;
                int pid_antigo = -1;
                int pagina_antiga = -1;

                if (usa_fifo) {
                    idx_vitima = fifo_index;
                    fifo_index = (fifo_index + 1) % num_frames;

                    pid_antigo = frames[idx_vitima].pid;
                    pagina_antiga = frames[idx_vitima].page;
                } else if (usa_clock) {
                    while (1) {
                        if (frames[clock_hand].R == 0) {
                            idx_vitima = clock_hand;
                            // Próximo da vez fica na posição seguinte
                            clock_hand = (clock_hand + 1) % num_frames;

                            pid_antigo = frames[idx_vitima].pid;
                            pagina_antiga = frames[idx_vitima].page;
                            break;
                        } else {
                            // Segunda chance: zera R e avança o relógio
                            frames[clock_hand].R = 0;
                            clock_hand = (clock_hand + 1) % num_frames;
                        }
                    }
                }

                int frame_escolhido = idx_vitima;

                printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. -> Página %d (PID %d) alocada no Frame %d\n",
                       pid, endereco, pagina, deslocamento,
                       pagina_antiga, pid_antigo, frame_escolhido,
                       pagina, pid, frame_escolhido);

                // Substitui a página no frame escolhido
                frames[frame_escolhido].pid = pid;
                frames[frame_escolhido].page = pagina;
                frames[frame_escolhido].R = 1; // referenced bit = 1 ao carregar
                frames[frame_escolhido].occupied = 1;
            }
        }
    }

    fclose(fa);

    // --- Resumo final ---
    printf("--- Simulação Finalizada (Algoritmo: %s)\n", algoritmo);
    printf("Total de Acessos: %d\n", total_acessos);
    printf("Total de Page Faults: %d\n", total_page_faults);

    free(frames);
    return 0;
}
