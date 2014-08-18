#include "ptk/shell.h"

#include <cctype>
#include <cstring>
#include <cstdio>

using namespace ptk;

ShellCommand::ShellCommand(const char *name) :
  SubThread(),
  name(name)
{
  next_command = Shell::commands;
  Shell::commands = this;
}

void ShellCommand::printf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  Shell::out->vprintf(fmt, args);
  va_end(args);
}

void ShellCommand::help(bool brief) {
  Shell::out->printf("%s ...\r\n", name);
}

DeviceInStream *Shell::in;
DeviceOutStream *Shell::out;
int Shell::argc;
const char *Shell::argv[MAX_ARGS];
ShellCommand *Shell::commands;

Shell::Shell(DeviceInStream &in, DeviceOutStream &out) :
  Thread(),
  line_length(0)
{
  Shell::in = &in;
  Shell::out = &out;
}

void Shell::run() {
  PTK_BEGIN();

  while (1) {
    // BUG: why doesn't the first prompt appear?
    out->puts("> ");

    line_length = 0;
    line_complete = false;

    while (!line_complete) {
      PTK_WAIT_EVENT(in->not_empty, TIME_INFINITE);

      uint8_t c;
      while (!line_complete && in->get(c)) {
        if (line_length == MAX_LINE-1) {
          line_complete = true;
          break;
        }

        switch (c) {
        case 0x04 : // ctrl-D
          out->puts("^D");
          line[line_length] = 0;
          line_complete = true;
          break;

        case 0x7f : // delete
        case 0x08 : // ctrl-H
          if (line_length > 0) {
            out->puts("\010 \010");
            line_length--;
          }
          break;

        case 0x15 : // ctrl-U
          out->puts("^U\r\n> ");
          line[line_length=0] = 0;
          break;

        case '\r' :
          out->puts("\r\n");
          line[line_length] = 0;
          line_complete = true;
          break;

        default :
          if (c < ' ' || c >= 0x80) {
            continue; // unprintable
          }

          if (line_length >= MAX_LINE-1) continue;
          out->put(c);
          line[line_length++] = c;
          break;
        }
      }

      if (line_complete) {
        parse_line();

        if (argc > 0) {
          ShellCommand *cmd = find_command(argv[0]);

          if (cmd) {
            PTK_WAIT_SUBTHREAD(*cmd, TIME_INFINITE);
          } else {
            out->printf("%s ?\r\n", argv[0]);
          }
        }
      }
    } // while (!line_complete)
  } // while (1)

  PTK_END();
}

void Shell::parse_line() {
  unsigned int i=0;
  argc = 0;

  while (i < line_length && argc < MAX_ARGS) {
    // start argument word
    argv[argc++] = &line[i];
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
    out->printf("  %-8s  -- %s\r\n", list[i].name, list[i].description);
  }
}

ShellCommand *Shell::find_command(const char *name) {
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
    Shell::out->printf("%-10s - %s\r\n", name, "list available commands");
  }

  virtual void run() {
    PTK_BEGIN();
    for (cmd = Shell::commands; cmd; cmd = cmd->next_command) {
      // wait a bit until there's (hopefully) room in the output buffer
      PTK_WAIT_UNTIL(Shell::out->available() > 16, 100);
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
    Shell::out->printf("%-10s - %s\r\n", name, "show active protothreads");
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
        PTK_WAIT_UNTIL(Shell::out->available() > 64, 100);
        Shell::out->printf("[%08x] %6s %s:%d\r\n",
                           thread,
                           thread->state_name(),
                           thread->debug_file,
                           thread->debug_line);
      }
    }
    PTK_END();
  }

} threads_command;
