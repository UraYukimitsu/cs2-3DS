#include "blowfish.h"

#define S(x,i) (bf->SBoxes[i][x.w.byte##i])
#define bf_F(x) (((S(x,0) + S(x,1)) ^ S(x,2)) + S(x,3))
#define ROUND(a,b,n) (a.dword ^= bf_F(b) ^ bf->PArray[n])

void Blowfish_encipher(Blowfish *bf, DWORD *xl, DWORD *xr)
{
	aword Xl, Xr;
	
	Xl.dword = *xl;
	Xr.dword = *xr;
	
	Xl.dword ^= bf->PArray[0];
	ROUND (Xr, Xl, 1) ;  ROUND (Xl, Xr, 2) ;
	ROUND (Xr, Xl, 3) ;  ROUND (Xl, Xr, 4) ;
	ROUND (Xr, Xl, 5) ;  ROUND (Xl, Xr, 6) ;
	ROUND (Xr, Xl, 7) ;  ROUND (Xl, Xr, 8) ;
	ROUND (Xr, Xl, 9) ;  ROUND (Xl, Xr, 10) ;
	ROUND (Xr, Xl, 11) ; ROUND (Xl, Xr, 12) ;
	ROUND (Xr, Xl, 13) ; ROUND (Xl, Xr, 14) ;
	ROUND (Xr, Xl, 15) ; ROUND (Xl, Xr, 16) ;
	Xr.dword ^= bf->PArray[17];
	
	(*xr) = Xl.dword;
	(*xl) = Xr.dword;
}

void Blowfish_decipher(Blowfish *bf, DWORD *xl, DWORD *xr)
{
	aword Xl, Xr;
	
	Xl.dword = *xl;
	Xr.dword = *xr;
	
	Xl.dword ^= bf->PArray[17];
	ROUND (Xr, Xl, 16) ;  ROUND (Xl, Xr, 15) ;
	ROUND (Xr, Xl, 14) ;  ROUND (Xl, Xr, 13) ;
	ROUND (Xr, Xl, 12) ;  ROUND (Xl, Xr, 11) ;
	ROUND (Xr, Xl, 10) ;  ROUND (Xl, Xr, 9) ;
	ROUND (Xr, Xl, 8) ;   ROUND (Xl, Xr, 7) ;
	ROUND (Xr, Xl, 6) ;   ROUND (Xl, Xr, 5) ;
	ROUND (Xr, Xl, 4) ;   ROUND (Xl, Xr, 3) ;
	ROUND (Xr, Xl, 2) ;   ROUND (Xl, Xr, 1) ;
	Xr.dword ^= bf->PArray[0];
	
	(*xr) = Xl.dword;
	(*xl) = Xr.dword;
}

void bf_initialize(Blowfish *bf, BYTE key[], int keybytes)
{
	int i, j;
	DWORD data, datal, datar;
	aword temp;
	
	for(i = 0; i < 18; i++)
		bf->PArray[i] = bf_P[i];
		
	for(i = 0; i < 4; i++)
		for(j = 0; j < 256; j++)
			bf->SBoxes[i][j] = bf_S[i][j];
			
	j = 0;
	for(i = 0; i < NPASS + 2; ++i)
	{
		temp.dword = 0;
		temp.w.byte0 = key[j];
		temp.w.byte1 = key[(j + 1) % keybytes];
		temp.w.byte2 = key[(j + 2) % keybytes];
		temp.w.byte3 = key[(j + 3) % keybytes];
		data = temp.dword;
		bf->PArray[i] ^= data;
		j = (j + 4) % keybytes;
	}
	
	datal = 0;
	datar = 0;
	
	for(i = 0; i < NPASS + 2; i += 2)
	{
		Blowfish_encipher(bf, &datal, &datar);
		bf->PArray[i] = datal;
		bf->PArray[i + 1] = datar;
	}
	
	for(i = 0; i < 4; ++i)
	{
		for(j = 0; j < 256; j += 2)
		{
			Blowfish_encipher(bf,&datal, &datar);
			bf->SBoxes[i][j] = datal;
			bf->SBoxes[i][j + 1] = datar;
		}
	}
}

