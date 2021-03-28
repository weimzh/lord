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

CUIManager g_UI;
bool g_fLocalReady = true;

CUIManager::CUIManager()
{
   int i, j;
   for (i = 0; i < 3; i++) {
      for (j = 0; j < 20; j++) {
         m_rgpCardImages[i][j] = NULL;
         m_rgpDiscardImages[i][j] = NULL;
      }
      m_rgpTextLabels[i] = NULL;
   }
   m_iSelectedCards = 0;
   m_iNumLocalCard = 0;
   m_fInGame = false;
   m_Time = -1;
}

CUIManager::~CUIManager()
{
   // We don't need to free anything by hand because the OS will do so
   // when the program quits.
}

void CUIManager::OpeningUI()
{
   while (1) {
      int m = MainMenu();
      switch (m) {
         case 1: // single player game
            gpGame->RunSingleGame();
            break;
         case 2: // multiplayer game
#if 0
            m = NetGameMenu();
            if (m == 1) {
               gpGame->RunNetGame();
            } else if (m == 2) {
               RunClient(cfg.Get("GAME", "HostName", ""),
                         cfg.Get("GAME", "PlayerName", "Player"),
                         cfg.Get("GAME", "ServerPwd", ""));
            }
#endif
            break;
         case 3: // options
            Options();
            break;
      }
   }
}

int CUIManager::MainMenu()
{
   gpGeneral->ClearScreen(false, false, true);

   CBox box(150, 200, 340, 240, 200, 50, 170);
   gpGeneral->DrawTextBrush("MAIN MENU", 205, 205, 255, 255, 50, 50);
   CButton btnSingleGame(1, 200, 265, 240, 30, 150, 20, 230);
#if 0
   CButton btnNetGame(2, 200, 305, 240, 30, 150, 20, 230);
#endif
   CButton btnOptions(3, 200, 345, 240, 30, 150, 20, 230);
   CButton btnQuit(4, 200, 385, 240, 30, 150, 20, 230);

   const char *szSingleGame = msg("menu_singlegame");
#if 0
   const char *szNetGame = msg("menu_netgame");
#endif
   const char *szOptions = msg("menu_options");
   const char *szQuit = msg("menu_quit");
   const char *szCopyright = msg("copyright");

   gpGeneral->DrawText(szSingleGame, 320 - strlen(szSingleGame) * 4, 272, 255, 255, 50);
#if 0
   gpGeneral->DrawText(szNetGame, 320 - strlen(szNetGame) * 4, 312, 255, 255, 50);
#endif
   gpGeneral->DrawText(szOptions, 320 - strlen(szOptions) * 4, 352, 255, 255, 50);
   gpGeneral->DrawText(szQuit, 320 - strlen(szQuit) * 4, 392, 255, 255, 50);
   gpGeneral->DrawText(szCopyright, 320 - strlen(szCopyright) * 4, 460, 255, 255, 255);

   while (1) {
      int key = gpGeneral->ReadKey() - SDLK_LAST;
      switch (key) {
         case 1:
         case 2:
         case 3:
            return key;
            break;
         case 4:
            UserQuit();
            break;
      }
   }
}

