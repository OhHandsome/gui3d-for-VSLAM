#pragma once

#include "log.h"
#include <fstream>
#include <Eigen/Core>

#define CMV_MAX_BUF 2048

namespace io {

/**
 *  Scene Format is
 *       TypeName:
 *       Number
 *       Item
 *  example:
 *       Poses:
 *       4
 *       T1
 *       T2
 *       T3
 *       T4
 */

/*
 *   skip over useless string in line
 */
inline void nop(std::fstream& fp) {
  char buf[CMV_MAX_BUF];
  fp.getline(buf, CMV_MAX_BUF);
  LOGD("%s", buf);
}

/*
*   read one number in line
*/
template<typename T>
inline T readByte(std::fstream& fp) {
  char buf[CMV_MAX_BUF];
  double x;
  fp >> x;
  fp.getline(buf, '\n');
  LOGD("%lf", x);
  return T(x);
}

/*
*   read MxN number in line
*/
template<typename T, int rows, int cols>
inline Eigen::Matrix<T, rows, cols> readMxN(std::fstream& fp) {
  char buf[CMV_MAX_BUF];
  Eigen::Matrix<T, rows, cols> Dmxn;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      double x;
      fp >> x;
      Dmxn(i, j) = T(x);
      print("%lf ", x);
    }
    fp.getline(buf, '\n');
    print("\n");
  }
  return Dmxn;
}

/*
*   read MxN number in column
*/
template<typename T, int rows, int cols>
inline Eigen::Matrix<T, rows, cols> readMxNbyColumn(std::fstream& fp) {
  Eigen::Matrix<T, rows, cols> Dmxn;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Dmxn(i, j) = readByte<T>(fp);
    }
  }
  return Dmxn;
}

template<typename T, int rows, int cols>
inline void writeMxN(std::fstream& fp, const Eigen::Matrix<T, rows, cols>& Dmxn) {
  fp << Dmxn << std::endl;
}

template<typename T>
inline void writeItem(std::fstream& fp, const char* name, T count) {
  fp << name << std::endl
     << count << std::endl;
}

}

const std::vector<std::string> explode(const std::string& s, const char& c);
std::string format(const char* fmt, ...);
void removeAllFile(const std::string& dir);
