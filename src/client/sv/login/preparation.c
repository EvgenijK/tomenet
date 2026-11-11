#include "login/flow-internal.h"
#include "protocol/protocol.h"
#include "../../../common/pack.h"
#include <stdlib.h>
#include <string.h>

SvSessionPreparation *sv_preparation_create(SvLoginFlow *flow)
{
    SvSessionPreparation *p=calloc(1,sizeof(*p));
    if (!p) return NULL;
    p->owner=flow;p->lua=flow->options.lua;
    p->root=SDL_strdup(flow->options.root?flow->options.root:flow->profile_root);
    p->library=SDL_strdup(flow->options.library?flow->options.library:"");
    if (flow->options.preferences) p->application=sv_preferences_clone(flow->options.preferences,true);
    p->app=sv_app_create((SvAlertSink){0});
    if (!p->root || !p->library || !p->app ||
        (flow->options.preferences && !p->application) ||
        sv_app_open(p->app,sv_login_version(flow->attempt.login))!=SV_OK) goto fail;
    uint64_t app_generation=sv_app_view(p->app).generation;
    if (sv_app_set_character_setup(p->app,app_generation,
        sv_contact_socket_setup(flow->attempt.connection))!=SV_OK) goto fail;
    const SvPreferences *character=flow->attempt.preferences;
    const SvSettingsOptions *options=character?sv_preferences_options(character):flow->options.settings;
    bool disable=false,thin=false;
    if (options) {
        int slot=sv_settings_option_slot("disable_flush");
        if(slot>=0)disable=options->values[slot];
        slot=sv_settings_option_slot("thin_down_flush");
        if(slot>=0)thin=options->values[slot];
    }
    sv_app_set_flush_options(p->app,(SvFlushOptions){disable,thin});
    if(character && sv_app_keymap_preferences(p->app,character,&p->keymap_cursor,true)!=SV_OK)goto fail;
    const SvLoginCharacter *selected=sv_login_selected_character(flow->attempt.login);
    if(selected)sv_app_set_identity_name(p->app,app_generation,selected->name);
    p->view=(SvPreparationView){.generation=flow->generation,.app_generation=app_generation,
        .early_profile=flow->attempt.settings_applied,.early_init=flow->options.lua_early_init};
    if(flow->options.audio)p->view.audio=*flow->options.audio;
    const SvSettingsConfig *config=flow->options.client_config;
    if(config) {
        SDL_strlcpy(p->resources.requested_text_font,config->text_font,sizeof(p->resources.requested_text_font));
        SDL_strlcpy(p->resources.requested_map_font,config->map_font,sizeof(p->resources.requested_map_font));
        SDL_strlcpy(p->resources.requested_tiles,config->graphic_tiles,sizeof(p->resources.requested_tiles));
        SDL_strlcpy(p->resources.requested_sound_pack,config->soundpack_folder,sizeof(p->resources.requested_sound_pack));
        SDL_strlcpy(p->resources.requested_music_pack,config->musicpack_folder,sizeof(p->resources.requested_music_pack));
        p->resources.requested_graphics=config->graphics;
        p->resources.requested_sound_subset=config->soundpack_subset;
        p->resources.requested_music_subset=config->musicpack_subset;
    }
    if(flow->options.effective_text_font)SDL_strlcpy(p->resources.effective_text_font,
        flow->options.effective_text_font,sizeof(p->resources.effective_text_font));
    if(flow->options.map_font)p->resources.map_font=*flow->options.map_font;
    if(character)sv_map_font_definitions(&p->resources.map_font.definitions,character);
    p->resources.effective_graphics=flow->options.graphics_mode;
    if(flow->options.graphics_tiles)SDL_strlcpy(p->resources.effective_tiles,
        flow->options.graphics_tiles,sizeof(p->resources.effective_tiles));
    p->graphics=flow->options.graphics_owner;
    memcpy(p->view.version,sv_login_version(flow->attempt.login),sizeof(p->view.version));
    p->view.prefix_config=sv_login_startup_prefix_config(flow->attempt.login);
    for(unsigned source=0;source<2;++source) {
        const SvPreferences *prefs=source?character:p->application;
        p->effect_count[source]=sv_preferences_effect_count(prefs);
        p->acked[source]=calloc(p->effect_count[source]?p->effect_count[source]:1,1);
        if(!p->acked[source])goto fail;
    }
    return p;
fail:
    p->owner=NULL;sv_session_preparation_destroy(p);return NULL;
}

