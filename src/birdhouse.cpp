#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define PIN_ANALOG_INPUT A0
#define PIR_PIN D7
#define PIN_DFPLAYER_RX D5
#define PIN_DFPLAYER_TX D6

#define WAIT_BETWEEN_PLAY_IN_SECONDS 300

#define MIN_VOLUME 3
#define MAX_VOLUME 7

SoftwareSerial mySoftwareSerial(PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
DFRobotDFPlayerMini myDFPlayer;

unsigned long getRandomSeed() {
    const unsigned long loop = 1000;

    unsigned long seed = 1;

    for (unsigned long n=1; n < loop; n++) {
        seed = seed + analogRead(PIN_ANALOG_INPUT);
    }

    return seed;
}

void printNumberAndBackspaces(unsigned int numberToPrint) {
    char numberBuffer[20];
    sprintf(numberBuffer, "%u   ", numberToPrint);

    // Pad string with backspaces and print
    char padBuffer[40];
    char padFill = '\b';
    int padLength = strlen(numberBuffer) * 2;
    Serial.printf("%s%s", numberBuffer, (char*)memset(padBuffer, padFill, padLength - strlen(numberBuffer)));
}

void waitSecondsAndBlink(unsigned long waitingTimeInSeconds) {
    unsigned long waitingEnds = millis() + (waitingTimeInSeconds * 1000);

    Serial.print("Now waiting...");

    while (millis() < waitingEnds) {
        printNumberAndBackspaces((waitingEnds - millis()) / 1000);
        delay(2000);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    Serial.println("Waiting finished");
}

int numberOfAudioFiles;

void setup () {
    mySoftwareSerial.begin(9600);
    Serial.begin(115200);

    Serial.println();
    Serial.println(F("Birdhouse ESP8266"));

    pinMode(PIR_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.print(F("Creating random seed..."));
    unsigned long newRandomSeed = getRandomSeed();
    randomSeed(newRandomSeed);
    Serial.println(newRandomSeed);

    Serial.print(F("Initializing DFPlayer ... (May take 3~5 seconds) ... "));

    if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
        Serial.println("");
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        while(true);
    }
    Serial.println(F(" OK"));

    numberOfAudioFiles = myDFPlayer.readFileCounts();
    Serial.print(F("Number of audio files: "));
    Serial.println(numberOfAudioFiles);
}

void waitSignalFromDistanceSensor() {
    bool notCloseEnough = true;

    Serial.print("Waiting for PIR...");

    unsigned long waitingStarted = millis();

    while (notCloseEnough) {
        if (digitalRead(PIR_PIN) == HIGH) {
            notCloseEnough = false;
        }

        printNumberAndBackspaces((millis() - waitingStarted) / 1000);

        delay(500);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    Serial.println("Detected");
}

void loop () {
    const int currentAudio = random(1, numberOfAudioFiles);
    const int currentVolume = random(MIN_VOLUME, MAX_VOLUME);

    waitSignalFromDistanceSensor();

    Serial.printf("Playing audio %d/%d with volume %d...", currentAudio, numberOfAudioFiles, currentVolume);

    myDFPlayer.volume(currentVolume);
    myDFPlayer.play(currentAudio);

    digitalWrite(LED_BUILTIN, LOW);

    unsigned long playingStarted = millis();

    while (myDFPlayer.readState() != 0) {
        printNumberAndBackspaces((millis() - playingStarted) / 1000);
        delay(1000);
    }

    Serial.println("Playing finished");
    digitalWrite(LED_BUILTIN, HIGH);
    myDFPlayer.volume(0);

    waitSecondsAndBlink(WAIT_BETWEEN_PLAY_IN_SECONDS);
}
