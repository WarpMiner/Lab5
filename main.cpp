#include "include.h"

class Entities {
public:
    int x, y; // Координаты
    int speed; // Скорость
    EntityType type;

    Entities(int x, int y, EntityType type) : x(x), y(y), type(type) {}

    virtual void update() {}
    virtual void draw(SDL_Renderer* renderer) {}
    virtual ~Entities() {}
};

class Player : public Entities {
public:
    int lives;
    int score; // Добавлена переменная для хранения очков
    int lastShotTime; // Время последнего выстрела
    const int cooldownTime = 500; // Время между выстрелами (в миллисекундах)

    Player(int x, int y) : Entities(x, y, PLAYER), lives(3), score(0), lastShotTime(0) {}

    void draw(SDL_Renderer* renderer, TTF_Font* font, int currentWave) {
        // Рисуем игрока
        SDL_Rect rect = { x, y, PLAYER_WIDTH, PLAYER_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Отображаем счет, жизни и волну
        displayScore(renderer, font, currentWave);
    }

    void loseLife() {
        lives--;
        score *= 0.25; // Уменьшаем счет на четверть за потерю жизни
    }

    void addScore(int points) {
        score += points; // Увеличиваем счет
    }

    void displayScore(SDL_Renderer* renderer, TTF_Font* font, int currentWave) {
        // Подготовка текста для отображения
        SDL_Color scoreColor = { 255, 255, 255 }; // Белый цвет для счета
        SDL_Color livesColor = { 255, 0, 0 }; // Красный цвет для жизней
        SDL_Color waveColor = { 0, 255, 255 }; // Циан для волн

        string scoreText = "Score: " + to_string(score);
        string livesText = "Lives: " + to_string(lives);
        string waveText = "Wave: " + to_string(currentWave + 1); // +1, чтобы волна начиналась с 1

        // Создание текстовой поверхности для счета
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), scoreColor);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

        // Отображение текста счета
        SDL_Rect scoreRect = { 10, 10, scoreSurface->w, scoreSurface->h };
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

        // Освобождение ресурсов
        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        // Создание текстовой поверхности для жизней
        SDL_Surface* livesSurface = TTF_RenderText_Solid(font, livesText.c_str(), livesColor);
        SDL_Texture* livesTexture = SDL_CreateTextureFromSurface(renderer, livesSurface);

        // Отображение текста жизней
        SDL_Rect livesRect = { 10, 40, livesSurface->w, livesSurface->h }; // Позиция под счетом
        SDL_RenderCopy(renderer, livesTexture, NULL, &livesRect);

        // Освобождение ресурсов
        SDL_FreeSurface(livesSurface);
        SDL_DestroyTexture(livesTexture);

        // Создание текстовой поверхности для волн
        SDL_Surface* waveSurface = TTF_RenderText_Solid(font, waveText.c_str(), waveColor);
        SDL_Texture* waveTexture = SDL_CreateTextureFromSurface(renderer, waveSurface);

        // Отображение текста волн
        SDL_Rect waveRect = { 10, 70, waveSurface->w, waveSurface->h }; // Позиция под жизнями
        SDL_RenderCopy(renderer, waveTexture, NULL, &waveRect);

        // Освобождение ресурсов
        SDL_FreeSurface(waveSurface);
        SDL_DestroyTexture(waveTexture);
    }
};

class Bullet : public Entities {
public:
    Bullet(int x, int y, EntityType bulletType) : Entities(x, y, bulletType) {
        speed = (bulletType == BULLET_PLAYER) ? 10 : 5; // Скорость пуль
    }

    void update() override {
        // Логика движения пули
        y -= (type == BULLET_PLAYER) ? speed : -speed; // пули игрока идут вверх, пули пришельцев - вниз
    }

