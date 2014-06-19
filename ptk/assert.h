#pragma once

void ptk_assert_failure(const char *msg, const char *file, int line);

#define PTK_ASSERT_FAILURE(msg,file,line)                       \
  ptk_assert_failure((msg), (file), (line))

#define PTK_ASSERT(cond,msg)                                    \
  do {                                                          \
    if (!(cond)) PTK_ASSERT_FAILURE(msg, __FILE__, __LINE__);   \
  } while (0)
