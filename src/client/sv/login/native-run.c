#include "login/ui/login-shell.h"
#include "login/ui/scale.h"
#include "login/ui/login-layout.h"
#include "login/native-run.h"
#include "login/flow-internal.h"
#include "app.h"
#include "login/ui/native-endpoint.h"
#include "login/ui/pregame-events.h"
#include "login/account/credentials.h"
#include "login/servers/metaserver.h"
#include "login/chronicle.h"
#include "login/characters/login-interaction.h"
#include "login/characters/native-login.h"
#include "login/characters/native-name.h"
#include "login/creation/native-birth.h"
#include "login/ui/endpoint-scene.h"
#include "protocol/contact-socket.h"
#include "protocol/login.h"
#include "transfer/startup.h"
#include "protocol/login-identity.h"
#include "settings/startup.h"
#include "settings/audio.h"
#include "lua/reload.h"
#include "ui/font-editor.h"
#include "settings/graphics.h"
#include "session/map-effects.h"
#include "login/login-view.h"
#include "login/pregame.h"
#include "../../../temporary/sv/stubs/pregame-ui.h"
#include "../../../common/pack.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

static void lua_diagnostic(void *context, const SvLuaResult *result)
{
    (void)context;
    fprintf(stderr,"SV Lua%s%s: %s\n",*result->path ? " " : "",
        result->path,result->diagnostic);
}

/* FPS0 retains the baseline uncapped mode. This is pacing, not a network
 * budget: measured work is subtracted before yielding the remainder. */
static void limit_frame(Uint64 start, int fps)
{
    if (fps <= 0) return;
    Uint64 interval = 1000u / (unsigned)fps;
    Uint64 spent = SDL_GetTicks() - start;
    if (spent < interval) SDL_Delay((Uint32)(interval - spent));
}

static bool draw_scene(SDL_Renderer *renderer, SvFont *font, const SvEndpoint *endpoint,
                       const SvEndpointInput *input, SvEndpointView *view,
                       SvBirthViewport *viewport)
{
    sv_endpoint_view_native(view, input);
    SvEndpointRenderMetrics metrics = {0};
    if (!sv_endpoint_draw(renderer, font, endpoint, view, &metrics)) return false;
    if (viewport) {
        viewport->summary_max = metrics.summary_max;
        viewport->detail_max = metrics.detail_max;
        if (viewport->summary_scroll > viewport->summary_max) viewport->summary_scroll = viewport->summary_max;
        if (viewport->detail_scroll > viewport->detail_max) viewport->detail_scroll = viewport->detail_max;
    }
    return true;
}

static void wipe_password(char *password, size_t size)
{
    volatile char *bytes = password;
    while (size--) *bytes++ = 0;
}

static bool profile_first_run(const char *root)
{
    char path[4096];
    int length = SDL_snprintf(path, sizeof(path), "%s/sv/tomenet.cfg", root);
    if (length < 0 || (size_t)length >= sizeof(path))
        return true;
    SDL_PathInfo info;
    return !SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_FILE;
}

enum { SV_CREDENTIAL_SCENE_ERROR=-1, SV_CREDENTIAL_SCENE_QUIT=0,
       SV_CREDENTIAL_SCENE_ACCEPTED=1, SV_CREDENTIAL_SCENE_BACK=2 };


/* The endpoint scene only drives the private input interaction and drawing. */
static int enter_credentials(SDL_Renderer *renderer, SvFont *font, SvEndpoint *endpoint,
                             SvEndpointInput *input, SvChronicle *chronicle,
                             SvChronicleSnapshot *chronicle_view, int frame_limit,
                             char account[80], char password[80], int fps)
{
    SDL_Window *window=SDL_GetRenderWindow(renderer);
    /* Retire input queued by server selection before giving this editor an
     * owner. Native keyboard clocks may lag SDL_GetTicksNS (e.g. Wayland), so
     * their timestamps cannot identify whether a freshly polled key is stale. */
    if (!SDL_StopTextInput(window)) return -1;
    sv_pregame_input_boundary();
    if (!SDL_StartTextInput(window)) return -1;
    float field_scale=sv_login_ui_scale(SDL_GetRenderWindow(renderer));
    if(field_scale<=0)field_scale=1;
    input->text_cell=sv_font_cell_width(font,field_scale)/field_scale;
    SvEndpointView scene = {.chronicle = chronicle_view};
    SvCredentialPresentation credential_view;
    SvCredentialInput credentials;
    sv_credentials_begin(&credentials, endpoint, account, password);
    const uint64_t input_since_ns=SDL_GetTicksNS();
    int outcome = 0, frames = 0;
    for (;;) {
        Uint64 frame_start = SDL_GetTicks();
        (void)sv_chronicle_poll(chronicle, chronicle_view);
        char status[180];
        sv_credentials_poll(&credentials);
        sv_credentials_status(&credentials, status);
        scene.contact_status = status;
        input->text_error = credentials.text_error;
        input->clipboard_unavailable = credentials.clipboard_unavailable;
        sv_credentials_view(&credentials, &credential_view);
        scene.credentials = &credential_view;
        scene.private_password = sv_credentials_private_password(&credentials);
        bool drawn = draw_scene(renderer, font, endpoint, input, &scene, NULL);
        scene.credentials = NULL;
        scene.private_password = NULL;
        if (!drawn) {
            outcome = -1;
            break;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_MOUSE_WHEEL &&
                (!event.common.timestamp || event.common.timestamp >= input_since_ns) &&
                sv_endpoint_chronicle_event(input, &event)) continue;
            bool pointer_gesture=event.type==SDL_EVENT_MOUSE_BUTTON_DOWN ||
                event.type==SDL_EVENT_MOUSE_BUTTON_UP;
            if(pointer_gesture && event.common.timestamp && event.common.timestamp<input_since_ns)continue;
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                outcome=SV_CREDENTIAL_SCENE_QUIT;goto done;
            }
            if (event.type == SDL_EVENT_RENDER_DEVICE_RESET && !sv_login_shell_reset(renderer)) {
                outcome=-1;goto done;
            }
            if(sv_endpoint_links_event(input,&event))continue;
            if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) credentials.reveal=false;
            SvCredentialAction action = SV_CREDENTIAL_EDITING;
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Window *window=SDL_GetRenderWindow(renderer);
                int w,h;
                float scale=sv_login_ui_scale(window),density=SDL_GetWindowPixelDensity(window);
                if(scale<=0 || density<=0 || !SDL_GetCurrentRenderOutputSize(renderer,&w,&h))continue;
                SvLoginLayout layout=sv_login_layout(w/scale,h/scale,endpoint->server_count,endpoint->selected);
                SvLoginFormLayout form=sv_login_form_layout(layout,h/scale);
                float x=event.button.x*density/scale,y=event.button.y*density/scale;
                int field=sv_login_contains(form.account,x,y)?0:sv_login_contains(form.password,x,y)?1:-1;
                if(sv_login_contains(form.guide,x,y)) {
                    if(!SDL_OpenURL("https://www.tomenet.eu/guide.php"))
                        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Cannot open account guide: %s",SDL_GetError());
                } else if(sv_login_contains(form.reveal,x,y)) {
                    sv_credentials_focus(&credentials,2);
                    sv_credentials_toggle_reveal(&credentials);
                } else if(field>=0) {
                    SvLoginRect rect=field?form.password:form.account;
                    float cell=sv_font_cell_width(font,scale)/scale;
                    size_t caret=x>rect.x+10?(size_t)((x-rect.x-10)/cell+.5f):0;
                    sv_credentials_select(&credentials,field,caret,(SDL_GetModState()&SDL_KMOD_SHIFT)!=0);
                } else if(sv_login_contains(form.submit,x,y)) {
                    sv_credentials_focus(&credentials,3);
                    action=sv_credentials_submit(&credentials);
                } else if(sv_login_contains(form.account_back,x,y)) {
                    action=sv_credentials_cancel(&credentials);
                }
            } else action = sv_credentials_event(&credentials, &event);
            if (action != SV_CREDENTIAL_EDITING) {
                outcome = action == SV_CREDENTIAL_ACCEPTED ? SV_CREDENTIAL_SCENE_ACCEPTED : SV_CREDENTIAL_SCENE_BACK;
                goto done;
            }
        }
        if (frame_limit && ++frames >= frame_limit) break;
        limit_frame(frame_start, fps);
    }
