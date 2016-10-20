#include <SDL2/SDL.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>

const int screenHeight = 1030;
const int screenWidth = 1850;

const int numPlatforms = 1000;
const int numEnemies = 5;

const float gravity = .5f;

//Defines properties of a player
typedef struct
{
    float x, y;
    float dy, dx;
    short life;
    int jumping;
    int animation;
    int facingLeft;
    int flip;
    int degrees;
} Player;

typedef struct
{
    float x, dx, w, h, y, dy;
    int spawnPoint;
    int life;
    int facingLeft;
    int jumping;
} Enemy;

//defines properties of a bullet
typedef struct
{
    float x, y, w, h, dx;
    int fireReady;
} Bullet;

//defines properties of a platform
typedef struct
{
    int x, y, w, h;
} Platform;

//defines everything that the game contains
typedef struct
{
    Player player;
    Enemy enemy[5];
    Bullet bullet;
    Platform platform[1000];
    SDL_Texture *boiText;
    SDL_Texture *brick;
    SDL_Texture *datboi;
    SDL_Texture *message;
    SDL_Texture *texture;
    SDL_Texture *enemyTexture;
    Mix_Music *music;
    Mix_Chunk *shots, *jump, *hitmarker, *ohgod, *ribbit;
    SDL_Renderer *renderer;
    TTF_Font *font;
    float scrollX;
    int time;
    int offset;
    int platIndex;
} Instance;

void enemyMove(Instance *instance)
{
    for(int i = 0; i < 5; i++)
    {
        instance->enemy[i].x += instance->enemy[i].dx;
        instance->enemy[i].facingLeft = 1;
        instance->enemy[i].y += instance->enemy[i].dy;
    }
}

//moves player based on values determined in the processEvents() method
void move(Instance *instance)
{
    instance->time++;

    enemyMove(instance);

    Player *player = &instance->player;

    instance->bullet.x += instance->bullet.dx;

    for(int i = 0; i < 5; i++)
    {
        instance->enemy[i].x += instance->enemy[i].dx;
        instance->enemy[i].y += instance->enemy[i].dy;
    }

    player->y += player->dy;
    player->x += player->dx;

    if(instance->bullet.fireReady == 1)
    {
        if(instance->player.facingLeft == 1)
        {
            instance->bullet.x = player->x - 2.5;
            instance->bullet.y = player->y + 2.5;
        }
        else
        {
            instance->bullet.x = player->x + 78;
            instance->bullet.y = player->y + 2.5;
        }
    }


    /**
    If the player is moving, after a certain amount of time jump to next individual of the sprite sheet for "player".
    **/
    if(player->dx != 0 && player->jumping)
    {
        if(instance->time % 3 == 0)
        {
            int animationSplit = 100; //the x distance of each individual sprite on the sprite sheet
            if(player->animation == 0)
            {
                player->animation = animationSplit;
            }
            else if(player->animation == animationSplit)
            {
                player->animation = 2 * animationSplit;
            }
            else if(player->animation == 2 * animationSplit)
            {
                player->animation = 3 * animationSplit;
            }
            else if(player->animation == 3 * animationSplit)
            {
                player->animation = 4 * animationSplit;
            }
            else if(player->animation == 4 * animationSplit)
            {
                player->animation = 0;
            }
        }
    }
    /**
    dy is velocity, gravity is acceleration
    **/
    player->dy += gravity;

    instance->scrollX = -instance->player.x + screenWidth / 2;

    if(instance->scrollX > 0)
        instance->scrollX = 0;

    if(instance->bullet.x > instance->player.x + screenWidth / 2 || instance->bullet.x < instance->player.x - screenWidth / 2)
    {
        instance->bullet.dx = 0;
        instance->bullet.fireReady = 1;
    }
}

//returns index of the top platform to check collision with
void trackPlats(Instance *instance)
{
    if(instance->player.x > instance->platform[instance->platIndex].x + instance->platform[instance->platIndex].w && instance->player.dx > 0)
        instance->platIndex++;
    else if(instance->player.dx < 0 && instance->player.x < instance->platform[instance->platIndex].x)
    {
        if(instance->platIndex > 0)
            instance->platIndex--;
    }

    printf("%d\n", instance->platIndex);
}

