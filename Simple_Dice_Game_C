#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void addCredits(int *credits);
void placeBet(int *credits, int *bet);
void rollDice(int *credits, int *bet);
void playAgain(int *credits, int *bet);

int main()
{
    srand(time(0));  // Seed the random number generator once
    int credits = 0;
    int bet = 0;
    printf(" --------------------------------\n");
    printf("|     Welcome to dice game       |\n");
    printf("|  Please insert credits to play |\n");
    printf(" --------------------------------\n\n");
    addCredits(&credits);
    placeBet(&credits, &bet);
    rollDice(&credits, &bet);
    playAgain(&credits, &bet);
    
    return 0;
}

void addCredits(int *pcredits)
{
    int tempCredits;
    printf("How many credits would you like to enter?\n");
    scanf("%d", &tempCredits);
    while (tempCredits <= 0 || tempCredits > 10000) // Example upper limit
    {
        printf("Please enter a valid credit amount (1-10000)\n");
        scanf("%d", &tempCredits);
    }
    *pcredits = tempCredits;
    printf("You have %d credits\n", *pcredits);
}

void placeBet(int *pcredits, int *pbet)
{
    printf("Please place your bet: ");
    scanf("%d", pbet);
    while (*pbet > *pcredits || *pbet <= 0)
    {
        printf("Invalid bet. Insufficient credits or bet is 0.\n");
        printf("Please enter a valid bet: ");
        scanf("%d", pbet);
    }
    printf("Your bet is %d\n", *pbet);
}

void rollDice(int *pcredits, int *pbet)
{
    int cpuRoll = (rand() % 6) + 1;
    int humanRoll = (rand() % 6) + 1;
    printf("\n--------------------------------------\n");
    printf("CPU roll is %d \n", cpuRoll);
    printf("Your roll is %d \n", humanRoll);

    if (cpuRoll > humanRoll)
    {
        *pcredits -= *pbet;
        printf("You lost %d credits\n", *pbet);
    }
    else if (humanRoll > cpuRoll)
    {
        *pcredits += *pbet;
        printf("You won %d credits\n", *pbet);
    }
    else
    {
        printf("Tied CPU. Credits unchanged\n");
    }

    printf("You now have %d credits\n\n", *pcredits);

    if (*pcredits <= 0)
    {
        printf("You have no more credits. Game over!\n");
        exit(0);
    }
}

void playAgain(int *pcredits, int *pbet)
{
    int play;
    printf("Press 1 to play again\n");
    printf("Press any other key to cash out and exit\n");
    scanf("%d", &play);
    while (play == 1)
    {
        placeBet(pcredits, pbet);
        rollDice(pcredits, pbet);
        printf("Press 1 to play again\n");
        printf("Press any other key to cash out and exit\n");
        scanf("%d", &play);
    }

    printf("Cashing out with %d credits\n", *pcredits);
    exit(0);
}
