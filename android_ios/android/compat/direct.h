#pragma once
#include <unistd.h>
#include <sys/stat.h>
#define _mkdir(dir) mkdir(dir, 0755)
#define _chdir(dir) chdir(dir)
#define _getcwd(buf, size) getcwd(buf, size)
