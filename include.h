#pragma once

using namespace std;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> // Для работы с текстом
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

enum EntityType {
    PLAYER,
    ALIEN_V,       // Обычные пришельцы
    ALIEN_O,       // Стреляющие пришельцы
    ALIEN_M,       // Медленные пришельцы
    ALIEN_W,       // Быстрые пришельцы
    BULLET_PLAYER,
    BULLET_ALIEN
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 20;
const int ALIEN_WIDTH = 50;
const int ALIEN_HEIGHT = 20;
const int BULLET_WIDTH = 5;
const int BULLET_HEIGHT = 10;
