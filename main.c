int received();
uint16_t SPI3_received_Spg();
uint32_t z_spannungBerechnen(uint16_t a_binaer);
uint16_t SPI3_received_Ang();
void updateParameters(int32_t parameters[]);
uint32_t interpolateSine(uint16_t angle, uint32_t scale);
int32_t Skalierungsfaktor(int16_t frequency);
int16_t f_soll;
int32_t ki_test;
int32_t  istDrehzahl();
float  tiefpass(int32_t uDrehzahl);
int32_t PIregler(int32_t drehzahl_soll,float Drehzahl_filter);

// ----- main() ---------------------------------------------------------------

int main(int argc, char *argv[]) {
	oled_25664_init();
	while(1)
	{
		/*****LEDS ANSTEUERUNG*****/
		GPIOG->BSRR=GPIO_BSRR_BR_2;
		HAL_Delay(500);
		GPIOG->BSRR=GPIO_BSRR_BS_2;
		HAL_Delay(500);
	}
	while (1) {
		/***********Zahlen per UART2 schicken**************/
		// ****** 2 zahlen addieren *******
		uint16_t ergebnis = 0;
		int zahl1 = received();
		int zahl2 = received();
		ergebnis = zahl1 + zahl2;
		// ****** Ergebnis schicken *******
		char str[20] = {0};
		sprintf(str, "%5d", ergebnis);  //Addition in str speichern
		const char* p_str = str;//pointer to str
		sendUARTString(USART2, p_str);//send Addition to uart->DR

//1.3.4 Display

		// *****Additionsergebnis auf dem Display
 N
//1.3.5 Auslesen Der Zwischen-Spannung

		uint16_t sensorSpg_aus = SPI3_received_Spg();
//Zwischenkreisspannung berechnen
		uint16_t V_dc = z_spannungBerechnen(sensorSpg_aus);

//Zwischenkreisspannung auf dem Display
		char Vdc[10] = { 0 };     //VDC maximal 5005V
		sprintf(Vdc, "%6d V", V_dc);
//strncat(Vdc, "V", 1);    //mit Einheit Volt
		char const* const pointer_Vdc = Vdc;
		oled_25664_ShowString(pointer_Vdc, 0x00, 0x00);
		sendUARTString(USART2, pointer_Vdc);

		//Encoder Auslesen

		uint16_t drehgeberAng_aus = SPI3_received_Ang();
		char Ang[15] = { 0 };

		if (drehgeberAng_aus <= 8191) {

		uint32_t Drehwinkel_360 =( (uint32_t) drehgeberAng_aus)* ((uint32_t) 360);
		uint32_t Drehwinkel = Drehwinkel_360 / 0x2000;  // Postion in 360 abbilden
			sprintf(Ang, "%6ld ", Drehwinkel);
			char const* const pointer_Ang = Ang;
			oled_25664_ShowString(pointer_Ang,0x00, 0x01);
		}
		else
		{
			char const* const fehler = "Fehler";
			oled_25664_ShowString(fehler, 0x00, 0x02);
		}
	}

	/*2.3.2 Messdaten von STM32 Erzeugen*/
		int32_t array[1];
		while (1) {
			for (int i = 0; i < 1200; i++) {
				array[0] = i;
				logMeasurements(1, array);
			}
		}

}

// ----------------------------------------------------------------------------
int received() {
	int sum = 0;
	unsigned char zeichen = receiveUARTSync(USART2);
	while (zeichen != '\n') {
	unsigned char chr_unterscheiden = zeichen; //unterscheiden zwi. Ziffer und Zahlen
	if (chr_unterscheiden >= '0' && chr_unterscheiden <= '9') {
		chr_unterscheiden -= '0';
		sum = sum * 10 + chr_unterscheiden;
			zeichen = receiveUARTSync(USART2);
		} else {
			return sum;}

	}
	return sum;
	
}

//--------------------------------------------------------------------------
uint16_t SPI3_received_Ang() {
	GPIOC->BSRR = GPIO_BSRR_BR_13;
	uint16_t drehgeberAng_aus = receiveSPISync(SPI3);
	GPIOC->BSRR = GPIO_BSRR_BS_13;
	return drehgeberAng_aus;
}

//------------------------------------------------------------------------
uint16_t SPI3_received_Spg() {
	GPIOD->BSRR = GPIO_BSRR_BR_9;
	uint16_t sensorSpg_aus = receiveSPISync(SPI3);
	GPIOD->BSRR = GPIO_BSRR_BS_9;
	return sensorSpg_aus;
}
//-----------------------------------------------------------------------

uint32_t z_spannungBerechnen(uint16_t a_binaer) {
//	uint16_t var=a_binaer;
	uint16_t spannung = 0xFFF;
	uint32_t zwiV = (uint32_t) ((uint32_t) (5005) * (uint32_t) a_binaer);
	uint32_t z_spannung = (uint32_t) (zwiV / (uint32_t) spannung);
	//uint16_t spannung_ausgabe= (uint16_t)z_spannung;

	//return spannung_ausgabe;
	return z_spannung;
}






//--------------------------------------------------------------------------
//send Command to OLED
//--------------------------------------------------------------------------
void oled_25664_Command(unsigned char Data) {
	sendSPIAsync(SPI5, (uint16_t) Data);
}

//--------------------------------------------------------------------------
//send Data to OLED
//--------------------------------------------------------------------------
void oled_25664_Data(unsigned char Data) {
	sendSPIAsync(SPI5, (0x100 | (uint16_t) Data));
}

