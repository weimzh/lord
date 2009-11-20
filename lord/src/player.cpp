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

#include "main.h"

CBasePlayer::CBasePlayer()
{
}

CBasePlayer::~CBasePlayer()
{
}

void CBasePlayer::NewRound()
{
   // get 17 random cards from the pile...
   int i;
   for (i = 0; i < 17; i++) {
      m_rgHandCard[i] = CCard::GetRandomCard();
   }
   m_iNumHandCard = 17;
   m_rgHandCard[17] = 255;
   SortCards();
}

void CBasePlayer::DoDiscard(const CCard rgDiscarded[20], int iNum)
{
   int i, j, k;

   // remove all discarded cards from the hand
   for (i = 0; i < iNum; i++) {
      k = 0;
      for (j = 0; j < m_iNumHandCard; j++) {
         if (m_rgHandCard[j].GetCode() != rgDiscarded[i].GetCode()) {
            m_rgHandCard[k++] = m_rgHandCard[j];
         }
      }
      m_iNumHandCard = k;
   }

   if (m_iNumHandCard < 20) {
      m_rgHandCard[m_iNumHandCard] = 255;
   }
}

void CBasePlayer::GetKittyCards(const CCard rgKittyCards[3])
{
   m_rgHandCard[17] = rgKittyCards[0];
   m_rgHandCard[18] = rgKittyCards[1];
   m_rgHandCard[19] = rgKittyCards[2];
   m_iNumHandCard = 20;
   SortCards();
}

void CBasePlayer::SortCards()
{
   CCard::Sort(m_rgHandCard, m_iNumHandCard);
}
