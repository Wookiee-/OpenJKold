/*
===========================================================================
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#pragma once

#include "qcommon/qcommon.h"

void		GVM_InitGame						( int levelTime, int randomSeed, int restart );
void		GVM_ShutdownGame					( int restart );
char *		GVM_ClientConnect					( int clientNum, qboolean firstTime, qboolean isBot );
void		GVM_ClientBegin						( int clientNum );
qboolean	GVM_ClientUserinfoChanged			( int clientNum );
void		GVM_ClientDisconnect				( int clientNum );
void		GVM_ClientCommand					( int clientNum );
void		GVM_ClientThink						( int clientNum, usercmd_t *ucmd );
void		GVM_RunFrame						( int levelTime );
qboolean	GVM_ConsoleCommand					( void );
int			GVM_BotAIStartFrame					( int time );
void		GVM_ROFF_NotetrackCallback			( int entID, const char *notetrack );
void		GVM_SpawnRMGEntity					( void );
int			GVM_ICARUS_PlaySound				( void );
qboolean	GVM_ICARUS_Set						( void );
void		GVM_ICARUS_Lerp2Pos					( void );
void		GVM_ICARUS_Lerp2Origin				( void );
void		GVM_ICARUS_Lerp2Angles				( void );
int			GVM_ICARUS_GetTag					( void );
void		GVM_ICARUS_Lerp2Start				( void );
void		GVM_ICARUS_Lerp2End					( void );
void		GVM_ICARUS_Use						( void );
void		GVM_ICARUS_Kill						( void );
void		GVM_ICARUS_Remove					( void );
void		GVM_ICARUS_Play						( void );
int			GVM_ICARUS_GetFloat					( void );
int			GVM_ICARUS_GetVector				( void );
int			GVM_ICARUS_GetString				( void );
void		GVM_ICARUS_SoundIndex				( void );
int			GVM_ICARUS_GetSetIDForString		( void );
qboolean	GVM_NAV_ClearPathToPoint			( int entID, vec3_t pmins, vec3_t pmaxs, vec3_t point, int clipmask, int okToHitEnt );
qboolean	GVM_NPC_ClearLOS2					( int entID, const vec3_t end );
int			GVM_NAVNEW_ClearPathBetweenPoints	( vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int ignore, int clipmask );
qboolean	GVM_NAV_CheckNodeFailedForEnt		( int entID, int nodeNum );
qboolean	GVM_NAV_EntIsUnlockedDoor			( int entityNum );
qboolean	GVM_NAV_EntIsDoor					( int entityNum );
qboolean	GVM_NAV_EntIsBreakable				( int entityNum );
qboolean	GVM_NAV_EntIsRemovableUsable		( int entNum );
void		GVM_NAV_FindCombatPointWaypoints	( void );
int			GVM_BG_GetItemIndexByTag			( int tag, int type );

sharedEntity_t *flatten(sharedEntity_t *ent);

void SV_BindGame( void );
void SV_UnbindGame( void );
void SV_InitGame( qboolean restart );
void SV_RestartGame( void );

namespace jampog {
	void init(const vm_t *vm);
}

namespace jampog {
	void detour(void * const before, const void * const after);
}

namespace jampog {
	void set_base(void *base);
	void set_base(uintptr_t base);
	void *base_ptr();
	uintptr_t base_addr();
}

namespace jampog {
class Entity {
private:
	static constexpr auto CLIENT_OFS = 864;
	static constexpr auto HEALTH_OFS = 1232;
	static constexpr size_t SIZE = 1516;
	static constexpr auto GENTITY_OFS = 0x006CE620;
	static constexpr auto INUSE_OFS = 880;
	static constexpr auto LOCALANIMINDEX_OFS = 0x220;
	static constexpr auto PARENT_OFS = 0x40C;
	uintptr_t base;
public:
	Entity() = delete;
	Entity(uintptr_t base)
		: base(base)
	{}
	Entity(void *ptr)
		: Entity((uintptr_t)ptr)
	{}
	Entity(client_t *cl)
		: Entity(cl->gentity)
	{}
	Entity(sharedEntity_t *ent)
		: Entity((void*)ent)
	{}
	Entity(int index)
		: Entity(SV_GentityNum(index))
	{}
	static constexpr size_t size() {
		return SIZE;
	}
	static constexpr size_t array_size() {
		return SIZE * 1024;
	}
	static void *start() {
		return (void*)(base_addr() + GENTITY_OFS);
	}
	entityState_t& s() const {
		return ((sharedEntity_t*)base)->s;
	}
	playerState_t& ps() const {
		playerState_t *ps = ((sharedEntity_t*)base)->playerState;
		if (ps == nullptr) throw "NULL PLAYERSTATE";
		return *ps;
	}
	entityShared_t& r() const {
		return ((sharedEntity_t*)base)->r;
	}
	sharedEntity_t& gent() const {
		return *(sharedEntity_t*)base;
	}
	sharedEntity_t *gent_ptr() const {
		return (sharedEntity_t*)base;
	}
	int number() const {
		return SV_NumForGentity(gent_ptr());
	}
	bool inuse() const {
		return *(qboolean*)(base + INUSE_OFS) == qtrue;
	}
	void set_inuse(bool value) {
		*(qboolean*)(base + INUSE_OFS) = value ? qtrue : qfalse;
	}
	// returns false if player is spectating
	bool is_player() const {
		return s().eType == ET_PLAYER;
	}
	bool is_npc() const {
		return s().eType == ET_NPC;
	}
	int health() const {
		return *(int*)(base + HEALTH_OFS);
	}
	void set_health(int value) {
		*(int*)(base + HEALTH_OFS) = value;
	}
	void teleport(const vec3_t origin) {
		void (*TeleportPlayer)(void *ent, const vec3_t orig, vec3_t angles);
		TeleportPlayer = (decltype(TeleportPlayer))(base_addr() + 0x00146324);
		TeleportPlayer((void*)base, origin, s().angles);
	}
	void teleport(float x, float y, float z) {
		vec3_t orig = {x, y, z};
		teleport(orig);
	}
	void teleport(const Entity& To) {
		float x = To.origin()[0];
		float y = To.origin()[1];
		float z = To.origin()[2];
		if (To.is_player() || To.is_npc()) {
			z += 120.0f;
		}
		teleport(x, y, z);
	}
	vec3_t& origin() const {
		if (0 <= number() && number() < MAX_CLIENTS) {
			return ps().origin;
		} else {
			return r().currentOrigin;
		}
	}
	int local_anim_index() const {
		return *(int*)(base + LOCALANIMINDEX_OFS);
	}
	sharedEntity_t *parent_ptr() const {
		return *(sharedEntity_t**)(base + PARENT_OFS);
	}
	Entity parent() const {
		return parent_ptr();
	}
	void set_parent(sharedEntity_t *ent) {
		*(sharedEntity_t**)(base + PARENT_OFS) = ent;
	}
};
}
