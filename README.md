# ProcessamentoImagens

## Integrantes
Danilo Yui Honda - 10419625
Luis Felipe Basacchi Darre - 10419477
Vitor Tibães Santos - 10418976

## Descrição do Projeto
O objetivo deste software é realizar o processamento de imagens digitais utilizando a linguagem C e a biblioteca SDL3. O programa opera via linha de comando e oferece funcionalidades de conversão para escala de cinza, análise de histograma e equalização.

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