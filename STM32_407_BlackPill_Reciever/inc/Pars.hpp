#ifndef PARS_HPP
#define PARS_HPP
#include <libopencm3/stm32/usart.h>
#include <cstdint>
#include <cmath> 
#include  <cstdlib>


//==============================ОПИСАНИЕ КЛАССА Circular_buffer==========================
constexpr uint8_t SIZE{64};
class Circular_buffer {
public:
	void put(uint8_t);
	uint8_t get();
	uint8_t get_rd();
	uint8_t get_wr();
	bool empty();
	bool full();
	Circular_buffer(); // Circular_buffer b1;
	Circular_buffer(uint8_t); // Circular_buffer b2(128);
	uint8_t readBytes(uint8_t* str, uint8_t amount);
	uint8_t count;
	uint8_t buf[SIZE]; // Противоречит конструктору с пармаетрами
private:
	
	uint8_t wr_idx;
	uint8_t rd_idx;
	
	bool full_;
};

void uart3_write(uint8_t* data, const uint32_t length );
void uart2_write(uint8_t* data, const uint32_t length );
void uart1_write(uint8_t* data, const uint32_t length );

//это для функции itoa
char* reverse(char* buffer, int i, int j);
// сама функция itoa
char* itoa(int value, char* buffer, int base);


class ParsFSM{
	public:
		ParsFSM(char test_value);
		void FSM();
		char pars_buf[1];
	private:
		uint8_t SYMBOLS_LENGTH;
		uint8_t STRINGS_LENGTH;
		char test_value;
};


void FSM(char arr[], uint8_t length,int &sym_index,int &str_index,uint8_t &pkg_is_begin,uint8_t &pkg_is_received, char sym);
void pars_buf_clear(char pars_buf[], uint8_t length);

#endif