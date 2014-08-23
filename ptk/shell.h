// -*- Mode:C++ -*-

#pragma once

#include "ptk/ilist.h"
#include "ptk/io.h"

namespace ptk {
  class Shell;

  /**
   * @class ShellCommand
   * @brief Parses lines read from a serial console and executes commands
   *
   * This class implements a simple command line shell. Input lines are parsed
   * into words separated by whitespace. The first word on each line names the
   * command to be executed.
   *
   * Each command is represented by a single instance of a ShellCommand
   * subclass, usually declared statically. ShellCommand's constructor
   * automatically adds each instance to a global list of known commands.
   */
  class ShellCommand : public SubThread {
    friend class Shell;

  protected:
    static int argc;
    static const char *argv[];
    static DeviceInStream *in;
    static DeviceOutStream *out;
    static ShellCommand *commands;

    const char *name;

    void printf(const char *fmt, ...);

  public:
    /**
     * @brief Constructs a ShellCommand instance
     * @param[in] name null terminated name
     */
    ShellCommand(const char *name);

    /**
     * @brief protothread run() method to execute the command
     * Arguments can be found in argc and argv.
     */
    virtual void run() = 0;

    /**
     * @brief prints a help message for this command
     * @param[in] brief
     */
    virtual void help(bool brief = false);

    /**
     * @brief Utility function for parsing parsing shell arguments
     */
    template<class T>
    bool parse_number(const char *str, T &out);
    bool parse_bool(const char *str, bool &out);

    /**
     * @brief Tries to locate a Command instance with the specified name
     * @param[in] name null terminated name
     * @returns a pointer to the command with a matching name
     * @returns 0 otherwise
     */
    static ShellCommand *find_command(const char *name);
    ShellCommand *next_command;
  };

  class Shell : public Thread {
    friend class ShellCommand;

  public:
    /**
     * @brief represents a keyword that can be recognized
     */
    struct keyword_t {
      const char *name;                 /// name to be searched for
      const char *description;          /// help string (optional)
      int id;                           /// identifier
    };

    /**
     * @brief searches for a keyword in a list of known words
     * @param[in] str the string being searched for
     * @param[in] list[] an array of possible keywords
     * @param[in] size is the size of the list[] array in bytes
     *            (i.e., not the number of elements in the array)
     *
     * @returns the id of the matching keyword, and -1 if there is no match
     */
    static int lookup_keyword(const char *str, const keyword_t list[], size_t size);

    /**
     * @brief prints a list of keywords and descriptions on the tty
     * @param[in] list[] an array of keywords
     * @param[in] size is the size of the list[] array in bytes
     *            (i.e., not the number of elements in the array)
     */
    static void print_keywords(const keyword_t list[], size_t size);

    Shell(DeviceInStream &in, DeviceOutStream &out);
    virtual void run();

  protected:
    enum {
      MAX_LINE = 100,
      MAX_ARGS = 10
    };

    char line[MAX_LINE];
    unsigned line_length;
    bool line_complete;

    void parse_line();
  };

};
