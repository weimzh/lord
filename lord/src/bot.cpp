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

//#define BOT_DEBUG 1

CBot::CBot()
{
}

CBot::~CBot()
{
}

// We only like to become the declarer when:
// 1) We have 3 or more beating cards (2's or Joker's)
// 2) We have 6 or more high-ranked cards (K/A/2/Joker)
// 3) We have 2 or less leftover cards which cannot be easily got rid of
// (single cards smaller than J or pairs smaller than 8)
// TODO: may be much smarter than this...
int CBot::Bid(int iAtLeast)
{
   UTIL_Delay(RandomLong(800, 1500)); // delay a while

   int iValue = 0, i, j, k, num_cards[16], iNumLeftOver = 0;
   int iNumTriple = 0, iNumHead = 0, subseq = -1, last_subseq = -1;

   // count all kind of cards
   memset(num_cards, 0, sizeof(num_cards));
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }

   if (num_cards[10] + num_cards[11] + num_cards[13] + num_cards[14] +
      num_cards[15] < 6) {
      // we have too few big cards, not feeling so good
      iValue = RandomLong(0, 1);
      goto end;
   }

   // take out all sequences
   for (i = 0; i < 8; i++) {
      if (num_cards[i] == 0) {
         continue;
      }
      for (j = i + 1; j < 12; j++) {
         if (num_cards[j] <= 0) {
            // the sequence ends right here
            break;
         }
      }
      if (j - i < 5) {
         i = j - 1;
         continue; // contains less than 5 cards, ignore this
      }
      if (num_cards[j - 1] >= 3 && j - i > 5) {
         j--; // do not end with a bomb or a triple
      }
      if (num_cards[i] >= 3 && j - i > 5) {
         i++; // do not start with a bomb or a triple
      }
      // take the sequence out
      for (k = i; k < j; k++) {
         num_cards[k]--; // take out one card from this rank
      }
      // see if we have some subsequences which can be included
      // in the main sequence to form 2 or more sequences...
      if (j - i < 7) {
         continue; // less than 7 cards is impossible
      }
      last_subseq = i;
      for (k = i; k < j; k++) {
         if (num_cards[k] > 0) {
            if (subseq == -1) {
               subseq = k;
            }
         } else {
            if (subseq != -1) {
               if (j - subseq >= 5 && k - last_subseq >= 5) {
                  // take this out as well
                  while (subseq < k) {
                     assert(num_cards[subseq] > 0);
                     num_cards[subseq++]--;
                  }
                  last_subseq = subseq;
               }
               subseq = -1;
            }
         }
      }
      i = j - 1; // skip the cards we have already proceeded
   }

   // take out all double sequences
   for (i = 0; i < 10; i++) {
      if (num_cards[i] < 2) {
         continue;
      }
      for (j = i + 1; j < 12; j++) {
         if (num_cards[j] < 2) {
            // the sequence ends right here
            break;
         }
      }
      if (j - i < 3) {
         i = j - 1;
         continue; // contains less than 5 cards, ignore this
      }
      if (num_cards[j - 1] >= 3 && j - i > 3) {
         j--; // do not end with a bomb or a triple
      }
      if (num_cards[i] >= 3 && j - i > 3) {
         i++; // do not start with a bomb or a triple
      }
      // take the sequence out
      for (k = i; k < j; k++) {
         num_cards[k] -= 2; // take out 2 cards from this rank
      }
   }

   // leftovers

   // pairs smaller than 8
   for (i = 0; i < 5; i++) {
      if (num_cards[i] == 2) {
         iNumLeftOver++;
      }
   }

   // singles smaller than J
   for (i = 0; i < 8; i++) {
      if (num_cards[i] == 1) {
         iNumLeftOver++;
      }
   }

   // triples and quads
   for (i = 0; i < 12; i++) {
      if (num_cards[i] == 3) {
         iNumTriple++;
      } else if (num_cards[i] == 4) {
         iNumHead++; // bomb
      }
   }

   // big cards
   iNumHead += num_cards[13]; // 2
   iNumHead += num_cards[14]; // Black Joker
   iNumHead += num_cards[15]; // Red Joker

   if (iNumHead < 3) {
      iValue = RandomLong(0, 1);
      goto end; // not feeling so good
   }

   if (!num_cards[15]) {
      iNumHead--;
   }
   if (!num_cards[14]) {
      iNumHead--;
   }

   i = iNumLeftOver - iNumTriple - iNumHead;

