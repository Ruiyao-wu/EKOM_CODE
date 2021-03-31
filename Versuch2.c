//initialize_hardware

void setupTIM1() {
	NVIC->ISER[0] |= 1 << TIM1_UP_TIM10_IRQn;


	//TIM1 einsetzen
	TIM1->CR1 = TIM_CR1_CEN | TIM_CR1_CMS_0 | TIM_CR1_CMS_1;
	TIM1->DIER = TIM_DIER_UIE;
	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
	TIM1->CCMR2 = TIM_CCMR1_OC3M_2 | TIM_CCMR1_OC3M_1;
	TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE;
	TIM1->PSC = 0;
	TIM1->ARR = 21000;
	TIM1->BDTR = TIM_BDTR_MOE | TIM_BDTR_DTG_1 | TIM_BDTR_DTG_3 | TIM_BDTR_DTG_6 | TIM_BDTR_DTG_7;
}

/* Initialisieren GPIOE fur Timer*/
   //gpioe8-13:mode10

	moder = (1 * GPIO_MODER_MODER8_1 + 0 * GPIO_MODER_MODER8_0) | (1 * GPIO_MODER_MODER9_1 + 0 * GPIO_MODER_MODER9_0) | (1 * GPIO_MODER_MODER10_1 + 0 * GPIO_MODER_MODER10_0) | (1 * GPIO_MODER_MODER11_1 + 0 * GPIO_MODER_MODER11_0) | (1 * GPIO_MODER_MODER12_1 + 0 * GPIO_MODER_MODER12_0) | (0 * GPIO_MODER_MODER13_1 + 1 * GPIO_MODER_MODER13_0); //MOSI weggelassen, Prozessor liest nur aus.
	modemask = ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | GPIO_MODER_MODER10 | GPIO_MODER_MODER11 | GPIO_MODER_MODER12 | GPIO_MODER_MODER13);
	GPIOC->MODER = GPIOC->MODER & modemask | moder;

	//PC8,9,10;11;12,13(AFRH): AF1 setzen
	GPIOC->AFR[1] = GPIO_AF1_TIM1 | (GPIO_AF1_TIM1 << 4) | (GPIO_AF1_TIM1 << 8) | (GPIO_AF1_TIM1 << 12) | (GPIO_AF1_TIM1 << 16) | (GPIO_AF1_TIM1 << 20);

//setupAdditionalHardware

setupAdditionalHardware()
{
    MX_USB_DEVICE_Init();
}




//main.c

//-------------------------------------------------------------------------
//2.3.1update Parameter durch UART
//--------------------------------------------------------------------------

void updateParameters(int32_t parameters[]) {
	char parameterArray[128] = "";
	for (int i = 0; i < 8; i++) {
		sprintf(parameterArray, "%d\r\n", parameters[i]);
		sendUARTString(USART2, parameterArray);
	}
}

//measurment_logger.c
/**2.3.2   Grundlagen2.1.7*/
/*eine Groesse fuer alle Ringpuffer,nur eine Position*/
/*Eintraege Groesse:BUF_SIZE   naechste zu beschreibende Index:outgoingbufferoffset*/

void logMeasurements(int count, const int32_t measurements[]) {
	//TODO
    int ii;
    for (ii = 0; i < count; ii++) {
        ringBuffer[ii][outgoingBufferOffset] = measurements[ii]; //verschieden Messdaten synchron aufgezeichnet werden
    }
    outgoingBufferOffset = (outgoingBufferOffset + 1) % BUF_SIZE;  //nextIndex=(index+increment) mod size
}


//main.c
/*2.3.2 Messdaten von STM32 Erzeugen*/
#if 1
	int32_t array[1];
	while (1) {
		for (int i = 0; i < 1200; i++) {
			array[0] = i;
			logMeasurements(1, array);
		}
	}
#endif


/*2.3.3 sinustest*/

uint32_t lookUpTable[17] = { 0, 1029, 2048, 3048, 4018,4949, 5833, 6661, 7424,
							8116, 8730, 9259, 9700, 10047, 10298, 10449, 10500 };


uint32_t interpolateSine(uint16_t angle, uint32_t scale) {
	int SinValue = 0;

	uint16_t index, nachkommaSt, Quadrant;
	Quadrant = (angle >> 14) & 3;
	index = (angle >> 10) & 15;
	nachkommaSt = angle & 1023;



	if (Quadrant == 0) //1-Qudrant
		{
			SinValue = lookUpTable[index] +((lookUpTable[index + 1] - lookUpTable[index]) * nachkommaSt >>10);
			SinValue = SinValue + 10500;
		}

	else if(Quadrant == 1)
		{
			SinValue = lookUpTable[16 - index] -((lookUpTable[16 - index] - lookUpTable[16 - index - 1]) * nachkommaSt>>10) ;
			SinValue = SinValue + 10500;

		}
	else if (Quadrant == 2)
		{
			SinValue = lookUpTable[index] +((lookUpTable[index + 1] - lookUpTable[index]) * nachkommaSt >> 10);
			SinValue = 10500 - SinValue;
		}

	else
		{
			SinValue = lookUpTable[16 - index] -((lookUpTable[16 - index] - lookUpTable[16 - index - 1]) * nachkommaSt >> 10);
			SinValue = 10500 - SinValue;

		}

	return SinValue;

}

//Skalierung
int32_t multiply(int16_t a, int16_t b) {

	uint64_t Ergebnis = (int32_t)a * (int32_t)b;
	Ergebnis = (int32_t)(Ergebnis >> 16);

	return Ergebnis;
}

int32_t Skalierungsfaktor(int32_t frequency) {
	int32_t faktor;
	if (frequency > 25 && frequency < -25) {
		faktor = 1 << 16;
	}
	else if (frequency >= 0) {
		faktor = multiply(frequency << 16, 2621); // Faktor f/25Hz int16_t 1/25=0.04=101000111101=2621
	}
	else {
		frequency = -frequency;
		faktor = multiply(frequency << 16, 2621);
	}
	return faktor;
}

/*2.3.3 ISR,Sinus-referenzsignal erzeugen*/
void TIM1_UP_TIM10_IRQHandler() {
	int16_t Winkel = 0;
	float f_soll = 2;
	int32_t increment = (65536 * f_soll) / 8000;
	Winkel += increment; //increment=2pi*f/fuev => 2^16/8000
	int32_t scale;
	scale= Skalierungsfaktor(f_soll);
	//3 Ringpuffer
	TIM1->CCR1 = interpolate(Winkel, scale);
	TIM1->CCR2 = interpolate(Winkel + 21845, scale); // Verschiebung um 2pi/3 ,120*182=21845
	TIM1->CCR3 = interpolate(Winkel + 43691, scale); // Verschiebung um 4pi/3,240*182=43691

	int32_t sinus[] = { TIM1->CCR1, TIM1->CCR2, TIM1->CCR3 };
	logMeasurements(3, sinus);
}