done:
    sv_credentials_end(&credentials);
    scene.contact_status = NULL;
    scene.credentials = NULL;
    input->text_error = SV_TEXT_OK;
    input->clipboard_unavailable = false;
    SDL_StopTextInput(SDL_GetRenderWindow(renderer));
    return outcome;
}

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SvFont *font;
    SvFontSelection *fonts;
    SvSettingsAudio *audio;
    SvGraphics *graphics;
    bool owns;
} SvPreparationNative;
static void destroy_preparation_native(void *context)
{
    SvPreparationNative *native=context;
    if(native->owns) {
        sv_settings_audio_destroy(native->audio);
        sv_graphics_destroy(native->graphics);
        SDL_DestroyRenderer(native->renderer);SDL_DestroyWindow(native->window);
        sv_font_selection_destroy(native->fonts);TTF_Quit();SDL_Quit();
    }
    SDL_free(native);
}
static bool draw_preparation(void *context,SvAppView view)
{
    SvPreparationNative *native=context;
    const SvCharacterIdentity *identity=&view.identity;
    char text[512];
    SDL_snprintf(text,sizeof(text),"%s  race %d  class %d  trait %d  sex %d  mode %u  lives %d",
        identity->available?identity->name:"Preparing session",identity->race,
        identity->class_id,identity->trait,identity->sex,identity->mode,identity->lives);
    return SDL_SetRenderDrawColor(native->renderer,12,15,24,255) &&
        SDL_RenderClear(native->renderer) && sv_font_draw(native->font,native->renderer,
        text,20,40,1.0f,(SDL_Color){235,235,235,255}) &&
        (!view.paused || sv_font_draw(native->font,native->renderer,
            "Paused - press a key to continue",20,80,1.0f,(SDL_Color){235,235,235,255}));
}
static bool bind_preparation_native(SvLoginFlow *flow,SDL_Window *window,
                                     SDL_Renderer *renderer,SvFont *font)
{
    SvSessionPreparation *p=flow->preparation;
    if(!p)return true;
    if(p->native_owner)return p->native_bound;
    SvPreparationNative *native=SDL_calloc(1,sizeof(*native));
    if(!native)return false;
    *native=(SvPreparationNative){.window=window,.renderer=renderer,.font=font};
    p->native_owner=native;p->destroy_native=destroy_preparation_native;
    SDL_strlcpy(p->resources.effective_text_font,sv_font_resource(font),
        sizeof(p->resources.effective_text_font));
    p->controls=(SvNativeControls){.renderer=renderer,.input=&p->native_input,
        .context=native,.draw=draw_preparation,.retain_batch=true};
    p->native_bound=sv_native_controls_bind(&p->controls,p->app)==SV_OK;
    return p->native_bound;
}
static int run_contact(SDL_Window *window, SDL_Renderer *renderer, SvFont *font,
                       SvEndpointInput *input, SvChronicle *chronicle,
                       SvChronicleSnapshot *chronicle_view, SvLoginFlow *flow,
                       const char *profile_root, bool first_run)
{
    /* Command surfaces must not generate text for the key that opens an editor. */
    if (!SDL_StopTextInput(window)) return 1;
    sv_pregame_input_boundary();
    if (!sv_login_flow_begin(flow, profile_root, first_run, sv_font_resource(font))) return 1;
    SvEndpointView scene = {.chronicle = chronicle_view};
    SvLoginView login_view = {0};
    SvCharacterViewport character_viewport = {0};
    SvBirthView birth_view = {0};
    SvBirthViewport birth_viewport = {0};
    SvBirthSnapshot birth_before_placeholder = {0};
    int frames = 0;
    bool quit = false;
    uint64_t input_epoch = flow->input_epoch;
    while (!quit) {
        Uint64 frame_start = SDL_GetTicks();
        (void)sv_chronicle_poll(chronicle, chronicle_view);
        sv_login_flow_poll(flow);
        if(!bind_preparation_native(flow,window,renderer,font)) {
            flow->attempt.result=1;
            flow->attempt.session_error=SV_INVALID;
            flow->attempt.state=SV_SOCKET_PROTOCOL_ERROR;
            SDL_SetError("Cannot bind session preparation controls");
            break;
        }
        SvBirthSnapshot birth_snapshot;
        sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
        SvLoginFlowView flow_view;
        sv_login_flow_view(flow, &flow_view);
        scene.contact_status = flow_view.status;
        scene.preparation_identity=flow->preparation && flow->preparation->view.identity_ready &&
            flow_view.characters.live_handoff && !flow_view.failed;
        char identity_status[512];
        if(flow->preparation && flow->preparation->view.identity_ready) {
            SvCharacterIdentity identity=flow->preparation->view.presentation.identity;
            SDL_snprintf(identity_status,sizeof(identity_status),
                "%s — race %d, class %d, trait %d, sex %d, mode %u, lives %d",
                identity.name,identity.race,identity.class_id,identity.trait,
                identity.sex,identity.mode,identity.lives);
            if(scene.preparation_identity)scene.contact_status=identity_status;
            if(!flow->preparation->identity_reported) {
                printf("SV C011 prepared identity=%s race=%d class=%d trait=%d profile=%d runtime=%d\n",
                    identity.name,identity.race,identity.class_id,identity.trait,
                    flow->preparation->view.profile_ready,flow->preparation->view.runtime_ready);
                fflush(stdout);flow->preparation->identity_reported=true;
            }
        }
        if(!flow_view.failed && flow->preparation && sv_app_view(flow->preparation->app).paused)
            scene.contact_status="Paused - press a key to continue";
        scene.temporary_surface_text = flow->attempt.temporary_surface_text;
        if (input_epoch != flow->input_epoch) {
            sv_pregame_input_boundary();
            input_epoch = flow->input_epoch;
            if (flow->attempt.login_input.ordinary_name_active) {
                if (!SDL_StartTextInput(window)) return 1;
            } else SDL_StopTextInput(window);
        }
        /* Printable PAUSE acknowledgement arrives as TEXT_INPUT. Credential
         * and name scenes have their own ownership; this temporary readiness
         * ends immediately after the pause or attempt closes. */
        bool pause_text=flow->preparation && sv_app_view(flow->preparation->app).paused;
        if(pause_text && !SDL_TextInputActive(window)) {
            if(!SDL_StartTextInput(window)) {
                flow->attempt.result=1;flow->attempt.session_error=SV_INVALID;
                flow->attempt.state=SV_SOCKET_PROTOCOL_ERROR;break;
            }
        } else if(!pause_text && !flow->attempt.login_input.ordinary_name_active && SDL_TextInputActive(window))
            SDL_StopTextInput(window);
        scene.login_view = NULL;
        scene.birth_view = NULL;
        scene.birth_viewport = NULL;
        if (flow_view.has_login) {
            login_view = flow_view.characters;
            int cw,ch;
            if(!SDL_GetCurrentRenderOutputSize(renderer,&cw,&ch))break;
            float cs=sv_login_ui_scale(window);if(cs<=0)cs=1;
            SvCharacterLayout cl=sv_character_layout(sv_character_shell_layout(cw/cs,ch/cs,flow->attempt.login_input.ordinary_name_active),character_viewport.first);
            sv_character_viewport_sync(&character_viewport,cl,login_view.count,login_view.count+login_view.free_slots[0]+login_view.free_slots[1]+login_view.free_slots[2]);
            login_view.selected=character_viewport.selected;
            login_view.first=character_viewport.first;
            login_view.focus=character_viewport.focus;
            scene.login_view = &login_view;
            if (flow_view.has_birth && !scene.preparation_identity) {
                birth_view = flow_view.birth;
                scene.birth_view = &birth_view;
                int output_width, output_height;
                if (!SDL_GetCurrentRenderOutputSize(renderer, &output_width,
                                                   &output_height)) break;
                birth_viewport.logical_width=(int)(output_width / cs);
                float scale = sv_login_ui_scale(window);
                if (scale <= 0) scale = 1.0f;
                sv_birth_viewport_sync(&birth_viewport, flow->attempt.birth,
                                      (int)(output_height / scale));
                scene.birth_viewport = &birth_viewport;
            }
        }
        scene.contact_failed = flow_view.failed;
        scene.retry_available = flow_view.retry_available;
        if (flow_view.failed) SDL_StopTextInput(window);
        if (!draw_scene(renderer, font, &flow->endpoint, input, &scene, scene.birth_viewport ? &birth_viewport : NULL)) break;
        if (flow->attempt.birth_placeholder_open && !flow->attempt.birth_placeholder_drawn) {
            flow->attempt.birth_placeholder.open_count++;
            flow->attempt.birth_placeholder.owner_mask |= 1u << (unsigned)flow->attempt.birth_placeholder_owner;
            SDL_strlcpy(flow->attempt.birth_placeholder.text, flow->attempt.temporary_surface_text,
                        sizeof(flow->attempt.birth_placeholder.text));
            flow->attempt.birth_placeholder_drawn = true;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_MOUSE_WHEEL &&
                (!event.common.timestamp || event.common.timestamp >= flow->generation) &&
                !scene.birth_view && !flow->attempt.login_input.ordinary_name_active &&
                sv_endpoint_chronicle_event(input, &event)) continue;
            if((!event.common.timestamp || event.common.timestamp>=flow->generation) &&
               !scene.birth_view && !flow->attempt.login_input.ordinary_name_active &&
               (!scene.login_view || !scene.login_view->motd || scene.contact_failed ||
                flow->attempt.temporary_surface_text) &&
               sv_endpoint_links_event(input,&event))continue;
            /* Scene geometry is expressed in UI units, while SDL mouse
             * coordinates are window units. Match the renderer's display
             * scale without changing the native adapters' layout contract. */
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
                event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_MOUSE_MOTION ||
                event.type == SDL_EVENT_MOUSE_WHEEL) {
                float scale = sv_login_ui_scale(window) /
                              SDL_GetWindowPixelDensity(window);
                if (scale > 0) {
                    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                        event.window.data1 = (int)(event.window.data1 / scale);
                        event.window.data2 = (int)(event.window.data2 / scale);
                    } else if(event.type==SDL_EVENT_MOUSE_MOTION) {
                        event.motion.x/=scale;event.motion.y/=scale;
                    } else if(event.type==SDL_EVENT_MOUSE_WHEEL) {
                        event.wheel.mouse_x/=scale;event.wheel.mouse_y/=scale;
                    } else {
                        event.button.x /= scale;
                        event.button.y /= scale;
                    }
                }
            }
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                sv_login_flow_command(flow, SV_LOGIN_FLOW_EXIT);
                quit = true;
                break;
            }
            if(flow->preparation) {
                SvAppView prepared_view=sv_app_view(flow->preparation->app);
                bool redraw=(event.type==SDL_EVENT_KEY_DOWN && event.key.key==SDLK_R &&
                    (event.key.mod&SDL_KMOD_CTRL)) ||
                    (event.type==SDL_EVENT_TEXT_INPUT && event.text.text &&
                     event.text.text[0]==18 && !event.text.text[1]);
                /* This preparation surface executes redraw and pause only.
                 * General gameplay routing/adoption remains the live owner. */
                if((prepared_view.paused || (redraw && scene.preparation_identity && flow->preparation->view.complete)) &&
                   sv_native_input(&flow->preparation->native_input,flow->preparation->app,&event))continue;
            }
            /* Queue barriers retire keyboard/text at owner changes. Pointer
             * gestures retain their SDL scene-time guard. */
            bool session_input = event.type == SDL_EVENT_KEY_DOWN ||
                event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_TEXT_INPUT ||
                event.type == SDL_EVENT_TEXT_EDITING ||
                event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_MOUSE_WHEEL;
            if (session_input && !sv_pregame_keyboard_event(&event) && event.common.timestamp &&
                event.common.timestamp < flow->generation) continue;
            if (flow->attempt.birth_placeholder_open) {
                if ((event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) ||
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    sv_birth_snapshot_prepare(&birth_snapshot, flow->attempt.birth);
                    flow->attempt.birth_placeholder.draft_preserved &=
                        !memcmp(&birth_snapshot, &birth_before_placeholder, sizeof(birth_snapshot));
                    flow->attempt.birth_placeholder_open = false;
                    sv_pregame_input_boundary();
                    flow->attempt.temporary_surface_text = NULL;
                    flow->attempt.birth_placeholder.close_count++;
                }
                continue;
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT &&
                (scene.contact_failed || !scene.login_view ||
                 (!scene.login_view->overview && !scene.login_view->birth && !scene.login_view->motd))) {
                int w,h;float scale=sv_login_ui_scale(window),density=SDL_GetWindowPixelDensity(window);
                if(scale>0 && density>0 && SDL_GetCurrentRenderOutputSize(renderer,&w,&h)) {
                    SvLoginLayout layout=sv_login_layout(w/scale,h/scale,flow->endpoint.server_count,flow->endpoint.selected);
                    SvLoginFormLayout form=sv_login_form_layout(layout,h/scale);
                    float x=event.button.x,y=event.button.y;
                    if(sv_login_contains(form.back,x,y)) {quit=true;break;}
                    if(scene.retry_available && sv_login_contains(form.submit,x,y)) {sv_login_flow_command(flow,SV_LOGIN_FLOW_RETRY);quit=flow->attempt.finished;break;}
                }
            }
            if (flow_view.retry_available &&
                event.type == SDL_EVENT_KEY_DOWN &&
                !event.key.repeat && event.key.key == SDLK_R) {
                sv_login_flow_command(flow, SV_LOGIN_FLOW_RETRY);
                quit = flow->attempt.finished;
                break;
            }
            if (flow->attempt.state == SV_SOCKET_READY && flow->attempt.login) {
                if (flow_view.has_birth && flow_view.birth_pending) {
                    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
                        quit = true;
                    if(event.type==SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button==SDL_BUTTON_LEFT) {
                        SvBirthLayout bl=sv_birth_layout(birth_viewport.logical_width,birth_viewport.logical_height,SV_BIRTH_READY);
                        if(sv_login_contains(bl.quit,event.button.x,event.button.y))quit=true;
                    }
                    continue;
                }
                if (flow_view.has_birth && !flow_view.birth_pending) {
                    SvBirthInputResult birth_result =
                        sv_native_birth_event(flow->attempt.birth, flow->generation,
                                              flow->attempt.birth_input_since_ns,
                                              &birth_viewport, &event);
                    sv_login_flow_birth_result(flow, birth_result);
                    if (flow->attempt.finished) { quit = true; break; }
                    if (birth_result == SV_BIRTH_INPUT_PLACEHOLDER) {
                        sv_native_birth_release(&birth_viewport);
                        sv_pregame_input_boundary();
                        flow->attempt.birth_placeholder_open = true;
                        flow->attempt.birth_placeholder_drawn = false;
                        sv_birth_snapshot_prepare(&birth_before_placeholder, flow->attempt.birth);
                        flow->attempt.birth_placeholder_owner = birth_before_placeholder.owner;
                        flow->attempt.temporary_surface_text =
                            SV_STUB_BIRTH_GUIDE_TEXT;
                        break;
                    }
                    continue;
                }
                int cw,ch;
                if(!SDL_GetCurrentRenderOutputSize(renderer,&cw,&ch))break;
                float cs=sv_login_ui_scale(window);if(cs<=0)cs=1;
                SvCharacterLayout cl=sv_character_layout(sv_character_shell_layout(cw/cs,ch/cs,flow->attempt.login_input.ordinary_name_active),character_viewport.first);
                SvLoginInputResult handled=SV_LOGIN_INPUT_IGNORED;
                if(!flow->attempt.login_input.ordinary_name_active && sv_login_state(flow->attempt.login)==SV_LOGIN_OVERVIEW) {
                    SvLoginCommand command;
                    size_t slots[3];sv_login_interaction_free_slots(&flow->attempt.login_input,slots);
                    if(sv_native_character_command(&event,cl,&character_viewport,sv_login_count(flow->attempt.login),
                        sv_login_count(flow->attempt.login)+slots[0]+slots[1]+slots[2],&command))
                        handled=sv_login_interaction_command(&flow->attempt.login_input,command);
                } else handled=sv_native_name_event_at(&flow->attempt.login_input,&event,cl,
                    sv_font_cell_width(font,cs)/cs,&input->text_error,&input->clipboard_unavailable);
                sv_login_flow_login_result(flow, handled);
                if (handled == SV_LOGIN_INPUT_NAME_STARTED ||
                    handled == SV_LOGIN_INPUT_NAME_CANCELLED ||
                    handled == SV_LOGIN_INPUT_NAME_SUBMITTED) {
                    sv_pregame_input_boundary();
                    input_epoch = flow->input_epoch;
                    if (flow->attempt.login_input.ordinary_name_active) {
                        if (!SDL_StartTextInput(window)) {
                            flow->attempt.session_error = SV_INVALID;
                            flow->attempt.state = SV_SOCKET_PROTOCOL_ERROR;
                        }
                    } else SDL_StopTextInput(window);
                }
                if (flow->attempt.finished) { quit = true; break; }
                if (handled != SV_LOGIN_INPUT_IGNORED) continue;

            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                quit = true;
                break;
            }
        }
        if(flow->attempt.birth_started && !flow->attempt.birth_placeholder_open && flow->attempt.state==SV_SOCKET_READY && flow->attempt.login &&
           sv_login_state(flow->attempt.login)==SV_LOGIN_SELECTED)
            (void)sv_native_birth_tick(flow->attempt.birth,flow->generation,&birth_viewport,SDL_GetTicks());
        sv_login_flow_finish_tick(flow);
        if(flow->preparation && flow->preparation->view.identity_ready) {
            (void)sv_native_controls_step(&flow->preparation->controls,flow->preparation->app,16);
            flow->preparation->view.presentation=sv_app_view(flow->preparation->app);
        }
        if (flow->attempt.finished) break;
        if (flow->options.frames && ++frames >= flow->options.frames) break;
        limit_frame(frame_start, flow->options.fps);
    }
    if(flow->options.preparation && flow->preparation && flow->preparation->native_bound &&
       flow->preparation->view.startup_complete && flow->attempt.result==0) {
        SvSessionPreparation **output=flow->options.preparation;
        *output=sv_login_flow_take_preparation(flow);
    }
    sv_login_flow_end_attempt(flow);
    SDL_StopTextInput(window);
    scene.login_view = NULL;
    scene.birth_view = NULL;
    scene.birth_viewport = NULL;
    flow->attempt.temporary_surface_text = NULL;
    flow->attempt.status = NULL;
    scene.contact_failed = scene.retry_available = false;
    (void)window;
    return flow->attempt.result;
}