int CUIManager::NetGameMenu()
{
   CBox box(100, 100, 440, 280, 200, 50, 170);
   CTextBox playerName(180, 120, 20, cfg.Get("GAME", "PlayerName", "Player"));
   gpGeneral->DrawText(msg("menu_playername"), 110, 120, 255, 250, 50);
   CTextBox pwd(180, 150, 20, cfg.Get("GAME", "ServerPwd", ""));
   gpGeneral->DrawText(msg("menu_password"), 110, 150, 255, 250, 50);
   CTextBox hostName(180, 180, 20, cfg.Get("GAME", "HostName", ""));
   gpGeneral->DrawText(msg("menu_hostaddr"), 110, 180, 255, 250, 50);

   CButton btnServer(1, 110, 350, 100, 20, 50, 220, 200);
   gpGeneral->DrawText(msg("menu_server"), 160 - strlen(msg("menu_server")) * 4, 352, 255, 250, 50);
   CButton btnClient(2, 220, 350, 100, 20, 50, 220, 200);
   gpGeneral->DrawText(msg("menu_client"), 270 - strlen(msg("menu_client")) * 4, 352, 255, 250, 50);
   CButton btnCancel(3, 330, 350, 100, 20, 50, 220, 200);
   gpGeneral->DrawText(msg("menu_cancel"), 380 - strlen(msg("menu_cancel")) * 4, 352, 255, 250, 50);

   while (1) {
      int key = gpGeneral->ReadKey() - SDLK_LAST;
      if (key == 1) {
         // SERVER button pressed
         if (strlen(playerName.Text()) > 0) {
            cfg.Set("GAME", "PlayerName", playerName.Text());
         }
         cfg.Set("GAME", "ServerPwd", pwd.Text());
         gpGeneral->DrawText(msg("connecting"), 0, 440, 255, 255, 0);
         return 1;
      } else if (key == 2) {
         // CLIENT button pressed
         if (strlen(hostName.Text()) <= 0) {
            continue;
         }
         if (strlen(playerName.Text()) > 0) {
            cfg.Set("GAME", "PlayerName", playerName.Text());
         }
         cfg.Set("GAME", "HostName", hostName.Text());
         cfg.Set("GAME", "ServerPwd", pwd.Text());
         gpGeneral->DrawText(msg("connecting"), 0, 440, 255, 255, 0);
         return 2;
      } else if (key == 3) {
         // Cancel button pressed
         return 0; // do nothing
      }
   }
   return 0;
}

void CUIManager::Options()
{
   CBox box(100, 100, 440, 280, 200, 50, 170);

   CTextBox playerName(180, 120, 20, cfg.Get("GAME", "PlayerName", "Player"));
   gpGeneral->DrawText(msg("menu_playername"), 110, 120, 255, 250, 50);
   CCheckBox cbSound(110, 150, !atoi(cfg.Get("OPTIONS", "NoSound", "0")));
   gpGeneral->DrawText(msg("menu_sound"), 130, 150, 255, 250, 50);
   CCheckBox cbShowCards(110, 180, (atoi(cfg.Get("GAME", "ShowCards", "1")) != 0));
   gpGeneral->DrawText(msg("menu_showcards"), 130, 180, 255, 250, 50);

   CButton btnOK(1, 110, 350, 100, 20, 50, 220, 200);
   gpGeneral->DrawText(msg("menu_ok"), 160 - strlen(msg("menu_ok")) * 4, 352, 255, 250, 50);
   CButton btnCancel(2, 220, 350, 100, 20, 50, 220, 200);
   gpGeneral->DrawText(msg("menu_cancel"), 270 - strlen(msg("menu_cancel")) * 4, 352, 255, 250, 50);

   while (1) {
      int key = gpGeneral->ReadKey() - SDLK_LAST;
      if (key == 1) {
         // OK button pressed
         if (strlen(playerName.Text()) > 0) {
            cfg.Set("GAME", "PlayerName", playerName.Text());
         }
         cfg.Set("OPTIONS", "NoSound", cbSound.Checked() ? "0" : "1");
         cfg.Set("GAME", "ShowCards", cbShowCards.Checked() ? "1" : "0");
         g_fNoSound = !cbSound.Checked();
         return;
      } else if (key == 2) {
         // Cancel button pressed
         return; // do nothing
      }
   }
}

void CUIManager::DrawPlayer(int location, const CCard handcard[20], int count_hc,
                            const CCard disc[20], int count_d)
{
   switch (location) {
      case PLAYER_LOCAL:
         m_Time = -1;
         DrawPlayerLocal(handcard, count_hc, disc, count_d);
         break;

      case PLAYER_LEFT:
         DrawPlayerLeft(handcard, count_hc, disc, count_d);
         break;

      case PLAYER_RIGHT:
         DrawPlayerRight(handcard, count_hc, disc, count_d);
         break;

      default:
         TerminateOnError("CUIManager::DrawPlayer(): Unknown location %d", location);
         break;
   }
}

