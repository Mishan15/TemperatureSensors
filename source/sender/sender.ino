#include <max6675.h>
#include <SPI.h>
#include <LoRa.h>
#include <avr/wdt.h>
#include <avr/sleep.h>


#define addr 13

#define thermoDO 7  // Определяем константу с указанием № вывода Arduino к которому подключён вывод DO  ( SO, MISO ) модуля на чипе MAX6675
#define thermoCS 6  // Определяем константу с указанзием № вывода Arduino к которому подключён вывод CS  ( SS )       модуля на чипе MAX6675
#define thermoCLK 5 // Определяем константу с указанием № вывода Arduino к которому подключён вывод CLK ( SCK )      модуля на чипе MAX6675

MAX6675 thermo(thermoCLK, thermoCS, thermoDO); // Объявляем объект thermo для работы с функциями и методами библиотеки MAX6675, указывая выводы ( CLK , CS , DO )

enum CODES
{
  OK,
  WRITE,
};

const int waiting_steps           = 1000;
const int temperature_indexes_num = 10;
const int send_time_interval      = 3000;
const size_t     BufferSize       = 8;

float     val                     = 0.0;
int       send_step               = 11;
int       now_temperature_index   = 0;
int       get_temperature_step;
long      last_send_time          = 0;
long      now_step                = 0;
uint8_t   Buffer[BufferSize];
bool      in_wait                 = 0;
float     temperature_values[temperature_indexes_num];
float     temperature             = 0.0;

typedef union union4byte_t {
  uint8_t Bytes[4];
  int8_t SmallInt;
  int32_t Int;
  int64_t BigInt;
  float Float;
};
union4byte_t BufferUnion;

void Sender() {
  LoRa.setTxPower(20);
  temperature = 0.0;
  for(int i = 1; i <= now_temperature_index; i++) temperature += temperature_values[i];
  Serial.print("Sending packet: ");
  Serial.print(addr);
  Serial.print(" ");
  Serial.println(temperature / now_temperature_index);
  // send packet
  LoRa.beginPacket();
  BufferUnion.Int = addr;
  LoRa.write(BufferUnion.Bytes, 4);
  BufferUnion.Float = temperature / now_temperature_index;
  LoRa.write(BufferUnion.Bytes, 4);
  LoRa.endPacket();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  //Serial.println("Lora wan говно");

  while(!LoRa.begin(430E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
  }
  Serial.println("LoRa sender starting");
  LoRa.enableCrc();
  get_temperature_step = send_step / temperature_indexes_num + 1;
  Serial.println(get_temperature_step);
}

void loop() {
  now_step++;
  
  if(now_step % get_temperature_step == 0)
  {      
    now_temperature_index++;
    temperature_values[now_temperature_index] = thermo.readCelsius();
    Serial.print("get to ");
    Serial.print(now_temperature_index);  
    Serial.print(" value ");
    Serial.println(temperature_values[now_temperature_index]);
  }
  
  if (now_step % send_step == 0) 
  {
    in_wait = 1;
    Serial.println("in send");
    while(in_wait)
    {
      Sender();
      for (int i = 0; i < waiting_steps; i++)
      {
        int packetSize = LoRa.parsePacket();
        if (packetSize)
        {
          Serial.print(" recieved packet: ");
          while (LoRa.available())
          {
            for (size_t i = 0; i < BufferSize; ++i)
            {
              Buffer[i] = LoRa.read();
            }
            Serial.write(Buffer, BufferSize);
          }
          Serial.println();
          BufferUnion.Bytes[0] = Buffer[4];
          BufferUnion.Bytes[1] = Buffer[5];
          BufferUnion.Bytes[2] = Buffer[6];
          BufferUnion.Bytes[3] = Buffer[7];
          if (BufferUnion.SmallInt == OK)
          {
            Serial.println("Sending secsess");
            in_wait = 0;
            now_temperature_index = 0;
            break;
          }
        }
        delay(1);
      }
    }
  }

  wdt_enable(WDTO_1S); //Задаем интервал сторожевого таймера (2с)
  WDTCSR |= (1 << WDIE); //Устанавливаем бит WDIE регистра WDTCSR для разрешения прерываний от сторожевого таймера
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Устанавливаем интересующий нас режим
  sleep_mode(); // Переводим МК в спящий режим
  
  /*in_wait = 1;
    Sender();
    while (in_wait) {
    while (millis() - last_send_time < send_time_interval) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        Serial.print(" recieved packet: ");
        while (LoRa.available()) {
          for (size_t i = 0; i < BufferSize; ++i) {
            Buffer[i] = LoRa.read();
          }
          Serial.write(Buffer, BufferSize);
        }
        Serial.println();
        Serial.println(Buffer[4]);
        Serial.println(Buffer[5]);
        Serial.println(Buffer[6]);
        Serial.println(Buffer[7]);
        if(YES) {
          Serial.println("YES");
          in_wait = 0;
        }
      }
    }
    last_send_time = millis();
    }//*/ //it is work
}

ISR (WDT_vect) {
  wdt_disable();
}
