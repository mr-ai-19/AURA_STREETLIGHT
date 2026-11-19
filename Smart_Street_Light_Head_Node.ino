#include <SPI.h>
#include <LoRa.h>

// ================= LORA =================
#define LORA_SS    15   // D8
#define LORA_RST   16   // D0
#define LORA_DIO0   4   // D2

// ================= PINS =================
#define RELAY_PIN   5   // D1
#define BUTTON_PIN  0   // D3
#define BUZZER_PIN  2   // D4

// ================= VARIABLES =================
bool relayState = false;
bool emergencyMode = false;
bool faultMode = false;

unsigned long lastHeartbeat = 0;

int pressCount = 0;
unsigned long firstPressTime = 0;
unsigned long buttonPressStart = 0;

bool buttonLastState = HIGH;

// ==================================================

void sendLoRa(String msg)
{
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();

  Serial.print("TX: ");
  Serial.println(msg);
}

// ==================================================

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin();

  LoRa.setPins(
    LORA_SS,
    LORA_RST,
    LORA_DIO0
  );

  if (!LoRa.begin(433E6))
  {
    Serial.println("LoRa FAIL");

    while (1);
  }

  Serial.println("LoRa READY");
}

// ==================================================

void handleButton()
{
  bool state = digitalRead(BUTTON_PIN);

  // Button pressed
  if (buttonLastState == HIGH && state == LOW)
  {
    buttonPressStart = millis();

    if (pressCount == 0)
    {
      firstPressTime = millis();
    }

    pressCount++;
  }

  // Long press
  if (state == LOW &&
      millis() - buttonPressStart > 3000)
  {
    emergencyMode = false;
    faultMode = false;

    digitalWrite(BUZZER_PIN, LOW);

    sendLoRa("CLEAR");

    Serial.println("CLEAR SENT");

    while (digitalRead(BUTTON_PIN) == LOW)
      delay(10);

    pressCount = 0;
  }

  // Evaluate presses after 1 second
  if (pressCount > 0 &&
      millis() - firstPressTime > 1000)
  {
    if (pressCount == 1)
    {
      emergencyMode = true;

      digitalWrite(BUZZER_PIN, HIGH);

      sendLoRa("SOS");

      Serial.println("SOS SENT");
    }

    else if (pressCount >= 3)
    {
      faultMode = true;

      sendLoRa("FAULT");

      Serial.println("FAULT SENT");
    }

    pressCount = 0;
  }

  buttonLastState = state;
}

// ==================================================

void receiveLoRa()
{
  int packetSize = LoRa.parsePacket();

  if (!packetSize)
    return;

  String msg = "";

  while (LoRa.available())
  {
    msg += (char)LoRa.read();
  }

  Serial.print("RX: ");
  Serial.println(msg);

  if (msg == "ON")
  {
    relayState = true;

    digitalWrite(RELAY_PIN, HIGH);

    sendLoRa("ACK_ON");
  }

  else if (msg == "OFF")
  {
    relayState = false;

    digitalWrite(RELAY_PIN, LOW);

    sendLoRa("ACK_OFF");
  }
}

// ==================================================

void sendHeartbeat()
{
  if (millis() - lastHeartbeat > 5000)
  {
    sendLoRa("HEARTBEAT");

    lastHeartbeat = millis();
  }
}

// ==================================================

void printStatus()
{
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 3000)
  {
    Serial.println("\n===== SLAVE STATUS =====");

    Serial.print("Relay : ");
    Serial.println(relayState ? "ON" : "OFF");

    Serial.print("SOS : ");
    Serial.println(emergencyMode ? "YES" : "NO");

    Serial.print("Fault : ");
    Serial.println(faultMode ? "YES" : "NO");

    Serial.println("========================");

    lastPrint = millis();
  }
}

// ==================================================

void loop()
{
  receiveLoRa();

  handleButton();

  sendHeartbeat();

  printStatus();
}