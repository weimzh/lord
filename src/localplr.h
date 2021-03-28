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

#ifndef LOCALPLR_H
#define LOCALPLR_H

#include "player.h"

class CPlayerLocal : public CBasePlayer
{
public:
   CPlayerLocal();
   ~CPlayerLocal();

   virtual int       Bid(int iAtLeast);  // returns the points the player called
   virtual int       Discard(CCard rgDiscarded[20], bool fFirstHand); // store the discarded cards in rgDiscarded

   virtual void      RoundBegin(); // called when a round begins
   virtual void      NoBidder(); // no one called
   virtual void      InvalidDiscard(); // cannot discard
   virtual void      BecomeLord(int iLocation, const CCard bottomcards[3]); // called when someone become the lord
   virtual void      PlayerBidded(int iLocation, int iBid); // called when a player bidded
   virtual void      PlayerDiscarded(int iLocation, const CCard rgCards[20], const CCard rgHand[20], int iNumCards, int iHandLeft); // called when a player discarded some cards
   virtual void      ShowAllCards(int iWinnerLoc, const CCard rgRemainCards[3][20]); // called when the round ends
   virtual void      Turn(int iLocation);
   virtual void      UpdateStatus(int base, int multiply);
   virtual void      ScoreBoard(const char rgszNames[3][20], int rgiGotScore[3], int rgiTotalScore[3], bool fWon);
   virtual void      SetPlayerNames(const char rgszNames[3][20]);
   virtual bool      IsReady(); // return true if the player is ready for playing, false if not
};

#endif