#ifdef BOT_DEBUG
   printf("CBot::Bid(): iNumLeftOver = %d, iNumTriple = %d, iNumHead =%d -> %d\n",
          iNumLeftOver, iNumTriple, iNumHead, i);
#endif

   if (i > 3) {
      iValue = RandomLong(0, 2); // we're not feeling so good
   } else if (i >= 2) {
      iValue = RandomLong(2, 3); // try a bet
   } else {
      iValue = 3; // very good
   }

end:
   return (iValue < iAtLeast) ? 0 : iValue;
}

int CBot::Discard(CCard rgDiscarded[20], bool fFirstHand)
{
   UTIL_Delay(470); // delay a while
   if (fFirstHand) {
      return FirstHandDiscard(rgDiscarded);
   }
   return FollowCard(rgDiscarded);
}

void CBot::AnalyzeHand()
{
   int         i, j, num_cards[16], has_triplet[16];

   // count all kind of cards
   memset(num_cards, 0, sizeof(num_cards));
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }

   m_iNumHand = 0;

   // check for double jokers
   if (num_cards[14] > 0 && num_cards[15] > 0) {
      m_rgHand[m_iNumHand].type = DT_DBLJOKER;
      m_rgHand[m_iNumHand].cnt = 1;
      m_rgHand[m_iNumHand].headval = 18;
      m_iNumHand++;

      num_cards[14] = 0;
      num_cards[15] = 0;
   }

   // check for bombs
   for (i = 0; i < 16; i++) {
      if (num_cards[i] == 4) {
         m_rgHand[m_iNumHand].type = DT_QUAD;
         m_rgHand[m_iNumHand].cnt = 1;
         m_rgHand[m_iNumHand].headval = i + 3;

         m_iNumHand++;
         num_cards[i] = 0;
      }
   }

   // check for triplets
   for (i = 0; i < 16; i++) {
      has_triplet[i] = -1;
   }

   for (i = 0; i < 16; i++) {
      if (num_cards[i] == 3) {
         m_rgHand[m_iNumHand].type = DT_TRIPLE;
         m_rgHand[m_iNumHand].cnt = 1;
         m_rgHand[m_iNumHand].headval = i + 3;

         num_cards[i] -= 3;

         for (j = i + 1; j < 12; j++) {
            if (num_cards[j] == 3) {
               m_rgHand[m_iNumHand].cnt++;
               m_rgHand[m_iNumHand].headval = j + 3;
               num_cards[j] -= 3;
            } else {
               break;
            }
         }

         // record the triplet index for future reference
         if (m_rgHand[m_iNumHand].cnt == 1) {
            has_triplet[m_rgHand[m_iNumHand].headval - 3] = m_iNumHand;
         }

         m_iNumHand++;
      }
   }

   // check for sequences
   for (i = 0; i < 8; i++) {
      if (num_cards[i] == 0) {
         continue;
      }

      for (j = i + 1; j < 12; j++) {
         if (num_cards[j] <= 0) {
            // the sequence ends right here
            break;
         }
      }
      if (j - i < 5) {
         i = j - 1;
         continue; // contains less than 5 cards, ignore this
      }

      m_rgHand[m_iNumHand].type = DT_SINGLE;
      m_rgHand[m_iNumHand].cnt = 5;
      m_rgHand[m_iNumHand].headval = i + 3 + (5 - 1);

      for (j = 0; j < 5; j++) {
         num_cards[i + j]--;
      }

      m_iNumHand++;
   }

   // try extending sequences
   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type == DT_SINGLE) {
         for (j = m_rgHand[i].headval + 1 - 3; j < 12; j++) {
            if (num_cards[j] == 0) {
               break;
            }
            m_rgHand[i].headval = j + 3;
            m_rgHand[i].cnt++;
            num_cards[j]--;
         }
      }
   }

   // try if we can form more sequences by breaking triplets
   for (i = 0; i < 12; i++) {
      if (num_cards[i] == 0 && has_triplet[i] == -1) {
         continue;
      }

      for (j = i + 1; j < 12; j++) {
         if (num_cards[j] <= 0 && has_triplet[j] == -1) {
            // the sequence ends right here
            break;
         }
      }
      if (j - i < 5) {
         i = j - 1;
         continue; // contains less than 5 cards, ignore this
      }

      m_rgHand[m_iNumHand].type = DT_SINGLE;
      m_rgHand[m_iNumHand].cnt = j - i;
      m_rgHand[m_iNumHand].headval = j - 1 + 3;
      m_iNumHand++;

      while (i < j) {
         if (num_cards[i] != 0) {
            num_cards[i]--;
         } else if (has_triplet[i] != -1) {
            m_rgHand[has_triplet[i]].type = DT_INVALID;
            has_triplet[i] = -1;
            num_cards[i] += 3 - 1;
         }
         i++;
      }

      i--;
   }
