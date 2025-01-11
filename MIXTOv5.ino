// ============ incluimos la libreira de adafruit ============
#include <Adafruit_ADS1X15.h>
// ============== incluimos la libreira de wifi ==============
#include <ESP8266WiFi.h>

//=================== conexion wifi ==========================
/* Comentar/Descomentar para conexion Fuera/Dentro de UPV
           |          
           v*/         
//#define WiFi_CONNECTION_UPV

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "AndroidAP_6690"; // red wifi
  const char WiFiPSK[] = "123456789"; //contrasenya
#endif

//=================== comprobar la conexion wifi ==========================

void connectWiFi()
{
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {

       Serial.println(".");
    delay(500);
  }
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
}

//============== conexion server thinkspeak ====================
#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;//pu
#else
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;
//============= HTTP REST Connection ==================

  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="H9P6ZLT5BN0X6P2R"; // Escribe la clave de tu canal ThingSpeak

#define NUM_FIELDS_TO_SEND 5 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

//==============================================================================================VARIABLES=============================================================
// ============ variables HUMEDAD ============

int en_mojado = 9759; // Almacena el valor digital leído en_mojado (primera vez manual)
int en_seco = 20335; // Almacena el valor digital leído en_seco (primera vez manual)
bool flag = false; // variable para acceder a la calibracion automatica humedad
int canal_humedad = 0; // Canal del ADS1115 por el que mediremos la humedad
int sensorValue = 0;   // almacena el valor digital leído
int humedad_porcentaje = 0; // almacena el porcentaje de humedad creado en el mapeo

// ================================================

// ============ variables SALINIDAD ============

#define power_pin 5 // Pin para alimentar el sensor de salinidad
#define sensor_pin A0 // Pin de entrada analógica para leer el sensor de salinidad
const int cuantos = 5;
float sensorSalValue[cuantos] = {200, 400, 600, 700, 900}; // Valores digitales calibrados
float gramos[cuantos] = {0,5,10,15,20};   // Valores correspondientes en gramos
bool flag2 = false; // variable para acceder a la calibracion automatica salinidad
int16_t adc; //definimos la variable adc
float salinity; //definimos la variable que guardara el valor de salinidad

// ================================================

// ============ variables TEMPERATURA ============

#define CANAL 3 // Pin de entrada analógica para leer el sensor de temperatura
int16_t lecturaADC; //definimos la variable que nos muestra la lectura de adc
float va; // guarda el calculo del voltage
float temperatura; // guarda el calculo de la temperatura

// ================================================

// =============== variables PH ===================
int canal_pH = 0;
#define channelValue 1 // Pin para alimentar el sensor de PH
#define Offset 0.20
#define samplingInterval 20
#define printInterval 800
#define  ArrayLength 40
int pHArray[ArrayLength];
int pHArrayIndex = 0;
static float pHValue, Voltage; // variables que cojen informacion de la funcion para mostrar a tablas

// ================================================

// ============ variables Luminosidad ============

int canal_luz = 2; // Pin para alimentar el sensor de Luminosidad
double umbralOscuridad[] = {500,1500,3000,4000};  // Definir los umbrales para los niveles de luz
float vout; // valor de voltage
int16_t lecturaADClum; //definimos la variable que nos muestra la lectura de adc
int porcentaje_lum = 0;
// ================================================

// ================= Definimos la clase de  ads1115 =============
Adafruit_ADS1115 ads1115;  
//=========================================================================================================================================================================

//==============================================================================================FUNCIONES=============================================================
// ================= SETUP =============================

void setup() {
  pinMode(power_pin, OUTPUT); // SALINIDAD: ajustamos el puerto como output ("salida")
  Serial.begin(9600); // Inicializamos la comunicación serie a 9600 baudios
  ads1115.begin();    // Inicializamos el ADS1115
  ads1115.setGain(GAIN_ONE); // HUMEDAD, SALINIDAD , TEMPERATURA: Configuramos la ganancia
    connectWiFi();
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
}
//======================================================

