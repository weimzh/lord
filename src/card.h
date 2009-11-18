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

#ifndef CARD_H
#define CARD_H

#include <assert.h>

enum { CLUBS = 0, DIAMONDS, HEARTS, SPADES, JOKERS };

class CCard
{
public:
   CCard(unsigned char code = 255);
   ~CCard();

   inline bool   IsValid()     const { return (m_ucCode < 54); }
   inline int    GetType()     const { return (m_ucCode / 13); }
   inline int    GetValue()    const { return (GetType() == JOKERS) ? (GetNum() + 16) :
      ((GetNum() < 3) ? (GetNum() * 2 + 12) : GetNum());  }
   inline int    GetNum()      const { return (m_ucCode % 13) + 1; }
   inline unsigned char GetCode() const { return m_ucCode; }

   static CCard  GetRandomCard();
   static void   NewRound();

   inline bool   IsJoker()     const { return (GetType() == JOKERS); }

   inline bool   operator>(const CCard &t)  const { return GetValue() > t.GetValue(); }
   inline bool   operator<(const CCard &t)  const { return GetValue() < t.GetValue(); }
   inline bool   operator>=(const CCard &t) const { return GetValue() >= t.GetValue(); }
   inline bool   operator<=(const CCard &t) const { return GetValue() <= t.GetValue(); }
   inline bool   operator==(const CCard &t) const { return GetValue() == t.GetValue(); }
   inline bool   operator!=(const CCard &t) const { return !(*this == t); }
   inline int    operator-(const CCard &t)  const { return GetValue() - t.GetValue(); }

   static void   Sort(CCard *rgCards, int iNum);

private:
   unsigned char            m_ucCode;
   static unsigned char     CardState[7];
};

#endif
