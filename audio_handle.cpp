#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "Audio.h"
#include <math.h>
#include <cmath>
#include <complex>
#include "kiss_fft.h"
int main(int argc, char** argv)
{
	if (argc != 3) {
		printf("Not enough input parameter\r\n");
		exit(1);
	}
	//std::string path = "D:/audio_/noise.wav";
	std::string path = argv[1];
	std::string output_path = argv[2];
	Audio inputfile(path, 960);
	printf("sample rate: %d\r\n", inputfile.get_sample_rate());
	printf("Number of channel:%d\r\n", inputfile.get_n_channels());
	printf("size:%d\r\n", inputfile.get_size());
	inputfile.set_data_chunking();
	inputfile.set_calculate_block_energy();
	/********************kiss fft init************************/
	kiss_fft_cfg fft_cfg = kiss_fft_alloc(inputfile.buffer_size, 0, NULL, NULL);   // FFT config (0 for forward FFT)
	kiss_fft_cfg ifft_cfg = kiss_fft_alloc(inputfile.buffer_size, 1, NULL, NULL);  // IFFT config (1 for inverse FFT)
	kiss_fft_cpx* cx_in = new kiss_fft_cpx[inputfile.buffer_size];
	kiss_fft_cpx* cx_out = new kiss_fft_cpx[inputfile.buffer_size];
	kiss_fft_cpx* cx_in2 = new kiss_fft_cpx[inputfile.buffer_size];
	kiss_fft_cpx* cx_out2 = new kiss_fft_cpx[inputfile.buffer_size];
	/****************************************************************************/

	double* amplitude_noise = new double[inputfile.buffer_size];
	memset(amplitude_noise, 0, sizeof(double) * inputfile.buffer_size);
	/*Estimate the noise spectrum in the first 5 frames for subsequent noise subtraction */
	/**************************channel 1**************************************************/
	int start_position = 0;
	int end_position = 5;
	float scale = 1.7;//There seems to be a problem with the amplitude spectrum calculation of this fft library. 
					//A correction scale is made to the amplitude
	for (int m = start_position; m < end_position; m++) {
		for (int i = 0; i < inputfile.buffer_size; i++) {
			cx_in[i].r = inputfile.buffer[m][i] * inputfile.hanning_window[i];
			cx_in[i].i = 0;
		}
		kiss_fft(fft_cfg, cx_in, cx_out);
		for (int i = 0; i < inputfile.buffer_size; i++) {
			amplitude_noise[i] += std::sqrt(cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i)* scale;
		}
	}
	for (int i = 0; i < inputfile.buffer_size; i++) {
		amplitude_noise[i] = amplitude_noise[i] / 5;
	}

	/**************************channel 2**************************************************/
	double* amplitude_noise2 = new double[inputfile.buffer_size];
	memset(amplitude_noise2, 0, sizeof(double) * inputfile.buffer_size);
	for (int m = start_position; m < end_position; m++) {
		for (int i = 0; i < inputfile.buffer_size; i++) {
			cx_in2[i].r = inputfile.buffer_2[m][i] * inputfile.hanning_window[i];
			cx_in2[i].i = 0;
		}
		kiss_fft(fft_cfg, cx_in2, cx_out2);
		for (int i = 0; i < inputfile.buffer_size; i++) {
			amplitude_noise2[i] += std::sqrt(cx_out2[i].r * cx_out2[i].r + cx_out2[i].i * cx_out2[i].i)* scale;
		}
	}
	for (int i = 0; i < inputfile.buffer_size; i++) {
		amplitude_noise2[i] = (amplitude_noise2[i] / 5);
	}
	/****************************************************************************/
	for (int i = 0; i < inputfile.number_buffer; i++) {
		kiss_fft_cpx* cx_in_temp = new kiss_fft_cpx[inputfile.buffer_size];
		kiss_fft_cpx* cx_out_temp = new kiss_fft_cpx[inputfile.buffer_size];
		kiss_fft_cpx* cx_in2_temp = new kiss_fft_cpx[inputfile.buffer_size];
		kiss_fft_cpx* cx_out2_temp = new kiss_fft_cpx[inputfile.buffer_size];
		kiss_fft_cpx* cx_out_temp_ifft = new kiss_fft_cpx[inputfile.buffer_size];
		kiss_fft_cpx* cx_out2_temp_ifft = new kiss_fft_cpx[inputfile.buffer_size];
		for (int m = 0; m < inputfile.buffer_size; m++) {
			cx_in_temp[m].r = inputfile.buffer[i][m] * inputfile.hanning_window[m];
			cx_in_temp[m].i = 0;
			cx_in2_temp[m].r = inputfile.buffer_2[i][m] * inputfile.hanning_window[m];
			cx_in2_temp[m].i = 0;
		}
		kiss_fft(fft_cfg, cx_in_temp, cx_out_temp);
		kiss_fft(fft_cfg, cx_in2_temp, cx_out2_temp);
		/*The spectral amplitude of the clean speech is calculated by spectral subtraction.*/
		for (int m = 0; m < inputfile.buffer_size; m++) {
			double amplitude_segment = std::sqrt(cx_out_temp[m].r * cx_out_temp[m].r + cx_out_temp[m].i * cx_out_temp[m].i);
			double amplitude_clean = std::max(amplitude_segment - amplitude_noise[m], 0.0); 
			std::complex<double> z(cx_out_temp[m].r, cx_out_temp[m].i);
			double phase = std::arg(z);  
			std::complex<double> complex_val = std::polar(amplitude_clean, phase);
			cx_out_temp[m].r = complex_val.real();  
			cx_out_temp[m].i = complex_val.imag();  
		}
		for (int m = 0; m < inputfile.buffer_size; m++) {
			double amplitude_segment = std::sqrt(cx_out2_temp[m].r * cx_out2_temp[m].r + cx_out2_temp[m].i * cx_out2_temp[m].i);
			double amplitude_clean = std::max(amplitude_segment - amplitude_noise2[m], 0.0);
			std::complex<double> z(cx_out2_temp[m].r, cx_out2_temp[m].i);
			double phase = std::arg(z);
			std::complex<double> complex_val = std::polar(amplitude_clean, phase);
			cx_out2_temp[m].r = complex_val.real();  
			cx_out2_temp[m].i = complex_val.imag();  
		}
		/*Convert the processed frequency domain signal back to time domain*/
		kiss_fft(ifft_cfg, cx_out_temp, cx_out_temp_ifft);
		kiss_fft(ifft_cfg, cx_out2_temp, cx_out2_temp_ifft);
		for (int m = 0; m < inputfile.buffer_size; m++) {  
			inputfile.buffer[i][m] = cx_out_temp_ifft[m].r;
		}
		for (int m = 0; m < inputfile.buffer_size; m++) {
			inputfile.buffer_2[i][m] = cx_out2_temp_ifft[m].r;
		}
		delete[] cx_in2_temp;
		delete[] cx_in_temp;
		delete[] cx_out2_temp;
		delete[] cx_out_temp;
		delete[] cx_out_temp_ifft;
		delete[] cx_out2_temp_ifft;
	}

	inputfile.write_wav(output_path);
	delete[] amplitude_noise;
	delete[] amplitude_noise2;
	
	return 0;
}


