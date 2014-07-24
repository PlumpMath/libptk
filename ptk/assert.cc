extern "C" {
  void ptk_halt(const char *msg) {
    while (1);
  }

  void ptk_assert_failure(const char *msg, const char *file, int line) __attribute__ ((weak, alias("ptk_halt")));
}