static bool load_list(SvEndpoint *endpoint, const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) return SDL_SetError("Cannot read selected server list");
    SvServer servers[SV_SERVER_COUNT] = {{0}};
    size_t count = 0;
    char line[512];
    bool ok = true;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (count == SV_SERVER_COUNT || !strchr(line, '\n')) { ok = false; break; }
        unsigned port;
        int parsed = sscanf(line, "%79s %u %95[^\n]", servers[count].host,
                            &port, servers[count].label);
        if (parsed < 2 || port < 1 || port > 65535) { ok = false; break; }
        servers[count].port = (uint16_t)port;
        servers[count].ping_ms = -1;
        ++count;
    }
    if (ferror(file)) ok = false;
    fclose(file);
    if (!ok || !sv_endpoint_servers(endpoint, servers, count))
        return SDL_SetError("Invalid server list row or capacity");
    return true;
}

typedef struct {
    SvPreferences **preferences;
    SvMapFont *map;
    SvLoginFlow *flow;
    SvFontSelection *fonts;
    SvGraphics **graphics;
    const SvSettingsConfig *config;
    SvSettingsConfig preview_config;
    const SvSettingsInvocation *invocation;
    SvGraphics *original_graphics;
    SvMapEffects *effects;
    SvMapEffectOptions effect_options;
    SvFont *original_font;
    SvPreferences *original_preferences;
    SvMapFont original_map;
} SvNativeFontOwner;
static void publish_font_options(SvNativeFontOwner *owner)
{
    if(!owner->flow)return;
    owner->flow->options.preferences=*owner->preferences;
    owner->flow->options.settings=sv_preferences_options(*owner->preferences);
    owner->flow->options.graphics_mode=sv_graphics_mode(*owner->graphics);
    owner->flow->options.graphics_tiles=sv_graphics_name(*owner->graphics);
    owner->flow->options.graphics_owner=*owner->graphics;
    owner->flow->options.effective_text_font=sv_font_resource(
        sv_font_selection_font(owner->fonts,SV_FONT_TEXT));
    if(owner->effects) {
        owner->effect_options.graphics_mode=sv_graphics_mode(*owner->graphics);
        sv_map_effects_options(owner->effects,&owner->effect_options);
    }
}
static bool preference_flag(const SvPreferences *preferences,const char *name)
{
    int slot=sv_settings_option_slot(name);
    const SvSettingsOptions *options=sv_preferences_options(preferences);
    return options && slot>=0 && options->values[slot];
}
static bool apply_map_font(void *opaque,SvFontRole role,SvFont *font)
{
    if(role==SV_FONT_TEXT)return true;
    SvNativeFontOwner *owner=opaque;
    if(font==owner->original_font && owner->original_preferences) {
        /* Cancel restores the prepared opening stack even when its source has
         * since disappeared. Restoring a view never reexecutes its PRF. */
        if(owner->flow && owner->flow->attempt.login &&
            sv_login_state(owner->flow->attempt.login)==SV_LOGIN_PLAY_ACKNOWLEDGED &&
            sv_login_update_rendering(owner->flow->attempt.login,
                &owner->original_map.definitions,sv_graphics_mode(owner->original_graphics),
                sv_graphics_name(owner->original_graphics),owner->original_map.effective)!=SV_OK)
            return false;
        sv_preferences_destroy(*owner->preferences);
        sv_graphics_destroy(*owner->graphics);
        *owner->graphics=owner->original_graphics;owner->original_graphics=NULL;
        *owner->preferences=owner->original_preferences;
        *owner->map=owner->original_map;
        owner->original_preferences=NULL;owner->original_font=NULL;
        publish_font_options(owner);
        return true;
    }
    SvMapFont candidate=*owner->map;
    SvPreferenceContext context=sv_preferences_default_context();
    context.generation=owner->flow?owner->flow->generation:0;
    SvPreferences *preferences=sv_map_font_reload(&candidate,*owner->preferences,
        sv_font_resource(font),&context);
    if(!preferences)return false;
    SvFontMetrics metrics;
    if(!sv_font_metrics(font,&metrics)) { sv_preferences_destroy(preferences);return false; }
    char graphics_error[256];
    SvGraphics *graphics=sv_graphics_prepare(candidate.user,candidate.library,owner->config,
        preferences,&context,metrics.width,metrics.height,graphics_error,sizeof(graphics_error));
    if(!graphics) {
        SDL_SetError("%s",graphics_error);sv_preferences_destroy(preferences);return false;
    }
    sv_graphics_cache_policy(graphics,owner->invocation->disable_tile_cache,
        owner->invocation->disable_tileset_cache);
    if(*graphics_error)fprintf(stderr,"SV graphics: %s\n",graphics_error);
    sv_map_font_definitions(&candidate.definitions,preferences);
    /* No pending session operation is reissued by the renderer. A playing
     * owner can publish setup+font in one atomic protocol append. */
    if(owner->flow && owner->flow->attempt.login &&
        sv_login_state(owner->flow->attempt.login)==SV_LOGIN_PLAY_ACKNOWLEDGED &&
        sv_login_update_rendering(owner->flow->attempt.login,&candidate.definitions,
            sv_graphics_mode(graphics),sv_graphics_name(graphics),candidate.effective)!=SV_OK) {
        sv_graphics_destroy(graphics);sv_preferences_destroy(preferences);return false;
    }
    SvPreferences *old=*owner->preferences;
    if(!owner->original_preferences) {
        owner->original_preferences=old;
        owner->original_font=sv_font_selection_font(owner->fonts,SV_FONT_MAP);
        owner->original_map=*owner->map;
        if(!owner->original_graphics)owner->original_graphics=*owner->graphics;
    }
    SvGraphics *old_graphics=*owner->graphics;
    *owner->graphics=graphics;
    *owner->preferences=preferences;*owner->map=candidate;
    publish_font_options(owner);
    if(old!=owner->original_preferences)sv_preferences_destroy(old);
    if(old_graphics!=owner->original_graphics)sv_graphics_destroy(old_graphics);
    return true;
}
static void resource_fields(SvSettingsConfig *out,const SvSettingsConfig *in)
{
    SDL_strlcpy(out->graphics_filter,in->graphics_filter,sizeof(out->graphics_filter));
    SDL_strlcpy(out->pcf_filter,in->pcf_filter,sizeof(out->pcf_filter));
    out->graphics_force_outline=in->graphics_force_outline;
}
static bool prepare_editor_resources(void *opaque,const SvSettingsConfig *config,
    SvFontRole role,SvFontEditorResourceAction action)
{
    SvNativeFontOwner *owner=opaque;
    SvFontMetrics metrics;
    if(!sv_font_metrics(sv_font_selection_font(owner->fonts,SV_FONT_MAP),&metrics))return false;
    SvGraphics *source=*owner->graphics;
    if(action==SV_EDITOR_RESOURCE_CANCEL && owner->original_graphics)source=owner->original_graphics;
    if(action==SV_EDITOR_RESOURCE_SAVED && role!=SV_FONT_MAP && owner->original_preferences &&
       owner->original_graphics)source=owner->original_graphics;
    char error[256];
    SvGraphics *candidate=sv_graphics_reprepare(source,
        sv_resource_filter(config->graphics_filter,SV_RESOURCE_LINEAR),config->graphics_force_outline,
        metrics.width,metrics.height,error,sizeof(error));
    if(!candidate) {
        /* The font Save has already succeeded. Do not later restore graphics
         * from the previously saved font when retaining its policy failed. */
        if(action==SV_EDITOR_RESOURCE_SAVED && (role==SV_FONT_MAP || !owner->original_preferences)) {
            sv_graphics_destroy(owner->original_graphics);owner->original_graphics=NULL;
        }
        return SDL_SetError("%s",error);
    }
    if(action==SV_EDITOR_RESOURCE_SAVED) {
        /* A saved map font establishes a new source baseline even if writing
         * its filter draft failed. Cancel restores that font with saved policy.
         * A Text save keeps any still-unsaved Map source baseline. */
        sv_graphics_destroy(owner->original_graphics);
        owner->original_graphics=candidate;
        return true;
    }
    if(action==SV_EDITOR_RESOURCE_CANCEL) {
        sv_graphics_destroy(*owner->graphics);
        sv_graphics_destroy(owner->original_graphics);owner->original_graphics=NULL;
    } else {
        if(!owner->original_graphics)owner->original_graphics=*owner->graphics;
        else if(*owner->graphics!=owner->original_graphics)sv_graphics_destroy(*owner->graphics);
    }
    *owner->graphics=candidate;
    resource_fields(&owner->preview_config,config);
    publish_font_options(owner);
    return true;
}

