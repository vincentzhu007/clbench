//
// Created by zgd on 1/3/23.
//

#include <iostream>
#include "gtest/gtest.h"

using namespace std;

class HelloGTest : public ::testing::Test {
 public:
  virtual void SetUp() override {
    cout << "set up test case" << endl;
  }
  virtual void TearDown() override {
    cout << "tear down test case" << endl;
  }
};

TEST_F(HelloGTest, Hello) {
  ASSERT_EQ(2 + 5, 7);
}
