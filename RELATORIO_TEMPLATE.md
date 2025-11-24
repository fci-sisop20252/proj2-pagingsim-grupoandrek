# Relatório do Projeto 2: Simulador de Memória Virtual

**Disciplina:** Sistemas Operacionais
**Professor:** Lucas Figueiredo
**Data:**

## Integrantes do Grupo

- Andre kim - 10226152
- Nome Completo - Matrícula
- Nome Completo - Matrícula
- Nome Completo - Matrícula

---

## 1. Instruções de Compilação e Execução

### 1.1 Compilação

Descreva EXATAMENTE como compilar seu projeto. Inclua todos os comandos necessários.

Para compilar o simulador, basta usar o compilador `gcc` diretamente no arquivo
```markdown
Para executar o simulador, passamos 3 argumentos na linha de comando:

```bash
./simulador <algoritmo> <arquivo_config> <arquivo_acessos>
### 1.2 Execução

Forneça exemplos completos de como executar o simulador.

**Exemplo com FIFO:**
```bash
./simulador fifo tests/config_1.txt tests/acessos_1.txt
```

**Exemplo com Clock:**
```bash
./simulador clock tests/config_1.txt tests/acessos_1.txt
```

---

## 2. Decisões de Design

### 2.1 Estruturas de Dados

Descreva as estruturas de dados que você escolheu para representar:

**Tabela de Páginas:**

No meu simulador eu não mantenho uma tabela de páginas completa separada
para cada processo. Em vez disso, represento a informação de quais páginas
estão na memória diretamente na estrutura de frames físicos.

Quando preciso saber se uma página (PID, número da página) está na memória,
eu percorro o vetor de frames e procuro um frame que tenha exatamente
aquele `pid` e aquele número de página. Se encontrar, considero HIT;
se não encontrar, é PAGE FAULT.

- Estrutura utilizada: busca no vetor de frames pelo par (pid, página).
- Informações para cada página: PID, número da página, R-bit e a posição
  (índice) do frame em que ela está carregada.
- Organização para múltiplos processos: o PID faz parte da chave
  (PID, página), então páginas de processos diferentes nunca “se misturam”.

**Justificativa:**  
Essa abordagem simplifica a implementação, porque não preciso manter uma
estrutura extra por processo. Como o número de frames é relativamente
pequeno, percorrer o vetor de frames para achar uma página é suficiente
para este simulador didático.

---

**Frames Físicos:**

Os frames físicos da memória são representados por um vetor de structs
`Frame`, de tamanho `num_frames`. Cada posição do vetor representa um frame
da RAM.

Para cada frame eu armazeno:

- `pid` do processo que está usando aquele frame;
- `page` (número da página virtual do processo);
- `R` (referenced bit), indicando se a página foi acessada recentemente;
- um campo/flag indicando se o frame está ocupado ou livre.

Também mantenho um contador `frames_usados` para saber quantos frames já
foram ocupados. Enquanto `frames_usados < num_frames`, ainda existem
frames livres.

**Justificativa:**  
Usar um vetor fixo de frames torna o acesso simples (índices 0..num_frames-1)
e deixa fácil implementar tanto o FIFO quanto o Clock, que só precisam
percorrer esse vetor.

---

**Estrutura para FIFO:**

Para o algoritmo FIFO, usei apenas um índice inteiro `fifo_index`, que indica
qual frame será o próximo a ser substituído.
- A ordem de chegada é mantida implicitamente: sempre que a memória está cheia
  e ocorre um PAGE FAULT, a página vítima é o frame apontado por `fifo_index`.
- Depois da substituição, faço:


fifo_index = (fifo_index + 1) % num_frames;


**Tabela de Páginas:**
- Estrutura utilizada: busca no vetor de frames pelo par (pid, página).
- Informações para cada página: PID, número da página, R-bit e a posição
  (índice) do frame em que ela está carregada.
- Organização para múltiplos processos: o PID faz parte da chave
  (PID, página), então páginas de processos diferentes nunca “se misturam”.
Essa abordagem simplifica a implementação, porque não preciso manter uma
estrutura extra por processo. Como o número de frames é relativamente
pequeno, percorrer o vetor de frames para achar uma página é suficiente
para este simulador didático.

**Frames Físicos:**
Os frames físicos da memória são representados por um vetor de structs
`Frame`, de tamanho `num_frames`. Cada posição do vetor representa um frame
da RAM.

Para cada frame eu armazeno:

