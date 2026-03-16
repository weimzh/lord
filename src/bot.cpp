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

//#define BOT_DEBUG 1

// Bot configuration - tune these for difficulty
#define BOT_DELAY_MIN 600
#define BOT_DELAY_MAX 1200
#define BOT_SEARCH_DEPTH 3

CBot::CBot()
{
   m_iCurLord = -1;
   m_iCurPoint = 0;
   memset(m_rgiRemainingCard, 0, sizeof(m_rgiRemainingCard));
}

CBot::~CBot()
{
}

// Calculate hand strength for bidding
int CBot::CalculateHandStrength()
{
   int i, num_cards[16], strength = 0;
   
   memset(num_cards, 0, sizeof(num_cards));
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }
   
   // Count big cards (2, Joker)
   int bigCards = num_cards[13] + num_cards[14] + num_cards[15];
   
   // Count bombs
   for (i = 0; i < 14; i++) {
      if (num_cards[i] == 4) {
         strength += 50; // bombs are very valuable
      }
   }
   
   // Count triples
   for (i = 0; i < 14; i++) {
      if (num_cards[i] == 3) {
         strength += 15;
      }
   }
   
   // Count pairs
   for (i = 0; i < 16; i++) {
      if (num_cards[i] == 2) {
         strength += 3;
      }
   }
   
   // Value big cards
   strength += bigCards * 12;
   
   // Value sequences (simplified)
   for (i = 0; i < 8; i++) {
      int seqLen = 0;
      for (int j = i; j < 12 && num_cards[j] > 0; j++) {
         seqLen++;
      }
      if (seqLen >= 5) {
         strength += (seqLen - 4) * 8;
         i += seqLen - 1;
      }
   }
   
   // Penalty for small single cards
   for (i = 0; i < 6; i++) {
      if (num_cards[i] == 1) {
         strength -= 5;
      }
   }
   
   return strength;
}