/*
   // try if we can extend sequences by >2 ranks by breaking triplets
   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type == DT_SINGLE) {
         for (j = m_rgHand[i].headval + 1 - 3; j < 12; j++) {
            if (num_cards[j] == 0 && has_triplet[j] == -1) {
               break;
            }
         }
         if (j - (m_rgHand[i].headval - 3) > 2) {
         }
      }
   }

   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type == DT_SINGLE) {
         for (j = m_rgHand[i].headval - 3 - m_rgHand[i].cnt - 1; j >= 0; j--) {
            if (num_cards[j] == 0 && has_triplet[j] == -1) {
               break;
            }
         }
         if (m_rgHand[i].headval - 3 - m_rgHand[i].cnt - j > 2) {
         }
      }
   }
*/
   // merge continuous sequences
   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type == DT_SINGLE) {
         for (j = 0; j < m_iNumHand; j++) {
            if (i == j) {
               continue;
            }
            if (m_rgHand[j].headval - m_rgHand[j].cnt == m_rgHand[i].headval) {
               m_rgHand[i].headval = m_rgHand[j].headval;
               m_rgHand[i].cnt += m_rgHand[j].cnt;
               m_rgHand[j].type = DT_INVALID;
               j = i;
            }
         }
      }
   }

   // try if we can merge sequences into pair sequences
   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type == DT_SINGLE) {
         for (j = i + 1; j < m_iNumHand; j++) {
            if (m_rgHand[j].type == DT_SINGLE &&
                m_rgHand[j].cnt == m_rgHand[i].cnt &&
                m_rgHand[j].headval == m_rgHand[i].headval) {
               m_rgHand[i].type = DT_DOUBLE;
               m_rgHand[j].type = DT_INVALID;
               break;
            }
         }
      }
   }

   // remove invalid hands from the list
   j = 0;
   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type != DT_INVALID) {
         m_rgHand[j] = m_rgHand[i];
         j++;
      }
   }
   m_iNumHand = j;

   // check for pairs and pair sequences
   for (i = 0; i < 16; i++) {
      assert(num_cards[i] < 3);
      if (num_cards[i] == 2) {
         m_rgHand[m_iNumHand].type = DT_DOUBLE;
         m_rgHand[m_iNumHand].cnt = 1;
         m_rgHand[m_iNumHand].headval = i + 3;
         num_cards[i] -= 2;

         for (j = i + 1; j < 16; j++) {
            if (num_cards[j] < 2) {
               break;
            }
         }

         if (j - i >= 3) {
            while (i + 1 < j) {
               m_rgHand[m_iNumHand].cnt++;
               m_rgHand[m_iNumHand].headval++;
               num_cards[i + 1] -= 2;
               i++;
            }
         }

         m_iNumHand++;
      }
   }

   // check for singles
   for (i = 0; i < 16; i++) {
      assert(num_cards[i] < 2);
      if (num_cards[i] == 1) {
         m_rgHand[m_iNumHand].type = DT_SINGLE;
         m_rgHand[m_iNumHand].cnt = 1;
         m_rgHand[m_iNumHand].headval = i + 3;
         num_cards[i]--;
         m_iNumHand++;
      }
   }
}

