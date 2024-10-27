#pragma once

//#define _CRT_SECURE_NO_DEPRECATE

#include <vector>
#include <utility>
#include <string>
#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <math.h>
#define ADD_WINDOW 0
typedef struct {
	long long value;
	int index;
} Element;
#define M_PI       3.14159265358979323846   
int compare(const void* a, const void* b);

class Audio {
public:
	// Overloaded Operators
	std::pair<short, short>& operator[](unsigned i) { return data[i]; }
	short* audio_data_right;
	short* audio_data_left;
	long long* energy_right;
	long long* energy_left;
	double* hanning_window;
	int** buffer;
	int** buffer_2;
	int buffer_size;
	int number_buffer;
	long long* sorted_energy_one;
	int* idx_one;
	long long* sorted_energy_two;
	int* idx_two;
	double window_sum = 0.0;
	// Constructors
	Audio(std::string str, int bufferSize);
	Audio() {};

	// Read / Write files
	void load_wav(std::string str);
	void write_wav(std::string str);

	// Get functions
	// Subchunk2Size = NumSamples * NumChannels * BitsPerSample / 8 <==> NumSamples = Subchunk2Size / (NumChannels*(BitsPerSample / 8))
	unsigned get_size() { return data.size(); }
	unsigned get_sample_rate() { return SampleRate; }
	unsigned get_n_channels() { return NumChannels; }
	// Set Functions
	void set_sample_rate(int n) { SampleRate = n; }
	void set_n_channels(int n) { if (n != 1 && n != 2)throw std::invalid_argument("n can only be 1 (MONO) or 2 (STEREO)!"); NumChannels = n; }
	void set_data_chunking() {
		int j = 0;

		for (int m = 0; m < this->number_buffer; m++) {
			for (int i = 0; i < this->buffer_size; i++) {
				this->buffer[m][i] = this->audio_data_right[j];
				this->buffer_2[m][i] = this->audio_data_left[j];
				j++;
			}
		}		
	}
	void set_calculate_block_energy() {
		for (int i = 0; i < this->number_buffer; i++) {
			for (int m = 0; m < this->buffer_size; m++) {
				long data = this->buffer[i][m] * this->buffer[i][m];
				long data2 =this->buffer_2[i][m] * this->buffer_2[i][m];
				this->energy_right[i] += sqrt(data);
				this->energy_left[i] += sqrt(data2);
			}
			//printf("%lld, index=%d\r\n",this->energy_right[i],i);
		}
	}


	void set_sort_with_indices(long long* array, int size, long long* sorted_array, int* indices) {
		//Element* elements = (Element*)malloc(size * sizeof(Element));
		Element* elements = new Element[size];
		for (int i = 0; i < size; i++) {
			elements[i].value = array[i];
			elements[i].index = i;
		}

		qsort(elements, size, sizeof(Element),compare);

		for (int i = 0; i < size; i++) {
			sorted_array[i] = elements[i].value;
			indices[i] = elements[i].index;
		}

		//free(elements);
		delete elements;
	}
	
protected:
	char type[5];
	char format[5];
	char Subchunk1ID[5];
	char Subchunk2ID[5];

	int ChunkSize;
	int Subchunk1Size;
	int SampleRate;
	int ByteRate;
	int Subchunk2Size;

	short AudioFormat;
	short NumChannels;
	short BlockAlign;
	short BitsPerSample;

	// utility
	unsigned NumSamples;

	std::vector<std::pair<short, short> > data;
	
};