SvPreparationView sv_session_preparation_view(const SvSessionPreparation *p)
{
    if(!p)return (SvPreparationView){0};
    SvPreparationView view=p->view;
    view.presentation=sv_app_view(p->app);
    view.deferred_bytes=p->deferred_size-p->deferred_at;
    view.deferred_frames=p->frame_count-p->frame_at;
    if(p->owner) {
        const SvSessionWire *wire=&p->owner->attempt.wire;
        SvOutput login_input=sv_login_take_remaining(p->owner->attempt.login,NULL,0);
        SvOutput login_output=sv_login_take_output(p->owner->attempt.login,NULL,0);
        SvOutput app_output=sv_app_take_output(p->app,view.app_generation,NULL,0);
        view.retained_input=p->input_size+wire->remaining_size-wire->remaining_at+
            (login_input.result==SV_OUTPUT_TOO_SMALL?login_input.size:0)+
            SV_PROTOCOL_CAPACITY-sv_app_receive_capacity(p->app);
        SvLoginTransfer transfer=p->owner->attempt.transfer;
        SvOutput file_output=transfer.take_output?
            transfer.take_output(transfer.context,NULL,0):(SvOutput){SV_WAITING,0};
        view.pending_output=wire->size-wire->sent+login_output.size+app_output.size+file_output.size;
    }
    return view;
}
const SvPreferences *sv_session_preparation_preferences(const SvSessionPreparation *p,bool application)
{ return p?(application?p->application:p->owner->attempt.preferences):NULL; }
const SvInputHistory *sv_session_preparation_history(const SvSessionPreparation *p)
{ return p&&p->owner?p->owner->attempt.history:NULL; }
const SvStartupIns *sv_session_preparation_inscriptions(const SvSessionPreparation *p)
{ return p?p->inscriptions:NULL; }
SvApp *sv_session_preparation_app(SvSessionPreparation *p){return p?p->app:NULL;}
const SvPreparationResources *sv_session_preparation_resources(const SvSessionPreparation *p)
{ return p?&p->resources:NULL; }
const SvMapFont *sv_session_preparation_map_font(const SvSessionPreparation *p)
{ return p?&p->resources.map_font:NULL; }
SvGraphics *sv_session_preparation_graphics(SvSessionPreparation *p)
{ return p?p->graphics:NULL; }

const SvLuaReload *sv_session_preparation_lua(const SvSessionPreparation *p){return p?p->lua:NULL;}
SvResult sv_session_preparation_ack(SvSessionPreparation *p,bool application,size_t index)
{
    unsigned source=application?0:1;
    if(!p || index>=p->effect_count[source])return SV_INVALID;
    p->acked[source][index]=1;return SV_OK;
}
bool sv_session_preparation_pending(const SvSessionPreparation *p,bool application,size_t index)
{
    unsigned source=application?0:1;
    return p&&index<p->effect_count[source]&&!p->acked[source][index];
}
void sv_session_preparation_destroy(SvSessionPreparation *p)
{
    if(!p)return;
    SvLoginFlow *flow=p->owner;
    if(flow) { flow->preparation=NULL;sv_login_flow_destroy(flow); }
    sv_app_destroy(p->app);
    sv_preferences_destroy(p->application);sv_startup_ins_destroy(p->inscriptions);
    free(p->acked[0]);free(p->acked[1]);
    if(p->owns_lua)sv_lua_reload_destroy(p->lua);
    if(p->destroy_native)p->destroy_native(p->native_owner);
    SDL_free(p->root);SDL_free(p->library);free(p);
}

static void preparation_results(SvSessionPreparation *p)
{
    SvLoginFlow *flow=p->owner;
    p->view.begin=sv_login_lua_begin_result(flow->attempt.login);
    memcpy(p->view.server_flags,sv_login_flags(flow->attempt.login),sizeof(p->view.server_flags));
    p->view.transfers=sv_login_transfer_summary(flow->attempt.login);
    p->view.transfer_failed=p->view.transfers.failures!=0;
    p->view.runtime_ready=p->lua&&sv_lua_ready(sv_lua_reload_runtime(p->lua));
    p->view.guide_valid=p->lua&&sv_lua_reload_guide_valid(p->lua);
    p->view.guide_required=p->lua&&sv_lua_reload_guide_required(p->lua);
    p->view.guide_status=p->view.guide_required?
        (p->view.guide_valid?SV_LUA_OK:p->view.transfers.completion.guide_status):SV_LUA_NOT_READY;
}

