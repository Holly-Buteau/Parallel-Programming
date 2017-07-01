// 1. Program header

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <omp.h>

#define NUMTRIES 10
#include "CL/cl.h"
#include "CL/cl_platform.h"

#define SIZE   32768




///#ifndef NMB
//#define	NMB			64
///#endif

#define NUM_ELEMENTS		6000

#ifndef LOCAL_SIZE
#define	LOCAL_SIZE		64
#endif

#define	NUM_WORK_GROUPS		NUM_ELEMENTS/LOCAL_SIZE

//const char *			CL_FILE_NAME = { "first.cl" };
//const float			TOL = 0.0001f;

//void				Wait( cl_command_queue );
//int				LookAtTheBits( float );

int
LookAtTheBits( float fp )
{
	int *ip = (int *)&fp;
	return *ip;
}

// wait until all queued tasks have taken place:

void
Wait( cl_command_queue queue )
{
      cl_event wait;
      cl_int      status;

      status = clEnqueueMarker( queue, &wait );
      if( status != CL_SUCCESS )
	      fprintf( stderr, "Wait: clEnqueueMarker failed\n" );

      status = clWaitForEvents( 1, &wait );
      if( status != CL_SUCCESS )
	      fprintf( stderr, "Wait: clWaitForEvents failed\n" );
}



