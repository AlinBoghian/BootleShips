#ifndef UTILS_H
#define UTILS_H

#define PrintLn(format, ...) Print(L"" format L"\r\n" __VA_OPT__(,) ##__VA_ARGS__)

#endif