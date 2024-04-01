#define BLYNK_TEMPLATE_ID "TMPL6UkXTp-zf"
#define BLYNK_TEMPLATE_NAME "PZEM  Project"
#define BLYNK_AUTH_TOKEN "Dh8IReGZ5HxDhofHcWqsXALSnNwoGhsw"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);


#include <PZEM004Tv30.h>


const char ssid[] = "s"; 
const char pass[] = "12345678";

   // change Gscript ID


// Timer for regular updates
BlynkTimer timer;

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif


#if defined(ESP32)
/*************************
 *  ESP32 initialization
 * ---------------------
 * 
 * The ESP32 HW Serial interface can be routed to any GPIO pin 
 * Here we initialize the PZEM on Serial2 with RX/TX pins 16 and 17
 */
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)
/*************************
 *  ESP8266 initialization
 * ---------------------
 * 
 * Not all Arduino boards come with multiple HW Serial ports.
 * Serial2 is for example available on the Arduino MEGA 2560 but not Arduino Uno!
 * The ESP32 HW Serial interface can be routed to any GPIO pin 
 * Here we initialize the PZEM on Serial2 with default pins
 */
//PZEM004Tv30 pzem(Serial1);
#else
/*************************
 *  Arduino initialization
 * ---------------------
 * 
 * Not all Arduino boards come with multiple HW Serial ports.
 * Serial2 is for example available on the Arduino MEGA 2560 but not Arduino Uno!
 * The ESP32 HW Serial interface can be routed to any GPIO pin 
 * Here we initialize the PZEM on Serial2 with default pins
 */
PZEM004Tv30 pzem(PZEM_SERIAL);
#endif
   // change Gscript ID

// Variables for energy calculation
float kWh = 0.000000;
unsigned long lastMillis = millis();

String GOOGLE_SCRIPT_ID = "AKfycbyW563HAe2Wk9qpgO8ectUuURNzUi8JfWX1uqA6hvCE_5tmcABjkGyi2AUOvdbZPtpf";    // change Gscript ID


void setup() {
    // Debugging Serial port
    Serial.begin(115200);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Initialize the LCD
    lcd.begin();
    lcd.backlight();

    // Uncomment in order to reset the internal energy counter
    // pzem.resetEnergy()
}

void loop() {
    // Print the custom address of the PZEM
    Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();


 

    float unit = 0.000000 ;
     unit = kWh - 0.0;

    float cost = 0;
     if (unit < 32)
     {
        cost = unit*38;
     }
  /*   else if (32<unit=<64)
     {
        cost = unit
     }
*/
    // Check if the data is valid
    if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
    } else {

  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();
  kWh += power * (currentMillis - lastMillis) / 3600000000.0;
  lastMillis = currentMillis;

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(kWh, 4);        Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);
        Serial.print("Unit: ");         Serial.println(unit, 4); 
        Serial.print("Cost: ");         Serial.print("Rs");           Serial.println(cost,4);


    }

    String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"voltage="+ String(voltage)+ "&current=" + String(current)+ "&power=" + String(power)+ "&kWh=" + String(kWh, 4)+ "&unit=" + String(unit, 4)+ "&cost=" + String(cost, 4);
    Serial.print("POST data to spreadsheet:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
    }
    //---------------------------------------------------------------------
    http.end();
  delay(1000);

    
      // Send data to Blynk
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, kWh );
  Blynk.virtualWrite(V7, unit);
  Blynk.virtualWrite(V8, cost);


  // Update the LCD with the new values
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vrms: ");
  lcd.print(voltage, 2);
  lcd.print(" V");

  lcd.setCursor(0, 1);
  lcd.print("Irms: ");
  lcd.print(current, 4);
  lcd.print(" A");

  delay(1000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power: ");
  lcd.print(power, 4);
  lcd.print(" W");

  lcd.setCursor(0, 1);
  lcd.print("kWh  : ");
  lcd.print(kWh, 4);
  lcd.print("kWh");

  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unit : ");
  lcd.print(unit, 4);
  
  lcd.setCursor(0, 1);
  lcd.print("Cost:");
  lcd.print("RS ");
  lcd.print(cost, 4);
  delay(1000);

  Serial.println();
  Blynk.run();
  timer.run();
  
}
