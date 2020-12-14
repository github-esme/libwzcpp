#include <iostream>
#include "nx/wz2nx_serializer.h"

using namespace wz::nx;

int main(int argc, char* argv[])
{
    WZ2NXSerializer serializer;
    serializer.Parse(argv[1], argv[2]);
    return 0;
}