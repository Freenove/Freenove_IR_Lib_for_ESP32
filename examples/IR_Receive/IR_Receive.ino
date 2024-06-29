#include "Freenove_IR_Lib_for_ESP32.h"

Freenove_ESP32_IR_Recv ir_recv(15); //ESP32-GPIO15

void setup() {
  Serial.begin(115200);
}

void loop() {
  ir_recv.task();
  if(ir_recv.nec_available())
  {
    Serial.printf("IR Protocol:%s, IR Code: %#x\r\n", ir_recv.protocol(), ir_recv.data());
  }
}
