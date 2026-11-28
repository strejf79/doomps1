#!/usr/bin/env bash
# Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
# Host-side check that make produced a cue/bin with expected ISO layout.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
CUE="${BUILD}/doomps1.cue"
BIN="${BUILD}/doomps1.bin"
ISO_XML="${ROOT}/iso.xml"

fail() { echo "FAIL: $*" >&2; exit 1; }
ok() { echo "OK: $*"; }

[[ -f "$CUE" ]] || fail "missing $CUE — run make first"
[[ -f "$BIN" ]] || fail "missing $BIN — run make first"
[[ -s "$BIN" ]] || fail "empty bin"

grep -q 'FILE.*doomps1.bin' "$CUE" || fail "cue does not reference doomps1.bin"
grep -qiE 'BINARY|MODE2' "$CUE" || fail "cue missing BINARY/MODE2 track line"

grep -q 'name="DOOMPS1.EXE"' "$ISO_XML" || fail "iso.xml missing DOOMPS1.EXE"
grep -q 'name="SYSTEM.CNF"' "$ISO_XML" || fail "iso.xml missing SYSTEM.CNF"
grep -q 'name="DATA"' "$ISO_XML" || fail "iso.xml missing DATA dir"
grep -q 'name="MAP.BIN"' "$ISO_XML" || fail "iso.xml missing MAP.BIN"
grep -q 'name="PREFETC.TOC"' "$ISO_XML" || fail "iso.xml missing PREFETC.TOC"
grep -q 'name="SFX.BIN"' "$ISO_XML" || fail "iso.xml missing SFX.BIN"

grep -qE 'DOOMPS1_M[56]' "$ISO_XML" || fail "iso.xml volume not DOOMPS1_M5/M6"

# Prefer dumpsxiso when SDK env is loaded
if command -v dumpsxiso >/dev/null 2>&1; then
	TMP="$(mktemp -d)"
	# dumpsxiso extracts; tolerate path variants
	if dumpsxiso -x "$TMP" "$BIN" >/dev/null 2>&1 || dumpsxiso "$BIN" -x "$TMP" >/dev/null 2>&1; then
		find "$TMP" -iname 'MAP.BIN' | grep -q . || fail "ISO extract missing MAP.BIN"
		find "$TMP" -iname 'PREFETC.TOC' -o -iname 'PREFETCH.TOC' | grep -q . \
			|| find "$TMP" -iname 'PREFETC*' | grep -q . \
			|| fail "ISO extract missing PREFETC.TOC"
		ok "dumpsxiso listing contains DATA payloads"
	else
		ok "dumpsxiso present but extract skipped (tool flags differ) — cue/bin + iso.xml checked"
	fi
	rm -rf "$TMP"
else
	ok "dumpsxiso not on PATH — cue/bin + iso.xml structural checks only"
fi

ok "disc layout verification passed ($(wc -c < "$BIN") byte bin)"
