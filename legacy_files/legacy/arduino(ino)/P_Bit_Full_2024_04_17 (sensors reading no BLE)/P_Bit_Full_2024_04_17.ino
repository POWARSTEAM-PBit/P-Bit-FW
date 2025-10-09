// Inclusión de librerías necesarias para el proyecto.
#include <Wire.h>                   // Permite la comunicación con dispositivos I2C, como sensores y LCDs.
#include <LiquidCrystal_PCF8574.h>  // Facilita la interacción con pantallas LCD a través del expansor de I/O PCF8574.
#include <DHT.h>                    // Proporciona funciones para leer sensores de temperatura y humedad DHT.
#include <MQ135.h>                  // Ofrece métodos para trabajar con el sensor de calidad del aire MQ135.
#include <OneWire.h>                // Habilita la comunicación OneWire, requerida por sensores como los DallasTemperature.
#include <DallasTemperature.h>      // Permite leer y gestionar sensores de temperatura Dallas.


// Configuración de dispositivos y pines
LiquidCrystal_PCF8574 lcd(39);    // Instancia un objeto LCD. El número (39) es la dirección I2C del LCD.
const byte buzzer = 5;            // Pin digital utilizado para el buzzer.
const byte potPin = A0;           // Pin analógico para leer el potenciómetro.
const byte ldrPin = A1;           // Pin analógico para el sensor LDR (Light Dependent Resistor).
const byte soilMoisturePin = A3;  // Pin analógico para el sensor de humedad del suelo. Compartido con soundSensorPin.
const byte soundSensorPin = A3;   // Pin analógico para el sensor de sonido. Compartido con soilMoisturePin.
const byte NUM_PANTALLAS = 6;     // Número de pantallas o estados en la interfaz de usuario.

// Configuración de LEDs
const byte RGBrojo = 9;    // Pin para el color rojo del LED RGB.
const byte RGBverde = 10;  // Pin para el color verde del LED RGB.
const byte RGBazul = 11;   // Pin para el color azul del LED RGB.
const byte LEDrojo = 12;   // Pin para el LED rojo independiente.
const byte LEDazul = 13;   // Pin para el LED azul independiente.



// Inicialización de sensores
DHT dht(4, DHT11);                    // Define el sensor DHT, asignado al pin 4 y utilizando el modelo DHT11.
MQ135 mq135_sensor(A3);               // Instancia el sensor de calidad del aire MQ135 en el pin A3.
OneWire oneWire(8);                   // Configura el bus OneWire en el pin 8, necesario para los sensores de temperatura Dallas.
DallasTemperature sensors(&oneWire);  // Crea una instancia para manejar los sensores de temperatura Dallas.


// Variables para la gestión de la interfaz de usuario y control de tiempo
byte pantallaActual = 0;           // Almacena el índice de la pantalla o estado actual de la interfaz de usuario.
unsigned long previousMillis = 0;  // Guarda la última vez que se actualizó el evento temporizado.
const long intervalDHT = 2000;     // Intervalo en milisegundos entre lecturas del sensor DHT.


// Constantes para la configuración del display LCD
const byte NUM_ROWS = 2;       // Número de filas en el display LCD.
const byte NUM_COLUMNS = 16;   // Número de columnas en el display LCD.
const byte BAR_CHAR_SIZE = 1;  // Tamaño del carácter utilizado para crear barras de progreso o gráficos.


// Variables para el manejo de lecturas sensoriales y cálculo de promedios
const byte numReadings = 5;  // Número de lecturas para almacenar en el buffer.
byte readings[numReadings];  // Array para almacenar las lecturas y calcular el promedio.
byte readIndex = 0;          // Índice actual para la siguiente lectura en el array.
int total = 0;               // Total acumulado de las lecturas en el array, para calcular el promedio.
int average = 0;             // Promedio de las lecturas almacenadas en el array.

// Variables para gestionar cuándo se actualiza la pantalla LCD en relación con las lecturas del sensor de sonido.
unsigned long lastLCDUpdate = 0;     // Tiempo desde la última actualización de la pantalla LCD.
const long updateLCDInterval = 300;  // Intervalo para actualizar la pantalla LCD (en milisegundos).
byte porcentaje;

