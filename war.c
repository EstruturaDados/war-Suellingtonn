// ============================================================================
//         PROJETO WAR ESTRUTURADO - DESAFIO DE CÓDIGO
// ============================================================================
//
// OBJETIVOS:
// - Modularizar completamente o código em funções especializadas.
// - Implementar um sistema de missões para um jogador.
// - Criar uma função para verificar se a missão foi cumprida.
// - Utilizar passagem por referência (ponteiros) para modificar dados e
//   passagem por valor/referência constante (const) para apenas ler.
// - Foco em: Design de software, modularização, const correctness, lógica de jogo.
//
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Constantes Globais ---
#define MAX_TERRITORIOS 6
#define MAX_NOME 30
#define MAX_COR  10
#define MAX_MISSAO 128

// --- Estrutura de Dados ---
typedef struct {
    char nome[MAX_NOME];
    char cor[MAX_COR];   // dono do território (ex.: "Azul", "Vermelha")
    int tropas;
} Territorio;

// --- Protótipos das Funções ---
// Setup e memória
Territorio* alocarMapa(int tamanho);
void inicializarTerritorios(Territorio* mapa, int tamanho);
void liberarMemoria(Territorio* mapa, char* missaoJogador);

// Interface
void exibirMenuPrincipal(void);
void exibirMapa(const Territorio* mapa, int tamanho);
void exibirMissao(const char* missao);

// Lógica principal
void atacar(Territorio* atacante, Territorio* defensor);
int verificarMissao(char* missao, Territorio* mapa, int tamanho);

// Missões
int sortearMissaoIndex(int totalMissoes);
void atribuirMissao(char* destino, char* missoes[], int totalMissoes);

// Utilitárias
void limparBufferEntrada(void);

