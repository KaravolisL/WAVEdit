// Luke Karavolis (ljk55)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct WAVHeader {
      char riffId[4];
      uint32_t fileSize;
      char waveId[4];
      char fmtId[4];
      uint32_t fmtSize;
      uint16_t dataFormat;
      uint16_t numberOfChannels;
      uint32_t samplesPerSecond;
      uint32_t bytesPerSecond;
      uint16_t blockAlignment;
      uint16_t bitsPerSample;
      char dataId[4];
      uint32_t dataSize;
} WAVHeader;

/* Function printHelp
* ----------------------
* prints info regarding program when it's run without arguments
*/
void printHelp() {
      printf("Usage: wavedit [FILE] [OPTION]...\n");
      printf("Read WAV file format\n\n");
      printf("  -rate [INT] \t\t plays file at given rate between 1 and 192000\n");
      printf("  -reverse    \t\t plays the file in reverse\n\n\n");
      printf("With no flag, the format information of the file will be displayed\n");
      printf("With no FILE, this menu is displayed and the program exits\n");
      return;
}

/* Function: invalidFail
* --------------------
* prints statement and exits program
*/
void invalidFail() {
      printf("Invalid WAV file\n");
      exit(EXIT_FAILURE);
      return;
}

/* Function: fileCheck
* ------------------------
* Takes a string and checks if it can open a valid binary file
*
* @param fileName string containing the name of a binary file
* @return true if the file exists, false if it doesn't exist
*/
FILE* fileCheck(char* fileName) {
      FILE* file = fopen(fileName, "rb");
      if (file == NULL) {
            invalidFail();
      }
      return file;
}

 /* Function: readFile
 * -----------------------
 * Reads a file and puts the info into a WAVHeader struct
 *
 * @param file file to be read
 * @param header pointer to a WAVHeader struct
 */
void readFile(FILE* file, WAVHeader* header) {
      fread(header, sizeof(*header), 1, file);
      fclose(file);
}

/* Function printInfo
* ------------------------
* Prints several values stored in the header struct
*
* @param header struct that hold the information to be printed
*/
void printInfo(WAVHeader* header) {
      printf("This is a %u-bit %uHz ", header->bitsPerSample, header->samplesPerSecond);
      if (header->numberOfChannels == 1) {
            printf("mono sound.\n");
      } else {
            printf("stereo sound.\n");
      }
      uint32_t lengthInSamples = header->dataSize/header->blockAlignment;
      float lengthInSeconds = (float)lengthInSamples/header->samplesPerSecond;
      printf("It is %u samples (%.3f seconds) long.\n", lengthInSamples, lengthInSeconds);
      return;
}

/* Function: isWAV
* --------------------
* Checks each field in the header for correct format. Exits program if incorrect
*
* @param header a pointer to the header of the file
*/
void isWAV(WAVHeader* header) {
      if (strncmp(header->riffId, "RIFF", 4) != 0) invalidFail();
      if (strncmp(header->waveId, "WAVE", 4) != 0) invalidFail();
      if (strncmp(header->fmtId, "fmt ", 4) != 0) invalidFail();
      if (strncmp(header->dataId, "data", 4) != 0) invalidFail();
      if (header->fmtSize != 16) invalidFail();
      if (header->dataFormat != 1) invalidFail();
      if (header->numberOfChannels != 1 && header->numberOfChannels != 2) invalidFail();
      if (header->samplesPerSecond < 1 || header->samplesPerSecond > 192000) invalidFail();
      if (header->bitsPerSample != 8 && header->bitsPerSample != 16) invalidFail();
      unsigned int number = header->samplesPerSecond * (header->bitsPerSample / 8) * header->numberOfChannels;
      if (header->bytesPerSecond != number) invalidFail();
      if (header->blockAlignment != (header->bitsPerSample / 8 * header->numberOfChannels)) invalidFail();
      return;
}

/* Function checkRate
* -----------------------
* Checks to make sure the rate argument is appropriate
*
* @param newRate rate argument passed in by user
*/
void checkRate(uint32_t newRate) {
      if (newRate < 1 || newRate > 192000) {
            printf("Rate must be between 1 and 192000\n");
            exit(EXIT_FAILURE);
      }
      return;
}