int sv_endpoint_run(SvEndpointOptions options)
{
    SvLoginFlow *flow = NULL;
    if(options.preparation)*options.preparation=NULL;
    SvEndpoint *endpoint = NULL;
    const char *root = options.root;
    char *owned_root = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SvFont *font = NULL;
    SvFontSelection *fonts=NULL;
    SvFontEditor *font_editor=NULL;
    SvMapFont map_font={0};
    SvGraphics *graphics=NULL;
    SvMapEffects *map_effects=NULL;
    SvSettingsAudio *audio = NULL;
    SvLuaReload *lua = NULL;
    SvMetaserver *provider = NULL;
    SvChronicle *chronicle = NULL;
    SvChronicleSnapshot chronicle_view = {.failed_sources = 9, .completed_sources = 9};
    SvSettingsConfig config;
    SvSettingsInvocation invocation = {0};
    SvPreferences *preferences = NULL;
    SvNativeFontOwner font_owner={.preferences=&preferences,.map=&map_font,
        .graphics=&graphics,.config=&config,.invocation=&invocation};
    sv_settings_config_defaults(&config);
    int result = 1;
    bool first_run = true;
    bool fixture_window = options.windowed != 0;
    if (!root) {
        root = SDL_getenv("TOMENET_SDL3_USER_PATH");
        if (!root || !*root) {
            owned_root = SDL_GetPrefPath("TomenetGame", "tomenet");
            root = owned_root;
        }
    }
    if (!root || !*root || !sv_settings_startup_load(&config, root)) goto done;
    if (options.startup_argc && !sv_settings_startup_arguments(&config, &invocation,
            options.startup_argc, options.startup_argv)) {
        SDL_SetError("Invalid startup setting argument"); result = 2; goto done;
    }
    if (invocation.redirect_log && !freopen("tomenet.log", "w", stderr)) {
        SDL_SetError("Cannot redirect diagnostics to tomenet.log"); goto done;
    }
    if (invocation.force_cui) {
        SDL_SetError("CUI/terminal frontend is excluded from the SV client"); result = 2; goto done;
    }
    if (invocation.help) { puts("TomeNET SV: use --help for invocation options"); result = 0; goto done; }
    const char *library = options.library ? options.library : SDL_getenv("TOMENET_PATH");
    char adjacent[4096];
    if (*invocation.library_override) library = invocation.library_override;
    if (!library || !*library) {
        const char *base = SDL_GetBasePath();
        if (!base || SDL_snprintf(adjacent,sizeof(adjacent),"%s/lib",base) >= (int)sizeof(adjacent)) goto done;
        library = adjacent;
    }
    /* Programmatic callers provide explicit overrides too. For CLI invocations
     * reduction preserves order, including -f following an earlier --port. */
    if (!options.startup_argc) {
        if (options.server) SDL_strlcpy(config.server, options.server, sizeof(config.server));
        if (options.port) config.port = (int)options.port;
        if (options.metaserver) SDL_strlcpy(config.meta, options.metaserver, sizeof(config.meta));
        if (options.real_name) SDL_strlcpy(config.realname, options.real_name, sizeof(config.realname));
        if (options.character) SDL_strlcpy(config.name, options.character, sizeof(config.name));
        if (options.account) SDL_strlcpy(config.nick, options.account, sizeof(config.nick));
    }
    options.root = root;
    options.library = library;
    options.server = *config.server ? config.server : NULL;
    options.port = (unsigned)config.port;
    options.metaserver = *config.meta ? config.meta : NULL;
    options.real_name = config.realname;
    options.character = *config.name ? config.name : NULL;
    options.account = *config.nick ? config.nick : NULL;
    if (strlen(config.nick) >= 80 || strlen(config.name) >= SV_LOGIN_NAME_CAPACITY) {
        SDL_SetError("Configured account/character exceeds the supported byte limit"); result = 2; goto done;
    }
    if (options.startup_argc && !invocation.password_stdin) options.password = NULL;
    if (invocation.password_present) options.password = invocation.password;
    options.invocation = &invocation;
    options.auto_reincarnation = invocation.auto_reincarnation;
    if (options.auto_reincarnation && strlen(config.name) > 15) {
        SDL_SetError("Reincarnation character name exceeds the supported byte limit"); result = 2; goto done;
    }
    options.protocol_override = invocation.protocol_override;
    options.skip_motd = options.skip_motd || invocation.skip_motd;
    if (!options.windowed) {
        options.windowed = !strcmp(config.window_mode, "window");
        options.width = config.window_width;
        options.height = config.window_height;
    }
    if (options.width < 1024) options.width = 1024;
    if (options.height < 768) options.height = 768;
    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) goto done;
    fonts=sv_font_selection_create(root,library,&config);
    if(!fonts)goto done;
    font_owner.fonts=fonts;
    font_owner.preview_config=config;font_owner.config=&font_owner.preview_config;
    font=sv_font_selection_font(fonts,SV_FONT_TEXT);
    if(!sv_map_font_init(&map_font,root,library,
        sv_font_resource(sv_font_selection_font(fonts,SV_FONT_MAP))))goto done;
    fprintf(stderr,"SV fonts: text requested=%s effective=%s; map requested=%s effective=%s; %s\n",
        config.text_font,sv_font_resource(font),config.map_font,
        sv_font_resource(sv_font_selection_font(fonts,SV_FONT_MAP)),sv_font_selection_error(fonts));
    preferences = sv_preferences_create(root, library, NULL);
    if (!preferences) { SDL_SetError("Cannot allocate application preferences"); goto done; }
    SvPreferenceContext preference_context = sv_preferences_default_context();
    SvFontMetrics map_metrics;
    if(!sv_font_metrics(sv_font_selection_font(fonts,SV_FONT_MAP),&map_metrics))goto done;
    SvGraphicsBootstrap graphics_bootstrap={.map=&map_font,.config=&config,.active=&graphics,
        .width=map_metrics.width,.height=map_metrics.height};
    SvPreferenceResult preference_result = sv_preferences_bootstrap(preferences,
        &preference_context, sv_graphics_bootstrap, &graphics_bootstrap);
    if(!graphics)goto done;
    sv_graphics_cache_policy(graphics,invocation.disable_tile_cache,invocation.disable_tileset_cache);
    if(*graphics_bootstrap.error)fprintf(stderr,"SV graphics: %s\n",graphics_bootstrap.error);
    fprintf(stderr,"SV graphics requested=%d/%s effective=%d/%s generation=%llu\n",
        config.graphics,config.graphic_tiles,sv_graphics_mode(graphics),sv_graphics_name(graphics),
        (unsigned long long)sv_graphics_generation(graphics));
    sv_map_font_definitions(&map_font.definitions,preferences);
    for (size_t i = 0; i < sv_preferences_diagnostic_count(preferences); ++i) {
        const SvPreferenceDiagnostic *diagnostic = sv_preferences_diagnostic(preferences, i);
        fprintf(stderr, "SV preferences %s:%u: %s\n", diagnostic->source.path,
            diagnostic->line, diagnostic->message);
    }
    if (preference_result.status == SV_PREF_FATAL) {
        SDL_SetError("Cannot retain application preference data"); goto done;
    }
    if(map_font.status==SV_PREF_UNAVAILABLE) {
        SDL_SetError("Map preference source is unreadable or invalid");goto done;
    }
    size_t preference_messages = 0, preference_actions = 0;
    for (size_t i = 0; i < sv_preferences_effect_count(preferences); ++i) {
        const SvPreferenceEffect *effect = sv_preferences_effect(preferences, i);
        preference_messages += effect->kind == SV_PREF_MESSAGE;
        preference_actions += effect->kind == SV_PREF_ACTION;
    }
    if (preference_messages || preference_actions)
        fprintf(stderr, "SV startup preferences: retained %zu messages and %zu actions; message presentation and macro runtime pending\n",
            preference_messages, preference_actions);
    options.preferences = preferences;
    options.map_font=&map_font;
    options.graphics_mode=sv_graphics_mode(graphics);
    options.graphics_tiles=sv_graphics_name(graphics);
    options.settings = sv_preferences_options(preferences);
    options.client_config=&config;
    options.effective_text_font=sv_font_resource(font);
    options.graphics_owner=graphics;
    options.settings_wide = !strcmp(config.layout, "wide");
    options.fps = config.fps;
    flow = sv_login_flow_create(options);
    font_owner.flow=flow;
    sv_font_selection_prepare(fonts,apply_map_font,&font_owner);
    sv_settings_invocation_clear_password(&invocation);
    if (!flow) goto done;
    endpoint = &flow->endpoint;
    if (options.server && !sv_endpoint_argument(endpoint, options.server)) {
        SDL_SetError("Invalid server address"); result = 2; goto done;
    }
    if (options.server_list && !load_list(endpoint, options.server_list)) { result = 2; goto done; }
    if (options.windowed && !fixture_window && !config.window_width && !config.window_height) {
        SDL_Rect area;
        if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &area)) {
            options.width = SDL_max(1024, area.w);
            options.height = SDL_max(768, area.h);
        }
    }
    first_run = invocation.reset || profile_first_run(root);
    if (!options.server && !options.server_list && !options.source_poll) {
        SvEndpoint meta;
        sv_endpoint_begin(&meta, 8801);
        if (!sv_endpoint_argument(&meta, options.metaserver ? options.metaserver : "meta.tomenet.eu")) {
            SDL_SetError("Invalid metaserver address"); goto done;
        }
        provider = sv_metaserver_start(meta.host, meta.port);
        if (!provider) goto done;
    }
    printf("SV settings schema=%d diagnostics=%u unsupported=%u writable=%d ui_scale=%d layout=%s fps=%d\n",
           config.schema_version, config.diagnostics, config.unsupported_records,
           config.overwrite_allowed, config.ui_scale_percent, config.layout, config.fps);
    printf("SV settings roots user=%s private=%s/sv library=%s\n", root, root, library);
    lua=sv_lua_reload_create(root,library,lua_diagnostic,NULL);
    if (!lua) { SDL_SetError("Cannot allocate application Lua runtime"); goto done; }
    SvLuaResult lua_startup=sv_lua_reload_init(lua);
    SvTransferStartupConfig transfer_config = {.library=library, .lua=lua,
        .no_updates=invocation.no_lua_updates};
    flow->options.transfer_factory = sv_transfer_startup_factory(&transfer_config);
    flow->options.lua=lua;flow->options.lua_early_init=lua_startup;
    fprintf(stderr,"SV Lua early startup: status=%d scripts=%u; session open awaits decoded server features\n",
        (int)lua_startup.status,lua_startup.scripts_loaded);
    /* Keep the local startup failure visible. C011 will decide readiness with
     * actual server features; pregame must not invent flags to run c-init.lua. */
    SvSettingsConfig audio_config = config;
    if (invocation.quiet) audio_config.sound = 0;
    audio = sv_settings_audio_start(&audio_config, root, library);
    if (!audio) goto done;
    const SvSettingsAudioSnapshot *audio_state = sv_settings_audio_snapshot(audio);
    flow->options.audio = audio_state;
    fprintf(stderr, "SV initial audio: %s (rate=%d speakers=%d tracks=%d buffer=%d)\n",
            audio_state->diagnostic, audio_state->effective_rate, audio_state->effective_speakers,
            audio_state->effective_tracks, audio_state->effective_buffer);
    printf("SV audio requested sound=%s subset=%d music=%s subset=%d rate=%d tracks=%d buffer=%d cache=%d\n",
           config.soundpack_folder, config.soundpack_subset, config.musicpack_folder,
           config.musicpack_subset, config.audio_sample_rate, config.audio_channels,
           config.audio_buffer, config.cache_audio);
    printf("SV audio effective counts=%d/%d sound=%s version=%s music=%s version=%s sound_config=%s music_config=%s\n",
           audio_state->sfx, audio_state->music, audio_state->sound_name,
           audio_state->sound_version, audio_state->music_name, audio_state->music_version,
           audio_state->sound_config, audio_state->music_config);
    printf("SV startup text requested=%s effective=%s\n", config.text_font, sv_font_resource(font));
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (!options.windowed) flags |= SDL_WINDOW_FULLSCREEN;
    window = SDL_CreateWindow("TomeNET SV - Server",options.width,options.height,flags);
    if (!window || !sv_login_ui_set_scale(window, config.ui_scale_percent) ||
        !sv_login_ui_ignore_dpi(window, invocation.ignore_scaling)) goto done;
    float window_scale = sv_login_display_scale(window) / SDL_GetWindowPixelDensity(window);
    if (window_scale <= 0 || !SDL_SetWindowMinimumSize(window,
            (int)SDL_ceilf(1024 * window_scale), (int)SDL_ceilf(768 * window_scale))) goto done;
    if (options.windowed && !SDL_SetWindowSize(window,
            (int)SDL_roundf(options.width * window_scale),
            (int)SDL_roundf(options.height * window_scale))) goto done;
    /* Finalize asynchronous startup sizing before software rendering binds the
     * window surface; otherwise its output can retain the unscaled size. */
    if (!SDL_SyncWindow(window)) goto done;
    options.shell.text_font_path = sv_font_resource(font);
    renderer = SDL_CreateRenderer(window,NULL);
    if (!renderer || !sv_login_shell_open_options(renderer, library, options.shell)) goto done;
    font_editor=sv_font_editor_create(fonts,root,library,&config,&map_font);
    if(!font_editor)goto done;
    map_effects=sv_map_effects_create(1,66,!strcmp(config.layout,"wide")?44:22,1);
    if(!map_effects)goto done;
    SvMapEffectOptions effect_options={.palette_animation=true,
        .use_colour=preference_flag(preferences,"use_color"),
        .no_flicker=preference_flag(preferences,"no_flicker"),
        .subterm_flicker=preference_flag(preferences,"subterm_flicker"),
        .misc_no_flicker=preference_flag(preferences,"misc_no_flicker"),
        .ascii_weather=preference_flag(preferences,"ascii_weather"),
        .no2mask_weather=preference_flag(preferences,"no2mask_weather"),
        .disable_lightning=preference_flag(preferences,"disable_lightning"),
        .lighter_dark_blue=config.lighter_dark_blue,.no_weather=invocation.no_weather,
        .graphics_mode=sv_graphics_mode(graphics)};
    effect_options.no_weather|=preference_flag(preferences,"no_weather");
    sv_map_effects_options(map_effects,&effect_options);
    for(unsigned i=0;i<16;++i) {
        uint32_t rgb=config.palette[i];
        if(sv_map_effects_palette(map_effects,1,(unsigned char)i,
            (unsigned char)(rgb>>16),(unsigned char)(rgb>>8),(unsigned char)rgb)!=SV_OK)goto done;
    }
    effect_options.palette_animation=preference_flag(preferences,"palette_animation");
    sv_map_effects_options(map_effects,&effect_options);
    sv_font_editor_rendering(font_editor,&graphics,map_effects);
    sv_font_editor_resource_prepare(font_editor,prepare_editor_resources,&font_owner);
    font_owner.effects=map_effects;font_owner.effect_options=effect_options;
    printf("SV startup window fullscreen=%d ui_scale=%d ignore_dpi=%d renderer=%s\n",
           (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0,
           config.ui_scale_percent, invocation.ignore_scaling, SDL_GetRendererName(renderer));
    if (invocation.no_lua_updates)
        fprintf(stderr, "SV startup: server file updates disabled (-u)\n");
    if (invocation.chat_log_mode || invocation.quit_no_prompt)
        fprintf(stderr, "SV invocation retains deferred export/quit requests; their late consumers are not active\n");
    int window_count = 0;
    SDL_Window **windows = SDL_GetWindows(&window_count);
    SDL_free(windows);
    if (window_count != 1) { SDL_SetError("SV requires exactly one system window"); goto done; }
    SvEndpointInput input;
    SvEndpointView scene = {.chronicle = &chronicle_view};
    sv_endpoint_input_begin(&input);
    chronicle = sv_chronicle_start();
    (void)sv_chronicle_poll(chronicle, &chronicle_view);
    scene.chronicle = &chronicle_view;
    if (!draw_scene(renderer,font,endpoint,&input,&scene,NULL) ||
        !sv_endpoint_render(renderer,font,endpoint,&scene,&(SvEndpointRenderMetrics){0})) goto done;
    SDL_Rect sample = {0,0,1,1};
    SDL_Surface *ready = SDL_RenderReadPixels(renderer,&sample);
    if (!ready) goto done;
    SDL_DestroySurface(ready);
    if (!SDL_RenderPresent(renderer) || !SDL_StartTextInput(window)) goto done;
    sv_endpoint_input_begin(&input);
    scene.chronicle = &chronicle_view;
    int frames = 0;
    unsigned text_generation=sv_font_selection_generation(fonts,SV_FONT_TEXT);
select_endpoint:
    sv_pregame_input_boundary();
    if (!SDL_StartTextInput(window)) goto done;
    bool quit = endpoint->phase == SV_ENDPOINT_SELECTED;
    while (!quit) {
        Uint64 frame_start = SDL_GetTicks();
        (void)sv_chronicle_poll(chronicle, &chronicle_view);
        bool failed = false;
        bool changed = options.source_poll ? options.source_poll(options.source_context, endpoint, &failed) :
                       provider && sv_metaserver_poll(provider, endpoint, &failed);
        if (changed && failed && endpoint->phase == SV_ENDPOINT_LIST) {
            sv_endpoint_manual(endpoint);
            sv_pregame_input_boundary();
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                quit = true; break;
            }
            if (event.type == SDL_EVENT_RENDER_DEVICE_RESET &&
                !sv_login_shell_reset(renderer)) goto done;
            if(event.type==SDL_EVENT_RENDER_DEVICE_RESET) {
                sv_graphics_reset_renderer(graphics);
                sv_graphics_reset_renderer(font_owner.original_graphics);
            }
            bool editing=sv_font_editor_open(font_editor);
            if(sv_font_editor_event(font_editor,&event)) {
                if(event.type==SDL_EVENT_KEY_DOWN && event.key.key==SDLK_S &&
                    sv_font_editor_role(font_editor)==SV_FONT_MAP &&
                    (event.key.mod&SDL_KMOD_CTRL) && !sv_font_selection_dirty(fonts,SV_FONT_MAP) &&
                    !*sv_font_selection_error(fonts) && font_owner.original_preferences) {
                    sv_preferences_destroy(font_owner.original_preferences);
                    font_owner.original_preferences=NULL;font_owner.original_font=NULL;
                }
                font=sv_font_selection_font(fonts,SV_FONT_TEXT);
                unsigned generation=sv_font_selection_generation(fonts,SV_FONT_TEXT);
                if(generation!=text_generation) {
                    options.shell.text_font_path=sv_font_resource(font);
                    if(!sv_login_shell_open_options(renderer,library,options.shell))goto done;
                    text_generation=generation;
                }
                if(editing && !sv_font_editor_open(font_editor)) {
                    sv_pregame_input_boundary();SDL_StartTextInput(window);
                }
                continue;
            }
            (void)sv_endpoint_event(&input,endpoint,&event,NULL,NULL);
            if (endpoint->phase == SV_ENDPOINT_CANCELLED ||
                endpoint->phase == SV_ENDPOINT_SELECTED) { quit = true; break; }
        }
        if (quit) break;
        float field_scale=sv_login_ui_scale(window);
        if(field_scale<=0)field_scale=1;
        input.text_cell=sv_font_cell_width(font,field_scale)/field_scale;
        if(sv_font_editor_open(font_editor)) {
            if(sv_map_effects_tick(map_effects,1,SDL_GetTicks())!=SV_OK)goto done;
            if(!sv_font_editor_draw(font_editor,renderer))goto done;
        } else if (!draw_scene(renderer,font,endpoint,&input,&scene,NULL)) goto done;
        if (options.frames && ++frames >= options.frames) break;
        limit_frame(frame_start, config.fps);
    }
    if (endpoint->phase == SV_ENDPOINT_SELECTED) {
        printf("SV endpoint selected host=%s port=%u\n",
               endpoint->host,(unsigned)endpoint->port);
        if (!options.selected_endpoint) {
            SvLoginFlowStep step = sv_login_flow_selected(flow);
            while (step != SV_LOGIN_FLOW_FINISHED) {
                if (step == SV_LOGIN_FLOW_CREDENTIALS) {
                    int entered = enter_credentials(renderer, font, endpoint, &input, chronicle,
                        &chronicle_view, options.frames, flow->account, flow->password, config.fps);
                    SvLoginCredentialResult credential_result = entered == SV_CREDENTIAL_SCENE_ACCEPTED ?
                        SV_LOGIN_CREDENTIAL_ACCEPTED : entered == SV_CREDENTIAL_SCENE_BACK ?
                        SV_LOGIN_CREDENTIAL_BACK : entered == SV_CREDENTIAL_SCENE_ERROR ?
                        SV_LOGIN_CREDENTIAL_ERROR : SV_LOGIN_CREDENTIAL_QUIT;
                    step = sv_login_flow_credentials_result(flow, credential_result);
                    result = flow->attempt.result;
                } else if (step == SV_LOGIN_FLOW_CONNECT) {
                    result = run_contact(window, renderer, font, &input, chronicle,
                        &chronicle_view, flow, root, first_run);
                    step = sv_login_flow_next(flow);
                } else if (step == SV_LOGIN_FLOW_SERVERS) {
                    float chronicle_scroll = input.chronicle_scroll;
                    sv_endpoint_input_begin(&input);
                    input.chronicle_scroll = chronicle_scroll;
                    scene = (SvEndpointView){.chronicle = &chronicle_view};
                    goto select_endpoint;
                }
            }
            wipe_password(flow->password, sizeof(flow->password));
            goto done;
        }
    }
    else if (endpoint->phase == SV_ENDPOINT_CANCELLED)
        puts("SV endpoint selection cancelled; no contact attempted");
    else puts("SV endpoint selection incomplete; no contact attempted");
    if (options.selected_endpoint) {
        *options.selected_endpoint = (SvEndpointChoice){.phase = endpoint->phase,
            .port = endpoint->port, .protocol = endpoint->protocol};
        memcpy(options.selected_endpoint->host, endpoint->host, sizeof(endpoint->host));
    }
    result = 0;