void enemyCollisionDetect(Instance *instance)
{
    for(int i = 0; i < numEnemies; i++)
    {
        if(instance->enemy[i].y <= 840)
        {
            instance->enemy[i].dy = 0;
            instance->enemy[i].y = 840;
        }
    }
}

void collisionDetect(Instance *instance)
{
    trackPlats(instance);

    int i = instance->platIndex;

    float sLeft = instance->player.x, sRight = instance->player.x + 90;
    float sTop = instance->player.y, sBot = instance->player.y + 140;

    float bLeft = instance->platform[i].x, bRight = instance->platform[i].x + instance->platform[i].w;
    float bTop = instance->platform[i].y, bBot = instance->platform[i].y + instance->platform[i].h;


    if(sLeft > bLeft && sRight < bRight)
    {
        if(sBot <= bTop + 20)
        {
            if(sBot >= bTop && instance->player.dy > 0)
            {
                instance->player.dy = 0;
                instance->player.y = bTop - 140;
                instance->player.jumping = 1;
            }
        }
        else if(sTop >= bBot - 20)
        {
            if(sTop < bBot && instance->player.dy < 0)
            {
                instance->player.y = bBot;
                instance->player.dy = 0;
            }
        }
    }
    else if(sRight <= bLeft + 10 && (bBot < sTop && bBot > sBot) || (bTop < sTop && bTop > sBot))
    {

        instance->player.dx = 0;
        instance->player.x = bLeft - 90;

    }




    if(instance->player.y > 840)
    {
        instance->player.y = 840;
        instance->player.dy = 0;
        instance->player.jumping = 1;
    }

    if(instance->player.x - 100 < 0)
    {
        instance->player.x = 100;
        instance->player.dx = 0;
    }
}

void procedurallyGenerate(Instance *instance)
{
    Platform *platform = instance->platform;

    for(int i = 0; i < numPlatforms / 2; i++)
    {
        platform[i].w = rand() % 350 + 300;
        platform[i].h = 50;
        platform[i].x = platform[i-1].x + platform[i - 1].w + rand() % 600 + 200;
        platform[i].y = (screenHeight - platform[i].h) - rand() % 300 - 200;
    }

    for(int i = numPlatforms / 2; i < numPlatforms; i++)
    {
        platform[i].w = 200;
        platform[i].h = 50;
        platform[i].x = (i - (numPlatforms / 2)) * 200;
        platform[i].y = screenHeight - platform[i].h;
    }
}

void initValues(Instance *instance)
{
    procedurallyGenerate(instance);

    instance->bullet.x = 0;
    instance->bullet.y = 0;
    instance->bullet.w = 30;
    instance->bullet.h = 30;
    instance->bullet.dx = 0;
    instance->bullet.fireReady = 1;

    instance->scrollX = 0;

    instance->player.x = 201;
    instance->player.y = 0;
    instance->player.dy = 0;
    instance->player.dx = 0;
    instance->player.jumping = 0;
    instance->player.animation = 0;
    instance->player.facingLeft = 0;
    instance->player.flip = 0;
    instance->player.degrees = 0;
    instance->player.life = 5;

    for(int i = 0; i < numEnemies; i++)
    {
        instance->enemy[i].spawnPoint = 1000 + 700 * i;
        instance->enemy[i].x = instance->enemy[i].spawnPoint;
        instance->enemy[i].dx = 0;
        instance->enemy[i].y = 200;
        instance->enemy[i].facingLeft = 1;
        instance->enemy[i].w = 200;
        instance->enemy[i].h = 200;
        instance->enemy[i].dy = 5;
        instance->enemy[i].jumping = 1;
        instance->enemy[i].life = 2;
    }

    instance->time = 0;
    instance->offset = 100;
    instance->platIndex = 0;
}

