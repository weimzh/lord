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

#ifndef GAME_H
#define GAME_H

enum { PLAYER_LOCAL = 0, PLAYER_LEFT, PLAYER_RIGHT };

// type of discarded cards
enum {
   DT_INVALID = 0,
   DT_SINGLE,
   DT_DOUBLE,
   DT_TRIPLE,
   DT_TRIPLEAT1, // triple + single
   DT_TRIPLEAT2, // triple + double
   DT_QUADAT1,   // quad + single * 2
   DT_QUADAT2,   // quad + double * 2
   DT_QUAD,
   DT_DBLJOKER,  // 2 jokers
};

#include "player.h"

typedef struct tagDiscardHand
{
   int        type, cnt;
   CCard      head;
} discardhand_t;

class CGame
{
public:
   CGame();
   ~CGame();

   void            Run();
   void            RunSingleGame();
   void            RunNetGame();
   void            AddPlayer(CBasePlayer *p, int iScore, const char *szName);

   inline CBasePlayer    *GetPlayer(int location) { return m_rgpPlayers[location]; }
   inline CBasePlayer    *GetLord()               { return m_rgpPlayers[m_iCurLord]; }
   inline bool            IsLord(int location)    { return location == m_iCurLord; }

   int                       PlayRound();
   static discardhand_t      GetDiscardType(const CCard *rgCards);
   static bool               CompareDiscard(const CCard rgFirst[20], const CCard rgNext[20],
                                            discardhand_t *pFirstDT = NULL, discardhand_t *pNextDT = NULL);

private:
   void            NewRound();
   int             Bidding();

   CBasePlayer    *m_rgpPlayers[3];
   int             m_iNumPlayer;
   int             m_iCurLord;
   int             m_iPoints;

   int             m_rgiPlayerScore[3];
   char            m_rgszPlayerNames[3][20];

   CCard           m_rgKittyCard[3];

   bool            m_fSingleGame;
};

extern CGame *gpGame;

#endif
