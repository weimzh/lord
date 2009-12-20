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

#ifndef BOT_H
#define BOT_H

#include "player.h"

typedef struct bothand_s {
   int type, cnt, headval;
} bothand_t;

class CBot : public CBasePlayer
{
public:
   CBot();
   ~CBot();

   virtual int       Bid(int iAtLeast);  // returns the points the player called
   virtual int       Discard(CCard rgDiscarded[20], bool fFirstHand); // store the discarded cards in rgDiscarded

   virtual void      RoundBegin(); // called when a round begins
   virtual void      NoBidder(); // no one bidded
   virtual void      InvalidDiscard(); // cannot discard
   virtual void      BecomeLord(int iLocation, const CCard bottomcards[3]); // called when someone become the lord
   virtual void      PlayerBidded(int iLocation, int iBid); // called when a player called a score
   virtual void      PlayerDiscarded(int iLocation, const CCard rgCards[20], const CCard rgHand[20], int iNumCards, int iHandLeft); // called when a player discarded some cards
   virtual void      ShowAllCards(int iWinnerLoc, const CCard rgRemainCards[3][20]); // called when the round ends
   virtual void      Turn(int iLocation);
   virtual void      UpdateStatus(int base, int multiply);
   virtual void      ScoreBoard(const char rgszNames[3][20], int rgiGotScore[3], int rgiTotalScore[3], bool fWon);
   virtual void      SetPlayerNames(const char rgszNames[3][20]);
   virtual bool      IsReady(); // return true if the player is ready for playing, false if not

private:
   discardhand_t     m_rgHandPlayed[3][20];
   discardhand_t    *m_pLastHandPlayed;
   int               m_rgiNumHandPlayed[3];
   CCard             m_rgBottomCard[3];
   int               m_rgiRemainingCard[15];

   bothand_t         m_rgHand[100];
   int               m_iNumHand;

   int               m_iCurLord;
   int               m_iCurPoint;

   void              RecordDiscard(int iLocation, const CCard rgCards[20], int iNumCards);
   inline bool       IsLord() { return m_iCurLord == PLAYER_LOCAL; }

   void                 CardPlayed(const CCard &c);
   inline int           GetRemainingCard(int num) { return m_rgiRemainingCard[num - 1]; }
   inline int           GetRemainingCard(const CCard &c) { return c.IsJoker() ? m_rgiRemainingCard[c.GetNum() + 12] : m_rgiRemainingCard[c.GetNum() - 1]; }
   const discardhand_t &GetLastDiscard();
   void                 AnalyzeHand();
   int                  FirstHandDiscard(CCard rgDiscarded[20]);
   int                  FollowCard(CCard rgDiscarded[20]);

   int                  FollowCardSingle(CCard rgDiscarded[20]);
   int                  FollowCardDouble(CCard rgDiscarded[20]);
   int                  FollowCardTriple(CCard rgDiscarded[20]);
   int                  FollowCardTripleAt1(CCard rgDiscarded[20]);
   int                  FollowCardTripleAt2(CCard rgDiscarded[20]);
   int                  FollowCardQuadAt1(CCard rgDiscarded[20]);
   int                  FollowCardQuadAt2(CCard rgDiscarded[20]);
   int                  FollowCardQuad(CCard rgDiscarded[20]);
};

#endif
