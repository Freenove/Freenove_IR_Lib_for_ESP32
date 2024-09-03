// Freenove_IR_Lib_for_ESP32.h
/**
 * Brief	An Arduino library for IR remote receive on ESP32.
 * Author	ZhentaoLin
 * Company	Freenove
 * Date		2024-06-28
 */

#ifndef _FREENOVE_IR_LIB_FOR_ESP32_h
#define _FREENOVE_IR_LIB_FOR_ESP32_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif

#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RMT_FREQ     1000000 
#define RMT_MEM_RX   RMT_MEM_NUM_BLOCKS_3

typedef struct {
  uint16_t header_high;
  uint16_t header_low;
  uint16_t one_high;
  uint16_t one_low;
  uint16_t zero_high;
  uint16_t zero_low;
  uint16_t footer_high;
  uint8_t footer_low;
  uint16_t frequency;
  const char *name;
} ir_protocol_t;

typedef struct {
  uint8_t     ir_state;
  uint32_t    ir_data;
  uint8_t     ir_type;
} ir_recv_t;

enum ir_protocol_type {
	UNK, 
	NEC,
	NEC_REP,
	SONY,
	SAM,
	RC5,
	PROTO_COUNT 
};
	
class Freenove_ESP32_IR_Recv
{
public:
	Freenove_ESP32_IR_Recv(uint8_t pin);
	virtual ~Freenove_ESP32_IR_Recv(void);
	
	bool         begin(uint8_t pin);
	bool         end(uint8_t pin);
	bool         available(void);
	bool         nec_available(void);
	bool         sony_available(void);
	bool         sam_available(void);
	bool         rc5_available(void);
	const char*  protocol(void);
	uint32_t     data(void);
	
	void         task(void);
	
private:
	rmt_data_t            ir_data[64];
	int                   ir_rx_pin;
	rmt_ch_dir_t          ir_ch_dir;
	rmt_reserve_memsize_t ir_memsize;
	uint32_t              ir_frequency;
	ir_recv_t             ir_recv_data;
	const uint16_t         bitMargin = 300;

	const ir_protocol_t proto[PROTO_COUNT] = {
	  [UNK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, "UNK" },
	  [NEC] = { 9000, 4500, 560, 1690, 560, 560, 560, 0, 38000, "NEC" },
	  [NEC_REP] = { 9000, 2250, 0, 0, 0, 0, 0, 0, 38000, "NEC_REP" },
	  [SONY] = { 2400, 600, 1200, 600, 600, 600, 0, 0, 40000, "SONY" },
	  [SAM] = { 4500, 4500, 560, 1690, 560, 560, 560, 0, 38000, "SAM" },
	  [RC5] = { 0, 0, 889, 889, 889, 889, 0, 0, 38000, "RC5" },
	};
	
	bool         check_bit(rmt_data_t &item, uint16_t high, uint16_t low);
	uint32_t     nec_rep_check(rmt_data_t *item, size_t &len);
	uint32_t     nec_check(rmt_data_t *item, size_t &len);
	uint32_t     sam_check(rmt_data_t *item, size_t &len);
	uint32_t     sony_check(rmt_data_t *item, size_t &len);
	uint32_t     rc5_check(rmt_data_t *item, size_t &len);
	bool         rc5_bit(uint32_t d, uint32_t v);
	
};

#endif