void oled_25664_SetupConnection() {
	GPIOD->BSRR = GPIO_BSRR_BS_10; //PD10 CS,MF_IOA2(RESET),set 1,Display starten

}
int vorzeichen, vorzeichen_alt=0;
void updateParameters(int32_t parameters[]){

			f_soll=parameters[1];
			ki_test=parameters[2];
			vorzeichen=f_soll>0?1:f_soll<0?-1:0;
			if(vorzeichen != vorzeichen_alt)
			{
				f_soll = 0;
			}
			vorzeichen_alt=vorzeichen;

}

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
			SinValue =(( scale* SinValue)>>16) + 10500;
		}

	else if(Quadrant == 1)
		{
			SinValue = lookUpTable[16 - index] -((lookUpTable[16 - index] - lookUpTable[16 - index - 1]) * nachkommaSt>>10) ;
			SinValue =(( scale* SinValue)>>16) + 10500;

		}
	else if (Quadrant == 2)
		{
			SinValue = lookUpTable[index] +((lookUpTable[index + 1] - lookUpTable[index]) * nachkommaSt >> 10);
			SinValue = 10500 - (( scale* SinValue)>>16);
		}

	else
		{
			SinValue = lookUpTable[16 - index] -((lookUpTable[16 - index] - lookUpTable[16 - index - 1]) * nachkommaSt >> 10);
			SinValue = 10500 -  (( scale* SinValue)>>16);

		}

	return SinValue;

}

/*2.3.3 ISR,Sinus-referenzsignal erzeugen*/
int16_t Winkel = 0;
int i =0;
void TIM1_UP_TIM10_IRQHandler() {

//	int32_t sinus[] = { s1, s2, s3 };
	//logMeasurements(3, sinus);
	int32_t  DrehZahl=istDrehzahl();
	float  Drehzahl_tiefpass= tiefpass(DrehZahl);
	//int32_t  drehzahl[]={DrehZahl,Drehzahl_tiefpass};

	//logMeasurements(2,drehzahl);

//Versuch3

	int32_t Drehzahl_soll=f_soll*60;
	int32_t u= PIregler( Drehzahl_soll,Drehzahl_tiefpass);
	float f=u/60;

//PWM Methode
	int32_t increment = (int32_t) (65536 *f / 8000);
	Winkel += increment; //increment=2pi*f/fuev => 2^16/8000}
	int32_t scale;
	int32_t drehzahl[2]={0,0};
	scale= Skalierungsfaktor(f);
	//3 Ringpuffer
	int32_t s1 =  interpolateSine(Winkel, scale);
	int32_t s2 =  interpolateSine(Winkel + 21845, scale); // Verschiebung um 2pi/3 ,120*182=21845
	int32_t s3  =  interpolateSine(Winkel + 43691, scale); // Verschiebung um 4pi/3,240*182=43691
	TIM1->CCR1=s1;
	TIM1->CCR2=s2;
	TIM1->CCR3=s3;

	i++;
	if(i%11==0){
	 drehzahl[0]=Drehzahl_soll;
	 drehzahl[1]=(int32_t)Drehzahl_tiefpass;
	 drehzahl[2]=u;
	 logMeasurements(3,drehzahl);
	}

	TIM1->SR&=~TIM_SR_UIF;
}

//Skalierung

int32_t Skalierungsfaktor(int16_t frequency) {
	int32_t faktor;
	if (frequency > 25 || frequency < -25) {
		faktor = 1<<16;
	}
	else if (frequency >= 0) {
		faktor = frequency*(( int32_t) 2621); // Faktor f/25Hz int16_t 1/25=0.04=101000111101=2621
	}
	else {
		frequency = -frequency;
		faktor = frequency*(( int32_t) 2621);
	}
	return faktor;
}






//-------------------------------------------------------------------------
//3.3.2. Berechnung der Ist-Drehzahl
//--------------------------------------------------------------------------
float pk_alt=0;
float s2_alt=0;
float s1_alt=0;
int32_t  istDrehzahl()
{
	float pk, s1,s2,Drehzahl;
	pk= SPI3_received_Ang();  //was macht man,wenn pk>=8192? Bit13:8191
	if (pk > 8191)
	{
		pk = pk_alt+s1_alt;

	}
	s1 = pk - pk_alt;
	pk_alt = pk;
	//Eliminate Overflow
	s1_alt=s1;
	if (s1 >= 4096)
	{
		s2 = s1 - 8192;
	}
	else if (s1 <= -4196)
	{
		s2 = s1 + 8192;
	}
	else {
		s2 = s1;
	}

	Drehzahl = s2 * 60 * 8000 / 8192; //Ts-Sample Rate -- f_uve=8kHz
	s2_alt = s2;


	return (int32_t) Drehzahl;
}

// low pass filter
float  Drehzahl_filter_alt=0;
float  tiefpass(int32_t uDrehzahl)
{
	float Drehzahl_filter;
	Drehzahl_filter = 0.0181* uDrehzahl + 0.9819 * Drehzahl_filter_alt; //?filtk)=  t_s/(t_s+t_Sw ) ?ufilt(k)+t_Sw/(t_s+t_Sw ) ?filt(k - 1)
	Drehzahl_filter_alt = Drehzahl_filter;
	return Drehzahl_filter;
}

float e_alt=0;
float u_alt=0;
int32_t PIregler(int32_t drehzahl_soll,float Drehzahl_filter)
{

	float ki=3.8;
	float kp=0.5;
	//float kp=0.11105;
	float u;
    float e=drehzahl_soll-Drehzahl_filter;
     u=(kp+ki*0.0000625)*e+(ki*0.000625-kp)*e_alt+u_alt; //  diskretisierte PI Regler
     if(u>4200)
     {
    	 u=4200;
     }
     else if (u<-4200)
     {
    	 u=-4200;
     }
     e_alt=e;
     u_alt=u;
     return (int32_t) u;
}

