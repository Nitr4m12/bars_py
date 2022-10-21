#include "bars/bars.h"

int main(int argc, char **argv)
{
  // 1. Load parser
  NSound::Parser parser{argv[1]};

  // 2. Load header
  parser.load();
}