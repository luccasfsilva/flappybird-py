"""Flappy Bird em Python + Pygame.

Controles:
    ESPAÇO / SETA PARA CIMA / CLIQUE DO MOUSE -> pular
    R                                         -> reiniciar (após perder)
    ESC                                       -> sair
"""
import asyncio
import random
import sys
from pathlib import Path

import pygame

# ----------------------------- Configurações ------------------------------
WIDTH, HEIGHT = 400, 600
FPS = 60
GROUND_HEIGHT = 80

GRAVITY = 0.45
FLAP_STRENGTH = -7.5
MAX_FALL_SPEED = 10

PIPE_WIDTH = 64
PIPE_GAP = 150
PIPE_SPEED = 3
PIPE_SPACING = 230  # distância horizontal entre canos
PIPE_MIN_TOP = 60

SKY = (113, 197, 207)
CLOUD = (240, 250, 250)
GROUND_TOP = (222, 216, 149)
GROUND_DARK = (84, 56, 71)
PIPE_GREEN = (115, 191, 46)
PIPE_DARK = (84, 56, 71)
BIRD_YELLOW = (250, 215, 50)
WHITE = (255, 255, 255)
BLACK = (30, 30, 30)

HIGHSCORE_FILE = Path(__file__).with_name("highscore.txt")


def load_highscore() -> int:
    try:
        return int(HIGHSCORE_FILE.read_text().strip())
    except (OSError, ValueError):
        return 0


def save_highscore(score: int) -> None:
    try:
        HIGHSCORE_FILE.write_text(str(score))
    except OSError:
        pass  # sem permissão de escrita: apenas ignora


# --------------------------------- Pássaro --------------------------------
class Bird:
    X = 90
    RADIUS = 13

    def __init__(self):
        self.y = HEIGHT / 2 - 40
        self.vel = 0.0
        self.sprite = self._make_sprite()

    @staticmethod
    def _make_sprite() -> pygame.Surface:
        surf = pygame.Surface((40, 32), pygame.SRCALPHA)
        pygame.draw.ellipse(surf, BLACK, (2, 2, 32, 28))
        pygame.draw.ellipse(surf, BIRD_YELLOW, (4, 4, 28, 24))
        pygame.draw.ellipse(surf, (240, 240, 240), (4, 14, 14, 10))  # asa
        pygame.draw.circle(surf, WHITE, (25, 12), 6)  # olho
        pygame.draw.circle(surf, BLACK, (27, 12), 3)
        pygame.draw.polygon(surf, (240, 100, 40), [(30, 18), (39, 20), (30, 24)])  # bico
        return surf

    def flap(self):
        self.vel = FLAP_STRENGTH

    def update(self):
        self.vel = min(self.vel + GRAVITY, MAX_FALL_SPEED)
        self.y += self.vel

    @property
    def hitbox(self) -> pygame.Rect:
        r = self.RADIUS
        return pygame.Rect(self.X - r + 2, int(self.y) - r + 2, 2 * r - 4, 2 * r - 4)

    def draw(self, screen: pygame.Surface):
        angle = -min(self.vel * 4, 70) if self.vel > 0 else 25
        rotated = pygame.transform.rotate(self.sprite, angle)
        screen.blit(rotated, rotated.get_rect(center=(self.X, int(self.y))))


# ---------------------------------- Canos ---------------------------------
class Pipe:
    def __init__(self, x: int):
        self.x = float(x)
        max_top = HEIGHT - GROUND_HEIGHT - PIPE_GAP - PIPE_MIN_TOP
        self.top_height = random.randint(PIPE_MIN_TOP, max_top)
        self.scored = False

    @property
    def rects(self):
        top = pygame.Rect(int(self.x), 0, PIPE_WIDTH, self.top_height)
        bottom_y = self.top_height + PIPE_GAP
        bottom = pygame.Rect(
            int(self.x), bottom_y, PIPE_WIDTH, HEIGHT - GROUND_HEIGHT - bottom_y
        )
        return top, bottom

    def update(self):
        self.x -= PIPE_SPEED

    def off_screen(self) -> bool:
        return self.x + PIPE_WIDTH < 0

    def collides(self, hitbox: pygame.Rect) -> bool:
        return any(hitbox.colliderect(r) for r in self.rects)

    def draw(self, screen: pygame.Surface):
        top, bottom = self.rects
        for rect, cap_y in ((top, top.bottom - 24), (bottom, bottom.top)):
            pygame.draw.rect(screen, PIPE_GREEN, rect)
            pygame.draw.rect(screen, PIPE_DARK, rect, 3)
            cap = pygame.Rect(rect.x - 4, cap_y, PIPE_WIDTH + 8, 24)
            pygame.draw.rect(screen, PIPE_GREEN, cap)
            pygame.draw.rect(screen, PIPE_DARK, cap, 3)


