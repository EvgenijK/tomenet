# Encoding and the server contract: bounded source audit

Local source investigation, 2026-09-15. This records facts, not a selected encoding or a promise of Unicode support. No runtime interoperability was tested.

## Three different operations

1. Font lookup chooses the drawing for a numeric glyph ID. It does not select the encoding of chat, commands or passwords.
2. Converting received text bytes to displayable Unicode can be a renderer-only projection: preserving the original bytes leaves the server payload and gameplay IDs unchanged.
3. Converting newly entered Unicode to outbound bytes changes what the server receives. The chosen conversion therefore affects field lengths, comparisons, command arguments, stored names and credential verification.

## Wire text

[Packet_printf](../../../src/common/sockbuf.c:550) copies C-string bytes, including NUL; [Packet_scanf](../../../src/common/sockbuf.c:779) copies them back. Neither performs charset conversion or Unicode validation. `%s` has `MAX_CHARS` capacity (80), `%S` has `MSG_LEN` capacity (256), `%I` uses `ONAME_LEN`; these capacities include the terminating NUL and are byte capacities, not Unicode character counts. Definitions: [defines.h](../../../src/common/defines.h:235). Source review of `pack.h`, client/server login/setup and message/request paths found no text charset negotiation. Numeric glyph-transfer width negotiation is a separate mechanism.

| Route | Observed contract and high-byte limitations |
|---|---|
| Chat and slash commands | [Send_msg](../../../src/client/nclient.c:7826) sends `%c%S`; [Receive_message](../../../src/server/nserver.c:14031) obtains raw bytes and passes them to `player_talk`. [player_talk](../../../src/server/util.c:6493) splits at byte TAB; [player_talk_aux](../../../src/server/util.c:5264) uses byte `strlen`, `strchr`, prefixes and formatted string operations. No generic high-byte rejection or UTF-8 decoder was found in this receive path. This does **not** establish arbitrary Unicode support: message markers, censoring, command parsing and downstream clients remain byte oriented. |
| Account name | [Enter_player](../../../src/server/nserver.c:1379) applies [validstring](../../../src/server/nserver.c:2343): bytes outside 32 through ASCII `z` are rejected. This excludes all bytes above ASCII irrespective of signedness. Additional name checks apply. |
| Character selection/name | [PKT_LOGIN](../../../src/client/nclient.c:1618) sends `%s`; server trims, validates lengths/forbidden names and punctuation in [Receive_login](../../../src/server/nserver.c:5390), and [Check_names](../../../src/server/nserver.c:1013) requires first byte `A` through `Z` and excludes colon. These checks alone do not establish a general high-byte repertoire for every remaining position; do not claim identical validation to account names. |
| Credentials | [Verify_client](../../../src/client/nclient.c:1430) sends three `%s` fields. Password change [receiver](../../../src/server/nserver.c:16047) undoes byte obfuscation, then [account_change_password](../../../src/server/party.c:6059) checks `strlen` limits and hashes via [t_crypt](../../../src/server/party.c:1028). Account lookup compares password hashes ([party.c](../../../src/server/party.c:558)). No universal ASCII restriction or Unicode normalization was found here. Changing encoding may change the password bytes and break verification; hash/backend behavior is also configuration dependent. |
| String requests | [Send_request_str](../../../src/client/nclient.c:9057) and [Receive_request_str](../../../src/server/nserver.c:16212) use `%s`, despite client editor being called with `MAX_CHARS_WIDE - 1` ([Receive_request_str](../../../src/client/nclient.c:7214)). [handle_request_return_str](../../../src/server/xtra1.c:10726) verifies ID/type and applies request-specific byte limits: quest reply 30, guild rename 40, etc. There is no universal accepted alphabet. |
| Inscription | [Receive_inscribe](../../../src/server/nserver.c:13325) obtains `%s`, caps `INSCR_LEN - 1`, invokes item command; receiver itself does not decode Unicode or apply a universal alphabet check. Downstream inscription semantics still matter. |
| Party menu text | [Receive_party](../../../src/server/nserver.c:15051) obtains `%s`, truncates at 40 bytes and replaces bytes comparing `< 32` with underscore. Since input is plain `char`, high-byte handling depends on signedness; on signed-char builds those bytes compare negative. |

## Existing input/clipboard are not a Unicode contract

[askfor_aux](../../../src/client/c-util.c:3259) inserts one input value into one `char` position and tests `isprint`; its behavior is byte/locale dependent. [SDL3 event handling](../../../src/client/main-sdl3.c:1376) identifies `SDL_EVENT_TEXT_INPUT`'s window, but the inspected switch processes `SDL_EVENT_KEY_DOWN`, not committed UTF-8 text. This is not an implemented full Unicode editor.

[SDL3 paste](../../../src/client/c-util.c:2500) reads clipboard text into a `char` buffer byte by byte, clips by byte length, then [filters](../../../src/client/c-util.c:2526) with `*c < 32`; on signed-char builds this silently drops high bytes. It also escapes `{` and selected `:` occurrences because those bytes have gameplay/chat syntax. It contains no Unicode-to-legacy conversion. Windows uses `CF_TEXT` ([c-util.c](../../../src/client/c-util.c:2397)), further demonstrating backend-dependent input sources. These are observed limitations, not an approved future fallback.

## Glyph IDs sent to the server

[Client setup](../../../src/client/nclient.c:1728) sends object/feature/monster glyph redefinitions as attributes plus numeric IDs: `%u` for supported server versions, `%c` for older versions. [char_transfer_bytes](../../../src/client/nclient.c:1780) is computed from maximum numeric ID and controls map transfer width. These values can describe font positions or graphics IDs; a 32-bit type does not imply UTF-8 chat or Unicode scalar validity. Converting a numeric glyph ID into a different ID changes server-visible map setup; selecting a different drawing for the same ID locally does not.

## What remains unproven

There is no source evidence here for a single universal text codepage covering every source or an interoperable arbitrary-Unicode server repertoire. Field-specific accepted alphabets need separate specification where outbound Unicode conversion is desired. Bundled font metadata can inform drawing/projection, but cannot establish the network text contract. Existing byte capacity mismatches and signed-char filters should be treated as explicit implementation/acceptance questions rather than silently repaired by selecting a font encoding.