- `pid` do processo que está usando aquele frame;
- `page` (número da página virtual do processo);
- `R` (referenced bit), indicando se a página foi acessada recentemente;
- um campo/flag indicando se o frame está ocupado ou livre.

Também mantenho um contador `frames_usados` para saber quantos frames já
foram ocupados. Enquanto `frames_usados < num_frames`, ainda existem
frames livres.
Usar um vetor fixo de frames torna o acesso simples (índices 0..num_frames-1)
e deixa fácil implementar tanto o FIFO quanto o Clock, que só precisam
percorrer esse vetor.

**Estrutura para FIFO:**
Para o algoritmo FIFO, usei apenas um índice inteiro `fifo_index`, que indica
qual frame será o próximo a ser substituído.

- A ordem de chegada é mantida implicitamente: sempre que a memória está cheia
  e ocorre um PAGE FAULT, a página vítima é o frame apontado por `fifo_index`.
- Depois da substituição, faço:


fifo_index = (fifo_index + 1) % num_frames;
É a forma mais simples de implementar FIFO: não preciso de uma fila
separada, apenas de um índice que anda circularmente pelo vetor de frames.

**Estrutura para Clock:**
Para o algoritmo Clock, usei um índice inteiro clock_hand que funciona como
o ponteiro do “relógio”.

O clock_hand percorre o vetor de frames de forma circular
((clock_hand + 1) % num_frames).

Em cada posição, olho para o R-bit:

se R == 1, dou “segunda chance”: coloco R = 0 e avanço o ponteiro;

se R == 0, escolho aquele frame como vítima.
Essa estrutura é uma extensão natural do vetor de frames. O Clock precisa
apenas de um ponteiro e do R-bit em cada frame, mantendo a implementação
simples e ao mesmo tempo aproximando o comportamento de um algoritmo LRU.

### 2.2 Organização do Código

Descreva como organizou seu código:

- Quantos arquivos/módulos criou?
- Qual a responsabilidade de cada arquivo/módulo?
- Quais são as principais funções e o que cada uma faz?

**Exemplo:**
Todo o código do simulador está em um único arquivo: `simulador.c`.

A função `main` é responsável por:

- ler os argumentos da linha de comando (`algoritmo`, `arquivo_config`,
  `arquivo_acessos`);
- carregar a configuração (número de frames, tamanho da página, processos);
- inicializar o vetor de frames;
- abrir o arquivo de acessos e executar o loop principal da simulação;
- no final, imprimir o resumo com total de acessos e total de page faults.

Além da `main`, organizei o código em funções auxiliares que separam
responsabilidades, por exemplo:

- uma função para ler o arquivo de configuração e preencher as variáveis
  globais/estruturas necessárias;
- uma função para traduzir o endereço virtual em (página, deslocamento);
- uma função que verifica se uma página já está carregada em algum frame
  (HIT) ou não (PAGE FAULT);
- funções específicas para escolher a página vítima com FIFO ou com Clock;
- uma função que imprime as mensagens de saída no formato exigido.

Essa organização ajuda a manter o código mais legível e facilita testar
separadamente cada parte da lógica do simulador.

### 2.3 Algoritmo FIFO

Todo o código do simulador está em um único arquivo: `simulador.c`.

A função `main` é responsável por:

- ler os argumentos da linha de comando (`algoritmo`, `arquivo_config`,
  `arquivo_acessos`);
- carregar a configuração (número de frames, tamanho da página, processos);
- inicializar o vetor de frames;
- abrir o arquivo de acessos e executar o loop principal da simulação;
- no final, imprimir o resumo com total de acessos e total de page faults.

Além da `main`, organizei o código em funções auxiliares que separam
responsabilidades, por exemplo:

- uma função para ler o arquivo de configuração e preencher as variáveis
  globais/estruturas necessárias;
- uma função para traduzir o endereço virtual em (página, deslocamento);
- uma função que verifica se uma página já está carregada em algum frame
  (HIT) ou não (PAGE FAULT);
- funções específicas para escolher a página vítima com FIFO ou com Clock;
- uma função que imprime as mensagens de saída no formato exigido.

Essa organização ajuda a manter o código mais legível e facilita testar
separadamente cada parte da lógica do simulador.

