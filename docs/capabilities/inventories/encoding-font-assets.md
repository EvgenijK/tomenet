# Bundled font encoding metadata audit

Read-only binary/source audit of every asset in `lib/xtra/font` and every BDF under `lib/xtra/posix_extra_fonts`. Metadata describes the asset declaration, not proof that each glyph drawing follows it. No Unicode conversion default is selected here.

Bundled directory: 50 PCF and 50 FON files (100 total). BDF source audit: 32 files.

## PCF declarations

ISO8859/1: 24, absent/absent: 26.

Ranges are first byte / second byte. Default is the encoded `default_char` field; missing counts are encoding-table entries equal to `0xffff`.

| Asset | Declared charset | Ranges | Default | Missing entries |
|---|---|---|---:|---:|
| `10x14tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `10x16tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `10x17tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `10x20.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `10x20tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `12x17tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `12x18tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `12x24.pcf` | ISO8859/1 | 0..0 / 1..255 | 32 | 34 |
| `12x24tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `14x20tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `16x16tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `16x22.pcf` | absent/absent | 0..0 / 0..127 | 0 | 0 |
| `16x22tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `16x24tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `16x24x.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `17x25tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `19x29tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `21x31tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `24x36tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `24x36x.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `4x6.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `5x7.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `5x8.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `5x8tg.pcf` | absent/absent | 0..0 / 0..255 | 0 | 0 |
| `6x10.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `6x10tg.pcf` | absent/absent | 0..0 / 0..255 | 0 | 0 |
| `6x12.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `6x12tg.pcf` | absent/absent | 0..0 / 0..255 | 0 | 0 |
| `6x13.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `6x13B.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `6x13O.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 64 |
| `6x13tg.pcf` | absent/absent | 0..0 / 0..255 | 0 | 0 |
| `6x9.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `6x9tg.pcf` | absent/absent | 0..0 / 0..255 | 0 | 0 |
| `7x13.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `7x13B.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `7x13O.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 64 |
| `7x13tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `7x14.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `7x14B.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `8x13.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `8x13B.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `8x13O.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 64 |
| `8x13tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `8x16.pcf` | ISO8859/1 | 0..0 / 1..255 | 32 | 34 |
| `9x15.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `9x15B.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `9x15tg.pcf` | absent/absent | 0..0 / 0..255 | 31 | 0 |
| `9x18.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |
| `9x18B.pcf` | ISO8859/1 | 0..0 / 0..255 | 0 | 33 |

## FON embedded bitmap-font headers

Parsed NE resource type `0x8008` (FONT), then FNT v2/v3 headers. Charset numbers below are raw `dfCharSet` values; no external codepage interpretation is inferred. Default field is `dfDefaultChar`, an offset relative to first character, shown with its computed byte position.

| Asset | Charset number | First..last byte | Default offset → byte |
|---|---:|---|---|
| `10X14TG.FON` | 0 | 0..255 | 31 → 31 |
| `10X14X.FON` | 0 | 0..255 | 31 → 31 |
| `10X16TG.FON` | 0 | 0..255 | 31 → 31 |
| `10X17K.FON` | 0 | 0..255 | 255 → 255 |
| `10X17KB.FON` | 0 | 0..255 | 0 → 0 |
| `10X17KS.FON` | 0 | 0..255 | 0 → 0 |
| `10X17TG.FON` | 0 | 0..255 | 31 → 31 |
| `10X20.FON` | 0 | 0..127 | 0 → 0 |
| `10X20TG.FON` | 0 | 0..255 | 31 → 31 |
| `12X17TG.FON` | 0 | 0..255 | 31 → 31 |
| `12X18K.FON` | 0 | 0..255 | 31 → 31 |
| `12X18KB.FON` | 0 | 0..255 | 31 → 31 |
| `12X18KS.FON` | 0 | 0..255 | 31 → 31 |
| `12X18TG.FON` | 0 | 0..255 | 31 → 31 |
| `12X18X.FON` | 0 | 0..255 | 31 → 31 |
| `12X24.FON` | 0 | 0..127 | 0 → 0 |
| `12X24TG.FON` | 0 | 0..255 | 31 → 31 |
| `14X20TG.FON` | 0 | 0..255 | 31 → 31 |
| `16X16TG.FON` | 0 | 0..255 | 31 → 31 |
| `16X16X.FON` | 0 | 0..255 | 31 → 31 |
| `16X22.FON` | 0 | 0..127 | 0 → 0 |
| `16X22.FON` | 0 | 0..127 | 0 → 0 |
| `16X22TG.FON` | 0 | 0..255 | 31 → 31 |
| `16X24TG.FON` | 0 | 0..255 | 31 → 31 |
| `16X24X.FON` | 0 | 0..255 | 31 → 31 |
| `17X25TG.FON` | 0 | 0..255 | 31 → 31 |
| `19X29TG.FON` | 0 | 0..255 | 31 → 31 |
| `21X31L.FON` | 0 | 1..255 | 31 → 32 |
| `21X31TG.FON` | 0 | 0..255 | 31 → 31 |
| `24X36TG.FON` | 0 | 0..255 | 31 → 31 |
| `24X36X.FON` | 0 | 0..255 | 0 → 0 |
| `5X8.FON` | 0 | 0..127 | 0 → 0 |
| `5X8TG.FON` | 0 | 0..255 | 0 → 0 |
| `6X10.FON` | 0 | 0..127 | 0 → 0 |
| `6X10TG.FON` | 0 | 0..255 | 0 → 0 |
| `6X12.FON` | 0 | 1..127 | 31 → 32 |
| `6X12TG.FON` | 0 | 0..255 | 0 → 0 |
| `6X13.FON` | 0 | 0..127 | 0 → 0 |
| `6X13B.FON` | 0 | 0..127 | 0 → 0 |
| `6X13TG.FON` | 0 | 0..255 | 0 → 0 |
| `6X9.FON` | 0 | 0..127 | 0 → 0 |
| `6X9TG.FON` | 0 | 0..255 | 0 → 0 |
| `7X13.FON` | 0 | 0..127 | 0 → 0 |
| `7X13B.FON` | 0 | 0..127 | 0 → 0 |
| `7X13TG.FON` | 0 | 0..255 | 31 → 31 |
| `8X13.FON` | 0 | 0..127 | 0 → 0 |
| `8X13B.FON` | 0 | 0..127 | 0 → 0 |
| `8X13TG.FON` | 0 | 0..255 | 31 → 31 |
| `9X15.FON` | 0 | 0..127 | 0 → 0 |
| `9X15B.FON` | 0 | 0..127 | 0 → 0 |
| `9X15TG.FON` | 0 | 0..255 | 31 → 31 |

