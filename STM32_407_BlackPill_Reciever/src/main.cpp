//ОБЯЗАТЕЛЬНО!Перед использованием проверь таймеры! Настрой их,если надо: 
//таймер3 на прерывание каждую милисекнду
//таймер2 на прерывание каждую микросекунду(4 микросекунды)
//также проверь функции задержки delay_ms() и delay_us()
//это надо делать, так как я работал с черной платой с OZON и там бедас тактовой частотой




//В этой программе реализован приём данных по радиоканалу от моудля E32
//приём данных происодит по специальному протоколу






#include "../inc/setup.hpp"
#include "../inc/Pars.hpp"
#include "../inc/time_setup.hpp"
#include "../inc/LiquidCrystalSTM.hpp"



uint8_t pkg_is_begin = 0;
uint8_t pkg_is_received = 0;

	
Circular_buffer b;
uint8_t ch;


//БУФФЕР ПАРСИНГА - хранит строку, очищенную от приамбулы и терминатора
const uint8_t PARS_SYMBOLS_SIZE =32;
int sym_index_2 = 0;int str_index_2 =0;
char pars_buf[PARS_SYMBOLS_SIZE];
// void pars_buf_clear(char pars_buf[], uint8_t length);
void pars_buf_uart_print(void);


//БУФФЕР СТРОК ПОСЛЕ ПАРСИНГА - хранит строки, разделенные разделителем
//строка 0 --- первое рапарсированное число (Ключ)
//строка 1 ---второе распарсированное число
const uint8_t SYMBOLS_LENGTH =5;
const uint8_t STRINGS_LENGTH =5;
int sym_indx = 0;int str_indx =0;
char buf[STRINGS_LENGTH][SYMBOLS_LENGTH];
void buf_uart_print(void);
void buf_clear(void);


uint32_t last_time_1 = 0;




void usart2_isr(void)
{
    if (((USART_CR1(USART2) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART2) & USART_SR_RXNE) != 0)) {
        
        if(usart_recv(USART2) =='$' ){pkg_is_begin=1;}
		
		if(pkg_is_begin ){
			b.put( static_cast<uint8_t>(usart_recv(USART2)));
			if(!b.empty()){
				ch = b.get();
				usart_send_blocking(USART3,ch);
				//использование конечного автомата		
				FSM(pars_buf,PARS_SYMBOLS_SIZE, sym_index_2, str_index_2,pkg_is_begin,pkg_is_received,ch);
			}	
				
		}     
	}

}



void config_radiomodule(void){
    gpio_set(GPIOB, GPIO0);
	gpio_set(GPIOB, GPIO1);
    
    delay_ms(200);
	uint8_t str_tx[]={0xC0,0x00,0x00,0x1D,0x06,0x44}; // Настройка для радиомодуля
    uart2_write(str_tx,6); // Записываем конфигурацию в радиомодуль
    delay_ms(200);
    
    gpio_clear(GPIOB, GPIO0);
	gpio_clear(GPIOB, GPIO1);
    delay_ms(200);
    
}