// We only like to become the declarer when:
// 1) We have 3 or more beating cards (2's or Joker's)
// 2) We have 6 or more high-ranked cards (K/A/2/Joker)
// 3) We have 2 or less leftover cards which cannot be easily got rid of
// (single cards smaller than J or pairs smaller than 8)
// TODO: may be much smarter than this...
int CBot::Bid(int iAtLeast)
{
   UTIL_Delay(RandomLong(BOT_DELAY_MIN, BOT_DELAY_MAX)); // delay a while

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
   // Consider opponent bids
   if (iAtLeast > 1 && iValue < iAtLeast) {
      // Check if we should compete
      int handStrength = CalculateHandStrength();
      if (handStrength > 40 && iAtLeast <= 2) {
         iValue = iAtLeast;
      }
   }
   
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

int CBot::PlayHand(const bothand_t &hand, CCard rgDiscarded[20])
{
   int t[20], i, j, k, c = 0;
   memset(t, 0, sizeof(t));

   switch (hand.type) {
      case DT_SINGLE:
         for (i = hand.headval - hand.cnt + 1; i <= hand.headval; i++) {
            for (j = 0; j < m_iNumHandCard; j++) {
               if (m_rgHandCard[j].GetValue() == i) {
                  rgDiscarded[c++] = m_rgHandCard[j];
                  break;
               }
            }
         }
         break;

      case DT_DOUBLE:
         for (i = hand.headval - hand.cnt + 1; i <= hand.headval; i++) {
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
         for (i = hand.headval - hand.cnt + 1; i <= hand.headval; i++) {
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
         for (i = hand.headval - hand.cnt + 1; i <= hand.headval; i++) {
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
         rgDiscarded[c++] = CCard(52);
         rgDiscarded[c++] = CCard(53);
         break;
   }

   rgDiscarded[c] = 255;
   return c;
}

// Find the smallest kicker cards
int CBot::FindKickers(CCard rgDiscarded[20], int numMain, int numKickers, int excludeValue)
{
   int i, j, count = 0;
   bool used[20];
   memset(used, 0, sizeof(used));
   
   // Mark main cards as used
   for (i = 0; i < numMain; i++) {
      for (j = 0; j < m_iNumHandCard; j++) {
         if (!used[j] && m_rgHandCard[j] == rgDiscarded[i]) {
            used[j] = true;
            break;
         }
      }
   }
   
   // Find smallest kickers
   for (i = 0; i < m_iNumHandCard && count < numKickers; i++) {
      if (!used[i] && m_rgHandCard[i].GetValue() != excludeValue) {
         rgDiscarded[numMain + count++] = m_rgHandCard[i];
         used[i] = true;
      }
   }
   
   return count;
}

int CBot::FirstHandDiscard(CCard rgDiscarded[20])
{
   int iMaxValue = -999999, iSelectedHand = -1, i, iValue;
   int iNumTriplets = 0, iNumSingles = 0, iNumPairs = 0;

   AnalyzeHand();

   for (i = 0; i < m_iNumHand; i++) {
      if (m_rgHand[i].type == DT_SINGLE && m_rgHand[i].cnt == 1) {
         iNumSingles++;
      } else if (m_rgHand[i].type == DT_DOUBLE && m_rgHand[i].cnt == 1) {
         iNumPairs++;
      } else if (m_rgHand[i].type == DT_TRIPLE) {
         iNumTriplets++;
      }
   }

   for (i = 0; i < m_iNumHand; i++) {
      iValue = 0;

      switch (m_rgHand[i].type) {
         case DT_SINGLE:
            if (m_rgHand[i].cnt > 1) {
               // sequence
               iValue = 6 * (19 - m_rgHand[i].headval) * (19 - m_rgHand[i].headval) - 80 * (iNumSingles + iNumPairs);
            } else {
               // single card
               iValue = 5 * (19 - m_rgHand[i].headval) * (19 - m_rgHand[i].headval) + 25 * iNumSingles;
            }
            break;

         case DT_DOUBLE:
            if (m_rgHand[i].cnt > 1) {
               // pair sequence
               iValue = 6 * (19 - m_rgHand[i].headval) * (19 - m_rgHand[i].headval) - 60 * (iNumSingles + iNumPairs);
            } else {
               // pair
               iValue = 5 * (19 - m_rgHand[i].headval) * (19 - m_rgHand[i].headval) + 25 * iNumPairs + 10;
            }
            break;

         case DT_TRIPLE:
            iValue = 2 * (19 - m_rgHand[i].headval) * (19 - m_rgHand[i].headval);
            if (iNumTriplets - iNumSingles - iNumPairs <= 2) {
               iValue += 850; // play triplets first if we don't have much leftovers
            }
            break;

         case DT_QUAD:
            iValue = (19 - m_rgHand[i].headval) / 2;
            break;

         case DT_DBLJOKER:
         default:
            if (m_iNumHand <= 2) {
               iValue = 999999;
            } else {
               iValue = 1;
            }
            break;
      }

      if (iValue > iMaxValue) {
         iMaxValue = iValue;
         iSelectedHand = i;
      }
   }

   assert(iSelectedHand != -1);

   int c = PlayHand(m_rgHand[iSelectedHand], rgDiscarded);
   
   // Add kicker cards for triples and quads
   if (m_rgHand[iSelectedHand].type == DT_TRIPLE && c < 20) {
      // Play with smallest kicker
      int kickerVal = 999;
      for (i = 0; i < m_iNumHandCard; i++) {
         bool isMain = false;
         for (int j = 0; j < c; j++) {
            if (m_rgHandCard[i].GetValue() == rgDiscarded[j].GetValue()) {
               isMain = true;
               break;
            }
         }
         if (!isMain && m_rgHandCard[i].GetValue() < kickerVal) {
            kickerVal = m_rgHandCard[i].GetValue();
         }
      }
      if (kickerVal != 999) {
         for (i = 0; i < m_iNumHandCard && c < 20; i++) {
            if (m_rgHandCard[i].GetValue() == kickerVal) {
               rgDiscarded[c++] = m_rgHandCard[i];
               break;
            }
         }
      }
   } else if (m_rgHand[iSelectedHand].type == DT_QUAD && c < 20) {
      // Play with two smallest kickers
      int kickers[2] = {999, 999};
      for (i = 0; i < m_iNumHandCard; i++) {
         bool isMain = false;
         for (int j = 0; j < c; j++) {
            if (m_rgHandCard[i].GetValue() == rgDiscarded[j].GetValue()) {
               isMain = true;
               break;
            }
         }
         if (!isMain) {
            int val = m_rgHandCard[i].GetValue();
            if (val < kickers[0]) {
               kickers[1] = kickers[0];
               kickers[0] = val;
            } else if (val < kickers[1]) {
               kickers[1] = val;
            }
         }
      }
      for (int k = 0; k < 2; k++) {
         if (kickers[k] != 999) {
            for (i = 0; i < m_iNumHandCard && c < 20; i++) {
               if (m_rgHandCard[i].GetValue() == kickers[k]) {
                  rgDiscarded[c++] = m_rgHandCard[i];
                  break;
               }
            }
         }
      }
   }
   
   rgDiscarded[c] = 255;
   return c;
}

// Helper: find smallest card that can beat the target
int CBot::FindSmallestBeatingCard(int minValue, int excludeValue)
{
   int i, bestVal = 999;
   for (i = 0; i < m_iNumHandCard; i++) {
      int val = m_rgHandCard[i].GetValue();
      if (val > minValue && val < bestVal && val != excludeValue) {
         bestVal = val;
      }
   }
   return (bestVal == 999) ? -1 : bestVal;
}

// Helper: find smallest pair that can beat the target
int CBot::FindSmallestBeatingPair(int minValue)
{
   int i, num_cards[16];
   memset(num_cards, 0, sizeof(num_cards));
   
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }
   
   for (i = 0; i < 16; i++) {
      if (num_cards[i] >= 2 && (i + 3) > minValue) {
         return i + 3;
      }
   }
   return -1;
}

// Helper: find smallest triple that can beat the target
int CBot::FindSmallestBeatingTriple(int minValue)
{
   int i, num_cards[16];
   memset(num_cards, 0, sizeof(num_cards));
   
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }
   
   for (i = 0; i < 14; i++) {
      if (num_cards[i] >= 3 && (i + 3) > minValue) {
         return i + 3;
      }
   }
   return -1;
}

// Helper: check if we have a bomb
bool CBot::HasBomb()
{
   int i, num_cards[16];
   memset(num_cards, 0, sizeof(num_cards));
   
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }
   
   for (i = 0; i < 14; i++) {
      if (num_cards[i] == 4) {
         return true;
      }
   }
   return false;
}

// Helper: find smallest bomb
int CBot::FindSmallestBomb()
{
   int i, num_cards[16];
   memset(num_cards, 0, sizeof(num_cards));
   
   for (i = 0; i < m_iNumHandCard; i++) {
      num_cards[m_rgHandCard[i].GetValue() - 3]++;
   }
   
   for (i = 0; i < 14; i++) {
      if (num_cards[i] == 4) {
         return i + 3;
      }
   }
   return -1;
}

int CBot::FollowCardSingle(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int i, j;
   
   if (ld.cnt == 1) {
      // Single card
      int beatVal = FindSmallestBeatingCard(ld.head.GetValue(), -1);
      if (beatVal != -1) {
         // Check if we should save big cards
         bool isLord = IsLord();
         bool shouldPlay = true;
         
         // Don't waste 2 or Joker unless necessary
         if (!isLord && beatVal >= 13 && m_iNumHandCard > 5) {
            // Check if we have smaller bombs or can pass
            if (!HasBomb() && beatVal > 14) {
               shouldPlay = false;
            }
         }
         
         if (shouldPlay) {
            for (i = 0; i < m_iNumHandCard; i++) {
               if (m_rgHandCard[i].GetValue() == beatVal) {
                  rgDiscarded[0] = m_rgHandCard[i];
                  rgDiscarded[1] = 255;
                  return 1;
               }
            }
         }
      }
   } else {
      // Sequence - find matching sequence
      int num_cards[16];
      memset(num_cards, 0, sizeof(num_cards));
      
      for (i = 0; i < m_iNumHandCard; i++) {
         num_cards[m_rgHandCard[i].GetValue() - 3]++;
      }
      
      // Try to find a sequence of same length that beats it
      for (i = ld.head.GetValue() - ld.cnt + 1; i < 12; i++) {
         bool canForm = true;
         for (j = 0; j < ld.cnt; j++) {
            if (num_cards[i + j - 3] < 1) {
               canForm = false;
               break;
            }
         }
         if (canForm && (i + ld.cnt - 1) > ld.head.GetValue()) {
            // Found a beating sequence
            int c = 0;
            for (j = 0; j < ld.cnt; j++) {
               for (int k = 0; k < m_iNumHandCard; k++) {
                  if (m_rgHandCard[k].GetValue() == (i + j)) {
                     rgDiscarded[c++] = m_rgHandCard[k];
                     break;
                  }
               }
            }
            rgDiscarded[c] = 255;
            return c;
         }
      }
   }
   
   return 0;
}

int CBot::FollowCardDouble(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int i, j;
   
   if (ld.cnt == 1) {
      // Single pair
      int beatVal = FindSmallestBeatingPair(ld.head.GetValue());
      if (beatVal != -1) {
         int c = 0;
         for (i = 0; i < m_iNumHandCard && c < 2; i++) {
            if (m_rgHandCard[i].GetValue() == beatVal) {
               rgDiscarded[c++] = m_rgHandCard[i];
            }
         }
         rgDiscarded[c] = 255;
         return c;
      }
   } else {
      // Pair sequence - find matching pair sequence
      int num_cards[16];
      memset(num_cards, 0, sizeof(num_cards));
      
      for (i = 0; i < m_iNumHandCard; i++) {
         num_cards[m_rgHandCard[i].GetValue() - 3]++;
      }
      
      // Try to find a pair sequence of same length that beats it
      for (i = ld.head.GetValue() - ld.cnt + 1; i < 12; i++) {
         bool canForm = true;
         for (j = 0; j < ld.cnt; j++) {
            if (num_cards[i + j - 3] < 2) {
               canForm = false;
               break;
            }
         }
         if (canForm && (i + ld.cnt - 1) > ld.head.GetValue()) {
            // Found a beating pair sequence
            int c = 0;
            for (j = 0; j < ld.cnt; j++) {
               for (int k = 0; k < 2; k++) {
                  for (int m = 0; m < m_iNumHandCard; m++) {
                     if (m_rgHandCard[m].GetValue() == (i + j) && 
                         (c == 0 || rgDiscarded[c-1].GetValue() != (i + j))) {
                        rgDiscarded[c++] = m_rgHandCard[m];
                     }
                  }
               }
            }
            rgDiscarded[c] = 255;
            return c;
         }
      }
   }
   
   return 0;
}

int CBot::FollowCardTriple(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int i, j, k;
   
   // Find a triple that beats the opponent's triple
   int beatVal = FindSmallestBeatingTriple(ld.head.GetValue());
   if (beatVal != -1) {
      int c = 0;
      // Add the triple
      for (i = 0; i < m_iNumHandCard && c < 3; i++) {
         if (m_rgHandCard[i].GetValue() == beatVal) {
            rgDiscarded[c++] = m_rgHandCard[i];
         }
      }
      
      // Add kickers if needed (match the opponent's kicker pattern)
      int kickerCount = ld.cnt - 1;
      if (kickerCount > 0) {
         // Find smallest kickers
         bool used[20];
         memset(used, 0, sizeof(used));
         
         // Mark triple cards as used
         for (i = 0; i < c; i++) {
            for (j = 0; j < m_iNumHandCard; j++) {
               if (!used[j] && m_rgHandCard[j] == rgDiscarded[i]) {
                  used[j] = true;
                  break;
               }
            }
         }
         
         // Find smallest kickers
         for (i = 0; i < m_iNumHandCard && kickerCount > 0; i++) {
            if (!used[i]) {
               rgDiscarded[c++] = m_rgHandCard[i];
               used[i] = true;
               kickerCount--;
            }
         }
      }
      
      rgDiscarded[c] = 255;
      return c;
   }
   
   return 0;
}

int CBot::FollowCardTripleAt1(CCard rgDiscarded[20])
{
   // Triple with single kicker
   return FollowCardTriple(rgDiscarded);
}

int CBot::FollowCardTripleAt2(CCard rgDiscarded[20])
{
   // Triple with pair kicker
   return FollowCardTriple(rgDiscarded);
}

int CBot::FollowCardQuadAt1(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int i, j;
   
   // Find smallest bomb that beats it
   int beatVal = FindSmallestBomb();
   if (beatVal != -1 && beatVal > ld.head.GetValue()) {
      int c = 0;
      for (i = 0; i < m_iNumHandCard && c < 4; i++) {
         if (m_rgHandCard[i].GetValue() == beatVal) {
            rgDiscarded[c++] = m_rgHandCard[i];
         }
      }
      
      // Add two single kickers
      bool used[20];
      memset(used, 0, sizeof(used));
      
      for (i = 0; i < c; i++) {
         for (j = 0; j < m_iNumHandCard; j++) {
            if (!used[j] && m_rgHandCard[j] == rgDiscarded[i]) {
               used[j] = true;
               break;
            }
         }
      }
      
      for (i = 0; i < m_iNumHandCard && c < 6; i++) {
         if (!used[i]) {
            rgDiscarded[c++] = m_rgHandCard[i];
            used[i] = true;
         }
      }
      
      rgDiscarded[c] = 255;
      return c;
   }
   
   return 0;
}

int CBot::FollowCardQuadAt2(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int i, j;
   
   // Find smallest bomb that beats it
   int beatVal = FindSmallestBomb();
   if (beatVal != -1 && beatVal > ld.head.GetValue()) {
      int c = 0;
      for (i = 0; i < m_iNumHandCard && c < 4; i++) {
         if (m_rgHandCard[i].GetValue() == beatVal) {
            rgDiscarded[c++] = m_rgHandCard[i];
         }
      }
      
      // Add two pair kickers
      bool used[20];
      memset(used, 0, sizeof(used));
      
      for (i = 0; i < c; i++) {
         for (j = 0; j < m_iNumHandCard; j++) {
            if (!used[j] && m_rgHandCard[j] == rgDiscarded[i]) {
               used[j] = true;
               break;
            }
         }
      }
      
      // Find pairs
      int num_cards[16];
      memset(num_cards, 0, sizeof(num_cards));
      for (i = 0; i < m_iNumHandCard; i++) {
         if (!used[i]) {
            num_cards[m_rgHandCard[i].GetValue() - 3]++;
         }
      }
      
      int pairsFound = 0;
      for (i = 0; i < 16 && pairsFound < 2; i++) {
         if (num_cards[i] >= 2) {
            for (j = 0; j < m_iNumHandCard && c < 8; j++) {
               if (!used[j] && m_rgHandCard[j].GetValue() == (i + 3)) {
                  rgDiscarded[c++] = m_rgHandCard[j];
                  used[j] = true;
               }
            }
            pairsFound++;
         }
      }
      
      rgDiscarded[c] = 255;
      return c;
   }
   
   return 0;
}

int CBot::FollowCardQuad(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int i, j;
   
   // Find smallest bomb that beats it
   int beatVal = FindSmallestBomb();
   if (beatVal != -1 && beatVal > ld.head.GetValue()) {
      int c = 0;
      for (i = 0; i < m_iNumHandCard && c < 4; i++) {
         if (m_rgHandCard[i].GetValue() == beatVal) {
            rgDiscarded[c++] = m_rgHandCard[i];
         }
      }
      rgDiscarded[c] = 255;
      return c;
   }
   
   return 0;
}

int CBot::FollowCard(CCard rgDiscarded[20])
{
   const discardhand_t &ld = GetLastDiscard();
   int result = 0;

   switch (ld.type) {
      case DT_SINGLE:
         result = FollowCardSingle(rgDiscarded);
         break;

      case DT_DOUBLE:
         result = FollowCardDouble(rgDiscarded);
         break;

      case DT_TRIPLE:
         result = FollowCardTriple(rgDiscarded);
         break;

      case DT_TRIPLEAT1:
         result = FollowCardTripleAt1(rgDiscarded);
         break;

      case DT_TRIPLEAT2:
         result = FollowCardTripleAt2(rgDiscarded);
         break;

      case DT_QUADAT1:
         result = FollowCardQuadAt1(rgDiscarded);
         break;

      case DT_QUADAT2:
         result = FollowCardQuadAt2(rgDiscarded);
         break;

      case DT_QUAD:
         result = FollowCardQuad(rgDiscarded);
         break;

      case DT_DBLJOKER:
      default:
         return 0; // we can't beat 2 jokers at all
   }
   
   // Strategic considerations
   if (result > 0) {
      bool isLord = IsLord();
      
      // If we're lord and have few cards left, play aggressively
      if (isLord && m_iNumHandCard - result <= 5) {
         return result;
      }
      
      // If we're not lord and opponent has few cards, consider using bomb
      if (!isLord && m_rgiNumHandPlayed[PLAYER_LEFT] + m_rgiNumHandPlayed[PLAYER_RIGHT] > 10) {
         // Opponent might be close to winning, consider bomb
         if (ld.type < DT_QUAD && HasBomb()) {
            // Try bomb
            int bombVal = FindSmallestBomb();
            if (bombVal != -1) {
               int i, c = 0;
               for (i = 0; i < m_iNumHandCard && c < 4; i++) {
                  if (m_rgHandCard[i].GetValue() == bombVal) {
                     rgDiscarded[c++] = m_rgHandCard[i];
                  }
               }
               rgDiscarded[c] = 255;
               return c;
            }
         }
      }
   }
   
   return result;
}

void CBot::RoundBegin()
{
   m_rgiNumHandPlayed[0] = 0;
   m_rgiNumHandPlayed[1] = 0;
   m_rgiNumHandPlayed[2] = 0;
   m_pLastHandPlayed = NULL;
   memset(m_rgiRemainingCard, 4, sizeof(m_rgiRemainingCard));
   m_rgiRemainingCard[13] = 1;
   m_rgiRemainingCard[14] = 1;
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
   // Track opponent's bid strength
}

void CBot::PlayerDiscarded(int iLocation, const CCard rgCards[20], const CCard rgHand[20],
                           int iNumCards, int iHandLeft)
{
   RecordDiscard(iLocation, rgCards, iNumCards);
   
   // Update card counting
   if (iHandLeft <= 5) {
      // Opponent is close to winning - be more aggressive
   }
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
   if (n >= 0 && n < 15) {
      m_rgiRemainingCard[n]--;
   }
}