    void draw(SDL_Renderer* renderer) override {
        SDL_Rect rect = { x, y, BULLET_WIDTH, BULLET_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Alien : public Entities {
public:
    int direction; // Направление движения
    bool movedDown; // Флаг для проверки, переместился ли вниз
    int shootCooldown; // Время до следующей стрельбы
    int lastShotTime; // Время последнего выстрела
    const int shootInterval = 1500; // Интервал стрельбы для пришельцев

    Alien(int x, int y, EntityType type) : Entities(x, y, type), movedDown(false), shootCooldown(0), lastShotTime(0) {
        speed = (type == ALIEN_M) ? 1 : (type == ALIEN_W) ? 5 : 3; // Разные скорости
        direction = 1; // Начальное направление вправо
    }

    void update() override {
        // Логика движения пришельца
        x += direction * speed; // Движение по оси X

        // Проверка столкновения со стенами
        if (x < 0) {
            x = 0; // Остановить на границе
            direction *= -1; // Изменение направления
            movedDown = true; // Установить флаг на движение вниз
        } else if (x + ALIEN_WIDTH > SCREEN_WIDTH) {
            x = SCREEN_WIDTH - ALIEN_WIDTH; // Остановить на границе
            direction *= -1; // Изменение направления
            movedDown = true; // Установить флаг на движение вниз
        }

        // Если мы переместили пришельца вниз, обновляем его позицию
        if (movedDown) {
            y += ALIEN_HEIGHT + 20; // Опускание на одну строчку вниз
            movedDown = false; // Сбрасываем флаг
        }

        // Логика стрельбы
        if (type == ALIEN_O) { // Если это стреляющий пришелец
            shootCooldown += 16; // Увеличиваем cooldown на основе времени обновления
            if (shootCooldown >= shootInterval) {
                shootCooldown = 0;
                if (rand() % shootInterval < 100) { // 100 шанс из shootInterval
                    // Здесь бы могла быть стрельба
                }
            }
        }
    }

    void draw(SDL_Renderer* renderer) override {
        SDL_Rect rect = { x, y, ALIEN_WIDTH, ALIEN_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет для обычного пришельца
        if (type == ALIEN_O) SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255); // Фиолетовый для стреляющего пришельца
        else if (type == ALIEN_M) SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Синий для медленного пришельца
        else if (type == ALIEN_W) SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Оранжевый для быстрого пришельца
        SDL_RenderFillRect(renderer, &rect);
    }

    Bullet* shoot() {
        return new Bullet(x + ALIEN_WIDTH / 2 - BULLET_WIDTH / 2, y + ALIEN_HEIGHT, BULLET_ALIEN);
    }
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    vector<Entities*> entities;
    Player* player;
    TTF_Font* font; // Шрифт для отображения текста
    int currentWave; // Текущая волна
    const int totalWaves = 5; // Общее количество волн

public:
    Game() : currentWave(0) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            exit(1);
        }

        if (TTF_Init() == -1) {
            cout << "TTF could not initialize! TTF_Error: " << TTF_GetError() << endl;
            exit(1);
        }

        window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        font = TTF_OpenFont("fonts/Arial.ttf", 24);
        if (!font) {
            cout << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
            exit(1);
        }

        player = new Player(SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - PLAYER_HEIGHT - 10);
        entities.push_back(player);
        spawnAliens(15); // Спавним первую волну
    }

    void spawnAliens(int n) {
        for (int i = 0; i < n; ++i) {
            int type = rand() % 4; // Случайный тип пришельца
            int x = rand() % (SCREEN_WIDTH - ALIEN_WIDTH); // Случайная позиция по X
            if (type == 0) {
                entities.push_back(new Alien(x, -ALIEN_HEIGHT, ALIEN_V)); // Обычный пришелец
            } else if (type == 1) {
                entities.push_back(new Alien(x, -ALIEN_HEIGHT, ALIEN_O)); // Стреляющий пришелец
            } else if (type == 2) {
                entities.push_back(new Alien(x, -ALIEN_HEIGHT, ALIEN_M)); // Медленный пришелец
            } else {
                entities.push_back(new Alien(x, -ALIEN_HEIGHT, ALIEN_W)); // Быстрый пришелец
            }
        }
    }

    void input() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (player->x > 0) {
                            player->x -= 10; // Движение влево
                        }
                        break;
                    case SDLK_RIGHT:
                        if (player->x < SCREEN_WIDTH - PLAYER_WIDTH) {
                            player->x += 10; // Движение вправо
                        }
                        break;
                    case SDLK_SPACE:
                        if (SDL_GetTicks() - player->lastShotTime >= player->cooldownTime) {
                            entities.push_back(new Bullet(player->x + PLAYER_WIDTH / 2 - BULLET_WIDTH / 2, player->y, BULLET_PLAYER));
                            player->lastShotTime = SDL_GetTicks(); // Обновляем время последнего выстрела
                        }
                        break;
                }
            }
        }
    }

    void update() {
        // Обновление всех объектов
        for (size_t i = 0; i < entities.size(); ++i) {
            entities[i]->update();
        }

        // Логика стрельбы пришельцев
        for (size_t i = 0; i < entities.size(); ++i) {
            Alien* alien = dynamic_cast<Alien*>(entities[i]);
            if (alien && alien->type == ALIEN_O && SDL_GetTicks() - alien->lastShotTime >= alien->shootInterval) {
                entities.push_back(alien->shoot());
                alien->lastShotTime = SDL_GetTicks(); // Обновляем время последнего выстрела
            }
        }

        // Логика столкновения пуль с пришельцами
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i]->type == BULLET_PLAYER) {
                Bullet* bullet = static_cast<Bullet*>(entities[i]);
                for (size_t j = 0; j < entities.size(); ++j) {
                    if (entities[j]->type == ALIEN_V || entities[j]->type == ALIEN_O || entities[j]->type == ALIEN_M || entities[j]->type == ALIEN_W) {
                        Alien* alien = static_cast<Alien*>(entities[j]);
                        if (bullet->x < alien->x + ALIEN_WIDTH &&
                            bullet->x + BULLET_WIDTH > alien->x &&
                            bullet->y < alien->y + ALIEN_HEIGHT &&
                            bullet->y + BULLET_HEIGHT > alien->y) {
                            // Уничтожение пришельца и пули
                            delete entities[j];
                            entities.erase(entities.begin() + j);
                            player->addScore(60); // Увеличиваем счет игрока на 60
                            delete entities[i];
                            entities.erase(entities.begin() + i);
                            break; // Прерываем, чтобы избежать ошибок
                        }
                    }
                }
            }
        }

        // Логика столкновения пришельцев с игроком
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i]->type == ALIEN_V || entities[i]->type == ALIEN_O || entities[i]->type == ALIEN_M || entities[i]->type == ALIEN_W) {
                Alien* alien = static_cast<Alien*>(entities[i]);
                if (alien->y + ALIEN_HEIGHT > player->y && alien->y < player->y + PLAYER_HEIGHT &&
                    alien->x < player->x + PLAYER_WIDTH && alien->x + ALIEN_WIDTH > player->x) {
                    player->loseLife(); // Уменьшаем жизни игрока
                    entities.erase(entities.begin() + i); // Удаляем пришельца
                    i--; // Уменьшаем индекс, так как мы удалили пришельца
                }
                if (alien->y > SCREEN_HEIGHT) { // Если пришелец добирается до низа
                    player->loseLife();
                    delete entities[i];
                    entities.erase(entities.begin() + i);
                    i--;
                }
            }
        }

        // Проверка столкновения пуль пришельцев с игроком
        for (size_t i = 0; i < entities.size(); ++i) {
            if (entities[i]->type == BULLET_ALIEN) {
                Bullet* alienBullet = static_cast<Bullet*>(entities[i]);
                if (alienBullet->y + BULLET_HEIGHT > player->y &&
                    alienBullet->y < player->y + PLAYER_HEIGHT &&
                    alienBullet->x < player->x + PLAYER_WIDTH &&
                    alienBullet->x + BULLET_WIDTH > player->x) {
                    player->loseLife(); // Уменьшаем жизни игрока
                    delete entities[i]; // Удаляем пулю пришельца
                    entities.erase(entities.begin() + i);
                    i--; // Уменьшаем индекс, так как мы удалили пулю

                    // Удаляем все пули на экране
                    for (size_t k = 0; k < entities.size(); ) {
                        if (entities[k]->type == BULLET_PLAYER || entities[k]->type == BULLET_ALIEN) {
                            delete entities[k];
                            entities.erase(entities.begin() + k);
                        } else {
                            k++;
                        }
                    }

                    if (player->lives <= 0) {
                        SDL_ShowSimpleMessageBox(0, "Игра окончена", "Жизни закончились!", window);
                        exit(0);
                    }
                }
            }
        }

        // Проверка на отсутствие пришельцев
        if (std::none_of(entities.begin(), entities.end(), [](Entities* e) { return e->type == ALIEN_V || e->type == ALIEN_O || e->type == ALIEN_M || e->type == ALIEN_W; })) {
            currentWave++; // Переход к следующей волне
            if (currentWave < totalWaves) {
                spawnAliens(20 + currentWave * 10); // Спавним пришельцев новой волны
            } else {
                SDL_ShowSimpleMessageBox(0, "Победа", "Все пришельцы побеждены!", window);
                exit(0);
            }
        }

        // Проверка жизни игрока
        if (player->lives <= 0) {
            SDL_ShowSimpleMessageBox(0, "Игра окончена", "Жизни закончились!", window);
            exit(0);
        }
    }

    void draw() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (auto entity : entities) {
            entity->draw(renderer);
        }

        player->draw(renderer, font, currentWave); // Обновленный вызов для отображения счета и текущей волны

        SDL_RenderPresent(renderer);
    }

    void run() {
        while (true) {
            input();
            update();
            draw();
            SDL_Delay(4); // Задержка для управления скоростью игры
        }
    }

    ~Game() {
        for (auto entity : entities) {
            delete entity;
        }
        TTF_CloseFont(font); // Закрытие шрифта
        TTF_Quit(); // Завершение работы TTF
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(0))); // Для случайного выбора пришельцев
    Game game;
    game.run();
    return 0;
}
