#ifndef __PARSESCRIPT_HPP
#define __PARSESCRIPT_HPP

#include "x_files.hpp"
#include "parsing\tokenizer.hpp"

#define PACKAGE_SECTION    "package:"
#define FILES_SECTION      "files:"
#define DESCRIPTOR_SECTION "descriptors:"
#define MUSIC_SECTION      "music:"
#define OUTPUT_SECTION     "output:"


xbool FindPackageKeyWord( token_stream* Tokenizer );
xbool ParsePackage      ( token_stream* Tokenizer );
xbool ParseFiles        ( token_stream* Tokenizer );
xbool ParseDescriptors  ( token_stream* Tokenizer );
xbool ParseMusic        ( token_stream* Tokenizer );
xbool ParseOutput       ( token_stream* Tokenizer );
xbool ResolveReferences ( void );

#endif
