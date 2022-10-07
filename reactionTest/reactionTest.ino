#define SERIALSPEED 9600
#define NUMTRIES 3
#define NUMUSERS 3
#define TESTINGPIN 5
#define STARTPIN 3
#define TESTINGLED 13
#define STATUSLED 2
#define RSEEDPORT A0
#define MINDELAY 500
#define MAXDELAY 5000

struct player {
    uint32_t reacTimes[NUMTRIES];
    uint32_t meanReacTime;
    uint32_t bestReacTime;
};

struct bestVal {
    uint32_t val;
    uint8_t playerIdx;
};

void setup(void)
{
    Serial.begin(SERIALSPEED);
    pinMode(TESTINGPIN, INPUT_PULLUP);
    pinMode(TESTINGLED, OUTPUT);
    pinMode(STARTPIN, INPUT_PULLUP);
    pinMode(STATUSLED, OUTPUT);
}

void waitStart(void)
{
    digitalWrite(STATUSLED, HIGH);
    while (digitalRead(STARTPIN) == HIGH);
    digitalWrite(STATUSLED, LOW);
}

void waitRandom(void)
{
    randomSeed(analogRead(RSEEDPORT));
    delay(random(MINDELAY, MAXDELAY));
}

uint32_t getReacTime(void)
{
    uint32_t tstart, tstop;

    waitRandom();

    tstart = millis();
    digitalWrite(TESTINGLED, HIGH);

    // Wait while TESTINGPIN is pulled low
    // since this means player is cheating
    while (digitalRead(TESTINGPIN) == LOW);

    // Wait until player pulls TESTINGPIN low
    while (digitalRead(TESTINGPIN) == HIGH);

    tstop = millis();
    digitalWrite(TESTINGLED, LOW);
    return tstop - tstart;
}

struct player testPlayerReacTime(struct player locPlayer)
{
    for (int i = 0; i < NUMTRIES; i++) {
        locPlayer.reacTimes[i] = getReacTime();
    }
    return locPlayer;
}

uint32_t getMean(uint32_t rTimes[NUMTRIES])
{
    uint32_t sum = 0;
    for (int i = 0; i < NUMTRIES; i++) {
        sum += rTimes[i];
    }
    return sum / NUMTRIES;
}

uint32_t getBest(uint32_t rTimes[NUMTRIES])
{
    uint32_t curBest = 0xffffffff;
    for (int i = 0; i < NUMTRIES; i++) {
        if (rTimes[i] < curBest)
            curBest = rTimes[i];
    }
    return curBest;
}

struct bestVal getBestMean(struct player players[NUMUSERS])
{
    struct bestVal curBest = {.val = 0xffffffff, .playerIdx = 0};
    for (int p = 0; p < NUMUSERS; p++) {
        if (players[p].meanReacTime < curBest.val) {
            curBest.val = players[p].meanReacTime;
            curBest.playerIdx = p;
        }
    }
    return curBest;
}

struct bestVal getBestReacTime(struct player players[NUMUSERS])
{
    struct bestVal curBest = {.val = 0xffffffff, .playerIdx = 0};
    for (int p = 0; p < NUMUSERS; p++) {
        if (players[p].bestReacTime < curBest.val) {
            curBest.val = players[p].bestReacTime;
            curBest.playerIdx = p;
        }
    }
    return curBest;
}

void printPlayerInfo(struct player locPlayer)
{
    Serial.println("Player results");
    for (int i = 0; i < NUMTRIES; i++) {
        Serial.print("Try #");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(locPlayer.reacTimes[i]);
    }
    Serial.print("Mean reaction time: ");
    Serial.println(locPlayer.meanReacTime);
    Serial.print("Best reaction time: ");
    Serial.println(locPlayer.bestReacTime);
}

void printGameInfo(struct bestVal bestMean, struct bestVal bestReacTime)
{
    Serial.println("Overall game info");
    Serial.print("Best mean reaction time is player ");
    Serial.print(bestMean.playerIdx);
    Serial.print(" with a mean reaction time of ");
    Serial.println(bestMean.val);
    Serial.print("Best singular reaction time is player ");
    Serial.print(bestReacTime.playerIdx);
    Serial.print(" with a reaction time of ");
    Serial.println(bestReacTime.val);
}

void loop(void)
{
    struct player players[NUMUSERS];

    for (int p = 0; p < NUMUSERS; p++) {
        Serial.print("Player ");
        Serial.print(p);
        Serial.println(", get ready!");
        waitStart();
        players[p] = testPlayerReacTime(players[p]);
        players[p].meanReacTime = getMean(players[p].reacTimes);
        players[p].bestReacTime = getBest(players[p].reacTimes);
        printPlayerInfo(players[p]);
    }

    struct bestVal bestMean = getBestMean(players);
    struct bestVal bestReacTime = getBestReacTime(players);

    printGameInfo(bestMean, bestReacTime);

    Serial.println("Starting over...");
}
