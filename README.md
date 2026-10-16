# 🐤 Flappy Bird em Python

▶️ **Jogue online:** [versão Python (Pygbag)](https://luccasfsilva.github.io/flappybird-py/) · [versão JavaScript](https://luccasfsilva.github.io/flappybird-py/js/) · [versão C++](https://luccasfsilva.github.io/flappybird-py/cpp/)

Clone do clássico Flappy Bird feito com [Pygame](https://www.pygame.org/).
Não usa imagens nem sons externos: tudo é desenhado por código, então o projeto é um único arquivo.

## Como jogar

| Ação | Tecla |
|------|-------|
| Pular | `ESPAÇO`, `↑` ou clique esquerdo do mouse |
| Reiniciar (após perder) | `R` ou clique |
| Sair | `ESC` |

Passe pelos espaços entre os canos sem encostar neles, no chão ou no teto. O recorde é salvo localmente em `highscore.txt`.

## Instalação

Requer Python 3.8+.

```bash
git clone https://github.com/luccasfsilva/flappybird-py.git
cd flappybird-py

python -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate

pip install -r requirements.txt
python main.py
```

## Versão web (GitHub Pages)

O jogo é compilado para WebAssembly com [pygbag](https://github.com/pygame-web/pygbag) e publicado automaticamente a cada `git push` na branch `main`, pelo workflow em `.github/workflows/deploy.yml`.

Para ativar: **Settings → Pages → Build and deployment → Source: GitHub Actions**.

Para testar a versão web localmente:

```bash
pip install pygbag
pygbag .
# abra http://localhost:8000
```

## Versão JavaScript

Em `js/index.html` há a mesma lógica do jogo em JavaScript puro (HTML5 Canvas), sem dependências.
Para jogar localmente, basta abrir o arquivo no navegador. Ela é publicada em `/js/` junto com a versão Python.

## Versão C++

Em `cpp/main.cpp` há a mesma lógica em C++17 com [SDL2](https://www.libsdl.org/). Não usa imagens nem fontes externas (o texto usa uma fonte bitmap embutida, por isso só maiúsculas).

```bash
# Ubuntu/Debian: sudo apt install build-essential cmake libsdl2-dev
# macOS:         brew install cmake sdl2
# Windows:       use o vcpkg (vcpkg install sdl2) ou o MSYS2 (pacman -S mingw-w64-x86_64-SDL2)

cd cpp
cmake -B build
cmake --build build
./build/flappy            # Windows: build\Debug\flappy.exe ou build\flappy.exe
```

O mesmo código é compilado para WebAssembly com Emscripten pelo workflow e publicado em `/cpp/`.

## Personalização

As constantes no topo do `main.py` (ou `<script>` em `js/index.html`, ou `cpp/main.cpp`) controlam a dificuldade:
`GRAVITY`, `FLAP_STRENGTH`, `PIPE_GAP`, `PIPE_SPEED` e `PIPE_SPACING`.

## Licença

MIT
