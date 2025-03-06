/* $Id$ */
/* File: util.c */

/* Purpose: Angband utilities -BEN- */


#define SERVER

#include "angband.h"
#ifdef TOMENET_WORLDS
 #include "../world/world.h"
#endif

/* For gettimeofday */
#include <sys/time.h>


/* Ignore roman number suffix when checking for similar names? */
#define SIMILAR_ROMAN

/* Attempt to concatenate multiple chat lines to spot broken-up swear words? */
#define ENABLE_MULTILINE_CENSOR

/* Checking intercept and dodge chances gives not just vs current level monster but also 2x that level info?
   (Note: Parry and block are independant of monster level, so they aren't affected.) */
#define CHECK_CHANCES_DUAL



static void console_talk_aux(char *message);

#ifndef HAS_MEMSET
/*
 * For those systems that don't have "memset()"
 *
 * Set the value of each of 'n' bytes starting at 's' to 'c', return 's'
 * If 'n' is negative, you will erase a whole lot of memory.
 */
char *memset(char *s, int c, huge n) {
	char *t;
	for (t = s; n--; ) *t++ = c;
	return(s);
}
#endif



#ifndef HAS_STRICMP

/*
 * For those systems that don't have "stricmp()"
 *
 * Compare the two strings "a" and "b" ala "strcmp()" ignoring case.
 */
int stricmp(cptr a, cptr b) {
	cptr s1, s2;
	char z1, z2;

	/* Scan the strings */
	for (s1 = a, s2 = b; TRUE; s1++, s2++) {
		z1 = FORCEUPPER(*s1);
		z2 = FORCEUPPER(*s2);
		if (z1 < z2) return(-1);
		if (z1 > z2) return(1);
		if (!z1) return(0);
	}
}

#endif

int in_banlist(char *acc, char *addr, int *time, char *reason) {
	struct combo_ban *ptr;
	int found = 0x0, time_longest = 0;

	for (ptr = banlist; ptr != (struct combo_ban*)NULL; ptr = ptr->next) {
		if (ptr->ip[0] && addr && !strcmp(addr, ptr->ip)) found |= 0x1;
		if (ptr->acc[0] && acc && !strcmp(acc, ptr->acc)) found |= 0x2;

		/* Of all applicable bans, return the one with the highest duration.
		   If more than one of equally long duraation are found, return the last one in the list.
		   time = -1 is the longest duration aka "permanently". */
		if (ptr->time == -1 || (time_longest != -1 && time_longest < ptr->time)) {
			/* Longer or at least same length ban found, use this new ban's duration */
			if (time) *time = ptr->time;
			if (reason) {
				/* If a reason is given, overwrite the previous reason with this new ban's reason */
				if (ptr->reason[0]) strcpy(reason, ptr->reason);
				/* If NO reason is given, only clear the previous reason (if it existed) if the new duration is longer than the previous one,
				   but keep the 'old' reason if they actually have the same duration. */
				else if (reason[0] && (time_longest != ptr->time)) reason[0] = 0;
			}

			/* Remember this as so far longest duration, and therefore prioritized, ban. */
			time_longest = ptr->time;
		}
	}
	return(found);
}

void check_banlist() {
	struct combo_ban *ptr, *new, *old = (struct combo_ban*)NULL;

	ptr = banlist;
	while (ptr != (struct combo_ban*)NULL) {
		if (ptr->time > 0) { // -1 stands for 'permanent'
			if (!(--ptr->time)) {
				if (ptr->reason[0]) s_printf("Unbanning due to ban timeout (ban reason was '%s'):\n", ptr->reason);
				else s_printf("Unbanning due to ban timeout:\n");
				if (ptr->ip[0]) s_printf(" Connections from %s\n", ptr->ip);
				if (ptr->acc[0]) s_printf(" Connections for %s\n", ptr->acc);

				if (!old) {
					banlist = ptr->next;
					new = banlist;
				} else {
					old->next = ptr->next;
					new = old->next;
				}
				free(ptr);
				ptr = new;
				continue;
			}
		}
		ptr = ptr->next;
	}
}


#ifdef SET_UID

# ifndef HAS_USLEEP

/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
static int usleep(huge microSeconds) {
	struct timeval Timer;
	int nfds = 0;
#ifdef FD_SET
	fd_set *no_fds = NULL;
#else
	int *no_fds = NULL;
#endif


	/* Was: int readfds, writefds, exceptfds; */
	/* Was: readfds = writefds = exceptfds = 0; */


	/* Paranoia -- No excessive sleeping */
	if (microSeconds > 4000000L) core("Illegal usleep() call");


	/* Wait for it */
	Timer.tv_sec = (microSeconds / 1000000L);
	Timer.tv_usec = (microSeconds % 1000000L);

	/* Wait for it */
	if (select(nfds, no_fds, no_fds, no_fds, &Timer) < 0) {
		/* Hack -- ignore interrupts */
		if (errno != EINTR) return(-1);
	}

	/* Success */
	return(0);
}

# endif


/*
 * Hack -- External functions
 */
extern struct passwd *getpwuid (__uid_t __uid);
extern struct passwd *getpwnam (const char *__name) __nonnull ((1));

/*
 * Find a default user name from the system.
 */
void user_name(char *buf, int id) {
	struct passwd *pw;

	/* Look up the user name */
	if ((pw = getpwuid(id))) {
		(void)strcpy(buf, pw->pw_name);
		buf[16] = '\0';

#ifdef CAPITALIZE_USER_NAME
		/* Hack -- capitalize the user name */
		if (islower(buf[0])) buf[0] = toupper(buf[0]);
#endif

		return;
	}

	/* Oops.  Hack -- default to "PLAYER" */
	strcpy(buf, "PLAYER");
}

#endif /* SET_UID */




/*
 * The concept of the "file" routines below (and elsewhere) is that all
 * file handling should be done using as few routines as possible, since
 * every machine is slightly different, but these routines always have the
 * same semantics.
 *
 * In fact, perhaps we should use the "path_parse()" routine below to convert
 * from "canonical" filenames (optional leading tilde's, internal wildcards,
 * slash as the path seperator, etc) to "system" filenames (no special symbols,
 * system-specific path seperator, etc).  This would allow the program itself
 * to assume that all filenames are "Unix" filenames, and explicitly "extract"
 * such filenames if needed (by "path_parse()", or perhaps "path_canon()").
 *
 * Note that "path_temp" should probably return a "canonical" filename.
 *
 * Note that "my_fopen()" and "my_open()" and "my_make()" and "my_kill()"
 * and "my_move()" and "my_copy()" should all take "canonical" filenames.
 *
 * Note that "canonical" filenames use a leading "slash" to indicate an absolute
 * path, and a leading "tilde" to indicate a special directory, and default to a
 * relative path, but MSDOS uses a leading "drivename plus colon" to indicate the
 * use of a "special drive", and then the rest of the path is parsed "normally",
 * and MACINTOSH uses a leading colon to indicate a relative path, and an embedded
 * colon to indicate a "drive plus absolute path", and finally defaults to a file
 * in the current working directory, which may or may not be defined.
 *
 * We should probably parse a leading "~~/" as referring to "ANGBAND_DIR". (?)
 */


#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 */


#else /* ACORN */


#ifdef SET_UID

/*
 * Extract a "parsed" path from an initial filename
 * Normally, we simply copy the filename into the buffer
 * But leading tilde symbols must be handled in a special way
 * Replace "~user/" by the home directory of the user named "user"
 * Replace "~/" by the home directory of the current user
 */
errr path_parse(char *buf, int max, cptr file) {
	cptr u, s;
	struct passwd *pw;
	char user[128];


	/* Assume no result */
	buf[0] = '\0';

	/* No file? */
	if (!file) return(-1);

	/* File needs no parsing */
	if (file[0] != '~') {
		strcpy(buf, file);
		return(0);
	}

	/* Point at the user */
	u = file + 1;

	/* Look for non-user portion of the file */
	s = strstr(u, PATH_SEP);

	/* Hack -- no long user names */
	if (s && (s >= u + sizeof(user))) return(1);

	/* Extract a user name */
	if (s) {
		int i;

		for (i = 0; u < s; ++i) user[i] = *u++;
		user[i] = '\0';
		u = user;
	}

	/* Look up the "current" user */
	if (u[0] == '\0') u = getlogin();

	/* Look up a user (or "current" user) */
	if (u) pw = getpwnam(u);
	else pw = getpwuid(getuid());

	/* Nothing found? */
	if (!pw) return(1);

	/* Make use of the info */
	(void)strcpy(buf, pw->pw_dir);

	/* Append the rest of the filename, if any */
	if (s) (void)strcat(buf, s);

	/* Success */
	return(0);
}


#else /* SET_UID */


/*
 * Extract a "parsed" path from an initial filename
 *
 * This requires no special processing on simple machines,
 * except for verifying the size of the filename.
 */
errr path_parse(char *buf, int max, cptr file)
{
	/* Accept the filename */
	strnfmt(buf, max, "%s", file);

	/* Success */
	return(0);
}


#endif /* SET_UID */


/*
 * Hack -- acquire a "temporary" file name if possible
 *
 * This filename is always in "system-specific" form.
 */
errr path_temp(char *buf, int max)
{
#ifdef WINDOWS
	static u32b tmp_counter;
	static char valid_characters[] =
			"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char rand_ext[4];

	rand_ext[0] = valid_characters[rand_int(sizeof (valid_characters))];
	rand_ext[1] = valid_characters[rand_int(sizeof (valid_characters))];
	rand_ext[2] = valid_characters[rand_int(sizeof (valid_characters))];
	rand_ext[3] = '\0';
	strnfmt(buf, max, "%s/server_%ud.%s", ANGBAND_DIR_DATA, tmp_counter, rand_ext);
	tmp_counter++;
#else
	cptr s;

	/* Temp file */
	s = tmpnam(NULL);

	/* Oops */
	if (!s) return(-1);

	/* Format to length */
	strnfmt(buf, max, "%s", s);
#endif
	/* Success */
	return(0);
}

/*
 * Hack -- replacement for "fopen()"
 */
FILE *my_fopen(cptr file, cptr mode) {
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return(NULL);

	/* Attempt to fopen the file anyway */
	return(fopen(buf, mode));
}


/*
 * Hack -- replacement for "fclose()"
 */
errr my_fclose(FILE *fff) {
	/* Require a file */
	if (!fff) return(-1);

	/* Close, check for error */
	if (fclose(fff) == EOF) return(1);

	/* Success */
	return(0);
}


#endif /* ACORN */


/*
 * Hack -- replacement for "fgets()"
 *
 * Read a string, without a newline, to a file
 *
 * Process tabs, strip internal non-printables
 */
static errr my_fgets_aux(FILE *fff, char *buf, huge n, bool conv, bool col) {
	huge i = 0;
	char *s;
	char tmp[1024];

	/* Read a line */
	if (fgets(tmp, 1024, fff)) {
		/* Convert weirdness */
		for (s = tmp; *s; s++) {
			/* Handle newline */
			if (*s == '\n') {
				/* Terminate */
				buf[i] = '\0';

				/* Success */
				return(0);
			}

			/* Handle tabs */
			else if (*s == '\t') {
				/* Hack -- require room */
				if (i + 8 >= n) break;

				/* Append a space */
				buf[i++] = ' ';

				/* Append some more spaces */
				while (!(i % 8)) buf[i++] = ' ';
			}

#ifdef X_INFO_TXT_COLOURS
			/* Even without 'conv' being TRUE: Allow using \{c colour codes in *_info.txt files too */
			else if (col && *s == '\\' && *(s + 1) == '{') {
				/* Convert '\{' to '\377' colour code */
				*s = '\377';

				/* Copy */
				buf[i++] = *s;

				/* Skip next char ('{') */
				s++;

				/* Check length */
				if (i >= n) break;
			}
#endif

			/* Handle printables */
			else if (isprint(*s) || *s == '\377') {
				/* easier to edit perma files */
				if (conv && *s == '{' && *(s + 1) != '{') *s = '\377';

				/* Copy */
				buf[i++] = *s;

				/* Check length */
				if (i >= n) break;
			}
		}
	}

	/* Nothing */
	buf[0] = '\0';

	/* Failure */
	return(1);
}
errr my_fgets(FILE *fff, char *buf, huge n, bool conv) {
	return(my_fgets_aux(fff, buf, n, conv, FALSE));
}
errr my_fgets_col(FILE *fff, char *buf, huge n, bool conv) {
	return(my_fgets_aux(fff, buf, n, conv, TRUE));
}


/*
 * Hack -- replacement for "fputs()"
 *
 * Dump a string, plus a newline, to a file
 *
 * XXX XXX XXX Process internal weirdness?
 */
errr my_fputs(FILE *fff, cptr buf, huge n)
{
	/* XXX XXX */
	n = n ? n : 0;

	/* Dump, ignore errors */
	(void)fprintf(fff, "%s\n", buf);

	/* Success */
	return(0);
}


#ifdef ACORN


/*
 * Most of the "file" routines for "ACORN" should be in "main-acn.c"
 *
 * Many of them can be rewritten now that only "fd_open()" and "fd_make()"
 * and "my_fopen()" should ever create files.
 */


#else /* ACORN */


/*
 * Code Warrior is a little weird about some functions
 */
#ifdef BEN_HACK
extern int open(const char *, int, ...);
extern int close(int);
extern int read(int, void *, unsigned int);
extern int write(int, const void *, unsigned int);
extern long lseek(int, long, int);
#endif /* BEN_HACK */


/*
 * The Macintosh is a little bit brain-dead sometimes
 */
#ifdef MACINTOSH
# define open(N,F,M) open((char*)(N),F)
# define write(F,B,S) write(F,(char*)(B),S)
#endif /* MACINTOSH */


/*
 * Several systems have no "O_BINARY" flag
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif /* O_BINARY */


/*
 * Hack -- attempt to delete a file
 */
errr fd_kill(cptr file) {
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return(-1);

	/* Remove */
	(void)remove(buf);

	/* XXX XXX XXX */
	return(0);
}


/*
 * Hack -- attempt to move a file
 */
errr fd_move(cptr file, cptr what) {
	char buf[1024];
	char aux[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return(-1);

	/* Hack -- Try to parse the path */
	if (path_parse(aux, 1024, what)) return(-1);

	/* Rename */
	(void)rename(buf, aux);

	/* XXX XXX XXX */
	return(0);
}


/*
 * Hack -- attempt to copy a file
 */
errr fd_copy(cptr file, cptr what) {
	char buf[1024];
	char aux[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return(-1);

	/* Hack -- Try to parse the path */
	if (path_parse(aux, 1024, what)) return(-1);

	/* Copy XXX XXX XXX */
	/* (void)rename(buf, aux); */

	/* XXX XXX XXX */
	return(1);
}


/*
 * Hack -- attempt to open a file descriptor (create file)
 *
 * This function should fail if the file already exists
 *
 * Note that we assume that the file should be "binary"
 *
 * XXX XXX XXX The horrible "BEN_HACK" code is for compiling under
 * the CodeWarrior compiler, in which case, for some reason, none
 * of the "O_*" flags are defined, and we must fake the definition
 * of "O_RDONLY", "O_WRONLY", and "O_RDWR" in "A-win-h", and then
 * we must simulate the effect of the proper "open()" call below.
 */
int fd_make(cptr file, int mode) {
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return(-1);

#ifdef BEN_HACK

	/* Check for existence */
	/* if (fd_close(fd_open(file, O_RDONLY | O_BINARY))) return(1); */

	/* Mega-Hack -- Create the file */
	(void)my_fclose(my_fopen(file, "wb"));

	/* Re-open the file for writing */
	return(open(buf, O_WRONLY | O_BINARY, mode));

#else /* BEN_HACK */

	/* Create the file, fail if exists, write-only, binary */
	return(open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, mode));

#endif /* BEN_HACK */

}


/*
 * Hack -- attempt to open a file descriptor (existing file)
 *
 * Note that we assume that the file should be "binary"
 */
int fd_open(cptr file, int flags) {
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return(-1);

	/* Attempt to open the file */
	return(open(buf, flags | O_BINARY, 0));
}


/*
 * Hack -- attempt to lock a file descriptor
 *
 * Legal lock types -- F_UNLCK, F_RDLCK, F_WRLCK
 */
errr fd_lock(int fd, int what) {
	/* XXX XXX */
	what = what ? what : 0;

	/* Verify the fd */
	if (fd < 0) return(-1);

#ifdef SET_UID

# ifdef USG

	/* Un-Lock */
	if (what == F_UNLCK) {
		/* Unlock it, Ignore errors */
		lockf(fd, F_ULOCK, 0);
	}
	/* Lock */
	else {
		/* Lock the score file */
		if (lockf(fd, F_LOCK, 0) != 0) return(1);
	}

#else

	/* Un-Lock */
	if (what == F_UNLCK) {
		/* Unlock it, Ignore errors */
		(void)flock(fd, LOCK_UN);
	}
	/* Lock */
	else {
		/* Lock the score file */
		if (flock(fd, LOCK_EX) != 0) return(1);
	}

# endif

#endif

	/* Success */
	return(0);
}


/*
 * Hack -- attempt to seek on a file descriptor
 */
errr fd_seek(int fd, huge n) {
	huge p;

	/* Verify fd */
	if (fd < 0) return(-1);

	/* Seek to the given position */
	p = lseek(fd, n, SEEK_SET);

	/* Failure */
	if (p == (huge) -1) return(1);

	/* Failure */
	if (p != n) return(1);

	/* Success */
	return(0);
}


/*
 * Hack -- attempt to read data from a file descriptor
 */
errr fd_read(int fd, char *buf, huge n) {
	/* Verify the fd */
	if (fd < 0) return(-1);

#ifndef SET_UID

	/* Read pieces */
	while (n >= 16384) {
		/* Read a piece */
		if (read(fd, buf, 16384) != 16384) return(1);

		/* Shorten the task */
		buf += 16384;

		/* Shorten the task */
		n -= 16384;
	}

#endif

	/* Read the final piece */
	if ((huge) read(fd, buf, n) != n) return(1);

	/* Success */
	return(0);
}


/*
 * Hack -- Attempt to write data to a file descriptor
 */
errr fd_write(int fd, cptr buf, huge n) {
	/* Verify the fd */
	if (fd < 0) return(-1);

#ifndef SET_UID

	/* Write pieces */
	while (n >= 16384) {
		/* Write a piece */
		if (write(fd, buf, 16384) != 16384) return(1);

		/* Shorten the task */
		buf += 16384;

		/* Shorten the task */
		n -= 16384;
	}

#endif

	/* Write the final piece */
	if ((huge) write(fd, buf, n) != n) return(1);

	/* Success */
	return(0);
}


/*
 * Hack -- attempt to close a file descriptor
 */
errr fd_close(int fd) {
	/* Verify the fd */
	if (fd < 0) return(-1);

	/* Close */
	(void)close(fd);

	/* XXX XXX XXX */
	return(0);
}


#endif /* ACORN */

/*
 * Convert a decimal to a single digit octal number
 */
static char octify(uint i) {
	return(hexsym[i % 8]);
}

/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(uint i) {
	return(hexsym[i % 16]);
}


/*
 * Convert a octal-digit into a decimal
 */
static int deoct(char c) {
	if (isdigit(c)) return(D2I(c));
	return(0);
}

/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c) {
	if (isdigit(c)) return(D2I(c));
	if (islower(c)) return(A2I(c) + 10);
	if (isupper(c)) return(A2I(tolower(c)) + 10);
	return(0);
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * I have no clue if this function correctly handles, for example,
 * parsing "\xFF" into a (signed) char.  Whoever thought of making
 * the "sign" of a "char" undefined is a complete moron.  Oh well.
 */
void text_to_ascii(char *buf, cptr str) {
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str) {
		/* Backslash codes */
		if (*str == '\\') {
			/* Skip the backslash */
			str++;

			/* Hex-mode XXX */
			if (*str == 'x') {
				*s = 16 * dehex(*++str);
				*s++ += dehex(*++str);
			}
			/* Hack -- simple way to specify "backslash" */
			else if (*str == '\\')
				*s++ = '\\';
			/* Hack -- simple way to specify "caret" */
			else if (*str == '^')
				*s++ = '^';
			/* Hack -- simple way to specify "space" */
			else if (*str == 's')
				*s++ = ' ';
			/* Hack -- simple way to specify Escape */
			else if (*str == 'e')
				*s++ = ESCAPE;
			/* Backspace */
			else if (*str == 'b')
				*s++ = '\b';
			/* Newline */
			else if (*str == 'n')
				*s++ = '\n';
			/* Return */
			else if (*str == 'r')
				*s++ = '\r';
			/* Tab */
			else if (*str == 't')
				*s++ = '\t';
			/* Octal-mode */
			else if (*str == '0') {
				*s = 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}
			/* Octal-mode */
			else if (*str == '1') {
				*s = 64 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}
			/* Octal-mode */
			else if (*str == '2') {
				*s = 64 * 2 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}
			/* Octal-mode */
			else if (*str == '3') {
				*s = 64 * 3 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Skip the final char */
			str++;
		}
		/* Normal Control codes */
		else if (*str == '^') {
			str++;
			*s++ = (*str++ & 037);
		}
		/* Normal chars */
		else *s++ = *str++;
	}

	/* Terminate */
	*s = '\0';
}


/*
 * Hack -- convert a string into a printable form
 */
void ascii_to_text(char *buf, cptr str) {
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str) {
		byte i = (byte)(*str++);

		if (i == ESCAPE) {
			*s++ = '\\';
			*s++ = 'e';
		} else if (i == ' ') {
			*s++ = '\\';
			*s++ = 's';
		} else if (i == '\b') {
			*s++ = '\\';
			*s++ = 'b';
		} else if (i == '\t') {
			*s++ = '\\';
			*s++ = 't';
		} else if (i == '\n') {
			*s++ = '\\';
			*s++ = 'n';
		} else if (i == '\r') {
			*s++ = '\\';
			*s++ = 'r';
		} else if (i == '^') {
			*s++ = '\\';
			*s++ = '^';
		} else if (i == '\\') {
			*s++ = '\\';
			*s++ = '\\';
		} else if (i < 32) {
			*s++ = '^';
			*s++ = i + 64;
		} else if (i < 127)
			*s++ = i;
		else if (i < 64) {
			*s++ = '\\';
			*s++ = '0';
			*s++ = octify(i / 8);
			*s++ = octify(i % 8);
		} else {
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify(i / 16);
			*s++ = hexify(i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}

/*
 * Local variable -- we are inside a "control-underscore" sequence
 */
/*static bool parse_under = FALSE;*/

/*
 * Local variable -- we are inside a "control-backslash" sequence
 */
/*static bool parse_slash = FALSE;*/

/*
 * Local variable -- we are stripping symbols for a while
 */
/*static bool strip_chars = FALSE;*/

/*
 * Flush the screen, make a noise
 */
void bell(void)
{
}


/*
 * Mega-Hack -- Make a (relevant?) sound
 */
#ifndef USE_SOUND_2010
void sound(int Ind, int val) {
	//Send_sound(Ind, val);
	Send_sound(Ind, val, 0, 0, 100, 0, 0, 0);
}
#else
/* Plays sound effect for a player, located directly at his position.
   'type' is used client-side, for efficiency options concerning near-simultaneous sounds
   'nearby' means if other players nearby would be able to also hear the sound. - C. Blue */
void sound(int Ind, cptr name, cptr alternative, int type, bool nearby) {
	bool dm = Players[Ind]->admin_dm;
#if 0 /* non-optimized way (causes LUA errors if sound() is called in fire_ball() which is in turn called from LUA - C. Blue */
	int val, val2;

	val = exec_lua(0, format("return get_sound_index(\"%s\")", name));

	if (alternative) {
		val2 = exec_lua(0, format("return get_sound_index(\"%s\")", alternative));
	} else {
		val2 = -1;
	}

#else /* optimized way... */
	player_type *p_ptr = Players[Ind];
	int val = -1, val2 = -1, i, d;

	/* Don't reveal the hidden DM by transportation sounds, especially when recalling */
	if (name && p_ptr->admin_dm && cfg.secret_dungeon_master && (
	    //!strcmp(name, "blink") ||
	    !strcmp(name, "phase_door") || !strcmp(name, "teleport")))
		nearby = FALSE;

	/* backward compatibility */
	if (type == SFX_TYPE_STOP && !is_newer_than(&p_ptr->version, 4, 6, 1, 1, 0, 0)) return;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

#endif

	//if (is_admin(p_ptr)) s_printf("USE_SOUND_2010: looking up sound %s -> %d.\n", name, val);

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			/* hack for SFX_TYPE_STOP */
			if (name || alternative)

			return;
		}
	}

	/* also send sounds to nearby players, depending on sound or sound type */
	if (nearby) {
		for (i = 1; i <= NumPlayers; i++) {
			if (Players[i]->conn == NOT_CONNECTED) continue;
			if (!inarea(&Players[i]->wpos, &p_ptr->wpos)) continue;
			if (dm && !Players[i]->player_sees_dm) continue;
			if (Ind == i) continue;

			/* backward compatibility */
			if (type == SFX_TYPE_STOP && !is_newer_than(&Players[i]->version, 4, 6, 1, 1, 0, 0)) continue;

			d = distance(p_ptr->py, p_ptr->px, Players[i]->py, Players[i]->px);
#if 0
			if (d > 10) continue;
			if (d == 0) d = 1; //paranoia oO

			Send_sound(i, val, val2, type, 100 / d, p_ptr->id, 0, 0);
			//Send_sound(i, val, val2, type, (6 - d) * 20, p_ptr->id);  hm or this?
#elif 0
			if (d > MAX_SIGHT) continue;
			d += 3;
			d /= 2;
			Send_sound(i, val, val2, type, 100 / d, p_ptr->id, 0, 0);
#else
			if (d > MAX_SIGHT) continue;
			Send_sound(i, val, val2, type, 100 - (d * 50) / 11, p_ptr->id, p_ptr->px - Players[i]->px, p_ptr->py - Players[i]->py);
#endif
		}
	}

	Send_sound(Ind, val, val2, type, 100, p_ptr->id, 0, 0);
}
/* Like sound() but allows specifying a volume. */
void sound_vol(int Ind, cptr name, cptr alternative, int type, bool nearby, int vol) {
	player_type *p_ptr = Players[Ind];
	int val = -1, val2 = -1, i, d;
	bool dm = p_ptr->admin_dm;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}


	//if (is_admin(Players[Ind])) s_printf("USE_SOUND_2010: looking up sound %s -> %d.\n", name, val);

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* also send sounds to nearby players, depending on sound or sound type */
	if (nearby) {
		for (i = 1; i <= NumPlayers; i++) {
			if (Players[i]->conn == NOT_CONNECTED) continue;
			if (!inarea(&Players[i]->wpos, &p_ptr->wpos)) continue;
			if (dm && !Players[i]->player_sees_dm) continue;
			if (Ind == i) continue;

			d = distance(p_ptr->py, p_ptr->px, Players[i]->py, Players[i]->px);
#if 0
			if (d > 10) continue;
			if (d == 0) d = 1; //paranoia oO

			Send_sound(i, val, val2, type, vol / d, p_ptr->id, 0, 0);
			//Send_sound(i, val, val2, type, vol... (6 - d) * 20, p_ptr->id);  hm or this?
#elif 0
			if (d > MAX_SIGHT) continue;
			d += 3;
			d /= 2;

			Send_sound(i, val, val2, type, vol / d, p_ptr->id, 0, 0);
#else
			if (d > MAX_SIGHT) continue;
			Send_sound(i, val, val2, type, ((100 - (d * 50) / 11) * vol) / 100, p_ptr->id, p_ptr->px - Players[i]->px, p_ptr->py - Players[i]->py);
#endif
		}
	}
	Send_sound(Ind, val, val2, type, vol, p_ptr->id, 0, 0);
}
/* Send sound to origin player and destination player,
   with destination player getting reduced volume depending on distance */
void sound_pair(int Ind_org, int Ind_dest, cptr name, cptr alternative, int type) {
	int val = -1, val2 = -1, i, d;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	if (Players[Ind_dest]->conn != NOT_CONNECTED &&
	    inarea(&Players[Ind_dest]->wpos, &Players[Ind_org]->wpos) &&
	    Ind_dest != Ind_org) {
		d = distance(Players[Ind_org]->py, Players[Ind_org]->px, Players[Ind_dest]->py, Players[Ind_dest]->px);
		if (d <= MAX_SIGHT) {
#if 0
			d += 3;
			d /= 2;
			Send_sound(Ind_dest, val, val2, type, 100 / d, Players[Ind_org]->id, 0, 0);
#else
			Send_sound(Ind_dest, val, val2, type, 100 - (d * 50) / 11, Players[Ind_org]->id, Players[Ind_org]->px - Players[Ind_dest]->px, Players[Ind_org]->py - Players[Ind_dest]->py);
#endif
		}
	}

	Send_sound(Ind_org, val, val2, type, 100, Players[Ind_org]->id, 0, 0);
}
/* Send sound to everyone on a particular wpos sector/dungeon floor */
void sound_floor_vol(struct worldpos *wpos, cptr name, cptr alternative, int type, int vol) {
	int val = -1, val2 = -1, i;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (!inarea(&Players[i]->wpos, wpos)) continue;
		Send_sound(i, val, val2, type, vol, Players[i]->id, 0, 0);
	}
}
/* send sound to player and everyone nearby at full volume, similar to
   msg_..._near(), except it also sends to the player himself.
   This is used for highly important and *loud* sounds such as 'shriek' - C. Blue */
void sound_near(int Ind, cptr name, cptr alternative, int type) {
	int i, d;

	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (!inarea(&Players[i]->wpos, &Players[Ind]->wpos)) continue;

		/* Can player see the target player? */
		//if (!(Players[i]->cave_flag[Players[Ind]->py][Players[Ind]->px] & CAVE_VIEW)) continue;

		/* within audible range? */
		d = distance(Players[Ind]->py, Players[Ind]->px, Players[i]->py, Players[i]->px);
		if (d > MAX_SIGHT) continue;

		sound(i, name, alternative, type, FALSE);
	}
}
/* send sound to all players nearby a certain location, and allow to specify
   a player to exclude, same as msg_print_near_site() for messages. - C. Blue
   'viewable': If TRUE, player must have view on x,y to hear the sound at all. */
void sound_near_site(int y, int x, worldpos *wpos, int Ind, cptr name, cptr alternative, int type, bool viewable) {
	int i, d;
	player_type *p_ptr;
	int val = -1, val2 = -1;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Skip specified player, if any */
		if (i == Ind) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Can player see the site via LOS? */
		if (viewable && !(p_ptr->cave_flag[y][x] & CAVE_VIEW)) continue;

		/* within audible range? */
		d = distance(y, x, Players[i]->py, Players[i]->px);
		/* NOTE: should be consistent with msg_print_near_site() */
		if (d > MAX_SIGHT) continue;

#if 0
		/* limit for volume calc */
		if (d > 20) d = 20;
		d += 3;
		d /= 3;
		Send_sound(i, val, val2, type, 100 / d, 0, 0, 0);
#else
		/* limit for volume calc */
		Send_sound(i, val, val2, type, 100 - (d * 50) / 11, 0, x - Players[i]->px, y - Players[i]->py);
#endif
	}
}
/* send sound to all players nearby a certain location, and allow to specify
   a player to exclude, same as msg_print_near_site() for messages. - C. Blue
   'viewable': If TRUE, player must have view on x,y to hear the sound at all. */
void sound_near_site_vol(int y, int x, worldpos *wpos, int Ind, cptr name, cptr alternative, int type, bool viewable, int vol) {
	int i, d;
	player_type *p_ptr;
	int val = -1, val2 = -1;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Skip specified player, if any */
		if (i == Ind) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Can player see the site via LOS? */
		if (viewable && !(p_ptr->cave_flag[y][x] & CAVE_VIEW)) continue;

		/* within audible range? */
		d = distance(y, x, Players[i]->py, Players[i]->px);
		/* NOTE: should be consistent with msg_print_near_site() */
		if (d > MAX_SIGHT) continue;

#if 0
		/* limit for volume calc */
		if (d > 20) d = 20;
		d += 3;
		d /= 3;
		Send_sound(i, val, val2, type, ((100 / d) * vol) / 100, 0, 0, 0);
#else
		/* limit for volume calc */
		Send_sound(i, val, val2, type, ((100 - (d * 50) / 11) * vol) / 100, 0, x - Players[i]->px, y - Players[i]->py);
#endif
	}
}
/* Send sound to all players nearby a certain area, and have no volume falloff inside the area's radius.
   Added for earthquakes, so people outside of the quake region still hear it somewhat. - C. Blue */
void sound_near_area(int y, int x, int rad, worldpos *wpos, cptr name, cptr alternative, int type) {
	int i, d;
	player_type *p_ptr;
	int val = -1, val2 = -1;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* within audible range? */
		d = distance(y, x, Players[i]->py, Players[i]->px);

		/* no falloff within the area */
		d -= rad;
		if (d < 0) d = 0;

		/* NOTE: should be consistent with msg_print_near_site() */
		if (d > MAX_SIGHT) continue;

#if 0
		/* limit for volume calc */
		if (d > 20) d = 20;
		d += 3;
		d /= 3;
		Send_sound(i, val, val2, type, 100 / d, 0, 0, 0);
#else
		/* limit for volume calc */
		Send_sound(i, val, val2, type, 100 - (d * 50) / 11, 0, x - Players[i]->px, y - Players[i]->py);
#endif
	}
}
/* Play sfx at full volume to everyone in a house, and at normal-distance volume to
   everyone near the door (as sound_near_site() would). */
void sound_house_knock(int h_idx, int dx, int dy) {
	house_type *h_ptr = &houses[h_idx];

	fill_house(h_ptr, FILL_SFX_KNOCK, NULL);

	/* basically sound_near_site(), except we skip players on CAVE_ICKY grids, ie those who are in either this or other houses! */
	int i, d;
	player_type *p_ptr;
	int val = -1, val2 = -1;

	const char *name = (houses[h_idx].flags & HF_MOAT) ? "knock_castle" : "knock";
	const char *alternative = "block_shield_projectile";
	cave_type **zcave;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, &h_ptr->wpos)) continue;

		zcave = getcave(&p_ptr->wpos);
		if (!zcave) continue;//paranoia
		/* Specialty for sound_house(): Is player NOT in a house? */
		if ((zcave[p_ptr->py][p_ptr->px].info & CAVE_ICKY)
		    /* however, exempt the moat! */
		    && zcave[p_ptr->py][p_ptr->px].feat != FEAT_DRAWBRIDGE
		    && zcave[p_ptr->py][p_ptr->px].feat != FEAT_DRAWBRIDGE_HORIZ
		    && zcave[p_ptr->py][p_ptr->px].feat != FEAT_DEEP_WATER)
			continue;

		/* within audible range? */
		d = distance(dy, dx, Players[i]->py, Players[i]->px);
		/* hack: if we knock on the door, assume distance 0 between us and door */
		if (d > 0) d--;
		/* NOTE: should be consistent with msg_print_near_site() */
		if (d > MAX_SIGHT) continue;

#if 0
		/* limit for volume calc */
		if (d > 20) d = 20;
		d += 3;
		d /= 3;
		Send_sound(i, val, val2, SFX_TYPE_COMMAND, 100 / d, 0, 0, 0);
#else
		/* limit for volume calc */
		Send_sound(i, val, val2, SFX_TYPE_COMMAND, 100 - (d * 50) / 11, 0, dx - Players[i]->px, dy - Players[i]->py);
#endif
	}

}
/* like msg_print_near_monster() just for sounds,
   basically same as sound_near_site() except no player can be exempt - C. Blue */
void sound_near_monster(int m_idx, cptr name, cptr alternative, int type) {
	int i, d;
	player_type *p_ptr;
	cave_type **zcave;

	monster_type *m_ptr = &m_list[m_idx];
	worldpos *wpos = &m_ptr->wpos;
	int val = -1, val2 = -1;

	if (!(zcave = getcave(wpos))) return;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Skip if not visible */
		//if (!p_ptr->mon_vis[m_idx]) continue;

		/* Can player see this monster via LOS? */
		//if (!(p_ptr->cave_flag[m_ptr->fy][m_ptr->fx] & CAVE_VIEW)) continue;

		/* within audible range? */
		d = distance(m_ptr->fy, m_ptr->fx, Players[i]->py, Players[i]->px);

		/* NOTE: should be consistent with msg_print_near_site() */
		if (d > MAX_SIGHT) continue;

#if 0
		/* limit for volume calc */
		if (d > 20) d = 20;
		d += 3;
		d /= 3;
		Send_sound(i, val, val2, type, 100 / d, 0, 0, 0);
#else
		/* limit for volume calc */
		Send_sound(i, val, val2, type, 100 - (d * 50) / 11, 0, m_ptr->fx - Players[i]->px, m_ptr->fy - Players[i]->py);
#endif
	}
}
/* like msg_print_near_monster() just for sounds,
   basically same as sound_near_site(). - C. Blue
   NOTE: This function assumes that it's playing a MONSTER ATTACK type sound,
         because it's checking p_ptr->sfx_monsterattack to mute it. */
void sound_near_monster_atk(int m_idx, int Ind, cptr name, cptr alternative, int type) {
	int i, d;
	player_type *p_ptr;
	cave_type **zcave;

	monster_type *m_ptr = &m_list[m_idx];
	worldpos *wpos = &m_ptr->wpos;
	int val = -1, val2 = -1;

	if (!(zcave = getcave(wpos))) return;

	if (name) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], name)) {
			val = i;
			break;
		}
	}

	if (alternative) for (i = 0; i < SOUND_MAX_2010; i++) {
		if (!audio_sfx[i][0]) break;
		if (!strcmp(audio_sfx[i], alternative)) {
			val2 = i;
			break;
		}
	}

	if (val == -1) {
		if (val2 != -1) {
			/* Use the alternative instead */
			val = val2;
			val2 = -1;
		} else {
			return;
		}
	}

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Skip specified player, if any */
		if (i == Ind) continue;

		/* Skip players who don't want to hear attack sounds */
		if (!p_ptr->sfx_monsterattack) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Skip if not visible */
		//if (!p_ptr->mon_vis[m_idx]) continue;

		/* Can player see this monster via LOS? */
		//if (!(p_ptr->cave_flag[m_ptr->fy][m_ptr->fx] & CAVE_VIEW)) continue;

		/* within audible range? */
		d = distance(m_ptr->fy, m_ptr->fx, Players[i]->py, Players[i]->px);

		/* NOTE: should be consistent with msg_print_near_site() */
		if (d > MAX_SIGHT) continue;

#if 0
		/* limit for volume calc */
		if (d > 20) d = 20;
		d += 3;
		d /= 3;
		Send_sound(i, val, val2, type, 100 / d, 0, 0, 0);
#else
		/* limit for volume calc */
		Send_sound(i, val, val2, type, 100 - (d * 50) / 11, 0, m_ptr->fx - Players[i]->px, m_ptr->fy - Players[i]->py);
#endif
	}
}

/* Find correct music for the player based on his current location - C. Blue
 * A value of -1 just means not to change anything, used for secondary music if there is no
   desired secondary music for an event.
 * An alternative music value of -2 means that the primary music is just optional ie non-canon
   (not set in the basic official music pack) and we should not stop playing our current music
   just because we cannot play the primary music wish.
 * (-3 is used internally for handling changes to shuffle option.)
 * A value of -4 means to fade out current music. Can be used as secondary music to stop playing
   in case the primary music could not be played.
 * Note - rarely played music:
 *     dungeons - generic/ironman/forcedownhellish
 *     towns - generic day/night (used for Menegroth/Nargothrond at times)
 * Note - dun-gen-iron and dun-gen-fdhell are currently swapped. */
#define GHOST_MUSIC_HACK Send_music(Ind, -1, -1, -1);
void handle_music(int Ind) {
	player_type *p_ptr = Players[Ind];
	dun_level *l_ptr = NULL;
	int i = -1, dlev = getlevel(&p_ptr->wpos);
	dungeon_type *d_ptr = getdungeon(&p_ptr->wpos);
	cave_type **zcave = getcave(&p_ptr->wpos);

#ifdef ARCADE_SERVER
	/* Special music for Arcade Server stuff */
	if (p_ptr->wpos.wx == WPOS_ARCADE_X && p_ptr->wpos.wy == WPOS_ARCADE_Y
	//    && p_ptr->wpos.wz == WPOS_ARCADE_Z
	    ) {
		p_ptr->music_monster = -2;
 #if 0
		if (p_ptr->wpos.wz == 0) {
			if (night_surface) Send_music(Ind, 49, 1, 0); /* 'generic town night' music (and 'generic town' -> 'generic' as fallback) instead of Bree default */
			else Send_music(Ind, 1, 0, -1); /* 'generic town' music (and 'generic' as fallback) instead of Bree default */
		} else {
			//47 and 48 are actually pieces used in other arena events
			if (rand_int(2)) Send_music(Ind, 47, 1, 1);
			else Send_music(Ind, 48, 1, 1);
		}
 #else
		if (p_ptr->wpos.wz == 0) Send_music(Ind, 48, night_surface ? 49 : 1, 1); /* 'arena' ;) sounds a bit like Unreal Tournament menu music hehe (generic town(night) music as fallback) */
		else Send_music(Ind, 47, 1, 1); /* 'death match' music (pvp arena), generic town musicas fallback */
 #endif
		return;
	}
#endif

#ifdef IRONDEEPDIVE_MIXED_TYPES
	if (in_irondeepdive(&p_ptr->wpos)) {
		l_ptr = getfloor(&p_ptr->wpos);
 #if 0 /* kinda annoying to have 2 floors of 'unspecified' music - C. Blue */
		if (!iddc[ABS(p_ptr->wpos.wz)].step) i = iddc[ABS(p_ptr->wpos.wz)].type;
		else i = 0; //Transition floors
 #else
		if (iddc[ABS(p_ptr->wpos.wz)].step < 2) i = iddc[ABS(p_ptr->wpos.wz)].type;
		else i = iddc[ABS(p_ptr->wpos.wz)].next;
 #endif
	} else
#endif
	if (p_ptr->wpos.wz != 0) {
		l_ptr = getfloor(&p_ptr->wpos);
		if (p_ptr->wpos.wz < 0) {
			i = wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].dungeon->type;
#if 1			/* allow 'themed' music too? (or keep music as 'original dungeon theme' exclusively) */
			if (!i && //except if this dungeon has particular flags (because flags are NOT affected by theme):
			    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].dungeon->theme &&
			    !(d_ptr->flags1 & DF1_FORCE_DOWN) && !(d_ptr->flags2 & (DF2_NO_DEATH | DF2_IRON | DF2_HELL)))
				i = wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].dungeon->theme;
#endif
		} else {
			i = wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].tower->type;
#if 1			/* allow 'themed' music too? (or keep music as 'original dungeon theme' exclusively) */
			if (!i && //except if this dungeon has particular flags (because flags are NOT affected by theme):
			    wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].tower->theme &&
			    !(d_ptr->flags1 & DF1_FORCE_DOWN) && !(d_ptr->flags2 & (DF2_NO_DEATH | DF2_IRON | DF2_HELL)))
				i = wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].tower->theme;
#endif
		}
	}

	if (in_netherrealm(&p_ptr->wpos) && dlev == netherrealm_end) {
		//Zu-Aon
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, 45, 14, 14);
		return;
	} else if ((i != -1) &&
	    /* Assuming that only Morgoth's floor has LF1_NO_GHOST, so we don't confuse it with Zu-Aon's floor: */
	    (l_ptr->flags1 & LF1_NO_GHOST)) {
		//Morgoth
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		if (p_ptr->total_winner && (l_ptr->flags2 & LF2_DEAD_BOSS)) Send_music(Ind, 88, 44, 44);
		else Send_music(Ind, 44, 14, 14);
		return;
	}

	if (in_valinor(&p_ptr->wpos)) {
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, 8, 1, 1); //Valinor
		return;
#ifdef DM_MODULES
	} else if (in_module(&p_ptr->wpos)) {
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, exec_lua(0, format("return adventure_locale(%d, 4)", p_ptr->wpos.wz)), 0, 0);
		return;
#endif
	} else if (in_pvparena(&p_ptr->wpos)) {
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, 66, 47, 47); //PvP Arena (Highlander Deathmatch as alternative)
		return;
	} else if (ge_special_sector && in_arena(&p_ptr->wpos)) {
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, 48, 0, 0); //Monster Arena Challenge
		return;
	} else if (in_sector000(&p_ptr->wpos)) {
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, sector000music, sector000musicalt, sector000musicalt2);
		return;
	} else if (in_sector000_dun(&p_ptr->wpos)) {
		//hack: init music as 'higher priority than boss-specific':
		p_ptr->music_monster = -2;
		Send_music(Ind, sector000music_dun, sector000musicalt_dun, sector000musicalt2_dun);
		return;
	} else if (d_ptr && !d_ptr->type && d_ptr->theme == DI_DEATH_FATE) {
		if (p_ptr->wpos.wz == 1 || p_ptr->wpos.wz == -1) Send_music(Ind, 98, 55, 55); //party/halloween
		else Send_music_vol(Ind, 98, 55, 55, 20);
		return;
	} else if (d_ptr && d_ptr->type == DI_DEATH_FATE && (l_ptr->flags2 & LF2_BROKEN)) {
		if (zcave && zcave[2][55].feat == FEAT_UNSEALED_DOOR) {
			Send_music(Ind, -4, -4, -4);
			return;
		}
		Send_music(Ind, 99, 47, 47); //misc/highlander, or 13 for generic-iron
		return;
	}

	/* No-tele grid: Re-use 'terrifying' bgm for this */
	if (p_ptr->music_monster != -5 && //hack: jails are also no-tele
	    zcave && (zcave[p_ptr->py][p_ptr->px].info & CAVE_STCK)) {
#if 0 /* hack: init music as 'higher priority than boss-specific': */
		p_ptr->music_monster = -2;
#endif
		Send_music(Ind, 46, 14, 14); //No-Tele vault
		return;
	}

	/* Ghost-specific music if available client-side --
	   note that this follows-up with a client-side hack necessary to ensure that ghost music
	   won't get overriden by the next subsequent Send_music() call from those below here.
	   If another Send_music() is not called, GHOST_MUSIC_HACK is used as dummy replacement
	   to satisfy the client's expectation. (Client 4.7.4b+ handle the ghost music.) */
	if (p_ptr->ghost && !p_ptr->admin_dm) Send_music(Ind, 89, -1, -1);

	/* rest of the music has lower priority than already running, boss-specific music */
	if (p_ptr->music_monster != -1
	    /* hacks for sickbay, tavern and jail music */
	    && p_ptr->music_monster != -3 && p_ptr->music_monster != -4 && p_ptr->music_monster != -5) {
		GHOST_MUSIC_HACK
		return;
	}

	/* Shopping music ^^ */
	if (p_ptr->store_num >= 0) {
		int a;

		switch (p_ptr->store_num) {
		//black markets
		case STORE_BLACK:
		case STORE_BLACKX:
		case STORE_BLACK_DUN: //dungeon town black market
			Send_music(Ind, 94, 93, 93);
			return;
		case STORE_BLACKS: //dungeon store
			Send_music(Ind, 94, 95, 95);
			return;
		//casino music! Could contain a lot of pseudo ambient sfx too ;)
		case STORE_CASINO:
			Send_music(Ind, 96, 93, 93);
			return;
		}
		/* Hack: Shops that don't offer the option to buy (store action '2') anything, aka 'service shops' */
		for (a = 0; a < MAX_STORE_ACTIONS; a++)
			if (st_info[p_ptr->store_num].actions[a] == 2) break;
		if (a == MAX_STORE_ACTIONS) Send_music(Ind, 97, -1, -1); //service shop music
		/* Normal shops (in town or elsewhere) */
		else if (!istownarea(&p_ptr->wpos, MAX_TOWNAREA) && !isdungeontown(&p_ptr->wpos)) Send_music(Ind, 95, -1, -1); //dungeon shops (and strange world-surface shops not attached to any town, if they exist oO)
		else Send_music(Ind, 93, -1, -1); //town shops
		return;
	}

	/* on world surface */
	if (p_ptr->wpos.wz == 0) {
		/* Jail hack */
		if (p_ptr->music_monster == -5) {
			Send_music(Ind, 87, 13, 13);
			return;
		}

		/* Play town music. Also in its surrounding area of houses, if we're coming from the town.
		   NOTE: I use the term 'seasonal music' for special seasonal events: Christmas, Halloween, New Year's Eve.
		         I use the term 'season music' for music depending on the four seasons, spring, summer, autumn, winter. - C. Blue */
		if (istownarea(&p_ptr->wpos, MAX_TOWNAREA)) {
			int tmus = 1, tmus_inverse = 1; //intended town day+night music (and 1 is generic town music as first fallback)
			int tmus_reserve = 0, tmus_inverse_reserve = 0; //bandaid replacement town day+night music (and 0 is generic game music as first fallback)
			bool seasonal_music = FALSE; //instead of town music we use special seasonal music
			int tmus_season = -1;

			i = wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].town_idx;

			/* Store daily music as default, nightly music as 'inverse'. */
			switch (town[i].type) {
			default:
			case TOWN_VANILLA: tmus_reserve = 1; tmus_inverse_reserve = 49; //default town
				switch (season) {
				case SEASON_SPRING: tmus = 130; tmus_inverse = 131; break;
				case SEASON_SUMMER: tmus = 142; tmus_inverse = 143; break;
				case SEASON_AUTUMN: tmus = 154; tmus_inverse = 155; break;
				case SEASON_WINTER: tmus = 166; tmus_inverse = 167; break;
				default: tmus = tmus_reserve; tmus_inverse = tmus_inverse_reserve; //paranoia
				}
				break;
			case TOWN_BREE: tmus_reserve = 3; tmus_inverse_reserve = 50; //Bree
				switch (season) {
				case SEASON_SPRING: tmus = 132; tmus_inverse = 133; break;
				case SEASON_SUMMER: tmus = 144; tmus_inverse = 145; break;
				case SEASON_AUTUMN: tmus = 156; tmus_inverse = 157; break;
				case SEASON_WINTER: tmus = 168; tmus_inverse = 169; break;
				default: tmus = tmus_reserve; tmus_inverse = tmus_inverse_reserve; //paranoia
				}
				break;
			case TOWN_GONDOLIN: tmus_reserve = 4; tmus_inverse_reserve = 51; //Gondo
				switch (season) {
				case SEASON_SPRING: tmus = 134; tmus_inverse = 135; break;
				case SEASON_SUMMER: tmus = 146; tmus_inverse = 147; break;
				case SEASON_AUTUMN: tmus = 158; tmus_inverse = 159; break;
				case SEASON_WINTER: tmus = 170; tmus_inverse = 171; break;
				default: tmus = tmus_reserve; tmus_inverse = tmus_inverse_reserve; //paranoia
				}
				break;
			case TOWN_MINAS_ANOR: tmus_reserve = 5; tmus_inverse_reserve = 52; //Minas
				switch (season) {
				case SEASON_SPRING: tmus = 136; tmus_inverse = 137; break;
				case SEASON_SUMMER: tmus = 148; tmus_inverse = 149; break;
				case SEASON_AUTUMN: tmus = 160; tmus_inverse = 161; break;
				case SEASON_WINTER: tmus = 172; tmus_inverse = 173; break;
				default: tmus = tmus_reserve; tmus_inverse = tmus_inverse_reserve; //paranoia
				}
				break;
			case TOWN_LOTHLORIEN: tmus_reserve = 6; tmus_inverse_reserve = 53; //Loth
				switch (season) {
				case SEASON_SPRING: tmus = 138; tmus_inverse = 139; break;
				case SEASON_SUMMER: tmus = 150; tmus_inverse = 151; break;
				case SEASON_AUTUMN: tmus = 162; tmus_inverse = 163; break;
				case SEASON_WINTER: tmus = 174; tmus_inverse = 175; break;
				default: tmus = tmus_reserve; tmus_inverse = tmus_inverse_reserve; //paranoia
				}
				break;
			case TOWN_KHAZADDUM: tmus_reserve = 7; tmus_inverse_reserve = 54; //Khaz
				switch (season) {
				case SEASON_SPRING: tmus = 140; tmus_inverse = 141; break;
				case SEASON_SUMMER: tmus = 152; tmus_inverse = 153; break;
				case SEASON_AUTUMN: tmus = 164; tmus_inverse = 165; break;
				case SEASON_WINTER: tmus = 176; tmus_inverse = 177; break;
				default: tmus = tmus_reserve; tmus_inverse = tmus_inverse_reserve; //paranoia
				}
				break;
			}
			if (night_surface) { //nightly music: The 'inverse' becomes the default and vice versa
				int x;

				x = tmus;
				tmus = tmus_inverse;
				tmus_inverse = x;

				x = tmus_reserve;
				tmus_reserve = tmus_inverse_reserve;
				tmus_inverse_reserve = x;
			}

			/* Seasonal music, overrides the music of the place (usually Bree) where it mainly takes place */
			if (season_halloween) {
				/* Designated place: Bree */
				if (town[i].type == TOWN_BREE) {
					/* We use town's season-independant nightly music as replacement for dedicated event music */
					tmus_reserve = night_surface ? tmus_reserve : tmus_inverse_reserve;
					tmus = 83;
					seasonal_music = TRUE;
				}
			}
			else if (season_xmas) {
				/* Designated place: Bree */
				if (town[i].type == TOWN_BREE) {
					/* We use town's season-independant nightly music as replacement for dedicated event music */
					tmus_reserve = night_surface ? tmus_reserve : tmus_inverse_reserve;
					tmus = 84;
					seasonal_music = TRUE;
					/* Different from to halloween and new year's eve, which are both perma-night,
					   xmas can actually have separate day- and night-music: */
					tmus_season = night_surface ? 181 : 180;
				}
			}
			else if (season_newyearseve) {
				/* Designated place: Bree */
				if (town[i].type == TOWN_BREE) {
					/* We use town's season-independant nightly music as replacement for dedicated event music */
					tmus_reserve = night_surface ? tmus_reserve : tmus_inverse_reserve;
					tmus = 85;
					seasonal_music = TRUE;
				}
			}

			/* Sickbay hack */
			if (p_ptr->music_monster == -3) {
				/* Play it safe: It we don't have sickbay music we can assume that we might not have seasonal music either */
				Send_music(Ind, 86, seasonal_music ? tmus_reserve : tmus, seasonal_music ? tmus_reserve : tmus);
				return;
			}
			/* Tavern hack */
			if (p_ptr->music_monster == -4) {
				/* Abuse tmus_inverse, it stores the town's specific tavern music now. */
				if (night_surface) switch (town[i].type) { //nightly tavern music
				default:
				case TOWN_VANILLA: tmus_inverse = 69; break; //default town
				case TOWN_BREE: tmus_inverse = 71; break; //Bree
				case TOWN_GONDOLIN: tmus_inverse = 73; break; //Gondo
				case TOWN_MINAS_ANOR: tmus_inverse = 75; break; //Minas
				case TOWN_LOTHLORIEN: tmus_inverse = 77; break; //Loth
				case TOWN_KHAZADDUM: tmus_inverse = 79; break; //Khaz
				}
				else switch (town[i].type) { //daily tavern music
				default:
				case TOWN_VANILLA: tmus_inverse = 68; break; //default town
				case TOWN_BREE: tmus_inverse = 70; break; //Bree
				case TOWN_GONDOLIN: tmus_inverse = 72; break; //Gondo
				case TOWN_MINAS_ANOR: tmus_inverse = 74; break; //Minas
				case TOWN_LOTHLORIEN: tmus_inverse = 76; break; //Loth
				case TOWN_KHAZADDUM: tmus_inverse = 78; break; //Khaz
				}
				/* Actual use seasonally independant town's music as replacement.
				   If we have seasonal music, this way the tavern makes a difference.
				   If we don't have seasonal music.. we cannot use it anyway =p
				   (...and fall back to the normal town music 'reserve', depending on day/night). */
				Send_music(Ind, tmus_inverse, seasonal_music ? tmus : tmus_reserve, tmus_reserve);
				return;
			}

			/* now the specialty: If we're coming from elsewhere,
			   we only switch to town music when we enter the town.
			   If we're coming from the town, however, we keep the
			   music while being in its surrounding area of houses. */
			if (istown(&p_ptr->wpos) || p_ptr->music_current == tmus
			    /* don't switch from town area music to wild music on day/night change: */
			    || p_ptr->music_current == tmus_inverse) {
				/* If the seasonal music is split up even further into day/night, use the day-time-independant season-music as first fallback, towns normal nightly music as 2nd fallback */
				if (tmus_season != -1) Send_music(Ind, tmus_season, tmus, tmus_reserve);
				else
				/* Season-specific town music; with season-indenpendant town music as reserve.
				   (And both are day/night-specific, so at least basic (season-independant)
				    day AND night music must be present for eachs town type, or there will be silence at some point.) */
				Send_music(Ind, tmus, tmus_reserve, tmus_reserve);
				return;
			}
			/* Fall through to wilderness music */
		}

		/* Play wilderness music */
		if (night_surface) {
			int m_terrain = -1, m_season = -1, m_fallback = 10;

			/* Nightly wilderness music */
			switch (season) {
			case SEASON_SPRING: m_season = 105; break;
			case SEASON_SUMMER: m_season = 107; break;
			case SEASON_AUTUMN: m_season = 109; break;
			case SEASON_WINTER: m_season = 111; break;
			}

			/* Terrain-specific wilderness music */
			switch (wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type) {
			case WILD_LAKE: m_terrain = 121; break;
			case WILD_GRASSLAND: m_terrain = 113; break;
			case WILD_FOREST: m_terrain = 115; break;
			case WILD_VOLCANO: m_terrain = 117; break;
			case WILD_SHORE1: m_terrain = 119; break;
			case WILD_SHORE2: m_terrain = 119; break;
			case WILD_OCEANBED1: m_terrain = 119; break;
			case WILD_WASTELAND: m_terrain = 125; break;
			case WILD_UNDEFINED: m_terrain = 113; break; //paranoia
			case WILD_CLONE: m_terrain = 113; break; //paranoia
			case WILD_TOWN: m_terrain = 113; break; //paranoia
			case WILD_OCEAN: m_terrain = 119; break;
			case WILD_RIVER: m_terrain = 121; break;
			case WILD_COAST: m_terrain = 119; break;
			case WILD_MOUNTAIN: m_terrain = 117; break;
			case WILD_DENSEFOREST: m_terrain = 115; break;
			case WILD_OCEANBED2: m_terrain = 119; break;
			case WILD_DESERT: m_terrain = 127; break;
			case WILD_ICE: m_terrain = 129; break;
			case WILD_SWAMP: m_terrain = 123; break;
			}

			if (is_atleast(&p_ptr->version, 4, 8, 1, 2, 0, 0))
				Send_music(Ind, m_terrain, m_season, m_fallback);
			else Send_music(Ind, m_season, m_fallback, m_fallback);
		} else {
			int m_terrain = -1, m_season = -1, m_fallback = 9;

			/* Daily wilderness music */
			switch (season) {
			case SEASON_SPRING: m_season = 104; break;
			case SEASON_SUMMER: m_season = 106; break;
			case SEASON_AUTUMN: m_season = 108; break;
			case SEASON_WINTER: m_season = 110; break;
			}

			/* Terrain-specific wilderness music */
			switch (wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].type) {
			case WILD_LAKE: m_terrain = 120; break;
			case WILD_GRASSLAND: m_terrain = 112; break;
			case WILD_FOREST: m_terrain = 114; break;
			case WILD_VOLCANO: m_terrain = 116; break;
			case WILD_SHORE1: m_terrain = 118; break;
			case WILD_SHORE2: m_terrain = 118; break;
			case WILD_OCEANBED1: m_terrain = 118; break;
			case WILD_WASTELAND: m_terrain = 124; break;
			case WILD_UNDEFINED: m_terrain = 112; break; //paranoia
			case WILD_CLONE: m_terrain = 112; break; //paranoia
			case WILD_TOWN: m_terrain = 112; break; //paranoia
			case WILD_OCEAN: m_terrain = 118; break;
			case WILD_RIVER: m_terrain = 120; break;
			case WILD_COAST: m_terrain = 118; break;
			case WILD_MOUNTAIN: m_terrain = 116; break;
			case WILD_DENSEFOREST: m_terrain = 114; break;
			case WILD_OCEANBED2: m_terrain = 118; break;
			case WILD_DESERT: m_terrain = 126; break;
			case WILD_ICE: m_terrain = 128; break;
			case WILD_SWAMP: m_terrain = 122; break;
			}

			if (is_atleast(&p_ptr->version, 4, 8, 1, 2, 0, 0))
				Send_music(Ind, m_terrain, m_season, m_fallback);
			else Send_music(Ind, m_season, m_fallback, m_fallback);
		}
		return;
	/* in the dungeon */
	} else {
		/* Dungeon towns have their own music to bolster the player's motivation ;) */
		if (isdungeontown(&p_ptr->wpos)) {
			if (is_fixed_irondeepdive_town(&p_ptr->wpos, dlev)) {
				if (dlev == 40) {
					/* Tavern hack */
					if (p_ptr->music_monster == -4) {
						Send_music(Ind, 81, 57, 57);
						return;
					}
					Send_music(Ind, 57, 1, 1); /* Menegroth: own music, fallback to generic town */
				} else {
					/* Tavern hack */
					if (p_ptr->music_monster == -4) {
						Send_music(Ind, 82, 58, 58);
						return;
					}
					Send_music(Ind, 58, 49, 49); /* Nargothrond: own music, fallback to generic town night */
				}
			} else {
				/* Tavern hack */
				if (p_ptr->music_monster == -4) {
					Send_music(Ind, 80, 2, 2);
					return;
				}
				Send_music(Ind, 2, 1, 1); /* the usual 'dungeon town' music for this case */
			}
			return;
		}

		/* Floor-specific music (monster/boss-independant)? */
		if ((i != DI_NETHER_REALM) /* not in the Nether Realm, really ^^ */
		    && (!(d_ptr->flags2 & DF2_NO_DEATH)) /* nor in easy dungeons */
		    && !(p_ptr->wpos.wx == WPOS_PVPARENA_X /* and not in pvp-arena */
		    && p_ptr->wpos.wy == WPOS_PVPARENA_Y && p_ptr->wpos.wz == WPOS_PVPARENA_Z)) {
			if (p_ptr->distinct_floor_feeling || is_admin(p_ptr)) {
				/* There are two different floor flag (combos) generating the 'terrifying' feeling: */
				if ((l_ptr->flags2 & LF2_OOD_HI) ||
				    ((l_ptr->flags2 & LF2_VAULT_HI) && (l_ptr->flags2 & LF2_OOD))) {
					Send_music(Ind, 46, 14, 14); //what a terrifying place
					return;
				}
			}
		}

		//we could just look through audio.lua, by querying get_music_name() I guess..
		switch (i) {
		default:
		case 0:
			if (d_ptr->flags3 & DF3_JAIL_DUNGEON) Send_music(Ind, 178, 14, 14); //take precedence over DF2_IRON for jail-specific dungeons; also, use hellish/forcedown for a change (as replacement) as it's underused..
			else if (d_ptr->flags2 & DF2_NO_DEATH) Send_music(Ind, 12, 11, 11);//note: music file (dungeon_generic_nodeath) is identical to the one of the Training Tower
			else if (d_ptr->flags2 & DF2_IRON) Send_music(Ind, 13, 11, 11);
			else if ((d_ptr->flags2 & DF2_HELL) || (d_ptr->flags1 & DF1_FORCE_DOWN)) Send_music(Ind, 14, 11, 11);
			else Send_music(Ind, 11, 0, 0); //dungeon, generic
			return;
		case 1: Send_music(Ind, 32, 11, 11); return; //Mirkwood
		case 2: Send_music(Ind, 17, 11, 11); return; //Mordor
		case 3: Send_music(Ind, 19, 11, 11); return; //Angband
		case 4: Send_music(Ind, 16, 11, 11); return; //Barrow-Downs
		case 5: Send_music(Ind, 21, 11, 11); return; //Mount Doom
		case 6: Send_music(Ind, 22, 13, 11); return; //Nether Realm
		case 7: Send_music(Ind, 35, 11, 11); return; //Submerged Ruins
		case 8: Send_music(Ind, 26, 11, 11); return; //Halls of Mandos
		case 9: Send_music(Ind, 30, 11, 11); return; //Cirith Ungol
		case 10: Send_music(Ind, 28, 11, 11); return; //The Heart of the Earth
		case 16: Send_music(Ind, 18, 11, 11); return; //The Paths of the Dead
		case 17: Send_music(Ind, 37, 11, 11); return; //The Illusory Castle
		case 18: Send_music(Ind, 39, 11, 11); return; //The Maze
		case 19: Send_music(Ind, 20, 11, 11); return; //The Orc Cave
		case 20: Send_music(Ind, 36, 11, 11); return; //Erebor
		case 21: Send_music(Ind, 27, 11, 11); return; //The Old Forest
		case 22: Send_music(Ind, 29, 11, 11); return; //The Mines of Moria
		case 23: Send_music(Ind, 34, 11, 11); return; //Dol Guldur
		case 24: Send_music(Ind, 31, 11, 11); return; //The Small Water Cave
		case 25: Send_music(Ind, 38, 11, 11); return; //The Sacred Land of Mountains
		case 26: Send_music(Ind, 24, 11, 11); return; //The Land of Rhun
		case 27: Send_music(Ind, 25, 11, 11); return; //The Sandworm Lair
		case 28: if (l_ptr && (l_ptr->flags2 & LF2_NO_RUN)) Send_music(Ind, 33, 11, 11);
			else Send_music(Ind, 14, 33, 11);
			return; //Death Fate
		case 29: Send_music(Ind, 23, 11, 11); return; //The Helcaraxe
		case 30: Send_music(Ind, 15, 12, 11); return; //The Training Tower
		//31 is handled above by in_valinor() check
		case 32:
			if (is_newer_than(&p_ptr->version, 4, 5, 6, 0, 0, 1)) Send_music(Ind, 56, 13, 13); //The Cloud Planes
			else {
				if (p_ptr->audio_mus == 56) Send_music(Ind, 13, 0, 0); /* outdated music pack? (use ironman music for now (forcedown/hellish doesn't fit)) */
				else Send_music(Ind, 56, 13, 13); /* the actual specific music for this dungeon */
			}
			return;
		}
	}

	/* Shouldn't happen - send default (dungeon) music */
	Send_music(Ind, 0, 0, 0);
}
void handle_seasonal_music(void) {
	player_type *p_ptr;
	int i;

	for (i = 1; i <= NumPlayers; i++) {
		p_ptr = Players[i];

		/* Currently all seasonal events take place in Bree */
		if (p_ptr->wpos.wz == 0 &&
		    istownarea(&p_ptr->wpos, MAX_TOWNAREA) &&
		    town[wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx].town_idx].type == TOWN_BREE)
			handle_music(i);
	}
}

void handle_ambient_sfx(int Ind, cave_type *c_ptr, struct worldpos *wpos, bool smooth) {
	player_type *p_ptr = Players[Ind];
	dun_level *l_ptr = getfloor(wpos);

	if (p_ptr->muted_when_idle) return;

	/* sounds that guaranteedly override everthing */
	if (in_valinor(wpos)) {
		Send_sfx_ambient(Ind, SFX_AMBIENT_SHORE, TRUE);
		return;
#ifdef DM_MODULES
	} else if (in_module(wpos)) {
		Send_sfx_ambient(Ind, exec_lua(0, format("return adventure_locale(%d, 5)", wpos->wz)), TRUE);
		return;
#endif
	}

	/* don't play outdoor (or any other) ambient sfx if we're in a special pseudo-indoors sector */
	if ((l_ptr && (l_ptr->flags2 & LF2_INDOORS)) ||
	    (in_sector000(wpos) && (sector000flags2 & LF2_INDOORS))) {
		Send_sfx_ambient(Ind, SFX_AMBIENT_NONE, FALSE);
		return;
	}

	/* disable certain ambient sounds if they shouldn't be up here */
	if (p_ptr->sound_ambient == SFX_AMBIENT_FIREPLACE && !inside_inn(p_ptr, c_ptr)) {
		Send_sfx_ambient(Ind, SFX_AMBIENT_NONE, smooth);
	} else if (p_ptr->sound_ambient == SFX_AMBIENT_SHORE && (wpos->wz != 0 || (wild_info[wpos->wy][wpos->wx].type != WILD_OCEAN && wild_info[wpos->wy][wpos->wx].bled != WILD_OCEAN))) {
		Send_sfx_ambient(Ind, SFX_AMBIENT_NONE, smooth);
	} else if (p_ptr->sound_ambient == SFX_AMBIENT_LAKE && (wpos->wz != 0 ||
	    (wild_info[wpos->wy][wpos->wx].type != WILD_LAKE && wild_info[wpos->wy][wpos->wx].bled != WILD_LAKE &&
	    wild_info[wpos->wy][wpos->wx].type != WILD_RIVER && wild_info[wpos->wy][wpos->wx].bled != WILD_RIVER &&
	    wild_info[wpos->wy][wpos->wx].type != WILD_SWAMP && wild_info[wpos->wy][wpos->wx].bled != WILD_SWAMP))) {
		Send_sfx_ambient(Ind, SFX_AMBIENT_NONE, smooth);
	} else {
		switch (p_ptr->sound_ambient) {
		case SFX_AMBIENT_STORE_GENERAL:
		case SFX_AMBIENT_STORE_ARMOUR:
		case SFX_AMBIENT_STORE_WEAPON:
		case SFX_AMBIENT_STORE_TEMPLE:
		case SFX_AMBIENT_STORE_ALCHEMY:
		case SFX_AMBIENT_STORE_MAGIC:
		case SFX_AMBIENT_STORE_BLACK:
		case SFX_AMBIENT_STORE_BOOK:
		case SFX_AMBIENT_STORE_RUNE:
		case SFX_AMBIENT_STORE_MERCHANTS:
		case SFX_AMBIENT_STORE_OFFICIAL:
		case SFX_AMBIENT_STORE_CASINO:
		case SFX_AMBIENT_STORE_MISC:
			if (p_ptr->store_num == -1) Send_sfx_ambient(Ind, SFX_AMBIENT_NONE, smooth);
		}
	}

	/* enable/switch to certain ambient loops */

	/* Shop-specific persistent (ie looped) ambient sfx */
	switch (p_ptr->store_num) {
	case 0: case 11: case 29: case 49: case 58:
	case 59:
	case 61: case 62: case 63: case 68:
	case 70:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_GENERAL, smooth);
		return;
	case 1:
	case 44: case 45:
	case 71:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_ARMOUR, smooth);
		return;
	case 2:
	case 38: case 39: case 40: case 41:
	case 55: case 64:
	case 72:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_WEAPON, smooth);
		return;
	case 3: case 12: case 19:
	case 23:
	case 26: case 34:
	case 73:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_TEMPLE, smooth);
		return;
	case 4:
	case 51:
	case 67:
	case 74:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_ALCHEMY, smooth);
		return;
	case 5:
	case 18:
	case 25:
	case 42: case 43:
	case 75:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_MAGIC, smooth);
		return;
	case 6: case 48: case 60:
	case 76:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_BLACK, smooth);
		return;
	case 7:
	case 77:
		//Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_, smooth); //Inn
		break; //fall through
	case 8:
	case 13:
	case 28:
	case 46: case 47:
	case 52: case 65:
	case 78:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_BOOK, smooth);
		return;
	case 9:
	case 79:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_RUNE, smooth);
		return;
	case 10:
	case 14:
	case 24:
	case 27:
	case 57:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_OFFICIAL, smooth);
		return;
	case 56:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_MERCHANTS, smooth);
		return;
	case 15:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_CASINO, smooth);
		return;
	case 16: case 17: case 20: case 21: case 22:
	case 30: case 31: case 32: case 33: case 35: case 36: case 37:
	case 50: case 53:
	case 54:
		Send_sfx_ambient(Ind, SFX_AMBIENT_STORE_MISC, smooth);
		return;
	}


#if 0 /* buggy: enable no_house_sfx while inside a house, and the sfx will stay looping even when leaving the house */
	if (p_ptr->sound_ambient != SFX_AMBIENT_FIREPLACE && (f_info[c_ptr->feat].flags1 & FF1_PROTECTED) && istown(wpos) && p_ptr->sfx_house) { /* sfx_house check is redundant with grid_affects_player() */
#elif 0 /* still buggy :-p */
	if (p_ptr->sound_ambient != SFX_AMBIENT_FIREPLACE && (f_info[c_ptr->feat].flags1 & FF1_PROTECTED) && istown(wpos)) {
#else /* dunno */
	if (p_ptr->sound_ambient != SFX_AMBIENT_FIREPLACE && inside_inn(p_ptr, c_ptr)) {
#endif
		Send_sfx_ambient(Ind, SFX_AMBIENT_FIREPLACE, smooth);
	} else if (p_ptr->sound_ambient != SFX_AMBIENT_FIREPLACE &&
	    p_ptr->sound_ambient != SFX_AMBIENT_SHORE && wpos->wz == 0 && (wild_info[wpos->wy][wpos->wx].type == WILD_OCEAN || wild_info[wpos->wy][wpos->wx].bled == WILD_OCEAN)) {
		Send_sfx_ambient(Ind, SFX_AMBIENT_SHORE, smooth);
	} else if (p_ptr->sound_ambient != SFX_AMBIENT_FIREPLACE && p_ptr->sound_ambient != SFX_AMBIENT_SHORE &&
	    p_ptr->sound_ambient != SFX_AMBIENT_LAKE && wpos->wz == 0 &&
	    (wild_info[wpos->wy][wpos->wx].type == WILD_LAKE || wild_info[wpos->wy][wpos->wx].bled == WILD_LAKE ||
	    wild_info[wpos->wy][wpos->wx].type == WILD_RIVER || wild_info[wpos->wy][wpos->wx].bled == WILD_RIVER ||
	    wild_info[wpos->wy][wpos->wx].type == WILD_SWAMP || wild_info[wpos->wy][wpos->wx].bled == WILD_SWAMP)) {
		if (!cold_place(wpos)) Send_sfx_ambient(Ind, SFX_AMBIENT_LAKE, smooth);
		else Send_sfx_ambient(Ind, SFX_AMBIENT_NONE, smooth);
	}
}

/* play single ambient sfx, synched for all players, depending on worldmap terrain - C. Blue */
void process_ambient_sfx(void) {
	int i, vol, typ = -1;
	player_type *p_ptr;
	wilderness_type *w_ptr;

	for (i = 1; i <= NumPlayers; i++) {
		p_ptr = Players[i];
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Only on world surface atm */
		if (p_ptr->wpos.wz) continue;
		/* for Dungeon Keeper event, considered indoors */
		//if (l_ptr && (l_ptr->flags2 & LF2_INDOORS)) continue;
		if (in_sector000(&p_ptr->wpos) && (sector000flags2 & LF2_INDOORS)) continue;

		w_ptr = &wild_info[p_ptr->wpos.wy][p_ptr->wpos.wx];
		if (w_ptr->ambient_sfx_counteddown) continue;
		if (w_ptr->ambient_sfx) {
			p_ptr->ambient_sfx_timer = 0;
			continue;
		}
		if (w_ptr->ambient_sfx_timer) {
			if (!w_ptr->ambient_sfx_counteddown) {
				w_ptr->ambient_sfx_timer--;
				w_ptr->ambient_sfx_counteddown = TRUE; //semaphore
			}
			continue;
		}

		vol = 25 + rand_int(75);
		//if (!rand_int(6)) vol = 100;

#ifdef TEST_SERVER
		/* hack for towns: allow some ambient sfx here too! */
		if (w_ptr->type == WILD_TOWN)
			switch (town[w_ptr->town_idx].type) {
			case TOWN_BREE: typ = WILD_FOREST; break; //WILD_GRASSLAND
			case TOWN_GONDOLIN: typ = WILD_GRASSLAND; break;
			case TOWN_MINAS_ANOR: typ = WILD_MOUNTAIN; break;
			case TOWN_LOTHLORIEN: typ = WILD_FOREST; break; //WILD_OCEAN
			case TOWN_KHAZADDUM: typ = WILD_MOUNTAIN; break; //WILD_VOLCANO
			default:
			case TOWN_VANILLA: typ = -1; break;
			//TODO: actually keep terrain type of a wilderness town, instead of just overwriting w_ptr->type with WILD_TOWN;
			//having w_ptr->town_idx should be enough to determine that a sector is a town.
			}
		else
#endif
			typ = w_ptr->type;

		switch (typ) { /* ---- ensure consistency with alloc_dungeon_level() ---- */
		case WILD_RIVER:
		case WILD_LAKE:
		case WILD_SWAMP:
			if (cold_place(&p_ptr->wpos)) break;
			sound_floor_vol(&p_ptr->wpos, "animal_toad", NULL, SFX_TYPE_AMBIENT, vol);
			w_ptr->ambient_sfx_timer = 4 + rand_int(4);
			break;
		case WILD_ICE:
		case WILD_MOUNTAIN:
		case WILD_WASTELAND:
			if (IS_NIGHT) {
				sound_floor_vol(&p_ptr->wpos, "animal_wolf", NULL, SFX_TYPE_AMBIENT, vol);
				w_ptr->ambient_sfx_timer = 30 + rand_int(60);
			} else {
				sound_floor_vol(&p_ptr->wpos, "animal_birdofprey", NULL, SFX_TYPE_AMBIENT, vol);
				w_ptr->ambient_sfx_timer = 30 + rand_int(60);
			}
			break;
		case WILD_VOLCANO:
		case WILD_DESERT:
			if (IS_DAY) sound_floor_vol(&p_ptr->wpos, "animal_birdofprey", NULL, SFX_TYPE_AMBIENT, vol);
			w_ptr->ambient_sfx_timer = 120 + rand_int(240); //very rarely
			break;
#if 0 /*  --used for both, oceans and lakes -- is it tho? on ~0 worldmap it doesn't look like it // todo: just check (wild_info[wpos->wy][wpos->wx].type == WILD_OCEAN || wild_info[wpos->wy][wpos->wx].bled == WILD_OCEAN) */
		case WILD_SHORE1:
		case WILD_SHORE2:
		case WILD_COAST:
			if (IS_DAY) sound_floor_vol(&p_ptr->wpos, "animal_seagull", NULL, SFX_TYPE_AMBIENT, vol);
			w_ptr->ambient_sfx_timer = 120 + rand_int(240); //very rarely
			break;
#endif
#if 0 /* verify whether cool (also see above, shore/coast, same for this?) */
		case WILD_OCEANBED1:
		case WILD_OCEANBED2:
#endif
		case WILD_OCEAN:
			if (IS_DAY) sound_floor_vol(&p_ptr->wpos, "animal_seagull", NULL, SFX_TYPE_AMBIENT, vol);
			w_ptr->ambient_sfx_timer = 30 + rand_int(60);
			break;
		case WILD_GRASSLAND:
			if (IS_DAY) {
				if (rand_int(4)) {
					if (!cold_place(&p_ptr->wpos)) sound_floor_vol(&p_ptr->wpos, "animal_bird", NULL, SFX_TYPE_AMBIENT, vol);
				} else sound_floor_vol(&p_ptr->wpos, "animal_birdofprey", NULL, SFX_TYPE_AMBIENT, vol);
				w_ptr->ambient_sfx_timer = 120 + rand_int(240); //very rarely
			}
			break;
		case WILD_FOREST:
		case WILD_DENSEFOREST:
			if (IS_DAY) {
				if (cold_place(&p_ptr->wpos)) {
					sound_floor_vol(&p_ptr->wpos, "animal_wolf", NULL, SFX_TYPE_AMBIENT, vol);
					w_ptr->ambient_sfx_timer = 30 + rand_int(60);
				} else {
					sound_floor_vol(&p_ptr->wpos, "animal_bird", NULL, SFX_TYPE_AMBIENT, vol);
					w_ptr->ambient_sfx_timer = 10 + rand_int(20);
				}
			} else {
				if (!rand_int(3)) {
					sound_floor_vol(&p_ptr->wpos, "animal_wolf", NULL, SFX_TYPE_AMBIENT, vol);
					w_ptr->ambient_sfx_timer = 30 + rand_int(60);
				} else {
					sound_floor_vol(&p_ptr->wpos, "animal_owl", NULL, SFX_TYPE_AMBIENT, vol);
					w_ptr->ambient_sfx_timer = 20 + rand_int(40);
				}
			}
			break;
		/* Note: Default terrain that doesn't have ambient sfx will automatically clear people's ambient_sfx_timer too.
		   This is ok for towns but not cool for fast wilderness travel where terrain is mixed a lot.
		   For that reason we give default terrain a "pseudo-timeout" to compromise a bit. */
		default:
			w_ptr->ambient_sfx_timer = 30 + rand_int(5); //should be > than time required for travelling across 1 wilderness sector
			break;
		}

		w_ptr->ambient_sfx = TRUE;
		p_ptr->ambient_sfx_timer = 0;
	}

	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (Players[i]->wpos.wz) continue;
		wild_info[Players[i]->wpos.wy][Players[i]->wpos.wx].ambient_sfx = FALSE;
		wild_info[Players[i]->wpos.wy][Players[i]->wpos.wx].ambient_sfx_counteddown = FALSE;
	}
}

/* generate an item-type specific sound, depending on the action applied to it
   action: 0 = pickup, 1 = drop, 2 = wear/wield, 3 = takeoff, 4 = throw, 5 = destroy */
void sound_item(int Ind, int tval, int sval, cptr action) {
	char sound_name[30];
	cptr item = NULL;

	/* action hack: re-use sounds! */
	action = "item_";

	/* choose sound */
	if (is_melee_weapon(tval)) switch (tval) {
		case TV_SWORD: item = "sword"; break;
		case TV_BLUNT:
			if (sval == SV_WHIP) item = "whip"; else item = "blunt";
			break;
		case TV_AXE: item = "axe"; break;
		case TV_POLEARM: item = "polearm"; break;
	} else if (is_armour(tval)) {
		if (is_textile_armour(tval, sval))
			item = "armour_light";
		else
			item = "armour_heavy";
	} else switch (tval) {
		/* equippable stuff */
		case TV_LITE: item = "lightsource"; break;
		case TV_RING: item = "ring"; break;
		case TV_AMULET: item = "amulet"; break;
		case TV_TOOL: item = "tool"; break;
		case TV_DIGGING: item = "tool_digger"; break;
		case TV_MSTAFF: item = "magestaff"; break;
		case TV_BOOMERANG: item = "boomerang"; break;
		case TV_BOW: item = "bow"; break;
		case TV_SHOT: item = "shot"; break;
		case TV_ARROW: item = "arrow"; break;
		case TV_BOLT: item = "bolt"; break;
		/* other items */
		case TV_BOOK: item = "book"; break;
		case TV_SCROLL: case TV_PARCHMENT:
			item = "scroll"; break;
		case TV_BOTTLE: item = "potion"; break;
		case TV_POTION: case TV_POTION2: case TV_FLASK:
			item = "potion"; break;
		case TV_RUNE: item = "rune"; break;
		case TV_SKELETON: item = "skeleton"; break;
		case TV_FIRESTONE: item = "firestone"; break;
		case TV_SPIKE: item = "spike"; break;
		case TV_CHEST: item = "chest"; break;
		case TV_JUNK:
			if (sval >= SV_GIFT_WRAPPING_START && sval <= SV_GIFT_WRAPPING_END) item = "scroll";
			else item = "junk";
			break;
		case TV_GAME:
			if (sval == SV_GAME_BALL) {
				item = "ball_pass"; break; }
			else if (sval <= SV_BLACK_PIECE) {
				item = "go_stone"; break; }
			item = "game_piece"; break;
		case TV_TRAPKIT: item = "trapkit"; break;
		case TV_STAFF: item = "staff"; break;
		case TV_WAND: item = "wand"; break;
		case TV_ROD: item = "rod"; break;
		case TV_ROD_MAIN: item = "rod"; break;
		case TV_FOOD: item = "food"; break;
		case TV_KEY: item = "key"; break;
		case TV_GOLEM:
			if (sval == SV_GOLEM_WOOD) {
				item = "golem_wood"; break; }
			else if (sval <= SV_GOLEM_ADAM) {
				item = "golem_metal"; break; }
			else if (sval >= SV_GOLEM_ATTACK) {
				item = "scroll"; break; }
			item = "golem_misc"; break;
#ifdef ENABLE_DEMOLITIONIST
		case TV_CHEMICAL: item = "powder"; break;
		case TV_CHARGE: item = "rune"; break;
#endif
		case TV_SPECIAL:
			switch (sval) {
			case SV_SEAL: item = "seal"; break;
			case SV_CUSTOM_OBJECT:
				//todo maybe:
				//if (o_ptr->xtra3 & 0x0200) sound_item(Ind, o_ptr->tval2, o_ptr->sval2, action);
				break;
			}
			if (sval >= SV_GIFT_WRAPPING_START && sval <= SV_GIFT_WRAPPING_END) item = "scroll";
			break;
#ifdef ENABLE_SUBINVEN
		case TV_SUBINVEN:
			switch (sval) {
			case SV_SI_SATCHEL:
			case SV_SI_TRAPKIT_BAG:
			case SV_SI_MDEVP_WRAPPING:
			case SV_SI_POTION_BELT:
			case SV_SI_FOOD_BAG:
				item = "armour_light"; //a textile bag, basically
				break;
			case SV_SI_CHEST_SMALL_WOODEN:
			case SV_SI_CHEST_SMALL_IRON:
			case SV_SI_CHEST_SMALL_STEEL:
			case SV_SI_CHEST_LARGE_WOODEN:
			case SV_SI_CHEST_LARGE_IRON:
			case SV_SI_CHEST_LARGE_STEEL:
				item = "chest";
				break;
			}
			break;
#endif
	}

	/* no sound effect available? */
	if (item == NULL) return;

	/* build sound name from action and item */
	strcpy(sound_name, action);
	strcat(sound_name, item);
	/* off we go */
	sound(Ind, sound_name, NULL, SFX_TYPE_COMMAND, FALSE);
}
#endif

/*
 * We use a global array for all inscriptions to reduce the memory
 * spent maintaining inscriptions.  Of course, it is still possible
 * to run out of inscription memory, especially if too many different
 * inscriptions are used, but hopefully this will be rare.
 *
 * We use dynamic string allocation because otherwise it is necessary
 * to pre-guess the amount of quark activity.  We limit the total
 * number of quarks, but this is much easier to "expand" as needed.
 *
 * Any two items with the same inscription will have the same "quark"
 * index, which should greatly reduce the need for inscription space.
 *
 * Note that "quark zero" is NULL and should not be "dereferenced".
 */

/*
 * Add a new "quark" to the set of quarks.
 */
s32b quark_add(cptr raw_str) {
	s32b i;
	char str[INSCR_LEN];

	strncpy(str, raw_str, INSCR_LEN);
	str[INSCR_LEN - 1] = 0;

	/* Look for an existing quark */
	for (i = 1; i < quark__num; i++) {
		/* Check for equality */
		if (streq(quark__str[i], str)) return(i);
	}

	/* Paranoia -- Require room */
	if (quark__num == QUARK_MAX) return(0);

	/* New maximal quark */
	quark__num = i + 1;

	/* Add a new quark */
	quark__str[i] = string_make(str);

	/* Return the index */
	return(i);
}


/*
 * This function looks up a quark
 */
cptr quark_str(s32b i) {
	cptr q;

	/* Verify */
	if ((i < 0) || (i >= quark__num)) i = 0;

	/* Access the quark */
	q = quark__str[i];

	/* Return the quark */
	return(q);
}

/* Check to make sure they haven't inscribed an item against what
 * they are trying to do -Crimson
 * look for "!*Erm" type, and "!* !A !f" type.
 * New (2023): Encode TRUE directly as -1 instead, and if TRUE and there's a number behind
 *             the inscription still within this same !-'segment', return that number + 1 (to encode a value of 0 too),
 *             just negative values aren't possible as -1 would interfere with 'FALSE'. -C. Blue
 *             Added this for !M and !G handling.
 * Returns <-1> if TRUE, <0> if FALSE, >0 if TRUE and a number is specified, with return value being <number+1>. */
int check_guard_inscription(s16b quark, char what) {
	const char *ax = quark_str(quark);
	int n = 0; //paranoia initialization

	if (ax == NULL) return(FALSE);

	while ((ax = strchr(ax, '!')) != NULL) {
		while (++ax) {
			if (*ax == 0) return(FALSE); /* end of quark, exit */
			if (*ax == ' ' || *ax == '@' || *ax == '#' || *ax == '-') break; /* end of segment, stop */
			if (*ax == what) { /* exact match, accept */
				/* Additionally scan for any 'amount' in case this inscription uses one */
				while (*(++ax)) {
					if (*ax == ' ' || *ax == '@' || *ax == '#' || *ax == '-') return(-1); /* end of segment, accepted, so exit */
					/* Check for number (Note: Evaluate atoi first, in case it's a number != 0 but with leading '0'. -0 and +0 will also be caught fine as simply 0.) */
					if ((n = atoi(ax)) || *ax == '0') return(n + 1); /* '+1' hack: Allow specifying '0' too, still distinguishing it from pure inscription w/o a number specified. */
				}
				return(-1); /* end of quark, exit */
			}
			/* '!*' special combo inscription */
			if (*ax == '*') {
				/* why so much hassle? * = all, that's it */
				/*return(TRUE); -- well, !'B'ash if it's on the ground sucks ;) */

				switch (what) { /* check for paranoid tags */
				case 'd': /* no drop */
				case 'h': /* (obsolete) no house ( sell a a key ) */
				case 'k': /* no destroy */
				case 's': /* no sell */
				case 'v': /* no thowing */
				case '=': /* force pickup */
#if 0
				case 'w': /* no wear/wield */
				case 't': /* no take off */
#endif
					return(-1);
				}
				//return(FALSE);
			}
			/* '!+' special combo inscription */
			if (*ax == '+') {
				/* why so much hassle? * = all, that's it */
				/*return(TRUE); -- well, !'B'ash if it's on the ground sucks ;) */

				switch (what) { /* check for paranoid tags */
				case 'h': /* (obsolete) no house ( sell a a key ) */
				case 'k': /* no destroy */
				case 's': /* no sell */
				case '=': /* force pickup */
#if 0
				case 'w': /* no wear/wield */
				case 't': /* no take off */
#endif
					return(-1);
				}
				//return(FALSE);
			}
		}
	}
	return(FALSE);
}


/*
 * Output a message to the top line of the screen.
 *
 * Break long messages into multiple pieces (40-72 chars).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are memorized for later reference (see above).
 *
 * We could do "Term_fresh()" to provide "flicker" if needed.
 *
 * XXX XXX XXX Note that we must be very careful about using the
 * "msg_print()" functions without explicitly calling the special
 * "msg_print(NULL)" function, since this may result in the loss
 * of information if the screen is cleared, or if anything is
 * displayed on the top line.
 *
 * XXX XXX XXX Note that "msg_print(NULL)" will clear the top line
 * even if no messages are pending.  This is probably a hack.
 */
bool suppress_message = FALSE, suppress_boni = FALSE;

void msg_print(int Ind, cptr msg_raw) {
	player_type *p_ptr = Players[Ind];
	char msg_dup[MSG_LEN], *msg = msg_dup;
#if defined(KIND_DIZ) && defined(SERVER_ITEM_PASTE_DIZ)
	char *ckt;
#endif
	int line_len = 80; /* maximum length of a text line to be displayed;
		     this is client-dependant, compare c_msg_print (c-util.c) */
	char msg_buf[line_len + 2 + 2 * 80]; /* buffer for 1 line. + 2 bytes for colour code (+2*80 bytes for colour codeeeezz) */
	char msg_minibuf[3]; /* temp buffer for adding characters */
	int text_len, msg_scan = 0, space_scan, tab_spacer = 0, tmp;
	char colour_code = 'w', prev_colour_code = 'w';
	bool no_colour_code = FALSE;
	bool first_character = TRUE;
	//bool is_chat = ((msg_raw != NULL) && (strlen(msg_raw) > 2) && (msg_raw[2] == '['));
	bool client_ctrlo = FALSE, client_chat = FALSE, client_all = FALSE;
	//for strings that get split up and sent as multiple lines: mark subsequent lines for the client, for topline handling
	bool first_line = TRUE, first_line_notok = is_older_than(&p_ptr->version, 4, 9, 2, 0, 0, 1);

	/* for {- feature */
	char first_colour_code = 'w';
	bool first_colour_code_set = FALSE;

	/* backward msg window width hack for windows clients (non-x11 clients rather) */
	if (!is_newer_than(&p_ptr->version, 4, 4, 5, 3, 0, 0) && !strcmp(p_ptr->realname, "PLAYER")) line_len = 72;

	/* Pfft, sorry to bother you.... --JIR-- */
	if (suppress_message) return;

	/* no message? */
	if (msg_raw == NULL) {
		/* Hack to clear the topline; now also hack to refresh message display (4.7.3) */
		Send_message(Ind, NULL);
		return;
	}

	strcpy(msg_dup, msg_raw); /* in case msg_raw was constant */

	/* Marker from client to server: lore-paste (arts/monsters), or kind-diz would need to begin here, if player wishes to read this kind of extra info. */
	if ((ckt = strchr(msg_dup, '\372'))) {
		/* Look for the first space, it'll be after the player name */
		char *csp = strchr(msg_dup, ' '), *ism;

		/* Look for inter-server message, denoted by '[N]' before first space,
		   in which case the first space isn't after the player name but after the server index!
		   NOTE: This assumes that player names are not allowed to start on a number! */
		if (csp && (ism = strchr(msg_dup, '[')) && ism < csp && *++ism >= '0' && *ism <= '9') {
			ism = strchr(csp, ']');
			csp = strchr(ism + 1, ' ');
		} else if (csp && (ism = strchr(msg_dup, ']'))) csp = strchr(ism + 1, ' ');

		/* Starts at the very beginning of the chat line? Then it is lore-paste (artifact/monster) */
		if (csp && ckt == csp + 1) {
			/* hide_lore_paste specifically is for global chat, not for private/party/guild chat.
			   This is achieved automatically, as the first space in the line would be longer before the \372 marker in that case. */

			/* The client replaced the \377 colour code marker by \372 marker to notice us there's a lore-paste.
			   We revert it to a usable colour code, or discard the whole message if the receiving player doesn't want to see lore-pastes. */
			if (p_ptr->hide_lore_paste) return; /* Discard whole message */
			else *ckt = '\377'; /* Restore colour code, display the now normal message */
		}

#if defined(KIND_DIZ) && defined(SERVER_ITEM_PASTE_DIZ)
		/* Starts later in the line? Then it's a kind-description for an item-paste */
		else if (p_ptr->add_kind_diz) *ckt = '\377';
		else *ckt = 0;
#endif
	}

	/* marker for client: add message to 'chat-only buffer', not to 'nochat buffer' */
	if (msg[0] == '\375') {
		client_chat = TRUE;
		msg++;
		/* hack: imply that chat-only messages are also always important messages */
		client_ctrlo = TRUE;
	}
	/* marker for client: add message to 'nochat buffer' AND to 'chat-only buffer' */
	else if (msg[0] == '\374') {
		client_all = TRUE;
		msg++;
		/* hack: imply that messages that go to chat-buffer are also always important messages */
		client_ctrlo = TRUE;
	}
	/* ADDITIONAL marker for client: add message to CTRL+O 'important scrollback buffer' */
	if (msg[0] == '\376') {
		client_ctrlo = TRUE;
		msg++;
	}
	/* note: neither \375 nor \374 means:
	   add message to 'nochat buffer', but not to 'chat-only buffer' (default) */

	/* neutralize markers if client version too old */
	if (!is_newer_than(&p_ptr->version, 4, 4, 2, 0, 0, 0))
		client_ctrlo = client_chat = client_all = FALSE;

#if 1	/* String longer than 1 line? -> Split it up! --C. Blue-- */
	while (msg != NULL && msg[msg_scan] != '\0') {
		/* Start a new line */
		strcpy(msg_buf, "");
		text_len = 0;

		/* Tabbing the line? */
		msg_minibuf[0] = ' ';
		msg_minibuf[1] = '\0';
 #if 0 /* 0'ed to remove backward compatibility via '~' character. We want to switch to \374..6 codes exlusively */
		if (is_chat && tab_spacer) {
			/* Start the padding for chat messages with '~' */
			strcat(msg_buf, "~");
			tmp = tab_spacer - 1;
		} else {
			tmp = tab_spacer;
		}
 #else
		tmp = tab_spacer;
 #endif
		while (tmp--) {
			text_len++;
			strcat(msg_buf, msg_minibuf);
		}

		/* Prefixing colour code? */
		if (colour_code) {
			msg_minibuf[0] = '\377';
			msg_minibuf[1] = colour_code;
			msg_minibuf[2] = '\0';
			strcat(msg_buf, msg_minibuf);
			///colour_code = 0;
		}

		/* Process the string... */
		while (text_len < line_len) {
			switch (msg[msg_scan]) {
			case '\0': /* String ends! */
				text_len = line_len;
				continue;
			case '\377': /* Colour code! Text length does not increase. */
				if (!no_colour_code) {
					/* broken \377 at the end of the text? ignore */
					if (color_char_to_attr(msg[msg_scan + 1]) == -1
					    && msg[msg_scan + 1] != '-' /* {-, {. and {{ are handled (further) below */
					    && msg[msg_scan + 1] != '.'
					    && msg[msg_scan + 1] != '\377') {
						msg_scan++;
						continue;
					}

					/* Capture double \377 which stand for a normal { char instead of a colour code: */
					if (msg[msg_scan + 1] != '\377') {
						msg_minibuf[0] = msg[msg_scan];
						msg_scan++;

						/* needed for rune sigil on items, pasted to chat */
						if (msg[msg_scan] == '.') {
							msg[msg_scan] = colour_code = prev_colour_code;
						/* needed for new '\377-' feature in multi-line messages: resolve it to actual colour */
						} else if (msg[msg_scan] == '-') {
							msg[msg_scan] = colour_code = first_colour_code;
						} else {
							prev_colour_code = colour_code;
							colour_code = msg[msg_scan];
							if (!first_colour_code_set) {
								first_colour_code_set = TRUE;
								first_colour_code = colour_code;
							}
						}
						msg_scan++;

						msg_minibuf[1] = colour_code;
						msg_minibuf[2] = '\0';
						strcat(msg_buf, msg_minibuf);
						break;
					}
					no_colour_code = TRUE;
				} else no_colour_code = FALSE;
				/* fall through if it's a '{' character */
				__attribute__ ((fallthrough));
			default: /* Text length increases by another character.. */
				/* Depending on message type, remember to tab the following
				   lines accordingly to make it look better ^^
				   depending on the first character of this line. */
				if (first_character) {
					switch (msg[msg_scan]) {
					case '*': tab_spacer = 2; break; /* Kill message */
					case '[': /* Chat message */
 #if 0
						tab_spacer = 1;
 #else
						{
							const char *bracket = strchr(&msg[msg_scan], ']');

							if (bracket) {
								const char *ptr = &msg[msg_scan];

								/* Pad lines according to how long the name is - mikaelh */
								tab_spacer = bracket - &msg[msg_scan] + 2;

								/* Ignore space reserved for colour codes:
								   Guild chat has coloured [ ] brackets. - C. Blue */
								while ((ptr = strchr(ptr, '\377')) && ptr < bracket) {
									ptr++;
									tab_spacer -= 2; /* colour code consists of two chars
									    (we can guarantee that here, since the server
									    generated this colour code) */
								}

								/* Hack: multiline /me emotes */
								if (tab_spacer > 70) tab_spacer = 1;

								/* Paranoia */
								if (tab_spacer < 1) tab_spacer = 1;
								if (tab_spacer > 30) tab_spacer = 30;
							} else {
								/* No ']' */
								tab_spacer = 1;
							}
						}
 #endif
						break;
					default: tab_spacer = 1;
					}
				}

				/* remember first actual chat colour (guild chat changes
				   colour a few times for [..] brackets and name).
				   This is for {- etc feature.
				   However, if there is no new colour specified before
				   beginning of chat text then use the one we had, or {-
				   wouldn't work correctly in private chat anymore. - C. Blue */
				if (msg[msg_scan] == ']') {
 #if 0  /* this is wrong, because the colour code COULD already be from \\a feature! */
				    ((msg[msg_scan + 1] == ' ' && msg[msg_scan + 2] == '\377') ||
 #endif
					if (msg[msg_scan + 1] == '\377') first_colour_code_set = FALSE;
				}

				/* Process text.. */
				first_character = FALSE;
				msg_minibuf[0] = msg[msg_scan];
				msg_minibuf[1] = '\0';
				strcat(msg_buf, msg_minibuf);
				msg_scan++;
				text_len++;

				/* Avoid cutting words in two */
				if ((text_len == line_len) && (msg[msg_scan] != '\0')
				    && (

				    (msg[msg_scan - 1] >= 'A' && msg[msg_scan - 1] <= 'Z') ||
				    (msg[msg_scan - 1] >= 'a' && msg[msg_scan - 1] <= 'z') ||
				    (msg[msg_scan - 1] >= '0' && msg[msg_scan - 1] <= '9') ||
				    msg[msg_scan - 1] == '_' || /* for pasting lore-flags to chat! */
				    msg[msg_scan - 1] == '(' ||
				    msg[msg_scan - 1] == '[' ||
				    msg[msg_scan - 1] == '{' ||
 #if 1 /* don't break smileys? */
				    msg[msg_scan - 1] == ':' || msg[msg_scan - 1] == ';' ||
				    msg[msg_scan - 1] == '^' || msg[msg_scan - 1] == '-' ||
 #endif
 #if 1 /* don't break quotes right at the start */
				    msg[msg_scan - 1] == '"' || msg[msg_scan - 1] == '\'' ||
 #endif
 #if 0
				    msg[msg_scan - 1] == ')' ||
				    msg[msg_scan - 1] == ']' ||
				    msg[msg_scan - 1] == '}' ||
 #endif
				    /* (maybe too much) for pasting items to chat, (+1) or (-2,0) : */
				    msg[msg_scan - 1] == '+' || msg[msg_scan - 1] == '-' ||
				    /* writing a slash command to chat */
				    msg[msg_scan - 1] == '/' ||
				    /* Don't break colour codes */
				    msg[msg_scan - 1] == '\377'

				    ) && (

				    (msg[msg_scan] >= 'A' && msg[msg_scan] <= 'Z') ||
				    (msg[msg_scan] >= 'a' && msg[msg_scan] <= 'z') ||
				    (msg[msg_scan] >= '0' && msg[msg_scan] <= '9') ||
 #if 1 /* specialty: don't break quotes */
				    //(msg[msg_scan] != ' ' && msg[msg_scan - 1] == '"') ||
				    msg[msg_scan - 1] == '"' || msg[msg_scan - 1] == '\'' ||
 #endif
 #if 1 /* don't break moar smileys? */
				    msg[msg_scan] == '(' ||
				    msg[msg_scan] == '[' ||
				    msg[msg_scan] == '{' ||
 #endif
 #if 1 /* don't break smileys? */
				    msg[msg_scan] == '-' ||
				    msg[msg_scan] == '^' ||
 #endif
				    msg[msg_scan] == '_' ||/* for pasting lore-flags to chat! */
				    msg[msg_scan] == ')' ||
				    msg[msg_scan] == ']' ||
				    msg[msg_scan] == '}' ||
				    /* (maybe too much) for pasting items to chat, (+1) or (-2,0) : */
				    msg[msg_scan] == '+' || msg[msg_scan] == '-' ||
				    /* interpunction at the end of a sentence */
				    msg[msg_scan] == '.' || msg[msg_scan] == ',' ||
				    msg[msg_scan] == ';' || msg[msg_scan] == ':' ||
				    msg[msg_scan] == '!' || msg[msg_scan] == '?' ||
				    /* Don't break colour codes */
				    msg[msg_scan] == '\377'

				    )) {
					space_scan = msg_scan;
					do {
						space_scan--;
					} while ((
						/* Optional != '/' checks: Allow "use torch/lamp" -> to linebreak before "/lamp" */
						(msg[space_scan - 1] >= 'A' && msg[space_scan - 1] <= 'Z') || // && msg[space_scan] != '/') ||
						(msg[space_scan - 1] >= 'a' && msg[space_scan - 1] <= 'z') || // && msg[space_scan] != '/') ||
						(msg[space_scan - 1] >= '0' && msg[space_scan - 1] <= '9') || // && msg[space_scan] != '/') ||
 #if 1 /* don't break smileys? */
						msg[space_scan - 1] == ':' || msg[space_scan - 1] == ';' ||
 #endif
 #if 0
						msg[space_scan - 1] == ')' ||
						msg[space_scan - 1] == ']' ||
						msg[space_scan - 1] == '}' ||
 #endif
 #if 0
						msg[space_scan - 1] == '_' ||/* for pasting lore-flags to chat! */
 #endif
						msg[space_scan - 1] == '(' ||
						msg[space_scan - 1] == '[' ||
						msg[space_scan - 1] == '{' ||
						/* (maybe too much) for pasting items to chat, (+1) or (-2,0) : */
						msg[space_scan - 1] == '+' || msg[space_scan - 1] == '-' ||
						/* pasting flags to chat ("SLAY_EVIL") */
						msg[space_scan - 1] == '_' ||
						/* writing a slash command to chat */
						msg[space_scan - 1] == '/' ||
						/* Don't break colour codes */
						msg[space_scan - 1] == '\377'
						) && space_scan > 0);

					/* Simply cut words that are very long - mikaelh */
					if (msg_scan - space_scan > 36) {
						space_scan = msg_scan;
					}

					if (space_scan) {
						msg_buf[strlen(msg_buf) - msg_scan + space_scan] = '\0';
						msg_scan = space_scan;
					}
				}
			}
		}
		Send_message(Ind, format("%s%s%s%s",
		    first_line ? "" : "\377\377",
		    client_chat ? "\375" : (client_all ? "\374" : ""),
		    client_ctrlo ? "\376" : "",
		    msg_buf));
		first_line = first_line_notok;
		/* hack: avoid trailing space in the next sub-line */
		//if (msg[msg_scan] == ' ') msg_scan++;
		while (msg[msg_scan] == ' ') msg_scan++;//avoid all trailing spaces in the next sub-line
	}

	if (msg == NULL) Send_message(Ind, NULL);

	return;
#endif // enable line breaks?

	Send_message(Ind, format("%s%s%s",
	    client_chat ? "\375" : (client_all ? "\374" : ""),
	    client_ctrlo ? "\376" : "",
	    msg));
}

/* Skip 'Ind', can be 0.
   NOTE: This particular function contains an 8ball-hack! */
void msg_broadcast(int Ind, cptr msg) {
	int i;
#ifdef TOMENET_WORLDS
	const char *c;
#endif

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;

		/* Skip the specified player */
		if (i == Ind) continue;

		/* Tell this one */
		msg_print(i, msg);
	 }

#ifdef TOMENET_WORLDS
	/* Hack to read 8ball responses on other servers too. 8ball is recognized by "<colourcode>[8ball]" message start. */
	c = strstr(msg, "[8ball]"); /* Can be \252\255c[8ball] (It) or \255c[8ball] (Moltor) */
	if (cfg.worldd_broadcast && (c == msg + 2 || c == msg + 3) && (msg[0] == '\377' || msg[1] == '\377'))
		world_chat(0, msg);
#endif
}
/* Same as msg_broadcast() but takes both a censored and an uncensored message and chooses per recipient. */
void msg_broadcast2(int Ind, cptr msg, cptr msg_u) {
	int i;

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED) continue;

		/* Skip the specified player */
		if (i == Ind) continue;

		/* Tell this one */
		msg_print(i, Players[i]->censor_swearing ? msg : msg_u);
	 }
}

/* Skip 'Ind', can be 0. */
void msg_admins(int Ind, cptr msg) {
	int i;

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED)  continue;

		/* Skip the specified player */
		if (i == Ind) continue;

		/* Skip non-admins */
		if (!is_admin(Players[i])) continue;

		/* Tell this one */
		msg_print(i, msg);
	 }
}

void msg_broadcast_format(int Ind, cptr fmt, ...) {
	//int i;
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	msg_broadcast(Ind, buf);
}

/* Send a formatted message only to admin chars.	-Jir- */
void msg_admin(cptr fmt, ...)
//void msg_admin(int Ind, cptr fmt, ...)
{
	int i;
	player_type *p_ptr;
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Tell every admin */
	for (i = 1; i <= NumPlayers; i++) {
		p_ptr = Players[i];

		/* Skip disconnected players */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Tell Mama */
		if (is_admin(p_ptr))
			msg_print(i, buf);
	 }
}



/*
 * Display a formatted message, using "vstrnfmt()" and "msg_print()".
 */
void msg_format(int Ind, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print(Ind, buf);
}

/*
 * Send a message to everyone on a floor, optionally except to Ind [or 0]
 */
static void floor_msg(int Ind, struct worldpos *wpos, cptr msg) {
	int i;

//system-msg, currently unused anyway-	if (cfg.log_u) s_printf("[%s] %s\n", Players[sender]->name, msg);
	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (i == Ind) continue;
		/* Check this guy */
		if (inarea(wpos, &Players[i]->wpos)) msg_print(i, msg);
	}
}
/*
 * Send a formatted message to everyone on a floor, optionally except to Ind [or 0]
 */
void floor_msg_format(int Ind, struct worldpos *wpos, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);
	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);
	/* End the Varargs Stuff */
	va_end(vp);
	/* Display */
	floor_msg(Ind, wpos, buf);
}
/*
 * Send a message from sender to everyone on a floor including himself, considering ignorance.
 */
#if 0 /* currently unused, just killing compiler warning.. */
static void floor_msg_ignoring(int sender, struct worldpos *wpos, cptr msg) {
	int i;

	if (cfg.log_u) s_printf("(%d,%d,%d)%s\n", wpos->wx, wpos->wy, wpos->wz, msg + 2);// Players[sender]->name, msg);
	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, sender)) continue;
		/* Check this guy */
		if (inarea(wpos, &Players[i]->wpos)) msg_print(i, msg);
	}
}
#endif
static void floor_msg_ignoring2(int sender, struct worldpos *wpos, cptr msg, cptr msg_u) {
	int i;

	if (cfg.log_u) s_printf("(%d,%d,%d)%s\n", wpos->wx, wpos->wy, wpos->wz, msg + 2);// Players[sender]->name, msg);
	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, sender)) continue;
		/* Check this guy */
		if (inarea(wpos, &Players[i]->wpos)) msg_print(i, Players[i]->censor_swearing ? msg : msg_u);
	}
}
/*
 * Send a formatted message from sender to everyone on a floor including himself, considering ignorance.
 */
#if 0 /* currently unused, just killing compiler warning.. */
static void floor_msg_format_ignoring(int sender, struct worldpos *wpos, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);
	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);
	/* End the Varargs Stuff */
	va_end(vp);
	/* Display */
	floor_msg_ignoring(sender, wpos, buf);
}
#endif
/*
 * Send a message to everyone on the world surface. (for season change)
 */
void world_surface_msg(cptr msg) {
	int i;

//system-msg, currently unused anyway-	if (cfg.log_u) s_printf("[%s] %s\n", Players[sender]->name, msg);
	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		/* Check this guy */
		if (Players[i]->wpos.wz == 0) msg_print(i, msg);
	}
}


/*
 * Display a message to everyone who is in sight on another player.
 *
 * This is mainly used to keep other players advised of actions done
 * by a player.  The message is not sent to the player who performed
 * the action.
 */
void msg_print_near(int Ind, cptr msg) {
	player_type *p_ptr = Players[Ind];
	int y, x, i;
	struct worldpos *wpos;

	wpos = &p_ptr->wpos;
	if (p_ptr->admin_dm) return;

	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Don't send the message to the player who caused it */
		if (Ind == i) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Can he see this player? */
		if (p_ptr->cave_flag[y][x] & CAVE_VIEW) {
			/* Send the message */
			msg_print(i, msg);
		}
	}
}
void msg_print_near2(int Ind, cptr msg, cptr msg_u) {
	player_type *p_ptr = Players[Ind];
	int y, x, i;
	struct worldpos *wpos;

	wpos = &p_ptr->wpos;
	if (p_ptr->admin_dm) return;

	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Don't send the message to the player who caused it */
		if (Ind == i) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Can he see this player? */
		if (p_ptr->cave_flag[y][x] & CAVE_VIEW) {
			/* Send the message */
			msg_print(i, p_ptr->censor_swearing ? msg : msg_u);
		}
	}
}

/* Whispering: Send message to adjacent players */
void msg_print_verynear(int Ind, cptr msg) {
	player_type *p_ptr = Players[Ind];
	int y, x, i;
	struct worldpos *wpos;
	wpos = &p_ptr->wpos;

	//if (p_ptr->admin_dm) return;

	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Don't send the message to the player who caused it */
		if (Ind == i) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Is he in range? */
		if (abs(p_ptr->py - y) <= 1 && abs(p_ptr->px - x) <= 1) {
			/* Send the message */
			msg_print(i, msg);
		}
	}
}
void msg_print_verynear2(int Ind, cptr msg, cptr msg_u) {
	player_type *p_ptr = Players[Ind];
	int y, x, i;
	struct worldpos *wpos;
	wpos = &p_ptr->wpos;

	//if (p_ptr->admin_dm) return;

	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Don't send the message to the player who caused it */
		if (Ind == i) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Is he in range? */
		if (abs(p_ptr->py - y) <= 1 && abs(p_ptr->px - x) <= 1) {
			/* Send the message */
			msg_print(i, p_ptr->censor_swearing ? msg : msg_u);
		}
	}
}

/* Take into account player hallucination.
   In theory, msg_print_near_monster() could be used, but that one doesn't support
   starting on colour codes, and this function is used in all places where messages
   ought to follow the game's message colour scheme.. this all seems not too efficient :/.
   m_idx == -1: assume seen. */
void msg_print_near_monvar(int Ind, int m_idx, cptr msg, cptr msg_garbled, cptr msg_unseen) {
	player_type *p_ptr = Players[Ind];
	int y, x, i;
	struct worldpos *wpos;
	wpos = &p_ptr->wpos;

	if (p_ptr->admin_dm) return;

	y = p_ptr->py;
	x = p_ptr->px;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Don't send the message to the player who caused it */
		if (Ind == i) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Can he see this player? */
		if (p_ptr->cave_flag[y][x] & CAVE_VIEW) {
			/* Send the message */
			if (m_idx != -1 && !p_ptr->mon_vis[m_idx]) msg_print(i, msg_unseen);
			else if (p_ptr->image) msg_print(i, msg_garbled);
			else msg_print(i, msg);
		}
	}
}

/*
 * Same as above, except send a formatted message.
 */
void msg_format_near(int Ind, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_near(Ind, buf);
}

/* for whispering */
void msg_format_verynear(int Ind, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_verynear(Ind, buf);
}

/* location-based - also, skip player Ind if non-zero */
void msg_print_near_site(int y, int x, worldpos *wpos, int Ind, bool view, cptr msg) {
	int i, d;
	player_type *p_ptr;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Skip specified player, if any */
		if (i == Ind) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Can (s)he see the site? */
		if (view) {
			if (!(p_ptr->cave_flag[y][x] & CAVE_VIEW)) continue;
		} else { /* can (s)he hear it? */
			/* within audible range? */
			d = distance(y, x, Players[i]->py, Players[i]->px);
			/* NOTE: should be consistent with sound_near_site() */
			if (d > MAX_SIGHT) continue;
		}

		/* Send the message */
		msg_print(i, msg);
	}
}

/*
 * Same as above, except send a formatted message.
 */
void msg_format_near_site(int y, int x, worldpos *wpos, int Ind, bool view, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_near_site(y, x, wpos, Ind, view, buf);
}

/*
 * Send a message about a monster to everyone who can see it.
 * Monster name is appended at the beginning. (XXX kludgie!)	- Jir -
 *
 * Example: msg_print_near_monster(m_idx, "wakes up.");
 */
/*
 * TODO: allow format
 * TODO: distinguish 'witnessing' and 'hearing'
 */
void msg_print_near_monster(int m_idx, cptr msg) {
	int i;
	player_type *p_ptr;
	cave_type **zcave;
	char m_name[MNAME_LEN];

	monster_type	*m_ptr = &m_list[m_idx];
	worldpos *wpos = &m_ptr->wpos;


	if (!(zcave = getcave(wpos))) return;

	/* Check each player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check this player */
		p_ptr = Players[i];

		/* Make sure this player is in the game */
		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Make sure this player is at this depth */
		if (!inarea(&p_ptr->wpos, wpos)) continue;

		/* Skip if not visible */
		if (!p_ptr->mon_vis[m_idx]) continue;

		/* Can he see this monster? */
		if (!(p_ptr->cave_flag[m_ptr->fy][m_ptr->fx] & CAVE_VIEW)) continue;

		/* Acquire the monster name */
		monster_desc(i, m_name, m_idx, 0);

		msg_format(i, "%^s %s", m_name, msg);
	}
}

/* send a message to all online party members */
void msg_party_format(int Ind, cptr fmt, ...) {
	int i;
	char buf[1024];
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);
	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);
	/* End the Varargs Stuff */
	va_end(vp);

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;

#if 0
		/* Skip the specified player */
		if (i == Ind)
			continue;
#endif

		/* skip players not in his party */
		if (Players[i]->party != Players[Ind]->party)
			continue;

		/* Tell this one */
		msg_print(i, buf);
	 }
}
/* send a message to all online party members */
void msg_party_print(int Ind, cptr msg, cptr msg_u) {
	int i;

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;

#if 0
		/* Skip the specified player */
		if (i == Ind)
			continue;
#endif

		/* skip players not in his party */
		if (Players[i]->party != Players[Ind]->party)
			continue;

		/* Tell this one */
		msg_print(i, Players[i]->censor_swearing ? msg : msg_u);
	 }
}

/* send a message to all online guild members */
void msg_guild_format(int Ind, cptr fmt, ...) {
	int i;
	char buf[1024];
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);
	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);
	/* End the Varargs Stuff */
	va_end(vp);

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;

#if 0
		/* Skip the specified player */
		if (i == Ind)
			continue;
#endif

		/* skip players not in his guild */
		if (Players[i]->guild != Players[Ind]->guild)
			continue;

		/* Tell this one */
		msg_print(i, buf);
	 }
}
/* send a message to all online guild members */
void msg_guild_print(int Ind, cptr msg, cptr msg_u) {
	int i;

	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED)
			continue;

#if 0
		/* Skip the specified player */
		if (i == Ind)
			continue;
#endif

		/* skip players not in his guild */
		if (Players[i]->guild != Players[Ind]->guild)
			continue;

		/* Tell this one */
		msg_print(i, Players[i]->censor_swearing ? msg : msg_u);
	 }
}

#if 0 /* unused atm */
static char* dodge_diz(int chance) {
	if (chance < 5) return("almost no");
	else if (chance < 14) return("a slight");
	else if (chance < 23) return("a significant");
	else if (chance < 30) return("a good");
	else if (chance < 40) return("a very good");
	else return("a high");
}
#endif

/*
 * Dodge Chance Feedback.
 */
void check_dodge(int Ind) {
	player_type *p_ptr = Players[Ind];
#ifndef NEW_DODGING
	int dun_level = getlevel(&p_ptr->wpos);
	int chance = p_ptr->dodge_level - (dun_level * 5 / 6);

	if (chance < 0) chance = 0;
	if (chance > DODGE_CAP) chance = DODGE_CAP;	// see DODGE_CAP in melee1.c
	//if (is_admin(p_ptr))
		msg_format(Ind, "You have a %d%% chance of dodging a level %d monster.", chance, dun_level);
 #if 0
	if (chance < 5)
		msg_format(Ind, "You have almost no chance of dodging a level %d monster.", dun_level);
	else if (chance < 10)
		msg_format(Ind, "You have a slight chance of dodging a level %d monster.", dun_level);
	else if (chance < 20)
		msg_format(Ind, "You have a significant chance of dodging a level %d monster.", dun_level);
	else if (chance < 40)
		msg_format(Ind, "You have a large chance of dodging a level %d monster.", dun_level);
	else if (chance < 70)
		msg_format(Ind, "You have a high chance of dodging a level %d monster.", dun_level);
	else
		msg_format(Ind, "You will usually dodge a level %d monster.", dun_level);
 #endif
#else
	int lev;

 #ifndef CHECK_CHANCES_DUAL
	lev = p_ptr->lev * 2 < 127 ? p_ptr->lev * 2 : 127;
	if (is_admin(p_ptr))
 #else
	if (is_admin(p_ptr)) lev = p_ptr->lev * 2 < 127 ? p_ptr->lev * 2 : 127;
	else lev = p_ptr->lev * 2 < 100 ? p_ptr->lev * 2 : 100;
	if (TRUE)
 #endif
	msg_format(Ind, "You have a %d%%/%d%% chance of dodging a level %d/%d monster.",
	    apply_dodge_chance(Ind, p_ptr->lev), apply_dodge_chance(Ind, lev), p_ptr->lev, lev);
	/*msg_format(Ind, "You have %s/%s chance of dodging a level %d/%d monster.",
	    dodge_diz(apply_dodge_chance(Ind, p_ptr->lev)), dodge_diz(apply_dodge_chance(Ind, lev)), p_ptr->lev, lev);*/
	else msg_format(Ind, "You have a %d%% chance of dodging a level %d monster's attack.",
	    apply_dodge_chance(Ind, p_ptr->lev), p_ptr->lev);
#endif
	return;
}

void check_intercept(int Ind) {
	player_type *p_ptr = Players[Ind];
	int lev;

#ifndef CHECK_CHANCES_DUAL
	lev = p_ptr->lev * 2 < 127 ? p_ptr->lev * 2 : 127;
	if (is_admin(p_ptr))
#else
	if (is_admin(p_ptr)) lev = p_ptr->lev * 2 < 127 ? p_ptr->lev * 2 : 127;
	else lev = p_ptr->lev * 2 < 100 ? p_ptr->lev * 2 : 100;
	if (TRUE)
#endif
	msg_format(Ind, "You have a %d%%/%d%% chance of intercepting a level %d/%d monster.",
	    calc_grab_chance(p_ptr, 100, p_ptr->lev), calc_grab_chance(p_ptr, 100, lev), p_ptr->lev, lev);
	else msg_format(Ind, "You have a %d%% chance of intercepting a level %d monster's attack.",
	    calc_grab_chance(p_ptr, 100, p_ptr->lev), p_ptr->lev);

	return;
}

void check_parryblock(int Ind) {
#ifdef ENABLE_NEW_MELEE
	char msg[MAX_CHARS];
	player_type *p_ptr = Players[Ind];
	int apc = apply_parry_chance(p_ptr, p_ptr->weapon_parry), abc = apply_block_chance(p_ptr, p_ptr->shield_deflect);

	if (is_admin(p_ptr)) {
		msg_format(Ind, "You have exactly %d%%/%d%% base chance of parrying/blocking.",
			p_ptr->weapon_parry, p_ptr->shield_deflect);
		msg_format(Ind, "You have exactly %d%%/%d%% real chance of parrying/blocking.",
			apc, abc);
	} else {
 #if 0
		if (!apc) strcpy(msg, "You cannot parry at the moment. ");
		else if (apc < 5) msg_print(Ind, "You have almost no chance of parrying.");
		else if (apc < 10) msg_print(Ind, "You have a slight chance of parrying.");
		else if (apc < 20) msg_print(Ind, "You have a significant chance of parrying.");
		else if (apc < 30) msg_print(Ind, "You have a good chance of parrying.");
		else if (apc < 40) msg_print(Ind, "You have a very good chance of parrying.");
		else if (apc < 50) msg_print(Ind, "You have an excellent chance of parrying.");
		else msg_print(Ind, "You have a superb chance of parrying.");
 #else
		if (!apc) strcpy(msg, "You cannot parry at the moment. ");
		else strcpy(msg, format("You have a %d%% chance of parrying. ", apc));
 #endif
 #if 0
		if (!abc) msg_print(Ind, "You cannot block at the moment.");
		else if (abc < 5) msg_print(Ind, "You have almost no chance of blocking.");
		else if (abc < 14) msg_print(Ind, "You have a slight chance of blocking.");
		else if (abc < 23) msg_print(Ind, "You have a significant chance of blocking.");
		else if (abc < 33) msg_print(Ind, "You have a good chance of blocking.");
		else if (abc < 43) msg_print(Ind, "You have a very good chance of blocking.");
		else if (abc < 48) msg_print(Ind, "You have an excellent chance of blocking.");
		else msg_print(Ind, "You have a superb chance of blocking.");
 #else
		if (!abc) strcat(msg, "You cannot block at the moment.");
		else strcat(msg, format("You have a %d%% chance of blocking.", abc));
 #endif
		msg_print(Ind, msg);
	}
#endif
	return;
}



void toggle_shoot_till_kill(int Ind) {
	player_type *p_ptr = Players[Ind];
	if (p_ptr->shoot_till_kill) {
		msg_print(Ind, "\377wFire-till-kill mode now off.");
		p_ptr->shooting_till_kill = FALSE;
	} else {
		msg_print(Ind, "\377wFire-till-kill mode now on!");
	}
	p_ptr->shoot_till_kill = !p_ptr->shoot_till_kill;
s_printf("SHOOT_TILL_KILL: Player %s toggles %s.\n", p_ptr->name, p_ptr->shoot_till_kill ? "true" : "false");
	p_ptr->redraw |= PR_STATE;
	return;
}

void toggle_dual_mode(int Ind) {
	player_type *p_ptr = Players[Ind];

	if (!get_skill(p_ptr, SKILL_DUAL)) {
		msg_print(Ind, "You are not proficient in dual-wielding.");
		return;
	}

	if (p_ptr->dual_mode) {
		if (cursed_p(&p_ptr->inventory[INVEN_WIELD]) || (p_ptr->dual_wield && cursed_p(&p_ptr->inventory[INVEN_ARM]))) {
			msg_print(Ind, "\377yYou cannot switch to main-hand mode while wielding a cursed weapon!");
			return;
		}
		msg_print(Ind, "\377wDual-wield mode: Main-hand. (This disables all dual-wield boni.)");
	} else msg_print(Ind, "\377wDual-wield mode: Dual-hand.");

	p_ptr->dual_mode = !p_ptr->dual_mode;
s_printf("DUAL_MODE: Player %s toggles %s.\n", p_ptr->name, p_ptr->dual_mode ? "true" : "false");
	p_ptr->redraw |= PR_STATE | PR_PLUSSES;
	calc_boni(Ind);
	return;
}

#ifdef CENSOR_SWEARING
/* similar to strstr(), but catches char repetitions and swap-arounds.
   TODO: current implementation is pretty naive, need to use more effective algo when less lazy. */
#define GET_MOST_DELAYED_MATCH /* must be enabled */
static char* censor_strstr(char *line, char *word, int *eff_len) {
	char bufl[MSG_LEN], bufs[NAME_LEN], *best = NULL;
	int i, j, add;

	if (line[0] == '\0') return((char*)NULL);
	if (word[0] == '\0') return((char*)NULL);//or line, I guess.

	strcpy(bufl, line);
	strcpy(bufs, word);
	*eff_len = 0;

	/* (Note: Since we're returning line[] without any char-mapping, we
	   can only do replacements here, not shortening/expanding. */
	/* replace 'ck' or 'kc' by just 'kk' */
	i = 0;
	while (bufl[i] && bufl[i + 1]) {
		if (bufl[i] == 'c' && bufl[i + 1] == 'k') bufl[i] = 'k';
		else if (bufl[i] == 'k' && bufl[i + 1] == 'c') bufl[i + 1] = 'k';
		i++;
	}
	i = 0;
	while (bufs[i] && bufs[i + 1]) {
		if (bufs[i] == 'c' && bufs[i + 1] == 'k') bufs[i] = 'k';
		else if (bufs[i] == 'k' && bufs[i + 1] == 'c') bufs[i + 1] = 'k';
		i++;
	}

	/* search while allowing repetitions/swapping of characters.
	   Important: Get the furthest possible match for the starting char of
	   the search term, required for 'preceeding/separated' check which is
	   executed directly on 'line' further below in censor_aux(). Eg:
	   sshit -> match the 2nd s and return it as position, not the 1st.
	   Otherwise, "this shit" -> "thisshit" -> the first s and the h would
	   be separated by a space in 'line' and therefore trigger the nonswear
	   special check. */
	i = 0;
	while (bufl[i]) {
		j = 0;
		add = 0; /* track duplicte chars */

 #ifdef GET_MOST_DELAYED_MATCH
		/* if we already got a match and the test of the upfollowing
		   position is negative, take that match, since we still want
		   to get all occurances eventually, not just the last one. */
		if (bufs[j] != bufl[i + j + add] && best) return(best);
 #endif

		while (bufs[j] == bufl[i + j + add]) {
			if (bufs[j + 1] == '\0') {
				*eff_len = j + add + 1;
 #ifndef GET_MOST_DELAYED_MATCH
				return(&line[i]); /* FOUND */
 #else
				best = &line[i];
				break;
 #endif
			}
			j++;

			/* end of line? */
			if (bufl[i + j + add] == '\0')
 #ifndef GET_MOST_DELAYED_MATCH
				return((char*)NULL); /* NOT FOUND */
 #else
				return(best);
 #endif

			/* reduce duplicate chars */
			if (bufs[j] != bufl[i + j + add]) {
				if (bufs[j - 1] == bufl[i + j + add])
					add++;
			}
		}
		/* NOT FOUND so far */
		i++;
	}
 #ifndef GET_MOST_DELAYED_MATCH
	return((char*)NULL); /* NOT FOUND */
 #else
	return(best);
 #endif
}

/* Censor swear words while keeping good words, and determining punishment level */
//NOTE: EXEMPT_BROKEN_SWEARWORDS and HIGHLY_EFFECTIVE_CENSOR shouldn't really work togehter (because latter one kills spaces etc.)
#define EXEMPT_BROKEN_SWEARWORDS	/* don't 'recognize' swear words that are broken up into 'innocent' parts */
#define HIGHLY_EFFECTIVE_CENSOR		/* strip all kinds of non-alpha chars too? ([disabled])*/
#define HEC_RESTRICTION			/* actually restrict HIGHLY_EFFECTIVE_CENSOR so it does _not_ strip a couple specific \
					   non-alpha chars which probably weren't used for masking swearing - eg: \
					   "as---s" obviously means "ass", but "as '?' symbol" shouldn't recognize "ass". */
#define CENSOR_PH_TO_F			/* (slightly picky) convert ph to f ?*/
#define REDUCE_DUPLICATE_H		/* (slightly picky) reduce multiple h to just one? */
#define REDUCE_H_CONSONANT		/* (slightly picky) drop h before consonants? */
#define CENSOR_LEET			/* 433+ $p34k: Try to translate certain numbers and symbols to letters? ([disabled]) */
#define EXEMPT_VSHORT_COMBINED		/* Exempt very short swear words if they're part of a bigger 'word'?
					   //(Problem: They could just be preceeded by 'the' or 'you' w/o a space) -
					   //practically unfeasible (~1250+ words!) to utilize nonswear list for this ([enabled]) */
 #ifdef EXEMPT_VSHORT_COMBINED
  #define VSHORT_STEALTH_CHECK		/* Perform a check if the swear word is masked by things like 'the', 'you', 'a(n)'.. */
 #endif
#define SMARTER_NONSWEARING		/* Match single spaces inside nonswearing words with any amount of consecutive insignificant characters in the chat */
static int censor_aux(char *buf, char *lcopy, int *c, bool leet, bool max_reduce, bool no_logging) {
	int i, j, k, offset, cc[MSG_LEN], pos, eff_len;
	char line[MSG_LEN];
	char *word, l0, l1, l2, l3;
	int level = 0;
 #ifdef VSHORT_STEALTH_CHECK
	bool masked;
 #endif
 #ifdef HEC_RESTRICTION
	char repeated = 0;
	bool hec_seq = FALSE, ok1 = FALSE, ok2 = FALSE, ok3 = FALSE;
 #endif

	/* create working copies */
	strcpy(line, buf);
	/* '!i' explanation: The first index (i=0) may not yet be the array's null-terminator, but just point at string index 0 and hence be 0.
	    If it is 0 and the string was empty, then c[1] is guaranteed to be a 0-terminator, to avoid buffer overrun here (so an additional i < MSG_LEN check is not needed here). */
	for (i = 0; !i || c[i]; i++) cc[i] = c[i];
	cc[i] = 0;

	/* replace certain non-alpha chars by alpha chars (leet speak detection)? */
	if (leet) {
 #ifdef HIGHLY_EFFECTIVE_CENSOR
		bool is_leet = FALSE, was_leet;
 #endif
		i = 0;
		while (lcopy[i]) {
 #ifdef HIGHLY_EFFECTIVE_CENSOR
			was_leet = is_leet;
			is_leet = TRUE;//(i != 0);//meaning 'i > 0', for efficiency
 #endif

			/* NOTE: If HIGHLY_EFFECTIVE_CENSOR is on, we must here replace any non-alpha character by an alpha-character as placeholder,
			   for any non-alpha character that is considered relevant for disturbing a swear-word.
			   Example: For some time, '2' was missing in the list below, resulting it to get dropped in HIGHLY_EFFECTIVE_CENSOR,
			   resulting in "(20k" becoming "(0k" and therefore "cok" triggering "cock" swearing. ^^
			   A special exception for "(0k)" could be added to nonswearing word list I guess. */

			/* keep table of leet chars consistent with the one in handle_censor() */
			switch (lcopy[i]) {
			case '@': lcopy[i] = 'a'; break;
			case '<': lcopy[i] = 'c'; break;
			case '!': lcopy[i] = 'i'; break;
			case '|': lcopy[i] = 'l'; break;//hm, could be i too :/
			case '$': lcopy[i] = 's'; break;
			case '+': lcopy[i] = 't'; break;
			case '1': lcopy[i] = 'i'; break;
			case '2': lcopy[i] = 'z'; break;//added to break "(20k" -> "c ok", but makes sense anyway apart from this
			case '3': lcopy[i] = 'e'; break;
			case '4': lcopy[i] = 'a'; break;
			case '5': lcopy[i] = 's'; break;
			case '6': lcopy[i] = 'g'; break;
			case '7': lcopy[i] = 't'; break;
			case '8': lcopy[i] = 'b'; break;
			case '9': lcopy[i] = 'g'; break;
			case '0': lcopy[i] = 'o'; break;

			/* important characters that are usually not leet speek but must be turned
			   into characters that will not get filtered out later, because these
			   characters listed here could actually break words and thereby cause
			   false positives. Eg 'headless (h, l27, 427)' would otherwise wrongly turn
			   into 'headlesshltxaxtx' and trigger false 'shit' positive. */
			case '(': lcopy[i] = 'c'; break; //all of these could be c,i,l I guess.. pick the least problematic one (i)
			case ')': lcopy[i] = 'i'; break;
			case '/': lcopy[i] = 'i'; break; // l prob: "mana/" -> mANAL :-p
			case '\\': lcopy[i] = 'i'; break;

 #ifdef HIGHLY_EFFECTIVE_CENSOR
			/* hack: Actually _counter_ the capabilities of highly-effective
			   censoring being done further below after this. Reason:
			   Catch numerical expressions that probably aren't l33t sp34k,
			   such as "4.5", "4,5" or "4/5" but also '4} (55'!
			   Exact rule: <leet char><non-leet non-alphanum char> = allowed. */
			default:
				is_leet = FALSE; //this character wasn't one of the leet cases above
				/* inconspicuous non-alphanum char? */
				if (was_leet &&
				    (lcopy[i] < 'a' || lcopy[i] > 'z') && (lcopy[i] < '0' || lcopy[i] > '9'))
					/* replace by, say 'x' (harmless^^) (Note: Capital character doesn't work, gets filtered out futher below */
					lcopy[i] = 'x';
				break;
 #else
			default:
				lcopy[i] = ' ';
  #if 0 /* nonsense, because nonswearing isn't applied after this anymore, but was long before in handle_censor() already! */
				lcopy[i] = '~'; /* ' ' is usually in use in nonswearing.txt,
				    so it would need to be removed there whenever we turn off HIGHLY_EFFECTIVE_CENSOR
				    and put back in when we reenable it. Using a non-used character instead of SPACE
				    is therefore a much more comfortable way. */
  #endif
				break;
 #endif
			}

			line[cc[i]] = lcopy[i];
			i++;
		}
	}

 #ifdef REDUCE_H_CONSONANT
	/* reduce 'h' before consonant */
	//TODO: do HIGHLY_EFFECTIVE_CENSOR first probably
	i = j = 0;
	while (lcopy[j]) {
		if (lcopy[j] == 'h' &&
		    lcopy[j + 1] != '\0' &&
		    lcopy[j + 1] != 'a' &&
		    lcopy[j + 1] != 'e' &&
		    lcopy[j + 1] != 'i' &&
		    lcopy[j + 1] != 'o' &&
		    lcopy[j + 1] != 'u' &&
		    lcopy[j + 1] != 'y'
		    && lcopy[j + 1] >= 'a' && lcopy[j + 1] <= 'z' /*TODO drop this, see 'TODO' above*/
		    /* detect 'shlt' as masked form of 'shit', without reducing it to 'slt' */
		    && (!j || (lcopy[j - 1] != 'c' && lcopy[j - 1] != 's'))
		    ) {
			j++;
		}

		/* build index map for stripped string */
		cc[i] = cc[j];
		lcopy[i] = lcopy[j];

		i++;
		j++;
	}
	/* ensure the reduced string is possibly terminated earlier */
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */

	/* reduce 'h' after consonant except for c or s */
	//TODO: do HIGHLY_EFFECTIVE_CENSOR first probably
	i = j = 1;
	if (lcopy[0])
	while (lcopy[j]) {
		if (lcopy[j] == 'h' &&
		    lcopy[j - 1] != 'c' &&
		    lcopy[j - 1] != 's' &&
		    lcopy[j - 1] != 'a' &&
		    lcopy[j - 1] != 'e' &&
		    lcopy[j - 1] != 'i' &&
		    lcopy[j - 1] != 'o' &&
		    lcopy[j - 1] != 'u'
		    && lcopy[j - 1] >= 'a' && lcopy[j - 1] <= 'z' /*TODO drop this, see 'TODO' above*/
		    ) {
			j++;
			continue;
		}

		/* build index map for stripped string */
		cc[i] = cc[j];
		lcopy[i] = lcopy[j];

		i++;
		j++;
	}
	/* ensure the reduced string is possibly terminated earlier */
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */
 #endif

 #ifdef HIGHLY_EFFECTIVE_CENSOR
	/* check for non-alpha chars and _drop_ them (!) */
	i = j = 0;
	while (lcopy[j]) {
		/* non-alphanum symbol? */
		if ((lcopy[j] < 'a' || lcopy[j] > 'z') &&
		    lcopy[j] != 'Z') {
  #ifdef HEC_RESTRICTION
			/* new sequence to look ahead into? */
			if (!hec_seq) {
				hec_seq = TRUE;
				k = j;
				while (lcopy[k] && (lcopy[k] < 'a' || lcopy[k] > 'z') && lcopy[k] != 'Z') {
					/* don't allow these 'bad' symbols to be the only ones occuring. Note: spaces don't count as bad symbols */
					if (lcopy[k] != '.' && lcopy[k] != ',' && lcopy[k] != '-') ok3 = TRUE;

					/* make sure it's not just a single symbol getting repeated */
					if (!repeated) {
						repeated = lcopy[k];
						k++;
						continue;
					}
					if (lcopy[k] != repeated) ok1 = TRUE; /* different symbols = rather inconspicuous */

					/* at least 3 non-alphanum symbols for the sequence to have any "meaning" */
					if (k - j >= 2) ok2 = TRUE;

					if (ok1 && ok2 && ok3) break;
					k++;
				}
			}
			if (!ok1 || !ok2 || !ok3) { /* conspicuous sequence? (possibly intended to mask swearing) */
				/* cut them out */
				j++;
				continue;
			}
  #else
			/* cut them out */
			j++;
			continue;
  #endif
		}
  #ifdef HEC_RESTRICTION
		else hec_seq = FALSE;
  #endif

		/* modify index map for stripped string */
		cc[i] = cc[j];
		lcopy[i] = lcopy[j];
		i++;
		j++;
	}
	/* ensure the reduced string is possibly terminated earlier */
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */
 #endif

	/* reduce repeated chars (>3 for consonants, >2 for vowel) */
	i = j = 0;
	while (lcopy[j]) {
		/* paranoia (if HIGHLY_EFFECTIVE_CENSOR is enabled) */
		if (lcopy[j] < 'a' || lcopy[j] > 'z') {
			cc[i] = cc[j];
			lcopy[i] = lcopy[j];
			i++;
			j++;
			continue;
		}

		/* reduce any char repetition and re-check for swear words */
		if (max_reduce) {
			if (lcopy[j + 1] == lcopy[j]) {
				j++;
				continue;
			}
		} else switch (lcopy[j]) {
		case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
			if (lcopy[j + 1] == lcopy[j] &&
			    lcopy[j + 2] == lcopy[j]) {
				j++;
				continue;
			}
			break;
		default:
			if (lcopy[j + 1] == lcopy[j] &&
			    lcopy[j + 2] == lcopy[j] &&
			    lcopy[j + 3] == lcopy[j]) {
				j++;
				continue;
			}
			break;
		}

		/* modify index map for reduced string */
		cc[i] = cc[j];
		lcopy[i] = lcopy[j];
		i++;
		j++;
	}
	/* ensure the reduced string is possibly terminated earlier */
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */

	/* check for swear words and censor them */
	for (i = 0; swear[i].word[0] && i < MAX_SWEAR; i++) {
		if (!swear[i].level) continue;
		offset = 0;

		/* check for multiple occurrances of this swear word */
		while ((word = censor_strstr(lcopy + offset, swear[i].word, &eff_len))) {
			pos = word - lcopy;
			l0 = tolower(line[cc[pos]]);
			l1 = l2 = l3 = 0; //kill compiler warnings
			if (cc[pos] >= 1) l1 = tolower(line[cc[pos] - 1]);
			if (cc[pos] >= 2) {
				l2 = tolower(line[cc[pos] - 2]);
				/* catch & dismiss colour codes */
				if (l2 == '\377') l1 = 'Y'; /* 'disable' char */
			}
			if (cc[pos] >= 3) {
				l3 = tolower(line[cc[pos] - 3]);
				/* catch & dismiss colour codes */
				if (l3 == '\377') l2 = 'Y'; /* 'disable' char */
			}

			/* prevent checking the same occurance repeatedly */
			offset = pos + 1;

 #ifdef EXEMPT_BROKEN_SWEARWORDS
			/* hm, maybe don't track swear words if proceeded and postceeded by some other letters
			   and separated by spaces inside it -- and if those nearby letters aren't the same letter,
			   if someone just duplicates it like 'sshitt' */
			/* check for swear-word-preceding non-duplicate alpha char */
			if (cc[pos] > 0 &&
			    l1 >= 'a' && l1 <= 'z' &&
			    (l1 != l0 || strlen(swear[i].word) <= 3)) {
  #ifdef EXEMPT_VSHORT_COMBINED
				/* special treatment for swear words of <= 3 chars length: */
				if (strlen(swear[i].word) <= 3) {
   #ifdef VSHORT_STEALTH_CHECK
					/* check for masking by 'the', 'you', 'a(n)' etc. */
					masked = TRUE;
					if (cc[pos] == 1 || cc[pos] == 2) {
						/* if there's only 1 or 2 chars before the swear word, it depends on the first
						   char of the swear word: vowel is ok, consonant is not. */
						switch (l0) {
						case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
							masked = FALSE;
							break;
						}
					}
					/* if there are more chars before the swear word, it's maybe ok if
					   the swear word starts on a vowel ('XXXXass' is very common!) */
					else {
						/* ..to be specific, if the char before the swear word is NOT a vowel (well, duh, true for most cases).
						   Yeah yeah, just trying to fix 'ass' detection here, and a 'y' or 'u' in front of it remains swearing.. */
						switch (l0) {
						case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
							switch (l1) {
							case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
								break;
							default:
								masked = FALSE;
								break;
							}
							break;
						}
					}
					/* if swear word is followed by a vowel, it's ok. */
					switch (tolower(line[cc[pos] + strlen(swear[i].word)])) {
					case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
						masked = FALSE;
						break;
					}
				    /* so if it is already determined by now to be a masked swear word, skip the vshort-exemption-checks */
				    if (!masked) {
   #endif

   #if 0 /* softened this up, see below */
					/* if there's UP TO 2 other chars before it or exactly 1 non-duplicate char, it's exempt.
					   (for more leading chars, nonswear has to be used.) */
					if (cc[pos] == 1 && l1 >= 'a' && l1 <= 'z' && l1 != l0) continue;
					if (cc[pos] == 2) {
						if (l1 >= 'a' && l1 <= 'z' && l2 >= 'a' && l2 <= 'z' &&
						    (l0 != l1 || l0 != l2 || l1 != l2)) continue;
						/* also test for only 1 leading alpha char */
						if ((l2 < 'a' || l2 > 'z') &&
						    l1 >= 'a' && l1 <= 'z' && l1 != l0) continue;
					}
					if (cc[pos] >= 3) {
						 if ((l3 < 'a' || l3 > 'z') &&
						    l1 >= 'a' && l1 <= 'z' && l2 >= 'a' && l2 <= 'z' &&
						    (l0 != l1 || l0 != l2 || l1 != l2)) continue;
						/* also test for only 1 leading alpha char */
						if ((l2 < 'a' || l2 > 'z') &&
						    l1 >= 'a' && l1 <= 'z' && l1 != l0) continue;
					}
					/* if there's no char before it but 2 other chars after it or 1 non-dup after it, it's exempt. */
					//TODO maybe - or just use nonswear for that
   #else
					/* if there's at least 2 other chars before it, which aren't both duplicates, or
					   if there's exactly 1 non-duplicate char before it, it's exempt. */
					if (cc[pos] == 1 && l1 >= 'a' && l1 <= 'z' && l1 != l0) continue;
					if (cc[pos] >= 2) {
						if (l1 >= 'a' && l1 <= 'z' && l2 >= 'a' && l2 <= 'z' &&
						    (l0 != l1 || l0 != l2 || l1 != l2)) continue;
						/* also test for only 1 leading alpha char */
						if ((l2 < 'a' || l2 > 'z') &&
						    l1 >= 'a' && l1 <= 'z' && l1 != l0) continue;
					}
					/* if there's no char before it but 2 other chars after it or 1 non-dup after it, it's exempt. */
					//TODO maybe - or just use nonswear for that
   #endif
   #ifdef VSHORT_STEALTH_CHECK
				    }
   #endif
				}
  #endif

				/* check that the swear word occurance was originally non-continuous, ie separated by space etc.
				   (if this is FALSE then this is rather a case for nonswearwords.txt instead.) */
				for (j = cc[pos]; j < cc[pos + strlen(swear[i].word) - 1]; j++) {
					if (tolower(line[j]) < 'a' || tolower(line[j] > 'z')) {
						/* heuristical non-swear word found! */
						break;
					}
				}
				/* found? */
				if (j < cc[pos + strlen(swear[i].word) - 1]) {
					/* prevent it from getting tested for swear words */
					continue;
				}
			}
 #endif

			/* we can skip ahead */
			offset = pos + strlen(swear[i].word);

			/* If it's level 1, allow for chat but not for naming (eg foreign language words that are deemed inoffensive while used in the respective language) */
			if (swear[i].level > 1) {
 #if 0
				/* censor it! */
				for (j = 0; j < eff_len; j++) {
					line[cc[pos + j]] = buf[cc[pos + j]] = '*';
					lcopy[pos + j] = '*';
				}
 #else
				/* actually censor separator chars too, just so it looks better ;) */
				for (j = 0; j < eff_len - 1; j++) {
					for (k = cc[pos + j]; k <= cc[pos + j + 1]; k++)
						line[k] = buf[k] = '*';

					/* MUST be disabled or interferes with detection overlaps: */
					/* for processing lcopy in a while loop instead of just 'if'
					   -- OBSOLETE ACTUALLY (thanks to 'offset')? */
					//lcopy[pos + j] = '*';
				}
				/* see right above - MUST be disabled: */
				//lcopy[pos + j] = '*';
 #endif
			}

 #if 1 //for debugging only
			if (!no_logging) s_printf("SWEARING: Matched '%s' (%d) <%s>\n", swear[i].word, swear[i].level, line);
 #endif
			level = MAX(level, swear[i].level);
		}
	}
	return(level);
}

/* Check for swear words, censor + punish */
int handle_censor(char *message) {
	int i, j, im, jm, cc[MSG_LEN], offset;
 #ifdef CENSOR_LEET
	int j_pre = 0, cc_pre[MSG_LEN];
 #endif
	char line[MSG_LEN], lcopy[MSG_LEN], lcopy2[MSG_LEN];
	char tmp1[MSG_LEN], tmp2[MSG_LEN], tmp3[MSG_LEN], tmp4[MSG_LEN];
	char *word, *c_p;

 #ifdef SMARTER_NONSWEARING
	/* non-swearing extra check (reduces insignificant characters) */
	bool reduce;
	int ccns[MSG_LEN];
 #endif

	/* Create primary working copy */
	word = message;
	c_p = line;
	while (*word) {
		/* discard all special markers */
		if (*word >= '\373' && *word <= '\376') {
			word++;
			continue;
		}

		/* discard _valid_ colour codes */
		if (*word == '\377' ||
		/* unhack: hlore uses \372 marker at the beginning as replacemen for \377, kdiz uses it later on in the line */
		    *word == '\372') {
			switch (word[1]) {
			case '\377':
				*c_p = '{';
				c_p++;
				/* fall through */
			case '-':
			case '.':
				word++;
				break;
			default:
				/* if no valid colour code symbol, treat as normal character, else skip it too */
				if (color_char_to_attr(word[1]) != -1) word++;
			}
		} else {
			*c_p = *word;
			c_p++;
		}
		word++;
	}
	*c_p = *word; /* null-terminate */

	strcpy(lcopy, line);

 #if 1	/* extra: also apply non-swearing in advance here, for exact matching (ie case-sensitive, non-alphanum, etc) */
	/* Maybe todo: Also apply nonswear to lcopy2 (the non-reduced version of lcopy) afterwards */
	for (i = 0; nonswear[i][0] && i < MAX_NONSWEAR; i++) {
  #ifndef HIGHLY_EFFECTIVE_CENSOR
		/* hack! If HIGHLY_EFFECTIVE_CENSOR is NOT enabled, skip all nonswearing-words that contain spaces!
		   This is done because those nonswearing-words are usually supposed to counter wrong positives
		   from HIGHLY_EFFECTIVE_CENSOR, and will lead to false negatives when it is disabled xD
		   (eg: 'was shit' chat with "as sh" non-swear)  - C. Blue */
		//if (strchr(nonswear[i], ' ')) continue; -- not here, in later application of non-swear yes
  #endif

		offset = 0;
		/* check for multiple occurrances of this nonswear word */
		//while ((word = strstr(lcopy + offset, nonswear[i]))) {
		while ((word = my_strcasestr(lcopy + offset, nonswear[i]))) { //CASE-INSENSITIVE instead!
			/* prevent checking the same occurance repeatedly */
			offset = word - lcopy + strlen(nonswear[i]);

			if ((nonswear_affix[i] & 0x1)) {
				if (word == lcopy) continue; //beginning of lcopy string?
				if (!((*(word - 1) >= 'a' && *(word - 1) <= 'z') ||
				    (*(word - 1) >= 'A' && *(word - 1) <= 'Z') ||
				    (*(word - 1) >= '0' && *(word - 1) <= '1') ||
				    *(word - 1) == '\''))
					continue;
			}
			if ((nonswear_affix[i] & 0x2)) {
				if (!word[strlen(nonswear[i])]) continue; //end of lcopy string?
				if (!((lcopy[offset] >= 'a' && lcopy[offset] <= 'z') ||
				    (lcopy[offset] >= 'A' && lcopy[offset] <= 'Z') ||
				    (lcopy[offset] >= '0' && lcopy[offset] <= '1') ||
				    lcopy[offset] == '\''))
					continue;
			}

			/* prevent it from getting tested for swear words */
			for (j = 0; j < strlen(nonswear[i]); j++)
				lcopy[(word - lcopy) + j] = 'Z';
		}
	}
 #endif

	/* special expressions to exempt: '@S...<.>blabla': '@S-.shards' -> '*****hards' :-p */
	if ((word = strstr(lcopy, "@S"))
	    && (word[2] != ':')) { /* it's a store sign! (@S:) */
		char *c = word + 2;
		bool terminated = FALSE;

		/* confirm inscription validity */

		if (*c == 'B') {
			if (c[2] == ';') c += 3;
			else if (c[3] == ';') c += 4;
			else c++;
		}

		if (*c == '+' || *c == '%') c++;

		while (TRUE) {
			switch (*c) {
			/* still within @S expression */
			case 'k': case 'K': case 'm': case 'M':
			case '0': case '1': case '2':
			case '3': case '4': case '5': case '6':
			case '7': case '8': case '9':
				c++;
				continue;
			/* end of @S expression */
			case 0: terminated = TRUE; /* fall through */
			case '-':
			case '.': case ' ':
				/* prevent the whole expression from getting tested for swear words */
				while (word <= (terminated ? c - 1 : c)) {
					//lcopy2[(word - lcopy)] = 'Z';
					*word = 'Z';
					word++;
				}
				c = lcopy + strlen(lcopy); //to exit outer while loop
				break;
			/* not a valid @S expression or not followed up by any further text */
			default:
				c = lcopy + strlen(lcopy); //to exit outer while loop
				break;
			}
			if (terminated) break;
		}
	}

	/* convert to lower case */
	i = 0;
	while (lcopy[i]) {
		lcopy[i] = tolower(lcopy[i]);
		i++;
	}

	/* strip colour codes and initialize index map (cc[]) at that */
	i = j = 0;
	while (lcopy[j]) {
		if (lcopy[j] == '\377') {
			j++;
			if (lcopy[j]) j++;
			continue;
		}

		/* initially build index map for stripped string */
		cc[i] = j;
		lcopy[i] = lcopy[j];

		i++;
		j++;
	}
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */

 #ifdef CENSOR_PH_TO_F
	/* reduce ph to f */
	i = j = 0;
	while (lcopy[j]) {
		if (lcopy[j] == 'p' && lcopy[j + 1] == 'h') {
			cc[i] = cc[j];
			lcopy[i] = 'f';
			i++;
			j += 2;
			continue;
		}

		/* build index map for stripped string */
		cc[i] = cc[j];
		lcopy[i] = lcopy[j];

		i++;
		j++;
	}
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */
 #endif

 #ifdef REDUCE_DUPLICATE_H
	/* reduce hh to h */
	i = j = 0;
	while (lcopy[j]) {
		if (lcopy[j] == 'h' && lcopy[j + 1] == 'h') {
			j++;
			continue;
		}

		/* build index map for stripped string */
		cc[i] = cc[j];
		lcopy[i] = lcopy[j];

		i++;
		j++;
	}
	lcopy[i] = '\0';
	cc[i] = 0;
	if (!i) cc[1] = 0; /* Add "null-terminator". The first cc[0] is not a real terminator but still a reference to a string position (0 in this case). */
 #endif

 #ifdef CENSOR_LEET
	/* special hardcoded case: variants of '455hole' leet-speak */
	j = 0;
	for (i = 0; i < strlen(lcopy); i++) {
		/* reduce duplicate chars (must do it this way instead of i==i+1, because we need full cc-mapped string) */
		if (i && lcopy[i] == lcopy[i - 1]) continue;
		/* build un-leeted, mapped string */
		if ((lcopy[i] >= '0' && lcopy[i] <= '9') || (lcopy[i] >= 'a' && lcopy[i] <= 'z')
		    || lcopy[i] == '@' || lcopy[i] == '$' || lcopy[i] == '!' || lcopy[i] == '|' || lcopy[i] == '+'
		    ) {
			/* keep table of leet chars consistent with the one in censor_aux() */
			switch (lcopy[i]) {
			case '@': lcopy2[j] = 'a'; break;
			case '<': lcopy2[j] = 'c'; break;
			case '!': lcopy2[j] = 'i'; break;
			case '|': lcopy2[j] = 'l'; break;//hm, could be i too :/
			case '$': lcopy2[j] = 's'; break;
			case '+': lcopy2[j] = 't'; break;
			case '1': lcopy2[j] = 'i'; break;
			case '3': lcopy2[j] = 'e'; break;
			case '4': lcopy2[j] = 'a'; break;
			case '5': lcopy2[j] = 's'; break;
			case '6': lcopy2[j] = 'g'; break;
			case '7': lcopy2[j] = 't'; break;
			case '8': lcopy2[j] = 'b'; break;
			case '9': lcopy2[j] = 'g'; break;
			case '0': lcopy2[j] = 'o'; break;
			default: lcopy2[j] = lcopy[i];
			}
			cc_pre[j] = i;
			j++;
		}
	}
	lcopy2[j] = 0;
	if ((word = strstr(lcopy2, "ashole")) ||
	    (word = strstr(lcopy2, "ashoie")))
		/* use severity level of 'ashole' (condensed) for 'asshole' */
		for (i = 0; swear[i].word[0] && i < MAX_SWEAR; i++) {
			if (!swear[i].level) continue;
			if (!strcmp(swear[i].word, "ashole")) {
				j_pre = swear[i].level;
				/* if it was to get censored, do so */
				if (j_pre) {
  #if 0
					for (offset = cc_pre[word - lcopy2]; offset <= cc_pre[(word - lcopy2) + 5]; offset++)
						lcopy[offset] = '*';
  #endif
					for (offset = cc[cc_pre[word - lcopy2]]; offset <= cc[cc_pre[(word - lcopy2) + 5]]; offset++)
						line[offset] = '*';
				}
				break;
			}
		}
 #endif

	/* check for legal words first */
	//TODO: could be moved into censor_aux and after leet speek conversion, to allow leet speeking of non-swear words (eg "c00k")
	strcpy(lcopy2, lcopy); /* use a 'working copy' to allow _overlapping_ nonswear words --- since we copy it below anyway, we don't need to create a working copy here */
	/*TODO!!! non-split swear words should take precedence over split-up non-swear words: "was shit" -> "as sh" is 'fine'.. BEEP!
	  however, this can be done by disabling HIGHLY_EFFECTIVE_CENSORING and enabling EXEMPT_BROKEN_SWEARWORDS as a workaround. */
 #ifdef SMARTER_NONSWEARING
	/* Replace insignificant symbols by spaces and reduce consecutive spaces to one */
	reduce = FALSE;
	j = 0;
	for (i = 0; i < strlen(lcopy2); i++) {
		ccns[j] = i;
		switch (lcopy2[i]) {
		case ' ':
			if (reduce) continue;
			reduce = TRUE;
			lcopy[j] = lcopy2[i];
			j++;
			continue;
		/* insignificant characters only, that won't break leet speak detection in censor_aux() if we filter them out here */
		case '"':
		case '\'':
  #if 1 /* need better way, but perfect solution impossible? ^^ -- for now benefit of doubt (these COULD be part of swearing): */
		case '(':
		case ')':
		case '/':
		case '\\':
  #endif
			if (reduce) continue;
			reduce = TRUE;
			lcopy[j] = ' ';
			j++;
			continue;
		/* normal characters */
		default:
			lcopy[j] = lcopy2[i];
			ccns[j] = i;
			j++;
			reduce = FALSE;
		}
	}
	lcopy[j] = 0;
 #endif
//s_printf("ns: '%s'  '%s'\n", lcopy, lcopy2); //DEBUG
	/* Maybe todo: Also apply nonswear to lcopy2 (the non-reduced version of lcopy) afterwards */
	for (i = 0; nonswear[i][0] && i < MAX_NONSWEAR; i++) {
 #ifndef HIGHLY_EFFECTIVE_CENSOR
		/* hack! If HIGHLY_EFFECTIVE_CENSOR is NOT enabled, skip all nonswearing-words that contain spaces!
		   This is done because those nonswearing-words are usually supposed to counter wrong positives
		   from HIGHLY_EFFECTIVE_CENSOR, and will lead to false negatives when it is disabled xD
		   (eg: 'was shit' chat with "as sh" non-swear)  - C. Blue */
		if (strchr(nonswear[i], ' ')) continue;
 #endif

		offset = 0;
		/* check for multiple occurrances of this nonswear word */
		//while ((word = strstr(lcopy + offset, nonswear[i]))) {
		while ((word = my_strcasestr(lcopy + offset, nonswear[i]))) { //CASE-INSENSITIVE instead!
			/* prevent checking the same occurance repeatedly */
			offset = word - lcopy + strlen(nonswear[i]);

			if ((nonswear_affix[i] & 0x1)) {
				if (word == lcopy) continue; //beginning of lcopy string?
				if (!((*(word - 1) >= 'a' && *(word - 1) <= 'z') ||
				    (*(word - 1) >= '0' && *(word - 1) <= '1') ||
				    *(word - 1) == '\''))
					continue;
			}
			if ((nonswear_affix[i] & 0x2)) {
				if (!word[strlen(nonswear[i])]) continue; //end of lcopy string?
				if (!((lcopy[offset] >= 'a' && lcopy[offset] <= 'z') ||
				    (lcopy[offset] >= '0' && lcopy[offset] <= '1') ||
				    lcopy[offset] == '\''))
					continue;
			}

			/* prevent it from getting tested for swear words */
 #ifdef SMARTER_NONSWEARING
			im = ccns[word - lcopy];
			jm = ccns[word - lcopy + strlen(nonswear[i]) - 1];
			for (j = im; j <= jm; j++)
				lcopy2[j] = 'Z';
 #else
			for (j = 0; j < strlen(nonswear[i]); j++)
				lcopy2[(word - lcopy) + j] = 'Z';
 #endif
		}
	}

	/* perform 2x2 more 'parallel' tests  */
	strcpy(tmp1, line);
	strcpy(tmp2, line);
	strcpy(tmp3, line);
	strcpy(tmp4, line);
	strcpy(lcopy, lcopy2);
	i = censor_aux(tmp1, lcopy, cc, FALSE, FALSE, FALSE); /* continue with normal check */
	strcpy(lcopy, lcopy2);
 #ifdef CENSOR_LEET
	j = censor_aux(tmp2, lcopy, cc, TRUE, FALSE, i); /* continue with leet speek check */
	if (j_pre > j) j = j_pre; //pre-calculated special case
	strcpy(lcopy, lcopy2);
 #else
	j = 0;
 #endif
	im = censor_aux(tmp3, lcopy, cc, FALSE, TRUE, i || j); /* continue with normal check */
	strcpy(lcopy, lcopy2);
 #ifdef CENSOR_LEET
	jm = censor_aux(tmp4, lcopy, cc, TRUE, TRUE, i || j || im); /* continue with leet speek check */
 #else
	jm = 0;
 #endif

	/* combine all censored characters */
	for (offset = 0; tmp1[offset]; offset++)
		if (tmp1[offset] == '*') line[offset] = '*';
 #ifdef CENSOR_LEET
	for (offset = 0; tmp2[offset]; offset++)
		if (tmp2[offset] == '*') line[offset] = '*';
 #endif
	for (offset = 0; tmp3[offset]; offset++)
		if (tmp3[offset] == '*') line[offset] = '*';
 #ifdef CENSOR_LEET
	for (offset = 0; tmp4[offset]; offset++)
		if (tmp4[offset] == '*') line[offset] = '*';
 #endif

	/* Translate working copy back into original message, reinserting its special characters (\372..\377) */
	word = message;
	c_p = line;
	while (*word) {
		/* discard all special markers */
		if (*word >= '\373' && *word <= '\376') {
			word++;
			continue;
		}

		/* discard _valid_ colour codes */
		if (*word == '\377' ||
		/* unhack: hlore uses \372 marker at the beginning as replacemen for \377, kdiz uses it later on in the line */
		    *word == '\372') {
			switch (word[1]) {
			case '\377':
				*c_p = '{';
				c_p++;
				/* fall through */
			case '-':
			case '.':
				word++;
				break;
			default:
				/* if no valid colour code symbol, treat as normal character, else skip it too */
				if (color_char_to_attr(word[1]) != -1) word++;
			}
		} else {
			*word = *c_p;
			c_p++;
		}
		word++;
	}

	/* return 'worst' result */
	if (j > i) i = j;
	if (im > i) i = im;
	if (jm > i) i = jm;
	return(i);
}

 #ifdef ENABLE_MULTILINE_CENSOR
int handle_ml_censor(int Ind, char *line) {
	int cl, old_keep = MSG_LEN / 3, max_keep = (MSG_LEN * 4) / 7, half_keep = max_keep / 2, min_keep = NAME_LEN + 2;
	player_type *p_ptr = Players[Ind];
	char tmpbuf[MSG_LEN], *c, *c_end;

	/* Empty line? - Clear */
	if (!(*line)) return(0);

	/* Attempt single-line censor first and quit if that already succeeds */
	cl = handle_censor(line);
	if (cl) {
		/* We have to clear the buffer after a successful detection */
		p_ptr->multi_chat_line[0] = 0;
		return(cl);
	}

	/* Construct relevant line to check, from the beginnning and ending parts of chat input lines.
	   Note: We abuse 'NAME_LEN' also as limiter/minimum ensured checking length for swear word length. */

	/* Note: If a line gets cut off in the middle of a word, new swearwords might get created that way, eg brass -> br -snip- ass.
	   For that reason we always try to not cut words into two pieces here. */

	/* We are already building a line? Continue with the last 'old_keep' chars from it. */
	if (*(p_ptr->multi_chat_line)) {
		/* Make room by discarding a bit of the beginning of the line, that has already long been checked */
		if (strlen(p_ptr->multi_chat_line) > old_keep) {
			/* Keep the rest of our line */
			c_end = p_ptr->multi_chat_line + strlen(p_ptr->multi_chat_line) - 1;
			c = c_end - old_keep;
			/* Don't cut words in parts, this could result in swear words that didn't exist previously */
			while (c_end - c > min_keep && *c != ' ' && *c != '.' && *c != ',') c++; /* only accept 'safe' characters, eg '!' could be part of l33tsp34k */

			strcpy(tmpbuf, c);
			strcpy(p_ptr->multi_chat_line, tmpbuf);
		}
	}

	/* New line is long -> add beginning and end of it! */
	if (strlen(line) > max_keep) {
		/* Add spacer (maybe superfluous) */
		strcat(p_ptr->multi_chat_line, " ");

		/* add beginning of the line */
		c = line + half_keep;
		/* Don't cut words in parts, this could result in swear words that didn't exist previously */
		while (c - line > min_keep && *c != ' ' && *c != '.' && *c != ',') c--; /* only accept 'safe' characters, eg '!' could be part of l33tsp34k */
		/* Add the beginning */
		strncat(p_ptr->multi_chat_line, line, c - line); // (note: strncat() always adds null-termination)

		/* Add spacer (maybe superfluous) */
		strcat(p_ptr->multi_chat_line, " ");

		/* Check if we pass just fine */
//s_printf("ML_CENSOR(1): %s\n", p_ptr->multi_chat_line);
		cl = handle_censor(p_ptr->multi_chat_line);
		if (cl) {
			/* We have to clear the buffer after a successful detection */
			p_ptr->multi_chat_line[0] = 0;
		}

		/* Add/start with the ending, for next time aka next line */
		c_end = line + strlen(line) - 1;
		c = c_end - half_keep;
		/* Don't cut words in parts, this could result in swear words that didn't exist previously */
		while (c_end - c > min_keep && *c != ' ' && *c != '.' && *c != ',') c++;

		strcpy(tmpbuf, c);
		strcpy(p_ptr->multi_chat_line, tmpbuf);
	} else { /* Line is short -> add the whole line */
		/* Add spacer. This is done because otherwise eg "hi" + "a" + "s" + "s" would just pass as it is "one word" */
		strcat(p_ptr->multi_chat_line, " ");

		strcat(p_ptr->multi_chat_line, line);

		/* Check if we pass just fine */
//s_printf("ML_CENSOR(2): %s\n", p_ptr->multi_chat_line);
		cl = handle_censor(p_ptr->multi_chat_line);
		if (cl) {
			/* We have to clear the buffer after a successful detection */
			p_ptr->multi_chat_line[0] = 0;
		}
	}

	/* We're clear? */
	if (!cl) return(0);

	/* Problem: the already written lines cannot be retracted/censored anymore.
	   So as "alleviation" we simply increase the punishment :D.
	   Plus, we can at least censor the final line */
	strcpy(line, " ***"); //whatever, just start on a space in case this was a slash command, to ensure it still is processed as one, as intended.

	return(cl + 1); //increased punishment ^^'
}
 #endif

/* Handle punishment after running handle_censor() to determine the severity of a swearing word =p */
void handle_punish(int Ind, int level) {
	switch (level) {
	case 0: //nothing to censor
		break;
	case 1: //not censored in chat, but censored in naming
		break;
	case 2: //censored, but no punishment for this
		break;
	case 3: //light punishment: reminder
		msg_print(Ind, "Please do not swear.");
		break;
	default: //normal/heavy punishment: go to jail
		imprison(Ind, (level - 3) * JAIL_SWEARING, "swearing");
	}
}
#else
int handle_censor(char *line) return(0);
void handle_punish(int Ind, int level) return(0);
#endif

/* Helper function for player_talk_aux():
   Allow privileged player accounts of administrators to address hidden dungeon masters via private chat */
bool may_address_dm(player_type *p_ptr) {
	/* DMs aren't hidden on this server? */
	if (!cfg.secret_dungeon_master) return(TRUE);
	/* Need to be privileged, simply to avoid someone creating such a name for the purpose of circumventing the rules. */
	if (!p_ptr->privileged) return(FALSE);
	/* List of admins' player accounts */
	if (strcasecmp(p_ptr->accountname, "moltor")
	    && strcasecmp(p_ptr->accountname, "kurzel")
	    && strcasecmp(p_ptr->accountname, "the_sandman")
	    && strcasecmp(p_ptr->accountname, "faith")
	    && strcasecmp(p_ptr->accountname, "mikaelh")
	    && strcasecmp(p_ptr->accountname, "c. blue")) return(FALSE);
	/* Pass */
	return(TRUE);
}

/*
 * A message prefixed by a player name is sent only to that player.
 * Otherwise, it is sent to everyone.
 */
/*
 * This function is hacked *ALOT* to add extra-commands w/o
 * client change.		- Jir -
 *
 * Yeah. totally.
 *
 * Muted players can't talk now (player_type.mutedchat-- can be enabled
 * through /mute <name> and disabled through /unmute <name>).
 *				- the_sandman
 */
//#define PUNISH_WHISPER /* Swearing in whispers gets punished (if not, it still gets censored depending on target's censor_swearing setting) */
//#define PUNISH_PARTYCHAT /* Swearing in party chat gets punished (if not, it still gets censored depending on target's censor_swearing setting) */
//#define PUNISH_GUILDCHAT /* Swearing in guild chat gets punished (if not, it still gets censored depending on target's censor_swearing setting) */
/* Colour of private messages received across worlds.
   Keep consistent with util.c definition! */
#define WP_PMSG_DEFAULT_COLOUR 's'
static void player_talk_aux(int Ind, char *message) {
	int i, len, target = 0, target_raw_len = 0;
	char search[MSG_LEN], sender[MAX_CHARS];
	char message2[MSG_LEN], message_u_forge[MSG_LEN], *message_u = message_u_forge;
	player_type *p_ptr = NULL, *q_ptr;
	char *colon, *colon_u;
	char *sipd, *sipd_chatmode;

	bool rp_me = FALSE, rp_me_gen = FALSE, log = TRUE, nocolon = FALSE;
	char c_n = 'B'; /* colours of sender name and of brackets (unused atm) around this name */
#ifdef KURZEL_PK
	char c_b = 'B';
#endif
	int mycolor = 0;
	bool admin = FALSE;
	bool broadcast = FALSE;
	bool slash_command = FALSE, slash_command_msg = FALSE, slash_command_censorable = FALSE;
#ifndef ARCADE_SERVER
	bool is_public = TRUE;
#endif
	char messagelc[MSG_LEN];
#ifdef TOMENET_WORLDS
	char tmessage[MSG_LEN];		/* TEMPORARY! We will not send the name soon */
	char tmessage_u[MSG_LEN];
#endif
	int censor_punish = 0;
	bool censor, reached;


	if (!Ind) {
		console_talk_aux(message);
		return;
	}

	/* Get sender's name */
	p_ptr = Players[Ind];
	censor = p_ptr->censor_swearing;
	/* Get player name */
	strcpy(sender, p_ptr->name);
	admin = is_admin(p_ptr);

	/* Default to no search string */
	strcpy(search, "");

	/* this ain't a proper colon (must have a receipient or chat code on the left side!) */
	if (message[0] == ':') nocolon = TRUE;


	/* Look for a player's name followed by a colon */

	/* tBot's stuff */
	/* moved here to allow tbot to see fake pvt messages. -Molt */
	strncpy(last_chat_owner, sender, CNAME_LEN);
	strncpy(last_chat_line, message, MSG_LEN);
	strncpy(last_chat_account, p_ptr->accountname, ACCNAME_LEN); // <- added this for chat-AI 1000 years later :) - C. Blue
	/* do exec_lua() not here instead of in dungeon.c - mikaelh */

	/* '-:' at beginning of message sends to normal chat, cancelling special chat modes - C. Blue */
	if (strlen(message) >= 4 && message[1] == ':' && message[2] == '-' && message[3] == ':')
		message += 4;
	/* Catch this case too anyway, just for comfort if someone uses fixed macros for -: */
	else if (strlen(message) >= 2 && message[0] == '-' && message[1] == ':')
		message += 2;


	/* hack: preparse for private chat target, and if found
	   then strip chat mode prefixes to switch to private chat mode - C. Blue */
	/* Form a search string if we found a colon */
	if (strlen(message) > 4 && message[1] == ':' && message[2] != ':' &&
	    (message[0] == '!' || message[0] == '#' || message[0] == '$')
	    && (colon = strchr(message + 3, ':'))) {
		/* Copy everything up to the colon to the search string */
		strncpy(search, message + 2, colon - message - 2);
		search[colon - message - 2] = '\0';
		/* Acquire length of search string */
		len = strlen(search);
		/* Look for a recipient who matches the search string */
		if (len) {
#ifdef TOMENET_WORLDS
			struct rplist *w_player;
#endif
			/* NAME_LOOKUP_LOOSE DESPERATELY NEEDS WORK */
			target = name_lookup_loose(Ind, search, TRUE, TRUE, TRUE);

			if (target) {
				/* strip leading chat mode prefix,
				   pretend that it's a private message instead */
				message += 2;
			}
#ifdef TOMENET_WORLDS
			else if (cfg.worldd_privchat) {
				w_player = world_find_player(search, 0);
				if (w_player) {
					/* strip leading chat mode prefix,
					   pretend that it's a private message instead */
					message += 2;
				}
			}
#endif
		}

		/* erase our traces */
		search[0] = '\0';
		len = 0;
	}


	colon = strchr(message, ':');
	/* only assume targetted chat if the 'name' would acually be of acceptable length */
	if (colon && (colon - message) > NAME_LEN) {
		/* in case the player actually used double-colon, not knowing of this clever mechanism here, fix it for him ;) */
		if (colon[1] == ':') {
			i = (int) (colon - message);
			do message[i] = message[i + 1];
			while (message[i++] != '\0');
		}

		colon = NULL;
	}

	/* If there's a space BEFORE the colon, it cannot be a name */
	if (colon > message && *(colon - 1) == ' ') colon = NULL;

	/* Ignore "smileys" or URL */
	//if (colon && strchr(")(-/:", colon[1]))
	/* (C. Blue) changing colon parsing. :: becomes
	    textual :  - otherwise : stays control char */
	if (colon) {
		bool smiley = FALSE;
		/* no double-colon '::' smileys inside possible item inscriptions */
		char *bracer = strchr(message, '{');
		bool maybe_inside_inscription = bracer != NULL && bracer < colon;

		target_raw_len = colon - message;

		/* if another colon followed this one then
		   it was not meant to be a control char */
		switch (colon[1]) {
		/* accept these chars for smileys, but only if the colon is either first in the line or stands after a SPACE,
		   otherwise it is to expect that someone tried to write to party/privmsg */
		case '(':	case ')':
		case '[':	case ']':
		case '{':	case '}':
/* staircases, so cannot be used for smileys here ->		case '<':	case '>': */
		case '\\':	case '|':
		case 'p': case 'P': case 'o': case 'O':
		case 'D': case '3':
			if (message == colon || colon[-1] == ' ' || colon[-1] == '>' || /* >:) -> evil smiley */
			    ((message == colon - 1) && (colon[-1] != '!') && (colon[-1] != '#') && (colon[-1] != '%') && (colon[-1] != '$') && (colon[-1] != '+'))) /* <- party names must be at least 2 chars then */
				colon = NULL; /* the check is mostly important for '(' */
			break;
		case '-':
			if (message == colon || colon[-1] == ' ' || colon[-1] == '>' || /* here especially important: '-' often is for numbers/recall depth */
			    ((message == colon - 1) && (colon[-1] != '!') && (colon[-1] != '#') && (colon[-1] != '%') && (colon[-1] != '$') && (colon[-1] != '+'))) /* <- party names must be at least 2 chars then */
				if (!strchr("123456789", *(colon + 2))) colon = NULL;
			break;
		case '/':
			/* only accept / at the end of a chat message */
			if ('\0' == *(colon + 2)) colon = NULL;
			/* or it might be a http:// style link */
			else if ('/' == *(colon + 2)) colon = NULL;
			break;
		case ':':
			/* remove the 1st colon found, .. */

			/* always if there's no chat-target in front of the double-colon: */
			if (message == colon || colon[-1] == ' ') {
				i = (int) (colon - message);
				do message[i] = message[i + 1];
				while (message[i++] != '\0');
				colon = NULL;
				break;
			}

			/* new hack: ..but only if the previous two chars aren't  !:  (party chat),
			   and if it's appearently meant to be a smiley. */
			if ((colon - message == 1) && (colon[-1] == '!' || colon[-1] == '#'
			    || colon[-1] == '%' || colon[-1] == '$' || colon[-1] == '+'))
			switch (*(colon + 2)) {
			case '(': case ')':
			case '[': case ']':
			case '{': case '}':
			case '<': case '>':
			case '-': case '|':
			case 'p': case 'P': case 'o': case 'O':
			case 'D': case '3':
			smiley = !maybe_inside_inscription; break; }

			/* check for smiley at end of the line */
			if ((message + strlen(message) - colon >= 3) &&
			    (message + strlen(message) - colon <= 4)) // == 3 for 2-letter-smileys only.
			//if ((*(colon + 2)) && !(*(colon + 3)))
			switch (*(colon + 2)) {
			case '(': case ')':
			case '[': case ']':
			case '{': case '}':
			case '<': case '>':
			case '-': case '/':
			case '\\': case '|':
			case 'p': case 'P': case 'o': case 'O':
			case 'D': case '3':
			smiley = !maybe_inside_inscription; break; }

			if (smiley) break;

			/* Pretend colon wasn't there */
			i = (int) (colon - message);
			do message[i] = message[i + 1];
			while (message[i++] != '\0');
			colon = NULL;
			break;
		case '\0':
			/* if colon is last char in the line, it's not a priv/party msg. */
#if 0 /* disabled this 'feature', might be more convenient - C. Blue */
			colon = NULL;
#else
			if (colon[-1] != '!' && colon[-1] != '#' && colon[-1] != '%' && colon[-1] != '$' && colon[-1] != '+')
				colon = NULL;
#endif
			break;
		}
	}

	strcpy(messagelc, message);
	i = 0;
	while (messagelc[++i]) messagelc[i] = tolower(messagelc[i]);

	/* Special - shutdown command (for compatibility) */
	if (prefix(message, "@!shutdown") && admin) {
		/*world_reboot();*/
		cfg.runlevel = 0;
		shutdown_server();
		return;
	}

	/* '%:' at the beginning sends to self - mikaelh */
	if ((strlen(message) >= 2) && (message[0] == '%') && (message[1] == ':')) {
		/* prevent buffer overflow */
		message[MSG_LEN - 1 + 2 - 8] = 0;
		/* Send message to self */
		msg_format(Ind, "\377o<%%>\377w %s", message + 2);
		/* Done */
		return;
	}

	/* don't log spammy slash commands */
	if (prefix(messagelc, "/untag ")
	    || prefix(messagelc, "/dis"))
		log = FALSE;

	/* no big brother */
	//if (cfg.log_u && (!colon || message[0] != '#' || message[0] != '/')) { /* message[0] != '#' || message[0] != '/' is always true -> big brother mode - mikaelh */
	if (cfg.log_u && log &&
	    (!colon || nocolon || (message[0] == '/' && strncmp(message, "/note", 5)))) {
		/* Shorten multiple identical messages to prevent log file spam,
		   eg recall rod activation attempts. - Exclude admins. */
		if (admin)
			s_printf("[%s] %s\n", sender, message);
		else if (strcmp(message, p_ptr->last_chat_line)) {
			if (p_ptr->last_chat_line_cnt) {
				s_printf("[%s (x%d)]\n", sender, p_ptr->last_chat_line_cnt + 1);
				p_ptr->last_chat_line_cnt = 0;
			}
			s_printf("[%s] %s\n", sender, message);

			/* Keep track of repeating chat lines to avoid log file spam (slash commands like '/rec' mostly) */
			strncpy(p_ptr->last_chat_line, message, MSG_LEN - 1);
			p_ptr->last_chat_line[MSG_LEN - 1] = 0;
		} else p_ptr->last_chat_line_cnt++;
	}

	/* -- Finished pre-processing of the message far enough that it can be checked for swear words now,
	      as the only thing left to pre-process is whether it's a slash command or direct chat. -- */

	if (message[0] == '/') {
		if (!strncmp(messagelc, "/me ", 4)) rp_me = TRUE;
		else if (!strncmp(messagelc, "/me'", 4)) rp_me = rp_me_gen = TRUE;
		else if (!strncmp(messagelc, "/broadcast ", 11)) broadcast = TRUE;
		else {
			slash_command = TRUE;
			/* Is it a slash command that results in actual message readable by others?
			   A space has been cat'ed for commands that have only read-access per se */
			if (prefix(messagelc, "/sho") || prefix(messagelc, "/yell") || prefix(messagelc, "/scr")
			    || prefix(messagelc, "/sayme") || prefix(messagelc, "/sme")
			    || prefix(messagelc, "/say") || prefix(messagelc, "/s ") || !strcmp(messagelc, "/s")
			    || prefix(messagelc, "/afk") || prefix(messagelc, "/bbs "))
				slash_command_msg = TRUE;
			if ((prefix(messagelc, "/wh") && !prefix(messagelc, "/who"))
			    || prefix(messagelc, "/note ")
			    || prefix(messagelc, "/pnote ") || prefix(messagelc, "/gnote ")
			    || prefix(messagelc, "/pbbs ") || prefix(messagelc, "/gbbs ")) {
				slash_command_msg = TRUE;
#ifndef ARCADE_SERVER
				is_public = FALSE;
#endif
			}
			/* Is it a slash command that results in actual output readable by others? */
			if (prefix(messagelc, "/info ")
			    || prefix(messagelc, "/tag ") || prefix(messagelc, "/t "))
				slash_command_censorable = TRUE;
		}
	}
#ifndef ARCADE_SERVER
	if (colon) is_public = FALSE;
#endif

#ifndef ARCADE_SERVER
	if (is_public /* Only prevent spam if not in party/private chat */
	    && !slash_command /* Slash commands don't write to public chat, so they can just be /ignored by the target (eg /whisper) - /shout etc are used in the dungeon! */
	    && !admin) /* Admins are exempt. This is required anyway for spamming LUA commands, as these aren't covered by 'slash commands' checks. */
		p_ptr->msgcnt++;
	if (p_ptr->msgcnt > 12) {
		time_t last = p_ptr->msg;

		time(&p_ptr->msg);
		if (p_ptr->msg - last < 6) {
			p_ptr->spam++;
			switch (p_ptr->spam) {
			case 1:
				msg_print(Ind, "\377yPlease don't spam the server");
				break;
			case 3:
			case 4:
				msg_print(Ind, "\374\377rWarning! this behaviour is unacceptable!");
				break;
			case 5:
#if 0 /* this might be unnecessary */
				p_ptr->chp = -3;
				strcpy(p_ptr->died_from, "hypoxia");
				p_ptr->died_from_ridx = 0;
				p_ptr->spam = 1;
				p_ptr->deathblow = 0;
				player_death(Ind);
				return;
#else
				imprison(Ind, JAIL_SPAM, "talking too much.");
 #if 0 /* permanent */
				if (!p_ptr->mutedchat) {
					p_ptr->mutedchat = 1; /* just 1, so private/party/guild chat is still possible */
					acc_set_flags(p_ptr->accountname, ACC_QUIET, TRUE);
					msg_print(Ind, "\374\377rYou have been muted!");
					s_printf("SPAM_MUTE: '%s' (%s) was muted for chat spam.\n", p_ptr->name, p_ptr->accountname);
				}
 #else /* temporary */
  #define MUTEDTURN_TIME 300
				p_ptr->mutedtemp = MUTEDTURN_TIME;
				if (MUTEDTURN_TIME < 120) msg_format(Ind, "\377rYou have been muted for %s seconds.", MUTEDTURN_TIME);
				else msg_format(Ind, "\377rYou have been muted for %s minutes.", MUTEDTURN_TIME / 60);
 #endif
				s_printf("SPAM_MUTE_TEMP: '%s' (%s) was muted for chat spam for %d seconds.\n", p_ptr->name, p_ptr->accountname, MUTEDTURN_TIME);
				break;
#endif
			}
		}
		if (p_ptr->msg - last > 240 && p_ptr->spam) p_ptr->spam--;
		p_ptr->msgcnt = 0;
	}
	if (p_ptr->spam > 1 || p_ptr->mutedchat == 2) {
		/* Still allow slash commands that don't output anything readble by others.
		   (Note that this doesn't take care of LUA commands, but that's covered by
		   exempting admins from the whole spam prevention code in the first place.) */
		if (!slash_command || slash_command_msg /* || slash_command_censorable */)
			return;
	}
#endif
	if (!slash_command || slash_command_msg) process_hooks(HOOK_CHAT, "d", Ind);


	/* If we find a \372 marker, it can be either a lore-paste marker (occurs right a the start of our message to the server)
	   or a kind-diz marker (occurs later in the line).
	   Process it if it's a kind-diz marker, otherwise ignore it here (handled solemnly in msg_print() then): */
#if defined(KIND_DIZ) && defined(SERVER_ITEM_PASTE_DIZ)
	sipd_chatmode = strchr(message, ':'); /* We might not be in global chat mode but in party or guild chat, which come with prefixed !: or $: */
	if (!sipd_chatmode) sipd_chatmode = message;
	else sipd_chatmode += 2; //skip the ':' and the space that always follows it
	if ((sipd = strchr(message, '\372')) && sipd > sipd_chatmode) { /* Test if the sipd is not at the beginning, then it's kdiz (otherwise lorepaste, which is not handled here) */
		char *ckt = strchr(sipd, ','); //comma is the tval,sval separator
		int tval = atoi(sipd + 1), sval = ckt ? atoi(ckt + 1) : -1;

		if ((sflags1 & SFLG1_SIPD) && tval >= 1 && tval <= TV_MAX && sval >= 0 && sval <= 255) {
			int k_idx = lookup_kind(tval, sval);

			if (p_ptr->obj_aware[k_idx] && k_info[k_idx].text) {
				char k_temp[MSG_LEN];

 #if 1 /* Use the 'd'-line method (vs normal 'D'-lines) to crop information too trivial to spam the chat with it. */
				/* Hack-marker '\377w\377w': Discard 'd:' lines here, those are only for inspection/examination */
				strcpy(k_temp, k_text + k_info[k_idx].text);
				if ((ckt = strstr(k_temp, "\377w\377w")))
					*(ckt - 1) = 0; /* Cut off and discard d-lines via null-terminator, at the space _before_ the \377w\377w. */
				ckt = k_temp;
 #else /* Without 'd'-lines, we can at least try these basic hacks to crop unimportant info. */
				/* Skip very basic descriptions such as for ammo and ranged weapons, this is really useless in chat and hence spammy.
				    Maybe only for flavoured items actually? Details about current (2024-10-03) k_info.txt state:
				    - All weapons and armour have no diz except for Costume, so display these.
				    - Launchers and trap kits just have a line about ammo type, so skip these, or just skip the 1st line in case a 2nd line ever gets added.
				    - TV_GOLEM (52) are sort of trivial (just 'required for a golem' style), but these are rare, so why not just display them.
				    - Ammo has the first line about launcher/trapkit type, optionally 2nd about magical return, so just skip the 1st line for ammo. */
				if (tval == TV_BOW || tval == TV_TRAPKIT || is_ammo(tval))
					/* Discard the first diz line */
					ckt = strchr(ckt, '\n') + 1;
 #endif

				/* k_text is in a format not fit for here, so translate back (cut off first 3 chars, replace linefeeds with spaces) */
				if (*ckt && strlen(message) < MSG_LEN - 10
				    ) {
					char *cs;

					sipd++; /* We leave the \372 in place as marker between the two strings, before is the normal message, after is the diz text. */
					cs = sipd;

					/* Don't write '\377' in front of 'D' colour attr on purpose:
					   The \372 will get replaced on a by-player basis with either string-termination or \377 to continue */
					strcpy(cs, "D - ");
					cs += 4;

					while (*++ckt && cs - message < MSG_LEN - 1) { /* ++ckt: skip the initial space (compare D-line processing in init1.c) */
						switch (*ckt) {
						case '\377': ckt++; /* fall through */
						case '\n': continue;
						}
						*cs++ = *ckt;
					}
					*cs = 0;

				}
				/* No effective diz text availale for this item or not enough space to append a kind-diz? Discard. */
				else {
					*sipd = 0;
					sipd = NULL;
				}
			}
			/* Player isn't aware of flavoured item, or there is no diz for this item, so discard the item-diz marker */
			else {
				*sipd = 0;
				sipd = NULL;
			}
		}
		/* Server doesn't allow diz-pasting or item is invalid - just discard the item-diz marker. */
		else {
			*sipd = 0;
			sipd = NULL;
		}
	}
#endif


	/* Keep uncensored message for everyone who disabled censoring */
	strcpy(message_u, message);

	/* Check for nasty language in chat/messaging */
	if (//is_public &&
	    (!slash_command || slash_command_msg || slash_command_censorable)) {
		char *c = strchr(message, ' ');
#ifndef ENABLE_MULTILINE_CENSOR
		/* Apply censorship and its penalty and keep uncensored version for those who wish to get uncensored information */
		/* Censor and get level of punishment. (Note: This if/else isn't really needed, we could just censor the complete message always..) */
		if (!slash_command) censor_punish = handle_censor(message); /* For chat, censor the complete message. */
		/* For commands, we can skip the actual command, just in caaaase part of the command somehow mixes up with the message to false-positive-trigger the censor check oO (paranoia?) */
		else censor_punish = handle_censor(rp_me ? message + 4 : (c ? c : message));
		/* The actual handle_punish() will be called right before returning from this function, so it appears _after_
		   the censored (or uncensored, depending on the receiving player's setting) output has been displayed,
		   not now, as that would be the wrong visual order. */
#else
		if (!slash_command) censor_punish = handle_ml_censor(Ind, message); /* For chat, censor the complete message. */
		else censor_punish = handle_ml_censor(Ind, rp_me ? message + 4 : (c ? c : message));
#endif
	}

	if (slash_command) {
		do_slash_cmd(Ind, message, message_u);
		handle_punish(Ind, censor_punish);
		return;
	}


	/* -- Finished pre-processing of the message. Now it IS a message and it is to be sent to some target user/group. -- */


	/* For party/floor/guild chat mode:
	   If message is a lore paste, it might have an additional space at the beginning,
	   from client's Send_paste_msg() which adds a space between chatmode prefix and the msg.
	   This could result in a double space, which breaks the lore-paste code. Trim it to one space. */
	if (strlen(message) >= 2 && message[1] == ':' && colon && message[2] == ' ' && (message[3] == '\372' || message[3] == '\377')) {
		message[2] = message[1];
		message[1] = message[0];
		message_u[2] = message_u[1];
		message_u[1] = message_u[0];
		message++;
		message_u++;
	}

	/* Special function '!:' at beginning of message sends to own party - sorry for hack, C. Blue */
	//if ((strlen(message) >= 1) && (message[0] == ':') && (!colon) && (p_ptr->party))
	if ((strlen(message) >= 2) && (message[0] == '!') && (message[1] == ':') && (colon) && (p_ptr->party)) {
		target = p_ptr->party;

#if 1 /* No private chat for invalid accounts ? */
		if (p_ptr->inval) {
			msg_print(Ind, "\377yYour account is not valid, wait for an admin to validate it.");
			return;
		}
#endif

		/* Send message to target party */
		if (p_ptr->mutedchat < 2) {
#ifdef GROUP_CHAT_NOCLUTTER
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - 10 - strlen(sender)] = 0;
			message_u[MSG_LEN - 1 - 10 - strlen(sender)] = 0;
			party_msg_ignoring2(Ind, target, format("\375\377%c[(P) %s] %s", COLOUR_CHAT_PARTY, sender, message + 2), format("\375\377%c[(P) %s] %s", COLOUR_CHAT_PARTY, sender, message_u + 2));
#else
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - 7 - strlen(sender) - strlen(parties[target].name)] = 0;
			message_u[MSG_LEN - 1 - 7 - strlen(sender) - strlen(parties[target].name)] = 0;
			party_msg_ignoring2(Ind, target, format("\375\377%c[%s:%s] %s", COLOUR_CHAT_PARTY, parties[target].name, sender, message + 2), format("\375\377%c[%s:%s] %s", COLOUR_CHAT_PARTY, parties[target].name, sender, message_u + 2));
#endif
		}

		/* Done */
#ifdef PUNISH_PARTYCHAT
		handle_punish(Ind, censor_punish);
#endif
		return;
	}

	/* '#:' at beginning of message sends to dungeon level - C. Blue */
	if ((strlen(message) >= 2) && (message[0] == '#') && (message[1] == ':') && (colon)) {
#if 1 /* No private chat for invalid accounts ? */
		if (p_ptr->inval) {
			msg_print(Ind, "\377yYour account is not valid, wait for an admin to validate it.");
			return;
		}
#endif

		if (!p_ptr->wpos.wz) {
#if 0
			msg_print(Ind, "You aren't in a dungeon or tower.");
#else /* Darkie's idea: */
			if (*(message + 2)) {
				/* prevent buffer overflow */
				message[MSG_LEN - 1 + 2 - strlen(p_ptr->name) - 9] = 0;
				message_u[MSG_LEN - 1 + 2 - strlen(p_ptr->name) - 9] = 0;
				msg_print_near2(Ind, format("\377%c%^s says: %s", COLOUR_CHAT, p_ptr->name, message + 2), format("\377%c%^s says: %s", COLOUR_CHAT, p_ptr->name, message_u + 2));
				msg_format(Ind, "\377%cYou say: %s", COLOUR_CHAT, censor ? message + 2 : message_u + 2);
				handle_punish(Ind, censor_punish);
			} else {
				msg_format_near(Ind, "\377%c%s clears %s throat.", COLOUR_CHAT, p_ptr->name, p_ptr->male ? "his" : "her");
				msg_format(Ind, "\377%cYou clear your throat.", COLOUR_CHAT);
			}
#endif
			return;
		}

		/* Send message to target floor */
		if (p_ptr->mutedchat < 2) {
			/* prevent buffer overflow */
			message[MSG_LEN - 1 + 2 - strlen(sender) - 6] = 0;
			message_u[MSG_LEN - 1 + 2 - strlen(sender) - 6] = 0;
			floor_msg_ignoring2(Ind, &p_ptr->wpos, format("\375\377%c[%s] %s", COLOUR_CHAT_LEVEL, sender, message + 2), format("\375\377%c[%s] %s", COLOUR_CHAT_LEVEL, sender, message_u + 2));
			handle_punish(Ind, censor_punish);
		}

		/* Done */
		return;
	}

	/* '+:' at beginning of message sends reply to last player who sent us a private msg  - C. Blue */
	if ((strlen(message) >= 2) && (message[0] == '+') && (message[1] == ':') && colon) {
#if 1 /* No private chat for invalid accounts ? */
		if (p_ptr->inval) {
			msg_print(Ind, "\377yYour account is not valid, wait for an admin to validate it.");
			return;
		}
#endif

		/* Haven't received an initial private msg yet? */
		if (!strlen(p_ptr->reply_name)) {
			msg_print(Ind, "You haven't received any private message to reply to yet.");
			return;
		}

		if (p_ptr->mutedchat == 2) return;

		/* Forge private message */
		strcpy(message2, p_ptr->reply_name);
		strcat(message2, message + 1);
		strcpy(message, message2);
		colon = message + strlen(p_ptr->reply_name);

		strcpy(message2, p_ptr->reply_name);
		strcat(message2, message_u + 1);
		strcpy(message_u, message2);

		/* Continue with forged private message */
	}

	/* '$:' at beginning of message sends to guild - C. Blue */
	if ((strlen(message) >= 2) && (message[0] == '$') && (message[1] == ':') && (colon) && (p_ptr->guild)) {
#if 1 /* No private chat for invalid accounts ? */
		if (p_ptr->inval) {
			msg_print(Ind, "\377yYour account is not valid, wait for an admin to validate it.");
			return;
		}
#endif

		/* Send message to guild party */
		if (p_ptr->mutedchat < 2) {
#ifdef GROUP_CHAT_NOCLUTTER
			/* prevent buffer overflow */
			message[MSG_LEN - 1 + 2 - strlen(sender) - 16] = 0;
			message_u[MSG_LEN - 1 + 2 - strlen(sender) - 16] = 0;
			guild_msg_ignoring2(Ind, p_ptr->guild,
			    format("\375\377y[\377%c(G) %s\377y]\377%c %s", COLOUR_CHAT_GUILD, sender, COLOUR_CHAT_GUILD, message + 2),
			    format("\375\377y[\377%c(G) %s\377y]\377%c %s", COLOUR_CHAT_GUILD, sender, COLOUR_CHAT_GUILD, message_u + 2));
#else
			/* prevent buffer overflow */
			message[MSG_LEN - 1 + 2 - strlen(sender) - strlen(guilds[p_ptr->guild].name) - 18] = 0;
			message_u[MSG_LEN - 1 + 2 - strlen(sender) - strlen(guilds[p_ptr->guild].name) - 18] = 0;
			guild_msg_ignoring2(Ind, p_ptr->guild,
			    format("\375\377y[\377%c%s\377y:\377%c%s\377y]\377%c %s", COLOUR_CHAT_GUILD, guilds[p_ptr->guild].name, COLOUR_CHAT_GUILD, sender, COLOUR_CHAT_GUILD, message + 2),
			    format("\375\377y[\377%c%s\377y:\377%c%s\377y]\377%c %s", COLOUR_CHAT_GUILD, guilds[p_ptr->guild].name, COLOUR_CHAT_GUILD, sender, COLOUR_CHAT_GUILD, message + 2));
#endif
		}

		/* Done */
#ifdef PUNISH_GUILDCHAT
		handle_punish(Ind, censor_punish);
#endif
		return;
	}


	/* Form a search string if we found a colon */
	if (colon) {
		if (p_ptr->mutedchat == 2) return;
#if 1 /* No private chat for invalid accounts ? */
		if (p_ptr->inval) {
			msg_print(Ind, "\377yYour account is not valid, wait for an admin to validate it.");
			return;
		}
#endif
		/* Copy everything up to the colon to the search string */
		strncpy(search, message, colon - message);

		/* Add a trailing NULL */
		search[colon - message] = '\0';
	} else if (p_ptr->mutedchat || p_ptr->mutedtemp) return;

	/* From here on we need colon_u */
	if (colon) colon_u = message_u + (colon - message);
	else colon_u = NULL;

	/* Acquire length of search string */
	len = strlen(search);

	/* Look for a recipient who matches the search string */
	if (len) {
#ifdef TOMENET_WORLDS
		struct rplist *w_player;
#endif

		/* NAME_LOOKUP_LOOSE DESPERATELY NEEDS WORK */
		target = name_lookup_loose(Ind, search, TRUE, TRUE, TRUE);
#ifdef TOMENET_WORLDS
		if (!target && cfg.worldd_privchat) {
			w_player = world_find_player(search, 0);
			if (w_player) {
				/* prevent buffer overflow */
				message[MSG_LEN - strlen(p_ptr->name) - 7 - 8 - strlen(w_player->name) + target_raw_len] = 0;//8 are world server tax
				world_pmsg_send(p_ptr->id, p_ptr->name, w_player->name, colon + 1);
				msg_format(Ind, "\375\377%c[%s:%s] %s", WP_PMSG_DEFAULT_COLOUR, p_ptr->name, w_player->name, colon + 1);

				/* hack: assume that the target player will become the
				   one we want to 'reply' to, afterwards, if we don't
				   have a reply-to target yet. */
				if (!strlen(p_ptr->reply_name)) strcpy(p_ptr->reply_name, w_player->name);

 #ifdef PUNISH_WHISPER
				handle_punish(Ind, censor_punish);
 #endif
				return;
			}
		}
#endif

		/* Move colon pointer forward to next word */
/* no, this isn't needed and actually has annoying effects if you try to write smileys:
		while (*colon && (isspace(*colon) || *colon == ':')) colon++; */
/* instead, this is sufficient: */
		if (colon) {
			colon++;
			colon_u++;
		}

		/* lookup failed */
		if (!target) {
			/* Bounce message to player who sent it */
			//msg_format(Ind, "(no receipient for: %s)", colon);
#ifndef ARCADE_SERVER
			msg_print(Ind, "(Cannot match desired recipient)");
#endif
			/* Allow fake private msgs to be intercepted - Molt */
			exec_lua(0, "chat_handler()");

			/* Give up */
			return;
		}
	}

	/* Send to appropriate player */
	if (len && target > 0) {
		/* Set target player */
		q_ptr = Players[target];

		if (q_ptr->ignoring_chat == 2 && (!q_ptr->party || q_ptr->party != p_ptr->party)) {
			msg_print(Ind, "(That player is currently in *private* mode.)");
			return;
		}
		else if (!check_ignore(target, Ind)) {
			/* Remember sender, to be able to reply to him quickly */
#if 0
			strcpy(q_ptr->reply_name, sender);
#else /* use his account name instead, since it's possible now */
			if (!strcmp(sender, p_ptr->name)) strcpy(q_ptr->reply_name, p_ptr->accountname);
			else strcpy(q_ptr->reply_name, sender);
#endif

			/* prevent buffer overflow */
			message[MSG_LEN - strlen(sender) - 7 - strlen(q_ptr->name) + target_raw_len] = 0;
			message_u[MSG_LEN - strlen(sender) - 7 - strlen(q_ptr->name) + target_raw_len] = 0;

			/* Send message to target */
			msg_format(target, "\375\377g[%s:%s] %s", sender, q_ptr->name, q_ptr->censor_swearing ? colon : colon_u);
			if ((q_ptr->page_on_privmsg ||
			    (q_ptr->page_on_afk_privmsg && q_ptr->afk)) &&
			    q_ptr->paging == 0)
				q_ptr->paging = 1;

			/* Also send back to sender */
			if (target != Ind)
				msg_format(Ind, "\375\377g[%s:%s] %s", sender, q_ptr->name, censor ? colon : colon_u);

			/* Only display this message once now - mikaelh */
			if (q_ptr->afk && !player_list_find(p_ptr->afk_noticed, q_ptr->id)) {
				msg_format(Ind, "\377o%s seems to be Away From Keyboard.", q_ptr->name);
				player_list_add(&p_ptr->afk_noticed, q_ptr->id);
			}

#if 0
			/* hack: assume that the target player will become the
			   one we want to 'reply' to, afterwards, if we don't
			   have a reply-to target yet. */
			if (!p_ptr->reply_name || !strlen(p_ptr->reply_name))
				strcpy(p_ptr->reply_name, q_ptr->name);
#else
			/* hack: assume that the target player will become the
			   one we want to 'reply' to, afterwards.
			   This might get somewhat messy if we're privchatting
			   to two players at the same time, but so would
			   probably the other variant above. That one stays
			   true to the '+:' definition given in the guide though,
			   while this one diverges a bit. */
 #if 0
			strcpy(p_ptr->reply_name, q_ptr->name);
 #else /* use his account name instead, since it's possible now */
			strcpy(p_ptr->reply_name, q_ptr->accountname);
 #endif
#endif

			//exec_lua(0, "chat_handler()");
#ifdef PUNISH_WHISPER
			handle_punish(Ind, censor_punish);
#endif
			return;
		} else {
#if 1 /* keep consistent with paging */
			/* Tell the sender */
			msg_print(Ind, "(That player is currently ignoring you.)");
#endif
			return;
		}
	}

	/* Send to appropriate party - only available to admins
	   Hm, maybe this stuff could just be deleted, could maybe unnecessarily interfere with private messaging, if player has similar name as a party?.. */
	if (len && target < 0) {
		if (!admin) {
			msg_print(Ind, "Use !: chat prefix or hit TAB key in the chat prompt to talk to your party.");
			return;
		}
#ifdef GROUP_CHAT_NOCLUTTER
		/* prevent buffer overflow */
		message[MSG_LEN - strlen(sender) - 10] = 0;
		/* Send message to target party */
		party_msg_format_ignoring(Ind, 0 - target, "\375\377%c[(P) %s] %s", COLOUR_CHAT_PARTY, sender, colon);
#else
		/* prevent buffer overflow */
		message[MSG_LEN - 7 - strlen(sender) - strlen(parties[0 - target].name) + target_raw_len] = 0;
		party_msg_format_ignoring(Ind, 0 - target, "\375\377%c[%s:%s] %s", COLOUR_CHAT_PARTY, parties[target].name, sender, colon);
#endif

		/* Also send back to sender if not in that party */
		if (!player_in_party(0 - target, Ind)) {
			msg_format(Ind, "\375\377%c[%s:%s] %s",
			    COLOUR_CHAT_PARTY, parties[0 - target].name, sender, colon);
		}

		exec_lua(0, "chat_handler()");

		/* Done */
#ifdef PUNISH_PARTYCHAT
		handle_punish(Ind, censor_punish);
#endif
		return;
	}

#if 0 /* Allow "}c" name colouring in chat? */
	if (strlen(message) > 1) mycolor = (prefix(message, "}") && (color_char_to_attr(*(message + 1)) != -1)) ? 2 : 0;
#else
	mycolor = 0;
#endif

	if (!Ind) c_n = 'y';
	/* Disabled this for now to avoid confusion. */
	else if (mycolor) c_n = *(message + 1);
	else {
		/* Dungeon Master / Dungeon Wizard have their own colour now :) */
		if (is_admin(p_ptr)) c_n = 'b';
		else if (p_ptr->total_winner) c_n = 'v';
		else if (p_ptr->ghost) c_n = 'r';
		else if (p_ptr->mode & MODE_EVERLASTING) c_n = 'B';
		else if (p_ptr->mode & MODE_PVP) c_n = COLOUR_MODE_PVP;
		else if (p_ptr->mode & MODE_SOLO) c_n = 's';
		else if (p_ptr->mode & MODE_NO_GHOST) c_n = 'D';
		else if (p_ptr->mode & MODE_HARD) c_n = 's';//deprecated
		else c_n = 'W'; /* normal mode */

		/* Color the brackets of some players... (Enabled for PK) */
#ifdef KURZEL_PK
		if ((p_ptr->mode & MODE_HARD) && (p_ptr->mode & MODE_NO_GHOST))
			c_b = 'r'; /* hellish mode */
		else if (p_ptr->pkill & PKILL_SET) c_b = 'R'; //KURZEL_PK
		else c_b = c_n;
#endif
	}

	/* Admins have exclusive colour - the_sandman */
	if (c_n == 'b' && !admin) return;
#ifdef KURZEL_PK
	if (admin) c_b = 'b';
#endif

#ifdef TOMENET_WORLDS
	if (broadcast) {
		/* prevent buffer overflow */
		message[MSG_LEN - 1 - strlen(sender) - 12 + 11] = 0;
		snprintf(tmessage, sizeof(tmessage), "\375\377r[\377%c%s\377r]\377%c %s", c_n, sender, COLOUR_CHAT, message + 11);
		message_u[MSG_LEN - 1 - strlen(sender) - 12 + 11] = 0;
		snprintf(tmessage_u, sizeof(tmessage_u), "\375\377r[\377%c%s\377r]\377%c %s", c_n, sender, COLOUR_CHAT, message_u + 11);
	} else if (!rp_me) {
 #ifndef KURZEL_PK
		/* prevent buffer overflow */
		message[MSG_LEN - 1 - strlen(sender) - 8 + mycolor] = 0;
		message_u[MSG_LEN - 1 - strlen(sender) - 8 + mycolor] = 0;

		snprintf(tmessage, sizeof(tmessage), "\375\377%c[%s]\377%c %s", c_n, sender, COLOUR_CHAT, message + mycolor);
		snprintf(tmessage_u, sizeof(tmessage_u), "\375\377%c[%s]\377%c %s", c_n, sender, COLOUR_CHAT, message_u + mycolor);
 #else
		/* prevent buffer overflow */
		message[MSG_LEN - 1 - strlen(sender) - 12 + mycolor] = 0;
		message_u[MSG_LEN - 1 - strlen(sender) - 12 + mycolor] = 0;

		snprintf(tmessage, sizeof(tmessage), "\375\377%c[\377%c%s\377%c]\377%c %s", c_b, c_n, sender, c_b, COLOUR_CHAT, message + mycolor);
		snprintf(tmessage_u, sizeof(tmessage_u), "\375\377%c[\377%c%s\377%c]\377%c %s", c_b, c_n, sender, c_b, COLOUR_CHAT, message_u + mycolor);
 #endif
	} else {
		/* Why not... */
		if (strlen(message) > 4) mycolor = (prefix(&message[4], "}") && (color_char_to_attr(*(message + 5)) != -1)) ? 2 : 0;
		else return;
		if (mycolor) c_n = message[5];

		if (rp_me_gen) {
 #ifndef KURZEL_PK
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 5 + 4 + mycolor] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 5 + 4 + mycolor] = 0;

			snprintf(tmessage, sizeof(tmessage), "\375\377%c[%s%s]", c_n, sender, message + 3 + mycolor);
			snprintf(tmessage_u, sizeof(tmessage_u), "\375\377%c[%s%s]", c_n, sender, message_u + 3 + mycolor);
 #else
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 9 + 4 + mycolor] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 9 + 4 + mycolor] = 0;

			snprintf(tmessage, sizeof(tmessage), "\375\377%c[\377%c%s%s\377%c]", c_b, c_n, sender, message + 3 + mycolor, c_b);
			snprintf(tmessage_u, sizeof(tmessage_u), "\375\377%c[\377%c%s%s\377%c]", c_b, c_n, sender, message_u + 3 + mycolor, c_b);
 #endif
		} else {
 #ifndef KURZEL_PK
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 6 + 4 + mycolor] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 6 + 4 + mycolor] = 0;

			snprintf(tmessage, sizeof(tmessage), "\375\377%c[%s %s]", c_n, sender, message + 4 + mycolor);
			snprintf(tmessage_u, sizeof(tmessage_u), "\375\377%c[%s %s]", c_n, sender, message_u + 4 + mycolor);
 #else
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 10 + 4 + mycolor] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 10 + 4 + mycolor] = 0;

			snprintf(tmessage, sizeof(tmessage), "\375\377%c[\377%c%s %s\377%c]", c_b, c_n, sender, message + 4 + mycolor, c_b);
			snprintf(tmessage_u, sizeof(tmessage_u), "\375\377%c[\377%c%s %s\377%c]", c_b, c_n, sender, message_u + 4 + mycolor, c_b);
 #endif
		}
	}

	/* worldd_pubchat decides if we broadcast all our chat out there or not. */
	/* in case privchat wasn't handled above (because it's disabled),
	   exempt it here so we only process real chat/broadcasts */
	if (!(!cfg.worldd_privchat && len && target != 0)) {
		if (broadcast && cfg.worldd_broadcast) {
			world_chat(0, tmessage); /* can't ignore */
		} else if (!broadcast && cfg.worldd_pubchat
		    && !p_ptr->limit_chat) { /* Actually never forward chat from players that have limit_chat on, because people on IRC or other servers might try to reply but the player cannot see it. */
			world_chat(p_ptr->id, tmessage);
		}
	}

	/* Send to everyone */
	reached = FALSE;
	for (i = 1; i <= NumPlayers; i++) {
		q_ptr = Players[i];

		if (!admin) {
			if (check_ignore(i, Ind)) continue;
			if (q_ptr->ignoring_chat) continue;
			if (!broadcast && (p_ptr->limit_chat || q_ptr->limit_chat) &&
			    !inarea(&p_ptr->wpos, &q_ptr->wpos)) continue;
		}
		reached = TRUE;
		msg_print(i, q_ptr->censor_swearing ? tmessage : tmessage_u);
	}
	if (!reached && p_ptr->limit_chat) msg_print(Ind, "(Nobody could hear you. Note that you have limit_chat enabled in =2 .)");

#else /* in case TOMENET_WORLDS is not defined: */

	/* Send to everyone */
	reached = FALSE;
	for (i = 1; i <= NumPlayers; i++) {
		q_ptr = Players[i];

		if (!admin) {
			if (check_ignore(i, Ind)) continue;
			if (q_ptr->ignoring_chat) continue;
			if (!broadcast && (p_ptr->limit_chat || q_ptr->limit_chat) &&
			    !inarea(&p_ptr->wpos, &q_ptr->wpos)) continue;
		}

		/* Send message */
		reached = TRUE;
		if (broadcast) {
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 12 + 11] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 12 + 11] = 0;
			msg_format(i, "\375\377r[\377%c%s\377r]\377%c %s", c_n, sender, COLOUR_CHAT, q_ptr->censor_swearing ? message + 11 : message_u + 11);
		} else if (!rp_me) {
 #ifndef KURZEL_PK
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 8 + mycolor] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 8 + mycolor] = 0;

			msg_format(i, "\375\377%c[%s]\377%c %s", c_n, sender, COLOUR_CHAT, q_ptr->censor_swearing ? message + mycolor : message_u + mycolor);
 #else
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 12 + mycolor] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 12 + mycolor] = 0;

			msg_format(i, "\375\377%c[\377%c%s\377%c]\377%c %s", c_b, c_n, sender, c_b, COLOUR_CHAT, q_ptr->censor_swearing ? message + mycolor : message_u + mycolor);
 #endif
			/* msg_format(i, "\375\377%c[%s] %s", Ind ? 'B' : 'y', sender, message); */
		}
		else {
			/* prevent buffer overflow */
			message[MSG_LEN - 1 - strlen(sender) - 1 + 4] = 0;
			message_u[MSG_LEN - 1 - strlen(sender) - 1 + 4] = 0;
			if (rp_me_gen) msg_format(i, "%s%s", sender, q_ptr->censor_swearing ? message + 3 : message_u + 3);
			else msg_format(i, "%s %s", sender, q_ptr->censor_swearing ? message + 4 : message_u + 4);
		}
	}
	if (!reached && p_ptr->limit_chat) msg_print(Ind, "(Nobody could hear you. Note that you have limit_chat enabled in =2 .)");
#endif

	p_ptr->warning_chat = 1;

	/* go to jail as a visitor */
	if (my_strcasestr(message, "please jail me")) imprison(Ind, JAIL_VISIT, "no reason");
	if (my_strcasestr(message, "i want to go to jail")) imprison(Ind, JAIL_VISIT, "no reason");
	if (my_strcasestr(message, "i would like to go to jail")) imprison(Ind, JAIL_VISIT, "no reason");
	if (my_strcasestr(message, "i'd like to go to jail")) imprison(Ind, JAIL_VISIT, "no reason");

	exec_lua(0, "chat_handler()");
	handle_punish(Ind, censor_punish);
}
/* Console talk is automatically sent by 'Server Admin' which is treated as an admin */
static void console_talk_aux(char *message) {
	cptr sender = "Server Admin";
	bool rp_me = FALSE, rp_me_gen = FALSE, log = TRUE;
	char c_n = 'y'; /* colours of sender name and of brackets (unused atm) around this name */
#ifdef KURZEL_PK
	char c_b = 'y';
#endif
	bool broadcast = FALSE;

#ifdef TOMENET_WORLDS
	char tmessage[MSG_LEN];		/* TEMPORARY! We will not send the name soon */
#endif

	/* tBot's stuff */
	/* moved here to allow tbot to see fake pvt messages. -Molt */
	strncpy(last_chat_owner, sender, CNAME_LEN);
	strncpy(last_chat_line, message, MSG_LEN);
	strncpy(last_chat_account, "", ACCNAME_LEN); // <- added this for chat-AI 1000 years later :) - C. Blue

	/* no big brother */
	if (cfg.log_u && log) s_printf("[%s] %s\n", sender, message);

	/* Special - shutdown command (for compatibility) */
	if (prefix(message, "@!shutdown")) {
		/*world_reboot();*/
		cfg.runlevel = 0;
		shutdown_server();
		return;
	}

	if (message[0] == '/') {
		if (!strncmp(message, "/me ", 4)) rp_me = TRUE;
		if (!strncmp(message, "/me'", 4)) rp_me = rp_me_gen = TRUE;
		else if (!strncmp(message, "/broadcast ", 11)) broadcast = TRUE;
		else return;
	}

#ifdef TOMENET_WORLDS
	if (broadcast) {
		snprintf(tmessage, sizeof(tmessage), "\375\377r[\377%c%s\377r]\377%c %s", c_n, sender, COLOUR_CHAT, message + 11);
	} else if (!rp_me) {
 #ifndef KURZEL_PK
		snprintf(tmessage, sizeof(tmessage), "\375\377%c[%s]\377%c %s", c_n, sender, COLOUR_CHAT, message);
 #else
		snprintf(tmessage, sizeof(tmessage), "\375\377%c[\377%c%s\377%c]\377%c %s", c_b, c_n, sender, c_b, COLOUR_CHAT, message);
 #endif
	} else {
		if (rp_me_gen) {
 #ifndef KURZEL_PK
			snprintf(tmessage, sizeof(tmessage), "\375\377%c[%s%s]", c_n, sender, message + 3);
 #else
			snprintf(tmessage, sizeof(tmessage), "\375\377%c[\377%c%s%s\377%c]", c_b, c_n, sender, message + 3, c_b);
 #endif
		} else {
 #ifndef KURZEL_PK
			snprintf(tmessage, sizeof(tmessage), "\375\377%c[%s %s]", c_n, sender, message + 4);
 #else
			snprintf(tmessage, sizeof(tmessage), "\375\377%c[\377%c%s %s\377%c]", c_b, c_n, sender, message + 4, c_b);
 #endif
		}
	}

#else
	/* Send to everyone */
	for (int i = 1; i <= NumPlayers; i++) {
		/* Send message */
		if (broadcast) {
			msg_format(i, "\375\377r[\377%c%s\377r]\377%c %s", c_n, sender, COLOUR_CHAT, message + 11);
		} else if (!rp_me) {
 #ifndef KURZEL_PK
			msg_format(i, "\375\377%c[%s]\377%c %s", c_n, sender, COLOUR_CHAT, message);
 #else
			msg_format(i, "\375\377%c[\377%c%s\377%c]\377%c %s", c_b, c_n, sender, c_b, COLOUR_CHAT, message);
 #endif
		} else msg_format(i, "%s %s", sender, message + 4);
	}
#endif
	exec_lua(0, "chat_handler()");
}

/* toggle AFK mode off if it's currently on, also reset idle time counter for in-game character.
   required for cloaking mode! - C. Blue */
void un_afk_idle(int Ind) {
	Players[Ind]->idle_char = 0;
	if (Players[Ind]->afk && !(Players[Ind]->admin_dm && cfg.secret_dungeon_master)) toggle_afk(Ind, "");
	stop_cloaking(Ind);
	if (Players[Ind]->muted_when_idle) Send_idle(Ind, FALSE);
}

/*
 * toggle AFK mode on/off.	- Jir -
 */
//#define ALLOW_AFK_COLOURCODES
#ifdef ALLOW_AFK_COLOURCODES
void toggle_afk(int Ind, char *msg0)
#else
void toggle_afk(int Ind, char *msg)
#endif
{
	player_type *p_ptr = Players[Ind];
	char afk[MAX_CHARS_WIDE];
	int i = 0;
#ifdef ALLOW_AFK_COLOURCODES
	char msg[MAX_CHARS_WIDE], *m = msg0;

	/* strip colour codes and cap message at len 60 */
	while ((*m) && i < 60) {
		if (*m == '\377') {
			m++;
			if (*m == '\377') {
				msg[i++] = '{';
			}
		} else msg[i++] = *m;
		m++;
	}
	msg[i] = 0;
#endif

	/* don't go un-AFK from auto-retaliation */
	if (p_ptr->afk && p_ptr->auto_retaliaty) return;

	/* don't go AFK while shooting-till-kill (target dummy, maybe ironwing ;)) */
	if (!p_ptr->afk && p_ptr->shooting_till_kill) return;

	strcpy(afk, "");

	if (p_ptr->afk && !msg[0]) {
		if (strlen(p_ptr->afk_msg) == 0)
			msg_print(Ind, "AFK mode is turned \377GOFF\377w.");
		else
			msg_format(Ind, "AFK mode is turned \377GOFF\377w. (%s\377w)", p_ptr->afk_msg);
		if (!p_ptr->admin_dm) {
			if (strlen(p_ptr->afk_msg) == 0)
				snprintf(afk, sizeof(afk) - 1, "\377%c%s has returned from AFK.", COLOUR_AFK, p_ptr->name);
			else
				snprintf(afk, sizeof(afk) - 1, "\377%c%s has returned from AFK. (%s\377%c)", COLOUR_AFK, p_ptr->name, p_ptr->afk_msg, COLOUR_AFK);
		}
		p_ptr->afk = FALSE;
		if (p_ptr->muted_when_idle) Send_idle(Ind, FALSE);

		/* Clear everyone's afk_noticed */
		for (i = 1; i <= NumPlayers; i++)
			player_list_del(&Players[i]->afk_noticed, p_ptr->id);
	} else {
		/* hack: lose health tracker so we actually get to see the 'AFK'
		   (for example we might've attacked the target dummy before). */
		health_track(Ind, 0);

		/* stop every major action */
		disturb(Ind, 1, 1); /* ,1) = keep resting! */

		strncpy(p_ptr->afk_msg, msg, MAX_CHARS - 1);
		p_ptr->afk_msg[MAX_CHARS - 1] = '\0';

		if (strlen(p_ptr->afk_msg) == 0)
			msg_print(Ind, "AFK mode is turned \377rON\377w.");
		else
			msg_format(Ind, "AFK mode is turned \377rON\377w. (%s\377w)", p_ptr->afk_msg);
		if (!p_ptr->admin_dm) {
			if (strlen(p_ptr->afk_msg) == 0)
				snprintf(afk, sizeof(afk) - 1, "\377%c%s seems to be AFK now.", COLOUR_AFK, p_ptr->name);
			else
				snprintf(afk, sizeof(afk) - 1, "\377%c%s seems to be AFK now. (%s\377%c)", COLOUR_AFK, p_ptr->name, p_ptr->afk_msg, COLOUR_AFK);
		}
		p_ptr->afk = TRUE;
		if (p_ptr->mute_when_idle && !p_ptr->muted_when_idle && istown(&p_ptr->wpos)) Send_idle(Ind, TRUE);

#if 0 /* not anymore */
		/* actually a hint for newbie rogues couldn't hurt */
		if (p_ptr->tim_blacklist)
			msg_print(Ind, "\376\377yNote: Your blacklist timer won't decrease while AFK.");
#endif

		/* still too many starvations, so give a warning - C. Blue */
		if (p_ptr->food < PY_FOOD_ALERT) {
			p_ptr->paging = 6; /* add some beeps, too - mikaelh */
			msg_print(Ind, "\374\377RWARNING: Going AFK while hungry or weak can result in starvation! Eat first!");
		}

		/* Since Mark's mimic died in front of Minas XBM due to this.. - C. Blue */
		if (p_ptr->tim_wraith) {
			p_ptr->paging = 6;
			msg_print(Ind, "\374\377RWARNING: Going AFK in wraithform is very dangerous because wraithform impairs auto-retaliation!");
		}
	}

	/* Replaced msg_broadcast by this, to allow /ignore and /ic of people's afk-status changes */
	/* Tell every player */
	for (i = 1; i <= NumPlayers; i++) {
		/* Skip disconnected players */
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, Ind)) continue;
		if (Players[i]->no_afk_msg) continue;
		if (Players[i]->ignoring_chat && !(p_ptr->party && player_in_party(p_ptr->party, i))) continue;

		/* Skip himself */
		if (i == Ind) continue;

		/* Tell this one */
		msg_print(i, afk);
	}

	p_ptr->redraw |= PR_EXTRA;
	redraw_stuff(Ind);
	return;
}

/*
 * A player has sent a message to the rest of the world.
 *
 * Parse it and send to everyone or to only the person(s) he requested.
 *
 * Note that more than one message may get sent at once, seperated by
 * tabs ('\t').  Thus, this function splits them and calls
 * "player_talk_aux" to do the dirty work.
 */
void player_talk(int Ind, char *message) {
	char *cur, *next;

	/* Start at the beginning */
	cur = message;

	/* Process until out of messages */
	while (cur) {
		/* Find the next tab */
		next = strchr(cur, '\t');

		/* Stop out the tab */
		if (next) {
			/* Replace with \0 */
			*next = '\0';
		}

		/* Process this message */
		player_talk_aux(Ind, cur);

		/* Move to the next one */
		if (next) {
			/* One step past the \0 */
			cur = next + 1;
		} else {
			/* No more message */
			cur = NULL;
		}
	}
}


/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch) {
	switch (ch) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'A':
	case 'E':
	case 'I':
	case 'O':
	case 'U':
	return(TRUE);
	}

	return(FALSE);
}

/*
 * Look up a player/party name, allowing abbreviations.  - Jir -
 * (looking up party name this way can be rather annoying though)
 *
 * returns 0 if not found/error(, minus value if party.)
 *
 * if 'party' is TRUE, party name is also looked up.
 * This is only used for 3 things:
 * 1) Declaring hostility to a whole party
 * 2) Ignoring a whole party
 * 3) Sending a message to a party (only possible from outside for admins)
 * NOTE: Now that we have guilds too, this might be deprecated kind of? Or should be extended in all 3 points for guilds too?..hmm
 * Note: Will fail if a non-admin tries to address an admin and requirements for that aren't met.
 */
int name_lookup_loose(int Ind, cptr name, u16b party, bool include_account_names, bool quiet) {
	int i, j, len, target = 0;
	player_type *q_ptr, *p_ptr;
	cptr problem = "";
	bool party_online;

	p_ptr = Players[Ind];

	/* don't waste time */
	if (p_ptr == (player_type*)NULL) return(0);

	/* Acquire length of search string */
	len = strlen(name);

	/* Look for a recipient who matches the search string */
	if (len) {
		/* Check players */
		for (i = 1; i <= NumPlayers; i++) {
			/* Check this one */
			q_ptr = Players[i];

			/* Skip if disconnected */
			if (q_ptr->conn == NOT_CONNECTED) continue;

			/* let admins chat */
			if (q_ptr->admin_dm && !q_ptr->admin_dm_chat && !is_admin(p_ptr) && !may_address_dm(p_ptr)) continue;

			/* Check name */
			if (!strncasecmp(q_ptr->name, name, len)) {
				/* Set target if not set already */
				if (!target) {
					target = i;
				} else {
					/* Matching too many people */
					problem = "players";
				}

				/* Check for exact match */
				if (len == (int)strlen(q_ptr->name)) {
					/* Never a problem */
					target = i;
					problem = "";

					/* Finished looking */
					break;
				}
			}
		}

		/* Then check accounts */
		if (include_account_names && !target) for (i = 1; i <= NumPlayers; i++) {
			/* Check this one */
			q_ptr = Players[i];

			/* Skip if disconnected */
			if (q_ptr->conn == NOT_CONNECTED) continue;

			/* let admins chat */
			if (q_ptr->admin_dm && !q_ptr->admin_dm_chat && !is_admin(p_ptr) && !may_address_dm(p_ptr)) continue;

			/* Check name */
			if (!strncasecmp(q_ptr->accountname, name, len)) {
				/* Set target if not set already */
				if (!target) {
					target = i;
				} else {
					/* Matching too many people */
					problem = "players";
				}

				/* Check for exact match */
				if (len == (int)strlen(q_ptr->accountname)) {
					/* Never a problem */
					target = i;
					problem = "";

					/* Finished looking */
					break;
				}
			}
		}

		/* Check parties */
		if (party && !target) {
			for (i = 1; i < MAX_PARTIES; i++) {
				/* Skip if empty */
				if (!parties[i].members) continue;

				/* Check that the party has players online - mikaelh */
				party_online = FALSE;
				for (j = 1; j <= NumPlayers; j++) {
					/* Check this one */
					q_ptr = Players[j];

					/* Skip if disconnected */
					if (q_ptr->conn == NOT_CONNECTED) continue;

					/* Check if the player belongs to this party */
					if (q_ptr->party == i) {
						party_online = TRUE;
						break;
					}
				}
				if (!party_online) continue;

				/* Check name */
				if (!strncasecmp(parties[i].name, name, len)) {
					/* Set target if not set already */
					if (!target) {
						target = 0 - i;
					} else {
						/* Matching too many parties */
						problem = "parties";
					}

					/* Check for exact match */
					if (len == (int)strlen(parties[i].name)) {
						/* Never a problem */
						target = 0 - i;
						problem = "";

						/* Finished looking */
						break;
					}
				}
			}
		}
	}

	/* Check for recipient set but no match found */
	if ((len && !target) || (target < 0 && !party)) {
		/* Send an error message */
		if (!quiet) msg_format(Ind, "Could not match name '%s'.", name);

		/* Give up */
		return(0);
	}

	/* Check for multiple recipients found */
	if (strlen(problem)) {
		/* Send an error message */
		if (!quiet) msg_format(Ind, "'%s' matches too many %s.", name, problem);

		/* Give up */
		return(0);
	}

	/* Success */
	return(target);
}

/* copy/pasted from name_lookup_loose(), just without being loose..
   Note: Will fail if a non-admin tries to address an admin and requirements for that aren't met. */
int name_lookup(int Ind, cptr name, u16b party, bool include_account_names, bool quiet) {
	int i, j, len, target = 0;
	player_type *q_ptr, *p_ptr;
	bool party_online;

	p_ptr = Players[Ind];

	/* don't waste time */
	if (p_ptr == (player_type*)NULL) return(0);

	/* Acquire length of search string */
	len = strlen(name);

	/* Look for a recipient who matches the search string */
	if (len) {
		/* Check players */
		for (i = 1; i <= NumPlayers; i++) {
			/* Check this one */
			q_ptr = Players[i];

			/* Skip if disconnected */
			if (q_ptr->conn == NOT_CONNECTED) continue;

			/* let admins chat */
			if (q_ptr->admin_dm && !q_ptr->admin_dm_chat && !is_admin(p_ptr) && !may_address_dm(p_ptr)) continue;

			/* Check name */
			if (!strcasecmp(q_ptr->name, name)) {
				/* Never a problem */
				target = i;

				/* Finished looking */
				break;
			}
		}

		/* Then check accounts */
		if (include_account_names && !target) for (i = 1; i <= NumPlayers; i++) {
			/* Check this one */
			q_ptr = Players[i];

			/* Skip if disconnected */
			if (q_ptr->conn == NOT_CONNECTED) continue;

			/* let admins chat */
			if (q_ptr->admin_dm && !q_ptr->admin_dm_chat && !is_admin(p_ptr) && !may_address_dm(p_ptr)) continue;

			/* Check name */
			if (!strcasecmp(q_ptr->accountname, name)) {
				/* Never a problem */
				target = i;

				/* Finished looking */
				break;
			}
		}

		/* Check parties */
		if (party && !target) {
			for (i = 1; i < MAX_PARTIES; i++) {
				/* Skip if empty */
				if (!parties[i].members) continue;

				/* Check that the party has players online - mikaelh */
				party_online = FALSE;
				for (j = 1; j <= NumPlayers; j++) {
					/* Check this one */
					q_ptr = Players[j];

					/* Skip if disconnected */
					if (q_ptr->conn == NOT_CONNECTED) continue;

					/* Check if the player belongs to this party */
					if (q_ptr->party == i) {
						party_online = TRUE;
						break;
					}
				}
				if (!party_online) continue;

				/* Check name */
				if (!strcasecmp(parties[i].name, name)) {
					/* Never a problem */
					target = 0 - i;

					/* Finished looking */
					break;
				}
			}
		}
	}

	/* Check for recipient set but no match found */
	if ((len && !target) || (target < 0 && !party)) {
		/* Send an error message */
		if (!quiet) msg_format(Ind, "Could not match name '%s'.", name);

		/* Give up */
		return(0);
	}

	/* Success */
	return(target);
}

/*
 * Convert bracer '{' into '\377'
 */
void bracer_ff(char *buf) {
	int i, len = strlen(buf);

	for (i = 0; i < len; i++) {
		if (buf[i] == '{') buf[i] = '\377';
	}
}

/*
 * make strings from worldpos '-1550ft of (17,15)'	- Jir -
 */
char *wpos_format(int Ind, worldpos *wpos) {
	int i = Ind, n, d = 0;
	cptr desc = "";
	bool ville = istown(wpos) && !isdungeontown(wpos);
	dungeon_type *d_ptr = NULL;

	/* Hack for Valinor originally */
	if (i < 0) i = -i;
	if (wpos->wz > 0 && (d_ptr = wild_info[wpos->wy][wpos->wx].tower))
		d = d_ptr->type;
	if (wpos->wz < 0 && (d_ptr = wild_info[wpos->wy][wpos->wx].dungeon))
		d = d_ptr->type;
	if (!wpos->wz && ville)
		for (n = 0; n < numtowns; n++)
			if (town[n].x == wpos->wx && town[n].y == wpos->wy) {
				desc = town_profile[town[n].type].name;
				break;
			}
	if (!strcmp(desc, "")) ville = FALSE;

	if (!i || Players[i]->depth_in_feet) {
		if (Ind >= 0 || (!d && !ville)) {
			return(format("%dft of (%d,%d)", wpos->wz * 50, wpos->wx, wpos->wy));
		} else
			if (!ville)
				return(format("%dft %s", wpos->wz * 50, get_dun_name(wpos->wx, wpos->wy, (wpos->wz > 0), d_ptr, 0, FALSE)));
			else
				return(format("%s", desc));
	} else {
		if (Ind >= 0 || (!d && !ville)) {
			return(format("Lv %d of (%d,%d)", wpos->wz, wpos->wx, wpos->wy));
		} else
			if (!ville)
				return(format("Lv %d %s", wpos->wz, get_dun_name(wpos->wx, wpos->wy, (wpos->wz > 0), d_ptr, 0, FALSE)));
			else
				return(format("%s", desc));
	}
}
char *wpos_format_compact(int Ind, worldpos *wpos) {
	int n;
	cptr desc = "";
	bool ville = istown(wpos) && !isdungeontown(wpos);

	if (!wpos->wz && ville)
		for (n = 0; n < numtowns; n++)
			if (town[n].x == wpos->wx && town[n].y == wpos->wy) {
				desc = town_profile[town[n].type].name;
				break;
			}
	if (!strcmp(desc, "")) ville = FALSE;

	if (ville) return(format("%s", desc));
	else {
		if (Players[Ind]->depth_in_feet)
			return(format("%dft (%d,%d)", wpos->wz * 50, wpos->wx, wpos->wy));
		else
			return(format("Lv %d (%d,%d)", wpos->wz, wpos->wx, wpos->wy));
	}
}


byte count_bits(u32b array) {
	byte k = 0, i;

	if (array)
		for (i = 0; i < 32; i++)
			if (array & (1U << i)) k++;

	return(k);
}

/*
 * Find a player
 */
int get_playerind(char *name) {
	int i;

	if (name == (char*)NULL) return(-1);
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (!stricmp(Players[i]->name, name)) return(i);
	}
	return(-1);
}
int get_playerind_loose(char *name) {
	int i, len = strlen(name);

	if (len == 0) return(-1);
	if (name == (char*)NULL) return(-1);

	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (!strncasecmp(Players[i]->name, name, len)) return(i);
	}

	/* New, since this function is only used in misc lua functions anyway:
	   Also check for account name if no such character name */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (!strncasecmp(Players[i]->accountname, name, len)) return(i);
	}

	return(-1);
}

int get_playerslot_loose(int Ind, char *iname) {
	int i, j;
	char o_name[ONAME_LEN], i_name[ONAME_LEN];

	if (iname == (char*)NULL) return(-1);
	if ((*iname) == 0) return(-1);

	for (j = 0; iname[j]; j++) i_name[j] = tolower(iname[j]);
	i_name[j] = 0;

	for (i = 0; i < INVEN_TOTAL; i++) {
		if (!Players[Ind]->inventory[i].k_idx) continue;

		object_desc(0, o_name, &Players[Ind]->inventory[i], FALSE, 0x01 + 0x02 + 0x10 + 0x20);
		for (j = 0; o_name[j]; j++) o_name[j] = tolower(o_name[j]);

		if (strstr(o_name, i_name)) return(i);
	}

	return(-1);
}

/*
 * Tell the player of the floor feeling.	- Jir -
 * NOTE: differs from traditional 'boring' etc feeling!
 * NOTE: added traditional feelings, to warn of dangers - C. Blue
 */
bool show_floor_feeling(int Ind, bool dungeon_feeling) {
	player_type *p_ptr = Players[Ind];
	worldpos *wpos = &p_ptr->wpos;
	struct dungeon_type *d_ptr = getdungeon(wpos);
	dun_level *l_ptr = getfloor(wpos);
	bool felt = FALSE;

	/* No feelings on world surface for now */
	if (!wpos->wz) return(TRUE);

	/* No feelings! */
	if (d_ptr && (d_ptr->type == DI_DEATH_FATE || (!d_ptr->type && d_ptr->theme == DI_DEATH_FATE))) return(TRUE);

	/* Hack for Valinor - C. Blue */
	if (in_valinor(wpos)) {
		msg_print(Ind, "\374\377gYou have a wonderful feeling of peace...");
		return(TRUE);
	}

	/* XXX devise a better formula */
	if (!in_irondeepdive(&p_ptr->wpos) && (p_ptr->lev * ((p_ptr->lev >= 40) ? 3 : 2) + 5 < getlevel(wpos))) {
		msg_print(Ind, "\374\377rYou feel an imminent danger!");
		felt = TRUE;
	}

#ifdef DUNGEON_VISIT_BONUS
	if (dungeon_feeling && d_ptr && dungeon_bonus[d_ptr->id]
	    && !(d_ptr->flags3 & DF3_NO_DUNGEON_BONUS)) {
		felt = TRUE;
		switch (dungeon_bonus[d_ptr->id]) {
		case 3: msg_print(Ind, "\377UThis place has not been explored in ages."); break;
		case 2: msg_print(Ind, "\377UThis place has not been explored in a long time."); break;
		case 1: msg_print(Ind, "\377UThis place has not been explored recently."); break;
		}
	}
#endif

	if (!l_ptr) return(felt);

#ifdef EXTRA_LEVEL_FEELINGS
	/* Display extra traditional feeling to warn of dangers. - C. Blue
	   Note: Only display ONE feeling, thereby setting priorities here.
	   Note: Don't display feelings in Training Tower (NO_DEATH). */
	if ((!p_ptr->distinct_floor_feeling && !is_admin(p_ptr)) || (d_ptr->flags2 & DF2_NO_DEATH) ||
	    in_pvparena(wpos) || isdungeontown(wpos)) {
		//msg_print(Ind, "\376\377yLooks like any other level..");
		//msg_print(Ind, "\377ypfft");
	}
	else if (l_ptr->flags2 & LF2_OOD_HI) {
		msg_print(Ind, "\374\377yWhat a terrifying place..");
		felt = TRUE;
	} else if ((l_ptr->flags2 & LF2_VAULT_HI) &&
		(l_ptr->flags2 & LF2_OOD)) {
		msg_print(Ind, "\374\377yWhat a terrifying place..");
		felt = TRUE;
	} else if ((l_ptr->flags2 & LF2_VAULT_OPEN) || // <- TODO: implement :/
		 ((l_ptr->flags2 & LF2_VAULT) && (l_ptr->flags2 & LF2_OOD_FREE))) {
		msg_print(Ind, "\374\377yYou sense an air of danger.."); //this is pretty rare actually, maybe not worth it?
		felt = TRUE;
	} else if (l_ptr->flags2 & LF2_VAULT) {
		msg_print(Ind, "\374\377yFeels somewhat dangerous around here..");
		felt = TRUE;
	} else if (l_ptr->flags2 & LF2_PITNEST_HI) {
		msg_print(Ind, "\374\377yFeels somewhat dangerous around here..");
		felt = TRUE;
	} else if (l_ptr->flags2 & LF2_OOD_FREE) {
		msg_print(Ind, "\374\377yThere's a sensation of challenge..");
		felt = TRUE;
	} /*	else if (l_ptr->flags2 & LF2_PITNEST) { //maybe enable, maybe too cheezy
		msg_print(Ind, "\374\377yYou feel your luck is turning..");
		felt = TRUE;
	} */ else if (l_ptr->flags2 & LF2_UNIQUE) {
		msg_print(Ind, "\374\377yThere's a special feeling about this place..");
		felt = TRUE;
	} /* else if (l_ptr->flags2 & LF2_ARTIFACT) { //probably too cheezy, if not then might need combination with threat feelings above
		msg_print(Ind, "\374\377y");
		felt = TRUE;
	} *//*	else if (l_ptr->flags2 & LF2_ITEM_OOD) { //probably too cheezy, if not then might need combination with threat feelings above
		msg_print(Ind, "\374\377y");
		felt = TRUE;
	} */ else {
		msg_print(Ind, "\374\377yWhat a boring place..");
		felt = TRUE;
	}
#endif

	/* Special feeling for dungeon bosses in IDDC */
	if ((p_ptr->distinct_floor_feeling || is_admin(p_ptr)) &&
	    in_irondeepdive(wpos) && (l_ptr->flags2 & LF2_DUN_BOSS))
		msg_print(Ind, "\374\377UYou feel a commanding presence..");


	/* Hack^2 -- display the 'feeling' */
	if (l_ptr->flags1 & LF1_NO_MAGIC) {
		msg_print(Ind, "\377oYou feel a suppressive air.");
		/* Automatically dis/re-enable wraith form */
		p_ptr->update |= PU_BONUS;
	}
	if (l_ptr->flags1 & LF1_NO_GENO)
//sounds as if it's good for the player		msg_print(Ind, "\377oYou have a feeling of peace...");
		msg_print(Ind, "\377oThe air seems distorted.");
	if (l_ptr->flags1 & LF1_NO_MAP)
		msg_print(Ind, "\377oYou lose all sense of direction...");
	if (l_ptr->flags1 & LF1_NO_MAGIC_MAP)
		msg_print(Ind, "\377oYou feel like a stranger...");
	if (l_ptr->flags1 & LF1_NO_DESTROY)
		msg_print(Ind, "\377oThe walls here seem very solid.");
	if (l_ptr->flags1 & LF1_NO_GHOST)
		msg_print(Ind, "\377RYou feel that your life hangs in the balance!"); //credits to Moltor actually, ha!:)
#if 0
	if (l_ptr->flags1 & DF1_NO_RECALL)
		msg_print(Ind, "\377oThere is strong magic enclosing this dungeon.");
#endif
	/* Can leave IRONMAN? */
	if (((l_ptr->flags1 & LF1_IRON_RECALL) || ((d_ptr->flags1 & DF1_FORCE_DOWN) && d_ptr->maxdepth == ABS(p_ptr->wpos.wz)))
	    && !(d_ptr->flags2 & DF2_NO_EXIT_WOR))
		msg_print(Ind, "\374\377gYou don't sense a magic barrier here!");

	return((l_ptr->flags1 & LF1_FEELING_MASK) ? TRUE : felt);
}

/*
 * Given item name as string, return the index in k_info array. Name
 * must exactly match (look out for commas and the like!), or else 0 is
 * returned. Case doesn't matter. -DG-
 */

int test_item_name(cptr name) {
	int i;

	/* Scan the items */
	for (i = 1; i < max_k_idx; i++) {
		object_kind *k_ptr = &k_info[i];
		cptr obj_name = k_name + k_ptr->name;

		/* If name matches, give us the number */
		if (stricmp(name, obj_name) == 0) return(i);
	}
	return(0);
}

/*
 * Middle-Earth (Imladris) calendar code from ToME
 */
/*
 * Break scalar time
 */
s32b bst(s32b what, s32b t) {
#if 0 /* TIMEREWRITE */
	s32b turns = t + (10 * DAY_START);

	switch (what) {
		case MINUTE:
			return((turns / 10 / MINUTE) % 60);
		case HOUR:
			return((turns / 10 / HOUR) % 24);
		case DAY:
			return((turns / 10 / DAY) % 365);
		case YEAR:
			return((turns / 10 / YEAR));
		default:
			return(0);
	}
#else
	s32b turns = t;

	if (what == MINUTE)
		return((turns / MINUTE) % 60);
	else if (what == HOUR)
		return((turns / HOUR) % 24);
	else if (what == DAY)
		return((((turns / DAY) + START_DAY) % 365));
	else if (what == YEAR)
		return((turns / YEAR) + START_YEAR);
	else
		return(0);
#endif
}

cptr get_month_name(int day, bool full, bool compact) {
	int i = 8;
	static char buf[40];

	/* Find the period name */
	while ((i > 0) && (day < month_day[i]))
		i--;

	switch (i) {
	/* Yestare/Mettare */
	case 0:
	case 8: {
		char buf2[20];

		snprintf(buf2, 20, "%s", get_day(day + 1));
		if (full) snprintf(buf, 40, "%s (%s day)", month_name[i], buf2);
		else snprintf(buf, 40, "%s", month_name[i]);
		break;
	}
	/* 'Normal' months + Enderi */
	default: {
		char buf2[6];
		char buf3[6];

		snprintf(buf2, 6, "%s", get_day(day + 1 - month_day[i]));
		snprintf(buf3, 6, "%s", get_day(day + 1));

		if (full) snprintf(buf, 40, "%s day of %s (%s day)", buf2, month_name[i], buf3);
		else if (compact) snprintf(buf, 40, "%s day of %s", buf2, month_name[i]);
		else snprintf(buf, 40, "%s %s", buf2, month_name[i]);
		break;
		}
	}

	return(buf);
}

cptr get_day(int day) {
	static char buf[20];
	cptr p = "th";

	if ((day / 10) == 1) ;
	else if ((day % 10) == 1) p = "st";
	else if ((day % 10) == 2) p = "nd";
	else if ((day % 10) == 3) p = "rd";

	snprintf(buf, 20, "%d%s", day, p);
	return(buf);
}

/* Fuzzy: Will allow one-off colour,
   Compact: Will increase stack size by factor [100] for all particular colour ranges. */
int gold_colour(s32b amt, bool fuzzy, bool compact) {
	s32b i;
	int unit = 1;

#if 1
	/* Limit to avoid overflow freeze */
	if (amt > 1000000000) amt = 1000000000;

	/* special: perform pre-rounding, to achieve smoother looking transitions */
	for (i = 1; i * 10 <= amt; i *= 10);
	/* keep first 2 digits, discard the rest */
	if (i >= 10) amt = (amt / (i / 10)) * (i / 10);
#endif

	if (compact)
#if 0 /* scale? */
		for (i = amt / 100; i >= 40; i = (i * 2) / 3, unit++) /* naught */;
#else /* stretch? */
 #if 0 /* was ok, but due to natural income flow many lower tier colours are sort of skipped */
		//for (i = amt; i >= 60; i = (i * 2) / 4, unit++) /* naught */;
 #else /* seems to scale better, allowing all colours to shine */
		for (i = amt; i >= 60; i = (i * 4) / 9, unit++) /* naught */;
 #endif
#endif
	else
		for (i = amt; i >= 40; i = (i * 2) / 3, unit++) /* naught */;
	if (fuzzy)
		//unit = unit - 1 + rand_int(3);
		//unit = unit - 1 + (rand_int(5) + 2) / 3;
		unit = unit - 1 + (rand_int(7) + 4) / 5;
	if (unit < 1) unit = 1;
	if (unit > SV_GOLD_MAX) unit = SV_GOLD_MAX;

	return(lookup_kind(TV_GOLD, unit));
}

/* Catching bad players who hack their client.. nasty! */
void lua_intrusion(int Ind, char *problem_diz) {
	s_printf("LUA INTRUSION: %s : %s\n", Players[Ind]->name, problem_diz);

#if 0 /* 4.4.8 client had a bug, mass-near-killing people. Let's turn this silly stuff off. -C. Blue */
	take_hit(Ind, Players[Ind]->chp - 1, "", 0);
	msg_print(Ind, "\377rThat was close huh?!");
#else
	if (!strcmp(problem_diz, "bad spell level")) {
		msg_print(Ind, "\376\377RERROR: You need higher skill to cast this spell. However, your book shows");
		msg_print(Ind, "\376\377R       that you may cast it because your LUA spell scripts are out of date!");
		msg_print(Ind, "\376\377R       Please update your client (or at least the LUA files in lib/scpt).");
		// (and don't use '-u' command-line option)
	} else {
		msg_print(Ind, "\376\377RERROR: Your LUA spell scripts seem to be out of date!");
		msg_print(Ind, "\376\377R       Please update your client (or at least the LUA files in lib/scpt).");
		// (and don't use '-u' command-line option)
	}
#endif
}

void bbs_add_line(cptr textline, cptr textline_u) {
	int i, j;
	/* either find an empty bbs entry (store its position in j) */
	for (i = 0; i < BBS_LINES; i++)
		if (!strcmp(bbs_line[i], "")) break;
	j = i;
	/* or scroll up by one line, discarding the first line */
	if (i == BBS_LINES)
		for (j = 0; j < BBS_LINES - 1; j++) {
			strcpy(bbs_line[j], bbs_line[j + 1]);
			strcpy(bbs_line_u[j], bbs_line_u[j + 1]);
		}
	/* write the line to the bbs */
	strncpy(bbs_line[j], textline, MAX_CHARS_WIDE - 3); /* lines get one leading spaces on outputting, so it's 78-1  //  was 77 */
	strncpy(bbs_line_u[j], textline_u, MAX_CHARS_WIDE - 3); /* lines get one leading spaces on outputting, so it's 78-1  //  was 77 */
}

void bbs_del_line(int entry) {
	int j;
	if (entry < 0) return;
	if (entry >= BBS_LINES) return;
	for (j = entry; j < BBS_LINES - 1; j++)
		strcpy(bbs_line[j], bbs_line[j + 1]);
	/* erase last line */
	strcpy(bbs_line[BBS_LINES - 1], "");
}

void bbs_erase(void) {
	int i;
	for (i = 0; i < BBS_LINES; i++)
		strcpy(bbs_line[i], "");
}

void pbbs_add_line(u16b party, cptr textline, cptr textline_u) {
	int i, j;
	/* either find an empty bbs entry (store its position in j) */
	for (i = 0; i < BBS_LINES; i++)
		if (!strcmp(pbbs_line[party][i], "")) break;
	j = i;
	/* or scroll up by one line, discarding the first line */
	if (i == BBS_LINES)
		for (j = 0; j < BBS_LINES - 1; j++) {
			strcpy(pbbs_line[party][j], pbbs_line[party][j + 1]);
			strcpy(pbbs_line_u[party][j], pbbs_line_u[party][j + 1]);
		}
	/* write the line to the bbs */
	strncpy(pbbs_line[party][j], textline, MAX_CHARS_WIDE - 3);
	strncpy(pbbs_line_u[party][j], textline_u, MAX_CHARS_WIDE - 3);
}

void gbbs_add_line(byte guild, cptr textline, cptr textline_u) {
	int i, j;
	/* either find an empty bbs entry (store its position in j) */
	for (i = 0; i < BBS_LINES; i++)
		if (!strcmp(gbbs_line[guild][i], "")) break;
	j = i;
	/* or scroll up by one line, discarding the first line */
	if (i == BBS_LINES)
		for (j = 0; j < BBS_LINES - 1; j++) {
			strcpy(gbbs_line[guild][j], gbbs_line[guild][j + 1]);
			strcpy(gbbs_line_u[guild][j], gbbs_line_u[guild][j + 1]);
		}
	/* write the line to the bbs */
	strncpy(gbbs_line[guild][j], textline, MAX_CHARS_WIDE - 3);
	strncpy(gbbs_line_u[guild][j], textline_u, MAX_CHARS_WIDE - 3);
}


/*
 * Add a player id to a linked list.
 * Doesn't check for duplicates
 * Takes a double pointer to the list
 */
void player_list_add(player_list_type **list, s32b player) {
	player_list_type *pl_ptr;

	MAKE(pl_ptr, player_list_type);

	pl_ptr->id = player;
	pl_ptr->next = *list;
	*list = pl_ptr;
}

/*
 * Check if a list contains an id.
 */
bool player_list_find(player_list_type *list, s32b player) {
	player_list_type *pl_ptr;

	pl_ptr = list;

	while (pl_ptr) {
		if (pl_ptr->id == player) return(TRUE);
		pl_ptr = pl_ptr->next;
	}

	return(FALSE);
}

/*
 * Delete an id from a list.
 * Takes a double pointer to the list
 */
bool player_list_del(player_list_type **list, s32b player) {
	player_list_type *pl_ptr, *prev;

	if (*list == NULL) return(FALSE);

	/* Check the first node */
	if ((*list)->id == player) {
		*list = (*list)->next;
		return(TRUE);
	}

	pl_ptr = (*list)->next;
	prev = *list;

	/* Check the rest of the nodes */
	while (pl_ptr) {
		if (pl_ptr->id == player) {
			prev->next = pl_ptr->next;
			FREE(pl_ptr, player_list_type);
			return(TRUE);
		}

		prev = pl_ptr;
		pl_ptr = pl_ptr->next;
	}

	return(FALSE);
}

/*
 * Free an entire list.
 */
void player_list_free(player_list_type *list) {
	player_list_type *pl_ptr, *tmp;

	pl_ptr = list;

	while (pl_ptr) {
		tmp = pl_ptr;
		pl_ptr = pl_ptr->next;
		FREE(tmp, player_list_type);
	}
}

/*
 * Since only GNU libc has memfrob, we use our own.
 */
void my_memfrob(void *s, int n) {
	int i;
	char *str;

	str = (char*) s;

	for (i = 0; i < n; i++) {
		/* XOR every byte with 42 */
		str[i] ^= 42;
	}
}

/* compare player mode compatibility - C. Blue
   Note: returns NULL if compatible.
   strict: Ignore IRONDEEPDIVE_ALLOW_INCOMPAT.
   Note: We don't handle MODE_SOLO, so it has to be checked explicitely whenever important. */
cptr compat_pmode(int Ind1, int Ind2, bool strict) {
	player_type *p1_ptr = Players[Ind1], *p2_ptr = Players[Ind2];

#ifdef IRONDEEPDIVE_ALLOW_INCOMPAT
	/* EXPERIMENTAL */
	if (!strict &&
	    in_irondeepdive(&p1_ptr->wpos) &&
	    in_irondeepdive(&p2_ptr->wpos))
		return(NULL);
#endif

#ifdef MODULE_ALLOW_INCOMPAT
	if (!strict &&
	    in_module(&p1_ptr->wpos) &&
	    in_module(&p2_ptr->wpos))
		return(NULL);
#endif

	if (p1_ptr->mode & MODE_PVP) {
		if (!(p2_ptr->mode & MODE_PVP)) {
			return("non-pvp");
		}
	} else if (p1_ptr->mode & MODE_EVERLASTING) {
		if (!(p2_ptr->mode & MODE_EVERLASTING)) {
			return("non-everlasting");
		}
	} else if (p2_ptr->mode & MODE_PVP) {
		return("pvp");
	} else if (p2_ptr->mode & MODE_EVERLASTING) {
		return("everlasting");
	}
	return(NULL); /* means "is compatible" */
}

/* compare object and player mode compatibility - C. Blue
   Note: returns NULL if compatible.
   Note: We don't handle MODE_SOLO, so it has to be checked explicitely whenever important. */
cptr compat_pomode(int Ind, object_type *o_ptr) {
	player_type *p_ptr = Players[Ind];

#ifdef IRONDEEPDIVE_ALLOW_INCOMPAT
	/* EXPERIMENTAL */
	if (in_irondeepdive(&p_ptr->wpos) &&
	    in_irondeepdive(&o_ptr->wpos))
		return(NULL);
#endif

#ifdef MODULE_ALLOW_INCOMPAT
	if (in_module(&p_ptr->wpos) &&
	    in_module(&o_ptr->wpos))
		return(NULL);
#endif

#ifdef ALLOW_NR_CROSS_ITEMS
	if (o_ptr->NR_tradable &&
	    in_netherrealm(&p_ptr->wpos))
		return(NULL);
#endif

	if (!o_ptr->owner || is_admin(p_ptr)) return(NULL); /* always compatible */
	if (p_ptr->mode & MODE_PVP) {
		if (!(o_ptr->mode & MODE_PVP)) {
			if (o_ptr->mode & MODE_EVERLASTING) {
				if (!(cfg.charmode_trading_restrictions & 2)) {
					return("non-pvp");
				}
			} else if (!(cfg.charmode_trading_restrictions & 4)) {
				return("non-pvp");
			}
		}
	} else if (p_ptr->mode & MODE_EVERLASTING) {
		if (o_ptr->mode & MODE_PVP) {
			return("pvp");
		} else if (!(o_ptr->mode & MODE_EVERLASTING)) {
			if (!(cfg.charmode_trading_restrictions & 1)) return("non-everlasting");
		}
	} else if (o_ptr->mode & MODE_PVP) {
		return("pvp");
	} else if (o_ptr->mode & MODE_EVERLASTING) {
		return("everlasting");
	}
	return(NULL); /* means "is compatible" */
}

/* compare two objects' mode compatibility for stacking/absorbing - C. Blue
   Note: returns NULL if compatible. */
cptr compat_omode(object_type *o1_ptr, object_type *o2_ptr) {
#ifdef IRONDEEPDIVE_ALLOW_INCOMPAT
	/* EXPERIMENTAL */
	if ((in_irondeepdive(&o1_ptr->wpos)) &&
	    in_irondeepdive(&o2_ptr->wpos))
		return(NULL);
#endif

#ifdef MODULE_ALLOW_INCOMPAT
	if ((in_module(&o1_ptr->wpos)) &&
	    in_module(&o2_ptr->wpos))
		return(NULL);
#endif

	/* ownership given for both items? */
	if (!o1_ptr->owner) {
		if (!o2_ptr->owner) return(NULL); /* always compatible */
		else return("owned");
	} else if (!o2_ptr->owner) return("owned");

	/* both are owned. so compare actual modes */
	if (o1_ptr->mode & MODE_PVP) {
		if (!(o2_ptr->mode & MODE_PVP)) {
			return("non-pvp");
		}
	} else if (o1_ptr->mode & MODE_EVERLASTING) {
		if (!(o2_ptr->mode & MODE_EVERLASTING)) {
			return("non-everlasting");
		}
	} else if (o2_ptr->mode & MODE_PVP) {
		return("pvp");
	} else if (o2_ptr->mode & MODE_EVERLASTING) {
		return("everlasting");
	}
	return(NULL); /* means "is compatible" */
}

/* compare mode compatibility (eg player/player for merchants guild post, player/party, player/guild member) - C. Blue
   Note: returns NULL if compatible.
   Note: We don't handle MODE_SOLO, so it has to be checked explicitely whenever important. */
cptr compat_mode(u32b mode1, u32b mode2) {
	if (mode1 & MODE_PVP) {
		if (!(mode2 & MODE_PVP)) {
			return("non-pvp");
		}
	} else if (mode1 & MODE_EVERLASTING) {
		if (!(mode2 & MODE_EVERLASTING)) {
			return("non-everlasting");
		}
	} else if (mode2 & MODE_PVP) {
		return("pvp");
	} else if (mode2 & MODE_EVERLASTING) {
		return("everlasting");
	}
	return(NULL); /* means "is compatible" */
}

/* cut out pseudo-id inscriptions from a string (a note ie inscription usually),
   save resulting string in s2,
   save highest found pseudo-id string in psid. - C. Blue */
void note_crop_pseudoid(char *s2, char *psid, cptr s) {
	char *p, s0[ONAME_LEN];
	int id = -1; /* assume no pseudo-id inscription */

	if (s == NULL) return;
	strcpy(s2, s);

	while (TRUE) {
		strcpy(s0, s2);
		strcpy(s2, "");

		if ((p = strstr(s0, "empty-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 6);
			if (id < 0) id = 0;
		} else if ((p = strstr(s0, "empty"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 5);
			if (id < 0) id = 0;
		} else if ((p = strstr(s0, "uncursed-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 9);
			if (id < 0) id = 1;
		} else if ((p = strstr(s0, "uncursed"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 8);
			if (id < 0) id = 1;
		} else if ((p = strstr(s0, "terrible-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 9);
			if (id < 1) id = 2;
		} else if ((p = strstr(s0, "terrible"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 8);
			if (id < 1) id = 2;
		} else if ((p = strstr(s0, "cursed-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 7);
			if (id < 2) id = 3;
		} else if ((p = strstr(s0, "cursed"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 6);
			if (id < 2) id = 3;
		} else if ((p = strstr(s0, "bad-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 4);
			if (id < 2) id = 4;
		} else if ((p = strstr(s0, "bad"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 3);
			if (id < 2) id = 4;
		} else if ((p = strstr(s0, "worthless-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 10);
			if (id < 4) id = 5;
		} else if ((p = strstr(s0, "worthless"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 9);
			if (id < 4) id = 5;
		} else if ((p = strstr(s0, "broken-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 7);
			if (id < 5) id = 6;
		} else if ((p = strstr(s0, "broken"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 6);
			if (id < 5) id = 6;
		} else if ((p = strstr(s0, "average-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 8);
			if (id < 6) id = 7;
		} else if ((p = strstr(s0, "average"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 7);
			if (id < 6) id = 7;
		} else if ((p = strstr(s0, "good-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 5);
			if (id < 7) id = 8;
		} else if ((p = strstr(s0, "good"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 4);
			if (id < 7) id = 8;
		} else if ((p = strstr(s0, "excellent-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 10);
			if (id < 8) id = 9;
		} else if ((p = strstr(s0, "excellent"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 9);
			if (id < 8) id = 9;
		} else if ((p = strstr(s0, "special-"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 8);
			if (id < 9) id = 10;
		} else if ((p = strstr(s0, "special"))) {
			strncpy(s2, s0, p - s0);
			s2[p - s0] = '\0';
			strcat(s2, p + 7);
			if (id < 9) id = 10;
		} else {
			/* no further replacements to make */
			break;
		}
	}

	strcpy(s2, s0);

	switch (id) {
	case 0: strcpy(psid, "empty"); break;
	case 1: strcpy(psid, "uncursed"); break;
	case 2: strcpy(psid, "terrible"); break;
	case 3: strcpy(psid, "cursed"); break;
	case 4: strcpy(psid, "bad"); break;
	case 5: strcpy(psid, "worthless"); break;
	case 6: strcpy(psid, "broken"); break;
	case 7: strcpy(psid, "average"); break;
	case 8: strcpy(psid, "good"); break;
	case 9: strcpy(psid, "excellent"); break;
	case 10: strcpy(psid, "special"); break;
	default:
		strcpy(psid, "");
	}
}

/* For when an item re-curses itself on equipping:
   Remove any 'uncursed' part in its inscription. */
void note_toggle_cursed(object_type *o_ptr, bool cursed) {
	char *cn, note2[ONAME_LEN], *cnp;

	if (!o_ptr->note) {
		if (cursed) o_ptr->note = quark_add("cursed");
		else o_ptr->note = quark_add("uncursed");
		return;
	}

	strcpy(note2, quark_str(o_ptr->note));

	/* remove old 'uncursed' inscriptions */
	if ((cn = strstr(note2, "uncursed"))) {
		while (note2[0] && (cn = strstr(note2, "uncursed"))) {
			cnp = cn + 7;
			if (cn > note2 && //the 'uncursed' does not start on the first character of note2?
			    *(cn - 1) == '-') cn--; /* cut out leading '-' delimiter before "uncursed" */

			/* strip formerly trailing delimiter if it'd end up on first position in the new inscription */
			if (cn == note2 && *(cnp + 1) == '-') cnp++;

			do {
				cnp++;
				*cn = *cnp;
				cn++;
			} while (*cnp);
		}
	}

	/* remove old 'cursed' inscription */
	if ((cn = strstr(note2, "cursed"))) {
		while (note2[0] && (cn = strstr(note2, "cursed"))) {
			cnp = cn + 5;
			if (cn > note2 && //the 'cursed' does not start on the first character of note2?
			    *(cn - 1) == '-') cn--; /* cut out leading '-' delimiter before "cursed" */

			/* strip formerly trailing delimiter if it'd end up on first position in the new inscription */
			if (cn == note2 && *(cnp + 1) == '-') cnp++;

			do {
				cnp++;
				*cn = *cnp;
				cn++;
			} while (*cnp);
		}
	}

	/* append the new cursed-state indicator */
	if (note2[0]) strcat(note2, "-");
	if (cursed) {
		strcat(note2, "cursed");
		o_ptr->note = quark_add(note2);
	} else {
		strcat(note2, "uncursed");
		o_ptr->note = quark_add(note2);
	}
}

/* Wands/staves: Add/crop 'empty' inscription. */
void note_toggle_empty(object_type *o_ptr, bool empty) {
	char *cn, note2[ONAME_LEN], *cnp;

	/* If no inscription, object_desc() will handle this by adding a pseudo-inscription 'empty'. */
	if (!o_ptr->note) return;

	strcpy(note2, quark_str(o_ptr->note));

	if (!empty) {
		/* remove old 'empty' inscription */
		if ((cn = strstr(note2, "empty"))) {
			while (note2[0] && (cn = strstr(note2, "empty"))) {
				cnp = cn + 4;
				if (cn > note2 && //the 'empty' does not start on the first character of note2?
				    *(cn - 1) == '-') cn--; /* cut out leading '-' delimiter before "empty" */

				/* strip formerly trailing delimiter if it'd end up on first position in the new inscription */
				if (cn == note2 && *(cnp + 1) == '-') cnp++;

				do {
					cnp++;
					*cn = *cnp;
					cn++;
				} while (*cnp);
			}
		}
		o_ptr->note = quark_add(note2);
		return;
	}

	/* inscription already exists? */
	if (strstr(note2, "empty")) return;

	/* don't add 'empty' inscription to items that show "(0 charges)" already, aka identified items */
	if (o_ptr->ident & ID_KNOWN) return;

	/* append the new empty-state indicator */
	if (note2[0]) strcat(note2, "-");
	if (empty) {
		strcat(note2, "empty");
		o_ptr->note = quark_add(note2);
	}
}

/* Convert certain characters into HTML entities
* These characters are <, >, & and "
* NOTE: The returned string is dynamically allocated
*/
char *html_escape(const char *str) {
	int i, new_len = 0;
	const char *tmp;
	char *result;

	if (!str) {
		/* Return an empty string */
		result = malloc(1);
		*result = '\0';
		return(result);
	}

	/* Calculate the resulting length */
	tmp = str;
	while (*tmp) {
		switch (*tmp) {
		case '<': case '>':
			new_len += 3;
			break;
		case '&':
			new_len += 4;
			break;
		case '"':
			new_len += 5;
			break;
		}
		new_len++;
		tmp++;
	}

	result = malloc(new_len + 1);
	i = 0;
	tmp = str;

	while (*tmp) {
		switch (*tmp) {
		case '<':
			result[i++] = '&';
			result[i++] = 'l';
			result[i++] = 't';
			result[i++] = ';';
			break;
		case '>':
			result[i++] = '&';
			result[i++] = 'g';
			result[i++] = 't';
			result[i++] = ';';
			break;
		case '&':
			result[i++] = '&';
			result[i++] = 'a';
			result[i++] = 'm';
			result[i++] = 'p';
			result[i++] = ';';
			break;
		case '"':
			result[i++] = '&';
			result[i++] = 'q';
			result[i++] = 'u';
			result[i++] = 'o';
			result[i++] = 't';
			result[i++] = ';';
			break;
		default:
			result[i++] = *tmp;
		}
		tmp++;
	}

	/* Terminate */
	result[new_len] = '\0';

	return(result);
}

#define JSON_ESCAPE_ARRAY_SIZE 93
const char *json_escape_chars[JSON_ESCAPE_ARRAY_SIZE] = {
	"\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007",
	"\\b",     "\\t",     "\\n",     "\\u000b", "\\f",     "\\r",     "\\u000e", "\\u000f",
	"\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017",
	"\\u0018", "\\u0019", "\\u001a", "\\u001b", "\\u001c", "\\u001d", "\\u001e", "\\u001f",
	NULL, NULL, "\\\"", NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, "\\\\"
};

const int json_escape_len[JSON_ESCAPE_ARRAY_SIZE] = {
	5, 5, 5, 5, 5, 5, 5, 5,
	2, 2, 2, 5, 2, 2, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	0, 0, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 2
};

/*
 * JSON spec requires that all control characters, ", and \ must be escaped.
 * 'dest' is the output buffer, 'src' is the input buffer and 'n' is the size of the output buffer.
 * The resulting string is always NULL-terminated even if the buffer is not big enough.
 */
char *json_escape_str(char *dest, const char *src, size_t n) {
	size_t destpos = 0;
	const char *outstr = NULL;
	int outlen = 0;
	int c = 0;

	while ((c = *src++)) {
		// Special characters need to be escaped
		if (c >= 0 && c < JSON_ESCAPE_ARRAY_SIZE) {
			outstr = json_escape_chars[c];
			outlen = json_escape_len[c];
		} else {
			outstr = NULL;
			outlen = 0;
		}
		if (outstr) {
			if (destpos + outlen + 1 < n) {
				char c2;

				while ((c2 = *outstr++)) dest[destpos++] = c2;
			}
		} else {
			// Make sure output buffer has enough room
			if (destpos + 1 < n) {
				dest[destpos++] = c;
			}
		}
	}
	dest[destpos] = '\0';
	return(dest);
}

/* level generation benchmark */
void do_benchmark(int Ind) {
	int i, n = 100;
	player_type *p_ptr = Players[Ind];
	struct timeval begin, end;
	int sec, usec;

#ifndef WINDOWS
	block_timer();
#endif

	/* Use gettimeofday to get precise time */
	gettimeofday(&begin, NULL);

	for (i = 0; i < n; i++) {
		generate_cave(&p_ptr->wpos, p_ptr);
	}

	gettimeofday(&end, NULL);

#ifndef WINDOWS
	allow_timer();
#endif

	/* Calculate the time */
	sec = end.tv_sec - begin.tv_sec;
	usec = end.tv_usec - begin.tv_usec;
	if (usec < 0) {
		usec += 1000000;
		sec--;
	}

	/* Print the result */
	msg_format(Ind, "Generated %d levels in %d.%06d seconds.", n, sec, usec);

	/* Log it too */
	s_printf("BENCHMARK: Generated %d levels in %d.%06d seconds.\n", n, sec, usec);
}

/*
 * Get the difference between two timevals as a string.
 */
cptr timediff(struct timeval *begin, struct timeval *end) {
	static char buf[20];
	int sec, msec, usec;

	sec = end->tv_sec - begin->tv_sec;
	usec = end->tv_usec - begin->tv_usec;

	if (usec < 0) {
		usec += 1000000;
		sec--;
	}

	msec = usec / 1000;

	snprintf(buf, 20, "%d.%03d", sec, msec);
	return(buf);
}

/* Strip special chars off player input 's', to prevent any problems/colours */
void strip_control_codes(char *ss, char *s) {
	char *sp = s, *ssp = ss;
	bool skip = FALSE;

	while (TRUE) {
		if (*sp == '\0') { /* end of string has top priority */
			*ssp = '\0';
			break;
		} else if (skip) skip = FALSE; /* a 'code parameter', needs to be stripped too */
		else if ((*sp >= 32) && (*sp <= 127)) { /* normal characters */
			*ssp = *sp;
			ssp++;
		} else if (*sp == '\377') {
			if (*(sp + 1) == '\377') { /* make two '{' become one real '{' */
				*ssp = '{';
				ssp++;
			}
			skip = TRUE; /* strip colour codes */
		}
		sp++;
	}
}

/* return a string displaying the flag state - C. Blue */
cptr flags_str(u32b flags) {
#if 0 /* disfunctional atm */
	char *fsp = malloc(40 * sizeof(char));
	int b;

	for (b = 0; b < 32; b++) {
		*fsp++ = (flags & (1U << b)) ? '1' : '0';
		if (b % 4 == 3) *fsp++ = ' ';
	}
	*fsp++ = '\0';

	return(fsp - 40);
#else
	return(NULL);
#endif
}

/* get player's racial attribute - return empty string if race is forced by class. */
cptr get_prace2(player_type *p_ptr) {
#ifdef ENABLE_MAIA
	if (p_ptr->prace == RACE_MAIA && p_ptr->ptrait) {
		if (p_ptr->ptrait == TRAIT_ENLIGHTENED)
			return("Enlightened ");
		else if (p_ptr->ptrait == TRAIT_CORRUPTED) {
 #ifdef ENABLE_HELLKNIGHT
			if (p_ptr->pclass == CLASS_HELLKNIGHT) return(""); else
 #endif
			return("Corrupted ");
		} else
			return(special_prace_lookup2[p_ptr->prace]);
	} else
#endif
#ifdef ENABLE_DEATHKNIGHT
	if (p_ptr->pclass == CLASS_DEATHKNIGHT) return(""); else
#endif
#ifdef ENABLE_HELLKNIGHT
	if (p_ptr->pclass == CLASS_HELLKNIGHT) return(""); else
#endif
	return(special_prace_lookup2[p_ptr->prace]);
}
/* like get_prace2(), but always returns the race. Also, no trailing space. */
cptr get_prace(player_type *p_ptr) {
#ifdef ENABLE_MAIA
	if (p_ptr->prace == RACE_MAIA && p_ptr->ptrait) {
		if (p_ptr->ptrait == TRAIT_ENLIGHTENED)
			return("Enlightened");
		else if (p_ptr->ptrait == TRAIT_CORRUPTED)
			return("Corrupted");
		else
			return(special_prace_lookup[p_ptr->prace]);
	} else
#endif
	return(special_prace_lookup[p_ptr->prace]);
}

/* get player's title */
cptr get_ptitle(player_type *p_ptr, bool short_form) {
	if (p_ptr->lev < 60) return(player_title[p_ptr->pclass][((p_ptr->lev / 5) < 10)? (p_ptr->lev / 5) : 10][(short_form ? 3 : 1) - p_ptr->male]);
	return(player_title_special[p_ptr->pclass][(p_ptr->lev < PY_MAX_PLAYER_LEVEL) ? (p_ptr->lev - 60) / 10 : 4][(short_form ? 3 : 1) - p_ptr->male]);
}
#ifdef ENABLE_SUBCLASS_TITLE
cptr get_ptitle2(player_type *p_ptr, bool short_form) {
	if (!(p_ptr->sclass)) return(""); // paranoia?
	if (p_ptr->lev < 60) return(player_title[p_ptr->sclass - 1][((p_ptr->lev / 5) < 10)? (p_ptr->lev / 5) : 10][(short_form ? 3 : 1) - p_ptr->male]);
	return(player_title_special[p_ptr->sclass - 1][(p_ptr->lev < PY_MAX_PLAYER_LEVEL) ? (p_ptr->lev - 60) / 10 : 4][(short_form ? 3 : 1) - p_ptr->male]);
}
#endif

#ifdef DUNGEON_VISIT_BONUS
void reindex_dungeons() {
# ifdef DUNGEON_VISIT_BONUS_DEPTHRANGE
	int i;
# endif
	int x, y;
	wilderness_type *w_ptr;
	struct dungeon_type *d_ptr;

	dungeon_id_max = 0;

	for (y = 0; y < MAX_WILD_Y; y++) {
		for (x = 0; x < MAX_WILD_X; x++) {
			w_ptr = &wild_info[y][x];
			if (w_ptr->flags & WILD_F_UP) {
				d_ptr = w_ptr->tower;
				d_ptr->id = ++dungeon_id_max;

				dungeon_visit_frequency[dungeon_id_max] = 0;
				dungeon_x[dungeon_id_max] = x;
				dungeon_y[dungeon_id_max] = y;
				dungeon_tower[dungeon_id_max] = TRUE;
				dungeon_ditype[dungeon_id_max] = d_ptr->type;

				/* Initialize all dungeons at 'low rest bonus' */
				set_dungeon_bonus(dungeon_id_max, TRUE);

				s_printf("  indexed tower   at %d,%d: id = %d\n", x, y, dungeon_id_max);
			}
			if (w_ptr->flags & WILD_F_DOWN) {
				d_ptr = w_ptr->dungeon;
				d_ptr->id = ++dungeon_id_max;

				dungeon_visit_frequency[dungeon_id_max] = 0;
				dungeon_x[dungeon_id_max] = x;
				dungeon_y[dungeon_id_max] = y;
				dungeon_tower[dungeon_id_max] = FALSE;
				dungeon_ditype[dungeon_id_max] = d_ptr->type;

				/* Initialize all dungeons at 'low rest bonus' */
				set_dungeon_bonus(dungeon_id_max, TRUE);

				s_printf("  indexed dungeon at %d,%d: id = %d\n", x, y, dungeon_id_max);
			}
		}
	}

# ifdef DUNGEON_VISIT_BONUS_DEPTHRANGE
	for (i = 0; i < 20; i++)
		depthrange_visited[i] = 0;
# endif

	s_printf("Reindexed %d dungeons/towers.\n", dungeon_id_max);
}

void set_dungeon_bonus(int id, bool reset) {
	if (reset) {
		/* Initialize dungeon at 'low rest bonus' */
		dungeon_visit_frequency[id] = ((VISIT_TIME_CAP * 17) / 20) - 1; /* somewhat below the threshold */
		dungeon_bonus[id] = 1;
		return;
	}

	if (dungeon_visit_frequency[id] < VISIT_TIME_CAP / 10)
		dungeon_bonus[id] = 3;
	else if (dungeon_visit_frequency[id] < VISIT_TIME_CAP / 2)
		dungeon_bonus[id] = 2;
	else if (dungeon_visit_frequency[id] < (VISIT_TIME_CAP * 19) / 20)
		dungeon_bonus[id] = 1;
	else
		dungeon_bonus[id] = 0;
}
#endif

/*
 * Shuffle an array of integers using the Fisher-Yates algorithm.
 */
void intshuffle(int *array, int size) {
	int i, j, tmp;

	for (i = size - 1; i > 0; i--) {
		j = rand_int(i + 1);
		tmp = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
}

/* for all the dungeons/towers that are special, yet use the type 0 dungeon template - C. Blue
   todo: actually create own types for these. would also make DF3_JAIL_DUNGEON obsolete.
   If 'extra' is set, special info is added: Town name, to keep the two Angbands apart. */
char *get_dun_name(int x, int y, bool tower, dungeon_type *d_ptr, int type, bool extra) {
	static char *jail = "an escape tunnel"; //"a Jail Dungeon";
	static char *pvp_arena = "The PvP Arena";
	static char *highlander = "The Highlands";
	static char *irondeepdive = "The Ironman Deep Dive Challenge";

	/* hacks for 'extra' info, ugh */
	static char *angband_lothlorien = "Angband (Lothlorien)";
	static char *angband_khazaddum = "Angband (Khazad-dum)";

	if (d_ptr) type = d_ptr->type;

	/* normal dungeon */
	if (type) {
		/* hack for the two Angbands - so much hardcoding.... */
		if (extra && !strcmp(d_name + d_info[type].name, "Angband")) {
			if (town[TIDX_LORIEN].x == x && town[TIDX_LORIEN].y == y)
				return(angband_lothlorien);
			else return(angband_khazaddum);
		} else /* default */
			return(d_name + d_info[type].name);
	}

	/* special type 0 (yet) dungeons */
	if (x == WPOS_PVPARENA_X &&
	    y == WPOS_PVPARENA_Y &&
	    tower == (WPOS_PVPARENA_Z > 0))
		return(pvp_arena);

	if (x == WPOS_HIGHLANDER_X &&
	    y == WPOS_HIGHLANDER_Y &&
	    tower == (WPOS_HIGHLANDER_DUN_Z > 0))
		return(highlander);

	if (x == WPOS_IRONDEEPDIVE_X &&
	    y == WPOS_IRONDEEPDIVE_Y &&
	    tower == (WPOS_IRONDEEPDIVE_Z > 0))
		return(irondeepdive);

	if (d_ptr && (
#if 0 /* obsolete fortunately (/fixjaildun) */
	    /* ughhh */
	    (d_ptr->baselevel == 30 && d_ptr->maxdepth == 30 &&
	    (d_ptr->flags1 & DF1_FORGET) &&
	    (d_ptr->flags2 & DF2_IRON))
	    ||
#endif
	    /* yay */
	    (d_ptr->flags3 & DF3_JAIL_DUNGEON) ))
		return(jail);

	/* really just "Wilderness" */
	return(d_name + d_info[type].name);
}

/* Add gold to the player's gold, for easier handling. - C. Blue
   Returns FALSE if we already own 2E9 Au.
   quiet: No message.
   exempt: Allow picking up this money without gaining 1 XP. */
bool gain_au(int Ind, u32b amt, bool quiet, bool exempt) {
	player_type *p_ptr = Players[Ind];

	/* hack: prevent s32b overflow */
	if (PY_MAX_GOLD - amt < p_ptr->au) {
		if (!quiet) msg_format(Ind, "\377yYou cannot carry more than %d gold!", PY_MAX_GOLD);
		return(FALSE);
	} else {
#ifdef EVENT_TOWNIE_GOLD_LIMIT
		if ((p_ptr->mode & MODE_DED_IDDC) && !in_irondeepdive(&p_ptr->wpos)
 #ifdef DED_IDDC_MANDOS
		    && !in_hallsofmandos(&p_ptr->wpos)
 #endif
		    && p_ptr->gold_picked_up == EVENT_TOWNIE_GOLD_LIMIT) {
			msg_print(Ind, "\377yYou cannot collect any more cash or your life would be forfeit.");
			return(FALSE);
		}
#endif

		/* Collect the gold */
		p_ptr->au += amt;
		p_ptr->redraw |= (PR_GOLD);
	}

	if (exempt) return(TRUE);
#ifdef EVENT_TOWNIE_GOLD_LIMIT
	/* if EVENT_TOWNIE_GOLD_LIMIT is 0 then nothing happens */
	if (p_ptr->gold_picked_up <= EVENT_TOWNIE_GOLD_LIMIT) {
		p_ptr->gold_picked_up += (amt > EVENT_TOWNIE_GOLD_LIMIT) ? EVENT_TOWNIE_GOLD_LIMIT : amt;
		if (p_ptr->gold_picked_up > EVENT_TOWNIE_GOLD_LIMIT
		    && !p_ptr->max_exp && !in_irondeepdive(&p_ptr->wpos)) {
			msg_print(Ind, "You gain a tiny bit of experience from collecting cash.");
			gain_exp(Ind, 1);
		}
	}
#endif
	return(TRUE);
}

/* backup all house prices and contents for all players to lib/save/estate/.
   partial: don't stop with failure if a character files can't be read. */
#define ESTATE_BACKUP_VERSION "v9"
bool backup_estate(bool partial) {
	FILE *fp;
	char buf[MAX_PATH_LENGTH], buf2[MAX_PATH_LENGTH], savefile[CNAME_LEN], c;
	cptr name;
	int i, j, k;
	int sy, sx, ey,ex , x, y;
	cave_type **zcave, *c_ptr;
	bool newly_created, allocated;
	u32b au;
	house_type *h_ptr;
	struct dna_type *dna;
	struct worldpos *wpos;
	object_type *o_ptr;

	s_printf("Backing up all real estate...\n");
	path_build(buf2, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "estate");

	/* create folder lib/save/estate if not existing */
#if defined(WINDOWS) && !defined(CYGWIN)
	mkdir(buf2);
#else
	mkdir(buf2, 0770);
#endif

	/* scan all houses */
	for (i = 0; i < num_houses; i++) {
		h_ptr = &houses[i];
		if (!h_ptr->dna->owner) continue;

		wpos = &h_ptr->wpos;
		dna = h_ptr->dna;
		/* assume worst possible charisma (3 (0))? */
		/* assume decent charisma (18 (aka 15 as it starts at 3) -> 100%)
		   still a 25% gain is possible with this for those who bought it at *** CHR originally here.. >,>  */
		au = house_price_player(dna->price, 15);
		//h_ptr->dy,dx

		/* WARNING: For now unable to save guild halls */
		if (h_ptr->dna->owner_type != OT_PLAYER) continue;

		name = lookup_player_name(h_ptr->dna->owner);
		if (!name) {
			s_printf("  warning: house %d's owner %d doesn't have a name.\n", i, h_ptr->dna->owner);
			continue;
		}

		/* create backup file if required, or append to it */
		/* create actual filename from character name (same as used for sf_delete or process_player_name) */
		k = 0;
		for (j = 0; name[j]; j++) {
			c = name[j];
			/* Accept some letters */
			if (isalphanum(c)) savefile[k++] = c;
			/* Convert space, dot, and underscore to underscore */
			else if (strchr(SF_BAD_CHARS, c)) savefile[k++] = '_';
		}
		savefile[k] = '\0';
		/* build path name and try to create/append to player's backup file */
		path_build(buf, MAX_PATH_LENGTH, buf2, savefile);
		if ((fp = fopen(buf, "rb")) == NULL)
			newly_created = TRUE;
		else {
			newly_created = FALSE;
			fclose(fp);
		}
		if ((fp = fopen(buf, "ab")) == NULL) {
			s_printf("  error: cannot open file '%s'.\n", buf);
			if (partial) continue;
			return(FALSE);
		} else if (newly_created) {
			newly_created = FALSE;
			/* begin with a version tag */
			fprintf(fp, "%s\n", ESTATE_BACKUP_VERSION);

#if 0 /* guild info is no longer tied to map reset! */
			/* add 2M Au if he's a guild master, since guilds will be erased if the server
			   savefile gets deleted (which is the sole purpose of calling this function..) */
			for (j = 1; j < MAX_GUILDS; j++)
				if (guilds[j].master == h_ptr->dna->owner) {
					fprintf(fp, "AU:%d\n", GUILD_PRICE);
					s_printf("  guild master: '%s'.\n", name);
					break;
				}
#endif
		}

		/* add house price to his backup file */
		fprintf(fp, "AU:%d\n", au);

		/* scan house contents and add them to his backup file */
		/* traditional house? */
		if (h_ptr->flags & HF_TRAD) {
			for (j = 0; j < h_ptr->stock_num; j++) {
				o_ptr = &h_ptr->stock[j];
				/* add object to backup file */
				fprintf(fp, "OB:");
				(void)fwrite(o_ptr, sizeof(object_type), 1, fp);
				/* store inscription too! */
				if (o_ptr->note) {
					fprintf(fp, "%d\n", (int)strlen(quark_str(o_ptr->note)));
					(void)fwrite(quark_str(o_ptr->note), sizeof(char), strlen(quark_str(o_ptr->note)), fp);
				} else
					fprintf(fp, "%d\n", -1);
			}
		}
		/* mang-style house? */
		else {
			/* allocate sector of the house to access objects */
			if (!(zcave = getcave(wpos))) {
				alloc_dungeon_level(wpos);
				wilderness_gen(wpos);
				if (!(zcave = getcave(wpos))) {
					s_printf("  error: cannot getcave(%d,%d,%d).\nfailed.\n", wpos->wx, wpos->wy, wpos->wz);
					fclose(fp);
					return(FALSE);
				}
				allocated = TRUE;
			} else allocated = FALSE;

			if (h_ptr->flags & HF_RECT) {
				sy = h_ptr->y + 1;
				sx = h_ptr->x + 1;
				ey = h_ptr->y + h_ptr->coords.rect.height - 1;
				ex = h_ptr->x + h_ptr->coords.rect.width - 1;
				for (y = sy; y < ey; y++) {
					for (x = sx; x < ex; x++) {
						c_ptr = &zcave[y][x];
						if (c_ptr->o_idx) {
							o_ptr = &o_list[c_ptr->o_idx];
							/* add object to backup file */
							fprintf(fp, "OB:");
							(void)fwrite(o_ptr, sizeof(object_type), 1, fp);
							/* store inscription too! */
							if (o_ptr->note) {
								fprintf(fp, "%d\n", (int)strlen(quark_str(o_ptr->note)));
								(void)fwrite(quark_str(o_ptr->note), sizeof(char), strlen(quark_str(o_ptr->note)), fp);
							} else
								fprintf(fp, "%d\n", -1);
						}
					}
				}
			} else {
				/* Polygonal house */
				//fill_house(h_ptr, FILL_CLEAR, NULL);
			}

			if (allocated) dealloc_dungeon_level(wpos);
		}

		/* done with this particular house */
		fclose(fp);
	}

	s_printf("done.\n");
	return(TRUE);
}
/* Backup all houses that belong to a specific character and give content ownership to another character:
   h_id = character who owns the houses, id = character name to use for the estate savefile. */
bool backup_char_estate(int Ind, s32b h_id, cptr target_name) {
	int i;
	bool res = TRUE;

	for (i = 0; i < num_houses; i++) {
		struct dna_type *dna = houses[i].dna;

		if (!dna->owner) continue; /* not owned */
		if ((dna->owner_type == OT_PLAYER) && (dna->owner == h_id)) {
			if (Ind) msg_format(Ind, "House %d at (%d,%d) %d,%d:", i, houses[i].wpos.wx, houses[i].wpos.wy, houses[i].dx, houses[i].dy);
			if (!backup_one_estate(&houses[i].wpos, houses[i].dx, houses[i].dy, -1, target_name)) res = FALSE;
		}
	}
	return(res);
}
/* Backup one house and give content ownership to a specific character.
   If h_idx is not -1, ie a house index is provided, hwpos/hx/hy will all be ignored,
   and instead derived from the house of the index h_idx. */
bool backup_one_estate(struct worldpos *hwpos, int hx, int hy, int h_idx, cptr target_name) {
	FILE *fp;
	char buf[MAX_PATH_LENGTH], buf2[MAX_PATH_LENGTH], savefile[CNAME_LEN], c;
	int i, j, k;
	int sy, sx, ey,ex , x, y;
	cave_type **zcave, *c_ptr;
	bool newly_created, allocated;
	house_type *h_ptr;
	struct worldpos *wpos;
	object_type *o_ptr;

	/* If h_idx is specified, ignore hx, hy */
	if (h_idx != -1) {
		i = h_idx;
		h_ptr = &houses[i];

		hx = h_ptr->dx;
		hy = h_ptr->dy;
		wpos = &h_ptr->wpos;

		s_printf("Backing up a house via h_idx (%d,%d,%d - %d,%d (%d) - %s)... ", wpos->wx, wpos->wy, wpos->wz, hx, hy, i, target_name);
	} else {
		s_printf("Backing up a house via d-x/y (%d,%d,%d - %d,%d ", hwpos->wx, hwpos->wy, hwpos->wz, hx, hy);

		/* scan house on which door we're sitting */
		i = pick_house(hwpos, hy, hx);
		if (i == -1) {
			s_printf("(%d) - %s)... ", i, target_name);
			s_printf(" error: No estate here.\n");
			return(FALSE);
		}

		s_printf("(%d)- %s)... ", i, target_name);
		h_ptr = &houses[i];
		wpos = &h_ptr->wpos;
	}

	path_build(buf2, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "estate");
	/* create folder lib/save/estate if not existing */
#if defined(WINDOWS) && !defined(CYGWIN)
	mkdir(buf2);
#else
	mkdir(buf2, 0770);
#endif

	/* create backup file if required, or append to it */
	/* create actual filename from character name (same as used for sf_delete or process_player_name) */
	k = 0;
	for (j = 0; target_name[j]; j++) {
		c = target_name[j];
		/* Accept some letters */
		if (isalphanum(c)) savefile[k++] = c;
		/* Convert space, dot, and underscore to underscore */
		else if (strchr(SF_BAD_CHARS, c)) savefile[k++] = '_';
	}
	savefile[k] = '\0';
	/* build path name and try to create/append to player's backup file */
	path_build(buf, MAX_PATH_LENGTH, buf2, savefile);
	if ((fp = fopen(buf, "rb")) == NULL)
		newly_created = TRUE;
	else {
		newly_created = FALSE;
		fclose(fp);
	}
	if ((fp = fopen(buf, "ab")) == NULL) {
		s_printf(" error: cannot open file '%s'.\n", buf);
		return(FALSE);
	} else if (newly_created) {
		newly_created = FALSE;
		/* begin with a version tag */
		fprintf(fp, "%s\n", ESTATE_BACKUP_VERSION);
	}

#if 0
	/* add house price to his backup file -- nope */
	fprintf(fp, "AU:%d\n", 1); //0 Au = 'corrupted' error ;-p
#else /* as this function is called for character-erasure backups, we need the house cost here too */
	/* assume worst possible charisma (3 (0))? */
	/* assume decent charisma (18 (aka 15 as it starts at 3) -> 100%)
	   still a 25% gain is possible with this for those who bought it at *** CHR originally here.. >,>  */
	fprintf(fp, "AU:%d\n", house_price_player(h_ptr->dna->price, 15));
#endif

	/* scan house contents and add them to his backup file */
	/* traditional house? */
	if (h_ptr->flags & HF_TRAD) {
		for (j = 0; j < h_ptr->stock_num; j++) {
			o_ptr = &h_ptr->stock[j];
			/* add object to backup file */
			fprintf(fp, "OB:");
			(void)fwrite(o_ptr, sizeof(object_type), 1, fp);
			/* store inscription too! */
			if (o_ptr->note) {
				fprintf(fp, "%d\n", (int)strlen(quark_str(o_ptr->note)));
				(void)fwrite(quark_str(o_ptr->note), sizeof(char), strlen(quark_str(o_ptr->note)), fp);
			} else
				fprintf(fp, "%d\n", -1);
		}
	}
	/* mang-style house? */
	else {
		/* allocate sector of the house to access objects */
		if (!(zcave = getcave(wpos))) {
			alloc_dungeon_level(wpos);
			wilderness_gen(wpos);
			if (!(zcave = getcave(wpos))) {
				s_printf(" error: cannot getcave(%d,%d,%d).\nfailed.\n", wpos->wx, wpos->wy, wpos->wz);
				fclose(fp);
				return(FALSE);
			}
			allocated = TRUE;
		} else allocated = FALSE;

		if (h_ptr->flags & HF_RECT) {
			sy = h_ptr->y + 1;
			sx = h_ptr->x + 1;
			ey = h_ptr->y + h_ptr->coords.rect.height - 1;
			ex = h_ptr->x + h_ptr->coords.rect.width - 1;

			for (y = sy; y < ey; y++) {
				for (x = sx; x < ex; x++) {
					c_ptr = &zcave[y][x];
					if (c_ptr->o_idx) {
						o_ptr = &o_list[c_ptr->o_idx];
						/* add object to backup file */
						fprintf(fp, "OB:");
						(void)fwrite(o_ptr, sizeof(object_type), 1, fp);
						/* store inscription too! */
						if (o_ptr->note) {
							fprintf(fp, "%d\n", (int)strlen(quark_str(o_ptr->note)));
							(void)fwrite(quark_str(o_ptr->note), sizeof(char), strlen(quark_str(o_ptr->note)), fp);
						} else
							fprintf(fp, "%d\n", -1);
					}
				}
			}
		} else {
			/* Polygonal house */
			//fill_house(h_ptr, FILL_CLEAR, NULL);
		}

		if (allocated) dealloc_dungeon_level(wpos);
	}

	/* done with this particular house */
	fclose(fp);

	s_printf(" done.\n");
	return(TRUE);
}

/* helper function for restore_estate():
   copy all remaining data from our save file to the temporary file
   and turn the temporary file into the new save file. */
void relay_estate(char *buf, char *buf2, FILE *fp, FILE *fp_tmp) {
	int c;

	/* relay the remaining data, if any */
	while ((c = fgetc(fp)) != EOF) fputc(c, fp_tmp);

	/* done */
	fclose(fp);
	fclose(fp_tmp);

	/* erase old save file, make temporary file the new save file */
	remove(buf);
	rename(buf2, buf);
}

/* get back the backed-up real estate from files */
void restore_estate(int Ind) {
	player_type *p_ptr = Players[Ind];
	FILE *fp, *fp_tmp;
	char buf[MAX_PATH_LENGTH], buf2[MAX_PATH_LENGTH], version[MAX_CHARS];
	char data[4], data_note[MSG_LEN];//MAX_OLEN?
	char o_name[MSG_LEN], *rc;
	unsigned long au;
	int data_len, r;
	object_type forge, *o_ptr = &forge;
	bool gained_anything = FALSE;

	int conversion = 0;
	object_type_v8 forge_v8, *o_ptr_v8 = &forge_v8;
	object_type_v7 forge_v7, *o_ptr_v7 = &forge_v7;
	object_type_v6 forge_v6, *o_ptr_v6 = &forge_v6;
	object_type_v5 forge_v5, *o_ptr_v5 = &forge_v5;
	object_type_v4 forge_v4, *o_ptr_v4 = &forge_v4;
	object_type_v3 forge_v3, *o_ptr_v3 = &forge_v3;
	object_type_v2 forge_v2, *o_ptr_v2 = &forge_v2;
	object_type_v2a forge_v2a, *o_ptr_v2a = &forge_v2a;
	object_type_v2b forge_v2b, *o_ptr_v2b = &forge_v2b;


	s_printf("Restoring real estate for %s...\n", p_ptr->name);

	/* create folder lib/save/estate if not existing */
	path_build(buf2, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "estate");

	/* build path name and try to create/append to player's backup file */
	path_build(buf, MAX_PATH_LENGTH, buf2, p_ptr->basename);
	if ((fp = fopen(buf, "rb")) == NULL) {
		s_printf("  error: No file '%s' exists to request from.\nfailed.\n", buf);
		msg_print(Ind, "\377yNo goods or money stored to request.");
		return;
	}
	s_printf("  opened file '%s'.\n", buf);

	/* Try to read version string */
	if (fgets(version, MAX_CHARS, fp) == NULL) {
		s_printf("  error: File is empty.\nfailed.\n");
		msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
		fclose(fp);
		return;
	}
	if (*version) version[strlen(version) - 1] = '\0';
	s_printf("  reading a version '%s' estate file.\n", version);

	/* convert older version object types */
	if (streq(version, ESTATE_BACKUP_VERSION)) conversion = 0;
	else if (streq(version, "v2")) conversion = 1;
	else if (streq(version, "v2a")) conversion = 2;
	else if (streq(version, "v2b")) conversion = 3;
	else if (streq(version, "v3")) conversion = 4;
	else if (streq(version, "v4")) conversion = 5;
	else if (streq(version, "v5")) conversion = 6;
	else if (streq(version, "v6")) conversion = 7;
	else if (streq(version, "v7")) conversion = 8;
	else if (streq(version, "v8")) conversion = 9;
	else {
		s_printf("  error: Invalid backup version.\nfailed.\n");
		msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
		fclose(fp);
		return;
	}

	/* open temporary file for writing the stuff the player left over */
	strcpy(buf2, buf);
	strcat(buf2, ".$$$");
	if ((fp_tmp = fopen(buf2, "wb")) == NULL) {
		s_printf("  error: cannot open temporary file '%s'.\nfailed.\n", buf2);
		msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
		fclose(fp);
		return;
	}

	/* relay to temporary file, do not convert its version,
	   but make it a 1:1 copy really except for items/gold the player received correctly */
	fprintf(fp_tmp, "%s\n", version);

	msg_print(Ind, "\377yChecking for money and items from estate stored for you...");
	while (TRUE) {
		/* scan file for either house price (AU:) or object (OB:) */
		if (!fread(data, sizeof(char), 3, fp)) {
			if (!gained_anything) {
				s_printf("  nothing to request.\ndone.\n");
				msg_print(Ind, "\377yNo goods or money left to request.");
			} else {
				s_printf("done.\n");
				msg_print(Ind, "\377yYou received everything that was stored for you.");
			}
			relay_estate(buf, buf2, fp, fp_tmp);
			return;
		}
		data[3] = '\0';

		/* get house price from backup file */
		if (!strcmp(data, "AU:")) {
			au = 0;
			r = fscanf(fp, "%lu\n", &au);
			if (r == EOF || r == 0 || !au) {
				s_printf("  error: Corrupted AU: line.\n");
				msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
				relay_estate(buf, buf2, fp, fp_tmp);
				return;
			}

			/* give gold to player if it doesn't overflow,
			   otherwise relay rest to temporary file, swap and exit */
			if (!gain_au(Ind, au, FALSE, FALSE)) {
				msg_print(Ind, "\377yDrop/deposite some gold to be able to receive more gold.");

				/* write failed gold gain back into new buffer file */
				fprintf(fp_tmp, "AU:%lu\n", au);

				relay_estate(buf, buf2, fp, fp_tmp);
				return;
			}
			gained_anything = TRUE;
			s_printf("  gained %ld Au.\n", au);
			msg_format(Ind, "You receive %ld gold pieces.", au);
			continue;
		}
		/* get object from backup file */
		else if (!strcmp(data, "OB:")) {
			switch (conversion) {
			case 0: r = fread(o_ptr, sizeof(object_type), 1, fp);
				break;
			case 1: r = fread(o_ptr_v2, sizeof(object_type_v2), 1, fp);
				o_ptr->owner = o_ptr_v2->owner;
				o_ptr->killer = o_ptr_v2->killer;
				o_ptr->level = o_ptr_v2->level;
				o_ptr->k_idx = o_ptr_v2->k_idx;
				o_ptr->h_idx = o_ptr_v2->h_idx;
				o_ptr->wpos = o_ptr_v2->wpos;
				o_ptr->iy = o_ptr_v2->iy;
				o_ptr->ix = o_ptr_v2->ix;
				o_ptr->tval = o_ptr_v2->tval;
				o_ptr->sval = o_ptr_v2->sval;
				o_ptr->tval2 = o_ptr_v2->tval2;
				o_ptr->sval2 = o_ptr_v2->sval2;
				o_ptr->bpval = o_ptr_v2->bpval;
				o_ptr->pval = o_ptr_v2->pval;
				o_ptr->pval2 = o_ptr_v2->pval2;
				o_ptr->pval3 = o_ptr_v2->pval3;
				o_ptr->pval_org = o_ptr_v2->pval_org;
				o_ptr->bpval_org = o_ptr_v2->bpval_org;
				o_ptr->to_h_org = o_ptr_v2->to_h_org;
				o_ptr->to_d_org = o_ptr_v2->to_d_org;
				o_ptr->to_a_org = o_ptr_v2->to_a_org;
				o_ptr->sigil = o_ptr_v2->sigil;
				o_ptr->sseed = o_ptr_v2->sseed;
				o_ptr->discount = o_ptr_v2->discount;
				o_ptr->number = o_ptr_v2->number;
				o_ptr->weight = o_ptr_v2->weight;
				o_ptr->name1 = o_ptr_v2->name1;
				o_ptr->name2 = o_ptr_v2->name2;
				o_ptr->name2b = o_ptr_v2->name2b;
				o_ptr->name3 = o_ptr_v2->name3;
				o_ptr->name4 = o_ptr_v2->name4;
				o_ptr->attr = o_ptr_v2->attr;
				o_ptr->mode = o_ptr_v2->mode;
				o_ptr->xtra1 = o_ptr_v2->xtra1;
				o_ptr->xtra2 = o_ptr_v2->xtra2;
				o_ptr->xtra3 = o_ptr_v2->xtra3;
				o_ptr->xtra4 = o_ptr_v2->xtra4;
				o_ptr->xtra5 = o_ptr_v2->xtra5;
				o_ptr->xtra6 = o_ptr_v2->xtra6;
				o_ptr->xtra7 = o_ptr_v2->xtra7;
				o_ptr->xtra8 = o_ptr_v2->xtra8;
				o_ptr->xtra9 = o_ptr_v2->xtra9;
				o_ptr->uses_dir = o_ptr_v2->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v2->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v2->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v2->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v2->to_h;
				o_ptr->to_d = o_ptr_v2->to_d;
				o_ptr->to_a = o_ptr_v2->to_a;
				o_ptr->ac = o_ptr_v2->ac;
				o_ptr->dd = o_ptr_v2->dd;
				o_ptr->ds = o_ptr_v2->ds;
				o_ptr->ident = o_ptr_v2->ident;
				o_ptr->timeout = o_ptr_v2->timeout;
				o_ptr->timeout_magic = o_ptr_v2->timeout_magic;
				o_ptr->recharging = o_ptr_v2->recharging;
				o_ptr->marked = o_ptr_v2->marked;
				o_ptr->marked2 = o_ptr_v2->marked2;
				o_ptr->questor = o_ptr_v2->questor;
				o_ptr->quest = o_ptr_v2->quest;
				o_ptr->quest_stage = o_ptr_v2->quest_stage;
				o_ptr->questor_idx = o_ptr_v2->questor_idx;
				o_ptr->questor_invincible = o_ptr_v2->questor_invincible;
				o_ptr->quest_credited = o_ptr_v2->quest_credited;
				o_ptr->note_utag = o_ptr_v2->note_utag;
				o_ptr->inven_order = o_ptr_v2->inven_order;
				o_ptr->next_o_idx = o_ptr_v2->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v2->held_m_idx;
				o_ptr->auto_insc = o_ptr_v2->auto_insc;
				o_ptr->stack_pos = o_ptr_v2->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v2->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v2->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v2->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v2->housed;
				o_ptr->changed = o_ptr_v2->changed;
				o_ptr->NR_tradable = o_ptr_v2->NR_tradable;
				o_ptr->temp = o_ptr_v2->temp;
				o_ptr->iron_trade = o_ptr_v2->iron_trade;
				o_ptr->iron_turn = o_ptr_v2->iron_turn;
				//convert; (2)
				o_ptr->no_soloist = FALSE;
				o_ptr->iron_turn = 0;
				//convert: (3)
				o_ptr->note = 0;
				 //convert: (4)
				o_ptr->embed = 0;
				//convert: (5)
				o_ptr->id = o_ptr->f_id = o_ptr->f_name[0] = 0; //don't generate an id here, whatever
				o_ptr->f_turn = o_ptr->f_time = 0;
				o_ptr->f_wpos = (struct worldpos){ 0, 0, 0 };
				o_ptr->f_dun = 0;
				o_ptr->f_player = o_ptr->f_player_turn = 0;
				o_ptr->f_ridx = o_ptr->f_reidx = 0;
				o_ptr->f_special = o_ptr->f_reward = 0;
				//convert: (6)
				o_ptr->number2 = 0;
				o_ptr->note2 = o_ptr->note2_utag = 0;
				//convert: (7)
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v2->mode; /* u16b = byte */
				break;
			case 2: r = fread(o_ptr_v2a, sizeof(object_type_v2a), 1, fp);
				o_ptr->owner = o_ptr_v2a->owner;
				o_ptr->killer = o_ptr_v2a->killer;
				o_ptr->level = o_ptr_v2a->level;
				o_ptr->k_idx = o_ptr_v2a->k_idx;
				o_ptr->h_idx = o_ptr_v2a->h_idx;
				o_ptr->wpos = o_ptr_v2a->wpos;
				o_ptr->iy = o_ptr_v2a->iy;
				o_ptr->ix = o_ptr_v2a->ix;
				o_ptr->tval = o_ptr_v2a->tval;
				o_ptr->sval = o_ptr_v2a->sval;
				o_ptr->tval2 = o_ptr_v2a->tval2;
				o_ptr->sval2 = o_ptr_v2a->sval2;
				o_ptr->bpval = o_ptr_v2a->bpval;
				o_ptr->pval = o_ptr_v2a->pval;
				o_ptr->pval2 = o_ptr_v2a->pval2;
				o_ptr->pval3 = o_ptr_v2a->pval3;
				o_ptr->pval_org = o_ptr_v2a->pval_org;
				o_ptr->bpval_org = o_ptr_v2a->bpval_org;
				o_ptr->to_h_org = o_ptr_v2a->to_h_org;
				o_ptr->to_d_org = o_ptr_v2a->to_d_org;
				o_ptr->to_a_org = o_ptr_v2a->to_a_org;
				o_ptr->sigil = o_ptr_v2a->sigil;
				o_ptr->sseed = o_ptr_v2a->sseed;
				o_ptr->discount = o_ptr_v2a->discount;
				o_ptr->number = o_ptr_v2a->number;
				o_ptr->weight = o_ptr_v2a->weight;
				o_ptr->name1 = o_ptr_v2a->name1;
				o_ptr->name2 = o_ptr_v2a->name2;
				o_ptr->name2b = o_ptr_v2a->name2b;
				o_ptr->name3 = o_ptr_v2a->name3;
				o_ptr->name4 = o_ptr_v2a->name4;
				o_ptr->attr = o_ptr_v2a->attr;
				o_ptr->mode = o_ptr_v2a->mode;
				o_ptr->xtra1 = o_ptr_v2a->xtra1;
				o_ptr->xtra2 = o_ptr_v2a->xtra2;
				o_ptr->xtra3 = o_ptr_v2a->xtra3;
				o_ptr->xtra4 = o_ptr_v2a->xtra4;
				o_ptr->xtra5 = o_ptr_v2a->xtra5;
				o_ptr->xtra6 = o_ptr_v2a->xtra6;
				o_ptr->xtra7 = o_ptr_v2a->xtra7;
				o_ptr->xtra8 = o_ptr_v2a->xtra8;
				o_ptr->xtra9 = o_ptr_v2a->xtra9;
				o_ptr->uses_dir = o_ptr_v2a->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v2a->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v2a->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v2a->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v2a->to_h;
				o_ptr->to_d = o_ptr_v2a->to_d;
				o_ptr->to_a = o_ptr_v2a->to_a;
				o_ptr->ac = o_ptr_v2a->ac;
				o_ptr->dd = o_ptr_v2a->dd;
				o_ptr->ds = o_ptr_v2a->ds;
				o_ptr->ident = o_ptr_v2a->ident;
				o_ptr->timeout = o_ptr_v2a->timeout;
				o_ptr->timeout_magic = o_ptr_v2a->timeout_magic;
				o_ptr->recharging = o_ptr_v2a->recharging;
				o_ptr->marked = o_ptr_v2a->marked;
				o_ptr->marked2 = o_ptr_v2a->marked2;
				o_ptr->questor = o_ptr_v2a->questor;
				o_ptr->quest = o_ptr_v2a->quest;
				o_ptr->quest_stage = o_ptr_v2a->quest_stage;
				o_ptr->questor_idx = o_ptr_v2a->questor_idx;
				o_ptr->questor_invincible = o_ptr_v2a->questor_invincible;
				o_ptr->quest_credited = o_ptr_v2a->quest_credited;
				o_ptr->note_utag = o_ptr_v2a->note_utag;
				o_ptr->inven_order = o_ptr_v2a->inven_order;
				o_ptr->next_o_idx = o_ptr_v2a->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v2a->held_m_idx;
				o_ptr->auto_insc = o_ptr_v2a->auto_insc;
				o_ptr->stack_pos = o_ptr_v2a->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v2a->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v2a->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v2a->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v2a->housed;
				o_ptr->changed = o_ptr_v2a->changed;
				o_ptr->NR_tradable = o_ptr_v2a->NR_tradable;
				o_ptr->temp = o_ptr_v2a->temp;
				o_ptr->iron_trade = o_ptr_v2a->iron_trade;
				//convert;
				o_ptr->no_soloist = FALSE;
				o_ptr->iron_turn = 0;
				//convert: (3)
				o_ptr->note = 0;
				 //convert: (4)
				o_ptr->embed = 0;
				//convert: (5)
				o_ptr->id = o_ptr->f_id = o_ptr->f_name[0] = 0; //don't generate an id here, whatever
				o_ptr->f_turn = o_ptr->f_time = 0;
				o_ptr->f_wpos = (struct worldpos){ 0, 0, 0 };
				o_ptr->f_dun = 0;
				o_ptr->f_player = o_ptr->f_player_turn = 0;
				o_ptr->f_ridx = o_ptr->f_reidx = 0;
				o_ptr->f_special = o_ptr->f_reward = 0;
				//convert: (6)
				o_ptr->number2 = 0;
				o_ptr->note2 = o_ptr->note2_utag = 0;
				//convert: (7)
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v2a->mode; /* u16b = byte */
				break;
			case 3: r = fread(o_ptr_v2b, sizeof(object_type_v2b), 1, fp);
				o_ptr->owner = o_ptr_v2b->owner;
				o_ptr->killer = o_ptr_v2b->killer;
				o_ptr->level = o_ptr_v2b->level;
				o_ptr->k_idx = o_ptr_v2b->k_idx;
				o_ptr->h_idx = o_ptr_v2b->h_idx;
				o_ptr->wpos = o_ptr_v2b->wpos;
				o_ptr->iy = o_ptr_v2b->iy;
				o_ptr->ix = o_ptr_v2b->ix;
				o_ptr->tval = o_ptr_v2b->tval;
				o_ptr->sval = o_ptr_v2b->sval;
				o_ptr->tval2 = o_ptr_v2b->tval2;
				o_ptr->sval2 = o_ptr_v2b->sval2;
				o_ptr->bpval = o_ptr_v2b->bpval;
				o_ptr->pval = o_ptr_v2b->pval;
				o_ptr->pval2 = o_ptr_v2b->pval2;
				o_ptr->pval3 = o_ptr_v2b->pval3;
				o_ptr->pval_org = o_ptr_v2b->pval_org;
				o_ptr->bpval_org = o_ptr_v2b->bpval_org;
				o_ptr->to_h_org = o_ptr_v2b->to_h_org;
				o_ptr->to_d_org = o_ptr_v2b->to_d_org;
				o_ptr->to_a_org = o_ptr_v2b->to_a_org;
				o_ptr->sigil = o_ptr_v2b->sigil;
				o_ptr->sseed = o_ptr_v2b->sseed;
				o_ptr->discount = o_ptr_v2b->discount;
				o_ptr->number = o_ptr_v2b->number;
				o_ptr->weight = o_ptr_v2b->weight;
				o_ptr->name1 = o_ptr_v2b->name1;
				o_ptr->name2 = o_ptr_v2b->name2;
				o_ptr->name2b = o_ptr_v2b->name2b;
				o_ptr->name3 = o_ptr_v2b->name3;
				o_ptr->name4 = o_ptr_v2b->name4;
				o_ptr->attr = o_ptr_v2b->attr;
				o_ptr->mode = o_ptr_v2b->mode;
				o_ptr->xtra1 = o_ptr_v2b->xtra1;
				o_ptr->xtra2 = o_ptr_v2b->xtra2;
				o_ptr->xtra3 = o_ptr_v2b->xtra3;
				o_ptr->xtra4 = o_ptr_v2b->xtra4;
				o_ptr->xtra5 = o_ptr_v2b->xtra5;
				o_ptr->xtra6 = o_ptr_v2b->xtra6;
				o_ptr->xtra7 = o_ptr_v2b->xtra7;
				o_ptr->xtra8 = o_ptr_v2b->xtra8;
				o_ptr->xtra9 = o_ptr_v2b->xtra9;
				o_ptr->uses_dir = o_ptr_v2b->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v2b->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v2b->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v2b->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v2b->to_h;
				o_ptr->to_d = o_ptr_v2b->to_d;
				o_ptr->to_a = o_ptr_v2b->to_a;
				o_ptr->ac = o_ptr_v2b->ac;
				o_ptr->dd = o_ptr_v2b->dd;
				o_ptr->ds = o_ptr_v2b->ds;
				o_ptr->ident = o_ptr_v2b->ident;
				o_ptr->timeout = o_ptr_v2b->timeout;
				o_ptr->timeout_magic = o_ptr_v2b->timeout_magic;
				o_ptr->recharging = o_ptr_v2b->recharging;
				o_ptr->marked = o_ptr_v2b->marked;
				o_ptr->marked2 = o_ptr_v2b->marked2;
				o_ptr->questor = o_ptr_v2b->questor;
				o_ptr->quest = o_ptr_v2b->quest;
				o_ptr->quest_stage = o_ptr_v2b->quest_stage;
				o_ptr->questor_idx = o_ptr_v2b->questor_idx;
				o_ptr->questor_invincible = o_ptr_v2b->questor_invincible;
				o_ptr->quest_credited = o_ptr_v2b->quest_credited;
				o_ptr->note_utag = o_ptr_v2b->note_utag;
				o_ptr->inven_order = o_ptr_v2b->inven_order;
				o_ptr->next_o_idx = o_ptr_v2b->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v2b->held_m_idx;
				o_ptr->auto_insc = o_ptr_v2b->auto_insc;
				o_ptr->stack_pos = o_ptr_v2b->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v2b->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v2b->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v2b->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v2b->housed;
				o_ptr->changed = o_ptr_v2b->changed;
				o_ptr->NR_tradable = o_ptr_v2b->NR_tradable;
				o_ptr->no_soloist = o_ptr_v2b->no_soloist;
				o_ptr->temp = o_ptr_v2b->temp;
				o_ptr->iron_trade = o_ptr_v2b->iron_trade;
				o_ptr->iron_turn = o_ptr_v2b->iron_turn;
				//convert:
				o_ptr->note = 0;
				 //convert: (4)
				o_ptr->embed = 0;
				//convert: (5)
				o_ptr->id = o_ptr->f_id = o_ptr->f_name[0] = 0; //don't generate an id here, whatever
				o_ptr->f_turn = o_ptr->f_time = 0;
				o_ptr->f_wpos = (struct worldpos){ 0, 0, 0 };
				o_ptr->f_dun = 0;
				o_ptr->f_player = o_ptr->f_player_turn = 0;
				o_ptr->f_ridx = o_ptr->f_reidx = 0;
				o_ptr->f_special = o_ptr->f_reward = 0;
				//convert: (6)
				o_ptr->number2 = 0;
				o_ptr->note2 = o_ptr->note2_utag = 0;
				//convert: (7)
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v2b->mode; /* u16b = byte */
				break;
			case 4: r = fread(o_ptr_v3, sizeof(object_type_v3), 1, fp);
				o_ptr->owner = o_ptr_v3->owner;
				o_ptr->killer = o_ptr_v3->killer;
				o_ptr->level = o_ptr_v3->level;
				o_ptr->k_idx = o_ptr_v3->k_idx;
				o_ptr->h_idx = o_ptr_v3->h_idx;
				o_ptr->wpos = o_ptr_v3->wpos;
				o_ptr->iy = o_ptr_v3->iy;
				o_ptr->ix = o_ptr_v3->ix;
				o_ptr->tval = o_ptr_v3->tval;
				o_ptr->sval = o_ptr_v3->sval;
				o_ptr->tval2 = o_ptr_v3->tval2;
				o_ptr->sval2 = o_ptr_v3->sval2;
				o_ptr->bpval = o_ptr_v3->bpval;
				o_ptr->pval = o_ptr_v3->pval;
				o_ptr->pval2 = o_ptr_v3->pval2;
				o_ptr->pval3 = o_ptr_v3->pval3;
				o_ptr->pval_org = o_ptr_v3->pval_org;
				o_ptr->bpval_org = o_ptr_v3->bpval_org;
				o_ptr->to_h_org = o_ptr_v3->to_h_org;
				o_ptr->to_d_org = o_ptr_v3->to_d_org;
				o_ptr->to_a_org = o_ptr_v3->to_a_org;
				o_ptr->sigil = o_ptr_v3->sigil;
				o_ptr->sseed = o_ptr_v3->sseed;
				o_ptr->discount = o_ptr_v3->discount;
				o_ptr->number = o_ptr_v3->number;
				o_ptr->weight = o_ptr_v3->weight;
				o_ptr->name1 = o_ptr_v3->name1;
				o_ptr->name2 = o_ptr_v3->name2;
				o_ptr->name2b = o_ptr_v3->name2b;
				o_ptr->name3 = o_ptr_v3->name3;
				o_ptr->name4 = o_ptr_v3->name4;
				o_ptr->attr = o_ptr_v3->attr;
				o_ptr->mode = o_ptr_v3->mode;
				o_ptr->xtra1 = o_ptr_v3->xtra1;
				o_ptr->xtra2 = o_ptr_v3->xtra2;
				o_ptr->xtra3 = o_ptr_v3->xtra3;
				o_ptr->xtra4 = o_ptr_v3->xtra4;
				o_ptr->xtra5 = o_ptr_v3->xtra5;
				o_ptr->xtra6 = o_ptr_v3->xtra6;
				o_ptr->xtra7 = o_ptr_v3->xtra7;
				o_ptr->xtra8 = o_ptr_v3->xtra8;
				o_ptr->xtra9 = o_ptr_v3->xtra9;
				o_ptr->uses_dir = o_ptr_v3->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v3->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v3->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v3->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v3->to_h;
				o_ptr->to_d = o_ptr_v3->to_d;
				o_ptr->to_a = o_ptr_v3->to_a;
				o_ptr->ac = o_ptr_v3->ac;
				o_ptr->dd = o_ptr_v3->dd;
				o_ptr->ds = o_ptr_v3->ds;
				o_ptr->ident = o_ptr_v3->ident;
				o_ptr->timeout = o_ptr_v3->timeout;
				o_ptr->timeout_magic = o_ptr_v3->timeout_magic;
				o_ptr->recharging = o_ptr_v3->recharging;
				o_ptr->marked = o_ptr_v3->marked;
				o_ptr->marked2 = o_ptr_v3->marked2;
				o_ptr->questor = o_ptr_v3->questor;
				o_ptr->quest = o_ptr_v3->quest;
				o_ptr->quest_stage = o_ptr_v3->quest_stage;
				o_ptr->questor_idx = o_ptr_v3->questor_idx;
				o_ptr->questor_invincible = o_ptr_v3->questor_invincible;
				o_ptr->quest_credited = o_ptr_v3->quest_credited;
				o_ptr->note = o_ptr_v3->note;
				o_ptr->note_utag = o_ptr_v3->note_utag;
				o_ptr->inven_order = o_ptr_v3->inven_order;
				o_ptr->next_o_idx = o_ptr_v3->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v3->held_m_idx;
				o_ptr->auto_insc = o_ptr_v3->auto_insc;
				o_ptr->stack_pos = o_ptr_v3->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v3->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v3->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v3->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v3->housed;
				o_ptr->changed = o_ptr_v3->changed;
				o_ptr->NR_tradable = o_ptr_v3->NR_tradable;
				o_ptr->no_soloist = o_ptr_v3->no_soloist;
				o_ptr->temp = o_ptr_v3->temp;
				o_ptr->iron_trade = o_ptr_v3->iron_trade;
				o_ptr->iron_turn = o_ptr_v3->iron_turn;
				//convert:
				o_ptr->embed = 0;
				//convert: (5)
				o_ptr->id = o_ptr->f_id = o_ptr->f_name[0] = 0; //don't generate an id here, whatever
				o_ptr->f_turn = o_ptr->f_time = 0;
				o_ptr->f_wpos = (struct worldpos){ 0, 0, 0 };
				o_ptr->f_dun = 0;
				o_ptr->f_player = o_ptr->f_player_turn = 0;
				o_ptr->f_ridx = o_ptr->f_reidx = 0;
				o_ptr->f_special = o_ptr->f_reward = 0;
				//convert: (6)
				o_ptr->number2 = 0;
				o_ptr->note2 = o_ptr->note2_utag = 0;
				//convert: (7)
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v3->mode; /* u16b = byte */
				break;
			case 5: r = fread(o_ptr_v4, sizeof(object_type_v4), 1, fp);
				o_ptr->owner = o_ptr_v4->owner;
				o_ptr->killer = o_ptr_v4->killer;
				o_ptr->level = o_ptr_v4->level;
				o_ptr->k_idx = o_ptr_v4->k_idx;
				o_ptr->h_idx = o_ptr_v4->h_idx;
				o_ptr->wpos = o_ptr_v4->wpos;
				o_ptr->iy = o_ptr_v4->iy;
				o_ptr->ix = o_ptr_v4->ix;
				o_ptr->tval = o_ptr_v4->tval;
				o_ptr->sval = o_ptr_v4->sval;
				o_ptr->tval2 = o_ptr_v4->tval2;
				o_ptr->sval2 = o_ptr_v4->sval2;
				o_ptr->bpval = o_ptr_v4->bpval;
				o_ptr->pval = o_ptr_v4->pval;
				o_ptr->pval2 = o_ptr_v4->pval2;
				o_ptr->pval3 = o_ptr_v4->pval3;
				o_ptr->pval_org = o_ptr_v4->pval_org;
				o_ptr->bpval_org = o_ptr_v4->bpval_org;
				o_ptr->to_h_org = o_ptr_v4->to_h_org;
				o_ptr->to_d_org = o_ptr_v4->to_d_org;
				o_ptr->to_a_org = o_ptr_v4->to_a_org;
				o_ptr->sigil = o_ptr_v4->sigil;
				o_ptr->sseed = o_ptr_v4->sseed;
				o_ptr->discount = o_ptr_v4->discount;
				o_ptr->number = o_ptr_v4->number;
				o_ptr->weight = o_ptr_v4->weight;
				o_ptr->name1 = o_ptr_v4->name1;
				o_ptr->name2 = o_ptr_v4->name2;
				o_ptr->name2b = o_ptr_v4->name2b;
				o_ptr->name3 = o_ptr_v4->name3;
				o_ptr->name4 = o_ptr_v4->name4;
				o_ptr->attr = o_ptr_v4->attr;
				o_ptr->mode = o_ptr_v4->mode;
				o_ptr->xtra1 = o_ptr_v4->xtra1;
				o_ptr->xtra2 = o_ptr_v4->xtra2;
				o_ptr->xtra3 = o_ptr_v4->xtra3;
				o_ptr->xtra4 = o_ptr_v4->xtra4;
				o_ptr->xtra5 = o_ptr_v4->xtra5;
				o_ptr->xtra6 = o_ptr_v4->xtra6;
				o_ptr->xtra7 = o_ptr_v4->xtra7;
				o_ptr->xtra8 = o_ptr_v4->xtra8;
				o_ptr->xtra9 = o_ptr_v4->xtra9;
				o_ptr->uses_dir = o_ptr_v4->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v4->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v4->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v4->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v4->to_h;
				o_ptr->to_d = o_ptr_v4->to_d;
				o_ptr->to_a = o_ptr_v4->to_a;
				o_ptr->ac = o_ptr_v4->ac;
				o_ptr->dd = o_ptr_v4->dd;
				o_ptr->ds = o_ptr_v4->ds;
				o_ptr->ident = o_ptr_v4->ident;
				o_ptr->timeout = o_ptr_v4->timeout;
				o_ptr->timeout_magic = o_ptr_v4->timeout_magic;
				o_ptr->recharging = o_ptr_v4->recharging;
				o_ptr->marked = o_ptr_v4->marked;
				o_ptr->marked2 = o_ptr_v4->marked2;
				o_ptr->questor = o_ptr_v4->questor;
				o_ptr->quest = o_ptr_v4->quest;
				o_ptr->quest_stage = o_ptr_v4->quest_stage;
				o_ptr->questor_idx = o_ptr_v4->questor_idx;
				o_ptr->questor_invincible = o_ptr_v4->questor_invincible;
				o_ptr->quest_credited = o_ptr_v4->quest_credited;
				o_ptr->note = o_ptr_v4->note;
				o_ptr->note_utag = o_ptr_v4->note_utag;
				o_ptr->inven_order = o_ptr_v4->inven_order;
				o_ptr->next_o_idx = o_ptr_v4->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v4->held_m_idx;
				o_ptr->auto_insc = o_ptr_v4->auto_insc;
				o_ptr->stack_pos = o_ptr_v4->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v4->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v4->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v4->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v4->housed;
				o_ptr->changed = o_ptr_v4->changed;
				o_ptr->NR_tradable = o_ptr_v4->NR_tradable;
				o_ptr->no_soloist = o_ptr_v4->no_soloist;
				o_ptr->temp = o_ptr_v4->temp;
				o_ptr->iron_trade = o_ptr_v4->iron_trade;
				o_ptr->iron_turn = o_ptr_v4->iron_turn;
				o_ptr->embed = o_ptr_v4->embed;
				//convert:
				o_ptr->id = o_ptr->f_id = o_ptr->f_name[0] = 0; //don't generate an id here, whatever
				o_ptr->f_turn = o_ptr->f_time = 0;
				o_ptr->f_wpos = (struct worldpos){ 0, 0, 0 };
				o_ptr->f_dun = 0;
				o_ptr->f_player = o_ptr->f_player_turn = 0;
				o_ptr->f_ridx = o_ptr->f_reidx = 0;
				o_ptr->f_special = o_ptr->f_reward = 0;
				//convert: (6)
				o_ptr->number2 = 0;
				o_ptr->note2 = o_ptr->note2_utag = 0;
				//convert: (7)
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v4->mode; /* u16b = byte */
				break;
			case 6: r = fread(o_ptr_v5, sizeof(object_type_v5), 1, fp);
				o_ptr->owner = o_ptr_v5->owner;
				o_ptr->killer = o_ptr_v5->killer;
				o_ptr->level = o_ptr_v5->level;
				o_ptr->k_idx = o_ptr_v5->k_idx;
				o_ptr->h_idx = o_ptr_v5->h_idx;
				o_ptr->wpos = o_ptr_v5->wpos;
				o_ptr->iy = o_ptr_v5->iy;
				o_ptr->ix = o_ptr_v5->ix;
				o_ptr->tval = o_ptr_v5->tval;
				o_ptr->sval = o_ptr_v5->sval;
				o_ptr->tval2 = o_ptr_v5->tval2;
				o_ptr->sval2 = o_ptr_v5->sval2;
				o_ptr->bpval = o_ptr_v5->bpval;
				o_ptr->pval = o_ptr_v5->pval;
				o_ptr->pval2 = o_ptr_v5->pval2;
				o_ptr->pval3 = o_ptr_v5->pval3;
				o_ptr->pval_org = o_ptr_v5->pval_org;
				o_ptr->bpval_org = o_ptr_v5->bpval_org;
				o_ptr->to_h_org = o_ptr_v5->to_h_org;
				o_ptr->to_d_org = o_ptr_v5->to_d_org;
				o_ptr->to_a_org = o_ptr_v5->to_a_org;
				o_ptr->sigil = o_ptr_v5->sigil;
				o_ptr->sseed = o_ptr_v5->sseed;
				o_ptr->discount = o_ptr_v5->discount;
				o_ptr->number = o_ptr_v5->number;
				o_ptr->weight = o_ptr_v5->weight;
				o_ptr->name1 = o_ptr_v5->name1;
				o_ptr->name2 = o_ptr_v5->name2;
				o_ptr->name2b = o_ptr_v5->name2b;
				o_ptr->name3 = o_ptr_v5->name3;
				o_ptr->name4 = o_ptr_v5->name4;
				o_ptr->attr = o_ptr_v5->attr;
				o_ptr->mode = o_ptr_v5->mode;
				o_ptr->xtra1 = o_ptr_v5->xtra1;
				o_ptr->xtra2 = o_ptr_v5->xtra2;
				o_ptr->xtra3 = o_ptr_v5->xtra3;
				o_ptr->xtra4 = o_ptr_v5->xtra4;
				o_ptr->xtra5 = o_ptr_v5->xtra5;
				o_ptr->xtra6 = o_ptr_v5->xtra6;
				o_ptr->xtra7 = o_ptr_v5->xtra7;
				o_ptr->xtra8 = o_ptr_v5->xtra8;
				o_ptr->xtra9 = o_ptr_v5->xtra9;
				o_ptr->uses_dir = o_ptr_v5->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v5->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v5->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v5->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v5->to_h;
				o_ptr->to_d = o_ptr_v5->to_d;
				o_ptr->to_a = o_ptr_v5->to_a;
				o_ptr->ac = o_ptr_v5->ac;
				o_ptr->dd = o_ptr_v5->dd;
				o_ptr->ds = o_ptr_v5->ds;
				o_ptr->ident = o_ptr_v5->ident;
				o_ptr->timeout = o_ptr_v5->timeout;
				o_ptr->timeout_magic = o_ptr_v5->timeout_magic;
				o_ptr->recharging = o_ptr_v5->recharging;
				o_ptr->marked = o_ptr_v5->marked;
				o_ptr->marked2 = o_ptr_v5->marked2;
				o_ptr->questor = o_ptr_v5->questor;
				o_ptr->quest = o_ptr_v5->quest;
				o_ptr->quest_stage = o_ptr_v5->quest_stage;
				o_ptr->questor_idx = o_ptr_v5->questor_idx;
				o_ptr->questor_invincible = o_ptr_v5->questor_invincible;
				o_ptr->quest_credited = o_ptr_v5->quest_credited;
				o_ptr->note = o_ptr_v5->note;
				o_ptr->note_utag = o_ptr_v5->note_utag;
				o_ptr->inven_order = o_ptr_v5->inven_order;
				o_ptr->next_o_idx = o_ptr_v5->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v5->held_m_idx;
				o_ptr->auto_insc = o_ptr_v5->auto_insc;
				o_ptr->stack_pos = o_ptr_v5->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v5->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v5->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v5->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v5->housed;
				o_ptr->changed = o_ptr_v5->changed;
				o_ptr->NR_tradable = o_ptr_v5->NR_tradable;
				o_ptr->no_soloist = o_ptr_v5->no_soloist;
				o_ptr->temp = o_ptr_v5->temp;
				o_ptr->iron_trade = o_ptr_v5->iron_trade;
				o_ptr->iron_turn = o_ptr_v5->iron_turn;
				o_ptr->embed = o_ptr_v5->embed;
				o_ptr->id = o_ptr_v5->id;
				o_ptr->f_id = o_ptr_v5->f_id;
				strcpy(o_ptr->f_name, o_ptr_v5->f_name);
				o_ptr->f_turn = o_ptr_v5->f_turn;
				o_ptr->f_time = o_ptr_v5->f_time;
				o_ptr->f_wpos = o_ptr_v5->f_wpos;
				o_ptr->f_dun = o_ptr_v5->f_dun;
				o_ptr->f_player = o_ptr_v5->f_player;
				o_ptr->f_player_turn = o_ptr_v5->f_player_turn;
				o_ptr->f_ridx = o_ptr_v5->f_ridx;
				o_ptr->f_reidx = o_ptr_v5->f_reidx;
				o_ptr->f_special = o_ptr_v5->f_special;
				o_ptr->f_reward = o_ptr_v5->f_reward;
				//convert:
				o_ptr->number2 = 0;
				o_ptr->note2 = o_ptr->note2_utag = 0;
				//convert: (7)
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v5->mode; /* u16b = byte */
				break;
			case 7: r = fread(o_ptr_v6, sizeof(object_type_v6), 1, fp);
				o_ptr->owner = o_ptr_v6->owner;
				o_ptr->killer = o_ptr_v6->killer;
				o_ptr->level = o_ptr_v6->level;
				o_ptr->k_idx = o_ptr_v6->k_idx;
				o_ptr->h_idx = o_ptr_v6->h_idx;
				o_ptr->wpos = o_ptr_v6->wpos;
				o_ptr->iy = o_ptr_v6->iy;
				o_ptr->ix = o_ptr_v6->ix;
				o_ptr->tval = o_ptr_v6->tval;
				o_ptr->sval = o_ptr_v6->sval;
				o_ptr->tval2 = o_ptr_v6->tval2;
				o_ptr->sval2 = o_ptr_v6->sval2;
				o_ptr->bpval = o_ptr_v6->bpval;
				o_ptr->pval = o_ptr_v6->pval;
				o_ptr->pval2 = o_ptr_v6->pval2;
				o_ptr->pval3 = o_ptr_v6->pval3;
				o_ptr->pval_org = o_ptr_v6->pval_org;
				o_ptr->bpval_org = o_ptr_v6->bpval_org;
				o_ptr->to_h_org = o_ptr_v6->to_h_org;
				o_ptr->to_d_org = o_ptr_v6->to_d_org;
				o_ptr->to_a_org = o_ptr_v6->to_a_org;
				o_ptr->sigil = o_ptr_v6->sigil;
				o_ptr->sseed = o_ptr_v6->sseed;
				o_ptr->discount = o_ptr_v6->discount;
				o_ptr->number = o_ptr_v6->number;
				o_ptr->weight = o_ptr_v6->weight;
				o_ptr->name1 = o_ptr_v6->name1;
				o_ptr->name2 = o_ptr_v6->name2;
				o_ptr->name2b = o_ptr_v6->name2b;
				o_ptr->name3 = o_ptr_v6->name3;
				o_ptr->name4 = o_ptr_v6->name4;
				o_ptr->attr = o_ptr_v6->attr;
				o_ptr->mode = o_ptr_v6->mode;
				o_ptr->xtra1 = o_ptr_v6->xtra1;
				o_ptr->xtra2 = o_ptr_v6->xtra2;
				o_ptr->xtra3 = o_ptr_v6->xtra3;
				o_ptr->xtra4 = o_ptr_v6->xtra4;
				o_ptr->xtra5 = o_ptr_v6->xtra5;
				o_ptr->xtra6 = o_ptr_v6->xtra6;
				o_ptr->xtra7 = o_ptr_v6->xtra7;
				o_ptr->xtra8 = o_ptr_v6->xtra8;
				o_ptr->xtra9 = o_ptr_v6->xtra9;
				o_ptr->uses_dir = o_ptr_v6->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v6->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v6->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v6->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v6->to_h;
				o_ptr->to_d = o_ptr_v6->to_d;
				o_ptr->to_a = o_ptr_v6->to_a;
				o_ptr->ac = o_ptr_v6->ac;
				o_ptr->dd = o_ptr_v6->dd;
				o_ptr->ds = o_ptr_v6->ds;
				o_ptr->ident = o_ptr_v6->ident;
				o_ptr->timeout = o_ptr_v6->timeout;
				o_ptr->timeout_magic = o_ptr_v6->timeout_magic;
				o_ptr->recharging = o_ptr_v6->recharging;
				o_ptr->marked = o_ptr_v6->marked;
				o_ptr->marked2 = o_ptr_v6->marked2;
				o_ptr->questor = o_ptr_v6->questor;
				o_ptr->quest = o_ptr_v6->quest;
				o_ptr->quest_stage = o_ptr_v6->quest_stage;
				o_ptr->questor_idx = o_ptr_v6->questor_idx;
				o_ptr->questor_invincible = o_ptr_v6->questor_invincible;
				o_ptr->quest_credited = o_ptr_v6->quest_credited;
				o_ptr->note = o_ptr_v6->note;
				o_ptr->note_utag = o_ptr_v6->note_utag;
				o_ptr->inven_order = o_ptr_v6->inven_order;
				o_ptr->next_o_idx = o_ptr_v6->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v6->held_m_idx;
				o_ptr->auto_insc = o_ptr_v6->auto_insc;
				o_ptr->stack_pos = o_ptr_v6->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v6->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v6->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v6->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v6->housed;
				o_ptr->changed = o_ptr_v6->changed;
				o_ptr->NR_tradable = o_ptr_v6->NR_tradable;
				o_ptr->no_soloist = o_ptr_v6->no_soloist;
				o_ptr->temp = o_ptr_v6->temp;
				o_ptr->iron_trade = o_ptr_v6->iron_trade;
				o_ptr->iron_turn = o_ptr_v6->iron_turn;
				o_ptr->embed = o_ptr_v6->embed;
				o_ptr->id = o_ptr_v6->id;
				o_ptr->f_id = o_ptr_v6->f_id;
				strcpy(o_ptr->f_name, o_ptr_v6->f_name);
				o_ptr->f_turn = o_ptr_v6->f_turn;
				o_ptr->f_time = o_ptr_v6->f_time;
				o_ptr->f_wpos = o_ptr_v6->f_wpos;
				o_ptr->f_dun = o_ptr_v6->f_dun;
				o_ptr->f_player = o_ptr_v6->f_player;
				o_ptr->f_player_turn = o_ptr_v6->f_player_turn;
				o_ptr->f_ridx = o_ptr_v6->f_ridx;
				o_ptr->f_reidx = o_ptr_v6->f_reidx;
				o_ptr->f_special = o_ptr_v6->f_special;
				o_ptr->f_reward = o_ptr_v6->f_reward;
				o_ptr->number2 = o_ptr_v6->number2;
				o_ptr->note2 = o_ptr_v6->note2;
				o_ptr->note2_utag = o_ptr_v6->note2_utag;
				//convert:
				o_ptr->slain_monsters = 0;
				o_ptr->slain_uniques = 0;
				o_ptr->slain_players = 0;
				o_ptr->times_activated = 0;
				o_ptr->time_equipped = 0;
				o_ptr->time_carried = 0;
				o_ptr->slain_orcs = 0;
				o_ptr->slain_trolls = 0;
				o_ptr->slain_giants = 0;
				o_ptr->slain_animals = 0;
				o_ptr->slain_dragons = 0;
				o_ptr->slain_demons = 0;
				o_ptr->slain_undead = 0;
				o_ptr->slain_evil = 0;
				o_ptr->slain_bosses = 0;
				o_ptr->slain_nazgul = 0;
				o_ptr->slain_superuniques = 0;
				o_ptr->slain_sauron = 0;
				o_ptr->slain_morgoth = 0;
				o_ptr->slain_zuaon = 0;
				o_ptr->done_damage = 0;
				o_ptr->done_healing = 0;
				o_ptr->got_damaged = 0;
				o_ptr->got_repaired = 0;
				o_ptr->got_enchanted = 0;
				o_ptr->custom_lua_carrystate = 0;
				o_ptr->custom_lua_equipstate = 0;
				o_ptr->custom_lua_destruction = 0;
				o_ptr->custom_lua_usage = 0;
				//convert (8):
				o_ptr->mode = o_ptr_v6->mode; /* u16b = byte */
				break;
			case 8: r = fread(o_ptr_v7, sizeof(object_type_v7), 1, fp);
				o_ptr->owner = o_ptr_v7->owner;
				o_ptr->killer = o_ptr_v7->killer;
				o_ptr->level = o_ptr_v7->level;
				o_ptr->k_idx = o_ptr_v7->k_idx;
				o_ptr->h_idx = o_ptr_v7->h_idx;
				o_ptr->wpos = o_ptr_v7->wpos;
				o_ptr->iy = o_ptr_v7->iy;
				o_ptr->ix = o_ptr_v7->ix;
				o_ptr->tval = o_ptr_v7->tval;
				o_ptr->sval = o_ptr_v7->sval;
				o_ptr->tval2 = o_ptr_v7->tval2;
				o_ptr->sval2 = o_ptr_v7->sval2;
				o_ptr->bpval = o_ptr_v7->bpval;
				o_ptr->pval = o_ptr_v7->pval;
				o_ptr->pval2 = o_ptr_v7->pval2;
				o_ptr->pval3 = o_ptr_v7->pval3;
				o_ptr->pval_org = o_ptr_v7->pval_org;
				o_ptr->bpval_org = o_ptr_v7->bpval_org;
				o_ptr->to_h_org = o_ptr_v7->to_h_org;
				o_ptr->to_d_org = o_ptr_v7->to_d_org;
				o_ptr->to_a_org = o_ptr_v7->to_a_org;
				o_ptr->sigil = o_ptr_v7->sigil;
				o_ptr->sseed = o_ptr_v7->sseed;
				o_ptr->discount = o_ptr_v7->discount;
				o_ptr->number = o_ptr_v7->number;
				o_ptr->weight = o_ptr_v7->weight;
				o_ptr->name1 = o_ptr_v7->name1;
				o_ptr->name2 = o_ptr_v7->name2;
				o_ptr->name2b = o_ptr_v7->name2b;
				o_ptr->name3 = o_ptr_v7->name3;
				o_ptr->name4 = o_ptr_v7->name4;
				o_ptr->attr = o_ptr_v7->attr;
				o_ptr->xtra1 = o_ptr_v7->xtra1;
				o_ptr->xtra2 = o_ptr_v7->xtra2;
				o_ptr->xtra3 = o_ptr_v7->xtra3;
				o_ptr->xtra4 = o_ptr_v7->xtra4;
				o_ptr->xtra5 = o_ptr_v7->xtra5;
				o_ptr->xtra6 = o_ptr_v7->xtra6;
				o_ptr->xtra7 = o_ptr_v7->xtra7;
				o_ptr->xtra8 = o_ptr_v7->xtra8;
				o_ptr->xtra9 = o_ptr_v7->xtra9;
				o_ptr->uses_dir = o_ptr_v7->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v7->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v7->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v7->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v7->to_h;
				o_ptr->to_d = o_ptr_v7->to_d;
				o_ptr->to_a = o_ptr_v7->to_a;
				o_ptr->ac = o_ptr_v7->ac;
				o_ptr->dd = o_ptr_v7->dd;
				o_ptr->ds = o_ptr_v7->ds;
				o_ptr->ident = o_ptr_v7->ident;
				o_ptr->timeout = o_ptr_v7->timeout;
				o_ptr->timeout_magic = o_ptr_v7->timeout_magic;
				o_ptr->recharging = o_ptr_v7->recharging;
				o_ptr->marked = o_ptr_v7->marked;
				o_ptr->marked2 = o_ptr_v7->marked2;
				o_ptr->questor = o_ptr_v7->questor;
				o_ptr->quest = o_ptr_v7->quest;
				o_ptr->quest_stage = o_ptr_v7->quest_stage;
				o_ptr->questor_idx = o_ptr_v7->questor_idx;
				o_ptr->questor_invincible = o_ptr_v7->questor_invincible;
				o_ptr->quest_credited = o_ptr_v7->quest_credited;
				o_ptr->note = o_ptr_v7->note;
				o_ptr->note_utag = o_ptr_v7->note_utag;
				o_ptr->inven_order = o_ptr_v7->inven_order;
				o_ptr->next_o_idx = o_ptr_v7->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v7->held_m_idx;
				o_ptr->auto_insc = o_ptr_v7->auto_insc;
				o_ptr->stack_pos = o_ptr_v7->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v7->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v7->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v7->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v7->housed;
				o_ptr->changed = o_ptr_v7->changed;
				o_ptr->NR_tradable = o_ptr_v7->NR_tradable;
				o_ptr->no_soloist = o_ptr_v7->no_soloist;
				o_ptr->temp = o_ptr_v7->temp;
				o_ptr->iron_trade = o_ptr_v7->iron_trade;
				o_ptr->iron_turn = o_ptr_v7->iron_turn;
				o_ptr->embed = o_ptr_v7->embed;
				o_ptr->id = o_ptr_v7->id;
				o_ptr->f_id = o_ptr_v7->f_id;
				strcpy(o_ptr->f_name, o_ptr_v7->f_name);
				o_ptr->f_turn = o_ptr_v7->f_turn;
				o_ptr->f_time = o_ptr_v7->f_time;
				o_ptr->f_wpos = o_ptr_v7->f_wpos;
				o_ptr->f_dun = o_ptr_v7->f_dun;
				o_ptr->f_player = o_ptr_v7->f_player;
				o_ptr->f_player_turn = o_ptr_v7->f_player_turn;
				o_ptr->f_ridx = o_ptr_v7->f_ridx;
				o_ptr->f_reidx = o_ptr_v7->f_reidx;
				o_ptr->f_special = o_ptr_v7->f_special;
				o_ptr->f_reward = o_ptr_v7->f_reward;
				o_ptr->number2 = o_ptr_v7->number2;
				o_ptr->note2 = o_ptr_v7->note2;
				o_ptr->note2_utag = o_ptr_v7->note2_utag;
				o_ptr->slain_monsters = o_ptr_v7->slain_monsters;
				o_ptr->slain_uniques = o_ptr_v7->slain_uniques;
				o_ptr->slain_players = o_ptr_v7->slain_players;
				o_ptr->times_activated = o_ptr_v7->times_activated;
				o_ptr->time_equipped = o_ptr_v7->time_equipped;
				o_ptr->time_carried = o_ptr_v7->time_carried;
				o_ptr->slain_orcs = o_ptr_v7->slain_orcs;
				o_ptr->slain_trolls = o_ptr_v7->slain_trolls;
				o_ptr->slain_giants = o_ptr_v7->slain_giants;
				o_ptr->slain_animals = o_ptr_v7->slain_animals;
				o_ptr->slain_dragons = o_ptr_v7->slain_dragons;
				o_ptr->slain_demons = o_ptr_v7->slain_demons;
				o_ptr->slain_undead = o_ptr_v7->slain_undead;
				o_ptr->slain_evil = o_ptr_v7->slain_evil;
				o_ptr->slain_bosses = o_ptr_v7->slain_bosses;
				o_ptr->slain_nazgul = o_ptr_v7->slain_nazgul;
				o_ptr->slain_superuniques = o_ptr_v7->slain_superuniques;
				o_ptr->slain_sauron = o_ptr_v7->slain_sauron;
				o_ptr->slain_morgoth = o_ptr_v7->slain_morgoth;
				o_ptr->slain_zuaon = o_ptr_v7->slain_zuaon;
				o_ptr->done_damage = o_ptr_v7->done_damage;
				o_ptr->done_healing = o_ptr_v7->done_healing;
				o_ptr->got_damaged = o_ptr_v7->got_damaged;
				o_ptr->got_repaired = o_ptr_v7->got_repaired;
				o_ptr->got_enchanted = o_ptr_v7->got_enchanted;
				o_ptr->custom_lua_carrystate = o_ptr_v7->custom_lua_carrystate;
				o_ptr->custom_lua_equipstate = o_ptr_v7->custom_lua_equipstate;
				o_ptr->custom_lua_destruction = o_ptr_v7->custom_lua_destruction;
				o_ptr->custom_lua_usage = o_ptr_v7->custom_lua_usage;
				//convert:
				o_ptr->mode = o_ptr_v7->mode; /* u16b = byte */
				break;
			case 9: r = fread(o_ptr_v8, sizeof(object_type_v8), 1, fp);
				o_ptr->owner = o_ptr_v8->owner;
				o_ptr->killer = o_ptr_v8->killer;
				o_ptr->level = o_ptr_v8->level;
				o_ptr->k_idx = o_ptr_v8->k_idx;
				o_ptr->h_idx = o_ptr_v8->h_idx;
				o_ptr->wpos = o_ptr_v8->wpos;
				o_ptr->iy = o_ptr_v8->iy;
				o_ptr->ix = o_ptr_v8->ix;
				o_ptr->tval = o_ptr_v8->tval;
				o_ptr->sval = o_ptr_v8->sval;
				o_ptr->tval2 = o_ptr_v8->tval2;
				o_ptr->sval2 = o_ptr_v8->sval2;
				o_ptr->bpval = o_ptr_v8->bpval;
				o_ptr->pval = o_ptr_v8->pval;
				o_ptr->pval2 = o_ptr_v8->pval2;
				o_ptr->pval3 = o_ptr_v8->pval3;
				o_ptr->pval_org = o_ptr_v8->pval_org;
				o_ptr->bpval_org = o_ptr_v8->bpval_org;
				o_ptr->to_h_org = o_ptr_v8->to_h_org;
				o_ptr->to_d_org = o_ptr_v8->to_d_org;
				o_ptr->to_a_org = o_ptr_v8->to_a_org;
				o_ptr->sigil = o_ptr_v8->sigil;
				o_ptr->sseed = o_ptr_v8->sseed;
				o_ptr->discount = o_ptr_v8->discount;
				o_ptr->number = o_ptr_v8->number;
				o_ptr->weight = o_ptr_v8->weight;
				o_ptr->name1 = o_ptr_v8->name1;
				o_ptr->name2 = o_ptr_v8->name2;
				o_ptr->name2b = o_ptr_v8->name2b;
				o_ptr->name3 = o_ptr_v8->name3;
				o_ptr->name4 = o_ptr_v8->name4;
				o_ptr->attr = o_ptr_v8->attr;
				o_ptr->xtra1 = o_ptr_v8->xtra1;
				o_ptr->xtra2 = o_ptr_v8->xtra2;
				o_ptr->xtra3 = o_ptr_v8->xtra3;
				o_ptr->xtra4 = o_ptr_v8->xtra4;
				o_ptr->xtra5 = o_ptr_v8->xtra5;
				o_ptr->xtra6 = o_ptr_v8->xtra6;
				o_ptr->xtra7 = o_ptr_v8->xtra7;
				o_ptr->xtra8 = o_ptr_v8->xtra8;
				o_ptr->xtra9 = o_ptr_v8->xtra9;
				o_ptr->uses_dir = o_ptr_v8->uses_dir;
#ifdef PLAYER_STORES
				o_ptr->ps_idx_x = o_ptr_v8->ps_idx_x;
				o_ptr->ps_idx_y = o_ptr_v8->ps_idx_y;
				o_ptr->appraised_value = o_ptr_v8->appraised_value;
#endif
				o_ptr->to_h = o_ptr_v8->to_h;
				o_ptr->to_d = o_ptr_v8->to_d;
				o_ptr->to_a = o_ptr_v8->to_a;
				o_ptr->ac = o_ptr_v8->ac;
				o_ptr->dd = o_ptr_v8->dd;
				o_ptr->ds = o_ptr_v8->ds;
				o_ptr->ident = o_ptr_v8->ident;
				o_ptr->timeout = o_ptr_v8->timeout;
				o_ptr->timeout_magic = o_ptr_v8->timeout_magic;
				o_ptr->recharging = o_ptr_v8->recharging;
				o_ptr->marked = o_ptr_v8->marked;
				o_ptr->marked2 = o_ptr_v8->marked2;
				o_ptr->questor = o_ptr_v8->questor;
				o_ptr->quest = o_ptr_v8->quest;
				o_ptr->quest_stage = o_ptr_v8->quest_stage;
				o_ptr->questor_idx = o_ptr_v8->questor_idx;
				o_ptr->questor_invincible = o_ptr_v8->questor_invincible;
				o_ptr->quest_credited = o_ptr_v8->quest_credited;
				o_ptr->note = o_ptr_v8->note;
				o_ptr->note_utag = o_ptr_v8->note_utag;
				o_ptr->inven_order = o_ptr_v8->inven_order;
				o_ptr->next_o_idx = o_ptr_v8->next_o_idx;
				o_ptr->held_m_idx = o_ptr_v8->held_m_idx;
				o_ptr->auto_insc = o_ptr_v8->auto_insc;
				o_ptr->stack_pos = o_ptr_v8->stack_pos;
				o_ptr->cheeze_dlv = o_ptr_v8->cheeze_dlv;
				o_ptr->cheeze_plv = o_ptr_v8->cheeze_plv;
				o_ptr->cheeze_plv_carry = o_ptr_v8->cheeze_plv_carry;
				o_ptr->housed = o_ptr_v8->housed;
				o_ptr->changed = o_ptr_v8->changed;
				o_ptr->NR_tradable = o_ptr_v8->NR_tradable;
				o_ptr->no_soloist = o_ptr_v8->no_soloist;
				o_ptr->temp = o_ptr_v8->temp;
				o_ptr->iron_trade = o_ptr_v8->iron_trade;
				o_ptr->iron_turn = o_ptr_v8->iron_turn;
				o_ptr->embed = o_ptr_v8->embed;
				o_ptr->id = o_ptr_v8->id;
				o_ptr->f_id = o_ptr_v8->f_id;
				strcpy(o_ptr->f_name, o_ptr_v8->f_name);
				o_ptr->f_turn = o_ptr_v8->f_turn;
				o_ptr->f_time = o_ptr_v8->f_time;
				o_ptr->f_wpos = o_ptr_v8->f_wpos;
				o_ptr->f_dun = o_ptr_v8->f_dun;
				o_ptr->f_player = o_ptr_v8->f_player;
				o_ptr->f_player_turn = o_ptr_v8->f_player_turn;
				o_ptr->f_ridx = o_ptr_v8->f_ridx;
				o_ptr->f_reidx = o_ptr_v8->f_reidx;
				o_ptr->f_special = o_ptr_v8->f_special;
				o_ptr->f_reward = o_ptr_v8->f_reward;
				o_ptr->number2 = o_ptr_v8->number2;
				o_ptr->note2 = o_ptr_v8->note2;
				o_ptr->note2_utag = o_ptr_v8->note2_utag;
				o_ptr->slain_monsters = o_ptr_v8->slain_monsters;
				o_ptr->slain_uniques = o_ptr_v8->slain_uniques;
				o_ptr->slain_players = o_ptr_v8->slain_players;
				o_ptr->times_activated = o_ptr_v8->times_activated;
				o_ptr->time_equipped = o_ptr_v8->time_equipped;
				o_ptr->time_carried = o_ptr_v8->time_carried;
				o_ptr->slain_orcs = o_ptr_v8->slain_orcs;
				o_ptr->slain_trolls = o_ptr_v8->slain_trolls;
				o_ptr->slain_giants = o_ptr_v8->slain_giants;
				o_ptr->slain_animals = o_ptr_v8->slain_animals;
				o_ptr->slain_dragons = o_ptr_v8->slain_dragons;
				o_ptr->slain_demons = o_ptr_v8->slain_demons;
				o_ptr->slain_undead = o_ptr_v8->slain_undead;
				o_ptr->slain_evil = o_ptr_v8->slain_evil;
				o_ptr->slain_bosses = o_ptr_v8->slain_bosses;
				o_ptr->slain_nazgul = o_ptr_v8->slain_nazgul;
				o_ptr->slain_superuniques = o_ptr_v8->slain_superuniques;
				o_ptr->slain_sauron = o_ptr_v8->slain_sauron;
				o_ptr->slain_morgoth = o_ptr_v8->slain_morgoth;
				o_ptr->slain_zuaon = o_ptr_v8->slain_zuaon;
				o_ptr->done_damage = o_ptr_v8->done_damage;
				o_ptr->done_healing = o_ptr_v8->done_healing;
				o_ptr->got_damaged = o_ptr_v8->got_damaged;
				o_ptr->got_repaired = o_ptr_v8->got_repaired;
				o_ptr->got_enchanted = o_ptr_v8->got_enchanted;
				o_ptr->custom_lua_carrystate = o_ptr_v8->custom_lua_carrystate;
				o_ptr->custom_lua_equipstate = o_ptr_v8->custom_lua_equipstate;
				o_ptr->custom_lua_destruction = o_ptr_v8->custom_lua_destruction;
				o_ptr->custom_lua_usage = o_ptr_v8->custom_lua_usage;
				//convert:
				o_ptr->mode = o_ptr_v8->mode; /* u32b = u16b */
				o_ptr->wId = 0;
				o_ptr->dummy1 = o_ptr->dummy2 = 0; /* (future use) */
			}
			if (r == 0) {
				s_printf("  error: Failed to read object.\n");
				msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
				fprintf(fp_tmp, "OB:");
				relay_estate(buf, buf2, fp, fp_tmp);
				return;
			}
			/* Update item's kind-index in case k_info.txt has been modified */
			o_ptr->k_idx = lookup_kind(o_ptr->tval, o_ptr->sval);
#ifdef SEAL_INVALID_OBJECTS
			if (!seal_or_unseal_object(o_ptr)) continue;
#endif

			/* also read inscription */
			o_ptr->note = 0;
			data_len = -2;
#if 0 /* scanf() sucks a bit */
			data_note[0] = '\0';
			r = fscanf(fp, "%d[^\n]", &data_len);
			r = fread(data_note, 1, 1, fp); //strip the \n that fscanf had to miss
#else
			rc = fgets(data_note, 4, fp);
			data_len = atoi(data_note);
			data_note[0] = '\0';
#endif
			if (rc == NULL || data_len == -2) {
				object_desc(Ind, o_name, o_ptr, TRUE, 3);
				s_printf("  error: Corrupted note line (item '%s').\n", o_name);
				msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
				relay_estate(buf, buf2, fp, fp_tmp);
				return;
			}
			if (data_len != -1) {
				r = fread(data_note, sizeof(char), data_len, fp);
				if (r == data_len) {
					data_note[data_len] = '\0';
					o_ptr->note = quark_add(data_note);
				} else {
					s_printf("  error: Failed to read note line (item '%s').\n", o_name);
					msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
					relay_estate(buf, buf2, fp, fp_tmp);
					return;
				}
			}

			/* is it a pile of gold? */
			if (o_ptr->tval == TV_GOLD) {
				/* give gold to player if it doesn't overflow,
				   otherwise relay rest to temporary file, swap and exit */
				au = o_ptr->pval;
				if (!gain_au(Ind, au, FALSE, FALSE)) {
					msg_print(Ind, "\377yDrop/deposite some gold to be able to receive more gold.");

					/* write failed gold gain back into new buffer file */
					fprintf(fp_tmp, "OB:");
#if 0
					(void)fwrite(o_ptr, sizeof(object_type), 1, fp_tmp);
#else
					switch (conversion) {
					case 0:
						(void)fwrite(o_ptr, sizeof(object_type), 1, fp_tmp);
						break;
					case 1:
						(void)fwrite(o_ptr_v2, sizeof(object_type_v2), 1, fp_tmp);
						break;
					case 2:
						(void)fwrite(o_ptr_v2a, sizeof(object_type_v2a), 1, fp_tmp);
						break;
					case 3:
						(void)fwrite(o_ptr_v2b, sizeof(object_type_v2b), 1, fp_tmp);
						break;
					case 4:
						(void)fwrite(o_ptr_v3, sizeof(object_type_v3), 1, fp_tmp);
						break;
					case 5:
						(void)fwrite(o_ptr_v4, sizeof(object_type_v4), 1, fp_tmp);
						break;
					case 6:
						(void)fwrite(o_ptr_v5, sizeof(object_type_v5), 1, fp_tmp);
						break;
					case 7:
						(void)fwrite(o_ptr_v6, sizeof(object_type_v6), 1, fp_tmp);
						break;
					case 8:
						(void)fwrite(o_ptr_v7, sizeof(object_type_v7), 1, fp_tmp);
						break;
					case 9:
						(void)fwrite(o_ptr_v8, sizeof(object_type_v8), 1, fp_tmp);
						break;
					}
#endif

					/* paranoia: should always be inscriptionless of course */
					/* ..and its inscription */
					if (o_ptr->note) {
						fprintf(fp_tmp, "%d\n", (int)strlen(quark_str(o_ptr->note)));
						(void)fwrite(quark_str(o_ptr->note), sizeof(char), strlen(quark_str(o_ptr->note)), fp_tmp);
					} else
						fprintf(fp_tmp, "%d\n", -1);

					relay_estate(buf, buf2, fp, fp_tmp);
					return;
				}
				gained_anything = TRUE;
				s_printf("  gained %ld Au.\n", au);
				msg_format(Ind, "You receive %ld gold pieces.", au);
				continue;
			}

			/* give item to player if he has space left,
			   otherwise relay rest to temporary file, swap and exit */
			if (!inven_carry_okay(Ind, o_ptr, 0x0)) {
				msg_print(Ind, "\377yYour inventory is full, make some space to receive more items.");

				/* write failed item back into new buffer file */
				fprintf(fp_tmp, "OB:");
				switch (conversion) {
				case 0:
					(void)fwrite(o_ptr, sizeof(object_type), 1, fp_tmp);
					break;
				case 1:
					(void)fwrite(o_ptr_v2, sizeof(object_type_v2), 1, fp_tmp);
					break;
				case 2:
					(void)fwrite(o_ptr_v2a, sizeof(object_type_v2a), 1, fp_tmp);
					break;
				case 3:
					(void)fwrite(o_ptr_v2b, sizeof(object_type_v2b), 1, fp_tmp);
					break;
				case 4:
					(void)fwrite(o_ptr_v3, sizeof(object_type_v3), 1, fp_tmp);
					break;
				case 5:
					(void)fwrite(o_ptr_v4, sizeof(object_type_v4), 1, fp_tmp);
					break;
				case 6:
					(void)fwrite(o_ptr_v5, sizeof(object_type_v5), 1, fp_tmp);
					break;
				case 7:
					(void)fwrite(o_ptr_v6, sizeof(object_type_v6), 1, fp_tmp);
					break;
				case 8:
					(void)fwrite(o_ptr_v7, sizeof(object_type_v7), 1, fp_tmp);
					break;
				case 9:
					(void)fwrite(o_ptr_v8, sizeof(object_type_v8), 1, fp_tmp);
					break;
				}

				/* ..and its inscription */
				if (o_ptr->note) {
					fprintf(fp_tmp, "%d\n", (int)strlen(quark_str(o_ptr->note)));
					(void)fwrite(quark_str(o_ptr->note), sizeof(char), strlen(quark_str(o_ptr->note)), fp_tmp);
				} else
					fprintf(fp_tmp, "%d\n", -1);

				relay_estate(buf, buf2, fp, fp_tmp);
				return;
			}

			gained_anything = TRUE;
			inven_carry(Ind, o_ptr);
			object_desc(Ind, o_name, o_ptr, TRUE, 3);
			msg_format(Ind, "You receive %s.", o_name);
			s_printf("  gained %s.\n", o_name);
			continue;
		} else {
			s_printf("  invalid data '%s'.\n", data);
			msg_print(Ind, "\377oAn error occurred, please contact an administrator.");
			relay_estate(buf, buf2, fp, fp_tmp);
			return;
		}
	}
}

/* For gathering statistical Ironman Deep Dive Challenge data on players clearing out monsters */
void log_floor_coverage(dun_level *l_ptr, struct worldpos *wpos) {
	cptr feel;

	if (!l_ptr) return;

	if (l_ptr->flags2 & LF2_OOD_HI) feel = "terrifying";
	else if ((l_ptr->flags2 & LF2_VAULT_HI) &&
	    (l_ptr->flags2 & LF2_OOD)) feel = "terrifying";
	else if ((l_ptr->flags2 & LF2_VAULT_OPEN) || // <- TODO: implement :/
	     ((l_ptr->flags2 & LF2_VAULT) && (l_ptr->flags2 & LF2_OOD_FREE))) feel = "danger";
	else if (l_ptr->flags2 & LF2_VAULT) feel = "dangerous";
	else if (l_ptr->flags2 & LF2_PITNEST_HI) feel = "dangerous";
	else if (l_ptr->flags2 & LF2_OOD_FREE) feel = "challenge";
	else if (l_ptr->flags2 & LF2_UNIQUE) feel = "special";
	else feel = "boring";

	if (l_ptr->monsters_generated + l_ptr->monsters_spawned == 0)
		s_printf("CVRG-IDDC: %3d, g:--- s:--- k:---, --%% (%s)\n", wpos->wz, feel);
	else
		s_printf("CVRG-IDDC: %3d, g:%3d s:%3d k:%3d, %2d%% (%s)\n", wpos->wz,
		    l_ptr->monsters_generated, l_ptr->monsters_spawned, l_ptr->monsters_killed,
		    (l_ptr->monsters_killed * 100) / (l_ptr->monsters_generated + l_ptr->monsters_spawned),
		    feel);
}

/* whenever the player enters a new grid (walk, teleport..)
   check whether he is affected in certain ways.. */
void grid_affects_player(int Ind, int ox, int oy) {
	player_type *p_ptr = Players[Ind];
	int x = p_ptr->px, y = p_ptr->py;
	cave_type **zcave;
	cave_type *c_ptr;
#ifdef USE_SOUND_2010
	bool inn = FALSE, music = FALSE;
#endif

	if (!(zcave = getcave(&p_ptr->wpos))) return;
	c_ptr = &zcave[p_ptr->py][p_ptr->px];

	if (c_ptr->feat == FEAT_FAKE_WALL) {
		msg_print(Ind, "\377sYou don't feel resistance as you cross the wall, just a little static charge in the air as you find yourself in another strange location..");
		p_ptr->auto_transport = AT_PARTY;
		return;
	}

#ifdef USE_SOUND_2010
	inn = inside_inn(p_ptr, c_ptr);
#endif

	if (outdoor_affects(&p_ptr->wpos) && !night_surface &&
	    !(p_ptr->global_event_temp & PEVF_INDOORS_00) && /* 'A "personal" outdoor_affects()' ie player-specific, not wpos-specific. Maybe improve this somehow. */
	    !inside_house(&p_ptr->wpos, x, y) && // || p_ptr->store_num != -1) &&  --not safe "inside" pstores, as we're still standing outside...
	    !(f_info[c_ptr->feat].flags2 & FF2_COVER) && //special: feat-protected, trees, walls, npc shops, doors offer cover
	    !(c_ptr->info & CAVE_PROT)) {
		if (!p_ptr->grid_sunlit) {
			p_ptr->grid_sunlit = TRUE;
			calc_boni(Ind);
		}
	} else if (p_ptr->grid_sunlit) {
		p_ptr->grid_sunlit = FALSE;
		calc_boni(Ind);
	}

	if (!p_ptr->warning_secret_area && ((zcave[y][x].info2 & CAVE2_SCRT) && (ox == -1 || !(zcave[oy][ox].info2 & CAVE2_SCRT)))) {
		p_ptr->warning_secret_area = TRUE;
		msg_print(Ind, "\377yYou have discovered a secret area!");
		s_printf("warning_secret_area: %s\n", p_ptr->name);
	}

	/* Handle entering/leaving no-teleport area */
	if ((zcave[y][x].info & CAVE_STCK) && (ox == -1 || !(zcave[oy][ox].info & CAVE_STCK))) {
		msg_print(Ind, "\377DThe air in here feels very still.");
		p_ptr->redraw |= PR_DEPTH; /* hack: depth colour indicates no-tele */
#ifdef USE_SOUND_2010
		/* New: Have bgm indicate no-tele too! */
		handle_music(Ind);
		music = TRUE;
#endif

		if (p_ptr->tim_wraith) {
			msg_format_near(Ind, "%s loses %s wraith powers.", p_ptr->name, p_ptr->male ? "his" : "her");
			msg_print(Ind, "You lose your wraith powers.");

			/* Automatically disable permanent wraith form (set_tim_wraith) */
			p_ptr->tim_wraith = 0; //avoid duplicate message
			p_ptr->tim_wraithstep &= ~0x1; //hack: mark as normal wraithform, to distinguish from wraithstep
			p_ptr->update |= PU_BONUS;
			p_ptr->redraw |= PR_BPR_WRAITH;
		}
	}
	if (ox != -1 && (zcave[oy][ox].info & CAVE_STCK) && !(zcave[y][x].info & CAVE_STCK)) {
		msg_print(Ind, "\377sFresh air greets you as you leave the vault.");
		p_ptr->redraw |= PR_DEPTH; /* hack: depth colour indicates no-tele */
		p_ptr->redraw |= PR_BPR_WRAITH;
#ifdef USE_SOUND_2010
		/* New: Have bgm indicate no-tele too! */
		handle_music(Ind);
		music = TRUE;
#endif

		/* Automatically re-enable permanent wraith form (set_tim_wraith) */
		p_ptr->update |= PU_BONUS;
	}

#ifdef ENABLE_OUNLIFE
	/* Leaving a wall with Wraithstep */
	if (p_ptr->tim_wraith && (p_ptr->tim_wraithstep & 0x1)
	    && cave_floor_grid(&zcave[y][x]) && (ox == -1 || !cave_floor_grid(&zcave[oy][ox]))) {
		set_tim_wraith(Ind, 0);
	}
#endif

#ifdef USE_SOUND_2010
	/* Handle entering a sickbay area (only possible via logging on into one) */
	if (p_ptr->music_monster != -3 && (c_ptr->feat == FEAT_SICKBAY_AREA || c_ptr->feat == FEAT_SICKBAY_DOOR)) {
		p_ptr->music_monster = -3; //hack sickbay music
		handle_music(Ind);
		music = TRUE;
	}

	/* Handle leaving a sickbay area */
	if (p_ptr->music_monster == -3 && c_ptr->feat != FEAT_SICKBAY_AREA && c_ptr->feat != FEAT_SICKBAY_DOOR) {
		p_ptr->music_monster = -1; //unhack sickbay music
		handle_music(Ind);
		music = TRUE;
	}

	/* Handle entering/leaving taverns. (-3 check is for distinguishing inn from sickbay) */
	if (p_ptr->music_monster != -3 && p_ptr->music_monster != -4 && inn) {
		p_ptr->music_monster = -4; //hack inn music
		handle_music(Ind);
		music = TRUE;

		/* Also take care of inn ambient sfx (fireplace) */
		handle_ambient_sfx(Ind, c_ptr, &p_ptr->wpos, TRUE);
	}
	if (p_ptr->music_monster == -4 && !inn) {
		p_ptr->music_monster = -1; //unhack inn music
		handle_music(Ind);
		music = TRUE;

		/* Also take care of inn ambient sfx (fireplace) */
		handle_ambient_sfx(Ind, c_ptr, &p_ptr->wpos, TRUE);
	}

	/* Handle entering a jail */
	if (p_ptr->music_monster != -5 && (c_ptr->info & CAVE_JAIL)) {
		p_ptr->music_monster = -5; //hack jail music
		handle_music(Ind);
		music = TRUE;
	}

	/* Handle leaving a jail */
	if (p_ptr->music_monster == -5 && !(c_ptr->info & CAVE_JAIL)) {
		p_ptr->music_monster = -1; //unhack jail music
		handle_music(Ind);
		music = TRUE;
	}
#endif

#if 0 /* moved directly to Send_sfx_ambient(), or there'll be a glitch in call order (cyclic), resulting in volume 'spike'. */
	/* Hack: Inns count as houses too */
	if (inside_house(&p_ptr->wpos, p_ptr->px, p_ptr->py) || inn || p_ptr->store_num != -1) {
		if (!p_ptr->grid_house) {
			p_ptr->grid_house = TRUE;
			if (!p_ptr->sfx_house) Send_sfx_volume(Ind, 0, 0);
			else if (p_ptr->sfx_house_quiet) Send_sfx_volume(Ind, p_ptr->sound_ambient == SFX_AMBIENT_FIREPLACE ? 100 : GRID_SFX_REDUCTION, GRID_SFX_REDUCTION);
		}
	} else if (p_ptr->grid_house) {
		p_ptr->grid_house = FALSE;
		if (p_ptr->sfx_house_quiet || !p_ptr->sfx_house) Send_sfx_volume(Ind, 100, 100);
	}
#endif

#ifdef USE_SOUND_2010
	/* Renew music too? */
	if (!music && ox == -1) handle_music(Ind);
#endif

	/* quests - check if he has arrived at a designated exact x,y target location */
	if (p_ptr->quest_any_deliver_xy_within_target) quest_check_goal_deliver(Ind);

	/* When the player enters a grid with items, he can destroy anything on it for 1x energy cost, until he enters another grid */
	p_ptr->destroyed_floor_item = FALSE;
}

/* Items that can be shared even between incompatible character modes or if level 0! */
bool exceptionally_shareable_item(object_type *o_ptr) {
	if (o_ptr->name1 || ((o_ptr->name2 || o_ptr->name2b) && o_ptr->tval != TV_FOOD)) return(FALSE);

	if ((o_ptr->tval == TV_SCROLL && (
	    o_ptr->sval == SV_SCROLL_WORD_OF_RECALL ||
	    o_ptr->sval == SV_SCROLL_SATISFY_HUNGER ||
	    o_ptr->sval == SV_SCROLL_FIREWORK)) ||
	    (o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_TORCH) ||
	    (o_ptr->tval == TV_FLASK && o_ptr->sval == SV_FLASK_OIL) ||
	    // "Why not share ale? -Molt" <- good idea, here too!
	    (o_ptr->tval == TV_FOOD && o_ptr->sval > SV_FOOD_MUSHROOMS_MAX && !o_ptr->name1
	    && o_ptr->sval != SV_FOOD_WAYBREAD //hrrm @ Lembas? but an infinite Lembas starter stack could be cheezed easily this way +_+
	    && o_ptr->sval != SV_FOOD_ATHELAS))
		return(TRUE);
	return(FALSE);
}
/* Starter items that can be shared despite being level 0! */
bool shareable_starter_item(object_type *o_ptr) {
	if (o_ptr->name1 || ((o_ptr->name2 || o_ptr->name2b) && o_ptr->tval != TV_FOOD)) return(FALSE);

	if ((o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_TORCH) ||
	    (o_ptr->tval == TV_SCROLL && o_ptr->sval == SV_SCROLL_SATISFY_HUNGER) ||
	    // "Why not share ale? -Molt" <- good idea, here too!
	    (o_ptr->tval == TV_FOOD && o_ptr->sval > SV_FOOD_MUSHROOMS_MAX && !o_ptr->name1
	    && o_ptr->sval != SV_FOOD_WAYBREAD //hrrm @ Lembas? but an infinite Lembas starter stack could be cheezed easily this way +_+
	    && o_ptr->sval != SV_FOOD_ATHELAS))
		return(TRUE);
	return(FALSE);
}

void kick_char(int Ind_kicker, int Ind_kickee, char *reason) {
	char kickmsg[MSG_LEN];

	if (reason) {
		msg_format(Ind_kicker, "Kicking %s out (%s).", Players[Ind_kickee]->name, reason);
		snprintf(kickmsg, MSG_LEN, "Kicked out (%s)", reason);
		Destroy_connection(Players[Ind_kickee]->conn, kickmsg);
	} else {
		msg_format(Ind_kicker, "Kicking %s out.", Players[Ind_kickee]->name);
		Destroy_connection(Players[Ind_kickee]->conn, "Kicked out");
	}
}

void kick_ip(int Ind_kicker, char *ip_kickee, char *reason, bool msg) {
	int i;
	char kickmsg[MSG_LEN];
	bool found = FALSE;

	if (reason) {
		if (msg) msg_format(Ind_kicker, "Kicking out connections from %s (%s).", ip_kickee, reason);
		snprintf(kickmsg, MSG_LEN, "Kicked out - %s", reason);
	} else {
		if (msg) msg_format(Ind_kicker, "Kicking out connections from %s.", ip_kickee);
		snprintf(kickmsg, MSG_LEN, "Kicked out");
	}

	/* Kick him out (note, this could affect multiple people at once if sharing an IP) */
	for (i = 1; i <= NumPlayers; i++) {
		if (!Players[i]) continue;
		if (!strcmp(get_player_ip(i), ip_kickee)) {
			found = TRUE;
			if (reason) s_printf("IP-kicked '%s' (%s).\n", Players[i]->name, reason);
			else s_printf("IP-kicked '%s'.\n", Players[i]->name);
			Destroy_connection(Players[i]->conn, kickmsg);
			i--;
		}
	}
	if (msg && !found) msg_print(Ind_kicker, "No matching player online to kick.");
}

/* Calculate basic success chance for magic devices for a player,
   before any "minimum granted chance" is applied. */
static int magic_device_base_chance(int Ind, object_type *o_ptr) {
	u32b dummy, f4;
	player_type *p_ptr = Players[Ind];
	/* Extract the item level */
	int lev = k_info[o_ptr->k_idx].level;
	/* Base chance of success */
	int chance = p_ptr->skill_dev;

#ifdef MSTAFF_MDEV_COMBO
	/* Don't use the mage staff's level, use the absorbed device's */
	if (o_ptr->tval == TV_MSTAFF) {
		if (o_ptr->xtra1) lev = k_info[lookup_kind(TV_STAFF, o_ptr->xtra1 - 1)].level;
		else if (o_ptr->xtra2) lev = k_info[lookup_kind(TV_WAND, o_ptr->xtra2 - 1)].level;
		else if (o_ptr->xtra3) lev = k_info[lookup_kind(TV_ROD, o_ptr->xtra3 - 1)].level;
		//else fall through, as it could be a legit artifact mage staff with activation
	}
#endif

	if (o_ptr->tval == TV_RUNE) {
		chance = exec_lua(0, format("return rcraft_rune(%d,%d)", Ind, o_ptr->sval));

		/* Confusion makes it much harder (maybe TODO: blind/stun?) */
		if (p_ptr->confused) chance = chance / 2;

		return(chance);
	}

#if 0 /* not needed anymore since x_dev and skill-ratios have been adjusted in tables.c */
	/* Reduce very high levels */
	lev = (400 - ((200 - lev) * (200 - lev)) / 100) / 4;//1..75
#endif

	/* Hack -- use artifact level instead */
	if (true_artifact_p(o_ptr)) lev = a_info[o_ptr->name1].level;

	/* Extract object flags */
	object_flags(o_ptr, &dummy, &dummy, &dummy, &f4, &dummy, &dummy, &dummy);
#ifdef MSTAFF_MDEV_COMBO
	if (o_ptr->tval == TV_MSTAFF && o_ptr->xtra5) f4 |= e_info[o_ptr->xtra5].flags4[0];
#endif

	/* Is it simple to use ? */
	if (f4 & TR4_EASY_USE) {
		chance += USE_DEVICE;

		/* High level objects are harder */
		chance = chance - (lev * 4) / 5;
	} else {
		/* High level objects are harder */
		//chance = chance - ((lev > 50) ? 50 : lev) - (p_ptr->antimagic * 2);
		chance = chance - lev;
	}

	/* Fix underflow 1/2 */
	if (chance < 0) chance = 0;

	/* Hacks: Certain items are easier/harder to use in general: */
#if 1
	/* equippable magic devices are especially easy to use? (ie no wands/staves/rods)
	   eg tele rings, serpent amulets, true artifacts */
	if (!is_magic_device(o_ptr->tval)
 #ifdef MSTAFF_MDEV_COMBO
	    && !(o_ptr->tval == TV_MSTAFF && (o_ptr->xtra1 || o_ptr->xtra2 || o_ptr->xtra3))
 #endif
	    ) {
		chance += 30;
		chance = chance - lev / 10;
	}
#else
	/* Rings of polymorphing and the Ring of Phasing shouldn't require magic device skill really */
	else if ((o_ptr->tval == TV_RING && o_ptr->sval == SV_RING_POLYMORPH) ||
	    (o_ptr->tval == TV_RING && o_ptr->sval == SV_RING_WRAITH)) {
		if (chance < USE_DEVICE * 2) chance = USE_DEVICE * 2;
	}
#endif

	/* Fix underflow 2/2 */
	if (chance < 0) chance = 0;

	/* Confusion makes it much harder (maybe TODO: blind/stun?) */
	if (p_ptr->confused) chance = chance / 2;

	/* prevent div0 (0) and overflow (1) */
	if (chance < 2) chance = 2;

	return(chance);
}

/* just for display purpose, return an actual average percentage value. ('bonus' is for WIELD_DEVICES.) */
int activate_magic_device_chance(int Ind, object_type *o_ptr, byte *permille, bool bonus) {
	int chance = magic_device_base_chance(Ind, o_ptr);

	if (o_ptr->tval == TV_RUNE) return(chance); // Hack: Rune Boni - Kurzel

#ifdef MSTAFF_MDEV_COMBO
	if (o_ptr->tval == TV_MSTAFF && (o_ptr->xtra1 || o_ptr->xtra2 || o_ptr->xtra3)) bonus = TRUE; /* We wield the 'magic device' -> bonus as usual */
#endif

	/* 100% not possible to reach:
	   For chance >= 201 this produces a rounding error that would result in displayed 100%,
	   when in reality activate_magic_device() can never reach 100% (even though it can go over 99%!).
	   So we should not return 100 here, really, as it gives a false sense of perfect security. */
	//*permille = (1000 - ((USE_DEVICE - 1) * 100) / chance) % 10; /* <- for calling function, to disregard '100%' fake result and display permille instead */
	//chance = 100 - ((USE_DEVICE - 1) * 100) / chance;

	/* 100% possible to reach: */
	*permille = (1000 - ((USE_DEVICE - 1) * 1000) / chance + (chance * 10) / 11) % 10;
	chance = 100 - ((USE_DEVICE - 1) * 100) / chance + chance / 11;
	if (bonus) chance += 20; //WIELD_DEVICES flat bonus for wands/staves/rods
	if (chance >= 100) {
		chance = 100;
		*permille = 0;
	}

	/* Give everyone a (slight) chance */
	if (!chance) return(1);

	return(chance);
}

bool activate_magic_device(int Ind, object_type *o_ptr, bool bonus) {
	player_type *p_ptr = Players[Ind];
	byte permille;
	int chance;

	if (o_ptr->tval == TV_RUNE) return(magik(activate_magic_device_chance(Ind, o_ptr, &permille, FALSE)));

#ifdef MSTAFF_MDEV_COMBO
	if (o_ptr->tval == TV_MSTAFF && (o_ptr->xtra1 || o_ptr->xtra2 || o_ptr->xtra3)) bonus = TRUE; /* We wield the 'magic device' -> bonus as usual */
#endif
	chance  = activate_magic_device_chance(Ind, o_ptr, &permille, bonus);

	/* Certain items are heavily restricted (todo: use WINNERS_ONLY flag instead for cleanliness) */
	if (o_ptr->name1 == ART_PHASING && !p_ptr->total_winner) {
		msg_print(Ind, "Only royalties may activate this Ring!");
		if (!is_admin(p_ptr)) return(FALSE);
	}

	/* Roll for usage */
	if (rand_int(1000) < chance * 10 + permille) return(TRUE);
	return(FALSE);
}

/* Condense an (account) name into a 'normalised' version, used to prevent
   new players from creating account names too similar to existing ones. - C. Blue */
#ifndef SIMILAR_ROMAN
void condense_name(char *condensed, const char *name) {
#else
void condense_name(char *condensed, const char *name_raw) {
#endif
	char *bufptr = condensed, current, multiple = 0, *ptr;
	bool space = TRUE;

#ifdef SIMILAR_ROMAN
	char name[NAME_LEN];

	strcpy(name, name_raw);
	if ((ptr = roman_suffix(name))) *(ptr - 1) = 0;
	//s_printf("condense: n='%s'", name); // debug (spammy)
#endif

	for (ptr = (char*)name; *ptr; ptr++) {
		/* skip spaces in the beginning */
		if (space && *ptr == ' ') continue;
		space = FALSE;

		/* ignore lower/upper case */
		current = tolower(*ptr);

		//discard non-alphanumeric characters
		if (!isalphanum(current)) continue;

		//condense multiples of the same character
		if (multiple == current) continue;
		multiple = current;

		//finally add the character
		*bufptr++ = current;
	}
	*bufptr = 0; //terminate

	//discard spaces at the end of the name
	for (ptr = bufptr - 1; ptr >= condensed; ptr--)
		if (*ptr == ' ') {
			*ptr = 0;
			bufptr--;
		}
		else break;

	//extra strict: discard digits at the end of the name
	for (ptr = bufptr - 1; ptr >= condensed; ptr--)
		if (isdigit(*ptr))
			*ptr = 0;
		else break;
}

/* Helper function to check account/character/guild.. names for too great similarity:
   Super-strict mode: Disallow (non-trivial) names that only have 1+ letter inserted somewhere. */
#ifndef SIMILAR_ROMAN
int similar_names(const char *name1, const char *name2) {
#else
int similar_names(const char *name1_raw, const char *name2_raw) {
#endif
	char tmpname[MAX_CHARS];
	const char *ptr, *ptr2;
	char *ptr3;
	int diff, min, diff_bonus;
	int diff_loc, diff2;
	bool words = FALSE;

#ifdef SIMILAR_ROMAN
	char name1[NAME_LEN], name2[NAME_LEN];

	strcpy(name1, name1_raw);
	strcpy(name2, name2_raw);
	if ((ptr3 = roman_suffix(name1))) *(ptr3 - 1) = 0;
	if ((ptr3 = roman_suffix(name2))) *(ptr3 - 1) = 0;
	//s_printf("similar: n1='%s',n2='%s'", name1, name2); // debug (spammy)
#endif

	if (strlen(name1) < 5 || strlen(name2) < 5) return(0); //trivial length, it's ok to be similar

	/* don't exaggerate */
	if (strlen(name1) > strlen(name2)) min = strlen(name2);
	else min = strlen(name1);

	/* '->' */
	diff = 0;
	ptr2 = name1;
	diff_loc = -1;
	for (ptr = name2; *ptr && *ptr2; ) {
		if (tolower(*ptr) != tolower(*ptr2)) {
			diff++;
			if (diff_loc == -1) diff_loc = ptr - name2;//remember where they started to be different
		}
		else ptr++;
		ptr2++;
	}
	//all remaining characters that couldn't be matched are also "different"
	if (diff_loc == -1 && (*ptr || *ptr2)) diff_loc = ptr - name2;//remember where they started to be different
	while (*ptr++) diff++;
	while (*ptr2++) diff++;
	//too little difference between names? forbidden!
	if (diff <= (min - 5) / 2 + 1) {
		//special check: if they differ early on, it weighs slightly more :)
		if (diff_loc < min / 2 - 1) {
			//loosened up slightly
			if (diff <= (min - 6) / 2 + 1) {
				s_printf("similar_names (1a): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
				return(1);
			}
		} else { //normal case
			s_printf("similar_names (1): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
			return(1);
		}
	}

	/* '<-' */
	diff = 0;
	ptr2 = name2;
	diff_loc = -1;
	for (ptr = name1; *ptr && *ptr2; ) {
		if (tolower(*ptr) != tolower(*ptr2)) {
			diff++;
			if (diff_loc == -1) diff_loc = ptr - name1;//remember where they started to be different
		}
		else ptr++;
		ptr2++;
	}
	//all remaining characters that couldn't be matched are also "different"
	if (diff_loc == -1 && (*ptr || *ptr2)) diff_loc = ptr - name1;//remember where they started to be different
	while (*ptr++) diff++;
	while (*ptr2++) diff++;
	//too little difference between names? forbidden!
	if (diff <= (min - 5) / 2 + 1) {
		//special check: if they differ early on, it weighs slightly more :)
		if (diff_loc < min / 2 - 1) {
			//loosened up slightly
			if (diff <= (min - 6) / 2 + 1) {
				s_printf("similar_names (2a): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
				return(2);
			}
		} else { //normal case
			s_printf("similar_names (2): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
			return(2);
		}
	}

	/* '='  -- note: weakness here is, the first 2 methods don't combine with this one ;).
	   So the checks could be tricked by combining one 'replaced char' with one 'too many char'
	   to circumvent the limitations for longer names that usually would require a 3+ difference.. =P */
	diff = 0;
	ptr2 = name2;
	diff_loc = -1;
	diff_bonus = 0;
	for (ptr = name1; *ptr && *ptr2; ) {
		if (tolower(*ptr) != tolower(*ptr2)) {
			diff++;
			if (diff_loc == -1) diff_loc = ptr - name1;//remember where they started to be different
			//consonant vs vowel = big difference
#if 0 /* [***] see below */
			if (diff_loc < min / 2 - 1) /* see below (*) */
#else
			if (diff_loc <= (min + 1) / 2) /* see below (*) */
#endif
				if (is_a_vowel(tolower(*ptr)) != is_a_vowel(tolower(*ptr2))) diff_bonus++;
		}
		ptr++;
		ptr2++;
	}
	//all remaining characters that couldn't be matched are also "different"
	if (diff_loc == -1 && (*ptr || *ptr2)) diff_loc = ptr - name1;//remember where they started to be different
	while (*ptr++) diff++;
	while (*ptr2++) diff++;
#if 0 /* [***] a bit too strict maybe: Arwen and Arjen accounts cannot coexist */
	//too little difference between names? forbidden!
	if (diff <= (min - 5) / 2 + 1) {
		//special check: if they differ early on, it weighs slightly more :)
		if (diff_loc < min / 2 - 1) { /* see above (*) */
			//loosened up slightly
			if (diff <= (min - 6 - (diff_bonus ? 1 : 0)) / 2 + 1) {
				s_printf("similar_names (3a): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
				return(3);
			}
		} else { //normal case
			s_printf("similar_names (3): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
			return(3);
		}
	}
#else
	//too little difference between names? forbidden!
	if (diff <= (min - 5) / 2 + 1) {
		//special check: if they differ early on, it weighs slightly more :)
		if (diff_loc <= (min + 1) / 2) { /* see above (*) */
			//loosened up slightly
			if (diff <= (min - 7 - (diff_bonus ? 1 : 0)) / 2 + 1) {
				s_printf("similar_names (3a): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
				return(3);
			}
		} else { //normal case
			s_printf("similar_names (3): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
			return(3);
		}
	}
#endif

#if 0	/* Check for anagrams */
	diff = 0;
	strcpy(tmpname, name2);
	for (ptr = name1; *ptr; ptr++) {
		for (ptr3 = tmpname; *ptr3; ptr3++) {
			if (tolower(*ptr) == tolower(*ptr3)) {
				*ptr3 = '*'; //'disable' this character
				break;
			}
		}
		if (!(*ptr3)) diff++;
	}
	diff2 = 0;
	strcpy(tmpname, name1);
	for (ptr = name2; *ptr; ptr++) {
		for (ptr3 = tmpname; *ptr3; ptr3++) {
			if (tolower(*ptr) == tolower(*ptr3)) {
				*ptr3 = '*'; //'disable' this character
				break;
			}
		}
		if (!(*ptr3)) diff2++;
	}
	if (diff2 > diff) diff = diff2; //max()
	//too little difference between names? forbidden!
	if (diff <= (min - 6) / 2 + 1) { //must use the 'loosened up' version used further above, since it'd override otherwise
		s_printf("similar_names (4): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
		return(4);
	}
#endif
#if 1	/* Check for exact anagrams */
	diff = 0;
	strcpy(tmpname, name2);
	for (ptr = name1; *ptr; ptr++) {
		for (ptr3 = tmpname; *ptr3; ptr3++) {
			if (tolower(*ptr) == tolower(*ptr3)) {
				*ptr3 = '*'; //'disable' this character
				break;
			}
		}
		if (!(*ptr3)) {
			diff = 1;
			break;
		}
	}
	strcpy(tmpname, name1);
	for (ptr = name2; *ptr; ptr++) {
		for (ptr3 = tmpname; *ptr3; ptr3++) {
			if (tolower(*ptr) == tolower(*ptr3)) {
				*ptr3 = '*'; //'disable' this character
				break;
			}
		}
		if (!(*ptr3)) {
			diff = 1;
			break;
		}
	}
	if (!diff) { //perfect anagrams?
		s_printf("similar_names (5): name1 '%s', name2 '%s' (tmp '%s')\n", name1, name2, tmpname);
		return(5);
	}
#endif

	/* Check for prefix */
	diff = 0;
	if (strlen(name1) <= strlen(name2)) {
		ptr = name1;
		ptr2 = name2;
		diff2 = strlen(name2);
	} else {
		ptr = name2;
		ptr2 = name1;
		diff2 = strlen(name1);
	}
	while (*ptr) {
		if (tolower(*ptr) != tolower(*ptr2)) break;
		if (!isalphanum(*ptr)) words = TRUE;
		ptr++;
		ptr2++;
		//REVERSE diff here
		diff++;
	}
	//too little difference between names? forbidden!
#if 0 /* normal */
	if (diff >= 5 && diff > (diff2 * (words ? 6 : 5)) / 10) {
	if (diff < 5 && !words && diff2 < diff + 2) {
#endif
#if 0
	 /* more lenient */
	if (diff >= 5 && diff > (diff2 * (words ? 6 : 6)) / 10) {
		s_printf("similar_names (6a): name1 '%s', name2 '%s'\n", name1, name2);
		return(6);
	}
	if (diff < 5 && !words && diff2 < diff + 2) {
		s_printf("similar_names (6b): name1 '%s', name2 '%s'\n", name1, name2);
		return(6);
	}
#else
	 /* even more lenient, for Darkie */
	if (diff >= 5 && diff > (diff2 * (words ? 8 : 8)) / 10) {
		s_printf("similar_names (6a): name1 '%s', name2 '%s'\n", name1, name2);
		return(6);
	}
	if (diff < 5 && !words && diff2 < diff + 2) {
		s_printf("similar_names (6b): name1 '%s', name2 '%s'\n", name1, name2);
		return(6);
	}
#endif

	return(0); //ok!
}

/* Update a character's expfact in case it has changed. Lua-accessible. - C. Blue */
void verify_expfact(int Ind, int p) {
	player_type *p_ptr;

	/* catch potential user errors if called via manual lua */
	if (Ind < 0 || Ind > NumPlayers) return;
	if (p <= 0 || p > NumPlayers) return;

	p_ptr = Players[p];
	p = p_ptr->expfact; //re-use
	p_ptr->expfact = p_ptr->rp_ptr->r_exp + p_ptr->cp_ptr->c_exp;
	if (Ind) {
		if (p == p_ptr->expfact) msg_format(Ind, "Verified XP%% for %s: Unchanged at %d.", p_ptr->name, p);
		else msg_format(Ind, "Verified XP%% for %s: Updated %d to %d.", p_ptr->name, p, p_ptr->expfact);
	} else {
		if (p != p_ptr->expfact) s_printf("Verified XP%% for %s: Updated %d to %d.\n", p_ptr->name, p, p_ptr->expfact);
	}
	return;
}

/* Based on Mikaelh's sanity check, extended to this helper function for subinventories. */
bool verify_inven_item(int Ind, int item) {
#ifdef ENABLE_SUBINVEN
	if (item >= SUBINVEN_INVEN_MUL) {
		/* Verify container location, must be inside inventory */
		if (item / SUBINVEN_INVEN_MUL - 1 < 0) return(FALSE);
		if (item / SUBINVEN_INVEN_MUL - 1 >= INVEN_PACK) return(FALSE);
		if (Players[Ind]->inventory[item / SUBINVEN_INVEN_MUL - 1].tval != TV_SUBINVEN) return(FALSE);

		/* Verify item location inside container */
		if (item % SUBINVEN_INVEN_MUL < 0) return(FALSE); //is this even... compiler specs please
		if ((item % SUBINVEN_INVEN_MUL) >= Players[Ind]->inventory[item / SUBINVEN_INVEN_MUL - 1].bpval) return(FALSE);

		return(TRUE);
	}
#endif
	/* Allow item on floor? -
	   The player shouldn't be able to specify those by index atm, and since this function is only used to
	   validate player input we can (have to?) skip items on the floor: */
#if 0
	if (-item >= o_max) return(FALSE);
#else
	/* Disallow items on floor. */
	if (item < 0) return(FALSE);
#endif
	/* Item in inventory */
	if (item >= INVEN_TOTAL) return(FALSE);
	return(TRUE);
}

/* Get an item in the player's inventory or (for /dis and /xdis) on the floor.
   NOTE: We assume the item is legal as a previous verify_inven_item() happened in nserver.c!
         So it is not neccesary to check if -item >= o_max.
   Returns TRUE if ok, FALSE if item doesn't exist (floor items only, paranoia?). */
bool get_inven_item(int Ind, int item, object_type **o_ptr) {
#ifdef ENABLE_SUBINVEN
	/* This function can be used for subinventories too, if using get_subinven_item() were overkill. */
	if (item >= SUBINVEN_INVEN_MUL) {
		/* Sanity/Paranoia check that the item is actually inside a subinventory */
		if (Players[Ind]->inventory[item / SUBINVEN_INVEN_MUL - 1].tval != TV_SUBINVEN) {
			msg_print(Ind, "ERROR: Not a subinventory.");
			s_printf("ERROR: Not a subinventory. (%s, %i)\n", Players[Ind]->name, item / SUBINVEN_INVEN_MUL - 1);
			return(FALSE);
		}

		/* Verify the item */
		//if (item / SUBINVEN_INVEN_MUL - 1 > INVEN_PACK || item % SUBINVEN_INVEN_MUL > SUBINVEN_PACK) return(FALSE); -- already done in verify_inven_item()

		/* Get the item */
		*o_ptr = &Players[Ind]->subinventory[item / SUBINVEN_INVEN_MUL - 1][item % SUBINVEN_INVEN_MUL];
		return(TRUE);
	}
#endif

	/* Get the item (in the pack) */
	if (item >= 0) {
		//if (item >= INVEN_TOTAL) return(FALSE); -- already done in verify_inven_item()
		*o_ptr = &Players[Ind]->inventory[item];
	}
	/* Get the item (on the floor) */
	else {
		if (-item > o_max) return(FALSE);// actually already done in verify_inven_item(), which prohibits floor items in general for now, as that feature is not used
		*o_ptr = &o_list[0 - item];
	}
	return(TRUE);
}

#ifdef ENABLE_SUBINVEN
/* Translate encoded item (>=SUBINVEN_INVEN_MUL) to subinventory index,
   or just identity if it's not a subinventory, so it includes get_inven_item() functionality.
   If both Ind and **o_ptr are not 0/NULL, o_ptr will be set to point to the indexed object.
   Ind should be non-zero, to ensure correct check of legal size for the subinventory.
   The pointers *sitem and *iitem are optional and hence can both be NULL.
   NOTE: We assume the item is legal as a previous verify_inven_item() happened in nserver.c! */
void get_subinven_item(int Ind, int item, object_type **o_ptr, int *sitem, int *iitem) {
	int i = item; /* For memory safety, in case item and iitem are the same object */

	/* Not a subinventory but just a 'direct' item? */
	if (i < SUBINVEN_INVEN_MUL) {
		/* Normal inven/equip item */
		if (sitem) *sitem = -1;
		if (iitem) *iitem = i;

		/* Optionally set o_ptr already for convenience */
		if (Ind && o_ptr) *o_ptr = &Players[Ind]->inventory[i];

		return;
	}

	/* Determine container slot in backpack */
	if (sitem) *sitem = i / SUBINVEN_INVEN_MUL - 1;
	/* Determine item slot inside the container */
	if (iitem) *iitem = i % SUBINVEN_INVEN_MUL;

	/* Optionally set o_ptr already for convenience */
	if (Ind && o_ptr && sitem && iitem) *o_ptr = &Players[Ind]->subinventory[*sitem][*iitem];
}
/* Reduce the size of a subinventory if it was from an outdated k_info.txt file version while the newer ones have less capacity.
   If 'check' is TRUE, the subinventory will be scanned for current usage. This must be done whenever it is
   not guaranteed to currently carry at most as many items as the new limit. */
void verify_subinven_size(int Ind, int slot, bool check) {
	player_type *p_ptr = Players[Ind];
	object_type *s_ptr = &p_ptr->inventory[slot];
	int i;

	/* If our subinventory was an older version and its capacity nowadays is smaller,
	   update it with the downgrade (opposite to load.c where it's an upgrade only) */

	/* Size is fine? */
	if (s_ptr->bpval == k_info[s_ptr->k_idx].pval) return;

	for (i = 0; i < s_ptr->bpval; i++) {
		if (p_ptr->subinventory[slot][i].k_idx) continue;

		/* Shrink to correct new size */
		if (i <= k_info[s_ptr->k_idx].pval) s_ptr->bpval = k_info[s_ptr->k_idx].pval;
		/* Shrink to at least somewhat smaller size for now */
		else s_ptr->bpval = i;

		break;
	}
}
/* Empty a subinventory, moving all contents to the player inventory, causing overflow if not enough space.
   drop: Drop items to the floor instead of unstowing them into the backpack inventory (for case of player death). */
void empty_subinven(int Ind, int item, bool drop, bool quiet) {
	player_type *p_ptr = Players[Ind];
	int i, s = p_ptr->inventory[item].bpval, k;
	object_type *o_ptr;
	char o_name[ONAME_LEN];
	bool overflow_msg = FALSE;

	/* Hack: Ensure basic cleanup in case weird container size changes happened in k_info */
	if (!s) {
		/* Log for checking :/ */
		object_desc(0, o_name, &p_ptr->inventory[item], TRUE, 3);
		s_printf("empty_subinven(<%s>,<%s>): Zero-capacity!\n", p_ptr->name, o_name);

		invwipe(&p_ptr->subinventory[item][0]);
		display_subinven(Ind, item);
		verify_subinven_size(Ind, item, FALSE);
		return;
	}

	/* Empty everything first, without live-updating the slots (FALSE) */
	for (i = 0; i < s; i++) {
		o_ptr = &p_ptr->subinventory[item][i];
		if (!o_ptr->tval) break;

		/* Just silently drop items (on death)? */
		if (drop) {
			death_drop_object(p_ptr, i, o_ptr);
			invwipe(&p_ptr->subinventory[item][i]);
			continue;
		}

		/* Place item into player inventory, or drop if full */
		if (inven_carry_okay(Ind, o_ptr, 0x0)) {
			k = inven_carry(Ind, o_ptr);
			if (k != -1) { /* Paranoia, as we just checked for inven_carry_okay, it must be != -1 */
				if (!quiet) {
					object_desc(Ind, o_name, o_ptr, TRUE, 3);
					msg_format(Ind, "You have %s (%c).", o_name, index_to_label(k));
				}
			} else s_printf("empty_subinven(%d) PARANOIA.\n", Ind);
		} else {
			if (!overflow_msg) {
				msg_format(Ind, "\376\377oYour pack overflows!");
				overflow_msg = TRUE;
			}
			object_desc(Ind, o_name, o_ptr, TRUE, 3);
			msg_format(Ind, "\376\377oYou drop %s.", o_name);
#ifdef USE_SOUND_2010
			sound_item(Ind, o_ptr->tval, o_ptr->sval, "drop_");
#endif
			drop_near(TRUE, Ind, o_ptr, 0, &p_ptr->wpos, p_ptr->py, p_ptr->px);
		}

		invwipe(&p_ptr->subinventory[item][i]);
	}

	/* Then, efficiently update it completely at once */
	display_subinven(Ind, item);

	verify_subinven_size(Ind, item, FALSE);
}
/* Erase all contents of a subinventory. Does not delete the subinventory container item iteself. */
void erase_subinven(int Ind, int item) {
	player_type *p_ptr = Players[Ind];
	int i, s = p_ptr->inventory[item].bpval;

	/* Hack: Ensure basic cleanup in case weird container size changes happened in k_info */
	if (!s) {
		invwipe(&p_ptr->subinventory[item][0]);
		display_subinven(Ind, item);
		verify_subinven_size(Ind, item, FALSE);
		return;
	}

	/* Empty everything first, without live-updating the slots (FALSE) */
	for (i = 0; i < s; i++)
		invwipe(&p_ptr->subinventory[item][i]);

	/* Then, efficiently update it completely at once */
	display_subinven(Ind, item);

	verify_subinven_size(Ind, item, FALSE);
}
 #ifdef SUBINVEN_LIMIT_GROUP
int get_subinven_group(int sval) {
	switch (sval) {
	case SV_SI_SATCHEL:
	case SV_SI_TRAPKIT_BAG:
	case SV_SI_MDEVP_WRAPPING:
	case SV_SI_POTION_BELT:
	case SV_SI_FOOD_BAG:
		return(sval); //identity
	default:
		//combine multiple choices to a single group, using the first element as its identifier
		if (sval >= SV_SI_GROUP_CHEST_MIN && sval <= SV_SI_GROUP_CHEST_MAX) return(SV_SI_GROUP_CHEST_MIN);
	}
	return(-1);
}
 #endif
#endif

/* Returns amount of bytes used in a string by colour codes - C. Blue
   Usage: Primarily to format colour-coded top-lines that must not exceed MAX_CHARS (ie 1 line, 80 chars)
   in printed (aka visible) length, but may contain arbitrarily many colours. */
int cclen(cptr str) {
	int l = 0;
	const char *c = str;

	while (*c) {
		if (*c == '\377') {
			l++;
			c++;
			if (*c) l++;
			else break;
		}
		c++;
	}

	return(l);
}
