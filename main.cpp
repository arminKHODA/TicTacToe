#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int TILE_SIZE = 100;
const int GRID_SIZE = 3;
const int SCREEN_WIDTH = TILE_SIZE * GRID_SIZE;
const int SCREEN_HEIGHT = TILE_SIZE * GRID_SIZE;

enum Player { NONE, PLAYER_X, PLAYER_O };
enum GameMode { NONE_MODE, PLAYER_VS_PLAYER, PLAYER_VS_COMPUTER };

struct Tile {
    Player owner;
};

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
Tile grid[GRID_SIZE][GRID_SIZE];
Player currentPlayer = PLAYER_X;
GameMode gameMode = NONE_MODE;
bool gameRunning = false;
bool gameWon = false;
Player winner = NONE;

void RenderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (surface == nullptr) {
        std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void InitGrid() {
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            grid[y][x] = { NONE };
        }
    }
    currentPlayer = PLAYER_X;
    gameRunning = true;
    gameWon = false;
    winner = NONE;
}

Player CheckWinCondition() {
    // Check rows
    for (int y = 0; y < GRID_SIZE; ++y) {
        if (grid[y][0].owner != NONE && grid[y][0].owner == grid[y][1].owner && grid[y][1].owner == grid[y][2].owner) {
            return grid[y][0].owner;
        }
    }

    // Check columns
    for (int x = 0; x < GRID_SIZE; ++x) {
        if (grid[0][x].owner != NONE && grid[0][x].owner == grid[1][x].owner && grid[1][x].owner == grid[2][x].owner) {
            return grid[0][x].owner;
        }
    }

    // Check diagonals
    if (grid[0][0].owner != NONE && grid[0][0].owner == grid[1][1].owner && grid[1][1].owner == grid[2][2].owner) {
        return grid[0][0].owner;
    }

    if (grid[0][2].owner != NONE && grid[0][2].owner == grid[1][1].owner && grid[1][1].owner == grid[2][0].owner) {
        return grid[0][2].owner;
    }

    // Check for tie
    bool tie = true;
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (grid[y][x].owner == NONE) {
                tie = false;
                break;
            }
        }
        if (!tie) break;
    }

    if (tie) {
        return NONE;
    }

    return NONE;
}

void RenderGrid() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            SDL_Rect rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);

            if (grid[y][x].owner == PLAYER_X) {
                RenderText("X", x * TILE_SIZE + 40, y * TILE_SIZE + 30, { 0, 0, 0, 255 });
            }
            else if (grid[y][x].owner == PLAYER_O) {
                RenderText("O", x * TILE_SIZE + 40, y * TILE_SIZE + 30, { 0, 0, 0, 255 });
            }
        }
    }

    if (gameWon) {
        std::string message = (winner != NONE) ? (winner == PLAYER_X ? "Player X Wins!" : "Player O Wins!") : "It's a Tie!";
        RenderText(message, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 10, { 255, 0, 0, 255 });
        RenderText("Press Enter to Restart", SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 20, { 255, 0, 0, 255 });
        RenderText("Press Escape to Quit", SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 50, { 255, 0, 0, 255 });
    }

    SDL_RenderPresent(renderer);
}

void HandleMouseClick(int x, int y) {
    int gridX = x / TILE_SIZE;
    int gridY = y / TILE_SIZE;

    if (grid[gridY][gridX].owner == NONE && !gameWon) {
        grid[gridY][gridX].owner = currentPlayer;
        currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
    }

    winner = CheckWinCondition();
    if (winner != NONE || gameWon) {
        gameWon = true;
    }
}

void ComputerMove() {
    if (!gameRunning || gameWon) return;

    // Simple AI: Random move
    std::vector<std::pair<int, int>> availableMoves;
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (grid[y][x].owner == NONE) {
                availableMoves.push_back({ x, y });
            }
        }
    }

    if (!availableMoves.empty()) {
        srand(static_cast<unsigned int>(time(0)));
        int choice = rand() % availableMoves.size();
        grid[availableMoves[choice].second][availableMoves[choice].first].owner = PLAYER_O;
        currentPlayer = PLAYER_X;
    }

    winner = CheckWinCondition();
    if (winner != NONE || gameWon) {
        gameWon = true;
    }
}

void RenderMenu() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    RenderText("1. Player vs Player", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 30, { 0, 0, 0, 255 });
    RenderText("2. Player vs Computer", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, { 0, 0, 0, 255 });
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() < 0) {
        std::cerr << "Could not initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Tic-Tac-Toe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        std::cerr << "Could not load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        if (gameMode == NONE_MODE) {
            RenderMenu();
        }
        else {
            if (gameRunning && currentPlayer == PLAYER_O && gameMode == PLAYER_VS_COMPUTER) {
                ComputerMove();
            }
            RenderGrid();
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && gameRunning) {
                int x = event.button.x;
                int y = event.button.y;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    HandleMouseClick(x, y);
                }
            }
            if (gameWon && event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    InitGrid();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
            if (gameMode == NONE_MODE && event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_1) {
                    gameMode = PLAYER_VS_PLAYER;
                    InitGrid();
                }
                else if (event.key.keysym.sym == SDLK_2) {
                    gameMode = PLAYER_VS_COMPUTER;
                    InitGrid();
                }
            }
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