### 2.4 Algoritmo Clock
1. Quando ocorre um PAGE FAULT e a memória está cheia, preciso escolher
   uma página vítima. Para isso:

   - começo no frame indicado por `clock_hand`;
   - se o frame estiver ocupado e `R == 1`:
     - dou “segunda chance”: coloco `R = 0`;
     - avanço o ponteiro: `clock_hand = (clock_hand + 1) % num_frames`;
   - se `R == 0`, encontro a vítima:
     - removo a página que estava nesse frame;
     - coloco a nova página nesse mesmo frame;
     - seto `R = 1` para a nova página;
     - avanço `clock_hand` para o próximo frame da roda.

2. Se todas as páginas estiverem com `R == 1`, o ponteiro pode dar uma volta
   completa zerando os R-bits. Na segunda volta, vai encontrar alguma página
   com `R == 0` e poderá removê-la.

3. Em todo acesso (HIT ou PAGE FAULT), o R-bit da página acessada é
   setado para 1. Assim o Clock sempre sabe quais páginas foram usadas
   recentemente.

Esse comportamento dá “segunda chance” às páginas recentemente acessadas
e aproxima a ideia de um algoritmo LRU, porém com implementação bem mais simples.


### 2.5 Tratamento de Page Fault

Explique como seu código distingue e trata os dois cenários:

**Cenário 1: Frame livre disponível**
- Eu mantenho um contador `frames_usados`.  
- Se `frames_usados < num_frames`, significa que ainda existe pelo menos
  um frame livre.
- Nesse caso:
  1. escolho o próximo índice livre (normalmente `frames_usados`);
  2. preencho o frame com o `pid` e a `page` que causaram o page fault;
  3. marco o frame como ocupado e seto `R = 1`;
  4. incremento `frames_usados`;
  5. imprimo a mensagem de PAGE FAULT com “frame livre”.

Nenhum algoritmo de substituição é usado aqui, pois ainda há espaço na memória.

**Cenário 2: Memória cheia (substituição)**
- Quando `frames_usados == num_frames`, considero que a memória está cheia.
- Se ocorre um PAGE FAULT nessa situação, preciso chamar o algoritmo de
  substituição:
  - se o usuário escolheu `fifo`, chamo a lógica do FIFO;
  - se escolheu `clock`, chamo a lógica do Clock.

Os passos gerais são:

1. O algoritmo seleciona o índice do frame vítima (`victim_index`),
   de acordo com a política (FIFO ou Clock).
2. Imprimo a mensagem informando qual página (PID antigo e número da página)
   será removida e qual frame está sendo liberado.
3. Sobrescrevo aquele frame com a nova página:
   - atualizo `pid` e `page`;
   - seto `R = 1` para a nova página.
4. O algoritmo atualiza seu estado interno
   (`fifo_index` ou `clock_hand`) para a próxima substituição futura.

---

## 3. Análise Comparativa FIFO vs Clock

### 3.1 Resultados dos Testes

Preencha a tabela abaixo com os resultados de pelo menos 3 testes diferentes:

| Descrição do Teste | Total de Acessos | Page Faults FIFO | Page Faults Clock | Diferença |
|-------------------|------------------|------------------|-------------------|-----------|
| Teste 1 - Básico  |          8       |      5           |        5          |    0      |
| Teste 2 - Memória Pequena | 10       |      10          |       10          |    0      |
| Teste 3 - Simples |         7        |      4           |        4          |    0      |
| Teste Próprio 1   |       100        |      39          |       38          |    1      |

### 3.2 Análise

Com base nos resultados acima, responda:

1. **Qual algoritmo teve melhor desempenho (menos page faults)?**
Nos três primeiros testes (Básico, Memória Pequena e Simples), os dois
algoritmos tiveram exatamente o mesmo número de page faults.  
No Teste Próprio 1, o algoritmo Clock teve desempenho um pouco melhor:
foram 39 page faults com FIFO e 38 com Clock.

2. **Por que você acha que isso aconteceu?**
Nos primeiros testes, o padrão de acessos é simples e bastante sequencial.
As páginas são usadas poucas vezes e em ordem, então dar “segunda chance”
com o R-bit não traz nenhum benefício extra e o Clock acaba se
comportando muito parecido com o FIFO.

No Teste Próprio 1, a sequência de acessos reaproveita algumas páginas
depois de um certo tempo. Nessa situação, o Clock consegue preservar mais
tempo na memória as páginas que foram acessadas recentemente (R=1),
enquanto o FIFO remove páginas apenas pela ordem de chegada, sem olhar
se elas ainda estão sendo usadas. Por isso o Clock acabou tendo 1 page
fault a menos.


3. **Em que situações Clock é melhor que FIFO?**
   O Clock tende a ser melhor que o FIFO em cenários onde existe reuso de
