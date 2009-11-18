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

CGame *gpGame = NULL;

// Relative location
inline int RL(int a, int b) {
   a -= b;
   if (a < 0) {
      a += 3;
   }
   return a;
}

CGame::CGame()
{
   m_iNumPlayer = 0;
   m_fSingleGame = false;
   m_rgpPlayers[0] = NULL;
   m_rgpPlayers[1] = NULL;
   m_rgpPlayers[2] = NULL;
}

CGame::~CGame()
{
   int i;
   for (i = 0; i < 3; i++) {
      if (m_rgpPlayers[i] != NULL) {
         delete m_rgpPlayers[i];
      }
      m_rgpPlayers[i] = NULL;
   }
}

void CGame::Run()
{
   int i, j;

   // the game must be played with 3 players
   if (m_iNumPlayer != 3) {
      TerminateOnError("CGame::Run(): m_iNumPlayer != 3!");
   }

   for (i = 0; i < 3; i++) {
      char player_names[3][20];
      strncpy(player_names[RL(0, i)], m_rgszPlayerNames[0], 20);
      strncpy(player_names[RL(1, i)], m_rgszPlayerNames[1], 20);
      strncpy(player_names[RL(2, i)], m_rgszPlayerNames[2], 20);
      player_names[0][19] = '\0';
      player_names[1][19] = '\0';
      player_names[2][19] = '\0';
      m_rgpPlayers[i]->SetPlayerNames(player_names);
   }

   // The main game loop
   while (1) {
      int iWinner = PlayRound(), scores[3];
      bool fLordWon = (iWinner == m_iCurLord);
      CCard rgRemainCards[3][20];
      for (i = 0; i < 3; i++) {
         for (j = 0; j < 20; j++) {
            rgRemainCards[i][j] = m_rgpPlayers[i]->Hand()[j];
         }
      }

      scores[0] = m_iPoints * ((fLordWon ^ (m_iCurLord != 0)) ? 1 : -1);
      scores[1] = m_iPoints * ((fLordWon ^ (m_iCurLord != 1)) ? 1 : -1);
      scores[2] = m_iPoints * ((fLordWon ^ (m_iCurLord != 2)) ? 1 : -1);
      scores[m_iCurLord] *= 2;

      if (atoi(cfg.Get("GAME", "ShowCards", "1"))) {
         m_rgpPlayers[0]->ShowAllCards(RL(iWinner, 0), rgRemainCards);
         m_rgpPlayers[1]->ShowAllCards(RL(iWinner, 1), rgRemainCards);
         m_rgpPlayers[2]->ShowAllCards(RL(iWinner, 2), rgRemainCards);
      }

      m_rgiPlayerScore[0] += scores[0];
      m_rgiPlayerScore[1] += scores[1];
      m_rgiPlayerScore[2] += scores[2];

      if (m_fSingleGame) {
         cfg.Set("GAME", "Score", va("%d", m_rgiPlayerScore[0]));
         cfg.Set("GAME", "BotScore1", va("%d", m_rgiPlayerScore[1]));
         cfg.Set("GAME", "BotScore2", va("%d", m_rgiPlayerScore[2]));
      }

      // send score information to players
      m_rgpPlayers[0]->ScoreBoard(m_rgszPlayerNames, scores, m_rgiPlayerScore, scores[0] > 0);
      m_rgpPlayers[1]->ScoreBoard(m_rgszPlayerNames, scores, m_rgiPlayerScore, scores[1] > 0);
      m_rgpPlayers[2]->ScoreBoard(m_rgszPlayerNames, scores, m_rgiPlayerScore, scores[2] > 0);

      // Wait for all players to get ready for next round
      time_t t;
      time(&t);
      while (1) {
         UTIL_Delay(10);
         if (m_rgpPlayers[0]->IsReady() && m_rgpPlayers[1]->IsReady() && m_rgpPlayers[2]->IsReady()) {
            break;
         }
         if (time(NULL) - t > 30 && !m_fSingleGame) {
            g_UI.PopMessage(msg("someone_quit"));
            return; // someone delayed for too long; close the server
         }
      }
   }
}

void CGame::RunSingleGame()
{
   AddPlayer(new CPlayerLocal, atoi(cfg.Get("GAME", "Score", "1000")), cfg.Get("GAME", "PlayerName", "Player"));
   AddPlayer(new CBot, atoi(cfg.Get("GAME", "BotScore1", "1000")), msg("bot_name1"));
   AddPlayer(new CBot, atoi(cfg.Get("GAME", "BotScore2", "1000")), msg("bot_name2"));
   m_fSingleGame = true;
   Run();
   for (int i = 0; i < 3; i++) {
      if (m_rgpPlayers[i] != NULL) {
         delete m_rgpPlayers[i];
      }
      m_rgpPlayers[i] = NULL;
   }
}

