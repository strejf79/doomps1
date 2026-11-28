/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * World pickups.
 */
#ifndef ENGINE_ITEMS_H
#define ENGINE_ITEMS_H

#include "engine/mapbin.h"
#include "engine/player.h"
#include "engine/types.h"

#define MAX_ITEMS 32

typedef enum ItemKind {
	ITEM_HEALTH = 0,
	ITEM_AMMO,
	ITEM_SHOTGUN,
	ITEM_CHAINGUN
} ItemKind;

typedef struct Item {
	bool active;
	ItemKind kind;
	i16 x, z;
	i16 sector;
} Item;

typedef struct ItemWorld {
	Item items[MAX_ITEMS];
	u16 count;
} ItemWorld;

void items_init_from_map(ItemWorld *iw, const MapRuntime *map);
void items_tick(ItemWorld *iw, Player *player);

#endif /* ENGINE_ITEMS_H */
