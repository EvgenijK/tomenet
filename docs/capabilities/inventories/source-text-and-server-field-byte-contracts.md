# Source text and server-field byte contracts

Source audit, 2026-09-18. This registry is an investigation for [Enumerate source text and server-field byte contracts](../issues/27-enumerate-source-text-and-server-field-byte-contracts.md), not a selected encoder or runtime Unicode-support claim. It reads source only; no credentials, personal profiles, account databases or private notes were opened. No implementation changed. The shared checkout was not switched. Working-tree source fingerprints and mechanical coverage appear below.

## Reading the registry

An atomic field is identified by producer + consumer + argument position + version/command discriminator. Rows below provide shared rules; the appendices enumerate the individual call sites and exact arguments, so a family is not a substitute for its leaves. `E=n` means editor accepts up to n payload bytes and requires **n+1 bytes of destination storage**. `W=s/S/I` means the packet string slot has total capacity **80/256/160 bytes**, including NUL for a properly terminated string; ordinary safe payloads are 79/255/159. A larger C array does not enlarge its packet slot. Limits count bytes, not characters, scalar values or graphemes. Definitions: [defines.h:235](../../../src/common/defines.h#L235), [editor:2656](../../../src/client/c-util.c#L2656), [codec:550](../../../src/common/sockbuf.c#L550).

Evidence layers remain separate: original byte sequence; semantic markers/tokens; optional explicitly declared Unicode correspondence; presentation glyph ID/profile; editor value; transformed outbound bytes. No charset declaration or negotiation was found in these packet paths. A bundled font's codepage/Unicode map cannot supply the missing wire charset. ASCII literals and explicitly checked ASCII ranges establish those bytes' meaning; arbitrary high bytes remain source-specific, not implicitly Latin-1, CP437 or UTF-8. Compare [Encoding and the server contract](encoding-server-contract.md) and [Map every packet field to semantic state](../../../docs/research/single-window-packet-state.md).

## Shared primitive contracts and corrections to the earlier bounded audit

- `askfor_aux` clips default at `buf[len]`, edits one byte per accepted key, uses `isprint(i)`, byte cursor/deletion/search, and returns false on Esc after clearing the buffer. Enter accepts default or edited contents, including empty. Search-mode Esc first cancels search. Private mode masks and suppresses history; ordinary fields of length >=20 participate in shared history. Chat has a separate history and can add a two-byte channel prefix **after** editing. Callers that ignore the boolean still continue after cancellation. [c-util.c:2613](../../../src/client/c-util.c#L2613), [exit/history:3350](../../../src/client/c-util.c#L3350), [prefix:3421](../../../src/client/c-util.c#L3421).
- Live-trim key insertion uppercases the first byte and requires A..Z (character mode optionally allows leading `+`); subsequent bytes outside letters/digits/` .,-'&_$%~#` become `_`. This is a concrete existing field transform, not authority for imposing that alphabet on chat or passwords. Clipboard/history/default paths are not the same insertion branch. [c-util.c:3227](../../../src/client/c-util.c#L3227).
- Ctrl-L obtains bytes into `tmpbuf[MSG_LEN]`; the editor **does crop to caller len**, then MSG_LEN, before insertion. Clipboard helper itself duplicates `{` and the first eligible `:` without an output-capacity parameter. Input clipping before expansion therefore does not establish output safety. SDL3 joins CR/LF with spaces and clips the pre-escaped input at 223 bytes (`256-20-13`); Windows CF_TEXT clips there, X11 uses xclip/file line reads with different concatenation. Plain `char < 32` drops high bytes on signed-char targets. [clipboard:2390](../../../src/client/c-util.c#L2390), [SDL3:2500](../../../src/client/c-util.c#L2500), [editor crop:3178](../../../src/client/c-util.c#L3178).
- **Codec boundary is not guaranteed NUL truncation.** Active `Packet_printf` copies at most slot capacity or remaining socket room, then breaks; it does not insert NUL when input fills the slot, and its `buf > stop` error check cannot fire from that break. `Packet_scanf` consumes up to slot capacity and overwrites the last destination byte with NUL if none was received. For an 80-byte non-NUL `%s` payload with adequate socket space, sender emits all 80 bytes, receiver consumes 80 and exposes 79; that is a source-derived output, not a runtime test. Short remaining socket room and chained fields need explicit framing disposition. The earlier audit's “including NUL” describes intended capacities, not guaranteed behavior on overlong inputs. [sockbuf.c:550](../../../src/common/sockbuf.c#L550), [sockbuf.c:779](../../../src/common/sockbuf.c#L779).
- `isprint`, `isspace`, `toupper`, `strcasecmp` and byte substring operations do not constitute Unicode normalization. Negative plain-char arguments to ctype calls require special scrutiny; locale/build signedness are input provenance, not a single common repertoire. No universal normalization is present in the inspected owners.

## Producer/source registry

| Atomic source class | Producer → consumer; representation, markers and capacity | Persistence/output owner and boundary |
|---|---|---|
| New UI labels and future committed text | New UI's declared Unicode strings are distinct from legacy source; existing `main-sdl3` event/key path is byte/key-sequence based, not evidence that the legacy editor consumes committed Unicode | New UI implementation decision; [main-sdl3.c](../../../src/client/main-sdl3.c), [input pipeline](../../../docs/research/single-window-input-loops.md) |
| Keyboard/terminal queue | Platform key handlers → Term queue → macro expansion → `inkey`/editor; navigation/control bytes and macro sentinels 28..31 are grammar, not glyph text | Queue is transient; macro definitions persist separately; [c-util.c:1160](../../../src/client/c-util.c#L1160) |
| Clipboard import and export | SDL3 clipboard string, Windows CF_TEXT, X11 helper file → byte filters/escaping above; no text codec selected in this path | OS clipboard / temporary file; [c-util.c:2390](../../../src/client/c-util.c#L2390) |
| Received packet strings, each argument in Appendix B | Server bytes → nclient receivers → named state/surface. `%s/%S/%I` own byte slots. Messages/item names/special lines include FF color sequences; message routing includes FC and other control metadata | Transient state, message ring; selected notes/history export owners below. Exact semantic state owners: [packet-state registry](../../../docs/research/single-window-packet-state.md) |
| Received numeric glyphs / object-feature-monster mappings | Attribute + numeric glyph IDs, including width/version negotiation; not a text string or proof of Unicode scalar identity | Mapping/resource owners; [nclient.c:1728](../../../src/client/nclient.c#L1728). Explicit visual profile is presentation only |
| C literals / formatted runtime values | C source literals + `format`/`sprintf` + numeric/user/server operands → terminal/display/message output. A composed output inherits each operand's provenance | Source bundle or transient state; semantic text must not be inferred by scraping terminal cells; [c-tables.c](../../../src/client/c-tables.c), [c-xtra1.c](../../../src/client/c-xtra1.c) |
| Lua bundled and user override strings | `scpt/*.lua` load → `string_exec_lua`, Lua/C `call_lua`, names/descriptions/prompts/chat. Lua strings crossing `tolua_*string` become C strings; embedded NUL cannot be treated as a full C field | Script resources and file-transfer owner; [c-script.c:533](../../../src/client/c-script.c#L533), [call_lua:561](../../../src/client/c-script.c#L561), [c-init.lua](../../../lib/scpt/c-init.lua) |
| Preference records / macro escaped text | `process_pref_file`/command parser, `text_to_ascii`/`ascii_to_text`: escapes/control-caret/macro waits and byte triggers; includes and platform key conversion are executable grammar, not prose | PRF loaders/dumpers; [c-files.c:135](../../../src/client/c-files.c#L135), [key conversion:954](../../../src/client/c-files.c#L954); persistence schema delegated below |
| Guide text | `TomeNET-Guide.txt` → `init_guide`; `fgets(...,81)` chunks at 80 bytes, allocation based on file length, no declared charset conversion | Resource file + bookmark/export owners; [c-init.c:2985](../../../src/client/c-init.c#L2985) |
| Monster/kind/artifact data and lore | `r/k/a` data resource loaders → names/lore/search/paste; multiple `my_fgets(...,1024)` paths, colon/opcode grammar | Bundled/user overrides; [c-init.c:415](../../../src/client/c-init.c#L415), [1531](../../../src/client/c-init.c#L1531), [1678](../../../src/client/c-init.c#L1678) |
| Server files / help / MOTD / special lines | Wire title/line bytes and local text resources → perusal; server search E=60, `%s`, regexp/version flags; local guide/note search distinct | Perusal transient; file transfer may write resource bytes; [c-files.c:1915](../../../src/client/c-files.c#L1915), [1951](../../../src/client/c-files.c#L1951), [nclient.c:5813](../../../src/client/nclient.c#L5813) |
| Audio metadata, event keys and paths | `sound.cfg`/`music.cfg`, defaults, volume files → parser/event browser/audio reporting. Separate names, versions, filename operands, numeric volume text | Audio config/resource owners; [snd-sdl3.c:1211](../../../src/client/snd-sdl3.c#L1211), [1304](../../../src/client/snd-sdl3.c#L1304), [1553](../../../src/client/snd-sdl3.c#L1553) |
| Font/tileset filenames, window labels and mappings | Filesystem/config/menu → resource selection; labels and filenames are not the asset's glyph semantics; selection can report basename via `%s` | Legacy settings/resource loaders; [c-util.c:13799](../../../src/client/c-util.c#L13799), [15221](../../../src/client/c-util.c#L15221), [nclient.c:9210](../../../src/client/nclient.c#L9210) |
| Config, argv, environment, metaserver/host and relogin fields | Config/CLI/network/environment bytes → identity/connection/path consumers. Windows command parser accepts different lengths from interactive editor; relogin includes credential bytes and reason | Config/INI writers and process connection state; no actual values inspected; [client.c:129](../../../src/client/client.c#L129), [main-win.c:5997](../../../src/client/main-win.c#L5997), [nclient.c:2063](../../../src/client/nclient.c#L2063) |
| Saved history, private notes, bookmarks, dumps/screenshots | History load `my_fgets(...,MSG_LEN)`; saved note/history byte strings, bookmark line/name, exported screen/message data | [c-init.c:3349](../../../src/client/c-init.c#L3349), [3384](../../../src/client/c-init.c#L3384), [3414](../../../src/client/c-init.c#L3414), [4492](../../../src/client/c-init.c#L4492), [c-files.c:2570](../../../src/client/c-files.c#L2570) |

`my_fgets2` itself strips line endings, expands TAB to eight spaces and admits `isprint(c)` bytes, with dynamically grown storage; it is not lossless file-byte decoding. Application of the already agreed lossless-content boundary: retain original resource bytes separately from this parser-produced projection; the parser behavior itself supplies no Unicode correspondence. `my_fgets` and direct `fgets` call sites are not interchangeable. [c-files.c:631](../../../src/client/c-files.c#L631).

## Outbound field registry

| Field / producer → consumer | Editor / wire / consumer capacity and transforms | Observable outcome, owner |
|---|---|---|
| Account name | Birth E=15, private config/CLI can differ; `%s` 80. Live trim on interactive insertion. Enter_player `validstring` rejects bytes outside 32..122; Check_names requires first A..Z, length minimum and no colon. Account name also passes Trim_name in login | Cancel retains/retries birth owner, login failures disconnect/error. Account database owns identity; [c-birth.c:212](../../../src/client/c-birth.c#L212), [nserver.c:1377](../../../src/server/nserver.c#L1377), [2343](../../../src/server/nserver.c#L2343), [5126](../../../src/server/nserver.c#L5126) |
| Character name / selection / reorder | E=15, CNAME_LEN16; `%s`80. `+` reincarnation prefix special; reorder `***` + three command bytes; server clips character choice at15, Trim_name replaces disallowed bytes with `_`, strips trailing whitespace, fixes existing name case, then validates ownership/forbidden names/punctuation | Name cancellation returns overview; empty login is overview request, not character name. Server saves character identity; [nclient.c:744](../../../src/client/nclient.c#L744), [1189](../../../src/client/nclient.c#L1189), [1595](../../../src/client/nclient.c#L1595), [nserver.c:5390](../../../src/server/nserver.c#L5390). This closes the prior audit's uncertainty about non-first high bytes: Trim_name transforms them, it does not admit them as raw name bytes |
| Login password | E=15 private, `%s`80; config/CLI/relogin separate producers. `my_memfrob` XOR42 only when server_protocol>=2; NUL-sensitive strlen/C-string transport; no normalization | Server credential verification/hash; account/config persistence path separate, no secret logged here. Private mode does not reject printable `*`; XOR42 maps it to NUL, so transformed C-string boundaries require explicit review; [c-birth.c:287](../../../src/client/c-birth.c#L287), [c-init.c:3824](../../../src/client/c-init.c#L3824), [c-util.c:17959](../../../src/client/c-util.c#L17959), [nclient.c:1430](../../../src/client/nclient.c#L1430) |
| Old password / new password / confirmation | Each E=15 private; repeat only local. Old/new obfuscated `%s`80 each; server deobfuscates then GetAccount, strlen(new)>=6 and <=ACCFILE_PASSWD_LEN20, hashes and writes | Empty old rejected; mismatched repeat retries; Esc no send. Server messages distinguish wrong password/length/write failure/success; server wipes temporary buffers. [c-util.c:13106](../../../src/client/c-util.c#L13106), [nserver.c:16047](../../../src/server/nserver.c#L16047), [party.c:6059](../../../src/server/party.c#L6059) |
| Real name / host name / initial account handshake | OS/config producers; `%s`80 slots in initial ibuf and verification. Real/host nonempty check; `validstrings` vs `validstring` branch must not be collapsed | Connection metadata, not editable character labels; [c-init.c:4128](../../../src/client/c-init.c#L4128), [nserver.c:1013](../../../src/server/nserver.c#L1013), [2318](../../../src/server/nserver.c#L2318) |
| Chat text / slash / recipient + body | `buf` total =256-strlen(cname)-16, E=sizeof(buf)-1; wire `%S`256. Editor channel prefix; `{`→FF; item/store substitutions; local slash dispatch or generic Send_msg; server TAB split, channel/private/slash grammar/censor | Esc no send; local-self and local slash may consume; history before transforms. All grammar leaves delegated to [slash inventory](slash-command-grammar-and-dispatch.md), not generic UTF-8 editing; [c-cmd.c:8100](../../../src/client/c-cmd.c#L8100), [server util.c:6493](../../../src/server/util.c#L6493) |
| Item inscription | Two E=59 prompts; wire `%s`80; server INSCR_LEN80 clamps79, semantic inscription tokens downstream; autoinscription tags are a separate producer | Prompt Esc no packet; item validity/replay may reject. Item object/quark/save owns persistence; [c-cmd.c:1700](../../../src/client/c-cmd.c#L1700), [nserver.c:13325](../../../src/server/nserver.c#L13325) |
| Party/iron-team create/name, add/join target, remove target, hostility target, peace target | Each menu E=79, wire `%s`80. Receive_party clips40 and replaces plain-char `<32` with `_`; signed-char high bytes therefore replaced. Create/rename legal function further rejects >=20 bytes and imposes group alphabet/minimum/censor/collision checks | Each command keeps distinct semantics; no generic party name for target fields. Esc no send; server party/player state owns success; [c-cmd.c:8820](../../../src/client/c-cmd.c#L8820), [nserver.c:15051](../../../src/server/nserver.c#L15051), [party.c:1407](../../../src/server/party.c#L1407) |
| Guild create/name, add/join target, remove target | E=79, `%s`80. Receiver does not share party sanitizer; guild creation defers through request confirmation, then guild_name_legal length<20, ASCII letters/digits/` .,-'&_$%~#`, trim, censor, collision/similarity checks | Confirmation and access/mode checks server-owned; guild database persistence; [nserver.c:15109](../../../src/server/nserver.c#L15109), [party.c:1485](../../../src/server/party.c#L1485) |
| Guild authorized-adder name / min-level | E=NAME_LEN20 name (21 storage needed), E=4 numeric; name `%s`80 with command/flags; server uppercases first byte and loose name lookup; min-level converted to integer | Numeric editor is not string wire field; exact target/action restrictions remain server-owned; [c-cmd.c:8698](../../../src/client/c-cmd.c#L8698), [nserver.c:15143](../../../src/server/nserver.c#L15143) |
| House owner name / list tag / encoded permissions | Owner at buf+2 E60 (`O1` prefix), tag at buf+1 E19 (`T` prefix), masks/items ASCII numeric payload; all `%s`80 | Owner prompt checks cancellation; tag prompt ignores result and can send `T` after Esc. House command discriminator owns parsing/save; [c-cmd.c:9111](../../../src/client/c-cmd.c#L9111), [9193](../../../src/client/c-cmd.c#L9193), [Send_house](../../../src/client/nclient.c#L8082) |
| Master module save / load / size / entry / vault / sign / summon / player target / message | Each leaf is distinct prefix grammar. E19 at +1 module/size; E1 entry; E77 at +2 vault/sign; E76 at +3 summon; E15 at +1 target; E69 at +1 message. `%s`80 total incl prefixes | Many callers ignore cancellation and send prefix/empty suffix. Admin permissions do not change byte contract; [c-cmd.c:9489](../../../src/client/c-cmd.c#L9489), [9571](../../../src/client/c-cmd.c#L9571), [9736](../../../src/client/c-cmd.c#L9736), [10154](../../../src/client/c-cmd.c#L10154), [10304](../../../src/client/c-cmd.c#L10304) |
| Remote script source / script upload filename | Script E80, storage81 → `%s`80 **mismatch at full editor length**. Upload name E30/storage81 → remote_update, file filename `%s`80; upload chunks 1024 only server>4.6.1.1.0.1, otherwise256 | Esc no upload/execute; arbitrary script language remains delegated authority, not text normalization; [c-cmd.c:10369](../../../src/client/c-cmd.c#L10369) |
| Server-file search | Two E60 prompts, `%s`80 on supported version branch; regex/search direction are separate flags | Cancel preserves owner's search state; server owns match/navigation. [c-files.c:2067](../../../src/client/c-files.c#L2067), [nclient.c:7977](../../../src/client/nclient.c#L7977) |
| Request string reply | Editor E159/storage160; incoming default `%s`80, outgoing `%s`80, server destination MSG_LEN256 but decoder still80 | Accepted reply or literal ESC byte+NUL; ID/type checked/cleared on server; **79 safe wire bytes vs159 editor**. See callback leaves below; [nclient.c:7214](../../../src/client/nclient.c#L7214), [9057](../../../src/client/nclient.c#L9057), [nserver.c:16212](../../../src/server/nserver.c#L16212) |
| File-transfer filename, ping echo, version/OS/tag, audio pack names/versions, font/tiles name | Every exact string argument and format in Appendix B is a separate field; file `%s`, ping `%S`, metadata `%s`. Generated/resource text can exceed slot even without an editor | Send_version gates >4.8.0, >4.9.1, >4.9.2 and 4.9.2.1 build branches; font/audio gates source-owned. No UI repertoire inference. [nclient.c:667](../../../src/client/nclient.c#L667), [8135](../../../src/client/nclient.c#L8135), [8190](../../../src/client/nclient.c#L8190), [9190](../../../src/client/nclient.c#L9190) |

## Request-specific reply consumers (not one generic string field)

All entries inherit request E159/W80 above and require matching nonzero ID and RTYPE_STR. Quest IDs are an open numeric range; fixed callbacks below exhaust `handle_request_return_str`'s current cases. Values may be rejected silently or by server message. The request slot is consumed before callback semantics. [xtra1.c:10726](../../../src/server/xtra1.c#L10726).

| Request discriminator | Byte transform, cancel/default and resulting owner |
|---|---|
| `id >= RID_QUEST` | `str[30]=0`, then `quest_reply(Ind,id-RID_QUEST,str)` including ESC; quest-specific keywords/stages are data. No universal accepted alphabet. [10739](../../../src/server/xtra1.c#L10739) |
| `RID_GO_MOVE` / ENABLE_GO_GAME | Discard if left building; `str[160]=0` (not a promise 160 bytes can arrive); go_engine_move_human owns coordinate/command parse and invalid-move retry. [10753](../../../src/server/xtra1.c#L10753), [go.c:1575](../../../src/server/go.c#L1575) |
| `RID_GUILD_RENAME` | Clip40; ESC or empty means cancel; guild_rename invokes stricter name<20 legal checks. [10760](../../../src/server/xtra1.c#L10760) |
| `RID_ITEM_ORDER` / ENABLE_ITEM_ORDER | Case-insensitive `cancel` has explicit active-order behavior; clip40, trim spaces, collapse repeated spaces into local `str2[40]`, parse article/count and case-insensitive item/spell names. **40-byte source plus NUL into 40-byte str2 boundary requires review.** Store/order state owns persistence. [10767](../../../src/server/xtra1.c#L10767) |
| `RID_SEND_ITEM` | Uppercase first byte, lookup player ID/account, validate item/fee/mode; unknown ESC suppresses unknown-addressee message. Mail state owns send. [11205](../../../src/server/xtra1.c#L11205) |
| `RID_SEND_GOLD` | Same byte first-letter/lookup semantics, independent funds/fee/mode flow. [11427](../../../src/server/xtra1.c#L11427) |
| `RID_LOSE_MEMORIES_I_SKILL` | RESET_SKILL; case-insensitive exact skill name, then confirmation and request_extra index. ESC is not generically suppressed: missing skill can yield error. [11532](../../../src/server/xtra1.c#L11532) |
| `RID_LOSE_MEMORIES_II_SKILL` | Same name lookup, distinct fee/confirmation, server-owned reset. [11533](../../../src/server/xtra1.c#L11533) |
| `RID_CONTACT_OWNER` / PLAYER_STORES | Censor, quota/store/account validation; clip at MSG_LEN-CNAME_LEN-1=239 even though incoming wire delivers<=79; may notify online recipient and persist note. No explicit generic ESC suppression in this case. [11563](../../../src/server/xtra1.c#L11563) |

Key/amount/number/confirmation requests carry text **prompts** but send numeric/key responses; key cancel sends0, amount/number use primitive results; they are not text reply encoders. [nclient.c:7175](../../../src/client/nclient.c#L7175).

Lua delegates are another boundary: selectors ask names in C, then use Lua-derived spell/ability metadata; generated binding APIs and `call_lua` pass C strings. The shipped Lua lexical inventory and reachability analysis in [Remaining client input loops](remaining-client-input-loops.md) cover `get_check2`, `power_get_check`, `get_item_aux` and inactive code. No shipped direct `askfor_aux/get_string` script call was found by the expanded scan. Arbitrary downloaded/local Lua programs are not a finite list of text fields; their exposed API/result boundaries are the inventory unit. Local script E80 has no network `%s` restriction, and must remain separate from remote script E80.

## Local editor registry and persistence ownership

Each Appendix A call remains a separate atomic row; this table supplies shared destination, comparison and cancellation semantics. Unless a row/call says otherwise: E=N→storage at least N+1, editor byte rules above, boolean checked→cancel leaves parent without operation, boolean ignored→caller proceeds with emptied buffer. There is no packet limit for a purely local field.

| Local fields / leaves | E / transformations / consumer and save owner |
|---|---|
| Item/spell/power/technique/ability name lookups | E79; immunity/element E49; polymorph E40 (boolean ignored). Names use owner-specific exact case-insensitive or substring matching; return numeric ID, not submitted name bytes. [c-inven.c:523](../../../src/client/c-inven.c#L523), [c-spell.c:523](../../../src/client/c-spell.c#L523), [skills.c:624](../../../src/client/skills.c#L624) |
| Numeric quantities/list position/palette/volume | QUANTITY_WIDTH10; skill-list line10; list line10, master theme3/hex8; palette2/3; volume4. `atoi`/hex parser and range handling own outputs. No inferred digit-only editor restriction. [c-util.c:5057](../../../src/client/c-util.c#L5057), [skills.c:388](../../../src/client/skills.c#L388), [snd-sdl3.c:4473](../../../src/client/snd-sdl3.c#L4473) |
| Skill-name search | E80 but actual `tmp[MAX_CHARS]` is80: the primitive writes NUL at index80 on entry. Case-insensitive substring search and expansion of the skill tree are the consumer, no text packet. [skills.c:424](../../../src/client/skills.c#L424), [431](../../../src/client/skills.c#L431) |
| Macro filename load/save/append/delete | Usually E70; load-pref E1023; actual path builders/file operations own path capacity. Human macro action E1023 (`MACRO_MAXLEN=1024`), wizard operand E159, wait6, numeric/string subprompts50. `text_to_ascii` decodes grammar, action construction may add prefix/suffix; PRF/macro dump and file-set writers own persistence. [c-util.c:7298](../../../src/client/c-util.c#L7298), [c-files.c:135](../../../src/client/c-files.c#L135) | 
| Macro set name/comment | E20 (`MACROSET_*_LEN=20`), temp buffer larger, but final `basefilename[20]` / `stage_comment[20]` receive unchecked strcpy: a20-byte input needs21. Set/stage metadata and trigger/action dump persistence. Exact branch predicates in appendix matter: set-name condition `askfor_aux(...) &#124;&#124; !tmpbuf[0]` differs from usual accept-only flow. [c-util.c:10536](../../../src/client/c-util.c#L10536), [11256](../../../src/client/c-util.c#L11256) |
| Autoinscription search/index/filename/match/tag | Search/index E80; search backing array80 (off-by-one at primitive entry), index temp160; filenames70; match54/storage55; tag18/storage19. Match regex/substrings and tag gameplay grammar separate; apply can send inscription packets. `.ins` reader/writer owns files. [c-util.c:12239](../../../src/client/c-util.c#L12239), [12534](../../../src/client/c-util.c#L12534), [c-files.c:2985](../../../src/client/c-files.c#L2985) |
| Recall show/search/line/export filename | E79, independent ordinary/important histories; string matching/regex and filename semantics distinct. Recall dump/export owner writes bytes. [c-xtra2.c:346](../../../src/client/c-xtra2.c#L346), [764](../../../src/client/c-xtra2.c#L764) |
| Guide/lore/notes/audio searches and bookmark label | E79 searches, line7, bookmark59; many search calls ignore boolean so cancellation clears search. Guide chapter term E79; bookmark data persisted by c-init. Audio search E79 separate from event numeric volume. [c-cmd.c:3544](../../../src/client/c-cmd.c#L3544), [4644](../../../src/client/c-cmd.c#L4644), [snd-sdl3.c:4622](../../../src/client/snd-sdl3.c#L4622) |
| Window title / font / tileset name | E39 title/storage40; E159 font/tileset. Filesystem/resource lookup can fail independently of editor acceptance; selected basename can subsequently become W80 reporting field. Legacy config writer owns storage. [c-util.c:13799](../../../src/client/c-util.c#L13799), [13997](../../../src/client/c-util.c#L13997), [15221](../../../src/client/c-util.c#L15221) |
| Options filename / search | E70 sites in do_cmd_options; file syntax and writer separate from selection. [c-util.c:17024](../../../src/client/c-util.c#L17024) |
| Archive/download passwords | Three E79 prompts use **mode=0**, not ASKFOR_PRIVATE. Therefore shared nonprivate history behavior applies; Esc/empty skips workflow, failed password can retry. This is distinct from account credential policy and requires explicit disposition if changed; no archive password values inspected. [c-util.c:16051](../../../src/client/c-util.c#L16051), [16221](../../../src/client/c-util.c#L16221), [16311](../../../src/client/c-util.c#L16311) |
| Server address / dump/save-chat filename / screenshot filename | E79 interactive; dump save/export prompts distinct from command-line host/path. `do_save_chat` optional filename prompt ignores boolean. Slash screenshot tail bypasses editor wrapper and has its own suffix/path semantics. [c-birth.c:2388](../../../src/client/c-birth.c#L2388), [c-init.c:3371](../../../src/client/c-init.c#L3371), [c-util.c:17860](../../../src/client/c-util.c#L17860) |

Persistence schema, defaults, migration, encryption and new profile fields are **not chosen here**. Exact existing readers/writers are indexed by [Persisted settings and files](persisted-settings-and-files.md); that registry owns config/INI, PRF, INS, DNA, macro metadata, chat history, notes, bookmarks, audio overrides, exports and temporary files. Original byte-source preservation is not permission to execute imported macros/scripts or rewrite source files.

## Dispositions required by input/encoding and acceptance

The following findings feed [Decide text-field boundaries and legacy defects](../issues/35-decide-text-field-boundaries-and-legacy-defects.md); that ticket owns the pending product dispositions. Profile/default ownership remains in the existing persistence-schema ticket, and runtime proof in acceptance.

1. Establish an explicit safe field boundary for editor capacity, transformed payload capacity, C destination capacity and packet framing. Include request E159/W80 and remote-script E80/W80, generated metadata/filenames and clipboard expansion; do not silently inherit unterminated packet output. Review socket-room exhaustion separately from ordinary slot overflow. Source facts do not choose truncate/reject UX.
2. Preserve legal accepted baseline bytes without introducing a new universal alphabet. Fields with existing ASCII validation/transforms (names/groups) are distinct from opaque chat/password/inscription/Lua/file bytes. Source-to-Unicode correspondence beyond proven ASCII remains unresolved per source; visual glyph mappings never settle outbound correspondence.
3. Freeze cancel/default observables per owner: no packet, ESC sentinel, zero numeric reply, prefix-only packet, emptied local search, callback error, or a persisted note are distinguishable. Server request case behavior is not generalized from guild-rename cancellation.
4. Keep compatibility and safety decisions explicit: signed-char filtering; locale-dependent ctype; request item-order40→str2[40]; skill/auto-inscription search E80 into arrays80; macro set name/comment E20 copied into arrays20; receive `%s` into `Receive_playerlist`'s `tmp_n[NAME_LEN=20]` although codec can write80 ([nclient.c:7372](../../../src/client/nclient.c#L7372)); password XOR42/NUL; archive-password history. These are static findings, not verified exploits/crashes or permission to change behavior.
5. Acceptance should cover lengths 0, limit-1, limit, limit+1 before **and after** transforms; NUL/control/FF markers, brace/colon expansion, ASCII edge/range bytes, representative high bytes, signed/unsigned char, locale and protocol branches. Test malformed/split packets and chained fields, cancellation in each request leaf, credentials without recording actual values. Runtime/platform/fixture construction belongs to acceptance, raster profile/oracles to raster; this ticket does not claim those proofs.

## Completeness and deliberate boundaries

This is exhaustive at the **lexical C editor call-site and client string packet-call boundary**, including declarations/comments/build alternatives (classified below), with whole calls captured across newlines. It is not a proof that all dynamic user programs, server slash verbs, filesystem encodings or every composition buffer are safe. The original input and remaining-loop inventories supply key/focus/return semantics; the slash inventory supplies every local dispatch leaf; packet-state inventory supplies every received semantic state owner. Literal source strings and arbitrary resource lines are inventoried by producer/parser class rather than enumerating every sentence. Numeric map glyph streams and raw file payload chunks are explicitly not string fields.

Open facts are narrow: the encoding of opaque high bytes for each undeclared source; actual runtime platform/locale/library behavior; downstream dynamic Lua/quest/slash program semantics; all remaining intermediate formatting buffers; producer-valid versus malformed peer field sizes. These require explicit dispositions or acceptance, not a guessed common charset. No remaining discovery is concealed by calling a family “Unicode”.

Mechanical searches used: `rg -n 'askfor_aux\(|get_string\(|get_string_large\(' src/client --glob '*.[ch]'`; `rg -n 'Packet_(printf|scanf).*%[sSI]' src/client --glob '*.[ch]'`; expanded multiline extraction below; `rg -n 'get_string|askfor' src/client/lua_bind.c src/client/*.pkg lib/scpt`; `rg -n 'case RID_|Send_request_str' src/server`; file/reader and ctype scans. The simple editor regex yields176 lines, including noncalls; simple client packet regex114 lines. Appendices retain inactive alternatives rather than pretending they all run in one build. A source location is a navigable citation; argument expressions are exact static code, never runtime values.

## Appendix A — atomic editor call-site ledger

The exact length/mode and enclosing owner below identify each leaf. Calls in comments, prototypes and primitive implementations are retained as excluded/delegate evidence; `cmd_get_string` is a Windows argv parser, not a blocking editor. For all true primitive calls, the length argument is payload capacity and required destination is length+1; see local/outbound tables for consumer/persistence/cancellation. Source lines preserve inline conditions; where the action is on the next line, follow the source link and the existing input-loop inventory for the full branch.

| Source | Owner / classification | Exact source line / inline cancel predicate |
|---|---|---|
| [c-birth.c:212](../../../src/client/c-birth.c#L212) | `choose_name`; delegate | `if (askfor_aux(tmp, ACCNAME_LEN - 1, ASKFOR_LIVETRIM)) strcpy(nick, tmp);` |
| [c-birth.c:287](../../../src/client/c-birth.c#L287) | `enter_password`; delegate | `if (askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) strcpy(pass, tmp);` |
| [c-birth.c:2388](../../../src/client/c-birth.c#L2388) | `enter_server_name`; delegate | `return(askfor_aux(server_name, 79, 0));` |
| [c-cmd.c:1700](../../../src/client/c-cmd.c#L1700) | `cmd_inscribe`; delegate | `if (!get_string("Inscription: ", buf, 59)) return;` |
| [c-cmd.c:1712](../../../src/client/c-cmd.c#L1712) | `cmd_inscribe`; delegate | `if (get_string("Inscription: ", buf, 59))` |
| [c-cmd.c:2351](../../../src/client/c-cmd.c#L2351) | `cmd_character`; delegate | `if (get_string("Filename (you can post it to https://angband.live): ", tmp, MAX_CHARS - 1)) {` |
| [c-cmd.c:3543](../../../src/client/c-cmd.c#L3543) | `cmd_the_guide`; comment (excluded) | `//askfor_aux(buf, 7, 0)); //was: numerical chapters only` |
| [c-cmd.c:3544](../../../src/client/c-cmd.c#L3544) | `cmd_the_guide`; delegate | `askfor_aux(buf, MAX_CHARS - 1, 0); //allow entering chapter terms too` |
| [c-cmd.c:3585](../../../src/client/c-cmd.c#L3585) | `cmd_the_guide`; comment (excluded) | `//askfor_aux(buf, 7, 0)); //was: numerical chapters only` |
| [c-cmd.c:3586](../../../src/client/c-cmd.c#L3586) | `cmd_the_guide`; delegate | `askfor_aux(buf, MAX_CHARS - 1, 0); //allow entering chapter terms too` |
| [c-cmd.c:4400](../../../src/client/c-cmd.c#L4400) | `cmd_the_guide`; delegate | `} else askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:4473](../../../src/client/c-cmd.c#L4473) | `cmd_the_guide`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:4571](../../../src/client/c-cmd.c#L4571) | `cmd_the_guide`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:4614](../../../src/client/c-cmd.c#L4614) | `cmd_the_guide`; delegate | `if (!askfor_aux(buf, 7, 0)) {` |
| [c-cmd.c:4644](../../../src/client/c-cmd.c#L4644) | `cmd_the_guide`; delegate | `askfor_aux(tempstr, 60 - 1, 0);` |
| [c-cmd.c:5647](../../../src/client/c-cmd.c#L5647) | `browse_local_file`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:5686](../../../src/client/c-cmd.c#L5686) | `browse_local_file`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:5780](../../../src/client/c-cmd.c#L5780) | `browse_local_file`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:5826](../../../src/client/c-cmd.c#L5826) | `browse_local_file`; delegate | `if (!askfor_aux(buf, 7, 0)) {` |
| [c-cmd.c:7567](../../../src/client/c-cmd.c#L7567) | `cmd_notes`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [c-cmd.c:8119](../../../src/client/c-cmd.c#L8119) | `cmd_message`; delegate | `if (get_string("Message: ", buf, sizeof(buf) - 1)) {` |
| [c-cmd.c:8698](../../../src/client/c-cmd.c#L8698) | `cmd_guild_options`; delegate | `if (!get_string("Specify new minimum level: ", buf0, 4)) continue;` |
| [c-cmd.c:8705](../../../src/client/c-cmd.c#L8705) | `cmd_guild_options`; delegate | `if (!get_string("Specify player name: ", buf0, NAME_LEN)) continue;` |
| [c-cmd.c:8820](../../../src/client/c-cmd.c#L8820) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_CREATE, buf);` |
| [c-cmd.c:8827](../../../src/client/c-cmd.c#L8827) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_CREATE_IRONTEAM, buf);` |
| [c-cmd.c:8835](../../../src/client/c-cmd.c#L8835) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_ADD, buf);` |
| [c-cmd.c:8839](../../../src/client/c-cmd.c#L8839) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_ADD, buf);` |
| [c-cmd.c:8847](../../../src/client/c-cmd.c#L8847) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_DELETE, buf);` |
| [c-cmd.c:8865](../../../src/client/c-cmd.c#L8865) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_HOSTILE, buf);` |
| [c-cmd.c:8871](../../../src/client/c-cmd.c#L8871) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_party(PARTY_PEACE, buf);` |
| [c-cmd.c:8878](../../../src/client/c-cmd.c#L8878) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_CREATE, buf);` |
| [c-cmd.c:8883](../../../src/client/c-cmd.c#L8883) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_ADD, buf);` |
| [c-cmd.c:8887](../../../src/client/c-cmd.c#L8887) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_ADD, buf);` |
| [c-cmd.c:8892](../../../src/client/c-cmd.c#L8892) | `cmd_party`; delegate | `if (askfor_aux(buf, 79, 0)) Send_guild(GUILD_DELETE, buf);` |
| [c-cmd.c:9073](../../../src/client/c-cmd.c#L9073) | `cmd_load_pref`; delegate | `if (!get_string("Load pref: ", buf, 1023)) return;` |
| [c-cmd.c:9111](../../../src/client/c-cmd.c#L9111) | `cmd_house_chown`; delegate | `if (get_string("Enter new name: ", &buf[2], 60))` |
| [c-cmd.c:9193](../../../src/client/c-cmd.c#L9193) | `cmd_house_tag`; delegate | `get_string("Enter a tag for the houses list (max 19 characters): ", &buf[1], 20 - 1);` |
| [c-cmd.c:9380](../../../src/client/c-cmd.c#L9380) | `cmd_master_aux_level`; delegate | `if (!get_string("Theme (ESC/0 = default vanilla, +100 to set type instead): ", ts, 3)) t = 0;` |
| [c-cmd.c:9449](../../../src/client/c-cmd.c#L9449) | `cmd_master_aux_level`; delegate | `(void)get_string("Custom DF1 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);` |
| [c-cmd.c:9455](../../../src/client/c-cmd.c#L9455) | `cmd_master_aux_level`; delegate | `(void)get_string("Custom DF2 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);` |
| [c-cmd.c:9461](../../../src/client/c-cmd.c#L9461) | `cmd_master_aux_level`; delegate | `(void)get_string("Custom DF3 flags (string of 8 hex chars, logical OR): ", fshextmp, 8);` |
| [c-cmd.c:9489](../../../src/client/c-cmd.c#L9489) | `cmd_master_aux_level`; delegate | `get_string("Save module name (max 19 char): ", &buf[1], 19);` |
| [c-cmd.c:9494](../../../src/client/c-cmd.c#L9494) | `cmd_master_aux_level`; delegate | `get_string("Load module name (max 19 char): ", &buf[1], 19);` |
| [c-cmd.c:9499](../../../src/client/c-cmd.c#L9499) | `cmd_master_aux_level`; delegate | `get_string("WxH string (eg. 1x1-5x5): ", &buf[1], 19);` |
| [c-cmd.c:9503](../../../src/client/c-cmd.c#L9503) | `cmd_master_aux_level`; delegate | `get_string("Set level entry (> < or +): ", &buf[0], 1);` |
| [c-cmd.c:9571](../../../src/client/c-cmd.c#L9571) | `cmd_master_aux_generate_vault`; delegate | `get_string("Enter vault name: ", &buf[2], 77);` |
| [c-cmd.c:9736](../../../src/client/c-cmd.c#L9736) | `cmd_master_aux_build`; delegate | `get_string("Sign: ", &buf[2], 77);` |
| [c-cmd.c:10154](../../../src/client/c-cmd.c#L10154) | `cmd_master_aux_summon`; delegate | `get_string("Summon which monster or character? ", &buf[3], 79 - 3);` |
| [c-cmd.c:10304](../../../src/client/c-cmd.c#L10304) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |
| [c-cmd.c:10308](../../../src/client/c-cmd.c#L10308) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |
| [c-cmd.c:10312](../../../src/client/c-cmd.c#L10312) | `cmd_master_aux_player`; delegate | `get_string("Enter player name (prefix with '!' for no-ghost kill): ", &buf[1], 15);` |
| [c-cmd.c:10316](../../../src/client/c-cmd.c#L10316) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |
| [c-cmd.c:10320](../../../src/client/c-cmd.c#L10320) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |
| [c-cmd.c:10324](../../../src/client/c-cmd.c#L10324) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |
| [c-cmd.c:10329](../../../src/client/c-cmd.c#L10329) | `cmd_master_aux_player`; delegate | `get_string("Enter player name: ", &buf[1], 15);` |
| [c-cmd.c:10335](../../../src/client/c-cmd.c#L10335) | `cmd_master_aux_player`; delegate | `get_string("Message: ", &buf[1], 69);` |
| [c-cmd.c:10375](../../../src/client/c-cmd.c#L10375) | `cmd_script_upload`; delegate | `if (!get_string("Script name: ", name, 30)) return;` |
| [c-cmd.c:10393](../../../src/client/c-cmd.c#L10393) | `cmd_script_exec`; delegate | `if (!get_string("Script> ", buf, 80)) return;` |
| [c-cmd.c:10402](../../../src/client/c-cmd.c#L10402) | `cmd_script_exec_local`; delegate | `if (!get_string("Script> ", buf, 80)) return;` |
| [c-files.c:2057](../../../src/client/c-files.c#L2057) | `peruse_file`; delegate | `if (askfor_aux(tmp, 10, 0)) cur_line = atoi(tmp);` |
| [c-files.c:2067](../../../src/client/c-files.c#L2067) | `peruse_file`; delegate | `if (askfor_aux(tmp, 60, 0)) {` |
| [c-files.c:2084](../../../src/client/c-files.c#L2084) | `peruse_file`; delegate | `if (askfor_aux(tmp, 60, 0)) {` |
| [c-init.c:3371](../../../src/client/c-init.c#L3371) | `do_save_chat`; delegate | `if (ask_to_confirm_filename) get_string("Filename:", buf, 79);` |
| [c-inven.c:523](../../../src/client/c-inven.c#L523) | `get_item_hook_find_obj`; delegate | `if (!get_string(get_item_hook_find_obj_what, buf, 79)) return(FALSE);` |
| [c-spell.c:523](../../../src/client/c-spell.c#L523) | `get_mimic_spell`; delegate | `if (!get_string("Power? ", buf, 79)) {` |
| [c-spell.c:677](../../../src/client/c-spell.c#L677) | `do_mimic`; delegate | `get_string("Which form (name or number; 0 for player; -1 for previous) ? ", out_val, 40);` |
| [c-spell.c:780](../../../src/client/c-spell.c#L780) | `do_mimic`; delegate | `if (!get_string("Immunity? ", buf, 49)) {` |
| [c-spell.c:891](../../../src/client/c-spell.c#L891) | `get_item_hook_find_spell`; delegate | `if (!get_string("Spell name? ", buf, 79))` |
| [c-spell.c:936](../../../src/client/c-spell.c#L936) | `get_item_hook_find_spell`; delegate | `if (!get_string("Spell name? ", buf, 79)) return(FALSE);` |
| [c-spell.c:1582](../../../src/client/c-spell.c#L1582) | `get_melee_technique`; delegate | `if (!get_string("Technique? ", buf, 79)) {` |
| [c-spell.c:1760](../../../src/client/c-spell.c#L1760) | `get_ranged_technique`; delegate | `if (!get_string("Technique? ", buf, 79)) {` |
| [c-spell.c:2053](../../../src/client/c-spell.c#L2053) | `get_breath`; delegate | `if (!get_string("Element? ", buf, 49)) {` |
| [c-util.c:1649](../../../src/client/c-util.c#L1649) | `inkey`; comment (excluded) | `/* Crazy hack: Enable use of arrow keys, added for askfor_aux() -- TODO: Check collision with multi-key-macro-sequence (parse_under/strip_chars)! */` |
| [c-util.c:2557](../../../src/client/c-util.c#L2557) | `paste_from_clipboard`; comment (excluded) | `/* Helper function for message-history search done inside askfor_aux(),` |
| [c-util.c:2613](../../../src/client/c-util.c#L2613) | `askfor_aux`; primitive/parser definition (excluded as field) | `bool askfor_aux(char *buf, int len, char mode) {` |
| [c-util.c:3464](../../../src/client/c-util.c#L3464) | `get_string`; primitive/parser definition (excluded as field) | `bool get_string(cptr prompt, char *buf, int len) {` |
| [c-util.c:3494](../../../src/client/c-util.c#L3494) | `get_string`; shared primitive | `res = askfor_aux(buf, len, askfor_mode);` |
| [c-util.c:5086](../../../src/client/c-util.c#L5086) | `c_get_quantity`; delegate | `if (!get_string(prompt, buf, QUANTITY_WIDTH)) return(0);` |
| [c-util.c:5195](../../../src/client/c-util.c#L5195) | `c_get_number`; delegate | `if (!get_string(prompt, buf, QUANTITY_WIDTH)) return(0);` |
| [c-util.c:7410](../../../src/client/c-util.c#L7410) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7431](../../../src/client/c-util.c#L7431) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7455](../../../src/client/c-util.c#L7455) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7473](../../../src/client/c-util.c#L7473) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7493](../../../src/client/c-util.c#L7493) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7511](../../../src/client/c-util.c#L7511) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7526](../../../src/client/c-util.c#L7526) | `interact_macros`; delegate | `if (!askfor_aux(buf, MACRO_MAXLEN - 1, 0)) continue;` |
| [c-util.c:7747](../../../src/client/c-util.c#L7747) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7793](../../../src/client/c-util.c#L7793) | `interact_macros`; delegate | `if (!askfor_aux(buf, MACRO_MAXLEN - 1, 0)) {` |
| [c-util.c:7839](../../../src/client/c-util.c#L7839) | `interact_macros`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:7886](../../../src/client/c-util.c#L7886) | `interact_macros`; delegate | `if (!askfor_aux(buf, MACRO_MAXLEN - 1, 0)) {` |
| [c-util.c:8093](../../../src/client/c-util.c#L8093) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) continue;` |
| [c-util.c:9000](../../../src/client/c-util.c#L9000) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:9021](../../../src/client/c-util.c#L9021) | `interact_macros`; delegate | `if (!askfor_aux(buf2, 159, 0)) {` |
| [c-util.c:9135](../../../src/client/c-util.c#L9135) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) goto mw_device_1;` |
| [c-util.c:9241](../../../src/client/c-util.c#L9241) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) goto mw_option_1;` |
| [c-util.c:9344](../../../src/client/c-util.c#L9344) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) goto mw_equip_1;` |
| [c-util.c:9466](../../../src/client/c-util.c#L9466) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:9478](../../../src/client/c-util.c#L9478) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:9498](../../../src/client/c-util.c#L9498) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:9563](../../../src/client/c-util.c#L9563) | `interact_macros`; delegate | `if (!askfor_aux(buf, 6, 0)) continue;` |
| [c-util.c:9571](../../../src/client/c-util.c#L9571) | `interact_macros`; delegate | `if (!askfor_aux(buf, 6, 0)) continue;` |
| [c-util.c:9794](../../../src/client/c-util.c#L9794) | `interact_macros`; delegate | `if (askfor_aux(tmp, 50, 0)) {` |
| [c-util.c:9809](../../../src/client/c-util.c#L9809) | `interact_macros`; delegate | `if (askfor_aux(tmp, 50, 0)) {` |
| [c-util.c:10188](../../../src/client/c-util.c#L10188) | `interact_macros`; delegate | `if (askfor_aux(tmp, 50, 0)) {` |
| [c-util.c:10536](../../../src/client/c-util.c#L10536) | `interact_macros`; delegate | `if (askfor_aux(tmpbuf, MACROSET_NAME_LEN, 0) \|\| !tmpbuf[0]) {` |
| [c-util.c:11256](../../../src/client/c-util.c#L11256) | `interact_macros`; delegate | `if (askfor_aux(tmpbuf, MACROSET_COMMENT_LEN, 0)) { /* change comment? (ENTER to clear comment, ESC to keep it as it is) */` |
| [c-util.c:11328](../../../src/client/c-util.c#L11328) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:11338](../../../src/client/c-util.c#L11338) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:11350](../../../src/client/c-util.c#L11350) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:11375](../../../src/client/c-util.c#L11375) | `interact_macros`; delegate | `if (!askfor_aux(buf, 159, 0)) {` |
| [c-util.c:11923](../../../src/client/c-util.c#L11923) | `interact_macros`; delegate | `if (askfor_aux(tmp, 50, 0)) {` |
| [c-util.c:12239](../../../src/client/c-util.c#L12239) | `auto_inscriptions`; delegate | `if (!askfor_aux(search, MAX_CHARS, 0)) continue;` |
| [c-util.c:12253](../../../src/client/c-util.c#L12253) | `auto_inscriptions`; delegate | `if (!askfor_aux(tmp, MAX_CHARS, 0)) continue;` |
| [c-util.c:12451](../../../src/client/c-util.c#L12451) | `auto_inscriptions`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:12468](../../../src/client/c-util.c#L12468) | `auto_inscriptions`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:12485](../../../src/client/c-util.c#L12485) | `auto_inscriptions`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:12501](../../../src/client/c-util.c#L12501) | `auto_inscriptions`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:12517](../../../src/client/c-util.c#L12517) | `auto_inscriptions`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:12534](../../../src/client/c-util.c#L12534) | `auto_inscriptions`; delegate | `if (!askfor_aux(buf, AUTOINS_MATCH_LEN - 1, 0)) continue;` |
| [c-util.c:12575](../../../src/client/c-util.c#L12575) | `auto_inscriptions`; delegate | `if (!askfor_aux(buf, AUTOINS_TAG_LEN - 1, 0)) {` |
| [c-util.c:13106](../../../src/client/c-util.c#L13106) | `do_cmd_options_acc`; delegate | `if (!askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) {` |
| [c-util.c:13123](../../../src/client/c-util.c#L13123) | `do_cmd_options_acc`; delegate | `if (!askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) {` |
| [c-util.c:13134](../../../src/client/c-util.c#L13134) | `do_cmd_options_acc`; delegate | `if (!askfor_aux(tmp, PASSWORD_LEN - 1, ASKFOR_PRIVATE)) {` |
| [c-util.c:13799](../../../src/client/c-util.c#L13799) | `do_cmd_options_fonts`; delegate | `if (!askfor_aux(tmp_name, 39, 0)) { //the array reserves [40]` |
| [c-util.c:13997](../../../src/client/c-util.c#L13997) | `do_cmd_options_fonts`; delegate | `if (!askfor_aux(tmp_name, 159, 0)) {` |
| [c-util.c:14010](../../../src/client/c-util.c#L14010) | `do_cmd_options_fonts`; delegate | `if (!askfor_aux(tmp_name, 159, 0)) {` |
| [c-util.c:15221](../../../src/client/c-util.c#L15221) | `do_cmd_options_tilesets`; delegate | `if (!askfor_aux(tmp_name, 159, 0)) {` |
| [c-util.c:16051](../../../src/client/c-util.c#L16051) | `do_cmd_options_install_audio_packs`; delegate | `if (!askfor_aux(password, MAX_CHARS - 1, 0) \|\| !password[0]) {` |
| [c-util.c:16221](../../../src/client/c-util.c#L16221) | `do_cmd_options_install_audio_packs`; delegate | `if (!askfor_aux(password, MAX_CHARS - 1, 0) \|\| !password[0]) {` |
| [c-util.c:16311](../../../src/client/c-util.c#L16311) | `do_cmd_options_install_audio_packs`; delegate | `if (!askfor_aux(password, MAX_CHARS - 1, 0) \|\| !password[0]) {` |
| [c-util.c:16673](../../../src/client/c-util.c#L16673) | `do_cmd_options_colourblindness`; delegate | `if (!askfor_aux(buf, 2, 0)) {` |
| [c-util.c:16685](../../../src/client/c-util.c#L16685) | `do_cmd_options_colourblindness`; delegate | `if (!askfor_aux(buf, 2, 0)) {` |
| [c-util.c:16699](../../../src/client/c-util.c#L16699) | `do_cmd_options_colourblindness`; delegate | `if (!askfor_aux(buf, 3, 0)) {` |
| [c-util.c:16712](../../../src/client/c-util.c#L16712) | `do_cmd_options_colourblindness`; delegate | `if (!askfor_aux(buf, 3, 0)) {` |
| [c-util.c:16725](../../../src/client/c-util.c#L16725) | `do_cmd_options_colourblindness`; delegate | `if (!askfor_aux(buf, 3, 0)) {` |
| [c-util.c:17024](../../../src/client/c-util.c#L17024) | `do_cmd_options`; delegate | `if (!askfor_aux(src, 70, 0)) continue;` |
| [c-util.c:17077](../../../src/client/c-util.c#L17077) | `do_cmd_options`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:17089](../../../src/client/c-util.c#L17089) | `do_cmd_options`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:17101](../../../src/client/c-util.c#L17101) | `do_cmd_options`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:17131](../../../src/client/c-util.c#L17131) | `do_cmd_options`; delegate | `if (!askfor_aux(tmp, 70, 0)) continue;` |
| [c-util.c:17860](../../../src/client/c-util.c#L17860) | `c_close_game`; delegate | `if (get_string("Filename (you can post it to https://angband.live): ", tmp, MAX_CHARS - 1)) {` |
| [c-xtra2.c:346](../../../src/client/c-xtra2.c#L346) | `do_cmd_messages`; delegate | `if (askfor_aux(tmp, 79, 0)) {` |
| [c-xtra2.c:385](../../../src/client/c-xtra2.c#L385) | `do_cmd_messages`; delegate | `if (!askfor_aux(shower, 79, 0)) {` |
| [c-xtra2.c:408](../../../src/client/c-xtra2.c#L408) | `do_cmd_messages`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |
| [c-xtra2.c:453](../../../src/client/c-xtra2.c#L453) | `do_cmd_messages`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |
| [c-xtra2.c:550](../../../src/client/c-xtra2.c#L550) | `do_cmd_messages`; delegate | `if (get_string("Filename: ", tmp, 79)) {` |
| [c-xtra2.c:764](../../../src/client/c-xtra2.c#L764) | `do_cmd_messages_important`; delegate | `if (askfor_aux(tmp, 79, 0)) {` |
| [c-xtra2.c:804](../../../src/client/c-xtra2.c#L804) | `do_cmd_messages_important`; delegate | `if (!askfor_aux(shower, 79, 0)) {` |
| [c-xtra2.c:826](../../../src/client/c-xtra2.c#L826) | `do_cmd_messages_important`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |
| [c-xtra2.c:871](../../../src/client/c-xtra2.c#L871) | `do_cmd_messages_important`; delegate | `if (!askfor_aux(finder, 79, 0)) {` |
| [c-xtra2.c:969](../../../src/client/c-xtra2.c#L969) | `do_cmd_messages_important`; delegate | `if (get_string("Filename: ", tmp, 79)) {` |
| [main-win.c:5997](../../../src/client/main-win.c#L5997) | `cmd_get_string`; primitive/parser definition (excluded as field) | `static int cmd_get_string(char *str, char *dest, int n, bool quoted) {` |
| [main-win.c:6227](../../../src/client/main-win.c#L6227) | `WinMain`; argv parser | `i += cmd_get_string(&lpCmdLine[i + 1], nick, MAX_CHARS, quoted);` |
| [main-win.c:6228](../../../src/client/main-win.c#L6228) | `WinMain`; argv parser | `i += cmd_get_string(&lpCmdLine[i + 1], pass, MAX_CHARS, FALSE);` |
| [main-win.c:6234](../../../src/client/main-win.c#L6234) | `WinMain`; argv parser | `i += cmd_get_string(&lpCmdLine[i + 1], cname, MAX_CHARS, quoted);` |
| [main-win.c:6240](../../../src/client/main-win.c#L6240) | `WinMain`; argv parser | `i += cmd_get_string(&lpCmdLine[i + 1], path, 1024, quoted);` |
| [main-win.c:6269](../../../src/client/main-win.c#L6269) | `WinMain`; argv parser | `i += cmd_get_string(&lpCmdLine[i], svname, MAX_CHARS, quoted);` |
| [nclient.c:1189](../../../src/client/nclient.c#L1189) | `Receive_login`; delegate | `if (!askfor_aux(c_name, CNAME_LEN - 1, ASKFOR_LIVETRIM \| ASKFOR_PLUSPREFIX)) {` |
| [nclient.c:7221](../../../src/client/nclient.c#L7221) | `Receive_request_str`; delegate | `if (get_string(prompt, buf, MAX_CHARS_WIDE - 1)) Send_request_str(id, buf);` |
| [skills.c:388](../../../src/client/skills.c#L388) | `do_cmd_skill`; delegate | `if (askfor_aux(tmp, 10, 0)) {` |
| [skills.c:431](../../../src/client/skills.c#L431) | `do_cmd_skill`; delegate | `if (!askfor_aux(tmp, MAX_CHARS, 0)) {` |
| [skills.c:624](../../../src/client/skills.c#L624) | `do_cmd_activate_skill_aux`; delegate | `if (!get_string("Skill action? ", buf, 79)) {` |
| [snd-sdl.c:4346](../../../src/client/snd-sdl.c#L4346) | `do_cmd_options_sfx_sdl`; delegate | `if (!askfor_aux(tmp, 4, 0)) {` |
| [snd-sdl.c:4465](../../../src/client/snd-sdl.c#L4465) | `do_cmd_options_sfx_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl.c:4488](../../../src/client/snd-sdl.c#L4488) | `do_cmd_options_sfx_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl.c:5184](../../../src/client/snd-sdl.c#L5184) | `do_cmd_options_mus_sdl`; delegate | `if (!askfor_aux(tmp, 4, 0)) {` |
| [snd-sdl.c:5782](../../../src/client/snd-sdl.c#L5782) | `do_cmd_options_mus_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl.c:5805](../../../src/client/snd-sdl.c#L5805) | `do_cmd_options_mus_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl3.c:4473](../../../src/client/snd-sdl3.c#L4473) | `do_cmd_options_sfx_sdl`; delegate | `if (!askfor_aux(tmp, 4, 0)) {` |
| [snd-sdl3.c:4622](../../../src/client/snd-sdl3.c#L4622) | `do_cmd_options_sfx_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl3.c:4645](../../../src/client/snd-sdl3.c#L4645) | `do_cmd_options_sfx_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl3.c:5185](../../../src/client/snd-sdl3.c#L5185) | `do_cmd_options_mus_sdl`; delegate | `if (!askfor_aux(tmp, 4, 0)) {` |
| [snd-sdl3.c:5807](../../../src/client/snd-sdl3.c#L5807) | `do_cmd_options_mus_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [snd-sdl3.c:5830](../../../src/client/snd-sdl3.c#L5830) | `do_cmd_options_mus_sdl`; delegate | `askfor_aux(searchstr, MAX_CHARS - 1, 0);` |
| [externs.h:738](../../../src/client/externs.h#L738) | `declaration/global`; declaration (excluded) | `extern bool get_string(cptr prompt, char *buf, int len);` |
| [externs.h:768](../../../src/client/externs.h#L768) | `declaration/global`; declaration (excluded) | `extern bool askfor_aux(char *buf, int len, char mode);` |

Expanded editor/parser lexical entries: **176** (including five Windows argv parser call sites and its definition; the simple unanchored get_string regex also matches these).



## Appendix B — atomic client string packet-call ledger

Each `%s`, `%S`, `%I` slot in the full argument list is a separate field; corresponding positional variable is its producer/destination. Capacities are80/256/160 including NUL; no charset conversion in codec. Enclosing owner supplies consumer, build/version branches remain visible at cited source. Receive field semantic destination/persistence is cross-indexed in the packet-state registry. Commented legacy alternatives are marked.

| Source | Owner / direction | Full call (each string argument is an atomic field) |
|---|---|---|
| [c-init.c:4128](../../../src/client/c-init.c#L4128) | `client_init`; outbound | `Packet_printf(&ibuf, "%s%hu%c", real_name, GetPortNum(ibuf.sock), 0xFF)` |
| [c-init.c:4129](../../../src/client/c-init.c#L4129) | `client_init`; outbound | `Packet_printf(&ibuf, "%s%s%hu", nick, host_name, version)` |
| [nclient.c:505](../../../src/client/nclient.c#L505) | `Receive_file`; inbound | `Packet_scanf(&rbuf, "%s", fname)` |
| [nclient.c:594](../../../src/client/nclient.c#L594) | `Receive_file`; inbound | `Packet_scanf(&rbuf, "%s", fname)` |
| [nclient.c:667](../../../src/client/nclient.c#L667) | `Send_file_check`; outbound | `Packet_printf(&wbuf, "%c%c%hd%s", PKT_FILE, PKT_FILE_CHECK, id, fname)` |
| [nclient.c:675](../../../src/client/nclient.c#L675) | `Send_file_init`; outbound | `Packet_printf(&wbuf, "%c%c%hd%s", PKT_FILE, PKT_FILE_INIT, id, fname)` |
| [nclient.c:744](../../../src/client/nclient.c#L744) | `reorder_characters`; outbound | `Packet_printf(&wbuf, "%c%s", PKT_LOGIN, format("***%c%c%c", sortA, sortB, mode))` |
| [nclient.c:998](../../../src/client/nclient.c#L998) | `Receive_login`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%hd%hd%s", &ch, &mode, colour_sequence, c_name, &level, &c_race, &c_class, loc)` |
| [nclient.c:1000](../../../src/client/nclient.c#L1000) | `Receive_login`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%hd%hd", &ch, &mode, colour_sequence, c_name, &level, &c_race, &c_class)` |
| [nclient.c:1001](../../../src/client/nclient.c#L1001) | `Receive_login`; inbound | `Packet_scanf(&rbuf, "%c%s%s%hd%hd%hd", &ch, colour_sequence, c_name, &level, &c_race, &c_class)` |
| [nclient.c:1003](../../../src/client/nclient.c#L1003) | `Receive_login`; inbound; commented alternative | `Packet_scanf(&rbuf, "%c%s%s%hd%hd%hd", &ch, colour_sequence, c_name, &level, &c_race, &c_class)` |
| [nclient.c:1327](../../../src/client/nclient.c#L1327) | `Net_setup`; inbound | `Packet_scanf(&cbuf, "%c%c%c%c%c%c%s%d", &b1, &b2, &b3, &b4, &b5, &b6, str, &race_info[i].choice)` |
| [nclient.c:1338](../../../src/client/nclient.c#L1338) | `Net_setup`; inbound | `Packet_scanf(&cbuf, "%c%c%c%c%c%c%s", &b1, &b2, &b3, &b4, &b5, &b6, str)` |
| [nclient.c:1363](../../../src/client/nclient.c#L1363) | `Net_setup`; inbound | `Packet_scanf(&cbuf, "%s%d", str, &trait_info[i].choice)` |
| [nclient.c:1430](../../../src/client/nclient.c#L1430) | `Net_verify`; outbound | `Packet_printf(&wbuf, "%c%s%s%s", PKT_VERIFY, real, nick, pass)` |
| [nclient.c:1595](../../../src/client/nclient.c#L1595) | `Net_login`; outbound | `Packet_printf(&wbuf, "%c%s", PKT_LOGIN, "")` |
| [nclient.c:1618](../../../src/client/nclient.c#L1618) | `Net_login`; outbound | `Packet_printf(&wbuf, "%c%s", PKT_LOGIN, cname)` |
| [nclient.c:1691](../../../src/client/nclient.c#L1691) | `Net_start`; outbound | `Packet_printf(&wbuf, "%hd%hd%hd%hd%hd%hd%hd%s%s", sex, race, class, trait, audio_sfx, audio_music, use_graphics, graphic_tiles, fname)` |
| [nclient.c:1693](../../../src/client/nclient.c#L1693) | `Net_start`; outbound | `Packet_printf(&wbuf, "%hd%hd%hd%hd%hd%hd%hd%s%s", sex, race, class, trait, audio_sfx, audio_music, use_graphics, "NO_GRAPHICS", fname)` |
| [nclient.c:2017](../../../src/client/nclient.c#L2017) | `Receive_quit`; inbound | `Packet_scanf(&rbuf, "%s", reason)` |
| [nclient.c:2063](../../../src/client/nclient.c#L2063) | `Receive_relogin`; inbound | `Packet_scanf(&rbuf, "%c%s%d%s%s%s%s%c", &pkt, relogin_host, &relogin_port, relogin_accname, relogin_accpass, relogin_charname, reason, &delay)` |
| [nclient.c:2093](../../../src/client/nclient.c#L2093) | `Receive_sanity`; inbound | `Packet_scanf(&rbuf, "%c%c%s%c%hd%hd", &ch, &attr, buf, &dam, &cur, &max)` |
| [nclient.c:2101](../../../src/client/nclient.c#L2101) | `Receive_sanity`; inbound | `Packet_scanf(&rbuf, "%c%c%s%c", &ch, &attr, buf, &dam)` |
| [nclient.c:2106](../../../src/client/nclient.c#L2106) | `Receive_sanity`; inbound | `Packet_scanf(&rbuf, "%c%c%s", &ch, &attr, buf)` |
| [nclient.c:2329](../../../src/client/nclient.c#L2329) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name)` |
| [nclient.c:2332](../../../src/client/nclient.c#L2332) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &uses_dir, name)` |
| [nclient.c:2335](../../../src/client/nclient.c#L2335) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
| [nclient.c:2338](../../../src/client/nclient.c#L2338) | `Receive_inven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%s", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
| [nclient.c:2422](../../../src/client/nclient.c#L2422) | `Receive_subinven`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%hu%hd%c%c%hd%hd%c%I", &ch, &iposc, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name)` |
| [nclient.c:2514](../../../src/client/nclient.c#L2514) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%I%c", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9, name, &ident)` |
| [nclient.c:2518](../../../src/client/nclient.c#L2518) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9, name)` |
| [nclient.c:2522](../../../src/client/nclient.c#L2522) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%c%c%c%c%c%c%c%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b, name)` |
| [nclient.c:2529](../../../src/client/nclient.c#L2529) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%c%c%c%c%c%c%c%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b, name)` |
| [nclient.c:2536](../../../src/client/nclient.c#L2536) | `Receive_inven_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%c%c%c%c%c%c%c%c%s", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b, name)` |
| [nclient.c:2624](../../../src/client/nclient.c#L2624) | `Receive_unique_monster`; inbound | `Packet_scanf(&rbuf, "%c%d%d%s", &ch, &u_idx, &killed, name)` |
| [nclient.c:2661](../../../src/client/nclient.c#L2661) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name)` |
| [nclient.c:2664](../../../src/client/nclient.c#L2664) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%c%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &uses_dir, name)` |
| [nclient.c:2667](../../../src/client/nclient.c#L2667) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%I", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
| [nclient.c:2670](../../../src/client/nclient.c#L2670) | `Receive_equip`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%s", &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, name)` |
| [nclient.c:2742](../../../src/client/nclient.c#L2742) | `Receive_equip_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hu%hd%c%c%hd%hd%c%I%hd%hd%hd%hd%hd%hd%hd%hd%hd", 	    &ch, &pos, &attr, &wgt, &amt, &tval, &sval, &pval, &name1, &uses_dir, name, 	    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9)` |
| [nclient.c:2825](../../../src/client/nclient.c#L2825) | `Receive_char_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%hd%hd%d%hd%s", &ch, &race, &class, &trait, &sex, &mode, &lives, cname)` |
| [nclient.c:2827](../../../src/client/nclient.c#L2827) | `Receive_char_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%hd%hd%hd%hd%s", &ch, &race, &class, &trait, &sex, &mode, &lives, cname)` |
| [nclient.c:2829](../../../src/client/nclient.c#L2829) | `Receive_char_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%hd%hd%hd%s", &ch, &race, &class, &trait, &sex, &mode, cname)` |
| [nclient.c:2912](../../../src/client/nclient.c#L2912) | `Receive_various`; inbound | `Packet_scanf(&rbuf, "%c%hu%hu%hu%hu%s", &ch, &hgt, &wgt, &age, &sc, buf)` |
| [nclient.c:3005](../../../src/client/nclient.c#L3005) | `Receive_skill_init`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%hd%hd%d%c%S%S%S", &ch, &i, 	    &father, &order, &mkey, &flags1, &tval, name, desc, act)` |
| [nclient.c:3150](../../../src/client/nclient.c#L3150) | `Receive_history`; inbound | `Packet_scanf(&rbuf, "%c%hu%s", &ch, &line, buf)` |
| [nclient.c:3346](../../../src/client/nclient.c#L3346) | `Receive_message`; inbound | `Packet_scanf(&rbuf, "%c%S", &ch, buf)` |
| [nclient.c:3611](../../../src/client/nclient.c#L3611) | `Receive_title`; inbound | `Packet_scanf(&rbuf, "%c%s", &ch, buf)` |
| [nclient.c:3634](../../../src/client/nclient.c#L3634) | `Receive_depth`; inbound | `Packet_scanf(&rbuf, "%c%hu%hu%hu%c%c%c%s%s%s", &ch, &x, &y, &z, &town, &colour, &colour_sector, buf, location_name2, location_pre)` |
| [nclient.c:3637](../../../src/client/nclient.c#L3637) | `Receive_depth`; inbound | `Packet_scanf(&rbuf, "%c%hu%hu%hu%c%c%c%s%s", &ch, &x, &y, &z, &town, &colour, &colour_sector, buf, location_name2)` |
| [nclient.c:3642](../../../src/client/nclient.c#L3642) | `Receive_depth`; inbound | `Packet_scanf(&rbuf, "%c%hu%hu%hu%c%c%c%s", &ch, &x, &y, &z, &town, &colour, &colour_sector, buf)` |
| [nclient.c:3647](../../../src/client/nclient.c#L3647) | `Receive_depth`; inbound | `Packet_scanf(&rbuf, "%c%hu%hu%hu%c%hu%s", &ch, &x, &y, &z, &town, &old_colour, buf)` |
| [nclient.c:3734](../../../src/client/nclient.c#L3734) | `Receive_bpr_wraith_prob`; inbound | `Packet_scanf(&rbuf, "%c%c%c%s", &ch, &bpr, &attr, bpr_str)` |
| [nclient.c:3965](../../../src/client/nclient.c#L3965) | `Receive_spell_info`; inbound | `Packet_scanf(&rbuf, "%c%d%d%d%hu%hu%hu%s", &ch, &spells[0], &spells[1], &spells[2], &realm, &book, &line, buf)` |
| [nclient.c:4520](../../../src/client/nclient.c#L4520) | `Receive_store_action`; inbound | `Packet_scanf(&rbuf, "%c%c%hd%hd%s%c%c%d%u", &ch, &pos, &bact, &action, name, &attr, &letter, &cost, &flag)` |
| [nclient.c:4522](../../../src/client/nclient.c#L4522) | `Receive_store_action`; inbound | `Packet_scanf(&rbuf, "%c%c%hd%hd%s%c%c%hd%hu", &ch, &pos, &bact, &action, name, &attr, &letter, &oldcost, &oldflag)` |
| [nclient.c:4526](../../../src/client/nclient.c#L4526) | `Receive_store_action`; inbound | `Packet_scanf(&rbuf, "%c%c%hd%hd%s%c%c%hd%c", &ch, &pos, &bact, &action, name, &attr, &letter, &oldcost, &oldflag2)` |
| [nclient.c:4556](../../../src/client/nclient.c#L4556) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%d%s", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval, &powers)` |
| [nclient.c:4559](../../../src/client/nclient.c#L4559) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd%s", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, &powers)` |
| [nclient.c:4563](../../../src/client/nclient.c#L4563) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b)` |
| [nclient.c:4567](../../../src/client/nclient.c#L4567) | `Receive_store`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%s%c%c%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b)` |
| [nclient.c:4605](../../../src/client/nclient.c#L4605) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%d%hd%hd%hd%hd%hd%hd%hd%hd%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9)` |
| [nclient.c:4609](../../../src/client/nclient.c#L4609) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd%hd%hd%hd%hd%hd%hd%hd%hd%hd", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, 		    &xtra1, &xtra2, &xtra3, &xtra4, &xtra5, &xtra6, &xtra7, &xtra8, &xtra9)` |
| [nclient.c:4614](../../../src/client/nclient.c#L4614) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%S%c%c%hd%c%c%c%c%c%c%c%c%c", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b)` |
| [nclient.c:4622](../../../src/client/nclient.c#L4622) | `Receive_store_wide`; inbound | `Packet_scanf(&rbuf, "%c%c%c%hd%hd%d%s%c%c%hd%c%c%c%c%c%c%c%c%c", &ch, &pos, &attr, &wgt, &num, &price, name, &tval, &sval, &pval16b, 		    &xtra1b, &xtra2b, &xtra3b, &xtra4b, &xtra5b, &xtra6b, &xtra7b, &xtra8b, &xtra9b)` |
| [nclient.c:4669](../../../src/client/nclient.c#L4669) | `Receive_store_special_str`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%s", &ch, &line, &col, &attr, str)` |
| [nclient.c:5453](../../../src/client/nclient.c#L5453) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char, &store_price_mul)` |
| [nclient.c:5455](../../../src/client/nclient.c#L5455) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d%c%c", &ch, &store_num, store_name, owner_name, &num_items, &max_cost, &store_attr, &store_char)` |
| [nclient.c:5457](../../../src/client/nclient.c#L5457) | `Receive_store_info`; inbound | `Packet_scanf(&rbuf, "%c%hd%s%s%hd%d", &ch, &store_num, store_name, owner_name, &num_items, &max_cost)` |
| [nclient.c:5520](../../../src/client/nclient.c#L5520) | `Receive_target_info`; inbound | `Packet_scanf(&rbuf, "%c%c%c%S", &ch, &x, &y, buf)` |
| [nclient.c:5522](../../../src/client/nclient.c#L5522) | `Receive_target_info`; inbound | `Packet_scanf(&rbuf, "%c%c%c%s", &ch, &x, &y, buf)` |
| [nclient.c:5813](../../../src/client/nclient.c#L5813) | `Receive_special_line`; inbound | `Packet_scanf(&rbuf, "%c%d%d%c%I", &ch, &max, &line, &attr, buf)` |
| [nclient.c:5817](../../../src/client/nclient.c#L5817) | `Receive_special_line`; inbound | `Packet_scanf(&rbuf, "%c%hd%hd%c%I", &ch, &old_max, &old_line, &attr, buf)` |
| [nclient.c:5960](../../../src/client/nclient.c#L5960) | `Receive_pickup_check`; inbound | `Packet_scanf(&rbuf, "%c%s", &ch, buf)` |
| [nclient.c:5976](../../../src/client/nclient.c#L5976) | `Receive_party_stats`; inbound | `Packet_scanf(&rbuf, "%c%d%d%s%d%d%d%d%d", &ch, &j, &color, &partymembername, &k, &chp, &mhp, &cmp, &mmp)` |
| [nclient.c:5989](../../../src/client/nclient.c#L5989) | `Receive_party`; inbound | `Packet_scanf(&rbuf, "%c%s%s%s", &ch, pname, pmembers, powner)` |
| [nclient.c:6040](../../../src/client/nclient.c#L6040) | `Receive_guild`; inbound | `Packet_scanf(&rbuf, "%c%s%s%s", &ch, gname, gmembers, gowner)` |
| [nclient.c:6092](../../../src/client/nclient.c#L6092) | `Receive_guild_config`; inbound | `Packet_scanf(&rbuf, "%s", dummy)` |
| [nclient.c:6096](../../../src/client/nclient.c#L6096) | `Receive_guild_config`; inbound | `Packet_scanf(&rbuf, "%s", guild.adder[i])` |
| [nclient.c:6260](../../../src/client/nclient.c#L6260) | `Receive_chardump`; inbound | `Packet_scanf(&rbuf, "%c%s", &ch, &type)` |
| [nclient.c:6378](../../../src/client/nclient.c#L6378) | `Receive_extra_status`; inbound | `Packet_scanf(&rbuf, "%c%s", &ch, &status)` |
| [nclient.c:6400](../../../src/client/nclient.c#L6400) | `Receive_ping`; inbound | `Packet_scanf(&rbuf, "%c%c%d%d%d%S", &ch, &pong, &id, &tim, &utim, &buf)` |
| [nclient.c:6471](../../../src/client/nclient.c#L6471) | `Receive_ping`; outbound | `Packet_printf(&wbuf, "%c%c%d%d%d%S", ch, pong, id, tim, utim, buf)` |
| [nclient.c:7181](../../../src/client/nclient.c#L7181) | `Receive_request_key`; inbound | `Packet_scanf(&rbuf, "%c%d%s", &ch, &id, prompt)` |
| [nclient.c:7194](../../../src/client/nclient.c#L7194) | `Receive_request_amt`; inbound | `Packet_scanf(&rbuf, "%c%d%s%d", &ch, &id, prompt, &max)` |
| [nclient.c:7206](../../../src/client/nclient.c#L7206) | `Receive_request_num`; inbound | `Packet_scanf(&rbuf, "%c%d%s%d%d%d", &ch, &id, prompt, &predef, &min, &max)` |
| [nclient.c:7218](../../../src/client/nclient.c#L7218) | `Receive_request_str`; inbound | `Packet_scanf(&rbuf, "%c%d%s%s", &ch, &id, prompt, buf)` |
| [nclient.c:7235](../../../src/client/nclient.c#L7235) | `Receive_request_cfr`; inbound | `Packet_scanf(&rbuf, "%c%d%s%c", &ch, &id, prompt, &dy)` |
| [nclient.c:7238](../../../src/client/nclient.c#L7238) | `Receive_request_cfr`; inbound | `Packet_scanf(&rbuf, "%c%d%s", &ch, &id, prompt)` |
| [nclient.c:7352](../../../src/client/nclient.c#L7352) | `Receive_Guide`; inbound | `Packet_scanf(&rbuf, "%c%c%d%s", &ch, &search_type, &lineno, search_string)` |
| [nclient.c:7390](../../../src/client/nclient.c#L7390) | `Receive_playerlist`; inbound | `Packet_scanf(&rbuf, "%s%I", tmp_n, tmp)` |
| [nclient.c:7406](../../../src/client/nclient.c#L7406) | `Receive_playerlist`; inbound | `Packet_scanf(&rbuf, "%s%I", tmp_n, tmp)` |
| [nclient.c:7427](../../../src/client/nclient.c#L7427) | `Receive_playerlist`; inbound | `Packet_scanf(&rbuf, "%s", tmp_n)` |
| [nclient.c:7484](../../../src/client/nclient.c#L7484) | `Receive_whats_under_you_feet`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%I", &ch, &crossmod_item, &cant_see, &on_pile, o_name)` |
| [nclient.c:7486](../../../src/client/nclient.c#L7486) | `Receive_whats_under_you_feet`; inbound | `Packet_scanf(&rbuf, "%c%c%c%c%s", &ch, &crossmod_item, &cant_see, &on_pile, o_name)` |
| [nclient.c:7702](../../../src/client/nclient.c#L7702) | `Send_inscribe`; outbound | `Packet_printf(&wbuf, "%c%hd%s", PKT_INSCRIBE, item, buf)` |
| [nclient.c:7829](../../../src/client/nclient.c#L7829) | `Send_msg`; outbound | `Packet_printf(&wbuf, "%c%S", PKT_MESSAGE, message)` |
| [nclient.c:7983](../../../src/client/nclient.c#L7983) | `Send_special_line`; outbound | `Packet_printf(&wbuf, "%c%c%d%s", PKT_SPECIAL_LINE, type, line, srcstr ? srcstr : "")` |
| [nclient.c:8012](../../../src/client/nclient.c#L8012) | `Send_party`; outbound | `Packet_printf(&wbuf, "%c%hd%s", PKT_PARTY, command, buf)` |
| [nclient.c:8019](../../../src/client/nclient.c#L8019) | `Send_guild`; outbound | `Packet_printf(&wbuf, "%c%hd%s", PKT_GUILD, command, buf)` |
| [nclient.c:8026](../../../src/client/nclient.c#L8026) | `Send_guild_config`; outbound | `Packet_printf(&wbuf, "%c%d%d%s", PKT_GUILD_CFG, command, flags, buf)` |
| [nclient.c:8082](../../../src/client/nclient.c#L8082) | `Send_admin_house`; outbound | `Packet_printf(&wbuf, "%c%hd%s", PKT_HOUSE, dir, buf)` |
| [nclient.c:8089](../../../src/client/nclient.c#L8089) | `Send_master`; outbound | `Packet_printf(&wbuf, "%c%hd%s", PKT_MASTER, command, buf)` |
| [nclient.c:8135](../../../src/client/nclient.c#L8135) | `Send_ping`; outbound | `Packet_printf(&wbuf, "%c%c%d%d%d%S", PKT_PING, pong, ++ping_id, tim, utim, buf)` |
| [nclient.c:8158](../../../src/client/nclient.c#L8158) | `Send_change_password`; outbound | `Packet_printf(&wbuf, "%c%s%s", PKT_CHANGE_PASSWORD, old_pass, new_pass)` |
| [nclient.c:8194](../../../src/client/nclient.c#L8194) | `Send_version`; outbound | `Packet_printf(&wbuf, "%c%s%s", PKT_VERSION, longVersion, os_version)` |
| [nclient.c:8217](../../../src/client/nclient.c#L8217) | `Send_version`; outbound | `Packet_printf(&wbuf, "%c%s%s%d", PKT_VERSION, longVersion, os_version, avg)` |
| [nclient.c:8219](../../../src/client/nclient.c#L8219) | `Send_version`; outbound | `Packet_printf(&wbuf, "%c%s%s%d%d%d%d%s%d", PKT_VERSION, longVersion, os_version, avg, guide_lastline, VERSION_BRANCH, VERSION_BUILD, CLIENT_VERSION_TAG, sys_lang)` |
| [nclient.c:8221](../../../src/client/nclient.c#L8221) | `Send_version`; outbound | `Packet_printf(&wbuf, "%c%s%s%d%d%d%d%s%d%s%s%s%s", PKT_VERSION, longVersion, os_version, avg, guide_lastline, VERSION_BRANCH, VERSION_BUILD, CLIENT_VERSION_TAG, sys_lang, 			    cfg_soundpack_name, cfg_soundpack_version, cfg_musicpack_name, cfg_musicpack_version)` |
| [nclient.c:8224](../../../src/client/nclient.c#L8224) | `Send_version`; outbound | `Packet_printf(&wbuf, "%c%s%s%d%d%s%d%s%s%s%s", PKT_VERSION, longVersion, os_version, avg, guide_lastline, CLIENT_VERSION_TAG, sys_lang, 			    cfg_soundpack_name, cfg_soundpack_version, cfg_musicpack_name, cfg_musicpack_version)` |
| [nclient.c:9059](../../../src/client/nclient.c#L9059) | `Send_request_str`; outbound | `Packet_printf(&wbuf, "%c%d%s", PKT_REQUEST_STR, id, str)` |
| [nclient.c:9198](../../../src/client/nclient.c#L9198) | `Send_audio`; outbound | `Packet_printf(&wbuf, "%c%hd%hd%s%s%s%s", PKT_AUDIO, audio_sfx, audio_music, 		    cfg_soundpack_name, cfg_soundpack_version, cfg_musicpack_name, cfg_musicpack_version)` |
| [nclient.c:9220](../../../src/client/nclient.c#L9220) | `Send_font`; outbound | `Packet_printf(&wbuf, "%c%hd%s%s", PKT_FONT, use_graphics, graphic_tiles, fname)` |
| [nclient.c:9222](../../../src/client/nclient.c#L9222) | `Send_font`; outbound | `Packet_printf(&wbuf, "%c%hd%s%s", PKT_FONT, use_graphics, "NO_GRAPHICS", fname)` |

Expanded complete-call string packet entries: **114**. Scope: every `src/client/*.[ch]` file; this number is not a global server/whole-repository count.

## Appendix C — server inbound string boundary cross-check

Server `Packet_scanf` text calls retain command/format discrimination, including initial handshake and version alternatives. These independently expose the receiver side of outbound fields; no assumption that sender editor limits protect all receivers.

| Source | Owner | Full inbound call |
|---|---|---|
| [control.c:795](../../../src/server/control.c#L795) | `NewConsole` | `Packet_scanf(&console_buf, "%s", passwd)` |
| [control.c:839](../../../src/server/control.c#L839) | `NewConsole` | `Packet_scanf(&console_buf, "%d%s", &i, buf)` |
| [control.c:846](../../../src/server/control.c#L846) | `NewConsole` | `Packet_scanf(&console_buf, "%s", buf)` |
| [control.c:853](../../../src/server/control.c#L853) | `NewConsole` | `Packet_scanf(&console_buf, "%s", buf)` |
| [nserver.c:1268](../../../src/server/nserver.c#L1268) | `Contact` | `Packet_scanf(&ibuf, "%s%hu%c", real_name, &port, &ch)` |
| [nserver.c:1276](../../../src/server/nserver.c#L1276) | `Contact` | `Packet_scanf(&ibuf, "%s%s%hu", nick_name, host_name, &version)` |
| [nserver.c:2417](../../../src/server/nserver.c#L2417) | `Handle_listening` | `Packet_scanf(&connp->r, "%c%s%s%s", &type, real, nick, pass)` |
| [nserver.c:4987](../../../src/server/nserver.c#L4987) | `Receive_login` | `Packet_scanf(&connp->r, "%s", choice)` |
| [nserver.c:5704](../../../src/server/nserver.c#L5704) | `Receive_play` | `Packet_scanf(&connp->r, "%hd%hd%hd%hd%hd%hd%hd%s%s", &sex, &race, &class, &trait, &sfx, &mus, &use_graphics, graphic_tiles, fname)` |
| [nserver.c:6068](../../../src/server/nserver.c#L6068) | `Receive_file` | `Packet_scanf(&connp->r, "%s", fname)` |
| [nserver.c:6110](../../../src/server/nserver.c#L6110) | `Receive_file` | `Packet_scanf(&connp->r, "%s", fname)` |
| [nserver.c:13339](../../../src/server/nserver.c#L13339) | `Receive_inscribe` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &item, inscription)` |
| [nserver.c:14038](../../../src/server/nserver.c#L14038) | `Receive_message` | `Packet_scanf(&connp->r, "%c%S", &ch, buf)` |
| [nserver.c:14063](../../../src/server/nserver.c#L14063) | `Receive_admin_house` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &dir, buf)` |
| [nserver.c:14703](../../../src/server/nserver.c#L14703) | `Receive_special_line` | `Packet_scanf(&connp->r, "%c%c%d%s", &ch, &type, &line, srcstr)` |
| [nserver.c:15068](../../../src/server/nserver.c#L15068) | `Receive_party` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)` |
| [nserver.c:15122](../../../src/server/nserver.c#L15122) | `Receive_guild` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)` |
| [nserver.c:15157](../../../src/server/nserver.c#L15157) | `Receive_guild_config` | `Packet_scanf(&connp->r, "%c%d%d%s", &ch, &command, &flags, adder)` |
| [nserver.c:15423](../../../src/server/nserver.c#L15423) | `Receive_master` | `Packet_scanf(&connp->r, "%c%hd%s", &ch, &command, buf)` |
| [nserver.c:15670](../../../src/server/nserver.c#L15670) | `Receive_ping` | `Packet_scanf(&connp->r, "%c%c%d%d%d%S", &ch, &pong, &id, &tim, &utim, &buf)` |
| [nserver.c:16054](../../../src/server/nserver.c#L16054) | `Receive_change_password` | `Packet_scanf(&connp->r, "%c%s%s", &ch, old_pass, new_pass)` |
| [nserver.c:16224](../../../src/server/nserver.c#L16224) | `Receive_request_str` | `Packet_scanf(&connp->r, "%c%d%s", &ch, &id, str)` |
| [nserver.c:16607](../../../src/server/nserver.c#L16607) | `Receive_audio` | `Packet_scanf(&connp->r, "%c%hd%hd%s%s%s%s", &ch, &sfx, &mus, 		    cfg_soundpack_name, cfg_soundpack_version, cfg_musicpack_name, cfg_musicpack_version)` |
| [nserver.c:16640](../../../src/server/nserver.c#L16640) | `Receive_font` | `Packet_scanf(&connp->r, "%c%hd%s%s", &ch, &use_graphics, graphic_tiles, fname)` |
| [nserver.c:16741](../../../src/server/nserver.c#L16741) | `Receive_version` | `Packet_scanf(&connp->r, "%c%s%s%d%d%s%d%s%s%s%s", &ch, version, os_version, &avg, &guide_lastline, v_tag, &sys_lang, 		    cfg_soundpack_name, cfg_soundpack_version, cfg_musicpack_name, cfg_musicpack_version)` |
| [nserver.c:16747](../../../src/server/nserver.c#L16747) | `Receive_version` | `Packet_scanf(&connp->r, "%c%s%s%d%d%d%d%s%d%s%s%s%s", &ch, version, os_version, &avg, &guide_lastline, &v_branch, &v_build, v_tag, &sys_lang, 		    cfg_soundpack_name, cfg_soundpack_version, cfg_musicpack_name, cfg_musicpack_version)` |
| [nserver.c:16753](../../../src/server/nserver.c#L16753) | `Receive_version` | `Packet_scanf(&connp->r, "%c%s%s%d%d%d%d%s%d", &ch, version, os_version, &avg, &guide_lastline, &v_branch, &v_build, v_tag, &sys_lang)` |
| [nserver.c:16758](../../../src/server/nserver.c#L16758) | `Receive_version` | `Packet_scanf(&connp->r, "%c%s%s%d", &ch, version, os_version, &avg)` |
| [nserver.c:16763](../../../src/server/nserver.c#L16763) | `Receive_version` | `Packet_scanf(&connp->r, "%c%s%s", &ch, version, os_version)` |

## Source snapshot

HEAD: `4211671279ff820575c32239272bca68cf8762f7`. SHA-256 of audited core files (working tree; existing unrelated modifications were left alone):

```text
fed6de422b6fa1b945989507425f4ac513895a7a44a493a7bf94ab0928052c4e  src/common/defines.h
8e58853b9768caf83351705632441630a1f1d16f556bb1f3f1db9033c998398e  src/common/sockbuf.c
a948ad57c78fbb22d04adddabd149cec85f311ead463ac17c30ee7b7c566f05c  src/client/c-util.c
fe190ffdb11ae636ec7cf96964a0e4e3cc895fd74e9bbdcd23dac5b51a9e9a8a  src/client/c-cmd.c
c3d026f307d71df84e75b10f0c99f21216fd09c3bbf90536ead7e4779babfc81  src/client/c-birth.c
ccbd1dcfbc6d743419719264991dd62960670a2eb66e08ca6177dd1a03887968  src/client/c-files.c
51eded7b8d2ed221f9995bd777aa4f7a23af9e876296f28ee2db119d82a28672  src/client/c-init.c
a0c6e85ec05944a1bc0e3115012a2a57b69f16f9c14e8a59bc5baabc0029ce60  src/client/nclient.c
35aff58dab60cdecc39002cee153fbf0e2b2829f0d226b1589a9094e7e0fab36  src/client/c-spell.c
83740e2fe5a36e340fbacabe258e5f9180035e84b4749f793044e5f12db831f7  src/client/c-script.c
7dd95056e29f2031fe22d1d83b9e2109799d8f13d8da08319c00c076b36ae9e7  src/client/snd-sdl3.c
dcf87b7b2cfd6812a30e58cebd55527048da8e37eedfaa5637398c140dfb5891  src/client/client.c
2bd3b7c1f4c7cf5a27c144425d39e49c967fbafbf5ec44f199e8f67c745206e9  src/server/nserver.c
bb50657f3cef7a3172fd9fc8aa0832fe9cb23aeb09f79a2b6a49cce304e43918  src/server/party.c
256b0aceeda0f69d9b15a389ca773fa5dc29a9dcccf8a78b7ac96c1ebf628e70  src/server/xtra1.c
```
