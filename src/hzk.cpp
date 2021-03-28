//
// Copyright (c) 2009 Wei Mingzhi <whistler_wmz@users.sf.net>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License,
// version 3, as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see http://www.gnu.org/licenses.
//

/*
 * Portions based on:
 *
 * YH - Console Chinese Environment -
 * Copyright (C) 1999 Red Flag Linux (office@sonata.iscas.ac.cn)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY RED FLAG LINUX ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TERRENCE R. LAMBERT BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "main.h"
#include "iso_font.h"

inline bool is_gb(unsigned char b1, unsigned char b2)
{
   if (b1 < 0xa1 || b1 > 0xfe)
      return false;
   if (b2 < 0xa1 || b2 > 0xfe)
      return false;
   return true;
}

CHZKFont::CHZKFont(const char *szFileNameHZK16):
m_ucHZK16Data(NULL)
{
   LoadHZK16(szFileNameHZK16);
}

CHZKFont::~CHZKFont()
{
   if (m_ucHZK16Data != NULL) {
      free(m_ucHZK16Data);
   }
}

void CHZKFont::LoadHZK16(const char *szFileName)
{
   if (m_ucHZK16Data == NULL) {
      m_ucHZK16Data = (unsigned char *)malloc(GB_FONT_BUF_SIZE);
      if (m_ucHZK16Data == NULL) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(255);
      }
   }
   Decode(szFileName, 0, m_ucHZK16Data, GB_FONT_BUF_SIZE);
}

unsigned char *CHZKFont::ReadGBGlyph(unsigned char gb_hz[2])
{
   if (is_gb(gb_hz[0], gb_hz[1]) && m_ucHZK16Data != NULL) {
      int idx = (gb_hz[0] - 0xa1) * 94 + (gb_hz[1] - 0xa1);
      return &m_ucHZK16Data[idx * GLYPH_BUF_SIZE];
   }

   return NULL;
}

unsigned char *CHZKFont::ReadASCGlyph(unsigned char c)
{
   return &iso_font[((int)c) << 4];
}
