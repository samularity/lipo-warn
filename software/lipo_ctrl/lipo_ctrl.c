/*
 * lipo_ctrl.c
 *
 * Created: 26.10.2014 21:22:14
 *  Author: Samuel Munz
 * visit https://github.com/samularity for more info
 */ 


/*
//PB0 out 1 (alert if voltage below threshold)
//PB1 out 2 (alert if voltage was below threshold once)
//PB2 Rread analog (lipo voltage)

Cell recoginiton with following values
cells	min V	max V
1		2,7		4,3
2		5,4		8,6
3		8,1		12,9
for now 3 Cells is Maximum, can be changed (Hardware and software needs to be edited)



13V ---|||||-------|||||----GND
	   130k	   |	12K
			   |
			  PB2 max 1,1V

13 000 mV max
10bit ADC von 0 bis 13000mV
2^10=1024
1 ADC-bit -> 12,695mV
*/
#include <avr/io.h>
#define F_CPU 1200000UL  // 1,2 MHz
#include <util/delay.h>

//function prototypes
void adc_setup (void);
uint16_t adc_read (void);
void beep(uint8_t anz);

#define threshold_voltage 3200 //3200mV threshold voltage, tests are necesarry
#define ADC_TO_mV(s)  s*12.695				// in mV
#define	SET_HIGH(s)		PORTB |= (1<<s);	//PB high
#define SET_LOW(s)		PORTB &= ~(1<<s);	//PB low


int main(void)
{	
	DDRB &= ~(1 << DDB2);	//set PB2 as Input
	DDRB |= (1 << DDB0)|(1 << DDB1);	//set as output
	SET_LOW(0);//turn off
	SET_LOW(1);//turn off
	adc_setup();//connected to PB2
	_delay_ms(250);//wait for all connections to be ok
	//hardware init done

	uint16_t accu_voltage=adc_read();	
	uint8_t nr_of_cells = 0; 
	//get number of connected cells
	if (accu_voltage>2500 && accu_voltage<4300 )
		{nr_of_cells=1;} 
	else if (accu_voltage>5400 && accu_voltage<8600 )
		{nr_of_cells=2;}
	else if (accu_voltage>8600 && accu_voltage<12900 )
		{nr_of_cells=3;}	
	
	beep(nr_of_cells); //visual output of detected cells 
	
	while(1)
    {
		if (adc_read()<(nr_of_cells*threshold_voltage)) //alert, voltage is below threshold
		{
			SET_HIGH(0); //turn "hard" alert on
			SET_HIGH(1); //turn "weak" alert on
		}
		else//was below threshold voltage once
		{
			SET_LOW(0); //turn "hard" alert off
		}
	_delay_ms(50);	//check every 50ms
    }
}

void adc_setup (void)
{
	ADMUX |= (1 << MUX0);	// Set the ADC input to PB2/ADC1
	ADMUX |= (1 << REFS0);	//use internal 1,1V Voltage ref
    //ADMUX |= (1 << ADLAR);//enable 8 bit mode
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);// Set the prescaler to clock/128 & enable ADC
}

uint16_t adc_read (void)
{
	uint16_t ret_val =0;
	for ( uint8_t i=0;i<4;i++)	//to create an average 
	{
		ADCSRA |= (1 << ADSC);			// Start the conversion
		while (ADCSRA & (1 << ADSC));	// Wait for it to finish	
		ret_val+=ADC;//for ADC 10 bit mode   (ret_val+=ADCH; //for ADC 8 bit mode)
	}
	return (ADC_TO_mV (ret_val/4) ); //returns value in mV
}


void beep(uint8_t anz)
{
	for (uint8_t i =0; i<anz; i++)
	{
	SET_HIGH(0);
	_delay_ms(250);		
	SET_LOW(0);
	_delay_ms(250);
	}
}