// ================= LOOP  =============================
void loop() {
  //===================== sacamos datos de HUMEDAD ========================
  HUMEDAD(); 
  //===================== sacamos datos SALINIDAD =========================
  SALINIDAD();
  //===================== sacamos datos Temperatura =======================
  TEMPERATURA();
  //===================== sacamos datos PH ================================
  PH();

  //===================== MENU DE RESULTADOS ===============================

  Serial.println();
  Serial.println("=================================================");
  Serial.println("*                     SENSORES                    *");
  Serial.println("=================================================");
  Serial.println("*                                                 *");
  Serial.println("*           HUMEDAD:           " + String(humedad_porcentaje) + "%");
  Serial.println("*           Valor del sensor:  " + String(sensorValue));
  Serial.println("*                                                 *");
  Serial.println("*           SALINIDAD:         " + String(salinity) + " g");
  Serial.println("*           Valor del sensor   " + String(adc));
  Serial.println("*                                                 *");
  Serial.println("*           TEMPERATURA:       " + String(temperatura) + " °C");
  Serial.println("*           VOLTAGE:           " + String(va) + "V");
  Serial.println("*           Valor del sensor   " + String(lecturaADC));
  Serial.println("*                                                 *");
  Serial.println("*           PH:                " + String(pHValue, 2));
  Serial.println("*           VOLTAGE:           " + String(Voltage, 2) + "V");
  Serial.println("*                                                 *");
  //===================== sacamos datos luminosidad ================================
  Serial.println("*           LUMINOSIDAD:      "+ String(porcentaje_lum) + "%");
  Luminosidad();
  Serial.println("*                                                 *");
  Serial.println("=================================================");
  Serial.println("* Pulse 1 para activar la calibracion de humedad  *");
  Serial.println("=================================================");
  Serial.println("* Pulse 2 para activar la calibracion de salinidad *");
  Serial.println("=================================================");
 
 //===================== ACTIVAR CALIBRADOR ========================
  
  activarcalibrador();

  //==================== Enviar informacion a thinkspeak ===================
      String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    data[ 1 ] = String( humedad_porcentaje ); //Escribimos el dato 1. Recuerda actualizar numFields

    data[ 2 ] = String( salinity ); //Escribimos el dato 2. 

    data[ 3 ] = String( temperatura ); //Escribimos el dato 3. 

    data[ 4 ] = String( pHValue, 2 ); //Escribimos el dato 4. 

    data[ 5 ] = String( porcentaje_lum ); //Escribimos el dato 5. 

     HTTPGet( data, NUM_FIELDS_TO_SEND );
  
  delay( 9000 ); 

}

//======================================================

//============   ACTIVAR EL CALIBRADOR    ============== 

void activarcalibrador(){

  // Comprobar si se activa la calibración
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '1') {
      flag = true;
    }
    if (input == '2') {
      flag2 = true;
    }
  }

  // entrada a la calibracion de Humedad
  if (flag) {
    calibrarSensorH();
    flag = false; // Reiniciamos la bandera después de la calibración
  }
    // entrada a la calibracion de salinidad

  if (flag2) {
    calibrarSensorS();
    flag2 = false; // Reiniciamos la bandera después de la calibración
  }

  delay(1000); // Retardo de 1 segundo
}


//======================================================

//===================   HUMEDAD  =======================
void HUMEDAD(){
  // Leemos el valor del ADC
  sensorValue = ads1115.readADC_SingleEnded(canal_humedad); // almcenos el valor que recopila el sensor


  // Mapeamos el valor leído para obtener % de humedad
  humedad_porcentaje = map(sensorValue, en_seco, en_mojado, 0, 100); // deinimos que valor se utiliza como minimo = 0 como maximo = 100 y el valor que se esta conparando
  humedad_porcentaje = constrain(humedad_porcentaje, 0, 100); // Limitamos el porcentaje entre 0 y 100

}
//======================================================

//=========================== FUNCIONES PARA CALIBRAR EL SENSOR DE HUMEDAD ===========================
// 1º GUIA DE CALIBRACION
void calibrarSensorH() {
  Serial.println("Entrando en el proceso de calibración...");
  delay(2000); // Pausa para dar tiempo al usuario

  // Calibración en seco
  en_seco = calibrarH("Coloque el sensor en seco y escriba el valor más alto que vea:");

  // Calibración en mojado
  en_mojado = calibrarH("Coloque el sensor en agua y escriba el valor más bajo que vea:");

  Serial.println("Calibración completada:");
  Serial.println("Valor en seco: " + String(en_seco));
  Serial.println("Valor en mojado: " + String(en_mojado));
}

// 2º CALIBRADO DEL SENSOR
int calibrarH(String mensaje) {
  int valor = 0;
  while (valor <= 0) {
    sensorValue = ads1115.readADC_SingleEnded(canal_humedad);
    Serial.println(mensaje);
    Serial.println(sensorValue);
    delay(2000);
    if (Serial.available() > 0) {
      valor = Serial.parseInt(); // Leer el número ingresado
      if (valor > 0) {
        Serial.println("Valor calibrado: " + String(valor));
      } else {
        Serial.println("Por favor, ingrese un valor válido.");
      }
    }
  }
  return valor;
}
//===============================================================================================

//===================   SALINIDAD  =======================
void SALINIDAD(){

  // Alimentamos el sensor con un tren de pulsos
  digitalWrite(power_pin, HIGH); // damos energuia a traves del puerto 5
  delay(500);
 
  adc = analogRead(sensor_pin); // Leemos el valor analógico
 
  digitalWrite(power_pin, LOW);//quitamos la energuia del puerto 5
  delay(100);

  // Calculamos la cantidad de sal usando Lagrange
  salinity = lagrange(adc);//llamamos a la funcion que nos calcula la salinidad y guardamos su resultado
  salinity = constrain(salinity, 0, 100); // Limitamos el porcentaje entre 0 y 100
}
//======================================================

