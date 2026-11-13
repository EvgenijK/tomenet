/* Transitional CLI entry: the existing synthetic/check launcher lives with
 * its diagnostic harness until the production CLI is separated. */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "diagnostics/launcher.h"

int main(int argc, char **argv)
{
    return sv_diagnostics_main(argc, argv);
}