void loadText(Instance *instance)
{
    SDL_Color white = { 255, 255, 255 };

    instance->font = TTF_OpenFont("GoodDog.otf", 100);

    SDL_Surface *surface = TTF_RenderText_Solid(instance->font, "OH SHIT WADDUP", white);
    instance->message = SDL_CreateTextureFromSurface(instance->renderer, surface);
    SDL_FreeSurface(surface);

    surface = TTF_RenderText_Solid(instance->font, "BOI", white);
    instance->boiText = SDL_CreateTextureFromSurface(instance->renderer, surface);
    SDL_FreeSurface(surface);
}

/**

makes the textures usable by renderer
**/
void loadImages(Instance *instance)
{
    /**
    create a surface, load the texture to it specifying the renderer youre using to do so
    **/

    SDL_Surface *surface = NULL;

    surface = IMG_Load("datboi.png");
    instance->datboi = SDL_CreateTextureFromSurface(instance->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("brick.png");
    instance->brick = SDL_CreateTextureFromSurface(instance->renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("enemy.png");
    instance->enemyTexture = SDL_CreateTextureFromSurface(instance->renderer, surface);
    SDL_FreeSurface(surface);
}

void loadMusic(Instance *instance)
{
    instance->music = Mix_LoadMUS("darude.mp3");
    Mix_VolumeMusic(60);
    //Mix_PlayMusic(instance->music, -1);

    instance->hitmarker = Mix_LoadWAV("HITMARKER.wav");
    instance->jump = Mix_LoadWAV("jump.wav");
    instance->ohgod = Mix_LoadWAV("ohgod.wav");
    instance->shots = Mix_LoadWAV("intervention.wav");
    instance->ribbit = Mix_LoadWAV("frogs.wav");
}

/**
Loads the images
**/
void loadGame(Instance *instance)
{
    initValues(instance);
    loadImages(instance);
    loadText(instance);
    loadMusic(instance);
}

/**
Handles exit events, key events, all other events etc
**/
int processEvents(SDL_Window *window, Instance *instance)
{
    SDL_Event event;
    int done = 0;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_WINDOWEVENT_CLOSE:
            {
                if(window)
                {
                    SDL_DestroyWindow(window);
                    window = NULL;
                    done = 1;
                }
            }
            break;
            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        done = 1;
                    break;
                    case SDLK_w:
                        if(instance->player.jumping)
                        {
                            //Mix_PlayChannel(-1, instance->jump, 0);
                            instance->player.dy = -15;
                            instance->player.jumping = 0;
                        }
                    break;
                }
            }
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if(state[SDL_SCANCODE_W])
    {
        instance->player.dy -= .2f;
    }

    if(state[SDL_SCANCODE_SPACE])
    {
        if(instance->bullet.fireReady == 1)
        {
            Mix_PlayChannel(-1, instance->shots, 0);
            if(instance->player.facingLeft == 1)
                instance->bullet.dx = -40;
            else if(instance->player.facingLeft == 0)
                instance->bullet.dx = 40;
        instance->bullet.fireReady = 0;
        }
    }

    if(state[SDL_SCANCODE_A])
    {
        instance->player.facingLeft = 1;
        instance->player.dx -= .5;
        if(instance->player.dx < -6)
        {
            instance->player.dx = -6;
        }
    }

    else if(state[SDL_SCANCODE_D])
    {
        instance->player.facingLeft = 0;
        instance->player.dx += .5;
        if(instance->player.dx > 6)
        {
            instance->player.dx =  6;
        }
    }

    else
    {
        instance->player.dx *= .8f;
        if(fabsf(instance->player.dx) < 0.1f)
        {
            instance->player.dx = 0;
        }
    }

    if(state[SDL_SCANCODE_O])
    {
        //Mix_PlayChannel(-1, instance->ohgod, 0);
    }

    if(state[SDL_SCANCODE_X] && instance->player.dy < 0)
        instance->player.flip = 1;

    if(instance->player.life <= 0)
        done = 1;

    return done;
}

