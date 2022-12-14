#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define PIN_ANALOG_INPUT A0
#define PIR_PIN D7
#define PIN_DFPLAYER_RX D5
#define PIN_DFPLAYER_TX D6

#define MIN_VOLUME 10
#define MAX_VOLUME 20

SoftwareSerial mySoftwareSerial(PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
DFRobotDFPlayerMini myDFPlayer;

unsigned long getRandomSeed() {
    const unsigned long loop = analogRead(PIN_ANALOG_INPUT);

    unsigned long seed = 1;

    Serial.println(loop);
    for (unsigned long n=1; n < loop; n++) {
        seed = seed + analogRead(PIN_ANALOG_INPUT);
        Serial.println(seed);
    }

    return seed;
}

unsigned int numberOfAudioFiles;

void setup () {
    mySoftwareSerial.begin(9600);
    Serial.begin(115200);

    Serial.println();
    Serial.println(F("Birdhouse ESP8266"));

    pinMode(PIR_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.print(F("Creating random seed..."));
    randomSeed(getRandomSeed());
    Serial.println(F("Done"));

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
}

void waitSignalFromDistanceSensor() {
    bool notCloseEnough = true;

    Serial.print("Waiting for PIR...");

    while (notCloseEnough) {
        if (digitalRead(PIR_PIN) == HIGH) {
            notCloseEnough = false;
        }
        Serial.print(".");
        delay(500);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    Serial.println("Detected");
}

void loop () {
    const int currentAudio = random(1, numberOfAudioFiles);
    const int currentVolume = random(MIN_VOLUME, MAX_VOLUME);

    waitSignalFromDistanceSensor();

    myDFPlayer.volume(currentVolume);
    myDFPlayer.play(currentAudio);

    Serial.print("Waiting for player...");
    digitalWrite(LED_BUILTIN, LOW);

    while (myDFPlayer.readState() != 0) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("Playing finished");
    digitalWrite(LED_BUILTIN, HIGH);
}