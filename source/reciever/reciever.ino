#include <SPI.h>
#include <LoRa.h>

//#define DEBUG_OUTPUT;

enum CODES
{
  OK,
  WRITE,
};
int32_t addr = 0;
float val = 325.87;
int send_step = 10950;

typedef union union4byte_t {
  uint8_t Bytes[4];
  int8_t SmallInt;
  int32_t Int;
  int64_t BigInt;
  float Float;
};
union4byte_t BufferUnion;
const size_t BufferSize = 8;
uint8_t Buffer[BufferSize];

const int min_addr = 1;
const int max_addr = 100;
const float min_val = 0.0;
const float max_val = 2000.0;

void Send() {
  //LoRa.setTxPower(20);

  #ifdef DEBUG_OUTPUT
  Serial.print("Sending answer ");
  #endif
  
  for (int i = 0; i < 10; i++) {
    LoRa.beginPacket();
    BufferUnion.Int = addr;
    LoRa.write(BufferUnion.Bytes, 4);
    BufferUnion.SmallInt = OK;
    BufferUnion.BigInt = int64_t(send_step) << 8;
    LoRa.write(BufferUnion.Bytes, 4);
    LoRa.endPacket();
    delay(3);
  }
  
  #ifdef DEBUG_OUTPUT
  Serial.println(" Answer sended");
  #endif
}

void onReceive(int packetSize) {
  #ifdef DEBUG_OUTPUT
  Serial.print(" recieved packet.");
  #endif

  if (packetSize == BufferSize) {
    for (size_t i = 0; i < BufferSize; ++i)
    {
      Buffer[i] = LoRa.read();
    }
    
    #ifdef DEBUG_OUTPUT
    Serial.print(" Ours: ");
    #endif
    
    BufferUnion.Bytes[0] = Buffer[0];
    BufferUnion.Bytes[1] = Buffer[1];
    BufferUnion.Bytes[2] = Buffer[2];
    BufferUnion.Bytes[3] = Buffer[3];

    #ifdef DEBUG_OUTPUT
    addr = BufferUnion.Int;
    Serial.print(" addres: ");
    Serial.print(addr);
    #endif

    BufferUnion.Bytes[0] = Buffer[4];
    BufferUnion.Bytes[1] = Buffer[5];
    BufferUnion.Bytes[2] = Buffer[6];
    BufferUnion.Bytes[3] = Buffer[7];

    #ifdef DEBUG_OUTPUT
    Serial.print(" value: ");
    Serial.print(BufferUnion.Float);
    Serial.println();
    #endif

    #ifndef DEBUG_OUTPUT
    Serial.write((uint8_t)'#');
    Serial.write(Buffer, BufferSize);
    //Serial.write((uint8_t)'@');
    #endif

    Send();
    //delay(100);
    //LoRa.receive();
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!LoRa.begin(430E6)) {
    Serial.write('!');
    #ifdef DEBUG_OUTPUT
    Serial.println("Starting LoRa failed!");
    #endif
    
    while (1);
  }
  #ifdef DEBUG_OUTPUT
  Serial.println("LoRa кусшумук starting");
  #endif
  
  LoRa.onReceive(onReceive);
  LoRa.enableCrc();
  LoRa.receive();
}

void loop() {
  delay(10);
  LoRa.receive();
}
