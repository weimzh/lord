//
// Copyright (c) 2009 Wei Mingzhi <whistler@openoffice.org>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see http://www.gnu.org/licenses.
//

#ifndef HZKFONT_H
#define HZKFONT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLYPH_BUF_SIZE                           32
#define GB_SECTION_GLYPH_BUF_SIZE                (GLYPH_BUF_SIZE * 94)
#define GB_FONT_BUF_SIZE                         (GB_SECTION_GLYPH_BUF_SIZE * 94)

class CHZKFont
{
public:
   CHZKFont(const char *szFileNameHZK16);
   ~CHZKFont();

   unsigned char        *ReadGBGlyph(unsigned char gb_hz[2]); // 16x16
   unsigned char        *ReadASCGlyph(unsigned char c); // 8x16

private:
   void                  LoadHZK16(const char *szFileName);

   unsigned char        *m_ucHZK16Data;
};

#endif