DWORD bf_encode(Blowfish *bf, BYTE *pInput, BYTE *pOutput, DWORD lSize)
{
	DWORD lCount, lOutSize, lGoodBytes;
	BYTE *pi, *po;
	int i, j;
	int SameDest = (pInput == pOutput ? 1: 0);
	
	lOutSize = bf_getOutputLength(lSize);
	
	for(lCount = 0; lCount < lOutSize; lCount += 8)
	{
		if(SameDest) //If data is being written in the input buffer
		{
			if(lCount < lSize - 7) //If not dealing with uneven bytes at end
				Blowfish_encipher(bf, (DWORD *) pInput, (DWORD *) (pInput + 4));
			else //Pad end of data with null bytes to complete encryption
			{
				po = pInput + lSize; //Point at byte past the end of actual data
				j = (int) (lOutSize - lSize); //Number of null bytes to add
				for(i = 0; i < j; i++)
					*po++ = 0;
					
				Blowfish_encipher(bf, (DWORD *) pInput, (DWORD *) (pInput + 4));
			}
			pInput += 8;
		}
		else //Output buffer different from input buffer, must copy output to input prior to encryption
		{
			if(lCount < lSize - 7) //If not dealing with uneven bytes at end
			{
				pi = pInput;
				po = pOutput;
				for(i = 0; i < 8; i++) //Copy bytes
					*po++ = *pi++;
				Blowfish_encipher(bf, (DWORD *) pInput, (DWORD *) (pInput + 4));
			}
			else //Pad end of data with null bytes to complete encryption
			{
				lGoodBytes = lSize - lCount;
				po = pOutput;
				
				for(i = 0; i < (int) lGoodBytes; i++)
					*po++ = *pInput++;
				for(j = i; j < 8; i++)
					*po++ = 0;
				Blowfish_encipher(bf, (DWORD *) pInput, (DWORD *) (pInput + 4));
			}
			pInput += 8;
			pOutput += 8;
		}
	}
	return lOutSize;
}

void bf_decode(Blowfish *bf, BYTE *pInput, BYTE *pOutput, DWORD lSize)
{
	DWORD lCount;
	BYTE *pi, *po;
	int i;
	int SameDest = (pInput == pOutput ? 1 : 0);
	
	for(lCount = 0; lCount < lSize; lCount += 8)
	{
		if (SameDest)	//If encoded data is being written into the input buffer
		{
	 	 	Blowfish_decipher(bf, (DWORD *) pInput, (DWORD *) (pInput + 4)) ;
		 	pInput += 8 ;
		}
		else 			//Output buffer not equal to input buffer
		{
	 		pi = pInput ;
	 		po = pOutput ;
	 		for (i = 0 ; i < 8 ; i++)
	 			*po++ = *pi++ ;
	 	 	Blowfish_decipher(bf, (DWORD *) pOutput, (DWORD *) (pOutput + 4)) ;
		 	pInput += 8 ;
		 	pOutput += 8 ;
		}
	}
}

DWORD bf_encrypt(Blowfish *bf, BYTE *pInput, DWORD lSize)
{
	BYTE *pOutput = NULL;
	DWORD ret;
	if(lSize != bf_getOutputLength(lSize))
		printf("Input length != Output length\n");
	pOutput = malloc(lSize * sizeof(BYTE));
	ret = bf_encode(bf, pInput, pOutput, lSize);
	memcpy(pInput, pOutput, lSize);
	free(pOutput);
	return ret;
}

void bf_decrypt(Blowfish *bf, BYTE *pInput, DWORD lSize)
{
	BYTE *pOutput = NULL;
	
	if(lSize != bf_getOutputLength(lSize))
		printf("Input length != Output length\n");
	pOutput = malloc(lSize * sizeof(BYTE));
	bf_decode(bf, pInput, pOutput, lSize);
	memcpy(pInput, pOutput, lSize);
	free(pOutput);
}