int CBot::FirstHandDiscard(CCard rgDiscarded[20])
{
//   AnalyzeHand();

   // TODO
//////////////////////////////////////////////////////////////////////////////////////
//TEST CODE
   int t[20];
if (m_iNumHandCard==20)AnalyzeHand();
   memset(t, 0, sizeof(t));
   if (m_iNumHand > 0) {
      int index = m_iNumHand - 1, i, j, k, c = 0;
      switch (m_rgHand[index].type) {
         case DT_SINGLE:
         printf("single head=%d, cnt=%d\n",m_rgHand[index].headval, m_rgHand[index].cnt);
            for (i = m_rgHand[index].headval - m_rgHand[index].cnt + 1; i <= m_rgHand[index].headval; i++) {
               for (j = 0; j < m_iNumHandCard; j++) {
                  if (m_rgHandCard[j].GetValue() == i) {
                     rgDiscarded[c++] = m_rgHandCard[j];
                     break;
                  }
               }
            }
            break;
         case DT_DOUBLE:
         printf("double head=%d, cnt=%d\n",m_rgHand[index].headval, m_rgHand[index].cnt);
            for (i = m_rgHand[index].headval - m_rgHand[index].cnt + 1; i <= m_rgHand[index].headval; i++) {
               for (k = 0; k < 2; k++) {
                  for (j = 0; j < m_iNumHandCard; j++) {
                     if (m_rgHandCard[j].GetValue() == i && !t[j]) {
                        rgDiscarded[c++] = m_rgHandCard[j];
                        t[j] = 1;
                        break;
                     }
                  }
               }
            }
            break;
         case DT_TRIPLE:
         printf("triple head=%d, cnt=%d\n",m_rgHand[index].headval, m_rgHand[index].cnt);
            for (i = m_rgHand[index].headval - m_rgHand[index].cnt + 1; i <= m_rgHand[index].headval; i++) {
               for (k = 0; k < 3; k++) {
                  for (j = 0; j < m_iNumHandCard; j++) {
                     if (m_rgHandCard[j].GetValue() == i && !t[j]) {
                        rgDiscarded[c++] = m_rgHandCard[j];
                        t[j] = 1;
                        break;
                     }
                  }
               }
            }
            break;
         case DT_QUAD:
            printf("quad head=%d, cnt=%d\n",m_rgHand[index].headval, m_rgHand[index].cnt);

            for (i = m_rgHand[index].headval - m_rgHand[index].cnt + 1; i <= m_rgHand[index].headval; i++) {
               for (k = 0; k < 4; k++) {
                  for (j = 0; j < m_iNumHandCard; j++) {
                     if (m_rgHandCard[j].GetValue() == i && !t[j]) {
                        rgDiscarded[c++] = m_rgHandCard[j];
                        t[j] = 1;
                        break;
                     }
                  }
               }
            }
            break;
         case DT_DBLJOKER:
            printf("2jok\n");
            rgDiscarded[c++] = CCard(52);
            rgDiscarded[c++] = CCard(53);
            break;
      }
      rgDiscarded[c] = 255;
m_iNumHand--;
      return c;
   } else {
      printf("SOMETHING IS WRONG IN ANALYZEHAND!\n");
      rgDiscarded[0] = m_rgHandCard[m_iNumHandCard - 1];
      rgDiscarded[1] = 255;
   }
   return 1;
////////////////////////////////////////////////////////////////
}