done:
    sv_settings_invocation_clear_password(&invocation);
    SvSessionPreparation *retained=options.preparation?*options.preparation:NULL;
    if(retained) {
        retained->owns_lua=true;lua=NULL;
        SvPreparationNative *native=retained->native_owner;
        if(native) {
            native->owns=true;native->fonts=fonts;native->audio=audio;
            native->graphics=graphics;retained->graphics=graphics;graphics=NULL;
            sv_font_selection_prepare(fonts,NULL,NULL);
            fonts=NULL;audio=NULL;window=NULL;renderer=NULL;
        }
        /* The retained transport no longer invokes the stack-backed factory. */
        flow->options.transfer_factory.context=NULL;
        flow->options.real_name=flow->options.character=NULL;
        flow->options.startup_argv=NULL;flow->options.startup_argc=0;
        flow->font_resource=NULL;flow->options.shell=(SvLoginShellOptions){0};
    }
    /* Attempt attachments borrow Lua; revoke them before application teardown. */
    sv_login_flow_destroy(flow);
    flow = NULL;
    sv_lua_reload_destroy(lua);
    sv_settings_audio_destroy(audio);
    sv_settings_config_dispose(&config);
    sv_chronicle_stop(chronicle);
    sv_metaserver_stop(provider);
    if (result) fprintf(stderr,"SV endpoint startup failed: %s\n",SDL_GetError());
    /* Pending startup intents never enter credential input. Their application
     * owner ends here; attempts own and cancel their separate character intents. */
    if (preferences && sv_preferences_effect_count(preferences))
        fprintf(stderr, "SV startup preferences: cancelling %zu retained effects at application close\n",
            sv_preferences_effect_count(preferences));
    sv_preferences_destroy(preferences);
    sv_preferences_destroy(font_owner.original_preferences);
    sv_font_editor_destroy(font_editor);
    sv_graphics_destroy(graphics);
    sv_graphics_destroy(font_owner.original_graphics);
    sv_map_effects_destroy(map_effects);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    sv_font_selection_destroy(fonts);
    if(!retained){TTF_Quit();SDL_Quit();}
    SDL_free(owned_root);
    return result;
}