void setup() {
  // Inicia la comunicación serial a 9600 baudios para la depuración y visualización de datos.
  Serial.begin(9600);

  // Configura el LCD con 16 columnas y 2 filas, e inicializa el brillo del fondo.
  lcd.begin(16, 2);
  lcd.setBacklight(255);  // Establece el brillo del backlight al máximo.

  // Configura los pines de los dispositivos como salidas.
  pinMode(buzzer, OUTPUT);    // Buzzer para señales audibles.
  pinMode(RGBrojo, OUTPUT);   // LED RGB, pin rojo.
  pinMode(RGBverde, OUTPUT);  // LED RGB, pin verde.
  pinMode(RGBazul, OUTPUT);   // LED RGB, pin azul.
  pinMode(LEDrojo, OUTPUT);   // LED rojo independiente.
  pinMode(LEDazul, OUTPUT);   // LED azul independiente.

  // Inicializa los sensores para su uso.
  dht.begin();      // Sensor de temperatura y humedad DHT.
  sensors.begin();  // Sensores de temperatura Dallas.

  // Limpia el LCD y muestra un mensaje de bienvenida.
  lcd.clear();
  lcd.setCursor(0, 0);       // Posiciona el cursor en la primera línea.
  lcd.print("POWER STEAM");  // Mensaje de bienvenida.
  lcd.setCursor(0, 1);       // Posiciona el cursor en la segunda línea.
  lcd.print("P-Bit");        // Nombre del proyecto.

  // Emite un tono corto para indicar el inicio del dispositivo.
  tone(buzzer, 500, 100);  // Frecuencia de 500Hz durante 100ms.

  // Secuencia de parpadeo de LEDs rojo y azul.
  for (int i = 0; i < 5; i++) {
    digitalWrite(LEDrojo, HIGH);
    digitalWrite(LEDazul, LOW);
    delay(50);
    digitalWrite(LEDrojo, LOW);
    digitalWrite(LEDazul, HIGH);
    delay(50);
  }
  digitalWrite(LEDrojo, LOW);  // Asegura que el LED rojo quede apagado.
  digitalWrite(LEDazul, LOW);  // Asegura que el LED azul quede apagado.

  // Secuencia para cambiar el color del LED RGB gradualmente.
  for (byte color = 0; color < 255; color++) {
    setColor(color, 255 - color, 255);  // Gradiente de color.
    delay(5);
  }
  for (byte color = 255; color > 0; color--) {
    setColor(color, 255 - color, 255);  // Gradiente de color inverso.
    delay(5);
  }
  setColor(0, 0, 0);  // Apaga el LED RGB al final de la secuencia.

  // Emite un tono más agudo para indicar el fin de la secuencia inicial.
  tone(buzzer, 1500, 200);  // Frecuencia de 1500Hz durante 200ms.

  // Segunda secuencia de parpadeo de LEDs rojo y azul, más rápida.
  for (byte i = 0; i < 10; i++) {
    digitalWrite(LEDrojo, HIGH);
    digitalWrite(LEDazul, LOW);
    delay(20);
    digitalWrite(LEDrojo, LOW);
    digitalWrite(LEDazul, HIGH);
    delay(20);
  }
  digitalWrite(LEDrojo, LOW);  // Asegura que el LED rojo quede apagado.
  digitalWrite(LEDazul, LOW);  // Asegura que el LED azul quede apagado.

  // Limpia el LCD al final del setup para comenzar con la interfaz principal.
  lcd.clear();
}




/**
 * Establece el color del LED RGB.
 * 
 * Esta función ajusta el color del LED RGB utilizando modulación por ancho de pulso (PWM) en los pines correspondientes.
 * 
 * @param rojo Intensidad del color rojo (0-255).
 * @param verde Intensidad del color verde (0-255).
 * @param azul Intensidad del color azul (0-255).
 */