int16_t turn;
int main(void)
{   
    clock_setup();
    gpio_setup();
    usart2_setup();
    usart3_setup();
    timer3_setup();
    timer2_setup();
	timer4_setup();

	rcc_periph_clock_enable(RCC_GPIOD);
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,GPIO_PUPD_NONE ,GPIO11|GPIO14|GPIO13|GPIO12);

	
	//частота счетчика
	rcc_periph_clock_enable(RCC_TIM6);
	timer_set_prescaler(TIM6,160-1);
	timer_set_period(TIM6,200-1);
	timer_enable_counter(TIM6);

	//прерывания
	timer_enable_irq(TIM6,TIM_DIER_UIE);//разрешение прерывания по update
	nvic_enable_irq(NVIC_TIM6_DAC_IRQ);//разерешили прохождения запросов в ЦПУ
  
    config_radiomodule();


	turn = (4096/8);

	
	delay_ms(2000);
	

	int OC =0;
	uint8_t flag = 0;

	
    while (1) {
        if(get_ms() - last_time_1 >= 1000){
            last_time_1 = get_ms();
			
        }


		//ВОТ САМОЕ ИНТЕРЕСНОЕ - если пакет получен
		if(pkg_is_received){
			pkg_is_received = 0;
			sym_indx = 0;
			str_indx = 0;
			buf_clear();

			//второй парсинг
			for( char pars_buf_ch: pars_buf){
				if (pars_buf_ch=='\0'){continue;}
				// usart_send_blocking(USART3,pars_buf_ch);
				if(pars_buf_ch!=',') {
                    buf[str_indx][sym_indx] = pars_buf_ch;
					// usart_send_blocking(USART3,buf[str_indx+1][sym_indx]);
					sym_indx++;
					sym_indx %= SYMBOLS_LENGTH;
					// usart_send_blocking(USART3,ch);
				}
				else if(pars_buf_ch=','){
					// pkg_is_received = 0;
					str_indx++; 
					str_indx %= STRINGS_LENGTH;
					sym_indx = 0;		
				}				
			}
			// usart_send_blocking(USART3,'\t');
			// buf_uart_print();

			
			//это для процессинга

			switch (atoi(buf[0])){
				case 0: 
					pinWrite(GPIOA,GPIO1,!atoi(buf[1]));
				break;
				case 1: 
					timer_set_oc_value(TIM4,TIM_OC4,atoi(buf[1]));
				break;
				case 2: 
					turn = atoi(buf[1]);
				break;
			}
		}
    }
    
return 0;
}

void buf_uart_print(void){//функция вывода ВСЕГО содержимого буфера на UART
	usart_send_blocking(USART3,'\t');	
			for(int i = 0; i<STRINGS_LENGTH; i++){
				for(int j =0; j< SYMBOLS_LENGTH; j++){
					if(buf[i][j]!='\0'){
						usart_send_blocking(USART3,buf[i][j]);
					}
					
				}
				usart_send_blocking(USART3,'\t');
			}usart_send_blocking(USART3,'\n');
}

void buf_clear(void){ //функция очистки буфера парсинга
	for(int i = 0; i<STRINGS_LENGTH; i++){
		for(int j =0; j< SYMBOLS_LENGTH; j++){
			buf[i][j]='\0';
		}
	}
}



void pars_buf_uart_print(void){
	usart_send_blocking(USART3,'\t');
			for(int i =0; i< PARS_SYMBOLS_SIZE; i++){
					if (pars_buf[i]=='\0'){continue;}
				usart_send_blocking(USART3,pars_buf[i]);	
			}
}



uint8_t led_num = 0b00000001;

void tim6_dac_isr(){ //функция-обработчик прерывания
	timer_clear_flag(TIM6,TIM_SR_UIF);
	if(turn>0){
		if(led_num >=16){led_num = 0b00000001;}

		switch(led_num){
            case 0b00000001:  
                gpio_set(GPIOD,GPIO11);
                gpio_clear(GPIOD,GPIO14);
                gpio_clear(GPIOD,GPIO13);
                gpio_clear(GPIOD,GPIO12);
            break;
            case 0b00000010:
                gpio_clear(GPIOD,GPIO11);
                gpio_set(GPIOD,GPIO14);
                gpio_clear(GPIOD,GPIO13);
                gpio_clear(GPIOD,GPIO12);

            break;
            case 0b00000100:
                gpio_clear(GPIOD,GPIO11);
                gpio_clear(GPIOD,GPIO14);
                gpio_set(GPIOD,GPIO13);
                gpio_clear(GPIOD,GPIO12);

            break;
            case 0b00001000:
                gpio_clear(GPIOD,GPIO11);
                gpio_clear(GPIOD,GPIO14);
                gpio_clear(GPIOD,GPIO13);
                gpio_set(GPIOD,GPIO12);

            break;
        }
        turn--;
		led_num *=2;

    }else{
gpio_clear(GPIOD,GPIO11);
gpio_clear(GPIOD,GPIO14);
gpio_clear(GPIOD,GPIO13);
gpio_clear(GPIOD,GPIO12);
    }

}