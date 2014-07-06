#include <gtest/gtest.h>
#include "ptk/fifo.h"

using namespace ptk;

class FIFOTest : public ::testing::Test {
protected:
  enum {FIFO_SIZE = 5};
  StaticFIFO<char, FIFO_SIZE> fifo;

public:
  FIFOTest() : fifo() { }
};

TEST_F(FIFOTest, TestConstructsAsEmpty) {
  EXPECT_EQ(fifo.read_capacity(), 0);
  EXPECT_EQ(fifo.write_capacity(), FIFO_SIZE);
}

TEST_F(FIFOTest, TestFullWrite) {
  char data[] = {'1', '2', '3'};

  size_t empty = fifo.write_capacity();
  size_t written = fifo.write(data, sizeof(data));

  EXPECT_EQ(written, sizeof(data));
  EXPECT_TRUE(fifo.write_capacity() > 0);
  EXPECT_EQ(fifo.read_capacity(), written);

  char ch;
  int i;
  for (i=0; i < written; ++i) {
    EXPECT_EQ(fifo.read(&ch, 1), 1);
    EXPECT_EQ(data[i], ch);
  }

  EXPECT_EQ(fifo.read(&ch, 1), 0);
  EXPECT_EQ(fifo.read_capacity(), 0);
  EXPECT_EQ(fifo.write_capacity(), empty);
}

TEST_F(FIFOTest, TestPartialWrite) {
  const char data[] = {'1', '2', '3', '4', '5', '6'};
  size_t empty = fifo.write_capacity();
  size_t written = fifo.write(data, sizeof(data));

  EXPECT_TRUE(written < sizeof(data));
  EXPECT_EQ(fifo.write_capacity(), 0);
  EXPECT_EQ(fifo.read_capacity(), written);

  char ch;
  int i;
  for (i=0; i < (written-1); ++i) {
    EXPECT_EQ(fifo.read(&ch, 1), 1);
    EXPECT_EQ(data[i], ch);
  }

  EXPECT_EQ(fifo.read_capacity(), 1);
  EXPECT_TRUE(fifo.write_capacity() > 0);

  EXPECT_EQ(fifo.read(&ch, 1), 1);
  EXPECT_EQ(data[i], ch);
  EXPECT_EQ(fifo.write_capacity(), empty);
}

TEST_F(FIFOTest, TestNonEmptyInterleavedReadWrite) {
  char data[3];
  char ch = 0;

  data[0] = ch++;
  data[1] = ch++;
  data[2] = ch++;

  size_t written = fifo.write(data, sizeof(data));
  
  EXPECT_EQ(written, sizeof(data));
  EXPECT_TRUE(fifo.write_capacity() > 0);
  EXPECT_EQ(fifo.read_capacity(), written);

  int i;
  for (i=0; i < 100; ++i) {
    char old_ch;
    EXPECT_EQ(fifo.read(&old_ch, 1), 1);
    EXPECT_EQ(old_ch, (char) (ch - 3));
    EXPECT_EQ(fifo.write(&ch, 1), 1);
    ch++;
  }
}