void setColor(byte rojo, byte verde, byte azul) {
  analogWrite(RGBrojo, rojo);    // Ajusta la intensidad del rojo.
  analogWrite(RGBverde, verde);  // Ajusta la intensidad del verde.
  analogWrite(RGBazul, azul);    // Ajusta la intensidad del azul.
}




/**
 * Lee el valor del sensor de luz (LDR) y muestra el resultado.
 * 
 * Esta función lee el valor analógico del sensor LDR, lo mapea a un rango de 0 a 100 para representar el porcentaje de luz percibida,
 * y muestra esta información tanto en el puerto serial como en la pantalla LCD junto con una representación gráfica.
 * 
 * @param ldrPin Pin analógico al que está conectado el sensor LDR.
 */
void readlight(byte ldrPin) {
  Serial.println("Leyendo sensor LDR...");
  int lightValue = analogRead(ldrPin);  // Lee el valor analógico del LDR.
  Serial.print("Valor bruto LDR: ");
  Serial.println(lightValue);
  int valorLuz = map(lightValue, 20, 1023, 0, 100);  // Mapea el valor leído a un rango de 0 a 100.
  Serial.print("Valor Luz (LDR) mapeado a porcentaje: ");
  Serial.println(valorLuz);

  // Muestra el valor de luz en la pantalla LCD y una barra de progreso correspondiente.
  pantallaSensorLuz("Luz: ", valorLuz);  // Función auxiliar para mostrar el valor en la pantalla LCD.
  mostrarBarra(valorLuz);                // Función auxiliar para mostrar la barra de progreso en la pantalla LCD.
  Serial.println(".....");
}






/**
 * Muestra el valor de luz en la pantalla LCD.
 * 
 * Esta función actualiza la pantalla LCD para mostrar el porcentaje de luz detectado por el sensor LDR.
 * Primero, ajusta el color del LED RGB a un rojo tenue para indicar actividad y luego muestra el valor en porcentaje.
 * 
 * @param nombre Etiqueta para el valor mostrado, en este caso, siempre será "Luz".
 * @param valorLuz El porcentaje de luz detectado, que se mostrará en la pantalla.
 */
void pantallaSensorLuz(const char *nombre, int valorLuz) {
  setColor(10, 0, 0);          // Establece un color rojo tenue en el LED RGB.
  lcd.setCursor(0, 0);         // Posiciona el cursor en el inicio de la primera línea.
  lcd.print("Luz:         ");  // Muestra la etiqueta y asegura borrar el valor anterior.
  lcd.setCursor(5, 0);         // Posiciona el cursor donde debe empezar el valor numérico.
  lcd.print(valorLuz);         // Imprime el valor de la luz en porcentaje.
  lcd.print(" %  ");           // Añade espacios adicionales para asegurar que caracteres sobrantes sean borrados.
}




/**
 * Dibuja una barra de progreso en la pantalla LCD basada en el porcentaje proporcionado.
 * 
 * La función calcula la longitud de la barra de progreso que se debe dibujar en la pantalla LCD, 
 * basándose en el porcentaje de luz detectado. Luego, limpia la línea inferior de la pantalla y dibuja
 * la barra de progreso utilizando caracteres sólidos.
 * 
 * @param porcentaje El porcentaje de luz detectado, utilizado para determinar la longitud de la barra de progreso.
 */
void mostrarBarra(byte porcentaje) {
  // Calcula el número de caracteres a llenar en la pantalla basado en el porcentaje.
  int numPixelsToFill = map(porcentaje, 0, 100, 0, NUM_COLUMNS * BAR_CHAR_SIZE);

  // Limpia la última línea de la pantalla para la nueva barra de progreso.
  lcd.setCursor(0, NUM_ROWS - 1);
  for (int i = 0; i < NUM_COLUMNS; i++) {
    lcd.write(' ');  // Escribe espacios para limpiar la línea.
  }

  // Dibuja la barra de progreso en la última línea de la pantalla.
  lcd.setCursor(0, NUM_ROWS - 1);  // Reposiciona el cursor al inicio de la última línea.
  for (int i = 0; i < numPixelsToFill; i++) {
    lcd.write((uint8_t)0);  // Dibuja la barra de progreso con el carácter sólido definido.
  }
  // No es necesario agregar espacios adicionales después de la barra, ya que se limpió la línea al inicio.
}




