#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NUM_INGREDIENTS 9
#define NUM_RECIPES 5

typedef struct {
    int flour;
    int sugar;
    int yeast;
    int bakingSoda;
    int salt;
    int cinnamon;
    int egg;
    int milk;
    int butter;
} Ingredients;

typedef struct {
    sem_t mixer;
    sem_t pantry;
    sem_t refrigerator;
    sem_t bowl;
    sem_t spoon;
    sem_t oven;
    sem_t ingredientMutex[NUM_INGREDIENTS];
} Resources;

sem_t recipeLock;

typedef struct {
    int id;
    Ingredients ingredients;
    Resources *kitchen;
    int cookedRecipes[NUM_RECIPES];
} Baker;

typedef struct {
    const char *name;
    const char *ingredients[9];
} Recipe;

Recipe recipes[NUM_RECIPES] = {
    {"Cookies", {"Flour", "Sugar", "Milk", "Butter", NULL}},
    {"Pancakes", {"Flour", "Sugar", "Baking soda", "Salt", "Egg", "Milk", "Butter", NULL}},
    {"Homemade pizza dough", {"Yeast", "Sugar", "Salt", NULL}},
    {"Soft Pretzels", {"Flour", "Sugar", "Salt", "Yeast", "Baking Soda", "Egg", NULL}},
    {"Cinnamon rolls", {"Flour", "Sugar", "Salt", "Butter", "Egg", "Cinnamon", NULL}},
};

void *bakerThread(void *bakerPtr);
void acquireIngredients(sem_t *pantryMutex, sem_t *refrigeratorMutex, const char **ingredientNames, int numIngredients, Baker *baker, const char *color);
void mixIngredients(Baker *baker, const char *color);
void cookRecipe(Baker *baker, Recipe *recipe, const char *color);
void announceFinished(Baker *baker, const char *color, int allRecipesFinished);

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

int main() {
    Resources kitchen;
    sem_init(&kitchen.mixer, 0, 1);
    sem_init(&kitchen.pantry, 0, 1);
    sem_init(&kitchen.refrigerator, 0, 2);
    sem_init(&kitchen.bowl, 0, 3);
    sem_init(&kitchen.spoon, 0, 5);
    sem_init(&kitchen.oven, 0, 1);

    for (int i = 0; i < NUM_INGREDIENTS; i++) {
        sem_init(&kitchen.ingredientMutex[i], 0, 1);
    }

    sem_init(&recipeLock, 0, 1);

    srand(time(NULL));

    int numBakers;
    printf("Enter the number of bakers: ");
    scanf("%d", &numBakers);

    pthread_t bakerThreads[numBakers];
    Baker bakers[numBakers];
    for (int i = 0; i < numBakers; i++) {
        bakers[i].id = i + 1;
        bakers[i].kitchen = &kitchen;
        for (int j = 0; j < NUM_RECIPES; j++) {
            bakers[i].cookedRecipes[j] = 0;
        }
        pthread_create(&bakerThreads[i], NULL, bakerThread, &bakers[i]);
    }

    for (int i = 0; i < numBakers; i++) {
        pthread_join(bakerThreads[i], NULL);
        announceFinished(&bakers[i], COLOR_RESET, i == numBakers - 1);
    }

    sem_destroy(&kitchen.mixer);
    sem_destroy(&kitchen.pantry);
    sem_destroy(&kitchen.refrigerator);
    sem_destroy(&kitchen.bowl);
    sem_destroy(&kitchen.spoon);
    sem_destroy(&kitchen.oven);

    for (int i = 0; i < NUM_INGREDIENTS; i++) {
        sem_destroy(&kitchen.ingredientMutex[i]);
    }

    sem_destroy(&recipeLock);

    return 0;
}