void CUIManager::DrawPlayerLocal(const CCard handcard[20], int count_hc,
                                 const CCard disc[20], int count_d)
{
   int top, i;
   top = 300 - 15 * count_hc / 2;
   DeleteDiscardImages(PLAYER_LOCAL);
   DeleteCardImages(PLAYER_LOCAL);
   m_iSelectedCards = 0;
   m_fInGame = true;
   m_iNumLocalCard = count_hc;

   for (i = 0; i < count_hc; i++) {
      m_rgLocalCard[i] = handcard[i];
   }

   RepaintLocalCards(0);

   if (count_d == 0) {
      m_rgpTextLabels[PLAYER_LOCAL] = new CTextLabelBrush("PASS", 270, 285, 50);
   } else {
      top = 300 - 15 * count_d / 2;
      for (i = 0; i < count_d; i++) {
         m_rgpDiscardImages[PLAYER_LOCAL][i] = new CCardImage(disc[i], top + 15 * i,
            270, -1, true);
      }
   }
   if (m_rgpTextLabels[PLAYER_RIGHT] != NULL && m_rgpTextLabels[PLAYER_LEFT] != NULL) {
      DeleteTextLabel(PLAYER_LEFT);
      DeleteTextLabel(PLAYER_RIGHT);
   }
   gpGeneral->UpdateScreen(); // FIXME: only update changed parts
}

void CUIManager::DrawPlayerRight(const CCard handcard[20], int count_hc,
                                 const CCard disc[20], int count_d)
{
   int top, i;
   top = 180 - count_hc * 15 / 2;
   DeleteDiscardImages(PLAYER_RIGHT);
   DeleteCardImages(PLAYER_RIGHT);
   for (i = 0; i < count_hc; i++) {
      m_rgpCardImages[PLAYER_RIGHT][i] = new CCardImage(handcard == NULL ? 255 : handcard[i],
         540, top + 15 * i, -1, true);
   }
   if (count_d == 0) {
      m_rgpTextLabels[PLAYER_RIGHT] = new CTextLabelBrush("PASS", 420, 175, 50);
   } else {
      top = 530;
      if (count_d > 10 || m_rgpDiscardImages[PLAYER_LEFT][10] != NULL) {
         if (count_d >= 15 || m_rgpDiscardImages[PLAYER_LEFT][14] != NULL) {
            DeleteDiscardImages(PLAYER_LEFT);
         }
         top = 580;
      }
      for (i = 0; i < count_d; i++) {
         m_rgpDiscardImages[PLAYER_RIGHT][i] = new CCardImage(disc[i], top - 15 * (count_d - i) - 55,
            170, -1, true);
      }
   }
   if (m_rgpTextLabels[PLAYER_LEFT] != NULL && m_rgpTextLabels[PLAYER_LOCAL] != NULL) {
      DeleteTextLabel(PLAYER_LOCAL);
      DeleteTextLabel(PLAYER_LEFT);
   }
   gpGeneral->UpdateScreen(); // FIXME: only update changed parts
}

void CUIManager::DrawPlayerLeft(const CCard handcard[20], int count_hc,
                                const CCard disc[20], int count_d)
{
   int top, i;
   top = 180 - count_hc * 15 / 2;
   DeleteDiscardImages(PLAYER_LEFT);
   DeleteCardImages(PLAYER_LEFT);
   for (i = 0; i < count_hc; i++) {
      m_rgpCardImages[PLAYER_LEFT][i] = new CCardImage(handcard == NULL ? 255 : handcard[i],
      40, top + 15 * i, -1, true);
   }
   if (count_d == 0) {
      m_rgpTextLabels[PLAYER_LEFT] = new CTextLabelBrush("PASS", 120, 175, 50);
   } else {
      top = 110;
      if (count_d > 10 || m_rgpDiscardImages[PLAYER_RIGHT][10] != NULL) {
         if (count_d >= 15 || m_rgpDiscardImages[PLAYER_RIGHT][14] != NULL) {
            DeleteDiscardImages(PLAYER_RIGHT);
         }
         top = 50;
      }
      for (i = 0; i < count_d; i++) {
         m_rgpDiscardImages[PLAYER_LEFT][i] = new CCardImage(disc[i], top + 15 * i,
            170, -1, true);
      }
   }
   if (m_rgpTextLabels[PLAYER_RIGHT] != NULL && m_rgpTextLabels[PLAYER_LOCAL] != NULL) {
      DeleteTextLabel(PLAYER_LOCAL);
      DeleteTextLabel(PLAYER_RIGHT);
   }
   gpGeneral->UpdateScreen(); // FIXME: only update changed parts
}