/**
 * Lee y muestra la humedad del suelo cuando la pantalla actual es la de humedad del suelo.
 * 
 * Esta función lee el valor analógico del sensor de humedad del suelo y lo mapea a un rango de 0 a 100%.
 * Si la pantalla actual corresponde a la visualización de la humedad del suelo, muestra este valor en el puerto serial
 * y actualiza la pantalla LCD con la información.
 * 
 * @param soilMoisturePin Pin analógico al que está conectado el sensor de humedad del suelo.
 */
void readsoil(int soilMoisturePin) {
  if (pantallaActual == 2) {  // Verifica si la pantalla actual es la de humedad del suelo.
    Serial.println("Leyendo sensor de humedad del suelo...");
    int soilMoistureValue = analogRead(soilMoisturePin);  // Lee el valor del sensor.
    Serial.print("Valor bruto humedad suelo: ");
    Serial.println(soilMoistureValue);
    int valorSuelo = map(soilMoistureValue, 470, 180, 0, 100);  // Mapea el valor leído a un porcentaje.
    Serial.print("Humedad Suelo mapeada a porcentaje: ");
    Serial.println(valorSuelo);

    pantallaSensorSuelo(valorSuelo);  // Actualiza la pantalla LCD con el valor de humedad.
  }
  Serial.println(".....");
}






/**
 * Muestra la humedad del suelo en la pantalla LCD.
 * 
 * Esta función actualiza la pantalla LCD para mostrar el porcentaje de humedad del suelo detectado.
 * Establece el color del LED RGB a verde para indicar la medición de humedad del suelo.
 * 
 * @param valorSuelo El porcentaje de humedad del suelo a mostrar.
 */
void pantallaSensorSuelo(int valorSuelo) {
  setColor(0, 255, 0);  // Establece el LED RGB a verde.
  lcd.setCursor(0, 0);
  lcd.print("Sensor Suelo      ");  // Muestra la etiqueta y limpia la línea con espacios.
  lcd.setCursor(0, 1);              // Mueve el cursor a la segunda línea.
  lcd.print("Humedad: ");
  lcd.print(valorSuelo);  // Muestra el porcentaje de humedad del suelo.
  lcd.print(" %   ");     // Borra caracteres sobrantes.
}




/**
 * Lee los valores de temperatura y humedad del sensor DHT11 y los muestra.
 * 
 * Realiza la lectura cada intervalo predefinido (intervalDHT). Si el tiempo actual supera el intervalo
 * desde la última lectura, lee los valores del sensor DHT11 y los muestra en el puerto serial.
 * En caso de error en la lectura (valores NaN), muestra un mensaje de error en la LCD.
 */
void readDHT() {
  unsigned long currentMillis = millis();  // Obtiene el tiempo actual.
  // Verifica si ha pasado el intervalo establecido desde la última lectura.
  if (currentMillis - previousMillis >= intervalDHT) {
    previousMillis = currentMillis;  // Actualiza el tiempo de la última lectura.
    float humidity = dht.readHumidity();  // Lee la humedad.
    float temperatureDHT = dht.readTemperature();  // Lee la temperatura.

    // Verifica si los valores leídos son válidos.
    if (isnan(humidity) || isnan(temperatureDHT)) {
      Serial.println(F("Failed to read from DHT sensor!"));  // Imprime el fallo en el Serial Monitor.
      lcd.clear();
      lcd.print("DHT11 Error");  // Muestra un mensaje de error en la LCD.
    } else {
      // Si los valores son válidos, primero imprime en el Serial Monitor.
      Serial.print("Humedad (DHT11): ");
      Serial.print(humidity);
      Serial.println("%");
      Serial.print("Temperatura (DHT11): ");
      Serial.print(temperatureDHT);
      Serial.println("°C");
      
      // Luego, actualiza la pantalla LCD con los valores leídos.
      pantallaSensorDHT11(humidity, temperatureDHT);
    }
  }
}






