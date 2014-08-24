#include "ptk/shell.h"

#include <cctype>
#include <cstring>
#include <cstdio>

using namespace ptk;

ShellCommand::ShellCommand(const char *name) :
  SubThread(),
  name(name)
{
  next_command = ShellCommand::commands;
  ShellCommand::commands = this;
}

void ShellCommand::printf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  out->vprintf(fmt, args);
  va_end(args);
}

void ShellCommand::help(bool brief) {
  out->printf("%s ...\r\n", name);
}

template<class T>
bool ShellCommand::parse_number(const char *str, T &out) {
  unsigned int hex;
  int dec, count;

  if (str[0] == '0' && str[1] == 'x') {
    if ((count = sscanf(str + 2, "%x", &hex))) {
      out = hex;
      return true;
    }
  }

  if ((count = sscanf(str, "%d", &dec))) {
    out = dec;
    return true;
  }

  return false;
}

template bool ShellCommand::parse_number(const char *str, uint16_t &out);
template bool ShellCommand::parse_number(const char *str, int16_t &out);
template bool ShellCommand::parse_number(const char *str, uint8_t &out);
template bool ShellCommand::parse_number(const char *str, int &out);

bool ShellCommand::parse_bool(const char *str, bool &out) {
  static const Shell::keyword_t booleans[] = {
    { "true",  0, 1 },
    { "on",    0, 1 },
    { "yes",   0, 1 },
    { "1",     0, 1 },
    { "high",  0, 1 },

    { "false", 0, 0 },
    { "off",   0, 0 },
    { "no",    0, 0 },
    { "0",     0, 0 },
    { "low",   0, 0 },
  };

  int value;

  if ((value = Shell::lookup_keyword(str, booleans, sizeof booleans)) >= 0) {
    out = (bool) value;
    return true;
  } else {
    return false;
  }
}

DeviceInStream *ShellCommand::in;
DeviceOutStream *ShellCommand::out;
int ShellCommand::argc;
const char *ShellCommand::argv[Shell::MAX_ARGS];
ShellCommand *ShellCommand::commands;

Shell::Shell(DeviceInStream &in, DeviceOutStream &out) :
  Thread(),
  line_length(0)
{
  ShellCommand::in = &in;
  ShellCommand::out = &out;
}

void Shell::run() {
  PTK_BEGIN();

  while (1) {
    // BUG: why doesn't the first prompt appear?
    ShellCommand::out->puts("> ");

    line_length = 0;
    line_complete = false;

    while (!line_complete) {
      PTK_WAIT_EVENT(ShellCommand::in->not_empty, TIME_INFINITE);

      uint8_t c;
      while (!line_complete && ShellCommand::in->get(c)) {
        if (line_length == MAX_LINE-1) {
          line_complete = true;
          break;
        }

        switch (c) {
        case 0x04 : // ctrl-D
          ShellCommand::out->puts("^D");
          line[line_length] = 0;
          line_complete = true;
          break;

        case 0x7f : // delete
        case 0x08 : // ctrl-H
          if (line_length > 0) {
            ShellCommand::out->puts("\010 \010");
            line_length--;
          }
          break;

        case 0x15 : // ctrl-U
          ShellCommand::out->puts("^U\r\n> ");
          line[line_length=0] = 0;
          break;

        case '\r' :
          ShellCommand::out->puts("\r\n");
          line[line_length] = 0;
          line_complete = true;
          break;

        default :
          if (c < ' ' || c >= 0x80) {
            continue; // unprintable
          }

          if (line_length >= MAX_LINE-1) continue;
          ShellCommand::out->put(c);
          line[line_length++] = c;
          break;
        }
      }

      if (line_complete) {
        parse_line();

        if (ShellCommand::argc > 0) {
          ShellCommand *cmd = ShellCommand::find_command(ShellCommand::argv[0]);

          if (cmd) {
            PTK_WAIT_SUBTHREAD(*cmd, TIME_INFINITE);
          } else {
            ShellCommand::out->printf("%s ?\r\n", ShellCommand::argv[0]);
          }
        }
      }
    } // while (!line_complete)
  } // while (1)

  PTK_END();
}

void Shell::parse_line() {
  unsigned int i=0;
  ShellCommand::argc = 0;

  while (i < line_length && ShellCommand::argc < MAX_ARGS) {
    // start argument word
    ShellCommand::argv[ShellCommand::argc++] = &line[i];
    while (i < line_length && !std::isspace(line[i])) i++;
    line[i++] = 0;

    // skip whitespace
    while (i < line_length && std::isspace(line[i])) i++;
  }
}

int Shell::lookup_keyword(const char *str,
                          const keyword_t list[],
                          size_t size)
{
  size /= sizeof(keyword_t);

  for (unsigned i=0; i < size; ++i) {
    if (!strcmp(str, list[i].name)) return list[i].id;
  }

  return -1;
}

void Shell::print_keywords(const keyword_t list[], size_t size) {
  size /= sizeof(keyword_t);

  for (unsigned i=0; i < size; ++i) {
    ShellCommand::out->printf("  %-8s  -- %s\r\n", list[i].name,
                              list[i].description);
  }
}

ShellCommand *ShellCommand::find_command(const char *name) {
  if (!strcmp(name, "?")) name = "help";

  for (ShellCommand *cmd = commands; cmd; cmd = cmd->next_command) {
    if (!strcmp(name, cmd->name)) return cmd;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

class HelpCommand : public ShellCommand {
  ShellCommand *cmd;

public:
  HelpCommand() : ShellCommand("help") {
  }

  virtual void help(bool brief) {
    printf("%-10s - %s\r\n", name, "list available commands");
  }

  virtual void run() {
    PTK_BEGIN();
    for (cmd = commands; cmd; cmd = cmd->next_command) {
      // wait a bit until there's (hopefully) room in the output buffer
      PTK_WAIT_UNTIL(ShellCommand::out->available() > 16, 10);
      cmd->help(true);
    }
    PTK_END();
  }
} help_command;

class ThreadsCommand : public ShellCommand {
  Thread *thread;

public:
  ThreadsCommand() : ShellCommand("threads") {
  }

  virtual void help(bool brief) {
    printf("%-10s - %s\r\n", name, "show active protothreads");
  }

public:
  virtual void run() {
    PTK_BEGIN();
    for (thread = all_registered_threads;
         thread;
         thread = thread->next_registered_thread)
    {
      if (thread->continuation) {
        // wait a bit until there's (hopefully) room in the output buffer
        PTK_WAIT_UNTIL(ShellCommand::out->available() > 64, 10);
        printf("[%08x] %6s %s:%d\r\n",
               thread,
               thread->state_name(),
               thread->debug_file,
               thread->debug_line);
      }
    }
    PTK_END();
  }

} threads_command;