void CGame::RunNetGame()
{
#if 0
   char name[20];
   int i;
   AddPlayer(new CPlayerLocal, 1000, cfg.Get("GAME", "PlayerName", "Player"));
   for (i = 0; i < 2; i++) {
      CBasePlayer *p = CClient::Accept(cfg.Get("GAME", "ServerPwd", ""), name);
      AddPlayer(p, 1000, name);
   }
   m_fSingleGame = false;
   Run();
   for (i = 0; i < 3; i++) {
      if (m_rgpPlayers[i] != NULL) {
         delete m_rgpPlayers[i];
      }
      m_rgpPlayers[i] = NULL;
   }
#endif
}

void CGame::AddPlayer(CBasePlayer *p, int iScore, const char *szName)
{
   if (m_iNumPlayer >= 3) {
      TerminateOnError("CGame::AddPlayer(): m_iNumPlayer >= 3!");
   }
   m_rgpPlayers[m_iNumPlayer] = p;
   m_rgiPlayerScore[m_iNumPlayer] = iScore;
   strncpy(m_rgszPlayerNames[m_iNumPlayer], szName, 20);
   m_rgszPlayerNames[m_iNumPlayer][19] = '\0';
   m_iNumPlayer++;
}

// New round started
void CGame::NewRound()
{
   CCard::NewRound();

   m_rgpPlayers[0]->NewRound();
   m_rgpPlayers[1]->NewRound();
   m_rgpPlayers[2]->NewRound();

   for (int i = 0; i < 3; i++) {
      m_rgKittyCard[i] = CCard::GetRandomCard();
   }

   m_rgpPlayers[0]->RoundBegin();
   m_rgpPlayers[1]->RoundBegin();
   m_rgpPlayers[2]->RoundBegin();
}

// Decide who the landlord is.
int CGame::Bidding()
{
   int iRandomLord = RandomLong(0, 2), c = 0, h = 0;
   m_iCurLord = -1;
   while (c < 3 && h <= 3) {
      int index = iRandomLord - c;
      if (index < 0) {
         index += 3;
      }

      m_rgpPlayers[0]->Turn(RL(index, 0));
      m_rgpPlayers[1]->Turn(RL(index, 1));
      m_rgpPlayers[2]->Turn(RL(index, 2));

      int s = GetPlayer(index)->Bid(h);
      if (s < h) {
         s = 0;
      }

      m_rgpPlayers[0]->PlayerBidded(RL(index, 0), s);
      m_rgpPlayers[1]->PlayerBidded(RL(index, 1), s);
      m_rgpPlayers[2]->PlayerBidded(RL(index, 2), s);

      if (s >= h) {
         m_iCurLord = index;
         h = s + 1;
      }
      c++;
   }
   return h - 1;
}