/**
 * Muestra los valores de humedad y temperatura leídos del sensor DHT11 en la pantalla LCD.
 * 
 * Esta función actualiza la pantalla LCD para mostrar los valores de humedad y temperatura
 * con una precisión de un decimal. Establece el color del LED RGB a azul para indicar la medición
 * de temperatura y humedad.
 * 
 * @param valorHumedad Valor de humedad leído del sensor DHT11.
 * @param valorTemperatura Valor de temperatura leído del sensor DHT11.
 */
void pantallaSensorDHT11(float valorHumedad, float valorTemperatura) {
  setColor(0, 0, 255);  // Establece el LED RGB a azul.
  // Prepara la pantalla y muestra la humedad.
  lcd.setCursor(0, 0);
  lcd.print("Hum.R:          ");  // Limpia la línea.
  lcd.setCursor(7, 0);            // Posiciona para el valor.
  lcd.print(valorHumedad, 1);     // Muestra la humedad con un decimal.
  lcd.print("%   ");              // Limpia caracteres sobrantes.

  // Prepara la pantalla y muestra la temperatura.
  lcd.setCursor(0, 1);
  lcd.print("TempC:          ");   // Limpia la línea.
  lcd.setCursor(7, 1);             // Posiciona para el valor.
  lcd.print(valorTemperatura, 1);  // Muestra la temperatura con un decimal.
  lcd.print(" C  ");               // Limpia caracteres sobrantes.
}




/**
 * Lee y muestra la concentración de CO2 medida por el sensor MQ135.
 * 
 * Esta función utiliza el sensor MQ135 para medir la concentración de CO2 en el ambiente. La lectura se corrige
 * utilizando la temperatura y humedad actual medida por el sensor DHT11. Los resultados se muestran en el puerto serial
 * y en la pantalla LCD.
 */
void readMQ135() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Verifica si los valores leídos del DHT11 son válidos.
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;  // Sale de la función si no se pudo leer del DHT11.
  }

  // Información para debugging: valores de temperatura y humedad utilizados para la corrección.
  Serial.print("Temperatura para corrección MQ135: ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("Humedad para corrección MQ135: ");
  Serial.print(humidity);
  Serial.println("%");

  // Lee la concentración de CO2 del sensor MQ135.
  float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);

  // Debugging: imprime la concentración de CO2 corregida.
  Serial.print("CO2 (MQ135) corregido: ");
  Serial.print(correctedPPM);
  Serial.println(" ppm");

  // Actualiza la pantalla LCD con el valor de CO2 corregido.
  pantallaSensorMQ135(correctedPPM);
  Serial.println(".....");
}





/**
 * Muestra la concentración de CO2 en la pantalla LCD.
 * 
 * Esta función actualiza la pantalla LCD para mostrar la concentración corregida de CO2
 * detectada por el sensor MQ135. El LED RGB se ajusta a un color naranja como indicador visual.
 * 
 * @param correctedPPM La concentración de CO2 en partes por millón (PPM).
 */
void pantallaSensorMQ135(float correctedPPM) {
  setColor(255, 100, 0);  // Establece el LED RGB a naranja.
  lcd.setCursor(0, 0);
  lcd.print("MQ135 Sensor     ");  // Prepara la pantalla mostrando el título del sensor.
  lcd.setCursor(0, 1);             // Mueve el cursor para mostrar el valor de CO2.
  lcd.print("CO2:             ");  // Limpia la línea.
  lcd.setCursor(5, 1);             // Posiciona el cursor para el valor.
  lcd.print(correctedPPM, 2);      // Muestra el valor de CO2 con dos decimales.
  lcd.print("ppm  ");              // Borra caracteres sobrantes.
}




/**
 * Lee y muestra la temperatura medida por el sensor DS18B20.
 * 
 * La función inicia una solicitud de temperatura al sensor DS18B20 y luego lee el valor de temperatura.
 * Este valor se muestra tanto en el puerto serial como en la pantalla LCD.
 */
