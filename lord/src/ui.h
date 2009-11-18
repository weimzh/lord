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

#ifndef UI_H
#define UI_H

class CUIManager
{
public:
   CUIManager();
   ~CUIManager();

   void         OpeningUI();
   void         DrawPlayer(int location, const CCard handcard[20], int count_hc, const CCard disc[20], int count_d);
   void         DrawStatus(int base, int multip);
   void         DrawTurn(int location, bool update = true);
   void         DrawKittyCard(const CCard bcard[3]);
   void         DrawLord(int location);
   void         PlayerBidded(int location, int pt);
   int          UIBid(int iAtLeast);
   int          UIDiscard(const CCard handcard[20], int count_hc, CCard *d, bool firsthand = false);
   void         PopMessage(const char *text, int timeout = 800);
   void         NewRound();
   void         ScoreBoard(const char rgszPlayerName[3][20], int rgiGotScore[3], int rgiTotalScore[3]);

   void         SetPlayerNames(const char rgszPlayerName[3][20]);
   void         MouseClick(int x, int y);

private:
   void         DrawPlayerLocal(const CCard handcard[20], int count_hc, const CCard disc[20], int count_d);
   void         DrawPlayerLeft(const CCard handcard[20], int count_hc, const CCard disc[20], int count_d);
   void         DrawPlayerRight(const CCard handcard[20], int count_hc, const CCard disc[20], int count_d);
   void         DeleteCardImages(int location);
   void         DeleteDiscardImages(int location);
   void         DeleteTextLabel(int location);
   void         RepaintLocalCards(int first = 0);
   void         DrawPlayerNames();
   void         DrawTime(int t);

   int          MainMenu();
   int          NetGameMenu();
   void         Options();

   CCardImage       *m_rgpCardImages[3][20];
   CCardImage       *m_rgpDiscardImages[3][20];
   CTextLabelBrush  *m_rgpTextLabels[3];
   char              m_rgszPlayerName[3][20];

   unsigned int      m_iSelectedCards;
   CCard             m_rgLocalCard[20];
   int               m_iNumLocalCard;
   bool              m_fInGame;
   int               m_iCurTurn;

   CBox             *m_pFakeReadyButton, *m_pScoreBox;
   time_t            m_Time;
};

extern CUIManager g_UI;

#endif
