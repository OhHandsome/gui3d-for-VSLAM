#pragma once

#include <Eigen/Core>
#include <chrono>
#include <random>

namespace gui3d {
// random seed
template<typename _Scalar, int m>
using Vec = Eigen::Matrix<_Scalar, m, 1>;

namespace system {

// normal distribution: [0,1]
template<typename _Scalar, int m>
Vec<_Scalar, m> randVec(std::mt19937 &generator) {
  std::normal_distribution<_Scalar> dis(0, 1);
  Vec<_Scalar, m> v;
  for (int i = 0; i < m; i++)
    v[i] = dis(generator);

  return v;
}

// uniform distribution: [0,1]
template<typename _Scalar, int m>
Vec<_Scalar, m> randUVec(std::mt19937 &generator) {
  std::uniform_real_distribution<_Scalar> dis(0, 1);
  Vec<_Scalar, m> v;
  for (int i = 0; i < m; i++)
    v[i] = dis(generator);
  return v;
}

// normal distribution: [0,1]
template<typename _Scalar, int m>
Vec<_Scalar, m> randVec() {
  const auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator((unsigned int) seed);
  return randVec<_Scalar, m>(generator);
}

// uniform distribution: [0,1]
template<typename _Scalar, int m>
Vec<_Scalar, m> randUVec() {
  const auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator((unsigned int) seed);
  return randUVec<_Scalar, m>(generator);
}

template<typename T>
std::vector<T> range(size_t n, int step = 1) {
  std::vector<T> array;
  array.reserve(n);
  for (int i = 0; i < n; i = i + step)
    array.push_back(T(i));
  return array;
}

template<typename T>
std::vector<T> line(T x1, T x2, T step) {
  std::vector<T> array;
  for (T i = x1; i < x2; i = i + step)
    array.push_back(i);
  return array;
}

} // namespace system
} // namespace gui3d
