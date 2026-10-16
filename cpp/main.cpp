// Flappy Bird em C++ + SDL2.
//
// Controles: ESPAÇO / SETA PARA CIMA / CLIQUE -> pular | R -> reiniciar | ESC -> sair
//
// Não usa imagens, fontes nem sons externos: o pássaro é desenhado por código
// e o texto usa uma fonte bitmap 5x7 embutida (por isso só há letras maiúsculas).
//
// Compila para desktop (CMake) e para o navegador (Emscripten).
#include <SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

// ----------------------------- Configurações ------------------------------
constexpr int W = 400, H = 600, GROUND_H = 80;
constexpr float GRAVITY = 0.45f, FLAP = -7.5f, MAX_FALL = 10.0f;
constexpr int PIPE_W = 64, PIPE_GAP = 150, PIPE_SPACING = 230, PIPE_MIN_TOP = 60;
constexpr float PIPE_SPEED = 3.0f;
constexpr float BIRD_X = 90.0f;
constexpr int BIRD_R = 13;
constexpr double STEP_MS = 1000.0 / 60.0;

struct Color { uint8_t r, g, b; };
constexpr Color SKY{113, 197, 207}, CLOUD{240, 250, 250}, GROUND_TOP{222, 216, 149},
    DARK{84, 56, 71}, GREEN{115, 191, 46}, STRIPE{100, 160, 40}, YELLOW{250, 215, 50},
    BLACK{30, 30, 30}, WHITE{255, 255, 255}, BEAK{240, 100, 40}, WING{240, 240, 240};