void readDS18B20() {
  sensors.requestTemperatures();  // Solicita la lectura de temperatura al sensor.
  float temperatureDS18B20 = sensors.getTempCByIndex(0);  // Lee la temperatura del primer sensor en el bus.

  if (temperatureDS18B20 == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: DS18B20 no detectado!"));
    
    lcd.clear();
    lcd.setCursor(0,0);  // Asegúrate de posicionar el cursor correctamente.
    lcd.print("DS18B20 Error");  // Mensaje corto para ajustarse a la limitación de caracteres.

  } else {
    Serial.print("Temperatura (DS18B20): ");
    Serial.print(temperatureDS18B20);
    Serial.println("°C");

    pantallaSensorDS18B20(temperatureDS18B20);
  }
}






/**
 * Muestra la temperatura medida por el sensor DS18B20 en la pantalla LCD.
 * 
 * Esta función actualiza la pantalla LCD para mostrar la temperatura detectada por el sensor DS18B20.
 * El LED RGB se ajusta a un color magenta como indicador visual.
 * 
 * @param temperatureDS18B20 La temperatura medida por el sensor DS18B20 en grados Celsius.
 */
void pantallaSensorDS18B20(float temperatureDS18B20) {
  setColor(255, 0, 100);  // Establece el LED RGB a magenta.
  lcd.setCursor(0, 0);
  lcd.print("DS18B20 Sensor:     ");  // Prepara la pantalla mostrando el título del sensor.
  lcd.setCursor(0, 1);                // Mueve el cursor para mostrar el valor de la temperatura.
  lcd.print("Temp C:             ");  // Limpia la línea.
  lcd.setCursor(8, 1);                // Posiciona el cursor para el valor.
  lcd.print(temperatureDS18B20, 1);   // Muestra el valor de temperatura con una precisión decimal.
  lcd.print(" C   ");                 // Borra caracteres sobrantes.
}




/**
 * Lee el nivel de sonido del entorno y calcula un promedio móvil.
 * 
 * Esta función mide el nivel de sonido utilizando un sensor de sonido y calcula un promedio móvil de las lecturas
 * para suavizar las fluctuaciones rápidas. Utiliza un array para almacenar las últimas lecturas y realiza un cálculo
 * del promedio. Además, inicializa el array solo una vez para evitar reiniciar los valores en cada llamada.
 */
void readSoundSensor() {
  static bool isInitialized = false;  // Indica si el array de lecturas ha sido inicializado.

  if (!isInitialized) {
    for (int i = 0; i < numReadings; i++) {
      readings[i] = 0;  // Inicializa el array con ceros.
    }
    isInitialized = true;
  }

  // Resta la lectura más antigua del total antes de actualizarla.
  total -= readings[readIndex];

  // Lee el nuevo valor del sensor de sonido.
  int newReading = analogRead(soundSensorPin);
  Serial.print("Lectura real del sensor de sonido: ");
  Serial.println(newReading);  // Imprime la lectura real del sensor.

  // Calcula la diferencia absoluta desde el punto medio (idealmente 512 para un ADC de 10 bits sin señal).
  int difference = abs(512 - newReading);
  Serial.print("Valor absoluto de la diferencia: ");
  Serial.println(difference);  // Imprime el valor absoluto de la diferencia.

  readings[readIndex] = difference;  // Almacena la nueva lectura en el array.
  total += readings[readIndex];  // Suma la nueva lectura al total.

  // Avanza al siguiente índice del array, envolviendo al principio si es necesario.
  readIndex = (readIndex + 1) % numReadings;

  // Calcula el promedio de las diferencias.
  average = total / numReadings;

  // Mapea el promedio de la amplitud a un porcentaje.
  byte percentage = map(average, 0, 300, 0, 100);

  Serial.println("Leyendo sensor de sonido...");
  Serial.print("Nivel de sonido (promedio mapeado): ");
  Serial.print(percentage);
  Serial.println("%");

  pantallaSensorSonido(percentage);
  Serial.println(".....");
}










