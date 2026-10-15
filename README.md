# 🐤 Flappy Bird em Python

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
git clone https://github.com/SEU-USUARIO/flappy-bird.git
cd flappy-bird

python -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate

pip install -r requirements.txt
python main.py
```

## Personalização

As constantes no topo do `main.py` controlam a dificuldade:
`GRAVITY`, `FLAP_STRENGTH`, `PIPE_GAP`, `PIPE_SPEED` e `PIPE_SPACING`.

## Licença

MIT