Embedded FNT charset counts: 0: 51.

## BDF source declarations

| Source | Charset properties | ENCODING range / count | DEFAULT_CHAR |
|---|---|---|---|
| [lib/xtra/posix_extra_fonts/misc/bdf/10X17K.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/10X17K.bdf:12) | absent/absent | 0..255 / 256 | 255 |
| [lib/xtra/posix_extra_fonts/misc/bdf/10X17KB.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/10X17KB.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/misc/bdf/10X17KS.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/10X17KS.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/misc/bdf/12X18K.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/12X18K.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/misc/bdf/12X18KB.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/12X18KB.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/misc/bdf/12X18KS.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/12X18KS.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/misc/bdf/21X31L.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/misc/bdf/21X31L.bdf:12) | absent/absent | 1..255 / 255 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/10X14TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/10X14TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/10X16TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/10X16TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/10X17TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/10X17TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/10X20TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/10X20TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/12X17TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/12X17TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/12X18TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/12X18TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/12X24TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/12X24TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/14X20TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/14X20TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/16X16TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/16X16TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/16X22TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/16X22TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/16X24TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/16X24TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/17X25TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/17X25TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/19X29TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/19X29TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/21X31TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/21X31TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/24X36TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/24X36TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/5X8TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/5X8TG.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/6X10TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/6X10TG.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/6X12TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/6X12TG.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/6X13TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/6X13TG.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/6X9TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/6X9TG.bdf:12) | absent/absent | 0..255 / 256 | 0 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/7X13TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/7X13TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/8X13TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/8X13TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/tangar/bdf/9X15TG.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/tangar/bdf/9X15TG.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/virus/16x24x.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/virus/16x24x.bdf:12) | absent/absent | 0..255 / 256 | 31 |
| [lib/xtra/posix_extra_fonts/virus/24x36x.bdf](/home/svechnik/Projects/tomenet_modern_client/lib/xtra/posix_extra_fonts/virus/24x36x.bdf:13) | absent/absent | 0..255 / 256 | 31 |

BDF charset declaration counts: absent/absent: 32.

## Renderer interpretation and limitations

- `src/config.h:870` selects `9x15` as the default SDL3 main font; that is one asset choice, not a wire-text encoding declaration.
- `src/client/main-sdl3.c:5632` reads PCF ranges/default; `:5638` rejects multibyte encodings. `:5662` substitutes encoded `default_char` directly for missing glyph indexes, without mapping it through the encoding table. `:5905` indexes the table with the unsigned byte directly, without subtracting the lower encoding bound. These are implementation limitations, not general PCF semantics to promise.
- `src/client/main-sdl3.c:854` passes the original byte string to SDL_ttf UTF-8 rendering, without an explicit conversion table. Emulated monospace slices the run by byte offsets at `:835`.
- FON header charset declarations and PCF charset properties cannot certify drawing identity: custom fonts can replace individual byte glyphs. This audit reads metadata and table bounds, not visual bitmap classification or Unicode equivalence.
- No evidence in this audit establishes a single Latin-1, CP437, CP1251, or UTF-8 interpretation for all legacy server/client text. FON raw numeric charset fields must not silently be promoted into a wire-text contract.
- All assets parsed successfully; no raster/runtime checks were performed. The parser checks binary headers, resource/table bounds, and supported FNT versions; it does not implement a complete font validator.