/**
 * Muestra el nivel de sonido en la pantalla LCD como un porcentaje.
 * 
 * Actualiza la pantalla LCD para mostrar visualmente el nivel de sonido detectado en porcentaje.
 * Establece el color del LED RGB a un azul claro como indicador visual del sensor de sonido activo.
 * Además, utiliza la función `mostrarBarra` para representar gráficamente este nivel en la segunda línea de la LCD.
 * 
 * @param porcentaje El porcentaje del nivel de sonido calculado a mostrar.
 */
void pantallaSensorSonido(byte porcentaje) {
  // Comprueba si ha pasado suficiente tiempo desde la última actualización de la pantalla.
  if (millis() - lastLCDUpdate > updateLCDInterval) {
    lastLCDUpdate = millis();  // Actualiza el momento de la última actualización.

    setColor(0, 255, 255);  // Ajusta el color del LED RGB a azul claro.
    lcd.setCursor(0, 0);
    lcd.print("Sonido:        ");
    lcd.setCursor(8, 0);
    lcd.print(porcentaje);
    lcd.print("%   ");

    mostrarBarra(porcentaje);  // Llama a la función para dibujar una barra de progreso en la pantalla.
  }
}





/**
 * Función principal que se ejecuta continuamente después de setup().
 * 
 * Este bucle infinito controla la lógica principal del programa. Lee el valor del potenciómetro para seleccionar
 * entre diferentes pantallas o modos de visualización, cada uno correspondiente a la lectura de un sensor diferente.
 * Implementa una lógica de "debounce" para evitar cambios rápidos e indeseados en la selección de pantalla debido a fluctuaciones
 * en la lectura del potenciómetro.
 */
void loop() {
  // Variable estática para recordar el último valor objetivo del potenciómetro para el debounce.
  static int lastPantallaObjetivo = -1;

  // Lee el valor del potenciómetro.
  int potValue = analogRead(potPin);
  // Mapea el valor del potenciómetro a un índice de pantalla, asegurando que esté dentro del rango válido.
  int pantallaObjetivo = map(potValue, 0, 1023, 0, NUM_PANTALLAS);
  pantallaObjetivo = constrain(pantallaObjetivo, 0, NUM_PANTALLAS - 1);

  // Debounce: Evita cambios rápidos y erráticos en la selección de pantalla.
  if (pantallaObjetivo != pantallaActual && pantallaObjetivo != lastPantallaObjetivo) {
    lastPantallaObjetivo = pantallaObjetivo;  // Actualiza el último valor objetivo para la próxima comparación.
    delay(50);                                // Demora para estabilizar la lectura del potenciómetro.

    // Re-confirma el valor del potenciómetro después de la demora para asegurar una selección estable.
    potValue = analogRead(potPin);
    pantallaObjetivo = map(potValue, 0, 1023, 0, NUM_PANTALLAS);
    pantallaObjetivo = constrain(pantallaObjetivo, 0, NUM_PANTALLAS - 1);

    // Actualiza la pantalla actual si se confirma el cambio después del debounce.
    if (pantallaObjetivo != pantallaActual) {
      pantallaActual = pantallaObjetivo;
      tone(buzzer, 1000, 100);  // Emite un tono como indicador auditivo del cambio de pantalla.
    }
  }

  // Selecciona y ejecuta la acción correspondiente a la pantalla actual mediante un switch case.
  switch (pantallaActual) {
    case 0:
      readlight(ldrPin);  // Lee y muestra la luz ambiental.
      break;
    case 1:
      readDHT();  // Lee y muestra la humedad y temperatura del DHT11.
      break;
    case 2:
      readsoil(soilMoisturePin);  // Lee y muestra la humedad del suelo.
      break;
    case 3:
      readMQ135();  // Lee y muestra la calidad del aire (CO2).
      break;
    case 4:
      readDS18B20();  // Lee y muestra la temperatura del DS18B20.
      break;
    case 5:
      readSoundSensor();  // Lee y muestra el nivel de sonido.
      break;
  }

  delay(100);  // Introduce una pequeña pausa para estabilizar las lecturas y la visualización entre ciclos.
}