int CGame::PlayRound()
{
   int turn, count, last_count = 0, num_pass = 0, i, base, multiply = 1;
   CCard rgDiscarded[20], rgLastDiscarded[20];
   bool lord_followed = false, other_followed = false;

   m_iCurLord = -1;
   while (1) {
      NewRound();
      base = Bidding();

      if (base > 0) {
         break;
      }

      m_rgpPlayers[0]->NoBidder();
      m_rgpPlayers[1]->NoBidder();
      m_rgpPlayers[2]->NoBidder();
   }

   m_rgpPlayers[m_iCurLord]->GetKittyCards(m_rgKittyCard);
   m_rgpPlayers[0]->BecomeLord(RL(m_iCurLord, 0), m_rgKittyCard);
   m_rgpPlayers[1]->BecomeLord(RL(m_iCurLord, 1), m_rgKittyCard);
   m_rgpPlayers[2]->BecomeLord(RL(m_iCurLord, 2), m_rgKittyCard);

   turn = m_iCurLord;

   rgLastDiscarded[0] = 255;

   while (1) {
      count = -1;
      m_rgpPlayers[0]->UpdateStatus(base, multiply);
      m_rgpPlayers[1]->UpdateStatus(base, multiply);
      m_rgpPlayers[2]->UpdateStatus(base, multiply);
      m_rgpPlayers[0]->Turn(RL(turn, 0));
      m_rgpPlayers[1]->Turn(RL(turn, 1));
      m_rgpPlayers[2]->Turn(RL(turn, 2));
      while (count < 0) {
         count = m_rgpPlayers[turn]->Discard(rgDiscarded, !last_count);
         CCard::Sort(rgDiscarded, count);
         if (count < 20) {
            rgDiscarded[count] = 255;
         }
         if (!last_count) {
            // first hand
            i = GetDiscardType(rgDiscarded).type;
            if (i == DT_INVALID) {
               m_rgpPlayers[turn]->InvalidDiscard();
               count = -1;
            } else {
               m_rgpPlayers[turn]->DoDiscard(rgDiscarded, count);
               if (i >= DT_QUAD) {
                  multiply *= 2;
               }
               for (i = 0; i < count; i++) {
                  rgLastDiscarded[i] = rgDiscarded[i];
               }
               if (i < 20) {
                  rgLastDiscarded[i] = 255;
               }
               last_count = count;
            }
         } else if (count != 0) {
            // following a card
            discardhand_t dh;
            if (!CompareDiscard(rgLastDiscarded, rgDiscarded, NULL, &dh)) {
               m_rgpPlayers[turn]->InvalidDiscard();
               count = -1;
            } else {
               m_rgpPlayers[turn]->DoDiscard(rgDiscarded, count);
               if (dh.type >= DT_QUAD) {
                  multiply *= 2;
               }
               for (i = 0; i < count; i++) {
                  rgLastDiscarded[i] = rgDiscarded[i];
               }
               if (i < 20) {
                  rgLastDiscarded[i] = 255;
               }
               last_count = count;
               num_pass = 0; // clear the counter of passes
               if (turn == m_iCurLord) {
                  lord_followed = true;
               } else {
                  other_followed = true;
               }
            }
         } else {
            // pass
            num_pass++;
            if (num_pass >= 2) {
               // 2 players passed
               num_pass = 0;
               rgLastDiscarded[0] = 255;
               last_count = 0;
               // if the next player is the lord
               if (((turn > 0) ? (turn - 1) : 2) == m_iCurLord) {
                  lord_followed = true;
               }
            }
         }
      }
#ifdef _DEBUG
      m_rgpPlayers[0]->PlayerDiscarded(RL(turn, 0), rgDiscarded, m_rgpPlayers[turn]->Hand(),
         count, m_rgpPlayers[turn]->NumHandCard());
      m_rgpPlayers[1]->PlayerDiscarded(RL(turn, 1), rgDiscarded, m_rgpPlayers[turn]->Hand(),
         count, m_rgpPlayers[turn]->NumHandCard());
      m_rgpPlayers[2]->PlayerDiscarded(RL(turn, 2), rgDiscarded, m_rgpPlayers[turn]->Hand(),
         count, m_rgpPlayers[turn]->NumHandCard());
#else
      m_rgpPlayers[0]->PlayerDiscarded(RL(turn, 0), rgDiscarded, turn == 0 ? m_rgpPlayers[turn]->Hand() : NULL,
         count, m_rgpPlayers[turn]->NumHandCard());
      m_rgpPlayers[1]->PlayerDiscarded(RL(turn, 1), rgDiscarded, turn == 1 ? m_rgpPlayers[turn]->Hand() : NULL,
         count, m_rgpPlayers[turn]->NumHandCard());
      m_rgpPlayers[2]->PlayerDiscarded(RL(turn, 2), rgDiscarded, turn == 2 ? m_rgpPlayers[turn]->Hand() : NULL,
         count, m_rgpPlayers[turn]->NumHandCard());
#endif
      if (m_rgpPlayers[turn]->NumHandCard() <= 0) {
         // this player won the round
         break;
      }
      turn--;
      if (turn < 0) {
         turn = 2;
      }
   }
   if (turn == m_iCurLord) {
      multiply *= other_followed ? 1 : 2;
   } else {
      multiply *= lord_followed ? 1 : 2;
   }
   m_rgpPlayers[0]->UpdateStatus(base, multiply);
   m_rgpPlayers[1]->UpdateStatus(base, multiply);
   m_rgpPlayers[2]->UpdateStatus(base, multiply);

   m_iPoints = base * multiply;
   return turn;
}

