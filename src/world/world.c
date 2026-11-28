/*
 * Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
 * Portal flood + cylinder collision against solid linedefs.
 */
#include "engine/world.h"

#include <string.h>

i16 world_find_sector(const World *w, i32 x, i32 z)
{
	u16 i;

	for (i = 0; i < w->sector_count; i++) {
		const Sector *s = &w->sectors[i];
		if (x >= s->min_x && x <= s->max_x && z >= s->min_z && z <= s->max_z) {
			return (i16)i;
		}
	}
	return -1;
}

u16 world_portal_flood(const World *w, i16 cam_sector, i16 *vis, u16 vis_cap)
{
	u8 seen[MAX_SECTORS];
	i16 queue[MAX_SECTORS];
	u16 qh = 0, qt = 0;
	u16 vis_n = 0;
	u16 i;

	if (cam_sector < 0 || (u16)cam_sector >= w->sector_count || vis_cap == 0) {
		return 0;
	}

	memset(seen, 0, sizeof(seen));
	queue[qt++] = cam_sector;
	seen[cam_sector] = 1;

	while (qh < qt && vis_n < vis_cap) {
		i16 sec = queue[qh++];
		const Sector *s = &w->sectors[sec];

		vis[vis_n++] = sec;

		for (i = 0; i < s->wall_count; i++) {
			const Wall *wall = &w->walls[s->wall_begin + i];
			i16 dest = wall->portal_to;

			if (dest < 0 || (u16)dest >= w->sector_count) {
				continue;
			}
			if (seen[dest]) {
				continue;
			}
			if (qt >= MAX_SECTORS) {
				break;
			}
			seen[dest] = 1;
			queue[qt++] = dest;
		}
	}

	return vis_n;
}

/* Closest point on segment AB to P; returns squared distance. */
static i32 dist2_point_seg(i32 px, i32 pz, i32 ax, i32 az, i32 bx, i32 bz, i32 *ox, i32 *oz)
{
	i32 abx = bx - ax;
	i32 abz = bz - az;
	i32 apx = px - ax;
	i32 apz = pz - az;
	i32 ab2 = abx * abx + abz * abz;
	i32 t;
	i32 qx, qz;
	i32 dx, dz;

	if (ab2 <= 0) {
		*ox = ax;
		*oz = az;
		dx = px - ax;
		dz = pz - az;
		return dx * dx + dz * dz;
	}

	/* t in 0..ab2 */
	t = apx * abx + apz * abz;
	if (t < 0) {
		t = 0;
	} else if (t > ab2) {
		t = ab2;
	}

	qx = ax + (abx * t) / ab2;
	qz = az + (abz * t) / ab2;
	*ox = qx;
	*oz = qz;
	dx = px - qx;
	dz = pz - qz;
	return dx * dx + dz * dz;
}

static void push_out_of_wall(i32 *x, i32 *z, i32 ax, i32 az, i32 bx, i32 bz, i32 radius)
{
	i32 ox, oz;
	i32 d2 = dist2_point_seg(*x, *z, ax, az, bx, bz, &ox, &oz);
	i32 r2 = radius * radius;
	i32 dx, dz;
	i32 dist;

	if (d2 >= r2 || d2 == 0) {
		return;
	}

	/* Integer length approx via iterative refine — for M1 walls are axis-aligned. */
	dx = *x - ox;
	dz = *z - oz;

	if (ax == bx) {
		/* Vertical wall (constant x) */
		if (dx == 0) {
			dx = (*x >= ax) ? 1 : -1;
		}
		*x = ox + (dx > 0 ? radius : -radius);
	} else if (az == bz) {
		/* Horizontal wall (constant z) */
		if (dz == 0) {
			dz = (*z >= az) ? 1 : -1;
		}
		*z = oz + (dz > 0 ? radius : -radius);
	} else {
		/* Generic: push along normal by crude scale */
		dist = 1;
		while (dist * dist < d2 && dist < 4096) {
			dist++;
		}
		if (dist > 0) {
			*x = ox + (dx * radius) / dist;
			*z = oz + (dz * radius) / dist;
		}
	}
}

void world_move_cylinder(
	const World *w,
	i16 *sector,
	i32 *x, i32 *z,
	i32 dx, i32 dz,
	i32 radius
)
{
	i32 nx = *x + dx;
	i32 nz = *z + dz;
	i16 sec = *sector;
	u16 pass;
	u16 i;

	if (sec < 0 || (u16)sec >= w->sector_count) {
		sec = world_find_sector(w, nx, nz);
		if (sec < 0) {
			return;
		}
		*sector = sec;
	}

	/* Separate axis slide: X then Z for nicer corners. */
	for (pass = 0; pass < 2; pass++) {
		i32 tx = (pass == 0) ? nx : *x;
		i32 tz = (pass == 0) ? *z : nz;
		const Sector *s = &w->sectors[sec];

		for (i = 0; i < s->wall_count; i++) {
			const Wall *wall = &w->walls[s->wall_begin + i];
			if (wall->portal_to >= 0) {
				continue; /* openings — no collision */
			}
			push_out_of_wall(&tx, &tz, wall->x0, wall->z0, wall->x1, wall->z1, radius);
		}

		if (pass == 0) {
			*x = tx;
		} else {
			*z = tz;
		}
	}

	/* Portal / sector membership after move */
	{
		i16 found = world_find_sector(w, *x, *z);
		if (found >= 0) {
			*sector = found;
		} else {
			/* Outside AABB: try neighbouring portal destinations, else revert */
			const Sector *s = &w->sectors[sec];
			i16 rescued = -1;

			for (i = 0; i < s->wall_count; i++) {
				const Wall *wall = &w->walls[s->wall_begin + i];
				i16 dest = wall->portal_to;
				if (dest < 0) {
					continue;
				}
				if (world_find_sector(w, *x, *z) == dest
					|| (*x >= w->sectors[dest].min_x && *x <= w->sectors[dest].max_x
						&& *z >= w->sectors[dest].min_z && *z <= w->sectors[dest].max_z)) {
					rescued = dest;
					break;
				}
			}

			if (rescued >= 0) {
				*sector = rescued;
			} else {
				/* Block: undo to last good point inside old sector AABB clamp */
				if (*x < s->min_x + radius) {
					*x = s->min_x + radius;
				}
				if (*x > s->max_x - radius) {
					*x = s->max_x - radius;
				}
				if (*z < s->min_z + radius) {
					*z = s->min_z + radius;
				}
				if (*z > s->max_z - radius) {
					*z = s->max_z - radius;
				}
			}
		}
	}
}
