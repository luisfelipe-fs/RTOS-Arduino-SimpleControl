#include <Arduino_FreeRTOS.h>

#define PIN_NTC A0
#define PIN_LED 12
#define PIN_PUSHB1 13
#define PIN_PUSHB2 11
#define PIN_ACTUATOR A1
#define S1 2
#define S2 3
#define S3 4
#define S4 5
#define S5 6
#define S6 7
#define S7 8
#define SOURCE1 9
#define SOURCE2 10

#define BLINK_TIME 25
#define DELAY_TIME 1000
#define CONSTRAINT_TIME 5000
#define MAXCOUNT 40
#define MINCOUNT 15
#define A 0.001125308852122
#define B 0.000234711863267
#define C 0.000000085663516

volatile int contador = MINCOUNT;
volatile int reference = MAXCOUNT;
volatile float temperature = 100;

float getTemperature(int adc);
void refreshDisplay (int x);
void TaskNTC(void *pvParameters);
void TaskPushButton(void *pvParameters);
void TaskDisplay(void *pvParameters);
void TaskControl(void *pvParameters);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  xTaskCreate(TaskDisplay, (const portCHAR*) "Display 7-Segments", 128, NULL, 1, NULL);
  xTaskCreate(TaskPushButton, (const portCHAR*) "PushButton", 128, NULL, 0, NULL);
  xTaskCreate(TaskNTC, (const portCHAR*) "NTC-10k", 128, NULL, 0, NULL);
  xTaskCreate(TaskControl, (const portCHAR*) "Control", 128, NULL, 0, NULL);
}

void loop() {}

void TaskControl (void *pvParameters)
{
  (void) pvParameters;
  static int aboveReference = 0;
  static int ledState = 0;
  static unsigned long ledTime = 0;
  static unsigned long constraintTime = 0;
  static int criticalState = 0;
  static unsigned long delayTime = 0;

  pinMode(PIN_LED, OUTPUT);
  
  for (;;)
  {
    if (temperature > reference && !aboveReference) {
      aboveReference = 1;
      ledTime = constraintTime = millis();
    }
    else if (temperature <= reference && aboveReference) {
      aboveReference = 0;
      criticalState = 0;
      digitalWrite(PIN_LED, LOW);
    }
    
    if (aboveReference) {
      if (millis()-delayTime > DELAY_TIME) {
        analogWrite(PIN_ACTUATOR, 0);
        delayTime = millis();
      }
      if (millis()-constraintTime > CONSTRAINT_TIME)
        criticalState = 1;
    }
    else {
      if (millis()-delayTime > DELAY_TIME) {
        analogWrite(PIN_ACTUATOR, 1024);
        delayTime = millis();
      }
      criticalState = 0;
    }
    
    if (criticalState && millis()-ledTime > BLINK_TIME) {
      ledState = !ledState;
      digitalWrite(PIN_LED, ledState);
      ledTime = millis();
    }
    
    vTaskDelay(1);
  }
}

float getTemperature (int adc) {
  float voltage = 5.0*(float)adc/1023.0;
  float ntc = 10000.0*voltage/(5.0-voltage);
  return 1.0/(A+B*log(ntc)+C*(log(ntc)*log(ntc)*log(ntc)))-273.15;
}

void TaskNTC (void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    temperature = getTemperature(analogRead(PIN_NTC));
    Serial.print(millis());
    Serial.print(' ');
    Serial.println(temperature);
    vTaskDelay(1);
  }
}

void TaskPushButton (void *pvParameters)
{
  (void) pvParameters;
  static int state1 = 0;
  static int state2 = 0;
  static int reading = 0;
  
  pinMode(PIN_PUSHB1, INPUT_PULLUP);
  pinMode(PIN_PUSHB2, INPUT_PULLUP);
  
  for (;;)
  {
    reading = digitalRead(PIN_PUSHB1);
    if (!reading && !state1) {
      if (contador < MAXCOUNT)
        contador++;
      else
        contador = MINCOUNT;
      state1 = 1;
    }
    else if (reading && state1)
      state1 = 0;
    
    reading = digitalRead(PIN_PUSHB2);
    if (!reading && !state2) {
      reference = contador;
      state2 = 1;
    }
    else if (reading && state2)
      state2 = 0;
    vTaskDelay(1);
  }
}

void refreshDisplay (int x) 
{
  void (*ws)(int, int) = &digitalWrite;
  switch (x)
  {
    case 0: ws(S1, 0);ws(S2, 0);ws(S3, 0);ws(S4, 0);ws(S5, 0);ws(S6, 0);ws(S7, 1); break;
    case 1: ws(S1, 1);ws(S2, 0);ws(S3, 0);ws(S4, 1);ws(S5, 1);ws(S6, 1);ws(S7, 1); break;
    case 2: ws(S1, 0);ws(S2, 0);ws(S3, 1);ws(S4, 0);ws(S5, 0);ws(S6, 1);ws(S7, 0); break;
    case 3: ws(S1, 0);ws(S2, 0);ws(S3, 0);ws(S4, 0);ws(S5, 1);ws(S6, 1);ws(S7, 0); break;
    case 4: ws(S1, 1);ws(S2, 0);ws(S3, 0);ws(S4, 1);ws(S5, 1);ws(S6, 0);ws(S7, 0); break;
    case 5: ws(S1, 0);ws(S2, 1);ws(S3, 0);ws(S4, 0);ws(S5, 1);ws(S6, 0);ws(S7, 0); break;
    case 6: ws(S1, 0);ws(S2, 1);ws(S3, 0);ws(S4, 0);ws(S5, 0);ws(S6, 0);ws(S7, 0); break;
    case 7: ws(S1, 0);ws(S2, 0);ws(S3, 0);ws(S4, 1);ws(S5, 1);ws(S6, 1);ws(S7, 1); break;
    case 8: ws(S1, 0);ws(S2, 0);ws(S3, 0);ws(S4, 0);ws(S5, 0);ws(S6, 0);ws(S7, 0); break;
    case 9: ws(S1, 0);ws(S2, 0);ws(S3, 0);ws(S4, 1);ws(S5, 1);ws(S6, 0);ws(S7, 0); break;
  }
}

void TaskDisplay (void *pvParameters)
{
  (void) pvParameters;
  int outputs[] = {S1, S2, S3, S4, S5, S6, S7, SOURCE1, SOURCE2};
  for (int i = 0; i < 9; i++) pinMode(outputs[i], OUTPUT);
  
  for (;;)
  {
    digitalWrite(SOURCE1, LOW);
    digitalWrite(SOURCE2, HIGH);
    refreshDisplay(contador/10);
    vTaskDelay(1);
    digitalWrite(SOURCE1, HIGH);
    digitalWrite(SOURCE2, LOW);
    refreshDisplay(contador%10);
    vTaskDelay(1);
  }
}