int CBot::FollowCardSingle(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardDouble(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardTriple(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardTripleAt1(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardTripleAt2(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardQuadAt1(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardQuadAt2(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCardQuad(CCard rgDiscarded[20])
{
   return 0; // TODO
}

int CBot::FollowCard(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();

   // TODO
   switch (ld.type) {
      case DT_SINGLE:
         return FollowCardSingle(rgDiscarded);

      case DT_DOUBLE:
         return FollowCardDouble(rgDiscarded);

      case DT_TRIPLE:
         return FollowCardTriple(rgDiscarded);

      case DT_TRIPLEAT1:
         return FollowCardTripleAt1(rgDiscarded);

      case DT_TRIPLEAT2:
         return FollowCardTripleAt2(rgDiscarded);

      case DT_QUADAT1:
         return FollowCardQuadAt1(rgDiscarded);

      case DT_QUADAT2:
         return FollowCardQuadAt2(rgDiscarded);

      case DT_QUAD:
         return FollowCardQuad(rgDiscarded);

      case DT_DBLJOKER:
      default:
         return 0; // we can't beat 2 jokers at all
   }

   return 0;
}

void CBot::RoundBegin()
{
   m_rgiNumHandPlayed[0] = 0;
   m_rgiNumHandPlayed[1] = 0;
   m_rgiNumHandPlayed[2] = 0;
   m_pLastHandPlayed = NULL;
}

void CBot::NoBidder()
{
   // Do nothing
   // FIXME: maybe increase the desire of bidding if this happens...
}

void CBot::InvalidDiscard()
{
   // THIS SHOULD NOT HAPPEN !!! SOMETHING MUST HAVE BEEN MESSED UP.
   TerminateOnError("CBot::InvalidDiscard()");
}

void CBot::BecomeLord(int iLocation, const CCard bottomcards[3])
{
   int i;
   m_iCurLord = iLocation;
   m_rgBottomCard[0] = bottomcards[0];
   m_rgBottomCard[1] = bottomcards[1];
   m_rgBottomCard[2] = bottomcards[2];

   // initialize the remaining cards
   for (i = 0; i < 13; i++) {
      m_rgiRemainingCard[i] = 4;
   }
   m_rgiRemainingCard[13] = 1;
   m_rgiRemainingCard[14] = 1;
   for (i = 0; i < m_iNumHandCard; i++) {
      CardPlayed(m_rgHandCard[i]);
   }
}

void CBot::PlayerBidded(int iLocation, int iBid)
{
}

void CBot::PlayerDiscarded(int iLocation, const CCard rgCards[20], const CCard rgHand[20],
                           int iNumCards, int iHandLeft)
{
   RecordDiscard(iLocation, rgCards, iNumCards);
}

void CBot::ShowAllCards(int iWinnerLoc, const CCard rgRemainCards[3][20])
{
   // Do nothing
}

void CBot::ScoreBoard(const char rgszNames[3][20], int rgiGotScore[3], int rgiTotalScore[3], bool fWon)
{
   // Do nothing
}

void CBot::Turn(int iLocation)
{
   // Do nothing
}

void CBot::UpdateStatus(int base, int multiply)
{
   m_iCurPoint = base * multiply;
}

void CBot::SetPlayerNames(const char rgszNames[3][20])
{
   // Do nothing
}

bool CBot::IsReady()
{
   return true; // Bots are always ready :)
}

void CBot::RecordDiscard(int iLocation, const CCard rgCards[20], int iNumCards)
{
   for (int i = 0; i < iNumCards; i++) {
      CardPlayed(rgCards[i]);
   }
   if (iNumCards > 0) {
      m_pLastHandPlayed = &(m_rgHandPlayed[iLocation][m_rgiNumHandPlayed[iLocation]]);
      *m_pLastHandPlayed = gpGame->GetDiscardType(rgCards);
   } else {
      m_rgHandPlayed[iLocation][m_rgiNumHandPlayed[iLocation]].type = DT_INVALID;
   }
   m_rgiNumHandPlayed[iLocation]++;
}

const discardhand_t &CBot::GetLastDiscard()
{
   assert(m_pLastHandPlayed != NULL);
   return *m_pLastHandPlayed;
}

void CBot::CardPlayed(const CCard &c)
{
   int n = c.GetNum() - 1;
   if (c.IsJoker()) {
      n += 13;
   }
   m_rgiRemainingCard[n]--;
}
