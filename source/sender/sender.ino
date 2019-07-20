#include <max6675.h>
#include <SPI.h>
#include <LoRa.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#define DEBUG_OUTPUT;

const uint8_t MagicByte = 1337 % 255;
const size_t PacketSize = 9;

const int addr = 12;

const int thermoDO = 7;  // Определяем константу с указанием № вывода Arduino к которому подключён вывод DO  ( SO, MISO ) модуля на чипе MAX6675
const int thermoCS = 6;  // Определяем константу с указанзием № вывода Arduino к которому подключён вывод CS  ( SS )       модуля на чипе MAX6675
const int thermoCLK = 5; // Определяем константу с указанием № вывода Arduino к которому подключён вывод CLK ( SCK )      модуля на чипе MAX6675

MAX6675 thermo(thermoCLK, thermoCS, thermoDO); // Объявляем объект thermo для работы с функциями и методами библиотеки MAX6675, указывая выводы ( CLK , CS , DO )

enum CODES
{
  OK,
  WRITE,
};

const int waiting_steps           = 1000;
const int temperature_indexes_num = 10;
const int send_time_interval      = 3000;

int       send_step               = 3;
int       now_temperature_index   = 0;
int       get_temperature_step;
long      last_send_time          = 0;
long      now_step                = 0;

bool      in_wait                 = 0;
float     temperature_values[temperature_indexes_num];
float     temperature             = 0.0;
float     val                     = 0.0;

const size_t     BufferSize       = 8;
uint8_t   Buffer[BufferSize];

union union4byte_t {
  uint8_t Bytes[4];
  int8_t SmallInt;
  int32_t Int;
  int64_t BigInt;
  float Float;
};
union4byte_t BufferUnion;

void Sender() {
  //LoRa.setTxPower(20);
  temperature = 0.0;
  for (int i = 1; i <= now_temperature_index; i++) temperature += temperature_values[i];

#ifdef DEBUG_OUTPUT
  Serial.print("Sending packet: ");
  Serial.print(addr);
  Serial.print(" ");
  Serial.println(temperature / now_temperature_index);
#endif

  // send packet
  LoRa.beginPacket();
  LoRa.write(MagicByte);
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

  while (!LoRa.begin(430E6)) {
    Serial.println("Starting LoRa failed!");
    delay(100);
  }

#ifdef DEBUG_OUTPUT
  Serial.println("LoRa sender starting");
#endif

  LoRa.enableCrc();
  get_temperature_step = send_step / temperature_indexes_num + 1;
  Serial.println(get_temperature_step);
}

void loop() {
  delay(50);
  Serial.begin(9600);
  now_step++;

  if (now_step % get_temperature_step == 0)
  {
    now_temperature_index++;
    do {
      temperature_values[now_temperature_index] = thermo.readCelsius();
    } while (isnan(temperature_values[now_temperature_index]));

#ifdef DEBUG_OUTPUT
    Serial.print("get to ");
    Serial.print(now_temperature_index);
    Serial.print(" value ");
    Serial.println(temperature_values[now_temperature_index]);
#endif
  }

  if (now_step % send_step == 0)
  {
    in_wait = 1;

#ifdef DEBUG_OUTPUT
    Serial.println("in send");
#endif

    while (in_wait)
    {
      Sender();
      LoRa.receive();
      for (int i = 0; i < waiting_steps; i++)
      {
        int packetSize = LoRa.parsePacket();
        if ((packetSize == PacketSize) && LoRa.read() == MagicByte)
        {
#ifdef DEBUG_OUTPUT
          Serial.print(" recieved packet: ");
#endif

          while (LoRa.available())
          {
            for (size_t i = 0; i < BufferSize; ++i)
            {
              Buffer[i] = LoRa.read();
              delay(5);
            }
            Serial.write(Buffer, BufferSize);
          }
          Serial.println();

          BufferUnion.Bytes[0] = Buffer[0];
          BufferUnion.Bytes[1] = Buffer[1];
          BufferUnion.Bytes[2] = Buffer[2];
          BufferUnion.Bytes[3] = Buffer[3];

          if (BufferUnion.Int == addr) {

            BufferUnion.Bytes[0] = Buffer[4];
            BufferUnion.Bytes[1] = Buffer[5];
            BufferUnion.Bytes[2] = Buffer[6];
            BufferUnion.Bytes[3] = Buffer[7];

            if (BufferUnion.SmallInt == OK)
            {
#ifdef DEBUG_OUTPUT
              Serial.println("Sending secsess");
#endif

              in_wait = 0;
              now_temperature_index = 0;
              break;
            }
          }
        }
        delay(1);
      }
    }
  }
  Serial.end();

  wdt_enable(WDTO_1S); //Задаем интервал сторожевого таймера (2с)
  WDTCSR |= (1 << WDIE); //Устанавливаем бит WDIE регистра WDTCSR для разрешения прерываний от сторожевого таймера
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Устанавливаем интересующий нас режим
  sleep_mode(); // Переводим МК в спящий режим
}

ISR (WDT_vect) {
  wdt_disable();
}
