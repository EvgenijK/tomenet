#include "settings.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void write_bytes(const char *path, const char *bytes)
{
    FILE *file = fopen(path, "wb");
    assert(file);
    assert(fwrite(bytes, 1, strlen(bytes), file) == strlen(bytes));
    assert(!fclose(file));
}

static char *read_bytes(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file);
    assert(!fseek(file, 0, SEEK_END));
    long size = ftell(file);
    assert(size >= 0 && !fseek(file, 0, SEEK_SET));
    char *bytes = malloc((size_t)size + 1);
    assert(bytes);
    assert(fread(bytes, 1, (size_t)size, file) == (size_t)size);
    bytes[size] = 0;
    assert(!fclose(file));
    return bytes;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    char cfg[4096], global[4096], class_file[4096], named[4096], old_global[4096],
         macro_prf[4096];
    char directory[4096];
    assert(snprintf(directory, sizeof(directory), "%s/sv", argv[1]) < (int)sizeof(directory));
    assert(snprintf(cfg, sizeof(cfg), "%s/tomenet.cfg", directory) < (int)sizeof(cfg));
    assert(snprintf(global, sizeof(global), "%s/global.opt", directory) < (int)sizeof(global));
    assert(snprintf(class_file, sizeof(class_file), "%s/Ranger.opt", directory) < (int)sizeof(class_file));
    assert(snprintf(named, sizeof(named), "%s/custom.opt", directory) < (int)sizeof(named));
    assert(snprintf(old_global, sizeof(old_global), "%s/global.opt", argv[1]) < (int)sizeof(old_global));
    assert(snprintf(macro_prf, sizeof(macro_prf), "%s/macro.prf", argv[1]) < (int)sizeof(macro_prf));
    write_bytes(cfg, "svSchemaVersion\t1\n# kept\npass\tsecret\nsvWindowMode\tfullscreen\n"
                     "svUiScalePercent\t100\nfutureKey\toriginal\n");
    write_bytes(old_global, "Y:newbie_hints\n");
    write_bytes(macro_prf, "A:original-macro\n");
    SvProfile profile;
    SvOptions options;
    assert(sv_profile_load(&profile, argv[1]));
    sv_options_defaults(&options);
    SvSettings s;
    assert(sv_settings_begin(&s, argv[1], &profile, &options));
    assert(sv_settings_edit_cfg(&s, "svUiScalePercent", "125"));
    assert(sv_settings_edit_cfg(&s, "svWindowMode", "window"));
    assert(sv_settings_dirty(&s));
    sv_settings_cancel(&s);
    assert(!sv_settings_dirty(&s));
    assert(s.profile.ui_scale == 100 && !s.profile.windowed);
    assert(sv_settings_edit_cfg(&s, "svUiScalePercent", "125"));
    write_bytes(cfg, "svSchemaVersion\t1\n# kept\npass\tsecret\nsvWindowMode\tfullscreen\n"
                     "svUiScalePercent\t100\nfutureKey\texternal\n");
    assert(sv_settings_save_cfg(&s) == SV_SETTINGS_OK);
    assert(!sv_settings_dirty(&s));
    char *bytes = read_bytes(cfg);
    assert(strstr(bytes, "svUiScalePercent\t125\n"));
    assert(strstr(bytes, "futureKey\texternal\n"));
    assert(strstr(bytes, "# kept\n"));
    assert(!strstr(bytes, "pass\t") && !strstr(bytes, "secret"));
    free(bytes);
    assert(sv_profile_load(&profile, argv[1]) && profile.ui_scale == 125);
    assert(sv_settings_edit_cfg(&s, "svUiScalePercent", "130"));
    sv_settings_cancel(&s);
    assert(s.profile.ui_scale == 100); /* Cancel uses the form's opening snapshot. */
    assert(sv_settings_dirty(&s)); /* The already-saved 125 remains on disk. */
    sv_settings_end(&s);

    assert(sv_settings_begin(&s, argv[1], &profile, &options));
    assert(sv_settings_edit_cfg(&s, "svUiScalePercent", "130"));
    write_bytes(cfg, "svSchemaVersion\t1\nsvUiScalePercent\t140\nfutureKey\texternal\n");
    assert(sv_settings_save_cfg(&s) == SV_SETTINGS_CONFLICT);
    assert(sv_settings_save_cfg(&s) == SV_SETTINGS_CONFLICT);
    assert(sv_settings_dirty(&s));
    bytes = read_bytes(cfg);
    assert(strstr(bytes, "svUiScalePercent\t140\n"));
    assert(!strstr(bytes, "130"));
    free(bytes);
    sv_settings_end(&s);

    assert(sv_profile_load(&profile, argv[1]) && profile.ui_scale == 140);
    assert(sv_settings_begin(&s, argv[1], &profile, &options));
    assert(sv_settings_edit_cfg(&s, "svUiScalePercent", "145"));
    bytes = read_bytes(cfg);
    assert(!chmod(directory, 0500));
    assert(sv_settings_save_cfg(&s) == SV_SETTINGS_IO_ERROR);
    assert(!chmod(directory, 0700));
    char *after = read_bytes(cfg);
    assert(!strcmp(bytes, after));
    assert(sv_settings_dirty(&s) && s.profile.ui_scale == 145);
    free(after);
    free(bytes);
    sv_settings_end(&s);

    assert(sv_settings_begin(&s, argv[1], &profile, &options));
    assert(sv_settings_edit_option(&s, "newbie_hints", false));
    assert(sv_settings_options_target(&s, SV_OPTIONS_GLOBAL, NULL));
    assert(sv_settings_save_options(&s) == SV_SETTINGS_OK);
    bytes = read_bytes(global);
    assert(strstr(bytes, "X:newbie_hints\n"));
    assert(strstr(bytes, "Y:censor_swearing\n"));
    free(bytes);
    bytes = read_bytes(old_global);
    assert(!strcmp(bytes, "Y:newbie_hints\n"));
    free(bytes);
    SvOptions loaded;
    assert(sv_options_load_base(&loaded, argv[1]));
    bool value;
    assert(sv_options_get(&loaded, "newbie_hints", &value) && !value);
    assert(!sv_settings_options_target(&s, SV_OPTIONS_CLASS, "../Ranger"));
    assert(sv_settings_edit_option(&s, "censor_swearing", false));
    assert(sv_settings_options_target(&s, SV_OPTIONS_CLASS, "Ranger"));
    assert(sv_settings_save_options(&s) == SV_SETTINGS_OK);
    bytes = read_bytes(class_file);
    assert(strstr(bytes, "X:newbie_hints\n"));
    assert(strstr(bytes, "X:censor_swearing\n"));
    free(bytes);
    assert(sv_settings_options_target(&s, SV_OPTIONS_NAMED, "custom.opt"));
    assert(sv_settings_save_options(&s) == SV_SETTINGS_OK);
    bytes = read_bytes(named);
    assert(strstr(bytes, "X:newbie_hints\n"));
    free(bytes);
    assert(sv_options_load_base(&loaded, argv[1]));
    assert(sv_options_get(&loaded, "newbie_hints", &value) && !value);
    assert(sv_options_get(&loaded, "censor_swearing", &value) && value);
    assert(sv_options_load_named(&loaded, argv[1], "custom.opt"));
    assert(sv_options_get(&loaded, "censor_swearing", &value) && !value);
    assert(!sv_options_load_named(&loaded, argv[1], "absent.opt"));
    assert(!sv_options_load_named(&loaded, argv[1], "../macro.prf"));
    bytes = read_bytes(macro_prf);
    assert(!strcmp(bytes, "A:original-macro\n"));
    free(bytes);
    sv_settings_end(&s);

    /* A same-key OPT edit made by another instance wins and stays reported. */
    assert(sv_options_load_base(&loaded, argv[1]));
    assert(sv_settings_begin(&s, argv[1], &profile, &loaded));
    assert(sv_settings_edit_option(&s, "newbie_hints", true));
    assert(sv_settings_options_target(&s, SV_OPTIONS_GLOBAL, NULL));
    bytes = read_bytes(global);
    char *line = strstr(bytes, "X:newbie_hints\n");
    assert(line);
    *line = 'Y';
    write_bytes(global, bytes);
    free(bytes);
    assert(sv_settings_save_options(&s) == SV_SETTINGS_CONFLICT);
    assert(sv_settings_save_options(&s) == SV_SETTINGS_CONFLICT);
    bytes = read_bytes(global);
    assert(strstr(bytes, "Y:newbie_hints\n"));
    free(bytes);
    sv_settings_end(&s);

    /* A fresh Save writes only edits, not unrelated runtime/CLI values. */
    write_bytes(cfg, "");
    assert(sv_profile_load(&profile, argv[1]));
    assert(sv_profile_edit(&profile, "svUiScalePercent", "135"));
    assert(sv_settings_begin(&s, argv[1], &profile, &options));
    assert(sv_settings_edit_cfg(&s, "svWindowMode", "window"));
    assert(sv_settings_save_cfg(&s) == SV_SETTINGS_OK);
    bytes = read_bytes(cfg);
    assert(strstr(bytes, "svWindowMode\twindow\n"));
    assert(!strstr(bytes, "svUiScalePercent\t"));
    free(bytes);
    sv_settings_end(&s);
    puts("SV-B-004 settings production save checks passed");
    return 0;
}
