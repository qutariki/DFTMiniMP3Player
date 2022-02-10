/******************************************
  Written by: Kemalcan Küçük
  DFTPlayerMini Functions by: Nick Koumaris
  DFMiniMp3 Library by: Michael Miller
*******************************************/
#include <LEDDisplay.h>
LEDDisplay *led;
#include <stdlib.h>
#include <Encoder.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(8, 9); //DFMP3 Player RX, TX pins.
Encoder myEnc(11, 12); //Encoder RX, TX pins.

# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

boolean isPlaying = false;
boolean isPaused = false;
int encoderButtonPause = 10;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(encoderButtonPause, INPUT);
  digitalWrite(encoderButtonPause, HIGH);

  int digitFlagPins[] = {A2, A3};
  int segmentPins[] = {2, 3, 4, 5 , 6 , 13 , A0, A1};
  int decimalPointPin = 9;
  led = new LEDDisplay(2, digitFlagPins, segmentPins, decimalPointPin);
  setVolume(10);
  setPlaybackMode(); //Playback on repeat
}

long oldPosition  = -999;

void loop() {
  long newPosition = (myEnc.read() / 4) % 100;
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition); //To check current song number
    playByNumber(newPosition);
  }
  displayTwo(newPosition);
  if (digitalRead(encoderButtonPause) == LOW)
  {
    if (isPlaying)
    {
      pause();
      isPlaying = false;
      isPaused = true;
    } else
    {
      isPlaying = true;
      isPaused = false;
      play();
    }
  }
}


void displayTwo(int num) {
  div_t quotient = div(num, 10);
  int rightDigit = quotient.rem;
  int leftDigit = quotient.quot;
  led->displayNumber(rightDigit, 0);
  led->displayNumber(leftDigit, 1);
}


void playFirst() { //Test function by the original creator.
  execute_CMD(0x3F, 0, 0);
  delay(500);
  setVolume(10);
  delay(500);
  execute_CMD(0x11, 0, 1);
  delay(500);
  isPlaying = true;
}

void playByNumber(int num) {
  execute_CMD(0x3F, 0, 0);
  delay(500);
  execute_CMD(0x3, 0, num);
  delay(500);
  isPlaying = true;
}
void setPlaybackMode() {
  execute_CMD(0x8, 0, 1); //Repeat/folder repeat/single repeat/ random => (0/1/2/3)
  delay(500);
}
void pause()
{
  execute_CMD(0x0E, 0, 0);
  delay(500);
}

void play()
{
  execute_CMD(0x0D, 0, 1);
  delay(500);
}

void playNext()
{
  execute_CMD(0x01, 0, 1);
  delay(500);
}

void playPrevious()
{
  execute_CMD(0x02, 0, 1);
  delay(500);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
{
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                            Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte
                          };
  for (byte i = 0; i < 10; i++)
  {
    mySerial.write( Command_line[i]);
  }
}
