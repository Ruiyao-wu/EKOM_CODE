#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


uint32_t lookUpTable[17] = { 0, 1029, 2048, 3048, 4018,4949, 5833, 6661, 7424,
							8116, 8730, 9259, 9700, 10047, 10298, 10449, 10500 };


uint32_t interpolateSine(uint16_t angle, uint32_t scale) {
	int SinValue = 0;

	uint16_t index, nachkommaSt, Vorzeichen, richtung; //Vorzeichen0:1,2-Qudrant Vorzeichen1:3,4-Qudrant
	uint16_t Quadrant = (angle >> 14) & 3;
	Vorzeichen = angle >> 15;
	richtung = (angle >> 14) & 1;
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
int main(){

	int SinValueValues[181] = {10500, 11231, 11958, 12680, 13390, 14085, 14765, 15427, 16056, 16662, 17244, 17786, 18291, 18764, 19201, 19581, 19925, 20229, 20476, 20674, 20831, 20938, 20981, 20983, 20940, 20833, 20677, 20480, 20233, 19929, 19587, 19207, 18770, 18298, 17793, 17251, 16670, 16064, 15435, 14773, 14094, 13399, 12689, 11967, 11240, 10509, 9777, 9050, 8328, 7617, 6922, 6242, 5581, 4951, 4344, 3762, 3220, 2714, 2241, 1804, 1423, 1079, 773, 527, 328, 171, 63, 19, 17, 59, 166, 321, 518, 764, 1068, 1409, 1789, 2225, 2697, 3201, 3743, 4324, 4929, 5558, 6220, 6899, 7593, 8303, 9025, 9752, 10483, 11215, 11942, 12665, 13375, 14070, 14750, 15412, 16042, 16649, 17232, 17774, 18280, 18755, 19191, 19573, 19918, 20224, 20471, 20670, 20828, 20936, 20980, 20983, 20942, 20835, 20681, 20485, 20238, 19936, 19595, 19216, 18780, 18308, 17805, 17263, 16683, 16078, 15449, 14788, 14109, 13415, 12705, 11983, 11256, 10525, 9793, 9065, 8343, 7633, 6938, 6257, 5595, 4964, 4357, 3774, 3231, 2725, 2250, 1814, 1431, 1086, 779, 532, 332, 173, 66, 20, 16, 56, 164, 317, 512, 759, 1061, 1401, 1779, 2216, 2686, 3189, 3731, 4311, 4915, 5544, 6205, 6884, 7577, 8288, 9009, 9736, 10467};

	int exact = 1;
	int nearly = 1;
	int reportFirstOnly = false;
	int alreadyReported = 0;
	int ii;
	int jj;

	printf("Rendering interpolated sine from [0,8 PI):\n");
	for(ii=0; ii<1440; ii+=10){
		int sin = interpolateSine(ii * 0xFFFF / 360, (1 << 16));
		for(jj=0; jj<21000; jj+=500){
			if(sin >= jj) printf("#");
		}
		printf("\n");
	}

	printf("\nVerifying precision of interpolated sine SinValues\n");

	int maxDiff = 0;

	for(ii=0; ii<(1<<17); ii+=182*4){
		int expect = SinValueValues[ii/(182*4)];
//		int expect = doubleSine(ii, (1 << 16));
		int sin = interpolateSine(ii, (1 << 16));
		//printf("%d\n", sin);
		int doReport = !reportFirstOnly || !alreadyReported;
		if(expect != sin){
			int diff = abs(expect - sin);
			maxDiff = (diff > maxDiff)? diff : maxDiff;
		    if (diff <= 10) {
		        if (doReport) {
		            printf("i %5d: expected %10d, got %10d   [within +-10]\n", (ii&0xFFFF), expect, sin);
		        }
		        alreadyReported = 1;
		        exact = 0;
		    } else {
		        if (doReport) {
		            printf("i %5d: expected %10d, got %10d   [FAILURE]\n", (ii&0xFFFF), expect, sin);
		        }
		        alreadyReported = 1;
		        nearly = 0;
		    }
		} else {
			printf("i %5d: expected %10d, got %10d\n", ii&0xFFFF, expect, sin);
		}
	}

	if(exact){
		printf("Test passed\n");
		return 0;
	}
	else if(nearly){
		printf("Test passed (withing margin of error) (max deviation: %d)\n", maxDiff);
		return 0;
	}
	else{
		printf("Test failed!  (max deviation: %d)\n", maxDiff);
		return 1;
	}
}


