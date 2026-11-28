/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Health / ammo / weapon pickups from cooked things.
 */
#include "engine/items.h"

#include <stdio.h>
#include <string.h>

void items_init_from_map(ItemWorld *iw, const MapRuntime *map)
{
	u16 i;
	u16 n = 0;

	memset(iw, 0, sizeof(*iw));
	for (i = 0; i < map->thing_count && n < MAX_ITEMS; i++) {
		const MapThing *t = &map->things[i];
		Item *it = &iw->items[n];
		if (t->type == THING_HEALTH || t->type == 2011 || t->type == 2012) {
			it->kind = ITEM_HEALTH;
		} else if (t->type == THING_AMMO) {
			it->kind = ITEM_AMMO;
		} else if (t->type == THING_SHOTGUN) {
			it->kind = ITEM_SHOTGUN;
		} else if (t->type == THING_CHAINGUN) {
			it->kind = ITEM_CHAINGUN;
		} else {
			continue;
		}
		it->active = true;
		it->x = t->x;
		it->z = t->z;
		it->sector = t->sector;
		n++;
	}

	/* Ensure demo pickups exist near spawn if map had none. */
	if (n == 0) {
		iw->items[0] = (Item){ true, ITEM_SHOTGUN, (i16)(map->spawn_x + 30), map->spawn_z, map->spawn_sector };
		iw->items[1] = (Item){ true, ITEM_HEALTH, (i16)(map->spawn_x - 30), map->spawn_z, map->spawn_sector };
		iw->items[2] = (Item){ true, ITEM_AMMO, map->spawn_x, (i16)(map->spawn_z + 30), map->spawn_sector };
		iw->items[3] = (Item){ true, ITEM_CHAINGUN, (i16)(map->spawn_x + 50), (i16)(map->spawn_z + 40), map->spawn_sector };
		n = 4;
		printf("items: seeded demo pickups near spawn\n");
	}
	iw->count = n;
	printf("items: %u pickups\n", n);
}

void items_tick(ItemWorld *iw, Player *player)
{
	u16 i;
	i32 px = player->x >> 12;
	i32 pz = player->z >> 12;

	for (i = 0; i < iw->count; i++) {
		Item *it = &iw->items[i];
		i32 dx, dz;
		if (!it->active) {
			continue;
		}
		dx = px - it->x;
		dz = pz - it->z;
		if (dx < 0) dx = -dx;
		if (dz < 0) dz = -dz;
		if (dx + dz > 20) {
			continue;
		}
		switch (it->kind) {
		case ITEM_HEALTH:
			player->health += 25;
			if (player->health > 100) player->health = 100;
			break;
		case ITEM_AMMO:
			player->ammo += 20;
			if (player->ammo > 200) player->ammo = 200;
			break;
		case ITEM_SHOTGUN:
			player->has_weapon[WPN_SHOTGUN] = 1;
			player->weapon = WPN_SHOTGUN;
			player->ammo += 8;
			break;
		case ITEM_CHAINGUN:
			player->has_weapon[WPN_CHAINGUN] = 1;
			player->weapon = WPN_CHAINGUN;
			player->ammo += 30;
			break;
		}
		it->active = false;
		player->flash = 4;
		printf("picked up kind=%d\n", it->kind);
	}
}
