/**
 * Brief	An Arduino library for IR remote receive on ESP32.
 * Author	ZhentaoLin
 * Company	Freenove
 * Date		2024-06-28
 */

#include "Freenove_IR_Lib_for_ESP32.h"

//public

Freenove_ESP32_IR_Recv::Freenove_ESP32_IR_Recv(uint8_t pin)
{
	//ir_task_handle = NULL;
	ir_rx_pin = (int)pin;
	ir_ch_dir = (rmt_ch_dir_t)RMT_RX_MODE;
	ir_memsize = (rmt_reserve_memsize_t)RMT_MEM_RX;
	ir_frequency = (uint32_t)RMT_FREQ;
	ir_recv_data.ir_state = 0;
	ir_recv_data.ir_type = UNK;
	ir_recv_data.ir_data = 0x00000000;
	begin(ir_rx_pin);
}

Freenove_ESP32_IR_Recv::~Freenove_ESP32_IR_Recv(void)
{	
	end(ir_rx_pin);
}

bool Freenove_ESP32_IR_Recv::begin(uint8_t pin)
{	
	ir_rx_pin = pin;
	if(!rmtInit(ir_rx_pin, ir_ch_dir, ir_memsize, ir_frequency))
	{
		return 0;
	}
	if(!rmtSetRxMaxThreshold(ir_rx_pin, 12000))
	{
		return 0;
	}
	if(!rmtSetRxMinThreshold(ir_rx_pin, 1))
	{
		return 0;
	}
	return 1;
}

bool Freenove_ESP32_IR_Recv::end(uint8_t pin)
{	
	return rmtDeinit(pin);
}

bool Freenove_ESP32_IR_Recv::available(void)
{
	return ir_recv_data.ir_state;
}
bool Freenove_ESP32_IR_Recv::nec_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)1)
		return 1;
	else
		return 0;
}
bool Freenove_ESP32_IR_Recv::sony_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)3)
		return 1;
	else
		return 0;
}
bool Freenove_ESP32_IR_Recv::sam_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)4)
		return 1;
	else
		return 0;
}
bool Freenove_ESP32_IR_Recv::rc5_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)5)
		return 1;
	else
		return 0;
}

const char* Freenove_ESP32_IR_Recv::protocol(void)
{
	return (const char*)proto[ir_recv_data.ir_type].name;
}

uint32_t Freenove_ESP32_IR_Recv::data(void)
{
	ir_recv_data.ir_state = 0;
	return ir_recv_data.ir_data;
}

bool reading_in_progress = false;  // Flag to track reading state

void Freenove_ESP32_IR_Recv::task(void){
	size_t rx_num_symbols = 64;

    if (!reading_in_progress) {
        // Start a new read operation if none is in progress
        rmtRead(ir_rx_pin, ir_data, &rx_num_symbols, 1000);
        reading_in_progress = true;  // Set the flag to indicate a read is in progress
    }

    // Check if the read operation has completed
    if (rmtReceiveCompleted(ir_rx_pin)) {       
        // Reset the flag to allow a new read operation in the next call
        reading_in_progress = false;
    }
	else {
		return;  // Exit early if data is not yet available
	}
	
	rmt_data_t *rx_items = (rmt_data_t *)ir_data;
	uint32_t rcode = 0;
	ir_protocol_type rproto = UNK;
	//for(int i=0; i<rx_num_symbols;i++)
	//	Serial.printf("%d: %d %d %d %d\r\n", i, rx_items[i].level0, rx_items[i].duration0, rx_items[i].level1, rx_items[i].duration1);
	//Serial.printf("\r\n");
	if (rcode = nec_check(rx_items, rx_num_symbols)) 
	{
		rproto = NEC;
	} 
	else if (rcode = nec_rep_check(rx_items, rx_num_symbols)) 
	{
		rproto = NEC;
	}
	else if (rcode = sam_check(rx_items, rx_num_symbols)) 
	{
		rproto = SAM;
	}
	else if (rcode = sony_check(rx_items, rx_num_symbols)) 
	{
		rproto = SONY;
	}
	else if (rcode = rc5_check(rx_items, rx_num_symbols)) 
	{
		rproto = RC5;
	}
	
	ir_recv_data.ir_state = 1;
	ir_recv_data.ir_type = rproto;
	ir_recv_data.ir_data = rcode;
}

//private

bool Freenove_ESP32_IR_Recv::check_bit(rmt_data_t &item, uint16_t high, uint16_t low) {
  return item.level0 == 0 && item.level1 != 0 && item.duration0 < (high + bitMargin) && item.duration0 > (high - bitMargin) && item.duration1 < (low + bitMargin) && item.duration1 > (low - bitMargin);
}

