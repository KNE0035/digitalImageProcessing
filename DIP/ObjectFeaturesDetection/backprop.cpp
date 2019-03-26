#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <vector>
#include "backprop.h"

#define LAMBDA 1
#define ETA 0.1

#define SQR( x ) ( ( x ) * ( x ) )

void randomize( double * p, int n ) 
{
	for ( int i = 0; i < n; i++ ) {
		p[i] = ( double )rand() / ( RAND_MAX );
	}
}

NN * createNN( int n, int h, int o ) 
{
	srand(time(NULL));
	NN * nn = new NN;
	
    nn->n = new int[3];
	nn->n[0] = n;
	nn->n[1] = h;
	nn->n[2] = o;
	nn->l = 3;

	nn->w = new double ** [nn->l - 1];
    

	for ( int k = 0; k < nn->l - 1; k++ ) 
    {
		nn->w[k] = new double * [nn->n[k + 1]];
		for ( int j = 0; j < nn->n[k + 1]; j++ ) 
        {
			nn->w[k][j] = new double[nn->n[k]];			
			randomize( nn->w[k][j], nn->n[k]);
			// BIAS
			//nn->w[k][j] = new double[nn->n[k] + 1];			
			//randomize( nn->w[k][j], nn->n[k] + 1 );
		}		
	}

	nn->y = new double * [nn->l];
	for ( int k = 0; k < nn->l; k++ ) {
		nn->y[k] = new double[nn->n[k]];
		memset( nn->y[k], 0, sizeof( double ) * nn->n[k] );
	}

	nn->in = nn->y[0];
	nn->out = nn->y[nn->l - 1];

	nn->d = new double * [nn->l];
	for ( int k = 0; k < nn->l; k++ ) {
		nn->d[k] = new double[nn->n[k]];
		memset( nn->d[k], 0, sizeof( double ) * nn->n[k] );
	}

	return nn;
}

void releaseNN( NN *& nn ) 
{
	for ( int k = 0; k < nn->l - 1; k++ ) {
		for ( int j = 0; j < nn->n[k + 1]; j++ ) {
			delete [] nn->w[k][j];
		}
		delete [] nn->w[k];
	}
	delete [] nn->w;
		
	for ( int k = 0; k < nn->l; k++ ) {
		delete [] nn->y[k];
	}
	delete [] nn->y;
	
	for ( int k = 0; k < nn->l; k++ ) {
		delete [] nn->d[k];
		
	}
	delete [] nn->d;

	delete [] nn->n;

	delete nn;
	nn = NULL;
}

void feedforward( NN * nn ) 
{ 
	for (int i = 0; i < (nn->l - 1); i++) {
		for (int j = 0; j < nn->n[i + 1]; j++) {
			double inputSum = 0;
			for (int k = 0; k < nn->n[i]; k++) {
				inputSum += nn->w[i][j][k] * nn->y[i][k];
			}
			nn->y[i + 1][j] = 1 / (1 + exp(-LAMBDA * inputSum));
		}
	}
}

double backpropagation( NN * nn, double * t ) 
{
	double error = 0.0;
	double errorSqrSum = 0.0;

	//get error for output layer and calculate derivation
	for (int i = 0; i < nn->n[nn->l - 1]; i++) {
		double nthOutput = nn->y[nn->l - 1][i];
		nn->d[nn->l - 1][i] = (t[i] - nthOutput) * LAMBDA * nthOutput * (1 - nthOutput);

		errorSqrSum += SQR(t[i] - nn->y[nn->l - 1][i]);
	}

	error = 0.5 * errorSqrSum;

	//propagate derivations to lower layers
	for (int i = nn->l - 1; i > 0; i--) {
		for (int j = 0; j < nn->n[i - 1]; j++) {
			double errorSum = 0;
			for (int k = 0; k < nn->n[i]; k++) {
				errorSum += nn->d[i][k] * nn->w[i - 1][k][j];
			}
			nn->d[i - 1][j] = errorSum * LAMBDA * nn->y[i - 1][j] * (1 - nn->y[i - 1][j]);
		}
	}
	 
	//change weights
	for (int i = 0; i < (nn->l - 1); i++) {
		for (int j = 0; j < nn->n[i + 1]; j++) {
			for (int k = 0; k < nn->n[i]; k++) {
				nn->w[i][j][k] += ETA * nn->d[i + 1][j] * nn->y[i][k];
			}	
		}
	}

	return error;
}

void setInput( NN * nn, double * in, bool verbose ) 
{
	memcpy( nn->in, in, sizeof( double ) * nn->n[0] );

	if ( verbose ) {
		printf( "input=(" );
		for ( int i = 0; i < nn->n[0]; i++ ) {
			printf( "%0.3f", nn->in[i] );
			if ( i < nn->n[0] - 1 ) {
				printf( ", " );
			}
		}
		printf( ")\n" );
	}
}

int getOutput( NN * nn, bool verbose ) 
{	
    double max = 0.0;
    int max_i = 0;
    if(verbose) printf( " output=" );
	for ( int i = 0; i < nn->n[nn->l - 1]; i++ ) 
    {
		if(verbose) printf( "%0.3f ", nn->out[i] );
        if(nn->out[i] > max) {
            max = nn->out[i];
            max_i = i;
        }
	}
	if(verbose) printf( " -> %d\n" , max_i);
    //if(nn->out[0] > nn->out[1] && nn->out[0] - nn->out[1] < 0.1) return 2;
    return max_i;
}

double trainOneStep(NN * nn, double * input, int label, bool verbose) {
	setInput(nn, input, verbose);
	feedforward(nn);
	double* outVectorLabel = new double[nn->n[nn->l - 1]];
	memset(outVectorLabel, 0, sizeof(double) * nn->n[nn->l - 1]);

	for (int i = 0; i < nn->n[nn->l - 1]; i++) {
		if (i == label) {
			outVectorLabel[i] = 1;
		}
	}

	double error = backpropagation(nn, outVectorLabel);
	delete[] outVectorLabel;

	if (3 == getOutput(nn, false) && label == 3) {
		printf("a");
	}
	else if(label == 3){
		printf("h");
	}

	return error;
}

int networkClassify(NN * nn, double * input) {
	setInput(nn, input, false);
	feedforward(nn);
	return getOutput(nn, false);
}