int
main( int argc, char *argv[ ] )
{

	int     Size;
        FILE *  fp;
        int     i;
	
	float *hArray = new float[ 2*Size ];
	float *hSums  = new float[ 1*Size ];

	//int     Size;
	//FILE *  fp;
	//int     i;
	fp = fopen( "signal.txt", "r" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open file 'signal.txt'\n" );
		exit( 1 );
	}
	fscanf( fp, "%d", &Size );
	Size = SIZE;
	
	for( i = 0; i < Size; i++ )
	{
		fscanf( fp, "%f", &hArray[i] );
		hArray[i+Size] = hArray[i];		// duplicate the array
	}
	fclose( fp );

	
	
	// see if we can even open the opencl kernel program
	// (no point going on if we can't):
	size_t numWorkGroups = NUM_ELEMENTS / LOCAL_SIZE;
	const char *			CL_FILE_NAME = { "Project7b-GPUOpenCL.cl" };
	const float			TOL = 0.0001f;
	FILE *fpt;
#ifdef WIN32
	errno_t err = fopen_s( &fpt, CL_FILE_NAME, "r" );
	if( err != 0 )
#else
	fpt = fopen( CL_FILE_NAME, "r" );
	if( fpt == NULL )
#endif
	{
		fprintf( stderr, "Cannot open OpenCL source file '%s'\n", CL_FILE_NAME );
		return 1;
	}


	cl_int status;		// returned status from opencl calls
				// test against CL_SUCCESS

	// get the platform id:

	cl_platform_id platform;
	status = clGetPlatformIDs( 1, &platform, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clGetPlatformIDs failed (2)\n" );
	
	// get the device id:

	cl_device_id device;
	status = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clGetDeviceIDs failed (2)\n" );

	// 2. allocate the host memory buffers:

	//float *hArray = new float[ 2*Size ];
	//float *hSums  = new float[ 1*Size ];
	//size_t abSize = NUM_ELEMENTS * sizeof(float);
	//size_t cSize = numWorkGroups * sizeof(float);	

	// fill the host memory buffers:

	

	//size_t dataSize = NUM_ELEMENTS * sizeof(float);

	// 3. create an opencl context:

	cl_context context = clCreateContext( NULL, 1, &device, NULL, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateContext failed\n" );

	// 4. create an opencl command queue:

	cl_command_queue cmdQueue = clCreateCommandQueue( context, device, 0, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateCommandQueue failed\n" );

	// 5. allocate the device memory buffers:

	/*cl_mem dA = clCreateBuffer( context, CL_MEM_READ_ONLY,  dataSize, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateBuffer failed (1)\n" );

	cl_mem dB = clCreateBuffer( context, CL_MEM_READ_ONLY,  dataSize, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateBuffer failed (2)\n" );

	cl_mem dC = clCreateBuffer( context, CL_MEM_WRITE_ONLY, dataSize, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateBuffer failed (3)\n" );
	*/

	
	cl_mem dArray = clCreateBuffer( context, CL_MEM_READ_ONLY,  2*Size*sizeof(cl_float), NULL, &status );
	cl_mem dSums  = clCreateBuffer( context, CL_MEM_WRITE_ONLY, 1*Size*sizeof(cl_float), NULL, &status );
	
	// 6. enqueue the 2 commands to write the data from the host buffers to the device buffers:

	/*status = clEnqueueWriteBuffer( cmdQueue, dA, CL_FALSE, 0, dataSize, hA, 0, NULL, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueWriteBuffer failed (1)\n" );

	status = clEnqueueWriteBuffer( cmdQueue, dB, CL_FALSE, 0, dataSize, hB, 0, NULL, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueWriteBuffer failed (2)\n" );

	
	status = clEnqueueWriteBuffer( cmdQueue, dD, CL_FALSE, 0, dataSize, hD, 0, NULL, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueWriteBuffer failed (1)\n" );
	*/


	//status = clEnqueueWriteBuffer( cmdQueue, dA, CL_FALSE, 0, abSize, hA, 0, NULL, NULL );
	//status = clEnqueueWriteBuffer( cmdQueue, dB, CL_FALSE, 0, abSize, hB, 0, NULL, NULL );

	Wait( cmdQueue );

	// 7. read the kernel code from a file:

	fseek( fp, 0, SEEK_END );
	size_t fileSize = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	char *clProgramText = new char[ fileSize+1 ];		// leave room for '\0'
	size_t n = fread( clProgramText, 1, fileSize, fp );
	clProgramText[fileSize] = '\0';
	fclose( fp );
	if( n != fileSize )
		fprintf( stderr, "Expected to read %d bytes read from '%s' -- actually read %d.\n", fileSize, CL_FILE_NAME, n );

	// create the text for the kernel program:

	char *strings[1];
	strings[0] = clProgramText;
	cl_program program = clCreateProgramWithSource( context, 1, (const char **)strings, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateProgramWithSource failed\n" );
	delete [ ] clProgramText;

	// 8. compile and link the kernel code:

	const char *options = { "" };
	status = clBuildProgram( program, 1, &device, options, NULL, NULL );
	if( status != CL_SUCCESS )
	{
		size_t size;
		clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size );
		cl_char *log = new cl_char[ size ];
		clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, size, log, NULL );
		fprintf( stderr, "clBuildProgram failed:\n%s\n", log );
		delete [ ] log;
	}

	// 9. create the kernel object:

	cl_kernel kernel = clCreateKernel( program, "ArrayMultReduce", &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateKernel failed\n" );

	// 10. setup the arguments to the kernel object:

	/*status = clSetKernelArg( kernel, 0, sizeof(cl_mem), &dA );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clSetKernelArg failed (1)\n" );

	status = clSetKernelArg( kernel, 1, sizeof(cl_mem), &dB );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clSetKernelArg failed (2)\n" );

	status = clSetKernelArg( kernel, 2, sizeof(cl_mem), &dC );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clSetKernelArg failed (3)\n" );

	
	status = clSetKernelArg( kernel, 3, sizeof(cl_mem), &dD );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clSetKernelArg failed (3)\n" );
*/

	
	status = clSetKernelArg( kernel, 0, sizeof(cl_mem), &dArray );
	status = clSetKernelArg( kernel, 1, sizeof(cl_mem), &dSums  );	

	// 11. enqueue the kernel object for execution:

	
	size_t globalWorkSize[3] = { Size,         1, 1 };
	size_t localWorkSize[3]  = { LOCAL_SIZE,   1, 1 };

	Wait( cmdQueue );
	double time0 = omp_get_wtime( );

	time0 = omp_get_wtime( );

	status = clEnqueueNDRangeKernel( cmdQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueNDRangeKernel failed: %d\n", status );

	Wait( cmdQueue );
	double time1 = omp_get_wtime( );

	// 12. read the results buffer back from the device to the host:

	//status = clEnqueueReadBuffer( cmdQueue, dC, CL_TRUE, 0, dataSize, hC, 0, NULL, NULL );
	//status = clEnqueueReadBuffer( cmdQueue, dC, CL_TRUE, 0, numWorkGroups*sizeof(float), hC, 0, NULL, NULL );
	//if( status != CL_SUCCESS )
	//	fprintf( stderr, "clEnqueueReadBuffer failed\n" );
	//PrintCLError( status, "clEnqueueReadBufferl failed: " );
	// did it work?

	//Wait( cmdQueue );
/*	double timeb = omp_get_wtime( );
	double megaMults = (double)SIZE * SIZE/(timeb-timea)/1000000.;
        sumMegaMults += megaMults;
        if( megaMults > maxMegaMults )
                maxMegaMults = megaMults;
        }

	double avgMegaMults = sumMegaMults/(double)NUMTRIES;
         printf( "Peak Performance 4 threads = %8.2lf MegaMults/Sec\n", maxMegaMults );
         printf( "Average Performance 4 threads = %8.2lf MegaMults/Sec\n", avgMegaMults );
*/
	int q;
	int shift;


	 	
	for (shift = 0; shift < Size; shift++ )
{
	float sum = 0.;
	for( int i = 0; i < Size; i++ )
	{
		sum += hArray[i] * hArray[i + shift];
	}
	hSums[shift] = sum;	// note the "fix #2" from false sharing if you are using OpenMP
}


/*
          sumMegaMults += megaMults;
          if( megaMults > maxMegaMults )
                  maxMegaMults = megaMults;
          }
 
	 double avgMegaMults = sumMegaMults/(double)NUMTRIES;
         printf( "Peak Performance 4 threads = %8.2lf MegaMults/Sec\n", maxMegaMults );
        printf( "Average Performance 4 threads = %8.2lf MegaMults/Sec\n", avgMegaMults );

*/
	int x;

	for (x = 1; x <= 512; x++)
	{
		printf("%f\n", hSums[x]);
	}
	

#ifdef WIN32
	Sleep( 2000 );
#endif


	// 13. clean everything up:

	clReleaseKernel(        kernel   );
	clReleaseProgram(       program  );
	clReleaseCommandQueue(  cmdQueue );
	clReleaseMemObject(     dArray  );
	clReleaseMemObject(     dSums  );
	//clReleaseMemObject(     dC  );
	
	delete [ ] hArray;
	delete [ ] hSums;
	//delete [ ] hC;

	  /*double timeb = omp_get_wtime( );
         double megaMults = (double)SIZE * SIZE/(timeb-timea)/1000000.;
         sumMegaMults += megaMults;
         if( megaMults > maxMegaMults )
                 maxMegaMults = megaMults;
         }

         double avgMegaMults = sumMegaMults/(double)NUMTRIES;
          printf( "Peak Performance 4 threads = %8.2lf MegaMults/Sec\n", maxMegaMults );
          printf( "Average Performance 4 threads = %8.2lf MegaMults/Sec\n", avgMegaMults );

*/
	return 0;
}