uint32_t Freenove_ESP32_IR_Recv::nec_rep_check(rmt_data_t *item, size_t &len) {
  const uint8_t totalData = 2;
  if (len < totalData) {
    return 0;
  }
  const uint32_t m = 0x80000000;
  uint32_t code = 0;
  for (uint8_t i = 0; i < totalData; i++) {
    if (i == 0) {  //header
      if (check_bit(item[i], proto[NEC_REP].header_high, proto[NEC_REP].header_low)) {
        code = 0xffffffff;
      } else {
        return 0;
      }
    }
  }
  return code;
}

uint32_t Freenove_ESP32_IR_Recv::nec_check(rmt_data_t *item, size_t &len) {
  const uint8_t totalData = 34;
  if (len < totalData) {
    return 0;
  }
  const uint32_t m = 0x80000000;
  uint32_t code = 0;
  for (uint8_t i = 0; i < totalData; i++) {
    if (i == 0) {  //header
      if (!check_bit(item[i], proto[NEC].header_high, proto[NEC].header_low)) {
        return 0;
      }
    } else if (i == 33) {  //footer
      if (!check_bit(item[i], proto[NEC].footer_high, proto[NEC].footer_low)) {
        return 0;
      }
    } else if (check_bit(item[i], proto[NEC].one_high, proto[NEC].one_low)) {
      code |= (m >> (i - 1));
    } else if (!check_bit(item[i], proto[NEC].zero_high, proto[NEC].zero_low)) {
      return 0;
    }
  }
  return code;
}

uint32_t Freenove_ESP32_IR_Recv::sam_check(rmt_data_t *item, size_t &len){
	const uint8_t  totalData = 34;
	if(len < totalData ){
		return 0;
	}
	const uint32_t m = 0x80000000;
	uint32_t code = 0;
	for(uint8_t i = 0; i < totalData; i++){
		if(i == 0){//header
			if(!check_bit(item[i], proto[SAM].header_high, proto[SAM].header_low)){return 0;}
		}else if(i == 33){//footer
			if(!check_bit(item[i], proto[SAM].footer_high, proto[SAM].footer_low)){return 0;}
		}else if(check_bit(item[i], proto[SAM].one_high, proto[SAM].one_low)){
			code |= (m >> (i - 1) );
		}else if(!check_bit(item[i], proto[SAM].zero_high, proto[SAM].zero_low)){
			return 0;
		}
	}
	return code;
}

uint32_t Freenove_ESP32_IR_Recv::sony_check(rmt_data_t *item, size_t &len){
	const uint8_t totalMin = 12;
	uint8_t i = 0;
	if(len < totalMin || !check_bit(item[i], proto[SONY].header_high, proto[SONY].header_low)){
		return 0;
	}
	i++;
	uint32_t m = 0x80000000;
	uint32_t code = 0;
	uint8_t maxData = 20;
	if(len < maxData){maxData = 15;}
	if(len < maxData){maxData = 12;}
	for(int j = 0; j < maxData - 1; j++){
		if(check_bit(item[i], proto[SONY].one_high, proto[SONY].one_low)){
			code |= (m >> j);
		}else if(check_bit(item[i], proto[SONY].zero_high, proto[SONY].zero_low)){
			code |= 0 & (m >> j);
		}else if(item[i].duration1 > 15000){ //space repeats
			break;
		}else{
			return 0;
		}
		i++;
	}
	code = code >> (32 - i);
	len = i+1;
	return code;
}

uint32_t Freenove_ESP32_IR_Recv::rc5_check(rmt_data_t *item, size_t &len){
	if(len < 13 || len > 30 ){
		return 0;
	}
	const uint16_t RC5_High = proto[RC5].one_high + proto[RC5].one_low;
	uint32_t code = 0; bool c = false;
	for(uint8_t i = 0; i < len; i++){
		uint32_t d0 = item[i].duration0;
		uint32_t d1 = item[i].duration1;
		if (rc5_bit(d0, proto[RC5].one_low)) {
			code = (code << 1) | c;
			c = rc5_bit(d1, RC5_High) ? !c : c;
		} else if (rc5_bit(d0, RC5_High)) {
			code = (code << 2) | (item[i].level0 << 1) | !item[i].level0;
			c = rc5_bit(d1, proto[RC5].one_low) ? !c : c;
		}else{
			return 0;
		}
	}
	return code;
}

bool Freenove_ESP32_IR_Recv::rc5_bit(uint32_t d, uint32_t v) {
	return (d < (v + bitMargin)) && (d > (v - bitMargin));
}


