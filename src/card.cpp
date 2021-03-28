//
// Copyright (c) 2009 Wei Mingzhi <whistler_wmz@users.sf.net>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License,
// version 3, as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see http://www.gnu.org/licenses.
//

#include "main.h"

unsigned char CCard::CardState[7];

CCard::CCard(unsigned char code)
{
   m_ucCode = code;
}

CCard::~CCard()
{
}

// Get a random card from the pile
CCard CCard::GetRandomCard()
{
   unsigned char pile[54];
   unsigned char i, j, remaining = 0;

   for (i = 0; i < 7; i++) {
      for (j = 0; j < 8; j++) {
         if (!(CardState[i] & (1 << j))) {
            // This card is available
            pile[remaining++] = ((i << 3) | j);
         }
      }
   }

   assert(remaining <= 54);
   if (remaining == 0) {
      TerminateOnError("CCard::GetRandomCard(): No card left!");
   }

   i = RandomLong(0, remaining - 1); // get a random card
   CardState[(pile[i] >> 3)] |= (1 << (pile[i] & 7)); // mark this card as unavailable

   return pile[i];
}

// Initialize the pile, mark all cards as available
void CCard::NewRound()
{
   memset(CardState, 0, sizeof(CardState));
   CardState[6] = 0xC0;
}

void CCard::Sort(CCard *rgCards, int iNum)
{
   if (iNum <= 1) {
      return;
   }

   CCard pivot = *rgCards, *a = rgCards, *b = rgCards + iNum - 1;

   while (a < b) {
      while (a < b && *b <= pivot)
         b--;
      *a = *b;
      while (a < b && *a >= pivot)
         a++;
      *b = *a;
   }

   *a = pivot;

   Sort(rgCards, a - rgCards);
   Sort(a + 1, iNum - (a - rgCards) - 1);
}
