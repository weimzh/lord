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

#ifndef PLAYER_H
#define PLAYER_H

// Player base class. For each kind of player (computer AI, local player,
// net clients) should derive one class from this class.
class CBasePlayer
{
public:
   CBasePlayer();
   virtual ~CBasePlayer();

   void              NewRound();

   void              DoDiscard(const CCard rgDiscarded[20], int iNum);
   void              GetKittyCards(const CCard rgKittyCards[3]);
   void              SortCards();

   inline CCard     *Hand()             { return m_rgHandCard; }
   inline int        NumHandCard()          { return m_iNumHandCard; }

   virtual int       Bid(int iAtLeast) = 0;  // returns the points the player bidded
   virtual int       Discard(CCard rgDiscarded[20], bool fFirstHand) = 0; // store the discarded cards in rgDiscarded

   virtual void      RoundBegin() = 0; // called when a round begins
   virtual void      NoBidder() = 0; // no one bidded
   virtual void      InvalidDiscard() = 0; // cannot discard
   virtual void      BecomeLord(int iLocation, const CCard rgKittyCard[3]) = 0; // called when someone become the lord
   virtual void      PlayerBidded(int iLocation, int iBid) = 0; // called when a player bidded
   virtual void      PlayerDiscarded(int iLocation, const CCard rgCards[20], const CCard rgHand[20], int iNumCards, int iHandLeft) = 0; // called when a player discarded some cards
   virtual void      ShowAllCards(int iWinnerLoc, const CCard rgRemainCards[3][20]) = 0; // called when the round ends
   virtual void      Turn(int iLocation) = 0;
   virtual void      UpdateStatus(int base, int multiply) = 0;
   virtual void      ScoreBoard(const char rgszNames[3][20], int rgiGotScore[3], int rgiTotalScore[3], bool fWon) = 0;
   virtual void      SetPlayerNames(const char rgszNames[3][20]) = 0;
   virtual bool      IsReady() = 0; // return true if the player is ready for playing, false if not

protected:
   CCard             m_rgHandCard[20];
   int               m_iNumHandCard;
};

#endif