páginas: por exemplo, quando o programa alterna entre alguns conjuntos
de páginas e volta a acessar páginas antigas após algum tempo. Nesses
casos, o R-bit indica que certas páginas ainda estão “quentes” (foram
usadas recentemente) e o Clock evita removê-las, escolhendo como vítima
páginas que não foram acessadas há mais tempo.

4. **Houve casos onde FIFO e Clock tiveram o mesmo resultado?**
  Sim, nos testes 1, 2 e 3 os dois algoritmos tiveram o mesmo número de
page faults. Isso aconteceu porque os acessos são simples, com pouca
reutilização de páginas, então a escolha baseada na ordem de chegada
(FIFO) e a escolha usando o R-bit (Clock) acabam removendo praticamente
as mesmas páginas, na mesma ordem.

5. **Qual algoritmo você escolheria para um sistema real e por quê?**
Para um sistema real eu escolheria o algoritmo Clock. Ele ainda é bem
simples de implementar (basta manter um ponteiro circular e o R-bit em
cada frame), mas leva em conta o histórico recente de acessos, o que
reduz page faults em muitos cenários práticos. O FIFO é mais simples,
mas ignora completamente o padrão de uso da memória e pode remover
páginas que ainda estão sendo usadas com frequência.
---

## 4. Desafios e Aprendizados

### 4.1 Maior Desafio Técnico

Descreva o maior desafio técnico que seu grupo enfrentou durante a implementação:

O maior desafio técnico foi organizar corretamente a lógica de page fault
quando a memória está cheia, principalmente no algoritmo Clock. No começo
foi fácil se confundir com o ponteiro circular (`clock_hand`) e com o
momento certo de zerar o R-bit e escolher a vítima.

Outra dificuldade foi garantir que o formato da saída estivesse exatamente
igual ao especificado no enunciado (mensagens de HIT, PAGE FAULT com frame
livre, PAGE FAULT com substituição). Para resolver isso, utilizei bastante
os arquivos `expected_*` da pasta `tests/` e o comando `diff` para comparar
a saída do meu programa com a saída esperada, ajustando os detalhes de
texto até ficarem idênticos.

Com isso aprendi a ter mais cuidado com detalhes de especificação e a
depurar passo a passo o comportamento de um algoritmo mais complexo.

### 4.2 Principal Aprendizado

O principal aprendizado foi entender na prática como funciona o
gerenciamento de memória com paginação. Antes eu via só na teoria os
conceitos de páginas, frames, page faults e algoritmos de substituição,
mas não tinha uma visão clara de como tudo isso se encaixava.

Implementando o simulador, ficou mais fácil enxergar:

- como um endereço virtual é traduzido em (página, deslocamento);
- como o sistema sabe se uma página está ou não na memória física;
- o impacto dos page faults no desempenho;
- e como diferentes algoritmos (FIFO e Clock) tomam decisões diferentes
  na hora de escolher a página vítima.

Depois desse projeto, o conceito de memória virtual e de algoritmos de
substituição de páginas ficou muito mais concreto.

---

## 5. Vídeo de Demonstração

**Link do vídeo:** [Insira aqui o link para YouTube, Google Drive, etc.]

### Conteúdo do vídeo:

Confirme que o vídeo contém:

- [ ] Demonstração da compilação do projeto
- [ ] Execução do simulador com algoritmo FIFO
- [ ] Execução do simulador com algoritmo Clock
- [ ] Explicação da saída produzida
- [ ] Comparação dos resultados FIFO vs Clock
- [ ] Breve explicação de uma decisão de design importante

---

## Checklist de Entrega

Antes de submeter, verifique:

- [ ] Código compila sem erros conforme instruções da seção 1.1
- [ ] Simulador funciona corretamente com FIFO
- [ ] Simulador funciona corretamente com Clock
- [ ] Formato de saída segue EXATAMENTE a especificação do ENUNCIADO.md
- [ ] Testamos com os casos fornecidos em tests/
- [ ] Todas as seções deste relatório foram preenchidas
- [ ] Análise comparativa foi realizada com dados reais
- [ ] Vídeo de demonstração foi gravado e link está funcionando
- [ ] Todos os integrantes participaram e concordam com a submissão

---
## Referências
Liste aqui quaisquer referências que utilizaram para auxiliar na implementação (livros, artigos, sites, **links para conversas com IAs.**)


---

## Comentários Finais

Use este espaço para quaisquer observações adicionais que julguem relevantes (opcional).

---