static SvResult preparation_app_output(SvSessionPreparation *p)
{
    SvResult pumped=sv_login_tick(p->owner->attempt.login);
    if(pumped!=SV_OK)return pumped;
    SvOutput pending=sv_app_take_output(p->app,p->view.app_generation,NULL,0);
    if(pending.result==SV_WAITING)return SV_OK;
    if(pending.result!=SV_OUTPUT_TOO_SMALL)return pending.result;
    if(pending.size>sv_login_output_capacity(p->owner->attempt.login))return SV_BACKPRESSURE;
    unsigned char bytes[SV_PROTOCOL_CAPACITY];
    SvOutput output=sv_app_take_output(p->app,p->view.app_generation,bytes,sizeof(bytes));
    if(output.result!=SV_OK)return output.result;
    return sv_login_queue_prepared_output(p->owner->attempt.login,bytes,output.size);
}

SvResult sv_preparation_poll(SvSessionPreparation *p)
{
    SvLoginFlow *flow=p->owner;
    p->view.startup_complete=sv_login_play_stream_complete(flow->attempt.login);
    preparation_results(p);
    if(!p->view.startup_complete)return SV_WAITING;
    p->view.prefix_config=sv_login_startup_prefix_config(flow->attempt.login);
    SvOutput pending=sv_login_take_remaining(flow->attempt.login,NULL,0);
    if(pending.result==SV_OUTPUT_TOO_SMALL || (pending.result==SV_OK && pending.size)) {
        if(pending.size>sizeof(p->input)-p->input_size)return SV_INPUT_OVERFLOW;
        pending=sv_login_take_remaining(flow->attempt.login,p->input+p->input_size,sizeof(p->input)-p->input_size);
        if(pending.result!=SV_OK)return pending.result;
        p->input_size+=pending.size;
    }
    /* Stop at an unsupported leading packet. No unknown recovery, byte scan or
     * inferred packet length can discard an unimplemented baseline consumer. */
    for(size_t budget=0;budget<4096;++budget) {
        SvResult forwarded=preparation_app_output(p);
        if(forwarded==SV_BACKPRESSURE)break;
        if(forwarded!=SV_OK)return forwarded;
        /* Identity is the bounded ingress boundary. Finish any already-started
         * transfer or coalesced FILE frames, retaining the ordinary live tail. */
        if(p->view.identity_ready && !p->view.transfers.active &&
           (!p->input_size || p->input[0]!=PKT_FILE)) {
            if(p->native_owner)p->controls.batch=false;
            break;
        }
        if(p->packet_active) {
            SvStep step=p->native_owner?sv_native_controls_step(&p->controls,p->app,4096):sv_app_step(p->app,1);
            if(step.result!=SV_OK && step.result!=SV_WAITING && step.result!=SV_BACKPRESSURE)return step.result;
            if(step.processed) {
                p->packet_active=false;++p->view.ingress_sequence;
            }
            p->view.presentation=sv_app_view(p->app);
            p->view.identity_ready=p->view.presentation.identity.available;
            if(p->view.identity_ready && !p->view.transfers.active &&
               (!p->input_size || p->input[0]!=PKT_FILE)) {
                if(p->native_owner)p->controls.batch=false;
                break;
            }
            if(step.result==SV_BACKPRESSURE)break;
            /* Native pacing WAITING did not ask the decoder for another byte.
             * Keep its current packet boundary until that deadline expires. */
            if(p->native_owner && SDL_GetTicksNS()<p->controls.due_ns)break;
            if(step.processed)continue;
        }
        if(!p->input_size) {
            SvOutput read;
            SvSessionWire *wire=&flow->attempt.wire;
            if(wire->remaining_at<wire->remaining_size) {
                size_t n=wire->remaining_size-wire->remaining_at;
                if(n>sizeof(p->input))n=sizeof(p->input);
                memcpy(p->input,wire->remaining+wire->remaining_at,n);wire->remaining_at+=n;
                read=(SvOutput){SV_OK,n};
            } else read=sv_contact_socket_read(flow->attempt.connection,p->input,sizeof(p->input));
            if(read.result==SV_WAITING) {
                if(p->native_owner)p->controls.batch=false;
                break;
            }
            if(read.result!=SV_OK)return read.result;
            p->input_size=read.size;
        }
        if(!p->packet_active && p->input[0]==PKT_FILE) {
            size_t consumed=0;
            SvResult received=sv_session_preparation_receive_file(p,p->view.generation,
                p->input,p->input_size,&consumed);
            if(received==SV_OK) {
                if(!consumed || consumed>p->input_size)return SV_DECODE_ERROR;
                memmove(p->input,p->input+consumed,p->input_size-consumed);p->input_size-=consumed;
                SvResult pumped=sv_session_preparation_pump(p,p->view.generation);
                if(pumped!=SV_OK && pumped!=SV_BACKPRESSURE)return pumped;
                if(pumped==SV_BACKPRESSURE)break;
                continue;
            }
            if(received==SV_BACKPRESSURE)break;
            if(received!=SV_WAITING)return received;
            if(p->input_size==sizeof(p->input))return SV_INPUT_OVERFLOW;
            SvSessionWire *wire=&flow->attempt.wire;
            size_t room=sizeof(p->input)-p->input_size;
            SvOutput more;
            if(wire->remaining_at<wire->remaining_size) {
                size_t n=wire->remaining_size-wire->remaining_at;if(n>room)n=room;
                memcpy(p->input+p->input_size,wire->remaining+wire->remaining_at,n);
                wire->remaining_at+=n;more=(SvOutput){SV_OK,n};
            } else more=sv_contact_socket_read(flow->attempt.connection,p->input+p->input_size,room);
            if(more.result==SV_WAITING) {
                if(p->native_owner)p->controls.batch=false;
                break;
            }
            if(more.result!=SV_OK)return more.result;
            p->input_size+=more.size;continue;
        }
        if(!p->packet_active && p->input[0]==PKT_QUIT) {
            /* Baseline reason buffer is MAX_CHARS_WIDE (160 including NUL).
             * This failure-only consumer never scans outside the leading frame. */
            size_t available=p->input_size-1;
            if(available>sizeof(p->view.failure_reason))available=sizeof(p->view.failure_reason);
            const unsigned char *end=memchr(p->input+1,0,available);
            if(end) {
                size_t length=(size_t)(end-p->input-1);
                memcpy(p->view.failure_reason,p->input+1,length);
                p->view.failure_reason[length]=0;
                if(!length)SDL_strlcpy(p->view.failure_reason,"Server closed preparation",sizeof(p->view.failure_reason));
                size_t consumed=length+2;
                memmove(p->input,p->input+consumed,p->input_size-consumed);p->input_size-=consumed;
                return SV_CLOSED;
            }
            if(available==sizeof(p->view.failure_reason))return SV_DECODE_ERROR;
            SvSessionWire *wire=&flow->attempt.wire;
            size_t room=1+sizeof(p->view.failure_reason)-p->input_size;
            SvOutput more;
            if(wire->remaining_at<wire->remaining_size) {
                size_t n=wire->remaining_size-wire->remaining_at;if(n>room)n=room;
                memcpy(p->input+p->input_size,wire->remaining+wire->remaining_at,n);
                wire->remaining_at+=n;more=(SvOutput){SV_OK,n};
            } else more=sv_contact_socket_read(flow->attempt.connection,p->input+p->input_size,room);
            if(more.result==SV_WAITING) {
                if(p->native_owner)p->controls.batch=false;
                break;
            }
            if(more.result!=SV_OK)return more.result;
            p->input_size+=more.size;continue;
        }
        if(!p->packet_active && !sv_protocol_supports_packet(p->input[0])) {
            if(!sv_startup_prefix_supports(p->input[0])) {
                p->view.unsupported_packet=p->input[0];break;
            }
            size_t consumed=0;
            SvResult decoded=sv_startup_prefix_decode(&p->view.prefix_config,
                p->input,p->input_size,&consumed);
            if(decoded==SV_OK) {
                if(!consumed || consumed>p->input_size)return SV_DECODE_ERROR;
                if(p->frame_count==SV_PREPARATION_DEFERRED_FRAMES ||
                   consumed>sizeof(p->deferred)-p->deferred_size)return SV_INPUT_OVERFLOW;
                memcpy(p->deferred+p->deferred_size,p->input,consumed);
                p->deferred_size+=consumed;
                p->frames[p->frame_count++]=(SvPreparationFrame){
                    ++p->view.ingress_sequence,p->input[0],consumed};
                memmove(p->input,p->input+consumed,p->input_size-consumed);p->input_size-=consumed;
                continue;
            }
            if(decoded!=SV_WAITING)return decoded;
            if(p->input_size==sizeof(p->input))return SV_INPUT_OVERFLOW;
            SvSessionWire *wire=&flow->attempt.wire;
            size_t room=sizeof(p->input)-p->input_size;
            SvOutput more;
            if(wire->remaining_at<wire->remaining_size) {
                size_t n=wire->remaining_size-wire->remaining_at;if(n>room)n=room;
                memcpy(p->input+p->input_size,wire->remaining+wire->remaining_at,n);
                wire->remaining_at+=n;more=(SvOutput){SV_OK,n};
            } else more=sv_contact_socket_read(flow->attempt.connection,p->input+p->input_size,room);
            if(more.result==SV_WAITING) {
                if(p->native_owner)p->controls.batch=false;
                break;
            }
            if(more.result!=SV_OK)return more.result;
            p->input_size+=more.size;continue;
        }
        SvResult accepted=sv_app_receive(p->app,p->view.app_generation,p->input,1);
        if(accepted!=SV_OK)return accepted;
        memmove(p->input,p->input+1,--p->input_size);p->packet_active=true;
    }
    if(p->view.identity_ready&&!p->inscriptions) {
        SvCharacterIdentity identity=p->view.presentation.identity;
        const SvContactSetup *setup=sv_contact_socket_setup(flow->attempt.connection);
        p->inscriptions=sv_startup_ins_load(p->root,identity.name,
            setup->classes[identity.class_id].title,setup->trait_count?setup->traits[identity.trait].title:NULL,
            setup->races[identity.race].title);
        if(!p->inscriptions)return SV_NO_MEMORY;
        p->view.profile_ready=true;
    }
    p->view.presentation=sv_app_view(p->app);
    p->view.deferred_bytes=p->deferred_size-p->deferred_at;
    p->view.deferred_frames=p->frame_count-p->frame_at;
    p->view.retained_input=p->input_size+
        flow->attempt.wire.remaining_size-flow->attempt.wire.remaining_at+
        SV_PROTOCOL_CAPACITY-sv_app_receive_capacity(p->app);
    SvSessionWire *wire=&flow->attempt.wire;
    SvOutput login_output=sv_login_take_output(flow->attempt.login,NULL,0);
    SvOutput app_output=sv_app_take_output(p->app,p->view.app_generation,NULL,0);
    /* Preserve packet-order replies: app output joins the existing login
     * queue before a subsequent FILE frame can produce its ACK/SUM. */
    SvResult forwarded=preparation_app_output(p);
    if(forwarded!=SV_OK && forwarded!=SV_BACKPRESSURE)return forwarded;
    login_output=sv_login_take_output(flow->attempt.login,NULL,0);
    app_output=sv_app_take_output(p->app,p->view.app_generation,NULL,0);
    p->view.pending_output=wire->size-wire->sent+login_output.size+app_output.size;
    preparation_results(p);
    p->view.complete=p->view.startup_complete&&p->view.identity_ready&&p->view.profile_ready&&
        !p->view.transfers.active&&p->view.runtime_ready&&!p->view.transfer_failed&&
        (!p->view.guide_required||p->view.guide_valid);
    return p->view.identity_ready?SV_OK:SV_WAITING;
}

