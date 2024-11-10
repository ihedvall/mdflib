/*
* Copyright 2021 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <xtensor/xarray.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xview.hpp>

namespace mdf::test {

TEST(XTensor, FirstExample) {
  xt::xarray<double> arr1
      {{1.0, 2.0, 3.0},
       {2.0, 5.0, 7.0},
       {2.0, 5.0, 7.0}};

  xt::xarray<double> arr2
      {5.0, 6.0, 7.0};

  xt::xarray<double> res = xt::view(arr1, 1.0) + arr2;

  std::cout << res << std::endl;

}

TEST(XTensor, SecondExample) {
  std::vector<size_t> shape1 = {2,3};
  xt::xarray<uint64_t> arr(shape1);
  std::cout << "Shape1: " << std::endl << arr << std::endl << std::endl;

  std::vector<size_t> shape2 = {3,3};
  arr.resize(shape2);
  std::cout << "Shape2: " << std::endl << arr << std::endl << std::endl;

  std::vector<size_t> shape1D = {9};
  xt::xarray<double> values = {1.0, 2.0, 3.0,4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  std::cout << "Value 1D: " << std::endl << values << std::endl << std::endl;

  std::vector<size_t> shape2D = {3,3};
  values.reshape(shape2D );
  std::cout << "Value 2D: " << std::endl << values << std::endl << std::endl;

  const auto values1 = values;
  std::cout << "Copy Value 2D: " << std::endl << values << std::endl << std::endl;

  const auto values2 = xt::sum(values1);
  std::cout << "Sum Value 2D: " << std::endl << values2 << std::endl << std::endl;

  const auto values3 = xt::mean(values1);
  std::cout << "Means Value 2D: " << std::endl << values3 << std::endl << std::endl;

  const auto values4 =  2 * values1;
  std::cout << "Mul Value 2D: " << std::endl << values4 << std::endl << std::endl;
}

} // end mdf::test