void CUIManager::DrawTurn(int location, bool update)
{
   m_iCurTurn = location;
   gpGeneral->EraseArea(290, 105, 60, 60, true);
   gpGeneral->DrawImage(IMG_PANEL, 290, 105, false);
   int loc[] = {IMG_LOCAL, IMG_LEFT, IMG_RIGHT};
   gpGeneral->DrawImage(loc[location], 290, 105, false);
   DeleteTextLabel(location);
   if (update) {
      gpGeneral->UpdateScreen(290, 105, 60, 60);
   }
}

void CUIManager::DrawTime(int t)
{
   char buf[3];
   if (t < 0) {
      t = 0;
   } else if (t > 99) {
      t = 99;
   }
   sprintf(buf, "%.2d", t);
   DrawTurn(m_iCurTurn, false);
   gpGeneral->DrawText(buf, 312, 127, 255, 0, 0);
   gpGeneral->UpdateScreen(312, 127, 16, 16);
}

void CUIManager::DrawKittyCard(const CCard bcard[3])
{
   int i;
   for (i = 0; i < 3; i++) {
      gpGeneral->DrawCard(bcard == NULL ? 255 : bcard[i],
         230 + 60 * i, 20);
   }
}

void CUIManager::DrawStatus(int base, int times)
{
   gpGeneral->EraseArea(0, 452, 640, 20, true);
   char *sz = va("%s - %s: %d,  %s: %d", m_rgszPlayerName[0],
      msg("basept"), base, msg("multiply"), times);
   gpGeneral->DrawText(sz, 320 - strlen(sz) * 4, 460, 255, 255, 255);
   gpGeneral->UpdateScreen(0, 440, 640, 20);
}

void CUIManager::DrawLord(int location)
{
   const int t[3][2] = {{98, 420}, // PLAYER_LOCAL
                        {115, 95}, // PLAYER_LEFT
                        {640 - 115 - 35, 95}}; // PLAYER_RIGHT
   gpGeneral->DrawImage(IMG_LORD, t[location][0], t[location][1]);
}

int CUIManager::UIBid(int iAtLeast)
{
   CBox box(200, 260, 225, 32, 80, 50, 200, 128, false);
   CButton btn1(1, 205, 265, 50, 22, 250, 70, 250);
   CButton btn2(2, 260, 265, 50, 22, 250, 70, 250);
   CButton btn3(3, 315, 265, 50, 22, 250, 70, 250);
   CButton btn4(0, 370, 265, 50, 22, 250, 70, 250);

   gpGeneral->DrawImage(iAtLeast > 1 ? IMG_1PD : IMG_1P, 210, 267);
   gpGeneral->DrawImage(iAtLeast > 2 ? IMG_2PD : IMG_2P, 265, 267);
   gpGeneral->DrawImage(IMG_3P, 320, 267);
   gpGeneral->DrawImage(IMG_NOTCALL, 375, 267);

   time_t t;
   time(&t);

   while (time(NULL) - t < 30) {
      DrawTime(30 - (time(NULL) - t));
      int key = gpGeneral->ReadKey(false) - SDLK_LAST;
      if ((key >= iAtLeast && key <= 3) || !key) {
         return key;
      }
      // allow using the keyboard as well
      key += SDLK_LAST - '0';
      if ((key >= iAtLeast && key <= 3) || !key) {
         return key;
      }
   }

   return 0;
}