// ----------------------------- Fonte bitmap 5x7 ----------------------------
struct Glyph { char c; uint8_t rows[7]; };
constexpr Glyph FONT[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {':', {0b00000, 0b00100, 0b00000, 0b00000, 0b00000, 0b00100, 0b00000}},
    {'!', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100}},
    {'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}},
    {'1', {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'2', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}},
    {'3', {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110}},
    {'4', {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}},
    {'5', {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}},
    {'6', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
    {'7', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}},
    {'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}},
    {'9', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}},
    {'A', {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
    {'B', {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}},
    {'C', {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}},
    {'D', {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}},
    {'E', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},
    {'F', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'G', {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111}},
    {'H', {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
    {'I', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'J', {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100}},
    {'K', {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}},
    {'L', {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
    {'M', {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}},
    {'N', {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001}},
    {'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'Q', {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}},
    {'R', {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}},
    {'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}},
    {'T', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'U', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'V', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}},
    {'W', {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}},
    {'X', {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}},
    {'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}},
};

const Glyph* findGlyph(char c) {
    for (const Glyph& g : FONT)
        if (g.c == c) return &g;
    return &FONT[0];
}

// ------------------------- Primitivas de desenho ---------------------------
void setColor(SDL_Renderer* r, Color c, uint8_t a = 255) { SDL_SetRenderDrawColor(r, c.r, c.g, c.b, a); }

void fillRect(SDL_Renderer* r, Color c, int x, int y, int w, int h) {
    setColor(r, c);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void strokeRect(SDL_Renderer* r, Color c, int x, int y, int w, int h, int thick) {
    setColor(r, c);
    for (int i = 0; i < thick; ++i) {
        SDL_Rect rect{x + i, y + i, w - 2 * i, h - 2 * i};
        SDL_RenderDrawRect(r, &rect);
    }
}

void fillEllipse(SDL_Renderer* r, Color c, int cx, int cy, int rx, int ry) {
    setColor(r, c);
    for (int dy = -ry; dy <= ry; ++dy) {
        float t = 1.0f - (static_cast<float>(dy) * dy) / (static_cast<float>(ry) * ry);
        int dx = static_cast<int>(rx * std::sqrt(std::max(0.0f, t)));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void fillCircle(SDL_Renderer* r, Color c, int cx, int cy, int rad) { fillEllipse(r, c, cx, cy, rad, rad); }

void fillTriangle(SDL_Renderer* r, Color c, int x1, int y1, int x2, int y2, int x3, int y3) {
    setColor(r, c);
    auto edge = [](int ax, int ay, int bx, int by, int px, int py) {
        return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    };
    for (int y = std::min({y1, y2, y3}); y <= std::max({y1, y2, y3}); ++y)
        for (int x = std::min({x1, x2, x3}); x <= std::max({x1, x2, x3}); ++x) {
            int e1 = edge(x1, y1, x2, y2, x, y), e2 = edge(x2, y2, x3, y3, x, y),
                e3 = edge(x3, y3, x1, y1, x, y);
            bool neg = e1 < 0 || e2 < 0 || e3 < 0, pos = e1 > 0 || e2 > 0 || e3 > 0;
            if (!(neg && pos)) SDL_RenderDrawPoint(r, x, y);
        }
}

void drawText(SDL_Renderer* r, const std::string& s, int cx, int cy, int scale) {
    const int width = static_cast<int>(s.size()) * 6 * scale - scale;
    for (int pass = 0; pass < 2; ++pass) {
        const int off = pass == 0 ? 2 : 0;  // primeiro a sombra, depois o texto
        const Color col = pass == 0 ? BLACK : WHITE;
        int x0 = cx - width / 2 + off, y0 = cy - (7 * scale) / 2 + off;
        for (char ch : s) {
            const Glyph* g = findGlyph(ch);
            for (int row = 0; row < 7; ++row)
                for (int colIdx = 0; colIdx < 5; ++colIdx)
                    if (g->rows[row] & (1 << (4 - colIdx)))
                        fillRect(r, col, x0 + colIdx * scale, y0 + row * scale, scale, scale);
            x0 += 6 * scale;
        }
    }
}

// --------------------------------- Recorde ---------------------------------
int loadHighscore() {
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({ return parseInt(localStorage.getItem('flappy-cpp-highscore') || '0', 10) || 0; });
#else
    std::ifstream f("highscore.txt");
    int v = 0;
    if (f >> v) return v;
    return 0;
#endif
}

void saveHighscore(int v) {
#ifdef __EMSCRIPTEN__
    EM_ASM({ localStorage.setItem('flappy-cpp-highscore', String($0)); }, v);
#else
    std::ofstream("highscore.txt") << v;
#endif
}

// ----------------------------------- Jogo ----------------------------------
enum class State { Menu, Playing, GameOver };

struct Pipe {
    float x;
    int top;
    bool scored = false;
};

struct Cloud { float x, y, s; };

class Game {
public:
    bool running = true;

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
        window_ = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window_) return false;
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer_) return false;
        SDL_RenderSetLogicalSize(renderer_, W, H);

        rng_.seed(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
        for (int i = 0; i < 5; ++i)
            clouds_.push_back({randf(0, W), randf(40, 300), randf(30, 55)});
        highscore_ = loadHighscore();
        makeBirdTexture();
        reset();
        last_ = SDL_GetTicks();
        return true;
    }

    void shutdown() {
        if (birdTex_) SDL_DestroyTexture(birdTex_);
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    void frame() {
        handleEvents();
        const Uint32 now = SDL_GetTicks();
        acc_ += std::min<double>(now - last_, 200.0);  // passo fixo de 60 FPS
        last_ = now;
        while (acc_ >= STEP_MS) {
            update();
            acc_ -= STEP_MS;
        }
        draw();
    }

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* birdTex_ = nullptr;
    std::mt19937 rng_;

    State state_ = State::Menu;
    float birdY_ = 0, birdVel_ = 0;
    std::vector<Pipe> pipes_;
    std::vector<Cloud> clouds_;
    int score_ = 0, highscore_ = 0;
    float groundOffset_ = 0;
    Uint32 last_ = 0;
    double acc_ = 0;

    float randf(float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng_); }

    Pipe newPipe(float x) {
        const int maxTop = H - GROUND_H - PIPE_GAP - PIPE_MIN_TOP;
        return {x, std::uniform_int_distribution<int>(PIPE_MIN_TOP, maxTop)(rng_)};
    }

    void reset() {
        birdY_ = H / 2.0f - 40;
        birdVel_ = 0;
        pipes_.clear();
        pipes_.push_back(newPipe(W + 100));
        score_ = 0;
        state_ = State::Menu;
    }

    void flapOrStart() {
        if (state_ == State::Menu) {
            state_ = State::Playing;
            birdVel_ = FLAP;
        } else if (state_ == State::Playing) {
            birdVel_ = FLAP;
        }
    }

    void gameOver() {
        state_ = State::GameOver;
        if (score_ > highscore_) {
            highscore_ = score_;
            saveHighscore(highscore_);
        }
    }

    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_SPACE:
                    case SDLK_UP: flapOrStart(); break;
                    case SDLK_r:
                        if (state_ == State::GameOver) reset();
                        break;
                    default: break;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (state_ == State::GameOver) reset(); else flapOrStart();
            }
        }
    }

    void scrollWorld() {
        groundOffset_ = std::fmod(groundOffset_ - PIPE_SPEED + 24.0f, 24.0f);
        for (Cloud& c : clouds_) c.x = c.x > -80 ? c.x - 0.5f : W + 40;
    }

    bool hits(const Pipe& p) const {
        const float bx = BIRD_X - BIRD_R + 2, by = birdY_ - BIRD_R + 2, bs = 2.0f * BIRD_R - 4;
        const bool overlapX = bx < p.x + PIPE_W && bx + bs > p.x;
        return overlapX && (by < p.top || by + bs > p.top + PIPE_GAP);
    }

    void update() {
        const float groundY = H - GROUND_H - BIRD_R;

        if (state_ == State::Menu) {
            birdY_ = H / 2.0f - 40 + ((SDL_GetTicks() / 300) % 2 ? 6 : -6);
            scrollWorld();
            return;
        }
        if (state_ == State::GameOver) {
            if (birdY_ < groundY) {
                birdVel_ = std::min(birdVel_ + GRAVITY, MAX_FALL);
                birdY_ = std::min(birdY_ + birdVel_, groundY);
            }
            return;
        }

        scrollWorld();
        birdVel_ = std::min(birdVel_ + GRAVITY, MAX_FALL);
        birdY_ += birdVel_;

        for (Pipe& p : pipes_) {
            p.x -= PIPE_SPEED;
            if (!p.scored && p.x + PIPE_W < BIRD_X) {
                p.scored = true;
                ++score_;
            }
            if (hits(p)) {
                gameOver();
                return;
            }
        }
        if (pipes_.back().x < W - PIPE_SPACING) pipes_.push_back(newPipe(W));
        pipes_.erase(std::remove_if(pipes_.begin(), pipes_.end(),
                                    [](const Pipe& p) { return p.x + PIPE_W < 0; }),
                     pipes_.end());

        if (birdY_ + BIRD_R >= H - GROUND_H || birdY_ - BIRD_R < -40) {
            birdY_ = std::min(birdY_, groundY);
            gameOver();
        }
    }

    // ------------------------------- Desenho -------------------------------
    void makeBirdTexture() {
        birdTex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 40, 32);
        SDL_SetTextureBlendMode(birdTex_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(renderer_, birdTex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
        SDL_RenderClear(renderer_);
        fillEllipse(renderer_, BLACK, 18, 16, 16, 14);
        fillEllipse(renderer_, YELLOW, 18, 16, 14, 12);
        fillEllipse(renderer_, WING, 11, 19, 7, 5);
        fillCircle(renderer_, WHITE, 25, 12, 6);
        fillCircle(renderer_, BLACK, 27, 12, 3);
        fillTriangle(renderer_, BEAK, 30, 18, 39, 20, 30, 24);
        SDL_SetRenderTarget(renderer_, nullptr);
    }

    void drawPipePart(int x, int y, int w, int h) {
        fillRect(renderer_, GREEN, x, y, w, h);
        strokeRect(renderer_, DARK, x, y, w, h, 3);
    }

    void drawPipe(const Pipe& p) {
        const int x = static_cast<int>(p.x), bottomY = p.top + PIPE_GAP;
        drawPipePart(x, 0, PIPE_W, p.top);
        drawPipePart(x - 4, p.top - 24, PIPE_W + 8, 24);
        drawPipePart(x, bottomY, PIPE_W, H - GROUND_H - bottomY);
        drawPipePart(x - 4, bottomY, PIPE_W + 8, 24);
    }

    void drawGround() {
        const int y = H - GROUND_H;
        fillRect(renderer_, GROUND_TOP, 0, y, W, GROUND_H);
        fillRect(renderer_, GREEN, 0, y, W, 14);
        fillRect(renderer_, DARK, 0, y, W, 3);
        setColor(renderer_, STRIPE);
        for (int x = -24; x < W + 24; x += 24) {
            const int sx = x + static_cast<int>(groundOffset_);
            for (int t = -2; t <= 2; ++t)  // linha diagonal "grossa"
                SDL_RenderDrawLine(renderer_, sx + t, y + 2, sx + 12 + t, y + 12);
        }
    }

    void drawBird() {
        const double angle = birdVel_ > 0 ? std::min(birdVel_ * 4.0f, 70.0f) : -25.0f;
        SDL_Rect dst{static_cast<int>(BIRD_X) - 20, static_cast<int>(birdY_) - 16, 40, 32};
        SDL_RenderCopyEx(renderer_, birdTex_, nullptr, &dst, angle, nullptr, SDL_FLIP_NONE);
    }

    void draw() {
        setColor(renderer_, SKY);
        SDL_RenderClear(renderer_);
        for (const Cloud& c : clouds_) {
            const int cx = static_cast<int>(c.x), cy = static_cast<int>(c.y);
            fillCircle(renderer_, CLOUD, cx, cy, static_cast<int>(c.s));
            fillCircle(renderer_, CLOUD, cx + static_cast<int>(c.s * 0.7f), cy + 6, static_cast<int>(c.s * 0.7f));
            fillCircle(renderer_, CLOUD, cx - static_cast<int>(c.s * 0.7f), cy + 8, static_cast<int>(c.s * 0.6f));
        }
        for (const Pipe& p : pipes_) drawPipe(p);
        drawGround();
        drawBird();

        switch (state_) {
            case State::Menu:
                drawText(renderer_, "FLAPPY BIRD", W / 2, 150, 5);
                drawText(renderer_, "ESPACO OU CLIQUE PARA PULAR", W / 2, 400, 2);
                drawText(renderer_, "RECORDE: " + std::to_string(highscore_), W / 2, 430, 2);
                break;
            case State::Playing:
                drawText(renderer_, std::to_string(score_), W / 2, 70, 6);
                break;
            case State::GameOver:
                drawText(renderer_, "GAME OVER", W / 2, 170, 6);
                drawText(renderer_, "PONTOS: " + std::to_string(score_), W / 2, 250, 3);
                drawText(renderer_, "RECORDE: " + std::to_string(highscore_), W / 2, 290, 3);
                drawText(renderer_, "R OU CLIQUE PARA REINICIAR", W / 2, 350, 2);
                break;
        }
        SDL_RenderPresent(renderer_);
    }
};

#ifdef __EMSCRIPTEN__
void emFrame(void* arg) { static_cast<Game*>(arg)->frame(); }
#endif

}  // namespace

int main(int, char**) {
    Game game;
    if (!game.init()) {
        SDL_Log("Falha ao iniciar o SDL: %s", SDL_GetError());
        return 1;
    }
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(emFrame, &game, 0, 1);
#else
    while (game.running) game.frame();
    game.shutdown();
#endif
    return 0;
}
