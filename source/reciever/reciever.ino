#include <SPI.h>
#include <LoRa.h>


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

void Reciever() {
  // try to parse packet
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
    //Send();
  }  
}


void Send() {
  LoRa.setTxPower(20);
  //Serial.print("Sending answer ");
  for (int i = 0; i < 10; i++) {
    BufferUnion.SmallInt = OK;
    BufferUnion.BigInt = int64_t(send_step) << 8;
    Buffer[4] = BufferUnion.Bytes[0];
    Buffer[5] = BufferUnion.Bytes[1];
    Buffer[6] = BufferUnion.Bytes[2];
    Buffer[7] = BufferUnion.Bytes[3];
    LoRa.beginPacket();
    LoRa.write(Buffer, BufferSize);
    LoRa.endPacket();
    delay(1);
  }
  //Serial.println(" Answer sended");
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!LoRa.begin(430E6)) {
    Serial.write('!');
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //Serial.println("LoRa кусшумук starting");
  LoRa.enableCrc();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 8) {
    //Serial.print(" recieved packet: ");
    while (LoRa.available()) {
      for (size_t i = 0; i < BufferSize; ++i) {
        Buffer[i] = LoRa.read();
      }
      Serial.write('#');
      Serial.write(Buffer, BufferSize);
      /*BufferUnion.Bytes[0] = Buffer[0];
      BufferUnion.Bytes[1] = Buffer[1];
      BufferUnion.Bytes[2] = Buffer[2];
      BufferUnion.Bytes[3] = Buffer[3];
      Serial.print(" addres: ");
      Serial.print(BufferUnion.Int);
      BufferUnion.Bytes[0] = Buffer[4];
      BufferUnion.Bytes[1] = Buffer[5];
      BufferUnion.Bytes[2] = Buffer[6];
      BufferUnion.Bytes[3] = Buffer[7];
      Serial.print(" value: ");
      Serial.print(BufferUnion.Float);
      Serial.println();      //*/
    }
    Send();
  }  
}
