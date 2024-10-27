#pragma once

#include "Audio.h"
int compare(const void* a, const void* b) {
	Element* elemA = (Element*)a;
	Element* elemB = (Element*)b;
	return (int)(elemB->value - elemA->value);
}
Audio::Audio(std::string str,int bufferSize) {
	this->buffer_size = bufferSize;
	if (str.substr(str.size() - 4) != ".wav")
		throw std::invalid_argument("Can only read WAV files!");
	load_wav(str);
}

void Audio::load_wav(std::string str) {
    FILE* fp = fopen(str.c_str(), "rb");
    if (fp == NULL)  
        throw std::runtime_error("Error opening file!");

 
    char type[5] = {0};  
    fread(type, sizeof(char), 4, fp);
    type[4] = '\0';

    printf("Chunk ID: %s\n", type);  
    if (std::strncmp(type, "RIFF", 4) != 0)
        throw std::runtime_error("Not a RIFF file!");

   
    int ChunkSize = 0;
    fread(&ChunkSize, sizeof(int), 1, fp);
    printf("Chunk Size: %d\n", ChunkSize);  

    
    char format[5] = {0};
    fread(format, sizeof(char), 4, fp);
    format[4] = '\0';

    printf("Format: %s\n", format);  
    if (std::strncmp(format, "WAVE", 4) != 0)
        throw std::runtime_error("Not a WAVE format!");

    
    char Subchunk1ID[5] = {0};
    fread(Subchunk1ID, sizeof(char), 4, fp);
    Subchunk1ID[4] = '\0';

    printf("Subchunk1 ID: %s\n", Subchunk1ID);  
    if (std::strncmp(Subchunk1ID, "fmt ", 4) != 0)
        throw std::runtime_error("Missing fmt header!");
	fread(&Subchunk1Size, sizeof(int), 1, fp);
	fread(&AudioFormat, sizeof(short), 1, fp);
	fread(&NumChannels, sizeof(short), 1, fp);
	fread(&SampleRate, sizeof(int), 1, fp);
	fread(&ByteRate, sizeof(int), 1, fp);
	fread(&BlockAlign, sizeof(short), 1, fp);
	fread(&BitsPerSample, sizeof(short), 1, fp);

	// 2nd Subchunk
	fread(Subchunk2ID, sizeof(char), 4, fp);
	if (strcmp(Subchunk2ID, "data") != 0)
		throw std::runtime_error("Missing data header!");
	fread(&Subchunk2Size, sizeof(int), 1, fp);
	// Data

	//Subchunk2Size = NumSamples * NumChannels * BitsPerSample/8
	int NumSamples = Subchunk2Size / (NumChannels * (BitsPerSample / 8));

	data = std::vector<std::pair<short, short> >(NumSamples);
	audio_data_right = new short[NumSamples];
	audio_data_left = new short[NumSamples];
	for (int i = 0; i < NumSamples; i++) {
		fread(&data[i].first, sizeof(short), 1, fp);
		fread(&data[i].second, sizeof(short), 1, fp);
		audio_data_right[i] = data[i].second;
		audio_data_left[i] = data[i].first;
		//printf("1st:%d,2nd:%d\r\n", audio_data_left[i], audio_data_right[i]);
	}
	
	hanning_window = new double[this->buffer_size];
	for (int i = 0; i < this->buffer_size; i++) {
#if ADD_WINDOW == 1
		hanning_window[i] = 0.5 * (1 - cos(2 * M_PI * i / (this->buffer_size - 1)));
#else
		hanning_window[i] =1.0;
#endif
		window_sum += hanning_window[i];
	}

	number_buffer = NumSamples / buffer_size;
	number_buffer = floor(number_buffer);
	buffer = new int* [number_buffer];
	buffer_2 = new int* [number_buffer];
	for (int i = 0; i < number_buffer; i++) {
		buffer[i] = new int[buffer_size];
		buffer_2[i] = new int[buffer_size];
	}
	energy_right = new long long[number_buffer];
	energy_left = new long long[number_buffer];
	sorted_energy_one = new long long[number_buffer];
	idx_one = new int[number_buffer];
	sorted_energy_two = new long long[number_buffer];
	idx_two = new int[number_buffer];
	for (int i = 0; i < this->number_buffer; i++) {
		this->energy_right[i] = 0;
		this->energy_left[i] = 0;
	}
	fclose(fp);
}

void Audio::write_wav(std::string str) {
    FILE* fp = fopen(str.c_str(), "wb");
    if (fp == NULL) {
        throw std::runtime_error("Error opening file for writing!");
    }

    // RIFF Chunk
    fwrite("RIFF", sizeof(char), 4, fp);
    int chunkSize = 36 + Subchunk2Size;  // 4 + (8 + Subchunk1Size) + (8 + Subchunk2Size)
    fwrite(&chunkSize, sizeof(int), 1, fp);
    fwrite("WAVE", sizeof(char), 4, fp);

    // fmt Subchunk
    fwrite("fmt ", sizeof(char), 4, fp);
    int subchunk1Size = 16;  // PCM
    fwrite(&subchunk1Size, sizeof(int), 1, fp);
    short audioFormat = 1;  // PCM = 1
    fwrite(&audioFormat, sizeof(short), 1, fp);
    fwrite(&NumChannels, sizeof(short), 1, fp);
    fwrite(&SampleRate, sizeof(int), 1, fp);
    int byteRate = SampleRate * NumChannels * BitsPerSample / 8;
    fwrite(&byteRate, sizeof(int), 1, fp);
    short blockAlign = NumChannels * BitsPerSample / 8;
    fwrite(&blockAlign, sizeof(short), 1, fp);
    fwrite(&BitsPerSample, sizeof(short), 1, fp);

    // data Subchunk
    fwrite("data", sizeof(char), 4, fp);
    int dataSize = data.size() * sizeof(short) * 2;  
    fwrite(&dataSize, sizeof(int), 1, fp);
    
    int j = 0;
    data.resize(number_buffer * buffer_size);  
    for (int i = 0; i < number_buffer; i++) {
        for (int m = 0; m < buffer_size; m++) {
            data[j].first = this->buffer_2[i][m];  
            data[j].second = this->buffer[i][m];   
            j++;
        }
    }
    
    for (int i = 0; i < data.size(); i++) {
        fwrite(&data[i].first, sizeof(short), 1, fp);  
        fwrite(&data[i].second, sizeof(short), 1, fp); 
    }

    fclose(fp);
}
