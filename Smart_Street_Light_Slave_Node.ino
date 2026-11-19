// ============================================================
//  AURA — Smart Street Light  |  SLAVE NODE
//  Avenbot Technologies Private Limited
//
//  Hardware : ESP8266 + SX1278 LoRa + Relay + Button + Buzzer
//  Protocol : Compatible with Smart_Street_Light_Head_Node.ino
//
//  Button gestures (same as head node):
//    1 press  → SOS  (buzzer ON)
//    3 presses→ FAULT
//    Long (3s)→ CLEAR (buzzer OFF)
//
//  Outgoing messages  : <NODE_ID>:HEARTBEAT / SOS / FAULT / CLEAR
//                        <NODE_ID>:ACK_ON   / ACK_OFF
//  Incoming messages  : <NODE_ID>:ON  |  <NODE_ID>:OFF   (addressed)
//                        ON  |  OFF                       (broadcast)
// ============================================================

#include <SPI.h>
#include <LoRa.h>

// ── Node Identity ────────────────────────────────────────────
// Change this for every slave: "S1", "S2", "S3" …
#define NODE_ID   "S1"

// ── LoRa Pins  (same wiring as head node) ───────────────────
#define LORA_SS    15   // D8
#define LORA_RST   16   // D0
#define LORA_DIO0   4   // D2

// ── Peripheral Pins ──────────────────────────────────────────
#define RELAY_PIN   5   // D1  — controls streetlight
#define BUTTON_PIN  0   // D3  — panic / SOS button (INPUT_PULLUP)
#define BUZZER_PIN  2   // D4  — emergency alert buzzer

// ── Timing ───────────────────────────────────────────────────
#define HEARTBEAT_INTERVAL   5000UL   // ms  — matches head node
#define PRESS_WINDOW         1000UL   // ms  — multi-press detection
#define LONG_PRESS_MS        3000UL   // ms  — CLEAR gesture
#define STATUS_INTERVAL      3000UL   // ms  — Serial status print

// ── State ────────────────────────────────────────────────────
bool relayState     = false;
bool emergencyMode  = false;
bool faultMode      = false;

// ── Heartbeat ────────────────────────────────────────────────
unsigned long lastHeartbeat = 0;

// ── Button state ─────────────────────────────────────────────
int           pressCount      = 0;
unsigned long firstPressTime  = 0;
unsigned long buttonPressStart = 0;
bool          buttonLastState  = HIGH;

// ============================================================
//  sendLoRa  — prepends NODE_ID so head node knows the source
// ============================================================
void sendLoRa(const String& msg)
{
  String packet = String(NODE_ID) + ":" + msg;

  LoRa.beginPacket();
  LoRa.print(packet);
  LoRa.endPacket();

  Serial.print("[TX] ");
  Serial.println(packet);
}

// ============================================================
//  setup
// ============================================================
void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=== AURA Slave Node ===");
  Serial.print  ("Node ID : ");
  Serial.println(NODE_ID);

  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN,  LOW);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin();
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6))
  {
    Serial.println("[ERROR] LoRa init FAILED — check wiring");
    while (1) delay(500);
  }

  Serial.println("[OK] LoRa ready on 433 MHz");
  Serial.println("=======================");
}

// ============================================================
//  receiveLoRa — parse addressed + broadcast commands
// ============================================================
void receiveLoRa()
{
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  String raw = "";
  while (LoRa.available())
    raw += (char)LoRa.read();

  Serial.print("[RX] ");
  Serial.println(raw);

  // ── Extract the command portion ──────────────────────────
  // Accept:
  //   "S1:ON"  → addressed to this node
  //   "ON"     → broadcast (no prefix, no colon)
  // Ignore:
  //   "S2:ON"  → addressed to a different slave

  String myPrefix = String(NODE_ID) + ":";
  String cmd      = "";

  if (raw.startsWith(myPrefix))
  {
    // Addressed to us
    cmd = raw.substring(myPrefix.length());
  }
  else if (raw.indexOf(':') == -1)
  {
    // Broadcast — no colon means no address prefix
    cmd = raw;
  }
  else
  {
    // Addressed to another node — ignore silently
    return;
  }

  // ── Execute command ──────────────────────────────────────
  cmd.trim();

  if (cmd == "ON")
  {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH);
    sendLoRa("ACK_ON");
    Serial.println("[INFO] Relay ON");
  }
  else if (cmd == "OFF")
  {
    relayState = false;
    digitalWrite(RELAY_PIN, LOW);
    sendLoRa("ACK_OFF");
    Serial.println("[INFO] Relay OFF");
  }
  else
  {
    Serial.print("[WARN] Unknown command: ");
    Serial.println(cmd);
  }
}

// ============================================================
//  handleButton — 1 press = SOS, 3 presses = FAULT, long = CLEAR
// ============================================================
void handleButton()
{
  bool state = digitalRead(BUTTON_PIN);

  // ── Rising edge: press starts ────────────────────────────
  if (buttonLastState == HIGH && state == LOW)
  {
    buttonPressStart = millis();

    if (pressCount == 0)
      firstPressTime = millis();

    pressCount++;
  }

  // ── Long press held for 3 s: CLEAR ──────────────────────
  if (state == LOW && (millis() - buttonPressStart) > LONG_PRESS_MS)
  {
    emergencyMode = false;
    faultMode     = false;

    digitalWrite(BUZZER_PIN, LOW);

    sendLoRa("CLEAR");
    Serial.println("[INFO] CLEAR sent");

    // Wait until button is released
    while (digitalRead(BUTTON_PIN) == LOW)
      delay(10);

    pressCount = 0;
  }

  // ── Evaluate press count after the detection window ─────
  if (pressCount > 0 && (millis() - firstPressTime) > PRESS_WINDOW)
  {
    if (pressCount == 1)
    {
      emergencyMode = true;
      digitalWrite(BUZZER_PIN, HIGH);
      sendLoRa("SOS");
      Serial.println("[ALERT] SOS sent");
    }
    else if (pressCount >= 3)
    {
      faultMode = true;
      sendLoRa("FAULT");
      Serial.println("[WARN] FAULT sent");
    }
    // 2 presses: ignored (matches head node behaviour)

    pressCount = 0;
  }

  buttonLastState = state;
}

// ============================================================
//  sendHeartbeat — every 5 s (matches head node interval)
// ============================================================
void sendHeartbeat()
{
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL)
  {
    sendLoRa("HEARTBEAT");
    lastHeartbeat = millis();
  }
}

// ============================================================
//  printStatus — Serial debug every 3 s
// ============================================================
void printStatus()
{
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > STATUS_INTERVAL)
  {
    Serial.println();
    Serial.println("===== SLAVE STATUS =====");
    Serial.print  ("Node  : "); Serial.println(NODE_ID);
    Serial.print  ("Relay : "); Serial.println(relayState    ? "ON"  : "OFF");
    Serial.print  ("SOS   : "); Serial.println(emergencyMode ? "YES" : "NO");
    Serial.print  ("Fault : "); Serial.println(faultMode     ? "YES" : "NO");
    Serial.println("========================");

    lastPrint = millis();
  }
}

// ============================================================
//  loop
// ============================================================
void loop()
{
  receiveLoRa();
  handleButton();
  sendHeartbeat();
  printStatus();
}