void *bakerThread(void *bakerPtr) {
    Baker *baker = (Baker *)bakerPtr;
    Ingredients *ingredients = &baker->ingredients;

    const char *color;
    if (baker->id == 1) {
        color = COLOR_RED;
    } else if (baker->id == 2) {
        color = COLOR_BLUE;
    } else if (baker->id == 3) {
        color = COLOR_GREEN;
    } else {
        color = COLOR_RESET;
    }

    for (int i = 0; i < NUM_RECIPES; i++) {
        printf("%sBaker %d started cooking %s.\n" COLOR_RESET, color, baker->id, recipes[i].name);
        acquireIngredients(&baker->kitchen->pantry, &baker->kitchen->refrigerator, recipes[i].ingredients, 4, baker, color);
        mixIngredients(baker, color);
        cookRecipe(baker, &recipes[i], color);
        baker->cookedRecipes[i] = 1;
        printf("%sBaker %d finished cooking %s.\n" COLOR_RESET, color, baker->id, recipes[i].name);
        announceFinished(baker, color, i == NUM_RECIPES - 1);
    }

    return NULL;
}

void acquireIngredients(sem_t *pantryMutex, sem_t *refrigeratorMutex, const char **ingredientNames, int numIngredients, Baker *baker, const char *color) {
    sem_wait(&recipeLock);

    int pantryEntered = 0;
    int fridgeEntered = 0;

    for (int i = 0; i < numIngredients && ingredientNames[i] != NULL; i++) {
        if (strcmp(ingredientNames[i], "Milk") == 0 || strcmp(ingredientNames[i], "Butter") == 0) {
            if (!fridgeEntered) {
                printf("%sBaker %d is entering the refrigerator.\n" COLOR_RESET, color, baker->id);
                fridgeEntered = 1;
            }
            sem_wait(refrigeratorMutex);
            printf("%sBaker %d acquired %s from the refrigerator.\n" COLOR_RESET, color, baker->id, ingredientNames[i]);
            sem_post(refrigeratorMutex);
        } else {
            if (!pantryEntered) {
                printf("%sBaker %d is entering the pantry.\n" COLOR_RESET, color, baker->id);
                pantryEntered = 1;
            }
            sem_wait(pantryMutex);
            printf("%sBaker %d acquired %s from the pantry.\n" COLOR_RESET, color, baker->id, ingredientNames[i]);
            sem_post(pantryMutex);
        }
        sleep(1);
    }

    if (pantryEntered) {
        printf("%sBaker %d is leaving the pantry.\n" COLOR_RESET, color, baker->id);
    }

    if (fridgeEntered) {
        printf("%sBaker %d is leaving the refrigerator.\n" COLOR_RESET, color, baker->id);
    }

    sem_post(&recipeLock);
}

void mixIngredients(Baker *baker, const char *color) {
    sem_wait(&baker->kitchen->bowl);
    sem_wait(&baker->kitchen->spoon);

    printf("%sBaker %d is using the bowl and spoon for mixing.\n" COLOR_RESET, color, baker->id);
    sem_wait(&baker->kitchen->mixer);

    printf("%sBaker %d is using the mixer for mixing.\n" COLOR_RESET, color, baker->id);
    sleep(2);

    sem_post(&baker->kitchen->mixer);
    sem_post(&baker->kitchen->spoon);
    sem_post(&baker->kitchen->bowl);

    printf("%sBaker %d is done with the mixer, spoon, and bowl.\n" COLOR_RESET, color, baker->id);
}

void cookRecipe(Baker *baker, Recipe *recipe, const char *color) {
    sleep(3);

    int ingredientCount = 0;
    sem_wait(&baker->kitchen->oven);

    printf("%sBaker %d is using the oven for cooking.\n" COLOR_RESET, color, baker->id);

    sem_post(&baker->kitchen->oven);

    printf("%sBaker %d is done with the oven.\n" COLOR_RESET, color, baker->id);
}

void announceFinished(Baker *baker, const char *color, int allRecipesFinished) {
    if (allRecipesFinished) {
        printf("%sBaker %d has finished baking.\n" COLOR_RESET, color, baker->id);
    }
}