/* Function writeBack
* ----------------------
* Writes the modified struct back to the file
*
* @param fileName file to write the struct back to
* @param header modified WAVHeader struct
*/
void writeBack(char* fileName, WAVHeader* header) {
      FILE* file = fopen(fileName, "rb+");
      fseek(file, 0, SEEK_SET);
      fwrite(header, sizeof(*header), 1, file);
      fclose(file);
}

/* Function reverse1, reverse2, reverse3
* ------------------------------------------
* Reads the files data into an appropriate array, reverses it, then writes
* it back to the file. The three versions are for the three combinations of
* number of channels and bits per sample
*
* @param fileName file to read and write from
* @param lengthInSamples number of samples in the file
*/
void reverse1(char* fileName, uint32_t lengthInSamples) {
      // 8-bit mono
      FILE* file = fopen(fileName, "rb+");
      int8_t dataArray[lengthInSamples];
      fseek(file, sizeof(WAVHeader), SEEK_SET);
      fread(dataArray, sizeof(int8_t), lengthInSamples, file);
      int8_t reversedArray[lengthInSamples];
      for (int i = lengthInSamples - 1, j = 0; i >= 0; i--, j++) {
            reversedArray[j] = dataArray[i];
      }
      fseek(file, sizeof(WAVHeader), SEEK_SET);
      fwrite(reversedArray, sizeof(int8_t), sizeof(reversedArray), file);
      fclose(file);
}

// See description above
void reverse2(char* fileName, uint32_t lengthInSamples) {
      // 16-bit stereo
      FILE* file = fopen(fileName, "rb+");
      int32_t dataArray[lengthInSamples];
      fseek(file, sizeof(WAVHeader), SEEK_SET);
      fread(dataArray, sizeof(int32_t), lengthInSamples, file);
      int32_t reversedArray[lengthInSamples];
      for (int i = lengthInSamples - 1, j = 0; i >= 0; i--, j++) {
            reversedArray[j] = dataArray[i];
      }
      fseek(file, sizeof(WAVHeader), SEEK_SET);
      fwrite(reversedArray, sizeof(int32_t), sizeof(reversedArray), file);
      fclose(file);
}

// See description above
void reverse3(char* fileName, uint32_t lengthInSamples) {
      // 8-bit stereo or 16-bit mono
      FILE* file = fopen(fileName, "rb+");
      int16_t dataArray[lengthInSamples];
      fseek(file, sizeof(WAVHeader), SEEK_SET);
      fread(dataArray, sizeof(int16_t), lengthInSamples, file);
      int16_t reversedArray[lengthInSamples];
      for (int i = lengthInSamples - 1, j = 0; i >= 0; i--, j++) {
            reversedArray[j] = dataArray[i];
      }
      fseek(file, sizeof(WAVHeader), SEEK_SET);
      fwrite(reversedArray, sizeof(int16_t), sizeof(reversedArray), file);
      fclose(file);
}

int main(int argc, char** argv) {
      // Prints the information when no arguments are passed
      if (argc == 1) {
            printHelp();
            return 0;
      }
      // Check that file exists
      char* fileName = argv[1];
      FILE* file = fileCheck(fileName);
      // Creating struct and filling it from file
      WAVHeader header;
      readFile(file, &header);
      // Checking if it's a valid file
      isWAV(&header);
      // Performing actions based on command-line arguments
      if (argc == 2) {
            printInfo(&header);
      } else if (argc == 4 && strcmp(argv[2], " -rate")) {
            // Checking the argument and writing it back to the file
            uint32_t newRate = atoi(argv[3]);
            checkRate(newRate);
            header.samplesPerSecond = newRate;
            header.bytesPerSecond = newRate * header.bitsPerSample / 8 * header.numberOfChannels;
            writeBack(fileName, &header);
      } else if (argc == 3 && strcmp(argv[2], " -reverse")) {
            uint32_t lengthInSamples = header.dataSize/header.blockAlignment;
            if (header.numberOfChannels == 1 && header.bitsPerSample == 8) {
                  // 8-bit mono
                  reverse1(fileName, lengthInSamples);
            } else if (header.numberOfChannels == 2 && header.bitsPerSample == 16) {
                  // 16-bit stereo
                  reverse2(fileName, lengthInSamples);
            } else {
                  // 8-bit stereo or 16-bit mono
                  reverse3(fileName, lengthInSamples);
            }
      } else {
            printf("Arguments not found");
      }
      return 0;
}
