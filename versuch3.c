/*Versuch3*/

//-------------------------------------------------------------------------
//3.3.2. Berechnung der Ist-Drehzahl
//--------------------------------------------------------------------------
float istDrehzahl()
{
	float pk,pk_alt, s1,s1_alt, s2,s2_alt, delta_s2,s3,s3_alt,Drehzahl;
	pk= SPI3_received_Ang();  //was macht man,wenn pk>=8192? Bit13:8191
	if (pk > 8191)
	{
		pk = pk_alt;
	}
	//Eliminate Overflow
	s1 = pk - pk_alt;
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
	//Eliminate Missreadings
	delta_s2 = s2 - s2_alt;
	if (delta_s2 >= 4)
	{
		s3 = s3_alt;
	}
	else
	{
		s3 = s2;
	}
	
	pk_alt = pk;
	s1_alt = s1;
	s2_alt = s2;
	s3_alt = s3;
	Drehzahl = s3 * 60 * 8000 / 8192; //Ts-Sample Rate -- f_uve=8kHz

	return Drehzahl;
}

// low pass filter
float tiefpass(float uDrehzahl)
{
	float Drehzahl_filter, Drehzahl_filter_alt;
	Drehzahl_filter = 0.0181* uDrehzahl + 0.9819 * Drehzahl_filter_alt; //ωfiltk)=  τ_s/(τ_s+τ_Sw ) ωufilt(k)+τ_Sw/(τ_s+τ_Sw ) ωfilt(k - 1)
	Drehzahl_filter_alt = Drehzahl_filter;
	return Drehzahl_filter;
}