# ---------------------------------- Jogo ----------------------------------
class Game:
    MENU, PLAYING, GAME_OVER = "menu", "playing", "game_over"

    def __init__(self):
        pygame.init()
        pygame.display.set_caption("Flappy Bird")
        self.screen = pygame.display.set_mode((WIDTH, HEIGHT))
        self.clock = pygame.time.Clock()
        self.font_big = pygame.font.Font(None, 72)
        self.font_med = pygame.font.Font(None, 38)
        self.font_small = pygame.font.Font(None, 24)
        self.highscore = load_highscore()
        self.ground_offset = 0
        self.clouds = [
            (random.randint(0, WIDTH), random.randint(40, 300), random.randint(30, 55))
            for _ in range(5)
        ]
        self.reset()

    def reset(self):
        self.bird = Bird()
        self.pipes = [Pipe(WIDTH + 100)]
        self.score = 0
        self.state = self.MENU

    # ------------------------------ Eventos ------------------------------
    def flap_or_start(self):
        if self.state == self.MENU:
            self.state = self.PLAYING
            self.bird.flap()
        elif self.state == self.PLAYING:
            self.bird.flap()

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.quit()
                elif event.key in (pygame.K_SPACE, pygame.K_UP):
                    self.flap_or_start()
                elif event.key == pygame.K_r and self.state == self.GAME_OVER:
                    self.reset()
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                if self.state == self.GAME_OVER:
                    self.reset()
                else:
                    self.flap_or_start()

    # ----------------------------- Atualização ----------------------------
    def game_over(self):
        self.state = self.GAME_OVER
        if self.score > self.highscore:
            self.highscore = self.score
            save_highscore(self.highscore)

    def update(self):
        if self.state == self.MENU:
            # balanço do pássaro parado
            self.bird.y = HEIGHT / 2 - 40 + 6 * (
                1 if (pygame.time.get_ticks() // 300) % 2 else -1
            )
            self.scroll_ground()
            return

        if self.state == self.GAME_OVER:
            # o pássaro cai até o chão
            ground_y = HEIGHT - GROUND_HEIGHT - Bird.RADIUS
            if self.bird.y < ground_y:
                self.bird.update()
                self.bird.y = min(self.bird.y, ground_y)
            return

        self.scroll_ground()
        self.bird.update()

        for pipe in self.pipes:
            pipe.update()
            if not pipe.scored and pipe.x + PIPE_WIDTH < Bird.X:
                pipe.scored = True
                self.score += 1
            if pipe.collides(self.bird.hitbox):
                self.game_over()
                return

        if self.pipes[-1].x < WIDTH - PIPE_SPACING:
            self.pipes.append(Pipe(WIDTH))
        self.pipes = [p for p in self.pipes if not p.off_screen()]

        hit_ground = self.bird.y + Bird.RADIUS >= HEIGHT - GROUND_HEIGHT
        hit_ceiling = self.bird.y - Bird.RADIUS < -40
        if hit_ground or hit_ceiling:
            self.bird.y = min(self.bird.y, HEIGHT - GROUND_HEIGHT - Bird.RADIUS)
            self.game_over()

    def scroll_ground(self):
        self.ground_offset = (self.ground_offset - PIPE_SPEED) % 24
        self.clouds = [
            (x - 0.5 if x > -80 else WIDTH + 40, y, s) for x, y, s in self.clouds
        ]

    # ------------------------------ Desenho -------------------------------
    def draw_text(self, text, font, center, color=WHITE):
        shadow = font.render(text, True, BLACK)
        label = font.render(text, True, color)
        self.screen.blit(shadow, shadow.get_rect(center=(center[0] + 2, center[1] + 2)))
        self.screen.blit(label, label.get_rect(center=center))

    def draw_ground(self):
        y = HEIGHT - GROUND_HEIGHT
        pygame.draw.rect(self.screen, GROUND_TOP, (0, y, WIDTH, GROUND_HEIGHT))
        pygame.draw.rect(self.screen, PIPE_GREEN, (0, y, WIDTH, 14))
        pygame.draw.line(self.screen, GROUND_DARK, (0, y), (WIDTH, y), 3)
        for x in range(-24, WIDTH + 24, 24):
            pygame.draw.line(
                self.screen,
                (100, 160, 40),
                (x + self.ground_offset, y + 2),
                (x + self.ground_offset + 12, y + 12),
                6,
            )

    def draw(self):
        self.screen.fill(SKY)
        for x, y, s in self.clouds:
            for dx, dy, r in ((0, 0, s), (s * 0.7, 6, s * 0.7), (-s * 0.7, 8, s * 0.6)):
                pygame.draw.circle(self.screen, CLOUD, (int(x + dx), int(y + dy)), int(r))

        for pipe in self.pipes:
            pipe.draw(self.screen)
        self.draw_ground()
        self.bird.draw(self.screen)

        if self.state == self.MENU:
            self.draw_text("Flappy Bird", self.font_big, (WIDTH // 2, 150))
            self.draw_text("ESPAÇO ou clique para pular", self.font_small, (WIDTH // 2, 400))
            self.draw_text(f"Recorde: {self.highscore}", self.font_small, (WIDTH // 2, 430))
        elif self.state == self.PLAYING:
            self.draw_text(str(self.score), self.font_big, (WIDTH // 2, 70))
        else:
            self.draw_text("Game Over", self.font_big, (WIDTH // 2, 170))
            self.draw_text(f"Pontos: {self.score}", self.font_med, (WIDTH // 2, 250))
            self.draw_text(f"Recorde: {self.highscore}", self.font_med, (WIDTH // 2, 290))
            self.draw_text("Pressione R ou clique para reiniciar", self.font_small, (WIDTH // 2, 350))

        pygame.display.flip()

    # ------------------------------- Loop ---------------------------------
    def quit(self):
        pygame.quit()
        sys.exit()

    async def run(self):
        while True:
            self.handle_events()
            self.update()
            self.draw()
            self.clock.tick(FPS)
            await asyncio.sleep(0)  # cede o controle ao navegador (pygbag)


async def main():
    await Game().run()


if __name__ == "__main__":
    asyncio.run(main())