SvContactSocket *sv_session_preparation_connection(SvSessionPreparation *p)
{ return p&&p->owner?p->owner->attempt.connection:NULL; }
SvOutput sv_session_preparation_take_input(SvSessionPreparation *p,void *bytes,size_t capacity)
{
    if(!p || !p->owner)return (SvOutput){SV_INVALID,0};
    if(p->frame_at<p->frame_count)return (SvOutput){SV_BUSY,0};
    SvSessionWire *wire=&p->owner->attempt.wire;
    size_t trailing=wire->remaining_size-wire->remaining_at;
    size_t count=p->input_size+trailing;
    if(!count)return (SvOutput){SV_WAITING,0};
    if(capacity<count)return (SvOutput){SV_OUTPUT_TOO_SMALL,count};
    if(!bytes)return (SvOutput){SV_INVALID,count};
    memcpy(bytes,p->input,p->input_size);
    if(trailing)memcpy((unsigned char *)bytes+p->input_size,wire->remaining+wire->remaining_at,trailing);
    p->input_size=0;wire->remaining_at=wire->remaining_size;
    return (SvOutput){SV_OK,count};
}
SvOutput sv_session_preparation_take_output(SvSessionPreparation *p,void *bytes,size_t capacity)
{
    if(!p || !p->owner)return (SvOutput){SV_INVALID,0};
    SvSessionWire *wire=&p->owner->attempt.wire;
    size_t pending=wire->size-wire->sent;
    if(pending) {
        if(capacity<pending)return (SvOutput){SV_OUTPUT_TOO_SMALL,pending};
        if(!bytes)return (SvOutput){SV_INVALID,pending};
        memcpy(bytes,wire->sending+wire->sent,pending);wire->sent=wire->size;
        return (SvOutput){SV_OK,pending};
    }
    SvResult forwarded=preparation_app_output(p);
    if(forwarded!=SV_OK && forwarded!=SV_BACKPRESSURE)return (SvOutput){forwarded,0};
    return sv_login_take_output(p->owner->attempt.login,bytes,capacity);
}