//=========================== FUNCIONES PARA CALIBRAR EL SENSOR DE SALINIDAD ===========================
// 1º GUIA DE CALIBRACION
void calibrarSensorS() {
  Serial.println("Entrando en el proceso de calibración...");
  delay(2000); // Pausa para dar tiempo al usuario
  for (int i = 0; i < cuantos ; i++){
  sensorSalValue[i] = calibrarS("Coloque "+ String(gramos[i])+" gramos de sal en el vaso y escriva el valor que mas se repita ");
  }
  Serial.println("Calibración completada:");
  Serial.println("Valor en 0g: " + String(sensorSalValue[0]));
  Serial.println("Valor en 5g: " + String(sensorSalValue[1]));
  Serial.println("Valor en 10g: " + String(sensorSalValue[2]));
  Serial.println("Valor en 15g: " + String(sensorSalValue[3]));
  Serial.println("Valor en 20g: " + String(sensorSalValue[4]));
}

// 2º CALIBRADO DEL SENSOR
int calibrarS(String mensaje) {
  int valor = 0;
  while (valor <= 0) {
   digitalWrite(power_pin, HIGH); // damos energuia a traves del puerto 5
    delay(500);
    
    int16_t adc; 
    adc = analogRead(sensor_pin); // Leemos el valor analógico
 
    digitalWrite(power_pin, LOW);//quitamos la energuia del puerto 5
    delay(100);

    Serial.println(mensaje);
    Serial.println(adc);
    
    delay(2000);
    if (Serial.available() > 0) {
      valor = Serial.parseInt(); // Leer el número ingresado
      if (valor > 0) {
        Serial.println("Valor calibrado: " + String(valor));
      } else {
        Serial.println("Por favor, ingrese un valor válido.");
      }
    }
  }
  return valor;
}
//===============================================================================================

//=========================== FUNCION que calcula el  polinomio de lagrange =====================
float lagrange(float x) {
  float result = 0.0;


  for (int i = 0; i < cuantos; i++) {
    float Li = 1.0;
    for (int n = 0; n < cuantos; n++) {
      if (i != n) {
        Li *= (x - sensorSalValue[n]) / (sensorSalValue[i] - sensorSalValue[n]);
      }
    }
    result += Li * gramos[i];
  }
  return result;
}
//===============================================================================================

//===================  TEMPERATURA  =======================
void TEMPERATURA(){
    // Leer el valor digital del canal seleccionado
   lecturaADC = ads1115.readADC_SingleEnded(CANAL);

   va = ((lecturaADC * 3.3) / 32767);

   // Aplicar la fórmula para calcular la temperatura
   // va - b / m
   float va2 = va - 0.79;
   temperatura = va2 / 0.034; // Conversión de voltios a °C
}
//======================================================

//===================      PH      =======================
void PH() {
  static unsigned long samplingTime = millis();
  float ph;
  float total;
  int i;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = ads1115.readADC_SingleEnded(canal_pH);
    if (pHArrayIndex == ArrayLength)pHArrayIndex =0;
    while (i<40) {
      total=total+pHArray[i];
      i++;
    }
    ph=total/40;
    Voltage = (ph*4.096)/32767;
    pHValue = 3.5 * Voltage + Offset;
    samplingTime = millis();
  }
}
//======================================================
//===================      Luminusidad      =======================
void Luminosidad(){
  int16_t lecturaADClum = ads1115.readADC_SingleEnded(canal_luz);
  //Convierte la lectura en voltios.
  vout=(lecturaADC*4.096)/30767;
  // Mostrar la lectura cruda del ADC
  Serial.print("*           Lectura ADC: ");
  Serial.println(lecturaADC);

  // Comparar la lectura con los umbrales e informar del nivel de luz
  if (lecturaADClum < umbralOscuridad[0]) {
    Serial.println("*           Nivel de luz: Oscuridad");
    porcentaje_lum = 0;
  } else if (lecturaADClum < umbralOscuridad[1]) {
    Serial.println("*           Nivel de luz: Sombra");
    porcentaje_lum = 25;
  } else if (lecturaADClum < umbralOscuridad[2]) {
    Serial.println("*           Nivel de luz: Luz Ambiente");
    porcentaje_lum = 50;
  } else if (lecturaADClum < umbralOscuridad[3]) {
    Serial.println("*           Nivel de luz: Alta Iluminación");
    porcentaje_lum = 75;
  } else{
    Serial.println("*           Nivel de luz: MUY Alta Iluminación");
    porcentaje_lum = 100;
  } 
  Serial.print("*           Vout: ");
  Serial.println(vout);
  // Esperar un poco antes de realizar otra lectura
  delay(1000);

}
//==========================  HTTP GET  ==============================
void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){

              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData += MyWriteAPIKey ;
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
              Serial.println( "Connecting to Server for update..." );
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();

              Serial.println( PostData );
              Serial.println();
    }
}