// --- Função Principal (main) ---
int main(void) {
    // 1. Configuração Inicial (Setup):
    srand((unsigned)time(NULL));

    int tamanho = MAX_TERRITORIOS;
    Territorio* mapa = alocarMapa(tamanho);
    if (!mapa) {
        fprintf(stderr, "Falha ao alocar mapa.\n");
        return 1;
    }
    inicializarTerritorios(mapa, tamanho);

    // Vetor de missões pré-definidas
    char* missoes[] = {
        "Conquistar 2 territorios inimigos em qualquer ordem",
        "Conquistar 3 territorios seguidos (no estado atual do mapa)",
        "Eliminar todas as tropas da cor Vermelha",
        "Possuir ao menos 4 territorios ao mesmo tempo",
        "Ter no minimo 15 tropas somadas em seus territorios"
    };
    int totalMissoes = (int)(sizeof(missoes) / sizeof(missoes[0]));

    // Missão do jogador (alocada dinamicamente e manipulada via ponteiro)
    char* missaoJogador = (char*)malloc(MAX_MISSAO * sizeof(char));
    if (!missaoJogador) {
        fprintf(stderr, "Falha ao alocar memoria para a missao.\n");
        liberarMemoria(mapa, NULL);
        return 1;
    }
    atribuirMissao(missaoJogador, missoes, totalMissoes);

    printf("\n===== BEM-VINDO AO WAR ESTRUTURADO =====\n");
    printf("Sua cor: Azul (inimigo: Vermelha)\n");
    printf("SUA MISSAO (exibida apenas uma vez):\n");
    exibirMissao(missaoJogador);

    // 2. Laço Principal do Jogo (Game Loop):
    int opcao;
    int venceu = 0;
    do {
        exibirMapa(mapa, tamanho);
        exibirMenuPrincipal();

        printf("Escolha: ");
        if (scanf("%d", &opcao) != 1) {
            limparBufferEntrada();
            printf("Entrada invalida.\n");
            continue;
        }

        switch (opcao) {
            case 1: {
                // Fase de ataque (interface simples)
                int iAtac, iDef;
                printf("Indice do territorio ATACANTE (0..%d): ", tamanho - 1);
                if (scanf("%d", &iAtac) != 1) { limparBufferEntrada(); break; }
                printf("Indice do territorio DEFENSOR (0..%d): ", tamanho - 1);
                if (scanf("%d", &iDef) != 1) { limparBufferEntrada(); break; }

                if (iAtac < 0 || iAtac >= tamanho || iDef < 0 || iDef >= tamanho) {
                    printf("Indices invalidos.\n");
                    break;
                }

                // Validar que só ataca inimigo
                if (strcmp(mapa[iAtac].cor, mapa[iDef].cor) == 0) {
                    printf("Ataque invalido: voce so pode atacar territorios inimigos.\n");
                    break;
                }
                // Validar que atacante tem tropas suficientes
                if (mapa[iAtac].tropas <= 1) {
                    printf("Ataque invalido: territorio atacante precisa ter ao menos 2 tropas.\n");
                    break;
                }

                atacar(&mapa[iAtac], &mapa[iDef]);
                break;
            }
            case 2: {
                // Verificação manual (feedback)
                int ok = verificarMissao(missaoJogador, mapa, tamanho);
                if (ok) {
                    printf("\n>>> MISSAO CUMPRIDA! Voce venceu!\n");
                    venceu = 1;
                } else {
                    printf("\nAinda nao cumpriu a missao. Continue jogando!\n");
                }
                break;
            }
            case 0:
                printf("Encerrando o jogo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
                break;
        }

        // Verificação silenciosa ao fim do turno
        if (!venceu && opcao != 0) {
            if (verificarMissao(missaoJogador, mapa, tamanho)) {
                printf("\n>>> MISSAO CUMPRIDA (verificacao automatica ao fim do turno)!\n");
                printf("Voce venceu o jogo!\n");
                venceu = 1;
            }
        }

        if (!venceu && opcao != 0) {
            printf("\nPressione ENTER para continuar...");
            limparBufferEntrada();
            getchar();
        }

    } while (opcao != 0 && !venceu);

    // 3. Limpeza:
    liberarMemoria(mapa, missaoJogador);
    printf("Memoria liberada. Ate a proxima!\n");
    return 0;
}

// --- Implementação das Funções ---

// Aloca dinamicamente o vetor de territorios
Territorio* alocarMapa(int tamanho) {
    return (Territorio*)calloc(tamanho, sizeof(Territorio));
}

// Preenche os dados iniciais (nome, cor, tropas)
void inicializarTerritorios(Territorio* mapa, int tamanho) {
    // Exemplo simples: metade Azul (jogador), metade Vermelha (inimigo)
    const char* nomes[MAX_TERRITORIOS] = {
        "Norte", "Sul", "Leste", "Oeste", "Centro", "Fortaleza"
    };

    for (int i = 0; i < tamanho; i++) {
        strncpy(mapa[i].nome, nomes[i % MAX_TERRITORIOS], MAX_NOME - 1);
        mapa[i].nome[MAX_NOME - 1] = '\0';

        if (i < tamanho / 2) {
            strncpy(mapa[i].cor, "Azul", MAX_COR - 1);
            mapa[i].tropas = 5; // jogador começa razoavelmente forte em metade
        } else {
            strncpy(mapa[i].cor, "Vermelha", MAX_COR - 1);
            mapa[i].tropas = 4;
        }
        mapa[i].cor[MAX_COR - 1] = '\0';
    }
}

// Libera memoria alocada (mapa e missao)
void liberarMemoria(Territorio* mapa, char* missaoJogador) {
    if (mapa) free(mapa);
    if (missaoJogador) free(missaoJogador);
}

// Mostra o menu principal
void exibirMenuPrincipal(void) {
    printf("\n===== MENU =====\n");
    printf("1 - Atacar\n");
    printf("2 - Verificar missao agora\n");
    printf("0 - Sair\n");
}

// Exibe o mapa como tabela (somente leitura com const)
void exibirMapa(const Territorio* mapa, int tamanho) {
    printf("\n----- MAPA ATUAL -----\n");
    printf("%-3s | %-12s | %-9s | %-6s\n", "ID", "Territorio", "Cor", "Tropas");
    printf("----+--------------+-----------+--------\n");
    for (int i = 0; i < tamanho; i++) {
        printf("%-3d | %-12s | %-9s | %-6d\n",
               i, mapa[i].nome, mapa[i].cor, mapa[i].tropas);
    }
}

// Exibe a missao do jogador
void exibirMissao(const char* missao) {
    printf(">> %s\n\n", missao);
}

// Rolagem de dados e resolução de combate
void atacar(Territorio* atacante, Territorio* defensor) {
    // Validações principais já foram feitas externamente
    int dadoAtk = (rand() % 6) + 1; // 1..6
    int dadoDef = (rand() % 6) + 1; // 1..6

    printf("\nATAQUE: %s(%s, %d) -> %s(%s, %d)\n",
           atacante->nome, atacante->cor, atacante->tropas,
           defensor->nome, defensor->cor, defensor->tropas);
    printf("Dados: Atacante=%d, Defensor=%d\n", dadoAtk, dadoDef);

    if (dadoAtk > dadoDef) {
        // Vitória do atacante: transfere cor e metade das tropas do atacante para o defensor
        // Regra: ao conquistar, move metade (arredondando para baixo) para ocupar
        int mover = atacante->tropas / 2;
        if (mover < 1) mover = 1; // garante ao menos 1
        atacante->tropas -= mover;
        defensor->tropas = mover; // ocupa com as tropas movidas
        strncpy(defensor->cor, atacante->cor, MAX_COR - 1);
        defensor->cor[MAX_COR - 1] = '\0';

        printf("Resultado: VITORIA do atacante! %s agora e %s com %d tropas.\n",
               defensor->nome, defensor->cor, defensor->tropas);
    } else {
        // Vitória do defensor: atacante perde 1 tropa
        atacante->tropas -= 1;
        if (atacante->tropas < 1) atacante->tropas = 1; // mantem mínimo para não zerar
        printf("Resultado: DEFESA bem-sucedida! Atacante perdeu 1 tropa (agora %d).\n",
               atacante->tropas);
    }
}

// Função exigida: sorteia e copia a missão escolhida para 'destino' usando strcpy
void atribuirMissao(char* destino, char* missoes[], int totalMissoes) {
    int idx = sortearMissaoIndex(totalMissoes);
    // ATENÇÃO: Assumimos que 'destino' tem espaço suficiente (MAX_MISSAO)
    strncpy(destino, missoes[idx], MAX_MISSAO - 1);
    destino[MAX_MISSAO - 1] = '\0';
}

// Sorteia um índice de missão
int sortearMissaoIndex(int totalMissoes) {
    if (totalMissoes <= 0) return 0;
    return rand() % totalMissoes;
}

// Verifica se a missão foi cumprida (lógica simples inicial)
// Retorna 1 se cumprida, 0 caso contrário.
// Observação: 'missao' por referência permite (futuro) marcar estado, logs etc.
int verificarMissao(char* missao, Territorio* mapa, int tamanho) {
    // 1) "Conquistar 2 territorios inimigos em qualquer ordem"
    if (strstr(missao, "Conquistar 2 territorios inimigos")) {
        int conquistados = 0;
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, "Azul") == 0) conquistados++;
        }
        // No início temos 3 azuis; cumprir exige ter pelo menos 5 (conquistou 2 do inimigo)
        return conquistados >= 5;
    }

    // 2) "Conquistar 3 territorios seguidos (no estado atual do mapa)"
    // Como não temos adjacencia definida, interpretamos como "ter 3 conquistas a mais que o estado inicial".
    if (strstr(missao, "Conquistar 3 territorios seguidos")) {
        int azuis = 0;
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, "Azul") == 0) azuis++;
        // Começou com 3 azuis; precisa chegar a 6 (max), ou ao menos +3 => 6
        return azuis >= 6;
    }

    // 3) "Eliminar todas as tropas da cor Vermelha"
    if (strstr(missao, "Eliminar todas as tropas da cor Vermelha")) {
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, "Vermelha") == 0 && mapa[i].tropas > 0) {
                return 0;
            }
        }
        return 1;
    }

    // 4) "Possuir ao menos 4 territorios ao mesmo tempo"
    if (strstr(missao, "Possuir ao menos 4 territorios")) {
        int azuis = 0;
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, "Azul") == 0) azuis++;
        return azuis >= 4;
    }

    // 5) "Ter no minimo 15 tropas somadas em seus territorios"
    if (strstr(missao, "Ter no minimo 15 tropas")) {
        int soma = 0;
        for (int i = 0; i < tamanho; i++)
            if (strcmp(mapa[i].cor, "Azul") == 0) soma += mapa[i].tropas;
        return soma >= 15;
    }

    // Caso padrão (não reconhecida)
    return 0;
}

// Limpa o buffer de entrada (stdin) para evitar problemas após scanf
void limparBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
}
