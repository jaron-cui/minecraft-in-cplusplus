#include "Image.hpp"
#include <fstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <memory>
#include <vector>

// a string holding all characters recognized as whitespace
const std::string WHITESPACE = " \n\t\r";

// given a string constant and mutable vector reference,
// parses tokens divided by whitespace and adds them to the vector
void splitWhitespace(const std::string& s, std::vector<std::string>& results) {
  int lastWhitespace = -1;
  while (true) {
    int nextWhitespace = s.find_first_of(WHITESPACE, lastWhitespace + 1);
    // if there is no more whitespace left
    if (nextWhitespace == -1) {
      // parse the last token if there is one before exiting
      if (s.find_first_not_of(WHITESPACE, lastWhitespace + 1) != -1) {
        results.push_back(s.substr(lastWhitespace + 1));
      }
      break;
    }
    // if there is a non-whitespace substring, add it to the vector
    if (nextWhitespace > lastWhitespace + 1) {
      results.push_back(s.substr(lastWhitespace + 1, nextWhitespace - lastWhitespace - 1));
    }
    lastWhitespace = nextWhitespace;
  }
}

// extracts the next non-comment line from the input stream and places it into a buffer
// returns a boolean indicating whether another non-comment line was found
bool nextLine(std::ifstream& in, std::string& line) {
  while (getline(in, line)) {
    if (line.find_first_not_of(WHITESPACE) == -1 || line.at(0) == '#') {
      continue;
    }
    return true;
  }
  return false;
}

// extracts the next string token from an input stream using line-by-line buffering and places it in a string buffer
// returns a boolean indicating whether another token was successfully extracted
bool nextToken(std::ifstream& in, std::vector<std::string>& buffer, size_t& bufferIndex, std::string& token) {
  if (bufferIndex >= buffer.size()) {
    // put the next line of tokens into the buffer
    buffer.clear();
    std::string line;
    if (!nextLine(in, line)) {
      return false;
    }
    splitWhitespace(line, buffer);
    bufferIndex = 0;
  }
  token = buffer[bufferIndex];
  bufferIndex += 1;
  return true;
}

// Constructor
Image::Image(std::string filepath) : m_filepath(filepath){
    
}

// Destructor
Image::~Image (){
    // Delete our pixel data.	
    // Note: We could actually do this sooner
    // in our rendering process.
    if(m_pixelData!=NULL){
        delete[] m_pixelData;
    }
}

// Little function for loading the pixel data
// from a PPM image.
// TODO: Expects a very specific version of PPM!
//
// flip - Will flip the pixels upside down in the data
//        If you use this be consistent.
void Image::LoadPPM(bool flip){
// open file
  std::ifstream in;
  in.open(m_filepath);
  if (!in.is_open()) {
    throw std::invalid_argument("File not found.");
  }
  
  // variable holding string tokens during processing
  std::string token;
  // buffer for string tokens to be processed
  std::vector<std::string> buffer;
  // index management for buffer
  size_t bufferIndex = 1;

  if (!nextToken(in, buffer, bufferIndex, token)) {
    throw std::invalid_argument("Invalid PPM file - missing file type.");
  }
  if (token != "P6") {
    throw std::invalid_argument("Invalid PPM file - incorrect file type: " + token);
  }
  // width
  if (!nextToken(in, buffer, bufferIndex, token)) {
    throw std::invalid_argument("Invalid PPM file - width");
  }
  m_width = stoi(token);
  // height
  if (!nextToken(in, buffer, bufferIndex, token)) {
    throw std::invalid_argument("Invalid PPM file - height");
  }
  m_height = stoi(token);
  // max value
  if (!nextToken(in, buffer, bufferIndex, token)) {
    throw std::invalid_argument("Invalid PPM file - max value");
  }
  int maxValue = stoi(token);
  if (maxValue < 0 || maxValue > 255) {
    throw std::invalid_argument("Invalid PPM file - max value.");
  }

  // pixel data
  m_pixelData = new uint8_t[m_height * m_width * 3];
  if (!getline(in, token)) {
    throw std::invalid_argument("Invalid PPM file - missing color tokens.");
  }
  strcpy((char*)m_pixelData, token.data());
  // for (int y = 0; y < m_height; y += 1) {
  //   if (!getline(in, token)) {
  //     std::cout << y << " "  << token.length() << std::endl;
  //     throw std::invalid_argument("Invalid PPM file - missing color tokens.");
  //   }
  //   strcpy((char*)m_pixelData + m_width * sizeof(char), token.data());
  // }
  in.close();
}

/*  ===============================================
Desc: Sets a pixel in our array a specific color
Precondition: 
Post-condition:
=============================================== */ 
void Image::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b){
  if(x > m_width || y > m_height){
    return;
  }
  else{
    /*std::cout << "modifying pixel at " 
              << x << "," << y << "from (" <<
              (int)color[x*y] << "," << (int)color[x*y+1] << "," <<
(int)color[x*y+2] << ")";*/
    m_pixelData[(x*3)+m_height*(y*3)] 	= r;
    m_pixelData[(x*3)+m_height*(y*3)+1] = g;
    m_pixelData[(x*3)+m_height*(y*3)+2] = b;
/*    std::cout << " to (" << (int)color[x*y] << "," << (int)color[x*y+1] << ","
<< (int)color[x*y+2] << ")" << std::endl;*/
  }
}

/*  ===============================================
Desc: 
Precondition: 
Post-condition:
=============================================== */ 
void Image::PrintPixels(){
    for(int x = 0; x <  m_width*m_height*3; ++x){
        std::cout << " " << (int)m_pixelData[x];
    }
    std::cout << "\n";
}

/*  ===============================================
Desc: Returns pixel data for our image
Precondition: 
Post-condition:
=============================================== */ 
uint8_t* Image::GetPixelDataPtr(){
    return m_pixelData;
}