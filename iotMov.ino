// Programmed by Miguel Raul Gonzalez
// July 5th 2021
//  EnergySaver Program for Arduino uno

#include <Wire.h>
#include <Adafruit_GFX.h>   // Graphic Library for Screen
#include <Adafruit_SSD1306.h>  // Monocromatic Screen 

#define ANCHO                 128
#define ALTO                  64
#define OLED_RESET            4
#define SCREEN_ADDRESS        0x3C  // Screen Address 
#define DISPARO               180  // 3 minutos sin detectar movimiento abre el relay de IOT
#define CONV_TEMP_CELCIUS     0.48828125  //Constante para convertir la lectura del sensor interno de temperatura a grados celcius

//  Definición de los puertos digitales
#define PIR_SENSOR            2   // pin conectado al sensor de movimiento
#define IOT_RELAY_PIN         4   // pin que va conectado al iotRelay
#define LED_ON_PIN            8   // pin led indica encendido el ioRelay
#define LED_MOV_PIN           11  // pin Led indica detección de movimiento
#define LED_BYPASS_PIN        10 // pin Led indica Bypass del sistema
#define BYPASS_BUTTON         3  // boton de baypass
#define INTERNAL_TEMP_SENSOR  0  // Sensor de temperatura en Analog 0   A0



Adafruit_SSD1306 oled (ANCHO,ALTO,&Wire, OLED_RESET); // Start Screen

//  Definición del sensor de temperatura y humedad

#include "DHT.h"
#define DHTTYPE DHT11   // DHT 11   type of sensor
#define DHTPIN 7 // pin que va conectado al sensor de temperatura y humedad
DHT dht(DHTPIN, DHTTYPE);



float tempA0;        // Almacena la temperatura sacada del puerto analógico A0  
int counter = 0;             // contador de segundos
volatile int reading = 0;    // variable de lectura del detector de movimiento
volatile boolean bypass = false;


void setup() {


  // -----  Distribución de los pins digitales en el Arduino UNO -----
  pinMode(IOT_RELAY_PIN, OUTPUT);
  pinMode(LED_MOV_PIN, OUTPUT);
  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_BYPASS_PIN,OUTPUT);
  pinMode(BYPASS_BUTTON,INPUT);
  pinMode(PIR_SENSOR,INPUT);
  

  // ----- Declara la rutina de interrupción que se dispara al recibir lectura 
  // ----- en el detector  que es siempre porque manda 0 si no recibe nada
  attachInterrupt(0, mov_ISR, CHANGE);


  // ----- Declara la rutina de interrupción que se dispara al presionar el botón de bypass
  // ----- para evitar que se vaya a dormir
 attachInterrupt(1, bypass_ISR, CHANGE);

 

  Serial.begin(9600);
  Serial.println("DHTxx test!");
  dht.begin();


  Wire.begin();
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  oled.display();
  delay(2000);
  oled.clearDisplay();
  oled.setTextColor(WHITE);
}
void loop() {

tempA0 = analogRead(INTERNAL_TEMP_SENSOR); // Lee la temperatura del sensor INTERNO
tempA0 =  tempA0 * CONV_TEMP_CELCIUS;   // Convierte la señal en grados celcius


if (bypass==true) {
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0,0);
        oled.setTextSize(4);
        oled.print("BYPASSS");
        oled.display();
        digitalWrite(IOT_RELAY_PIN, HIGH);
        // digitalWrite(LED_BYPASS_PIN, HIGH);  //Nunca funcionó
        delay(2000);
        
       
        oled.clearDisplay();
        oled.display();
         delay(2000);
        
       
  } else {

    // digitalWrite(LED_BYPASS_PIN, LOW);
    delay(1000);  //  Delay de un segundo en cada ciclo
    counter = counter + 1;   // Counter de intervalo en segundos

  
     if (counter > 1000){
          counter = DISPARO + 1;  // Vuelve a cero para que no vaya  a hacer un overflow
      }

    //Humidity
      float h = dht.readHumidity();
      //Temp in Celsius
      float t = dht.readTemperature();
      //Heat Index in Celsius
      float hic = dht.computeHeatIndex(t, h, false);
    
      
     if (isnan(h) || isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
     }
    
      if (counter < DISPARO) {    
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0,0);
        oled.setTextSize(1);
        oled.print("Humedad Rel%: ");
        oled.print(h);
        oled.setCursor(0,10);
        oled.print("Temp C: ");
        oled.print(t);
        oled.setCursor(0,20);
        oled.print("Sens Term C: ");
        oled.print(hic);
        oled.setCursor(0,30);
        oled.print("Temp.Interna C: ");
        oled.print((byte) tempA0);
        oled.display();
      } else  {
        oled.clearDisplay();
        oled.display();  
      }
    
      
      
      
      
      // Si el acumulador sobrepasa el intervalo de DISPARO (ver funcion de interrupción)
      // Entonces apaga el iotrelay y el le de power sino mantiene encendido el ioterelay
      if (counter > DISPARO) {  
        digitalWrite(LED_ON_PIN, LOW);  
        digitalWrite(IOT_RELAY_PIN, LOW);
        
      } else {
        digitalWrite(LED_ON_PIN, HIGH);
        digitalWrite(IOT_RELAY_PIN, HIGH);
      }
} // if bypass    
  
}  // End LOOP

// Función de interrupt que hace la lectura del sensor de movimiento
void mov_ISR (){
  
  reading = digitalRead(PIR_SENSOR);    // lee el sensor de movimiento

  // --- Si sensa movimiento mantiene el iotrelay encendido 
  if (reading==HIGH){
        digitalWrite(LED_MOV_PIN, HIGH);
        counter = 0;   // Inicializa el counter de intervalo en segundos a cero
  } else {
          digitalWrite(LED_MOV_PIN, LOW);
  }

}

// Función de interrupt que hace la lectura del boton de bypass
void bypass_ISR (){
  
  reading = digitalRead(BYPASS_BUTTON);    // verifica si el boton fue presionado

  // --- Si presiona el boton
  if (reading == LOW){
      if (bypass == false){
            bypass = true;
            counter = 0;
            } else {
              bypass = false; 
              counter = 0;
            }
         
      }
        
        
  

}
