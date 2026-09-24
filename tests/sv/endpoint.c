#include "input/endpoint.h"
#include "input/confirm.h"
#include <assert.h>
#include <string.h>

static void selection_survives_ping_and_refresh(void)
{
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint, 18348);
    const SvServer first[] = {{"eu.tomenet.eu", 18348, "Europe", 0, 2},
                              {"us.tomenet.eu", 18349, "America", 0, 1}};
    assert(sv_endpoint_servers(&endpoint, first, 2));
    assert(sv_endpoint_choose(&endpoint, 1));
    assert(!strcmp(endpoint.host, "us.tomenet.eu") && endpoint.port == 18349 && endpoint.protocol == 1);
    assert(sv_endpoint_ping(&endpoint, "us.tomenet.eu", 18349, 42));
    assert(endpoint.selected == 1 && endpoint.servers[1].ping_ms == 42);
    const SvServer updated[] = {{"us.tomenet.eu", 18349, "America", 12, 1},
                                {"eu.tomenet.eu", 18348, "Europe", 20, 2}};
    assert(sv_endpoint_servers(&endpoint, updated, 2));
    assert(endpoint.selected == 0 && endpoint.port == 18349);
    assert(!strcmp(endpoint.host, "us.tomenet.eu"));
    SvServer malformed = {0};
    memset(malformed.host,'x',sizeof(malformed.host));
    malformed.port = 18348;
    assert(!sv_endpoint_servers(&endpoint,&malformed,1));
    assert(endpoint.selected == 0 && endpoint.server_count == 2);
}

static void manual_cancel_never_chooses_default(void)
{
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint, 18348);
    sv_endpoint_manual(&endpoint);
    assert(endpoint.phase == SV_ENDPOINT_MANUAL);
    sv_endpoint_key(&endpoint, SV_EDIT_CANCEL);
    assert(endpoint.phase == SV_ENDPOINT_CANCELLED && !endpoint.host[0]);
}

static void effective_port_keeps_startup_override(void)
{
    SvEndpoint endpoint;
    sv_endpoint_begin(&endpoint,19000);
    assert(sv_endpoint_argument(&endpoint,"host.example"));
    assert(endpoint.port == 19000 && endpoint.protocol == 2);
    sv_endpoint_begin(&endpoint,19000);
    assert(sv_endpoint_argument(&endpoint,"host.example:18349"));
    assert(endpoint.port == 18349);
    sv_endpoint_begin(&endpoint,19000);
    assert(!sv_endpoint_argument(&endpoint,"host.example:0"));
    assert(!endpoint.host[0]);
}

static void bounded_field_uses_selection_and_exact_bytes(void)
{
    SvTextField field;
    sv_text_begin(&field, "abc", 5, false);
    assert(sv_text_select(&field, 1, 3));
    assert(sv_text_paste(&field, "WXYZQ", 5) == SV_TEXT_TRUNCATED);
    assert(!strcmp(field.bytes, "aWXYZ") && field.cursor == 5);
    assert(sv_text_insert_utf8(&field, "\xE2\x82\xAC") == SV_TEXT_ENCODING_ERROR);
    assert(!strcmp(field.bytes, "aWXYZ"));
    sv_text_key(&field, SV_EDIT_HOME);
    sv_text_key(&field, SV_EDIT_DELETE);
    assert(!strcmp(field.bytes, "WXYZ"));
}

static void history_search_keeps_query_until_match_is_chosen(void)
{
    SvTextHistory history = {0};
    SvTextField field;
    sv_text_begin_history(&field,&history,"",79,false);
    assert(sv_text_insert_utf8(&field,"US.example") == SV_TEXT_OK);
    sv_text_key(&field,SV_EDIT_ACCEPT);
    sv_text_begin_history(&field,&history,"",79,false);
    assert(sv_text_insert_utf8(&field,"us*") == SV_TEXT_OK);
    sv_text_key(&field,SV_EDIT_SEARCH);
    assert(field.searching && !strcmp(field.preview,"US.example"));
    assert(!strcmp(field.bytes,"us*"));
    sv_text_key(&field,SV_EDIT_ACCEPT);
    assert(!field.searching && !field.accepted && !strcmp(field.bytes,"US.example"));
    sv_text_key(&field,SV_EDIT_ACCEPT);
    assert(field.accepted);
    for (int i = 0; i < SV_TEXT_HISTORY; ++i) {
        sv_text_begin_history(&field,&history,"",79,false);
        assert(sv_text_insert_utf8(&field,"entry") == SV_TEXT_OK);
        sv_text_key(&field,SV_EDIT_ACCEPT);
    }
    sv_text_begin_history(&field,&history,"draft",79,false);
    sv_text_key(&field,SV_EDIT_HISTORY_FORWARD);
    assert(!strcmp(field.bytes,"draft"));
}

int main(void)
{
    selection_survives_ping_and_refresh();
    manual_cancel_never_chooses_default();
    effective_port_keeps_startup_override();
    bounded_field_uses_selection_and_exact_bytes();
    history_search_keeps_query_until_match_is_chosen();
    SvConfirm confirm;
    sv_confirm_begin(&confirm, SV_CONFIRM_STRICT, false);
    assert(sv_confirm_key(&confirm, 27) == SV_CONFIRM_WAIT);
    assert(sv_confirm_key(&confirm, 'y') == SV_CONFIRM_YES);
    assert(confirm.flush_input);
    sv_confirm_begin(&confirm, SV_CONFIRM_DEFAULT_YES, false);
    assert(sv_confirm_key(&confirm, 27) == SV_CONFIRM_YES);
    sv_confirm_begin(&confirm, SV_CONFIRM_TERNARY, true);
    assert(sv_confirm_key(&confirm, 'a') == SV_CONFIRM_ALL);
    sv_confirm_begin(&confirm, SV_CONFIRM_TERNARY, true);
    assert(sv_confirm_key(&confirm, 27) == SV_CONFIRM_NO);
    return 0;
}