// Return the number of the discarded cards, and store the discarded cards into d.
int CUIManager::UIDiscard(const CCard handcard[20], int count_hc, CCard *d, bool firsthand)
{
   int i, k;

   DeleteDiscardImages(PLAYER_LOCAL);
   gpGeneral->UpdateScreen();

   i = firsthand ? 287 : 260;
   k = firsthand ? 60 : 115;
   CButton btnOK(2, i, 315, 50, 22, 250, 70, 250);
   CButton *btnPass = firsthand ? NULL : new CButton(3, 315, 315, 50, 22, 250, 70, 250);
   gpGeneral->DrawImage(IMG_DISCARD, i + 5, 317);
   if (!firsthand) {
      gpGeneral->DrawImage(IMG_ABORT, 320, 317);
   }

   if (m_Time < 0) {
      time(&m_Time);
   }

   while (time(NULL) - m_Time < 30) {
      DrawTime(30 - (time(NULL) - m_Time));
      int k = gpGeneral->ReadKey(false) - SDLK_LAST;
      if (k == 2 || k == SDLK_RIGHT - SDLK_LAST) {
         // OK
         CCard *p = d;
         for (i = 0; i < count_hc; i++) {
            if (m_iSelectedCards & (1 << i)) {
               *p++ = handcard[i];
            }
         }
         if (p == d) {
            continue; // no cards selected
         }
         if (btnPass != NULL) {
            delete btnPass;
         }
         *p = 255;
         return p - d;
      } else if (k == 3) {
         // Pass
         delete btnPass;
         *d = 255;
         return 0;
      }
   }

   if (firsthand) {
      *d = handcard[0];
      *(d + 1) = 255;
      return 1;
   }

   delete btnPass;
   *d = 255;
   return 0;
}

void CUIManager::PlayerBidded(int location, int pt)
{
   const int xy[3][2] = {{320, 300}, {220, 240}, {420, 240}};
   gpGeneral->DrawImage(IMG_BID, xy[location][0] - 65 / 2, xy[location][1] - 65 / 2);
   gpGeneral->DrawText(!pt ? msg("not_call") : va(msg("point"), pt),
      xy[location][0] - 65 / 2 + 3, xy[location][1] - 65 / 2 + 16, 20, 20, 20);
   gpGeneral->UpdateScreen(xy[location][0] - 65 / 2, xy[location][1] - 65 / 2, 65, 65);
   const int sounds[4] = {SOUND_PASS, SOUND_HINT, SOUND_HINT, SOUND_BID};
   gpGeneral->PlaySound(sounds[pt]);
}

void CUIManager::PopMessage(const char *text, int timeout)
{
   CBox box(150, 300, 340, 26, 20, 50, 120, false);
   gpGeneral->DrawText(text, 320 - strlen(text) * 4, 305, 255, 255, 50);
   gpGeneral->UpdateScreen(150, 300, 340, 36);
   UTIL_Delay(timeout);
}

void CUIManager::NewRound()
{
   int i;
   for (i = 0; i < 3; i++) {
      DeleteCardImages(i);
      DeleteDiscardImages(i);
   }
   gpGeneral->ClearScreen(true, true, true);
   gpGeneral->PlaySound(SOUND_HINT2);
   DrawPlayerNames();
   DrawStatus(0, 0);
}

void CUIManager::ScoreBoard(const char rgszPlayerName[3][20], int rgiGotScore[3], int rgiTotalScore[3])
{
   UTIL_Delay(500); // delay a while
   int i;
   m_pScoreBox = new CBox(150, 150, 340, 150, 200, 20, 200);
   gpGeneral->DrawText(msg("got_pt_this_round"), 330, 160, 255, 255, 0);
   gpGeneral->DrawText(msg("total_pt"), 410, 160, 255, 255, 0);
   for (i = 0; i < 3; i++) {
      gpGeneral->DrawText(rgszPlayerName[i], 160, 190 + i * 20, 255, 255, 255);
      gpGeneral->DrawText(va("%8d", rgiGotScore[i]), 330, 190 + i * 20, 255, 255, 255);
      gpGeneral->DrawText(va("%8d", rgiTotalScore[i]), 410, 190 + i * 20, 255, 255, 255);
   }
   const char *pszContinue = msg("continue");
   m_pFakeReadyButton = new CBox(300, 260, 50, 22, 25, 70, 250);
   gpGeneral->DrawText(pszContinue, 325 - strlen(pszContinue) * 4, 263, 255, 255, 0);
   m_fInGame = false;
   g_fLocalReady = false;
}

