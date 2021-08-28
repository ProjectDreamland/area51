#ifndef __PACKAGE_HPP
#define __PACKAGE_HPP
#include "parse.hpp"

void    package_Write(parse_output &Output,xbool verbose_flag);
void    package_cfx(const complex_effect &Element,cfx_stored_element &Stored);

#endif