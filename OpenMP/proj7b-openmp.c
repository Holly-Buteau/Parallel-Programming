#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>
#include <math.h>

#define SIZE   32768
#define NUMT 1 
#define NUMTRIES 10

float Array[2*SIZE];
float  Sums[1*SIZE];


int main()
{
	#ifndef _OPENMP
        fprintf( stderr, "OpenMP is not supported here -- sorry.\n" );
        return 1;
	#endif

	double maxMegaMults = 0.;
        double sumMegaMults = 0.;


	omp_set_num_threads(NUMT);
        fprintf( stderr, "Cannot open file 'signal.txt'\n" );

	int     Size;
	FILE *  fp;
	int     i;
	int 	shift;
	fp = fopen( "signal.txt", "r" );

	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open file 'signal.txt'\n" );
		exit( 1 );
	}

	fscanf( fp, "%d", &Size );
	Size = SIZE;
	//Array = (float *)malloc( 2 * Size * sizeof(float) );
	//Sums  = (float *)malloc( 1 * Size * sizeof(float) );

	for( i = 0; i < Size; i++ )
	{
		fscanf( fp, "%f", &Array[i] );
		Array[i+Size] = Array[i];		// duplicate the array
	}

	fclose( fp );

//	double time0 = omp_get_wtime( );
	int t;
	for( t = 0; t < NUMTRIES; t++ )
	{
	double time0 = omp_get_wtime( );

	#pragma omp parallel for
	for( shift = 0; shift < Size; shift++ )
	{
		float sum = 0.;
		for( i = 0; i < Size; i++ )
		{
			sum += Array[i] * Array[i + shift];
		}
	Sums[shift] = sum;	// note the "fix #2" from false sharing if you are using OpenMP
	}
	double time1 = omp_get_wtime( );

	double megaMults = (double)SIZE * SIZE/(time1-time0)/1000000.;
        sumMegaMults += megaMults;
        if( megaMults > maxMegaMults )
		maxMegaMults = megaMults;

	}
	double avgMegaMults = sumMegaMults/(double)NUMTRIES;
        printf( "Peak Performance %d threads = %8.2lf MegaMults/Sec\n", NUMT, maxMegaMults );
        printf( "Average Performance %d threads = %8.2lf MegaMults/Sec\n", NUMT, avgMegaMults );

	int x;

	/*for(x = 1; x <= 512; x++)
	{
		printf("%f\n", Sums[x]);
	}
*/
	return 0;
}
