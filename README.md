# ProcessamentoImagens

## Integrantes
Turma 07P


Danilo Yui Honda - 10419625
Luis Felipe Basacchi Darre - 10419477
Vitor Tibães Santos - 10418976


Prof. André Kishimoto.

## O que é o projeto
Este projeto consiste em um software de processamento de imagens desenvolvido na linguagem C. O objetivo principal é carregar imagens em diversos formatos, convertê-las para escala de cinza e aplicar técnicas de equalização de histograma para melhorar o contraste. O software utiliza a biblioteca gráfica SDL3 (Simple DirectMedia Layer) para renderizar a interface e exibir os resultados em tempo real.

## Como o projeto funciona
O programa é executado via linha de comando e processa a imagem fornecida seguindo um fluxo de funcionalidades obrigatórias:

1. **Carregamento:** Lê arquivos de imagem (PNG, JPG, BMP) utilizando a biblioteca `SDL_image`. Se o arquivo for inválido ou não for encontrado, o programa encerra de forma segura e exibe o erro.
2. **Conversão para Escala de Cinza:** Analisa se a imagem é colorida e, caso seja, converte os pixels aplicando a fórmula de luminância: Y = 0.2125*R + 0.7154*G + 0.0721*B.
3. **Interface Gráfica (GUI):** Exibe duas janelas simultâneas. 
   * A janela principal adapta-se ao tamanho da imagem e exibe o resultado visual.
   * A janela secundária exibe os dados analíticos e os controles.
4. **Análise de Histograma:** Calcula o histograma da imagem e o renderiza em formato de gráfico de barras. Utilizando `SDL_ttf`, exibe a classificação da imagem com base na média de intensidade (Clara, Média, Escura) e no desvio padrão (Contraste Alto, Médio, Baixo).
5. **Equalização:** Um botão interativo na janela secundária permite aplicar a equalização do histograma (melhorando o contraste global) ou reverter para a versão original em escala de cinza. O estado visual do botão responde à interação do mouse.
6. **Salvamento:** Ao pressionar a tecla `S`, a imagem que está visível na janela principal é salva e exportada como `output_image.png`.
## Requisitos e Dependências
Compilador: GCC (compatível com padrão C99 ou superior).
Bibliotecas: SDL3, SDL3_image e SDL3_ttf.
Recursos de Fonte: É necessário o arquivo arial.ttf no diretório de execução.

## Compilação (Ambiente MSYS2/UCRT64)
Para compilar o projeto localmente, utilize o seguinte comando no terminal:
bash
```
gcc main.c -o programa -lSDL3 -lSDL3_image -lSDL3_ttf -lm
```

## Execução
Após a compilação, execute o programa passando o caminho de uma imagem como argumento:
Bash
```
./programa caminho_da_imagem.png
```

### Contribuições

Danilo: Responsável por estabelecer a estrutura inicial do código no main, configurar os subsistemas da biblioteca SDL3 e criar as instâncias das janelas (Principal e Secundária).

Luis: Desenvolveu a lógica de pré-processamento, implementando a conversão da imagem original para escala de cinza usando a fórmula de luminância exigida, além de estruturar a renderização base da imagem na tela.

Victor: Encarregado da implementação do histograma. Construiu o algoritmo que calcula a frequência dos tons de cinza e programou a renderização visual do gráfico de barras na janela secundária.
