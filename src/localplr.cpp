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

#include "main.h"

CPlayerLocal::CPlayerLocal()
{
}

CPlayerLocal::~CPlayerLocal()
{
}

int CPlayerLocal::Bid(int iAtLeast)
{
   return g_UI.UIBid(iAtLeast);
}

int CPlayerLocal::Discard(CCard rgDiscarded[20], bool fFirstHand)
{
   return g_UI.UIDiscard(m_rgHandCard, m_iNumHandCard, rgDiscarded, fFirstHand);
}

void CPlayerLocal::RoundBegin()
{
   g_UI.NewRound();
   gpGeneral->PlaySound(SOUND_HINT2);
   g_UI.DrawPlayer(PLAYER_LEFT, NULL, 17, NULL, -1);
   g_UI.DrawPlayer(PLAYER_RIGHT, NULL, 17, NULL, -1);
   g_UI.DrawPlayer(PLAYER_LOCAL, m_rgHandCard, 17, NULL, -1);
   g_UI.DrawKittyCard(NULL);
}

void CPlayerLocal::NoBidder()
{
   g_UI.PopMessage(msg("noone_call"));
}

void CPlayerLocal::InvalidDiscard()
{
   g_UI.PopMessage(msg("invalid_discard"));
}

void CPlayerLocal::BecomeLord(int iLocation, const CCard rgKittyCards[3])
{
   UTIL_Delay(800);
   gpGeneral->EraseArea(220 - 65 / 2, 240 - 65 / 2, 270, 140, true);
   gpGeneral->UpdateScreen(220 - 65 / 2, 240 - 65 / 2, 270, 150);
   g_UI.DrawLord(iLocation);
   g_UI.DrawKittyCard(rgKittyCards);
   g_UI.DrawPlayer(iLocation, iLocation == PLAYER_LOCAL ? m_rgHandCard : NULL,
      20, NULL, -1);
}

void CPlayerLocal::PlayerBidded(int iLocation, int iCall)
{
   g_UI.PlayerBidded(iLocation, iCall);
}

void CPlayerLocal::PlayerDiscarded(int iLocation, const CCard rgCards[20], const CCard rgHand[20],
                                   int iNumCards, int iHandLeft)
{
   // Update the player on the screen
   g_UI.DrawPlayer(iLocation, rgHand, iHandLeft, rgCards, iNumCards);
   // Also play a sound if he actually discarded something
   if (iNumCards > 0) {
      gpGeneral->PlaySound(CGame::GetDiscardType(rgCards).type >= DT_QUAD ?
         SOUND_BOMB : SOUND_MOVECARD);
   }
}

void CPlayerLocal::ShowAllCards(int iWinnerLoc, const CCard rgRemainCards[3][20])
{
   int i, j;
   for (i = 0; i < 3; i++) {
      if (i == iWinnerLoc) {
         continue;
      }
      for (j = 0; j < 20; j++) {
         if (!rgRemainCards[i][j].IsValid()) {
            break;
         }
      }
      g_UI.DrawPlayer(i, rgRemainCards[i], j, NULL, -1);
   }
}

void CPlayerLocal::ScoreBoard(const char rgszNames[3][20], int rgiGotScore[3], int rgiTotalScore[3], bool fWon)
{
   g_UI.ScoreBoard(rgszNames, rgiGotScore, rgiTotalScore);
   // also play a sound
   gpGeneral->PlaySound(fWon ? SOUND_WIN : SOUND_LOST);
}

void CPlayerLocal::Turn(int iLocation)
{
   g_UI.DrawTurn(iLocation);
}

void CPlayerLocal::UpdateStatus(int base, int multiply)
{
   g_UI.DrawStatus(base, multiply);
}

void CPlayerLocal::SetPlayerNames(const char rgszNames[3][20])
{
   g_UI.SetPlayerNames(rgszNames);
}

bool CPlayerLocal::IsReady()
{
   extern bool g_fLocalReady;
   return g_fLocalReady;
}
