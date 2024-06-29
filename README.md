# Freenove IR Lib for ESP32

## Description
This is an Arduino library for receiving ir data on esp32.

Based on the https://github.com/junkfix/esp32-rmt-ir

Thanks junkfix for the example.


## Examples:

Here are some simple examples.

### Show Rainbow

This example make your strip show a flowing rainbow.

```
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
```

```
#include "Freenove_IR_Lib_for_ESP32.h"

Freenove_ESP32_IR_Recv ir_recv(2); //ESP32-GPIO15

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
```

