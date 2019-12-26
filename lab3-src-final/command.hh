#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"

// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  int numsimplecommands = 0;
  int _outFlag = 0;
  int _inFlag = 0;
  int _append = 0;
  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();
  int builtInFunction(int i);
  static SimpleCommand *_currentSimpleCommand;
};

#endif
