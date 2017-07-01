#include "simd.p5.h"

#define ARRAYSIZE 1000

float Array1[ARRAYSIZE];
float Array2[ARRAYSIZE];
float Array3[ARRAYSIZE];
#define SIZE   32768
float Array[2*SIZE];
float  Sums[1*SIZE];

float Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX
        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


void
SimdMul( float *a, float *b,   float *c,   int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
	__asm
	(
		".att_syntax\n\t"
		"movq    -24(%rbp), %rbx\n\t"		// a
		"movq    -32(%rbp), %rcx\n\t"		// b
		"movq    -40(%rbp), %rdx\n\t"		// c
	);

	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		__asm
		(
			".att_syntax\n\t"
			"movups	(%rbx), %xmm0\n\t"	// load the first sse register
			"movups	(%rcx), %xmm1\n\t"	// load the second sse register
			"mulps	%xmm1, %xmm0\n\t"	// do the multiply
			"movups	%xmm0, (%rdx)\n\t"	// store the result
			"addq $16, %rbx\n\t"
			"addq $16, %rcx\n\t"
			"addq $16, %rdx\n\t"
		);
	}

	for( int i = limit; i < len; i++ )
	{
		c[i] = a[i] * b[i];
	}
}



float
SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	__asm
	(
		".att_syntax\n\t"
		"movq    -40(%rbp), %rbx\n\t"		// a
		"movq    -48(%rbp), %rcx\n\t"		// b
		"leaq    -32(%rbp), %rdx\n\t"		// &sum[0]
		"movups	 (%rdx), %xmm2\n\t"		// 4 copies of 0. in xmm2
	);

	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		__asm
		(
			".att_syntax\n\t"
			"movups	(%rbx), %xmm0\n\t"	// load the first sse register
			"movups	(%rcx), %xmm1\n\t"	// load the second sse register
			"mulps	%xmm1, %xmm0\n\t"	// do the multiply
			"addps	%xmm0, %xmm2\n\t"	// do the add
			"addq $16, %rbx\n\t"
			"addq $16, %rcx\n\t"
		);
	}

	__asm
	(
		".att_syntax\n\t"
		"movups	 %xmm2, (%rdx)\n\t"	// copy the sums back to sum[ ]
	);

	for( int i = limit; i < len; i++ )
	{
		sum[i-limit] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}

int main()
{

	FILE *fp = fopen( "signal.txt", "r" );
if( fp == NULL )
{
	fprintf( stderr, "Cannot open file 'signal.txt'\n" );
	exit( 1 );
}
int Size;
fscanf( fp, "%d", &Size );
Size = SIZE;
for( int i = 0; i < Size; i++ )
{
	fscanf( fp, "%f", &Array[i] );
	Array[i+Size] = Array[i];		// duplicate the array
}
fclose( fp );	
	double maxMegaMults = 0.;
        double sumMegaMults = 0.;
	int NUMTRIES = 10;
	double megaMults;

	unsigned int seed = 0;

	for (int x = 0; x < ARRAYSIZE; x++)
        {
                Array1[x] = Ranf( &seed, -1.f, 1.f);
        }
	
	for (int x = 0; x < ARRAYSIZE; x++)
        {
                Array2[x] = Ranf( &seed, -1.f, 1.f);
        }
	
	for (int x = 0; x < ARRAYSIZE; x++)
        {
               Array3[x] = 0;
        }


	//for (int x = 0; x < NUMTRIES; x++)
	//{	
	double time0 = omp_get_wtime( );
	
	//SimdMul(Array1, Array2, Array3, ARRAYSIZE);
	int x;
	for (x = 0; x < 10; x++)
	{
	int shift = 0;
	for (shift = 0; shift < Size; shift++)
	{
	Sums[shift] = SimdMulSum( &Array[0], &Array[0+shift], Size );
	//SimdMulSum(Array1, Array2, ARRAYSIZE);
	}
	double time1 = omp_get_wtime( );

	
	//double time2 = omp_get_wtime( );
	
//	SimdMulSum(Array1, Array2, ARRAYSIZE);
//
//	double time3 = omp_get_wtime( );
	

	megaMults = (double)(SIZE * SIZE)/(time1-time0)/1000000.;
	sumMegaMults += megaMults;
	if( megaMults > maxMegaMults )
		maxMegaMults = megaMults;
	}
	//double avgMegaMults = sumMegaMults/(double)NUMTRIES;

	printf("SimdMul =  %8.2lf MegaHeights/Sec\n", megaMults);
	printf( " Peak Performance = %8.2lf MFLOPS\n", maxMegaMults );
	printf( "Average Performance = %8.2lf MFLOPS\n", sumMegaMults/(double)10 );


	//double megaHeights2 = (double)(ARRAYSIZE)/(time3-time2)/1000000.;
	
	//printf("SimdMulSum =  %8.2lf MegaHeights/Sec\n", megaHeights);
	
	//float elapsedTime1 = time1 - time0;
	
	//float elapsedTime2 = time3 - time2;

	//printf("Elapsed Time for SimdMul =  %.9lf\n", elapsedTime1);
	//printf("Elapsed Time for SimdMulSum =  %.9lf\n", elapsedTime1);
	

	

	return 0;
}
