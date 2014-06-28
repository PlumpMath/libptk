#include <gtest/gtest.h>
#include "ptk/ilist.h"

using namespace ptk;

struct TestElement {
  i2link_t l;
  const int value;

  TestElement(int v) : value(v) {}
};

class I2ListTest : public ::testing::Test {
protected:
  TestElement e1;
  TestElement e2;
  TestElement e3;
  TestElement e4;
  TestElement e5;

  I2List<TestElement> list;

  int value(I2List<TestElement> &x) {
    int sum = 0;
    auto iter = x.iter();
    while (iter.more()) {
      sum = 10*sum + iter->value;
      iter.next();
    }

    return sum;
  }

  int value(I2List<TestElement>::Iterator &x) {
    int sum = 0;
    while (x.more()) {
      sum = 10*sum + x->value;
      x.next();
    }

    return sum;
  }

public:
  I2ListTest() :
    e1(1),
    e2(2),
    e3(3),
    e4(4),
    e5(5),
    list(&TestElement::l)
  {
  }
};

TEST_F(I2ListTest, TestEmpty) {
  EXPECT_TRUE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_EQ(e1.l.left, &e1.l);
  EXPECT_EQ(e1.l.right, &e1.l);
  EXPECT_EQ(value(list), 0);
}

TEST_F(I2ListTest, TestPush1f) {
  list.push(e1);

  EXPECT_EQ(value(list), 1);
  EXPECT_FALSE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_EQ(e1.l.left, &e1.l);
  EXPECT_EQ(e1.l.right, &e1.l);
}

TEST_F(I2ListTest, TestPush1b) {
  list.push_back(e1);

  EXPECT_EQ(value(list), 1);
  EXPECT_FALSE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_EQ(e1.l.left, &e1.l);
  EXPECT_EQ(e1.l.right, &e1.l);
}

TEST_F(I2ListTest, TestPush1f2f) {
  list.push(e1);
  list.push(e2);

  EXPECT_EQ(value(list), 21);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
}

TEST_F(I2ListTest, TestPush1b2f) {
  list.push_back(e1);
  list.push(e2);

  EXPECT_EQ(value(list), 21);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
}

TEST_F(I2ListTest, TestPush1b2b) {
  list.push_back(e1);
  list.push_back(e2);

  EXPECT_EQ(value(list), 12);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3f) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  EXPECT_EQ(value(list), 321);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_TRUE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2b3f) {
  list.push(e1);
  list.push_back(e2);
  list.push(e3);

  EXPECT_EQ(value(list), 312);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_TRUE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd1) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e1);

  EXPECT_EQ(value(list), 32);
  EXPECT_FALSE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_TRUE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd2) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e2);

  EXPECT_EQ(value(list), 31);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_TRUE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd3) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e3);

  EXPECT_EQ(value(list), 21);
  EXPECT_FALSE(list.empty());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd3d2) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e3);
  list.remove(e2);

  EXPECT_EQ(value(list), 1);
  EXPECT_FALSE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd3d2d1) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e3);
  list.remove(e2);
  list.remove(e1);

  EXPECT_EQ(value(list), 0);
  EXPECT_TRUE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd2d1) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e2);
  list.remove(e1);

  EXPECT_EQ(value(list), 3);
  EXPECT_FALSE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());
}

TEST_F(I2ListTest, TestPush1f2f3fd2d3) {
  list.push(e1);
  list.push(e2);
  list.push(e3);
  list.remove(e2);
  list.remove(e3);

  EXPECT_EQ(value(list), 1);
  EXPECT_FALSE(list.empty());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());
}

TEST_F(I2ListTest, Iter123) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();

  EXPECT_TRUE(iter.more());
  EXPECT_EQ(value(iter), 321);
}

TEST_F(I2ListTest, Iter123n) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  iter.next();

  EXPECT_TRUE(iter.more());
  EXPECT_EQ(value(iter), 21);
}

TEST_F(I2ListTest, Iter123nn) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.next();
  EXPECT_TRUE(iter.more());
  iter.next();

  EXPECT_TRUE(iter.more());
  EXPECT_EQ(value(iter), 1);
}

TEST_F(I2ListTest, Iter123nnn) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.next();
  EXPECT_TRUE(iter.more());
  iter.next();
  EXPECT_TRUE(iter.more());
  iter.next();

  EXPECT_FALSE(iter.more());
  EXPECT_EQ(value(iter), 0);
}

TEST_F(I2ListTest, Iter123r) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.remove();

  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());

  EXPECT_TRUE(iter.more());
  EXPECT_EQ(value(iter), 21);
}

TEST_F(I2ListTest, Iter123rr) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.remove();
  EXPECT_TRUE(iter.more());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());

  iter.remove();
  EXPECT_TRUE(iter.more());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());

  EXPECT_TRUE(iter.more());
  EXPECT_EQ(value(iter), 1);
}

TEST_F(I2ListTest, Iter123rrr) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.remove();
  EXPECT_TRUE(iter.more());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());

  iter.remove();
  EXPECT_TRUE(iter.more());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());

  iter.remove();
  EXPECT_FALSE(iter.more());
  EXPECT_FALSE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());

  EXPECT_EQ(value(iter), 0);
}

TEST_F(I2ListTest, Iter123nr) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.next();
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_TRUE(e3.l.is_joined());
  EXPECT_TRUE(iter.more());
  iter.remove();

  EXPECT_TRUE(iter.more());
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_FALSE(e2.l.is_joined());
  EXPECT_TRUE(e3.l.is_joined());

  EXPECT_EQ(value(iter), 1);
}

TEST_F(I2ListTest, Iter123rn) {
  list.push(e1);
  list.push(e2);
  list.push(e3);

  auto iter = list.iter();
  EXPECT_TRUE(iter.more());
  iter.remove();
  EXPECT_TRUE(e1.l.is_joined());
  EXPECT_TRUE(e2.l.is_joined());
  EXPECT_FALSE(e3.l.is_joined());
  EXPECT_TRUE(iter.more());
  iter.next();
  EXPECT_TRUE(iter.more());

  EXPECT_EQ(value(iter), 1);
}


