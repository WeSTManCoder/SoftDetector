/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#define GAMECONFIG_FILE "softdetector"
 
//#undef DLL_EXPORT
#include "extension.h"

#include "iostream"
using namespace std;

#include "CDetour/detours.h"

#include <iclient.h>
#include <igameevents.h>

#include "netmessages.h"

SoftDetector g_SoftDetector;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_SoftDetector);

IGameConfig *g_pGameConf = nullptr;
CDetour *g_pProcessListenEventsDetour = nullptr;
int g_iCountEvents[MAXPLAYERS+1];

class CBaseClient : public IGameEventListener2, public IClient {};

DETOUR_DECL_MEMBER1(ProcessListenEventsHook, bool, CLC_ListenEvents *, msg) {
	CBaseClient *pBaseClient = reinterpret_cast<CBaseClient *>(this);
	
	if (!pBaseClient->IsFakeClient()) {
		int client = pBaseClient->GetPlayerSlot()+1;
		g_iCountEvents[client] = 0;
		for (int i = 0; i < MAX_EVENT_NUMBER; i++)
			if (msg->m_EventArray.Get(i)) g_iCountEvents[client]++;
			
		ConMsg(0, "%s - event count: %d\n", pBaseClient->GetClientName(), g_iCountEvents[client]);
		if (g_iCountEvents[client] > 25 && g_iCountEvents[client] < 78) ConMsg(0, "[SoftDetector]::HookListenEvent Kick: %s | events: %d\n", pBaseClient->GetClientName(), g_iCountEvents[client]);
	}
	
	return DETOUR_MEMBER_CALL(ProcessListenEventsHook)(msg);
}

bool SoftDetector::SDK_OnLoad(char* error, size_t maxlength, bool late) {
	if (!gameconfs->LoadGameConfigFile(GAMECONFIG_FILE, &g_pGameConf, error, maxlength)) {
		cout << "err load gameconfig file" << endl;
		return false;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);
	
	return true;
}

void SoftDetector::SDK_OnAllLoaded() {
	if (!g_pProcessListenEventsDetour) {
		g_pProcessListenEventsDetour = DETOUR_CREATE_MEMBER(ProcessListenEventsHook, "CBaseClient::ProcessListenEvents");
		if (g_pProcessListenEventsDetour) g_pProcessListenEventsDetour->EnableDetour();
		else cout << "failed detour" << endl;
	}
	
	playerhelpers->AddClientListener(&g_SoftDetector);
}

void SoftDetector::SDK_OnUnload() {
//
}

void SoftDetector::OnClientPutInServer(int client) {
	IGamePlayer *pClient = playerhelpers->GetGamePlayer(client);
	if (pClient->IsFakeClient()) return;
	
	if (g_iCountEvents[client] == 0) ConMsg(0, "[SoftDetector]::PutInServer Kick: %s | events: %d", pClient->GetName(), g_iCountEvents[client]);
}

void SoftDetector::OnClientDisconnected(int client) {
	g_iCountEvents[client] = 0;
}