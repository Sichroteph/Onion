#include "./activity_tracker.h"

static bool quit = false;

static void sigHandler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            quit = true;
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    log_setName("activity_tracker");
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();
    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    TTF_Font *font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    TTF_Font *font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
    TTF_Font *fontRomName25 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 25);
    SDL_Color color_white = {255, 255, 255};
    SDL_Color color_lilla = {136, 97, 252};
    SDL_Surface *imageBackground = IMG_Load("./res/background.png");
    SDL_Surface *imageRomPosition;
    SDL_Surface *imageRomImage;
    SDL_Surface *imageRomPlayTime;
    SDL_Surface *imageRomName;
    SDL_Surface *imagePages;
    SDL_Surface *imageMileage;
    SDL_BlitSurface(imageBackground, NULL, screen, NULL);
    SDL_Rect rectPages = {561, 430, 90, 44};
    SDL_Rect rectMileage = {484, 8, 170, 42};
    PlayActivities *play_activities = play_activity_find_all();
    int h, m;
    int nPages = (int)ceil((double)play_activities->count / (double)4);
    int nCurrentPage = 0;
    char *cPosition = (char *)malloc(5);
    char *cPages = (char *)malloc(10);
    sprintf(cPages, "%d/%d", nCurrentPage + 1, nPages);
    imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
    int play_time_total = play_activities->play_time_total;
    char *play_time_total_formatted =(char *)malloc(50);
    for (int i = 0; i < 4; i++) {
        int index = nCurrentPage * 4 + i;
        if (index < play_activities->count) {
            sprintf(cPosition, "%d", index + 1);
            h = play_activities->play_activity[index]->play_time_total / 3600;
            m = (play_activities->play_activity[index]->play_time_total - 3600 * h) / 60;
            sprintf(play_time_total_formatted, "%d:%02d", h, m);
            imageRomPosition = TTF_RenderUTF8_Blended(font40, cPosition, color_lilla);
            if (exists(play_activities->play_activity[index]->rom->image_path)) {
                SDL_Surface* loadedRomImage = IMG_Load(play_activities->play_activity[index]->rom->image_path);
                imageRomImage = SDL_CreateRGBSurface(0, 96, 96, loadedRomImage->format->BitsPerPixel, loadedRomImage->format->Rmask, loadedRomImage->format->Gmask, loadedRomImage->format->Bmask, loadedRomImage->format->Amask);
                SDL_Rect src_rect = {0, 0, loadedRomImage->w, loadedRomImage->h};
                SDL_Rect dst_rect = {0, 0, imageRomImage->w, imageRomImage->h};
                SDL_SoftStretch(loadedRomImage, &src_rect, imageRomImage, &dst_rect);
                SDL_FreeSurface(loadedRomImage);
            } else {
                imageRomImage = TTF_RenderUTF8_Blended(font40, "N/A", color_lilla);
            }
            imageRomPlayTime = TTF_RenderUTF8_Blended(font40, play_time_total_formatted, color_white);
            imageRomName = TTF_RenderUTF8_Blended(fontRomName25, play_activities->play_activity[index]->rom->name, color_white);
            SDL_Rect rectPosition = {16, 78 + 90 * i, 76, 39};
            SDL_Rect rectRomImage = {77, 66 + 90 * i, 96, 96};
            SDL_Rect rectRomPlayTime = {189, 66 + 90 * i, 100, 56};
            SDL_Rect rectRomNames = {190, 104 + 90 * i, 400, 40};
            SDL_BlitSurface(imageRomPosition, NULL, screen, &rectPosition);
            SDL_BlitSurface(imageRomImage, NULL, screen, &rectRomImage);
            SDL_BlitSurface(imageRomPlayTime, NULL, screen, &rectRomPlayTime);
            SDL_BlitSurface(imageRomName, NULL, screen, &rectRomNames);
        }
    }
    h = play_time_total / 3600;
    m = (play_time_total - (3600 * h)) / 60;
    sprintf(play_time_total_formatted, "%d:%02d", h, m);
    imageMileage = TTF_RenderUTF8_Blended(font30, play_time_total_formatted, color_white);
    SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
    SDL_BlitSurface(imageMileage, NULL, screen, &rectMileage);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    bool changed;
    KeyState keystate[320] = {(KeyState)0};
    while (!quit) {
        msleep(20);
        changed = false;
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (nCurrentPage < nPages - 1) {
                    nCurrentPage++;
                    changed = true;
                }
            }
            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (nCurrentPage > 0) {
                    nCurrentPage--;
                    changed = true;
                }
            }
        }
        if (!changed)
            continue;
        // Page update
        SDL_BlitSurface(imageBackground, NULL, screen, NULL);
        sprintf(cPages, "%d/%d", nCurrentPage + 1, nPages);
        imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
        SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
        SDL_BlitSurface(imageMileage, NULL, screen, &rectMileage);
        for (int i = 0; i < 4; i++) {
            int index = nCurrentPage * 4 + i;
            if (index < play_activities->count) {
                sprintf(cPosition, "%d", index + 1);
                h = play_activities->play_activity[index]->play_time_total / 3600;
                m = (play_activities->play_activity[index]->play_time_total - 3600 * h) / 60;
                sprintf(play_time_total_formatted, "%d:%02d", h, m);
                imageRomPosition = TTF_RenderUTF8_Blended(font40, cPosition, color_lilla);
                imageRomImage = IMG_Load(play_activities->play_activity[index]->rom->image_path);
                imageRomPlayTime = TTF_RenderUTF8_Blended(font40, play_time_total_formatted, color_white);
                imageRomName = TTF_RenderUTF8_Blended(fontRomName25, play_activities->play_activity[index]->rom->name, color_white);
                SDL_Rect rectPosition = {16, 78 + 90 * i, 76, 39};
                SDL_Rect rectRomImage = {77, 66 + 90 * i, 96, 96};
                SDL_Rect rectRomPlayTime = {77, 66 + 90 * i, 254, 56};
                SDL_Rect rectRomNames = {78, 104 + 90 * i, 600, 40};
                SDL_BlitSurface(imageRomPosition, NULL, screen, &rectPosition);
                SDL_BlitSurface(imageRomImage, NULL, screen, &rectRomImage);
                SDL_BlitSurface(imageRomPlayTime, NULL, screen, &rectRomPlayTime);
                SDL_BlitSurface(imageRomName, NULL, screen, &rectRomNames);
            }
        }
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
    return EXIT_SUCCESS;
}