// Get the type of discarded cards stored in rgCards
discardhand_t CGame::GetDiscardType(const CCard *rgCards)
{
   static discardhand_t dh;

#define RETURN_DH(cc, bb, tt) { \
   dh.cnt = cc; \
   dh.head = bb; \
   dh.type = tt; \
   return dh; \
}

   if (rgCards == NULL) {
      return dh; // return last result
   }

   CCard cards[20], sortcards[4][20];
   int count, i, j, sortcount[4] = {0, 0, 0, 0};

   for (count = 0; count < 20; count++) {
      if (!rgCards[count].IsValid()) {
         break;
      }
      cards[count] = rgCards[count];
   }

   // If we only have a few cards (less than 4), no need to sort out the cards...
   switch (count) {
      case 0:
         RETURN_DH(0, 0, DT_INVALID);

      case 1:
         RETURN_DH(1, cards[0], DT_SINGLE);

      case 2:
         if (cards[0].IsJoker() && cards[1].IsJoker()) {
            RETURN_DH(1, cards[0], DT_DBLJOKER); // two jokers
         } else if (cards[0] != cards[1]) {
            RETURN_DH(0, 0, DT_INVALID);
         }
         RETURN_DH(1, cards[0], DT_DOUBLE);

      case 3:
         if (cards[0] != cards[1] || cards[1] != cards[2]) {
            RETURN_DH(0, 0, DT_INVALID);
         }
         RETURN_DH(1, cards[0], DT_TRIPLE);
   }

   // more than 3 cards, sort out the cards
   CCard::Sort(cards, count);

   i = 0;
   while (i < count) {
      j = i;
      while (i < count && cards[j] == cards[i]) {
         i++;
      }
      sortcards[i - j - 1][sortcount[i - j - 1]++] = cards[j];
   }

   if (sortcount[3] > 1) {
      RETURN_DH(0, 0, DT_INVALID); // 2 or more quads are not possible
   } else if (sortcount[3] == 1) {
      if (sortcount[2] > 0) {
         RETURN_DH(0, 0, DT_INVALID); // triples are not possible
      }
      if (sortcount[1] == 0 && sortcount[0] == 0) {
         RETURN_DH(1, cards[0], DT_QUAD); // quad
      } else if (sortcount[1] == 2 && sortcount[0] == 0) {
         RETURN_DH(1, sortcards[3][0], DT_QUADAT2); // quad with two pairs
      } else if (sortcount[0] == 2 || sortcount[1] == 1) {
         RETURN_DH(1, sortcards[3][0], DT_QUADAT1); // quad with two singles
      }
   } else if (sortcount[2] >= 1) {
      for (i = 0; i < sortcount[2] - 1; i++) {
         if (sortcards[2][i] - sortcards[2][i + 1] != 1) {
            RETURN_DH(0, 0, DT_INVALID);
         }
      }
      if (sortcount[2] == sortcount[1] && sortcount[0] == 0) {
         RETURN_DH(sortcount[2], sortcards[2][0], DT_TRIPLEAT2);
      } else if (sortcount[2] == sortcount[0] + sortcount[1] * 2) {
         RETURN_DH(sortcount[2], sortcards[2][0], DT_TRIPLEAT1);
      } else if (sortcount[0] == 0 && sortcount[1] == 0) {
         RETURN_DH(sortcount[2], sortcards[2][0], DT_TRIPLE);
      }
      RETURN_DH(0, 0, DT_INVALID);
   } else if (sortcount[1] >= 1) {
      if (sortcount[0] >= 1 || sortcount[1] < 3) {
         RETURN_DH(0, 0, DT_INVALID); // singles and doubles cannot be played together
      }
      for (i = 0; i < sortcount[1] - 1; i++) {
         if (sortcards[1][i] - sortcards[1][i + 1] != 1) {
            RETURN_DH(0, 0, DT_INVALID); // fail
         }
      }
      RETURN_DH(sortcount[1], sortcards[1][0], DT_DOUBLE);
   } else {
      if (sortcount[0] < 5) {
         // less than 5 cards, not possible
         RETURN_DH(0, 0, DT_INVALID);
      }
      for (i = 0; i < sortcount[0] - 1; i++) {
         if (sortcards[0][i] - sortcards[0][i + 1] != 1) {
            RETURN_DH(0, 0, DT_INVALID); // fail
         }
      }
      RETURN_DH(sortcount[0], sortcards[0][0], DT_SINGLE);
   }
   RETURN_DH(0, 0, DT_INVALID); // should not really reach here
}

// return true if rgNext can beat rgFirst, false if not
bool CGame::CompareDiscard(const CCard rgFirst[20], const CCard rgNext[20],
   discardhand_t *pFirstDT, discardhand_t *pNextDT)
{
   discardhand_t dtFirst, dtNext;

   dtFirst = GetDiscardType(rgFirst);
   dtNext = GetDiscardType(rgNext);

   if (pFirstDT != NULL) {
      *pFirstDT = dtFirst;
   }

   if (pNextDT != NULL) {
      *pNextDT = dtNext;
   }

   if (dtFirst.type == DT_INVALID || dtNext.type == DT_DBLJOKER) {
      return true;
   } else if (dtFirst.type == DT_DBLJOKER || dtNext.type == DT_INVALID) {
      return false;
   } else if (dtFirst.type == dtNext.type) {
      return dtFirst.cnt == dtNext.cnt && dtFirst.head < dtNext.head;
   } else if (dtNext.type == DT_QUAD) {
      return true;
   }

   return false;
}
