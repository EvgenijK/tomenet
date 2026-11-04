#include "login/characters/native-login.h"
#include <assert.h>

int main(void)
{
    assert(SDL_Init(0));
    SDL_Event event = {0};
    SvLoginCommand command;
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_A;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CHOOSE && command.slot == 0);
    event.key.key = SDLK_N;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CHOOSE && command.slot == 13);
    event.key.mod = SDL_KMOD_SHIFT;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CREATE_ORDINARY);
    event.key.key = SDLK_E;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CREATE_EXCLUSIVE);
    event.key.mod = 0;
    event.key.key = SDLK_ESCAPE;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_CANCEL);
    event.key.key = SDLK_Q;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_QUIT);
    event.key.key = SDLK_SPACE;
    assert(sv_native_login_command(&event, &command));
    assert(command.kind == SV_LOGIN_COMMAND_ACK_MOTD);
    event.key.key = 0;
    assert(!sv_native_login_command(&event, &command));
    SvCharacterViewport viewport={.selected=12,.first=6};
    SvCharacterLayout layout=sv_character_layout(sv_character_shell_layout(1024,768,0),6);
    event=(SDL_Event){.type=SDL_EVENT_MOUSE_BUTTON_DOWN};
    event.button.button=SDL_BUTTON_LEFT;
    event.button.clicks=2;
    layout=sv_character_content_layout(layout,16);
    viewport.first=layout.first=16-layout.rows;
    SvLoginRect row=sv_character_row(layout,12);
    event.button.x=row.x+10;event.button.y=row.y+10;
    assert(sv_native_character_command(&event,layout,&viewport,16,16,&command));
    assert(command.kind==SV_LOGIN_COMMAND_CHOOSE && command.slot==12);
    event.button.y=layout.list.y+layout.list.h+1;
    assert(!sv_native_character_command(&event,layout,&viewport,16,16,&command));
    layout=sv_character_layout(sv_character_shell_layout(1440,900,0),viewport.first);
    viewport.selected=15;
    layout=sv_character_content_layout(layout,16);
    sv_character_viewport_sync(&viewport,layout,16,16);
    /* Sync recomputes the visible row count after moving the scroll offset. */
    assert(viewport.selected>=viewport.first && viewport.selected<viewport.first+viewport.rows);
    layout=sv_character_layout(sv_character_shell_layout(1024,768,0),viewport.first);
    layout=sv_character_content_layout(layout,16);
    sv_character_viewport_sync(&viewport,layout,16,16);
    assert(viewport.selected>=viewport.first && viewport.selected<viewport.first+viewport.rows);
    viewport=(SvCharacterViewport){0};
    layout=sv_character_layout(sv_character_shell_layout(1024,768,0),0);
    sv_character_viewport_sync(&viewport,layout,8,16);
    event=(SDL_Event){.type=SDL_EVENT_MOUSE_WHEEL};event.wheel.y=-1;
    for(int i=0;i<12;++i) {
        layout=sv_character_layout(sv_character_shell_layout(1024,768,0),viewport.first);
        assert(!sv_native_character_command(&event,layout,&viewport,8,16,&command));
        layout.first=viewport.first;
        sv_character_viewport_sync(&viewport,layout,8,16);
    }
    layout=sv_character_content_layout(layout,8);
    assert(viewport.first>8 && viewport.first+layout.rows>=16);
    event=(SDL_Event){.type=SDL_EVENT_MOUSE_BUTTON_DOWN};
    event.button.button=SDL_BUTTON_LEFT;
    event.button.x=layout.quit.x+4;event.button.y=layout.quit.y+4;
    assert(sv_native_character_command(&event,layout,&viewport,8,16,&command));
    assert(command.kind==SV_LOGIN_COMMAND_CANCEL);
    viewport.focus=4;
    event=(SDL_Event){.type=SDL_EVENT_KEY_DOWN};event.key.key=SDLK_RETURN;
    assert(sv_native_character_command(&event,layout,&viewport,8,16,&command));
    assert(command.kind==SV_LOGIN_COMMAND_CANCEL);
    sv_native_login_clear_previous_keys(SV_LOGIN_TRANSITION_OVERVIEW);
    SDL_Quit();
    return 0;
}