void CUIManager::SetPlayerNames(const char rgszPlayerName[3][20])
{
   memcpy(m_rgszPlayerName, rgszPlayerName, sizeof(m_rgszPlayerName));
}

void CUIManager::MouseClick(int x, int y)
{
   if (!m_fInGame) {
      // not in game
      if (!g_fLocalReady && x > 300 && x < 350 && y > 260 && y < 282) {
         delete m_pFakeReadyButton;
         delete m_pScoreBox;
         g_fLocalReady = true;
      }
   }
   int num = x - 300 + 15 * m_iNumLocalCard / 2, top = 365;
   num /= 15;
   if (num - m_iNumLocalCard >= 0 && num - m_iNumLocalCard <= 2) {
      num = m_iNumLocalCard - 1;
   }
   if (num < 0 || num >= m_iNumLocalCard) {
      return; // not clicking on cards
   }
   if (m_iSelectedCards & (1 << num)) {
      top -= 10;
   }
   if (y < top || y > top + 84) {
      return; // not clicking on cards
   }
   m_iSelectedCards ^= (1 << num);
   RepaintLocalCards(num);
}

void CUIManager::RepaintLocalCards(int first)
{
   int left = 300 - 15 * m_iNumLocalCard / 2, i;

   for (i = m_iNumLocalCard - 1; i >= first; i--) {
      if (m_rgpCardImages[PLAYER_LOCAL][i] != NULL) {
         delete m_rgpCardImages[PLAYER_LOCAL][i];
      }
   }

   for (i = first; i < m_iNumLocalCard; i++) {
      m_rgpCardImages[PLAYER_LOCAL][i] = new CCardImage(m_rgLocalCard[i], left + 15 * i,
         (m_iSelectedCards & (1 << i)) ? 355: 365, -1, true);
      if (m_iSelectedCards & (1 << i)) {
         UTIL_Rect(gpScreen, left + 15 * i, 355, 60, 84, 0, 255, 0);
      }
   }

   gpGeneral->UpdateScreen(0, 355, 640, 480 - 355);
}

void CUIManager::DrawPlayerNames()
{
   gpGeneral->DrawTextVerticalUp(m_rgszPlayerName[PLAYER_LEFT], 15, 240 + 4 * strlen(m_rgszPlayerName[PLAYER_LEFT]));
   gpGeneral->DrawTextVerticalDown(m_rgszPlayerName[PLAYER_RIGHT], 610, 240 - 4 * strlen(m_rgszPlayerName[PLAYER_RIGHT]));
}

void CUIManager::DeleteCardImages(int location)
{
   int i;
   for (i = 19; i >= 0; i--) {
      if (m_rgpCardImages[location][i] != NULL) {
         delete m_rgpCardImages[location][i];
      }
      m_rgpCardImages[location][i] = NULL;
   }
}

void CUIManager::DeleteDiscardImages(int location)
{
   int i;
   for (i = 19; i >= 0; i--) {
      if (m_rgpDiscardImages[location][i] != NULL) {
         delete m_rgpDiscardImages[location][i];
      }
      m_rgpDiscardImages[location][i] = NULL;
   }
   DeleteTextLabel(location);
}

void CUIManager::DeleteTextLabel(int location)
{
   if (m_rgpTextLabels[location] != NULL) {
      delete m_rgpTextLabels[location];
   }
   m_rgpTextLabels[location] = NULL;
}