SvOutput sv_session_preparation_take_deferred(SvSessionPreparation *p,
    SvPreparationFrame *frame,void *bytes,size_t capacity)
{
    if(!p || !frame)return (SvOutput){SV_INVALID,0};
    if(p->frame_at==p->frame_count)return (SvOutput){SV_WAITING,0};
    SvPreparationFrame next=p->frames[p->frame_at];
    *frame=next;
    if(capacity<next.size)return (SvOutput){SV_OUTPUT_TOO_SMALL,next.size};
    if(!bytes)return (SvOutput){SV_INVALID,next.size};
    memcpy(bytes,p->deferred+p->deferred_at,next.size);
    p->deferred_at+=next.size;++p->frame_at;
    return (SvOutput){SV_OK,next.size};
}

SvResult sv_session_preparation_receive_file(SvSessionPreparation *p,uint64_t generation,
    const void *bytes,size_t size,size_t *consumed)
{
    if(!consumed)return SV_INVALID;
    *consumed=0;
    if(!p || !p->owner)return SV_INVALID;
    if(generation!=p->view.generation)return SV_STALE;
    SvResult forwarded=preparation_app_output(p);
    if(forwarded!=SV_OK)return forwarded;
    SvLoginTransfer *transfer=&p->owner->attempt.transfer;
    if(!transfer->receive)return SV_INVALID;
    SvResult received=transfer->receive(transfer->context,bytes,size,consumed);
    if(received==SV_OK)++p->view.ingress_sequence;
    preparation_results(p);
    p->view.complete=p->view.startup_complete&&p->view.identity_ready&&p->view.profile_ready&&
        !p->view.transfers.active&&p->view.runtime_ready&&!p->view.transfer_failed&&
        (!p->view.guide_required||p->view.guide_valid);
    return received;
}
SvResult sv_session_preparation_pump(SvSessionPreparation *p,uint64_t generation)
{
    if(!p || !p->owner)return SV_INVALID;
    if(generation!=p->view.generation)return SV_STALE;
    SvResult result=sv_login_tick(p->owner->attempt.login);
    preparation_results(p);
    p->view.complete=p->view.startup_complete&&p->view.identity_ready&&p->view.profile_ready&&
        !p->view.transfers.active&&p->view.runtime_ready&&!p->view.transfer_failed&&
        (!p->view.guide_required||p->view.guide_valid);
    return result;
}

bool sv_session_preparation_was_applied(const SvSessionPreparation *p,uint64_t sequence)
{
    if(!p || !sequence || sequence>p->view.ingress_sequence)return false;
    for(size_t i=0;i<p->frame_count;++i)
        if(p->frames[i].sequence==sequence)return false;
    return true;
}