void drawPlatforms(Instance *instance)
{
    for(int i = 0; i < numPlatforms; i++)
    {
        SDL_Rect platRect = { instance->scrollX+instance-> platform[i].x - instance->offset, instance ->platform[i].y, instance->platform[i].w, instance->platform[i].h };
        SDL_RenderCopy(instance->renderer, instance->brick, NULL, &platRect);
    }
}

void drawEnemies(Instance *instance)
{
    for(int i = 0; i < 5; i++)
    {
        SDL_Rect oneEnemy = { instance->enemy[i].x + instance->scrollX - instance->offset, instance->enemy[i].y - 200, 200, 200 };
        SDL_Rect bigEnemy = { 0, 0, 64, 64 };
        SDL_RenderCopy(instance->renderer, instance->enemyTexture, &bigEnemy, &oneEnemy);
    }
}

/**
draws everything loaded
**/
void render(SDL_Renderer *renderer, Instance *instance)
{
    SDL_SetRenderDrawColor(renderer, 80, 128, 128, 255);

    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    drawPlatforms(instance);

    drawEnemies(instance);

    SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL;

    /**
    draw boi
    **/
    SDL_Rect oneBoi = { instance->scrollX+instance->player.x - instance->offset, instance->player.y, 90, 140 };
    SDL_Rect bigBoi = { instance->player.animation, 0, 100, 158 };

    SDL_Rect textRect = { instance->scrollX +instance->bullet.x - instance->offset, instance->bullet.y + 15, 30, 40 };

    if(instance->player.flip == 1)
    {
        if(instance->player.facingLeft == 1)
        {
            instance->player.degrees -= 12;
            if(instance->player.degrees == -360)
            {
                instance->player.degrees = 0;
                instance->player.flip = 0;
            }
            SDL_RenderCopyEx(renderer, instance->datboi, &bigBoi, &oneBoi, instance->player.degrees, NULL, 0);
        }
        else
        {
            instance->player.degrees += 12;
            if(instance->player.degrees == 360)
            {
                instance->player.degrees = 0;
                instance->player.flip = 0;
            }
            SDL_RenderCopyEx(renderer, instance->datboi, &bigBoi, &oneBoi, instance->player.degrees, NULL, flip);
        }
        SDL_RenderCopyEx(renderer, instance->boiText, NULL, &textRect, instance->player.degrees, NULL, 0);
    }
    else
    {
        if(instance->player.facingLeft == 0)
        {
            SDL_RenderCopyEx(renderer, instance->datboi, &bigBoi, &oneBoi, 0, NULL, flip);
            SDL_RenderCopyEx(renderer, instance->boiText, NULL, &textRect, instance->player.degrees, NULL, 0);
        }
        else
        {
            SDL_RenderCopy(renderer, instance->datboi, &bigBoi, &oneBoi);
            SDL_RenderCopy(renderer, instance->boiText, NULL, &textRect); //BOI TEXT
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int arg, char *agrv[])
{
    Instance instance;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);

    TTF_Init();

    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);

    window = SDL_CreateWindow(
    "The Birth of Dat Boi",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    screenWidth,
    screenHeight,
    0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    instance.renderer = renderer;

    loadGame(&instance);

    int done = 0;

    while(!done)
    {
        done = processEvents(window, &instance);
        collisionDetect(&instance);
        move(&instance);
        render(renderer, &instance);
    }

    Mix_FreeChunk(instance.hitmarker);
    Mix_FreeChunk(instance.jump);
    Mix_FreeChunk(instance.ohgod);
    Mix_FreeChunk(instance.shots);
    Mix_FreeChunk(instance.ribbit);
    Mix_FreeMusic(instance.music);
    SDL_DestroyTexture(instance.message);
    SDL_DestroyTexture(instance.boiText);
    SDL_DestroyTexture(instance.datboi);
    SDL_DestroyTexture(instance.brick);
    SDL_DestroyTexture(instance.enemyTexture);
    SDL_DestroyTexture(instance.texture);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    Mix_CloseAudio();
    TTF_Quit();
    return 0;
}


