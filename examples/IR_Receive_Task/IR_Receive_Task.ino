#include "Freenove_IR_Lib_for_ESP32.h"

Freenove_ESP32_IR_Recv ir_recv(15); //ESP32-GPIO15

void ir_task(void* parameter){
	parameter = parameter;
  while(1){
    ir_recv.task();
  }
	vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  xTaskCreatePinnedToCore(ir_task, "ir_task", 2048, NULL, 10, NULL, 1);
}

void loop() {
  if(ir_recv.nec_available())
  {
    Serial.printf("IR Protocol:%s, IR Code: %#x\r\n", ir_recv.protocol(), ir_recv.data());
  }
}
