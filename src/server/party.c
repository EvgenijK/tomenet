/* $Id$ */
/*
 * Support for the "party" system.
 */

/* added this for consistency in some (unrelated) header-inclusion,
   it IS a server file, isn't it? */
#define SERVER

#include <sys/types.h>
#include <sys/stat.h>
#include "angband.h"

#ifdef TOMENET_WORLDS
#include "../world/world.h"
#endif


/*
 * Give some exp-bonus to encourage partying (aka "C.Blue party bonus") [2]
 * formula: (PARTY_XP_BOOST+1)/(PARTY_XP_BOOST + (# of applicable players))
 */
#define PARTY_XP_BOOST	(cfg.party_xp_boost)
#define IDDC_PARTY_XP_BOOST	8
/* [7..10] 10-> 2:92%, 3:85%, 4:79%, 5:73%, 6:69%; 8-> 2:90%, 3:82%, 4:75%, 5:69%, 6:64%
   8 is maybe best, assuming that of a larger group one or two will quickly die anyway xD */

/* Keep these ANTI_MAXPLV_EXPLOIT..  defines consistent with cmd4.c:do_write_others_attributes()! */
/* prevent exploit strategies */
#define ANTI_MAXPLV_EXPLOIT	/* prevent exploiting by having a powerful char losing levels deliberately to get in range with lowbies to boost */
// #define ANTI_MAXPLV_EXPLOIT_SOFTLEV	/* be somewhat less strict (average between max_plv and current max_lev) */
 #define ANTI_MAXPLV_EXPLOIT_SOFTEXP	/* be somewhat less strict (use reduced exp instead of preventing any exp) */

/* Check for illegal party/guild name that could be abused to catch special chat commands */
//!(((N)[0] >= 'A' && (N)[0] <= 'Z') || ((N)[0] >= 'a' && (N)[0] <= 'z') || ((N)[0] >= '0' && (N)[0] <= '9'))
#define ILLEGAL_GROUP_NAME(N) \
    ((N)[0] == 0 || (N)[0] == ' ' || \
    streq(N, "Neutral") || \
    streq(N, "!") || streq(N, "#") || streq(N, "%") || streq(N, "$") || streq(N, "-"))


#ifdef HAVE_CRYPT
#include <unistd.h>
#endif	// HAVE_CRYPT


static char *t_crypt(char *inbuf, cptr salt);
static u32b new_accid(void);

/* The hash table itself */
hash_entry *hash_table[NUM_HASH_ENTRIES];


/* admin only - account edit function */
bool WriteAccount(struct account *r_acc, bool new) {
	FILE *fp;
	short found = 0;
	struct account c_acc;
	long delpos = -1L;
	char buf[1024];
	size_t retval;

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb+");

	/* Attempt to create a new file */
	if (!fp) fp = fopen(buf, "wb+");

	if (!fp) s_printf("Could not open tomenet.acc file! (errno = %d)\n", errno);
	else {
		while (!feof(fp) && !found) {
			retval = fread(&c_acc, sizeof(struct account), 1, fp);
			if (retval == 0) break; /* EOF reached, nothing read into c_acc - mikaelh */
			if (c_acc.flags & ACC_DELD) {
				if (delpos == -1L) delpos = (ftell(fp) - sizeof(struct account));
				if (new) break;
				continue;
			}
			if (!strcmp(c_acc.name, r_acc->name)) found = 1;
		}
		if (found) {
			fseek(fp, -((signed int)sizeof(struct account)), SEEK_CUR);
			if (fwrite(r_acc, sizeof(struct account), 1, fp) < 1)
				s_printf("Writing to account file failed: %s\n", feof(fp) ? "EOF" : strerror(ferror(fp)));
		}
		if (new) {
			if (delpos != -1L) fseek(fp, delpos, SEEK_SET);
			else fseek(fp, 0L, SEEK_END);
			if (fwrite(r_acc, sizeof(struct account), 1, fp) < 1)
				s_printf("Writing to account file failed: %s\n", feof(fp) ? "EOF" : strerror(ferror(fp)));
			found = 1;
		}
	}
	memset(c_acc.pass, 0, sizeof(c_acc.pass));
	fclose(fp);
	return(found);
}

/*
 Get an existing account and set default valid flags on it
 That will be SCORE on only (hack it for MULTI)
 Modified to return(0) if not found, 1 if found but already 100% validated,
 and -1 if found and there was still a character to validate on that account - C. Blue
 */
int validate(char *name) {
	struct account acc;
	int i;
	bool effect = FALSE;

	/* Read from disk */
	if (!GetcaseAccount(&acc, name, name, TRUE)) return(0);

	/* Modify account flags */
	if (acc.flags & ACC_TRIAL) {
		effect = TRUE;
		acc.flags &= ~(ACC_TRIAL | ACC_NOSCORE);
		s_printf("VALIDATE: %s '%s'\n", showtime(), name);

		/* Remove from "new players that need validation" list aka 'list-invalid.txt'. */
		for (i = 0; i < MAX_LIST_INVALID; i++) {
			if (!list_invalid_name[i][0]) break;
			if (strcmp(list_invalid_name[i], name)) continue;

			/* Remove and slide everyone else down by once, to maintain the sorted-by-date state */
			while (i < MAX_LIST_INVALID - 1) {
				if (!list_invalid_name[i][0]) break;
				strcpy(list_invalid_name[i], list_invalid_name[i + 1]);
				strcpy(list_invalid_host[i], list_invalid_host[i + 1]);
				strcpy(list_invalid_addr[i], list_invalid_addr[i + 1]);
				strcpy(list_invalid_date[i], list_invalid_date[i + 1]);
				i++;
			}
			list_invalid_name[i][0] = 0;
			break;
		}
	}

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Validate the player too */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->account != acc.id) continue;
		if (!Players[i]->inval) continue;

		effect = TRUE;
		Players[i]->inval = 0;
		msg_print(i, "\377G -- Welcome to TomeNET, your account has just been validated!");
		msg_print(i, "\377G    You can press \377B?\377G key to see a list of all command keys.");
		msg_print(i, "\377G    Just ask in chat if you have any questions or need help. Enjoy! --");
	}

	/* Success */
	if (effect) return(-1);
	/* Found but already all characters valid */
	return(1);
}

/* invalidate - opposite to validate() */
int invalidate(char *name, bool admin) {
	struct account acc;
	int i;
	bool effect = FALSE;

	/* Read from disk */
	if (!GetcaseAccount(&acc, name, name, TRUE)) return(0);

#if 0 /* actually this is controlled in slash.c and privileged players can also invalidate */
	/* Security check: Only admins can invalidate admin accounts */
	if (!admin && (acc.flags & ACC_ADMIN)) {
		WIPE(&acc, struct account);
		return(2);
	}
#endif

	/* Modify account flags */
	if (!(acc.flags & ACC_TRIAL)) {
		effect = TRUE;
		acc.flags |= (ACC_TRIAL | ACC_NOSCORE);
		s_printf("INVALIDATE: %s '%s'\n", showtime(), name);

		/* Potentially add to "new players that need validation" list aka 'list-invalid.txt'. */
		for (i = 0; i < MAX_LIST_INVALID; i++) {
			if (!list_invalid_name[i][0]) {
				time_t ct = time(NULL);
				struct tm* ctl = localtime(&ct);
				//static char day_names[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
				//, day_names[ctl->tm_wday]

				/* add accountname to the list */
				strcpy(list_invalid_name[i], name);
				strcpy(list_invalid_host[i], acc.hostname);
				strcpy(list_invalid_addr[i], acc.addr);
				strcpy(list_invalid_date[i], format("%04d/%02d/%02d - %02d:%02d:%02dh", 1900 + ctl->tm_year, ctl->tm_mon + 1, ctl->tm_mday, ctl->tm_hour, ctl->tm_min, ctl->tm_sec));
				break;
			}
			if (strcmp(list_invalid_name[i], name)) continue;
			/* accountname is already on the list -- should not happen here as we tested against ACC_TRIAL above. */
			break;
		}
		if (i == MAX_LIST_INVALID) s_printf("Warning: list-invalid is full.\n");
	}

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Invalidate the player too */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->account != acc.id) continue;
		if (Players[i]->inval) continue;

		effect = TRUE;
		Players[i]->inval = 1;
	}

	/* Success */
	if (effect) return(-1);
	/* Found but already all characters invalid */
	return(1);
}

int privilege(char *name, int level) {
	struct account acc;
	int i;
	bool effect = FALSE;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);

	/* Modify account flags */
	switch (level) {
	case 0:
		if (acc.flags & (ACC_VPRIVILEGED | ACC_PRIVILEGED)) {
			effect = TRUE;
			acc.flags &= ~(ACC_VPRIVILEGED | ACC_PRIVILEGED);
		}
		break;
	case 1:
		if (!(acc.flags & ACC_PRIVILEGED)) {
			effect = TRUE;
			acc.flags |= ACC_PRIVILEGED;
		}
		break;
	case 2:
		if (!(acc.flags & ACC_VPRIVILEGED)) {
			effect = TRUE;
			acc.flags |= ACC_VPRIVILEGED;
		}
		break;
	}

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Re-privilege the player too */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->account == acc.id) {
			if (Players[i]->privileged != level) effect = TRUE;
			Players[i]->privileged = level;
		}
	}

	/* Return value of -1 indicates no effect */
	if (effect) return(-1);

	/* Success */
	return(1);
}
int makeadmin(char *name) {
	struct account acc;
	int i;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(FALSE);

	/* Modify account flags */
	acc.flags &= ~(ACC_TRIAL);
	acc.flags |= (ACC_ADMIN | ACC_NOSCORE);

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Make the player an admin too */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->account == acc.id) {
			Players[i]->inval = 0;
			if (!strcmp(name, Players[i]->name))
				Players[i]->admin_dm = 1;
			else
				Players[i]->admin_wiz = 1;
		}
	}

	/* Success */
	return(TRUE);
}

/* set or clear account flags */
int acc_set_flags(char *name, u32b flags, bool set) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);

	/* Modify account flags */
	if (set) acc.flags |= (flags);
	else acc.flags &= ~(flags);

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}

/* get account flags */
u32b acc_get_flags(char *name) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);
	return(acc.flags);
}

/* set or clear account flags */
int acc_set_flags_id(u32b id, u32b flags, bool set) {
	struct account acc;
	char acc_name[MAX_CHARS] = { '\0' };

	strncpy(acc_name, lookup_accountname(id), sizeof(acc_name));
	acc_name[sizeof(acc_name) - 1] = '\0';

	/* Check that the name isn't empty */
	if (acc_name[0] == '\0') return(0);

	/* Read from disk */
	if (!GetAccount(&acc, acc_name, NULL, TRUE, NULL, NULL)) return(0);

	/* Modify account flags */
	if (set) acc.flags |= (flags);
	else acc.flags &= ~(flags);

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}

/* set account guild info */
int acc_set_guild(char *name, s32b id) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);
	acc.guild_id = id;

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}
int acc_set_guild_dna(char *name, u32b dna) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);
	acc.guild_dna = dna;

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}

/* get account guild info */
s32b acc_get_guild(char *name) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);
	return(acc.guild_id);
}
u32b acc_get_guild_dna(char *name) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);
	return(acc.guild_dna);
}

int acc_set_deed_event(char *name, byte deed_sval) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);

	acc.deed_event = deed_sval;

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Preven the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}
char acc_get_deed_event(char *name) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);

	return(acc.deed_event);
}
int acc_set_deed_achievement(char *name, byte deed_sval) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);

	acc.deed_achievement = deed_sval;

	/* Write to disk */
	WriteAccount(&acc, FALSE);

	/* Preven the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}
char acc_get_deed_achievement(char *name) {
	struct account acc;

	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);
	return(acc.deed_achievement);
}
/* get account houses //ACC_HOUSE_LIMIT */
char acc_get_houses(const char *name) {
	struct account acc;

	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);
	return(acc.houses);
}

/* set account houses */
int acc_set_houses(const char *name, char houses) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return(0);
	acc.houses = houses;

	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Success */
	return(1);
}

unsigned char acc_get_runtime(const char *name) {
	struct account acc;

	if (!GetAccount(&acc, name, NULL, FALSE, NULL, NULL)) return(0);
	return(acc.runtime);
}

/* set account houses */
void acc_set_runtime(const char *name, unsigned char runtime) {
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, name, NULL, TRUE, NULL, NULL)) return;

	acc.runtime = runtime;
	/* Write account to disk */
	WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	return;
}

/*
 return player account information (by name)
 */
//void accinfo(char *name) {
//}

/* Most account type stuff was already in here.
   A separate file should probably be made in order to split party/guild from account and database handling. */
/* Note. Accounts will be deleted when empty. They will not be subject to their own 90 days timeout,
         but will be removed upon the removal of the last character. */
/* The caller is responsible for allocating memory for 'c_acc'. */
bool GetAccount(struct account *c_acc, cptr name, char *pass, bool leavepass, char *hostname, char *addr) {
	FILE *fp;
	char buf[1024];
	WIPE(c_acc, struct account);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb+");
	if (!fp) {
		if (errno == ENOENT) {	/* ONLY if non-existent */
			s_printf("WARNING (GetAccount): tomenet.acc file does not exist.\n (path=<%s>)\n", buf);
			fp = fopen(buf, "wb+");
			if (!fp) {
				s_printf("CRITICAL ERROR (GetAccount): Couldn't create a new tomenet.acc file!\n");
				KILL(c_acc, struct account);
				return(FALSE);
			}
			s_printf("Generated new account file!\n");
		} else {
			s_printf("CRITICAL ERROR (GetAccount): tomenet.acc file exists but is inaccessible!\n (path=<%s>)\n", buf);
			KILL(c_acc, struct account);
			return(FALSE);	/* failed */
		}
	}
	while (fread(c_acc, sizeof(struct account), 1, fp)) {
		if (c_acc->flags & ACC_DELD) continue;
		if (!strcmp(c_acc->name, name)) {
			int val;

			if (pass == NULL) {	/* direct name lookup */
				val = 0;
			} else {
				val = strcmp(c_acc->pass, t_crypt(pass, name));

				/* Update the timestamp if the password is successfully verified - mikaelh */
				if (val == 0) {
					/* and on that occasion also update last-used hostname+id to current values */
					if (hostname) strcpy(c_acc->hostname, hostname);
					if (addr) strcpy(c_acc->addr, addr);

					c_acc->acc_laston_real = c_acc->acc_laston = time(NULL);
					fseek(fp, -((signed int)sizeof(struct account)), SEEK_CUR);
					if (fwrite(c_acc, sizeof(struct account), 1, fp) < 1) {
						s_printf("Writing to account file failed: %s\n", feof(fp) ? "EOF" : strerror(ferror(fp)));
					}
				}
			}
			if (!leavepass || pass != NULL) {
				memset(c_acc->pass, 0, sizeof(c_acc->pass));
			}
			if (val != 0) {
				fclose(fp);
				WIPE(c_acc, struct account);
				s_printf("GetAccount: Password check failed.\n");
				return(FALSE);
			} else {
				fclose(fp);
				return(TRUE);
			}
		}
	}
	/* New accounts always have pass */
	if (!pass) {
		WIPE(c_acc, struct account);
		fclose(fp);
		s_printf("GetAccount: New account has no password.\n");
		return(FALSE);
	}

	/* New account creation: Ensure password minimum length */
	if (strlen(pass) < PASSWORD_MIN_LEN) {
		s_printf("Password length must be at least %d.\n", PASSWORD_MIN_LEN);
		return(FALSE);
	}

	if (strlen(pass) > ACCFILE_PASSWD_LEN) {
		s_printf("Password length must be at most %d.\n", ACCFILE_PASSWD_LEN);
		return(FALSE);
	}

	/* No account found. Create trial account */
	WIPE(c_acc, struct account);
	c_acc->id = new_accid();
	if (c_acc->id != 0L) {
		if (c_acc->id == 1)
			c_acc->flags = (ACC_ADMIN | ACC_NOSCORE);
		else {
			int i;

			c_acc->flags = (ACC_TRIAL | ACC_NOSCORE);
			if (hostname) strcpy(c_acc->hostname, hostname);
			if (addr) strcpy(c_acc->addr, addr);

			/* Log new account creation in the log file (for auto-coloring in do_cmd_help_aux() (when invoking /log command)) */
			s_printf("(%s) ACCOUNT_CREATED: <%s@%s>(%s) with flags %d\n", showtime(), name, hostname ? hostname : "<NULLHOST>", addr ? addr : "<NULLADDR>", c_acc->flags);

			/* Potentially add to "new players that need validation" list aka 'list-invalid.txt'. */
			for (i = 0; i < MAX_LIST_INVALID; i++) {
				if (!list_invalid_name[i][0]) {
					time_t ct = time(NULL);
					struct tm* ctl = localtime(&ct);
					//static char day_names[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
					//, day_names[ctl->tm_wday]

					/* add accountname to the list */
					strcpy(list_invalid_name[i], name);
					if (hostname) strcpy(list_invalid_host[i], hostname); else strcpy(list_invalid_host[i], "<NULLHOST>");
					if (addr) strcpy(list_invalid_addr[i], addr); else strcpy(list_invalid_addr[i], "<NULLADDR>");
					strcpy(list_invalid_date[i], format("%04d/%02d/%02d - %02d:%02d:%02dh", 1900 + ctl->tm_year, ctl->tm_mon + 1, ctl->tm_mday, ctl->tm_hour, ctl->tm_min, ctl->tm_sec));
					break;
				}
				if (strcmp(list_invalid_name[i], name)) continue;
				/* accountname is already on the list */
				break;
			}
			if (i == MAX_LIST_INVALID) s_printf("Warning: list-invalid is full.\n");
		}

		strncpy(c_acc->name, name, ACCFILE_NAME_LEN - 1);
		c_acc->name[ACCFILE_NAME_LEN - 1] = '\0';

		condense_name(buf, c_acc->name);
		strncpy(c_acc->name_normalised, buf, ACCFILE_NAME_LEN - 1);
		c_acc->name_normalised[ACCFILE_NAME_LEN - 1] = '\0';

		strncpy(c_acc->pass, t_crypt(pass, name), sizeof(c_acc->pass));
		c_acc->pass[sizeof(c_acc->pass) - 1] = '\0';
		if (!(WriteAccount(c_acc, TRUE))) {
			WIPE(c_acc, struct account);
			fclose(fp);
			s_printf("GetAccount: Account-write failed.\n");
			return(FALSE);
		}
	}
	memset(c_acc->pass, 0, sizeof(c_acc->pass));
	fclose(fp);
	if (c_acc->id) return(TRUE);

	WIPE(c_acc, struct account);
	s_printf("GetAccount: Account has no id.\n");
	return(FALSE);
}
/* Case-insensitive GetAccount() - but does NOT allow 'pass' and hence doesn't allow account-creation.
   Instead it takes correct_name parameter and sets it to the case-correct account name found. */
bool GetcaseAccount(struct account *c_acc, cptr name, char *correct_name, bool leavepass) {
	FILE *fp;
	char buf[1024];
	WIPE(c_acc, struct account);

	/* Hack: Assume empty pass, because case-insensitive accountname lookup does not allow new accounts to be created for now */
	char *pass = NULL;

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb+");
	if (!fp) {
		if (errno == ENOENT) {	/* ONLY if non-existent */
			s_printf("WARNING (GetcaseAccount): tomenet.acc file does not exist.\n (path=<%s>)\n", buf);
			fp = fopen(buf, "wb+");
			if (!fp) {
				s_printf("CRITICAL ERROR (GetcaseAccount): Couldn't create a new tomenet.acc file!\n");
				KILL(c_acc, struct account);
				return(FALSE);
			}
			s_printf("Generated new account file!\n");
		} else {
			s_printf("CRITICAL ERROR (GetcaseAccount): tomenet.acc file exists but is inaccessible!\n (path=<%s>)\n", buf);
			KILL(c_acc, struct account);
			return(FALSE);	/* failed */
		}
	}
	while (fread(c_acc, sizeof(struct account), 1, fp)) {
		if (c_acc->flags & ACC_DELD) continue;
		if (!strcasecmp(c_acc->name, name)) {
			int val;

			/* Write back the case-correct name? */
			if (correct_name != NULL) strcpy(correct_name, c_acc->name);

			if (pass == NULL) {	/* direct name lookup */
				val = 0;
			} else {
				val = strcmp(c_acc->pass, t_crypt(pass, name));

				/* Update the timestamp if the password is successfully verified - mikaelh */
				if (val == 0) {
					c_acc->acc_laston_real = c_acc->acc_laston = time(NULL);
					fseek(fp, -((signed int)sizeof(struct account)), SEEK_CUR);
					if (fwrite(c_acc, sizeof(struct account), 1, fp) < 1) {
						s_printf("Writing to account file failed: %s\n", feof(fp) ? "EOF" : strerror(ferror(fp)));
					}
				}
			}
			if (!leavepass || pass != NULL) {
				memset(c_acc->pass, 0, sizeof(c_acc->pass));
			}
			if (val != 0) {
				fclose(fp);
				WIPE(c_acc, struct account);
				return(FALSE);
			} else {
				fclose(fp);
				return(TRUE);
			}
		}
	}
	/* New accounts always have pass */
	if (!pass) {
		WIPE(c_acc, struct account);
		fclose(fp);
		return(FALSE);
	}

	/* No account found. Create trial account */
	WIPE(c_acc, struct account);
	c_acc->id = new_accid();
	if (c_acc->id != 0L) {
		if (c_acc->id == 1)
			c_acc->flags = (ACC_ADMIN | ACC_NOSCORE);
		else
			c_acc->flags = (ACC_TRIAL | ACC_NOSCORE);

		strncpy(c_acc->name, name, ACCFILE_NAME_LEN - 1);
		c_acc->name[ACCFILE_NAME_LEN - 1] = '\0';

		condense_name(buf, c_acc->name);
		strncpy(c_acc->name_normalised, buf, ACCFILE_NAME_LEN - 1);
		c_acc->name_normalised[ACCFILE_NAME_LEN - 1] = '\0';

		strncpy(c_acc->pass, t_crypt(pass, name), sizeof(c_acc->pass));
		c_acc->pass[sizeof(c_acc->pass) - 1] = '\0';
		if (!(WriteAccount(c_acc, TRUE))) {
			WIPE(c_acc, struct account);
			fclose(fp);
			return(FALSE);
		}
	}
	memset(c_acc->pass, 0, sizeof(c_acc->pass));
	fclose(fp);
	if (c_acc->id) {
		return(TRUE);
	} else {
		WIPE(c_acc, struct account);
		return(FALSE);
	}
}

/* Return account structure of a specified account name */
bool Admin_GetAccount(struct account *c_acc, cptr name) {
	FILE *fp;
	char buf[1024];
	WIPE(c_acc, struct account);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) return(FALSE); /* cannot access account file */
	while (fread(c_acc, sizeof(struct account), 1, fp)) {
		if (c_acc->flags & ACC_DELD) continue;
		if (!strcmp(c_acc->name, name)) {
			fclose(fp);
			return(TRUE);
		}
	}
	fclose(fp);
	WIPE(c_acc, struct account);
	return(FALSE);
}
/* Like Admin_GetAccount but case-insensitive and returns case-correct account name if found */
bool Admin_GetcaseAccount(struct account *c_acc, cptr name, char *correct_name) {
	FILE *fp;
	char buf[1024];
	WIPE(c_acc, struct account);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) return(FALSE); /* cannot access account file */
	while (fread(c_acc, sizeof(struct account), 1, fp)) {
		if (c_acc->flags & ACC_DELD) continue;
		if (!strcasecmp(c_acc->name, name)) {
			if (correct_name != NULL) strcpy(correct_name, c_acc->name);
			fclose(fp);
			return(TRUE);
		}
	}
	fclose(fp);
	WIPE(c_acc, struct account);
	return(FALSE);
}

/* Check for an account of similar name to 'name'. If one is found, the name
   will be forbiden to be used, except if 'accname' is identical to the found
   account's name:
   If checking for an account name, accname must be NULL.
   If checking for a character name, accname must be set to its account holder.
    - C. Blue */
/* Super-strict mode? Disallow (non-trivial) char/acc names that only have 1+
   _letter_ inserted somewhere compared to existing account names */
#define STRICT_SIMILAR_NAMES
/* Only apply super-strict check above to account names being created,
   let character names be created without this extra check. */
//#define SIMILAR_CHARNAMES_OK
bool lookup_similar_account(cptr name, cptr accname) {
	FILE *fp;
	char buf[1024], tmpname[ACCNAME_LEN > CNAME_LEN ? ACCNAME_LEN : CNAME_LEN];
	struct account acc;


	if (allow_similar_names) return(FALSE); //hack: always allow?

	/* special exceptions (admins and their player accounts) */
	if (!strcasecmp(name, "mikaelh") || /* vs 'mikael' */
	    !strcasecmp(name, "c. blue")) /* vs 'c.blue' */
		return(FALSE); //allow!

	WIPE(&acc, struct account);

	condense_name(tmpname, name);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) {
		s_printf("ERROR: COULDN'T ACCESS ACCOUNT FILE IN lookup_similar_account()!\n");
		return(FALSE); /* cannot access account file */
	}
	while (fread(&acc, sizeof(struct account), 1, fp)) {
		/* Make sure passwords don't leak */
		memset(acc.pass, 0, sizeof(acc.pass));

		/* Skip deleted entries */
		if (acc.flags & ACC_DELD) continue;

		/* We may create character names similar to our own account name as we like */
		if (accname && !strcmp(acc.name, accname)) {
			continue;
		}

#ifdef STRICT_SIMILAR_NAMES
		if (
 #ifdef SIMILAR_CHARNAMES_OK
		    /*only apply this check to account names being created, be lenient for character names */
		    !accname &&
 #endif
		    similar_names(name, acc.name)) {
			s_printf("lookup_similar_account failed.\n");
			return(TRUE);
		}
#endif

		/* Differring normalised names? Skip. */
		if (strcmp(acc.name_normalised, tmpname)) continue;

		/* We found same normalised names. Check! */
		fclose(fp);

		/* Identical name (account vs character)? that's fine. */
		if (!strcmp(acc.name, name)) {
			return(FALSE);
		}

		/* not identical but just too similar? forbidden! */
		s_printf("lookup_similar_account (4): name '%s', aname '%s' (tmp '%s')\n", name, acc.name, tmpname);
		return(TRUE);
	}
	fclose(fp);

	/* no identical/similar account found, all green! */
	return(FALSE);
}

/* Check for a character of similar name to 'name'. If one is found, the name
   will be forbiden to be used, except if 'accname' is identical to the found
   character's account name. - C. Blue */
/* Super-strict mode? Disallow (non-trivial) char/acc names that only have 1+
   _letter_ inserted somewhere compared to existing account names */
#define STRICT_SIMILAR_NAMES
/* Only apply super-strict check above to account names being created,
   let character names be created without this extra check. */
//#define SIMILAR_CHARNAMES_OK
bool lookup_similar_character(cptr name, cptr accname) {
	FILE *fp;
	char buf[1024], tmpname[ACCNAME_LEN > CNAME_LEN ? ACCNAME_LEN : CNAME_LEN];
	struct account acc;
return(FALSE); //TODO: Implement

	if (allow_similar_names) return(FALSE); //hack: always allow?

	/* special exceptions (admins and their player accounts) */
	if (!strcasecmp(name, "mikaelh") || /* vs 'mikael' */
	    !strcasecmp(name, "c. blue")) /* vs 'c.blue' */
		return(FALSE); //allow!

	WIPE(&acc, struct account);

	condense_name(tmpname, name);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) {
		s_printf("ERROR: COULDN'T ACCESS ACCOUNT FILE IN lookup_similar_character()!\n");
		return(FALSE); /* cannot access account file */
	}
	while (fread(&acc, sizeof(struct account), 1, fp)) {
		/* Make sure passwords don't leak */
		memset(acc.pass, 0, sizeof(acc.pass));

		/* Skip deleted entries */
		if (acc.flags & ACC_DELD) continue;

		/* We may create character names similar to our own account name as we like */
		if (accname && !strcmp(acc.name, accname)) {
			continue;
		}

#ifdef STRICT_SIMILAR_NAMES
		if (
 #ifdef SIMILAR_CHARNAMES_OK
		    /*only apply this check to account names being created, be lenient for character names */
		    !accname &&
 #endif
		    similar_names(name, acc.name)) {
			s_printf("lookup_similar_character failed.\n");
			return(TRUE);
		}
#endif

		/* Differring normalised names? Skip. */
		if (strcmp(acc.name_normalised, tmpname)) continue;

		/* We found same normalised names. Check! */
		fclose(fp);

		/* Identical name (account vs character)? that's fine. */
		if (!strcmp(acc.name, name)) {
			return(FALSE);
		}

		/* not identical but just too similar? forbidden! */
		s_printf("lookup_similar_character (4): name '%s', aname '%s' (tmp '%s')\n", name, acc.name, tmpname);
		return(TRUE);
	}
	fclose(fp);

	/* no identical/similar account found, all green! */
	return(FALSE);
}

/* Return account name of a specified PLAYER id */
cptr lookup_accountname(int p_id) {
	FILE *fp;
	char buf[1024];
	/* Static memory used for the return value */
	static struct account acc;
	u32b acc_id = lookup_player_account(p_id);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) return(NULL); /* cannot access account file */
	while (fread(&acc, sizeof(struct account), 1, fp)) {
		/* Make sure passwords don't leak */
		memset(acc.pass, 0, sizeof(acc.pass));

		/* Skip deleted entries */
		if (acc.flags & ACC_DELD) continue;

		/* Check account id */
		if (acc.id == acc_id) {
			fclose(fp);
			return(acc.name);
		}
	}
	fclose(fp);
	return(NULL);
}

/* Return account name of a specified account id.
   Does not return NULL but "" if account doesn't exist! */
cptr lookup_accountname2(u32b acc_id) {
	FILE *fp;
	char buf[1024];
	/* Static memory used for the return value */
	static struct account acc;

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) return(""); /* cannot access account file */
	while (fread(&acc, sizeof(struct account), 1, fp)) {
		/* Prevent the password from leaking */
		memset(acc.pass, 0, sizeof(acc.pass));

		/* Skip deleted entries */
		if (acc.flags & ACC_DELD) continue;

		/* Check the account id */
		if (acc.id == acc_id) {
			fclose(fp);
			return(acc.name);
		}
	}
	fclose(fp);
	return("");
}

/* our password encryptor */
static char *t_crypt(char *inbuf, cptr salt) {
#ifdef HAVE_CRYPT
	static char out[64];
 #if 1 /* fix for len-1 names */
	char setting[3];

	/* only 1 character long salt? expand to 2 chars length */
	if (!salt[1]) {
		setting[0] = '.';
		setting[1] = salt[0];
		setting[2] = 0;
		strcpy(out, (char*)crypt(inbuf, setting));
	} else
 #endif
 #if 0 /* 2021-12-22 - suddenly crypt() returns a null pointer if 3rd character is a space, wth */
	if (FALSE) {
		char fixed_name[ACCNAME_LEN];
		int n;

		strcpy(fixed_name, salt);
		for (n = 0; n < strlen(fixed_name); n++) {
			if (!fixed_name[n]) break;
			if (!((fixed_name[n] >= 'A' && fixed_name[n] <= 'Z') ||
			    (fixed_name[n] >= 'a' && fixed_name[n] <= 'z') ||
			    (fixed_name[n] >= '0' && fixed_name[n] <= '9')
			    //||fixed_name[n] == '.'))
			    ))
				fixed_name[n] = 'x';
		}
		strcpy(out, (char*)crypt(inbuf, fixed_name));
	} else
 #endif
 #if 1 /* SPACE _ ! - ' , and probably more as _2nd character_ cause crypt() to return a null pointer ('.' is ok) */
	if (!((salt[1] >= 'A' && salt[1] <= 'Z') ||
	    (salt[1] >= 'a' && salt[1] <= 'z') ||
	    (salt[1] >= '0' && salt[1] <= '9') ||
	    salt[1] == '.')) {
		char fixed_name[ACCNAME_LEN];

		strcpy(fixed_name, salt);
		fixed_name[1] = '.';
  #if 1 /* 2021-12-22 - suddenly crypt() returns a null pointer if 3rd character is a space, wth */
		fixed_name[2] = 0; //just terminate, as we only use 2 chars for salt anyway (!)
  #endif
		strcpy(out, (char*)crypt(inbuf, fixed_name));
	} else
  #if 1 /* 2021-12-22 - suddenly crypt() returns a null pointer if 3rd character is a space, wth */
	if (TRUE) {
		char fixed_name[ACCNAME_LEN];

		strcpy(fixed_name, salt);
		fixed_name[2] = 0; //just terminate, as we only use 2 chars for salt anyway (!)
		strcpy(out, (char*)crypt(inbuf, fixed_name));
	} else
  #endif
 #endif
		strcpy(out, (char*)crypt(inbuf, salt));
	return(out);
#else
	return(inbuf);
#endif
}

int check_account(char *accname, char *c_name, int *Ind) {
	struct account acc, acc2;
	u32b id, a_id;
	u32b flags;
	hash_entry *ptr;
	int i, success = 1;
#ifndef RPG_SERVER
	int ded_iddc, ded_pvp;
#endif
	bool acc1_success, acc2_success;
	/* Make sure noone creates a character of the same name as another player's accountname!
	   This is important for new feat of messaging to an account instead of character name. - C. Blue */
	char c2_name[MAX_CHARS];

	*Ind = 0;

	strcpy(c2_name, c_name);
	//c2_name[0] = toupper(c2_name[0]);
	acc1_success = GetAccount(&acc, accname, NULL, FALSE, NULL, NULL);
	acc2_success = GetAccount(&acc2, c2_name, NULL, FALSE, NULL, NULL);
	if (acc1_success && acc2_success && acc.id != acc2.id) {
		/* However, since ppl might have already created such characters, only apply this
		   rule for newly created characters, to avoid someone being unable to login on
		   an already existing character that unfortunately violates this rule :/ */
		int *id_list, chars;

		chars = player_id_list(&id_list, acc.id);
		for (i = 0; i < chars; i++)
			if (!strcmp(c_name, lookup_player_name(id_list[i]))) break;
		if (chars) C_KILL(id_list, chars, int);
		if (i == chars) return(0); /* 'name already in use' */
	} else if (!acc1_success && acc2_success) {
		return(0); /* we don't even have an account yet? 'name already in use' for sure */
	}

	if (acc1_success) {
		int *id_list, chars;
#ifndef RPG_SERVER
		int max_cpa = MAX_CHARS_PER_ACCOUNT, max_cpa_plus = 0, plus_free = 0;
#endif

		chars = player_id_list(&id_list, acc.id);
#ifdef RPG_SERVER /* Allow only up to 1 character per account! */
		/* If this account DOES have characters, but the chosen character name is
		   NOT equal to the first character of this account, don't allow it!
		   (To restrict players to only 1 character per account! - C. Blue) */
		/* allow multiple chars for admins, even on RPG server */
		if ((chars > 0) && strcmp(c_name, lookup_player_name(id_list[0])) && !(acc.flags & ACC_ADMIN)) {
			C_KILL(id_list, chars, int);
			return(-1);
		}
#else
 #ifdef ALLOW_DED_IDDC_MODE
		max_cpa_plus += MAX_DED_IDDC_CHARS;
		plus_free += MAX_DED_IDDC_CHARS;
 #endif
 #ifdef ALLOW_DED_PVP_MODE
		max_cpa_plus += MAX_DED_PVP_CHARS;
		plus_free += MAX_DED_PVP_CHARS;
 #endif

		ded_iddc = 0;
		ded_pvp = 0;
		for (i = 0; i < chars; i++) {
			u32b m = lookup_player_mode(id_list[i]);

			if ((m & MODE_DED_IDDC) && ded_iddc < MAX_DED_IDDC_CHARS) {
				ded_iddc++;
				plus_free--;
			}
			if ((m & MODE_DED_PVP) && ded_pvp < MAX_DED_PVP_CHARS)  {
				ded_pvp++;
				plus_free--;
			}
			/* paranoia (server-client version/type mismatch might cause this in the future) */
			if (plus_free < 0) {
				s_printf("debug error: plus_free is %d\n", plus_free);
				plus_free = 0;
			}
		}
		//s_printf("plus_free=%d, ded_iddc=%d, ded_pvp=%d\n", plus_free, ded_iddc, ded_pvp);
		//s_printf("chars=%d, max_cpa=%d, max_cpa_plus=%d\n", chars, max_cpa, max_cpa_plus);

		/* no more free chars */
		if (chars >= max_cpa + max_cpa_plus) {
			for (i = 0; i < chars; i++)
				if (!strcmp(c_name, lookup_player_name(id_list[i]))) break;
			/* We're out of free character slots: Char creation failed! */
			if (i == chars) {
				if (chars) C_KILL(id_list, chars, int);
				return(-3);
			}
		/* only exclusive char slots left */
		} else if (chars >= max_cpa + max_cpa_plus - plus_free) {
			/* paranoia (maybe if slot # gets changed again in the future) */
			if (ded_iddc == MAX_DED_IDDC_CHARS && ded_pvp == MAX_DED_PVP_CHARS) {
				/* out of character slots */
				if (chars) C_KILL(id_list, chars, int);
				return(-3);
			}
			if (ded_iddc == MAX_DED_IDDC_CHARS) success = -4; /* set char mode to MODE_DED_PVP */
			else if (ded_pvp == MAX_DED_PVP_CHARS) success = -5; /* set char mode to MODE_DED_IDDC */
			else success = -6; /* char mode can be either, make it depend on user choice */
		}
		/* at least one non-exclusive slot free */
		else {
 #if 1 /* new: allow any dedicated mode character to be created in 'normal' slots! */
			success = -7;
 #else
			if (ded_iddc < MAX_DED_IDDC_CHARS) {
				if (ded_pvp < MAX_DED_PVP_CHARS) success = -7; /* allow willing creation of any exlusive slot */
				else success = -8; /* allow willing creating of iddc-exclusive slot */
			} else if (ded_pvp < MAX_DED_PVP_CHARS) success = -9; /* allow willing creating of pvp-exclusive slot */
 #endif
		}
#endif
		a_id = acc.id;
		flags = acc.flags;

		if (chars) C_KILL(id_list, chars, int);

		id = lookup_player_id(c_name);
		ptr = lookup_player(id);
		/* Is the character either new, or is its ID belonging
		   to the same account as our account (ie it's us)? */
		if (!ptr || ptr->account == a_id) {
			/* Cannot create another character while being logged on, if not ACC_MULTI.
			   Allow to login with the same name to 'resume connection' though. */
			if (!(flags & ACC_MULTI)
			    && c_name[0]) { /* For use in slash.c, for /convertexclusive */
				player_type *p_ptr;

				/* check for login-timing exploit (currently makes subsequent 'normal' check redundant). */
				if ((i = check_multi_exploit(accname, c_name))) {
					*Ind = -i;
					return(-2);
				}
				/* check for normal ineligible multi-login attempts */
				for (i = 1; i <= NumPlayers; i++) {
					p_ptr = Players[i];
					if (p_ptr->account == a_id && !(flags & ACC_MULTI) && strcmp(c_name, p_ptr->name)) {
#ifdef ALLOW_LOGIN_REPLACE_IN_TOWN
						if (istown(&p_ptr->wpos)) {
							s_printf("Replacing connection for: <%s> <%s@%s>\n", p_ptr->name, p_ptr->realname, p_ptr->addr);
							Destroy_connection(p_ptr->conn, "replacing connection");
							break;
						} else {
							*Ind = i;
 #if 0 /* this is not thought out correctly: in nserver.c 'still has an alive connection' is sth we cannot really know, it might just be timing out instead! So, ... */
							return(-10);
 #else /* ...prefer -2, as it used to be before ALLOW_LOGIN_REPLACE_IN_TOWN: */
							return(-2);
 #endif
						}
#else
						*Ind = i;
						return(-2);
#endif
					}
				}
			}

			/* all green */
			return(success);
		}
		/* Log failure */
		s_printf("Name already in use: (%d vs %d) <%s>\n", a_id, ptr->account, ptr->accountname);
		//TODO maybe: Allow admin-dm to take over expired characters here if accountname is ""
	}
	/* "Name already in use by another player" (coming from 'else' branch above),
	   ie character isn't new and it belongs to a different account than ours. */
	return(0);
}

bool GetAccountID(struct account *c_acc, u32b id, bool leavepass) {
	FILE *fp;
	char buf[1024];

	/* we may want to store a local index for fast
	   id/name/filepos lookups in the future */
	WIPE(c_acc, struct account);

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) return(FALSE);	/* failed */
	while (fread(c_acc, sizeof(struct account), 1, fp)) {
		if (id == c_acc->id && !(c_acc->flags & ACC_DELD)) {
			if (!leavepass) memset(c_acc->pass, 0, sizeof(c_acc->pass));
			fclose(fp);
			return(TRUE);
		}
	}
	fclose(fp);
	WIPE(c_acc, struct account);
	return(FALSE);
}

static u32b new_accid() {
	u32b id;
	FILE *fp;
	char *t_map;
	char buf[1024];
	struct account t_acc;
	int num_entries = 0;
	id = account_id;

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb");
	if (!fp) return(0L);

	C_MAKE(t_map, MAX_ACCOUNTS / 8, char);
	while (fread(&t_acc, sizeof(struct account), 1, fp)) {
		if (t_acc.flags & ACC_DELD) continue;
		t_map[t_acc.id / 8] |= ((char)(1U << (t_acc.id % 8)));
		num_entries++;
	}

	fclose(fp);

	/* HACK - Make account id 1 unavailable if the file is not empty.
	 * This prevents the next new player from becoming an admin if the
	 * first account is ever deleted.
	 *  - mikaelh
	 */
	if (num_entries) {
		t_map[0] |= (1U << 1);
	}

	/* Make account id 0 unavailable just to be safe */
	t_map[0] |= (1U << 0);

	/* Find the next free account ID */
	for (id = account_id; id < MAX_ACCOUNTS; id++) {
		if (!(t_map[id / 8] & (1U << (id % 8)))) break;
	}

	if (id == MAX_ACCOUNTS) {
		/* Wrap around */
		for (id = 1; id < account_id; id++) {
			if (!(t_map[id / 8] & (1U << (id % 8)))) break;
		}

		/* Oops, no free account IDs */
		if (id == account_id) {
			s_printf("WARNING: No account ID numbers available!\n");
			id = 0;
		}
	}

	C_KILL(t_map, MAX_ACCOUNTS / 8, char);
	account_id = id + 1;

	return(id); /* temporary */
}

/*
 * Lookup a guild number by name.
 */
int guild_lookup(cptr name) {
	int i;

	/* Check each guild */
	for (i = 1; i < MAX_GUILDS; i++) {
		/* Check name */
		if (streq(guilds[i].name, name)) return(i);
	}

	/* No match */
	return(-1);
}

/*
 * Lookup a party number by name.
 */
int party_lookup(cptr name) {
	int i;

	/* Check each party */
	for (i = 1; i < MAX_PARTIES; i++) { /* was i = 0 but real parties start from i = 1 - mikaelh */
		/* Check name */
		if (streq(parties[i].name, name)) return(i);
	}

	/* No match */
	return(-1);
}


/*
 * Check for the existence of a player in a party.
 */
bool player_in_party(int party_id, int Ind) {
	player_type *p_ptr = Players[Ind];

	/* Check - Fail on non party */
	if (party_id && p_ptr->party == party_id)
		return(TRUE);

	/* Not in the party */
	return(FALSE);
}

static bool group_name_legal_characters(cptr name) {
	const char *ptr;

	/* remove special chars that are used for parsing purpose */
	for (ptr = &name[strlen(name)]; ptr-- > name; )
		if (!((*ptr >= 'A' && *ptr <= 'Z') ||
		    (*ptr >= 'a' && *ptr <= 'z') ||
		    (*ptr >= '0' && *ptr <= '9') ||
		    //strchr(" .,-'&_$%~#<>|", *ptr))) /* chars allowed for character name */
		    strchr(" .,-'&_$%~#", *ptr))) /* chars allowed for character name */
			return(FALSE);
	return(TRUE);
}

static bool party_name_legal(int Ind, char *name) {
	char *ptr, buf[NAME_LEN], buf2[NAME_LEN];

	if (strlen(name) >= NAME_LEN) {
		msg_format(Ind, "\377yParty name must not exceed %d characters!", NAME_LEN - 1);
		return(FALSE);
	}

	/* optional, but tired of lame names */
	if (strlen(name) < PARTY_NAME_MIN_LEN) {
		msg_format(Ind, "\377yParty name must have at least %d characters!", PARTY_NAME_MIN_LEN);
		return(FALSE);
	}

	strncpy(buf2, name, NAME_LEN);
	buf[NAME_LEN - 1] = '\0';
	/* remove spaces at the beginning */
	for (ptr = buf2; ptr < buf2 + strlen(buf2); ) {
		if (isspace(*ptr)) ptr++;
		else break;
	}
	strcpy(buf, ptr);
	/* name consisted only of spaces? */
	if (!buf[0]) {
		msg_print(Ind, "\377ySorry, names must not just consist of spaces.");
		return(FALSE);
	}

	/* remove spaces at the end */
	for (ptr = buf + strlen(buf); ptr-- > buf; ) {
		if (isspace(*ptr)) *ptr = '\0';
		else break;
	}
	/* name consisted only of spaces? */
	if (!buf[0]) {
		msg_print(Ind, "\377ySorry, names must not just consist of spaces.");
		return(FALSE);
	}
	strcpy(name, buf);

	/* Check for weird characters */
	if (!group_name_legal_characters(name)) {
		msg_print(Ind, "\377ySorry, that name contains illegal characters or symbols.");
		return(FALSE);
	}
	/* Prevent abuse */
	strcpy(buf2, name);
	if (ILLEGAL_GROUP_NAME(name) || handle_censor(buf2)) {
		msg_print(Ind, "\377yThat party name is not available, please try again.");
		return(FALSE);
	}

	/* Check for already existing party by that name */
	if (party_lookup(name) != -1) {
		msg_print(Ind, "\377yA party by that name already exists.");
		return(FALSE);
	}
	/* Specific 'Iron Team' collision? */
	if (!strcasecmp(name, "Iron Team")) { //exact match only, allow 'Iron Team of XYZ' style names
		msg_print(Ind, "\377yA party may not exactly be called 'Iron Team'. Vary the name a bit.");
		return(FALSE);
	}

	/* Check for already existing guild by that name */
	if (guild_lookup(name) != -1) {
		msg_print(Ind, "\377yThere's already a guild using that name.");
		return(FALSE);
	}
	/* Specific 'Merchants Guild' collision? */
	if (my_strcasestr(name, "Merchant") || my_strcasestr(name, "Mercant")) { //catch silyl typoing :p
		msg_print(Ind, "\377yThere's already a guild using a too similar name."); //..and it's run by NPCs :)
		return(FALSE);
	}

	/* Success */
	return(TRUE);
}

static bool guild_name_legal(int Ind, char *name) {
	int index;
	char *ptr, buf[NAME_LEN];
	player_type *p_ptr = Players[Ind];
	object_type forge, *o_ptr = &forge;

	char buf2[NAME_LEN];


	if (strlen(name) >= NAME_LEN) {
		msg_format(Ind, "\377yGuild name must not exceed %d characters!", NAME_LEN - 1);
		return(FALSE);
	}

	/* optional, but tired of lame names */
	if (strlen(name) < GUILD_NAME_MIN_LEN) {
		msg_format(Ind, "\377yGuild name must have at least %d characters!", GUILD_NAME_MIN_LEN);
		return(FALSE);
	}

	strncpy(buf, name, NAME_LEN);
	buf[NAME_LEN - 1] = '\0';
	/* remove spaces at the beginning */
	for (ptr = buf; ptr < buf + strlen(buf); ) {
		if (isspace(*ptr)) ptr++;
		else break;
	}
	strcpy(buf, ptr);
	/* remove spaces at the end */
	for (ptr = buf + strlen(buf); ptr-- > buf; ) {
		if (isspace(*ptr)) *ptr = '\0';
		else break;
	}
	/* name consisted only of spaces? */
	if (!buf[0]) {
		msg_print(Ind, "\377ySorry, names must not just consist of spaces.");
		return(FALSE);
	}
	strcpy(name, buf);

	/* Check for weird characters */
	if (!group_name_legal_characters(name)) {
		msg_print(Ind, "\377ySorry, that name contains illegal characters or symbols.");
		return(FALSE);
	}

	/* Prevent abuse */
	strcpy(buf2, name);
	if (ILLEGAL_GROUP_NAME(name) || handle_censor(buf2)) {
		msg_print(Ind, "\377yThat guild name is not available, please try again.");
		return(FALSE);
	}

	/* Check for already existing party by that name */
	if (party_lookup(name) != -1) {
		msg_print(Ind, "\377yThere's already a party using that name.");
		return(FALSE);
	}

	/* Specific 'Merchants Guild' collision? */
	if (my_strcasestr(name, "Merchant") || my_strcasestr(name, "Mercant")) { //catch silyl typoing :p
		msg_print(Ind, "\377yA guild by a too similar name already exists."); //..and it's run by NPCs :)
		return(FALSE);
	}

	/* Check for already existing guild by that name */
	if ((index = guild_lookup(name) != -1)) {
		/* Admin can actually create a duplicate 'spare' key this way */
		if (p_ptr->admin_dm) {
			/* make the guild key */
			invcopy(o_ptr, lookup_kind(TV_KEY, SV_GUILD_KEY));
			o_ptr->number = 1;
			o_ptr->pval = index;
			o_ptr->level = 1;
			o_ptr->owner = p_ptr->id;
			o_ptr->ident |= ID_NO_HIDDEN;
			o_ptr->mode = p_ptr->mode;
			object_known(o_ptr);
			object_aware(Ind, o_ptr);
			(void)inven_carry(Ind, o_ptr);
			msg_print(Ind, "Spare key created.");
			return(FALSE);
		}
		msg_print(Ind, "\377yA guild by that name already exists.");
		return(FALSE);
	}

	/* Check for already existing guild with too similar name */
	condense_name(buf, name);
	/* Check each guild */
	for (index = 1; index < MAX_GUILDS; index++) {
		/* for renaming a guild: skip similarity-check if it's our own guild! */
		if (guilds[index].master == p_ptr->id) continue;

		/* compare condensed names */
		condense_name(buf2, guilds[index].name);
		if (!strcmp(buf, buf2)) {
			msg_print(Ind, "\377yA guild by a too similar name already exists.");
			return(FALSE);
		}

//#ifdef STRICT_SIMILAR_NAMES
#if 1
		if (
 #if 1
		    TRUE &&
 #else
 //#ifdef SIMILAR_CHARNAMES_OK
		    FALSE &&
 #endif
		    similar_names(buf, buf2)) {
			s_printf("guild_name_legal failure.\n");
			msg_print(Ind, "\377yA guild by a too similar name already exists.");
			return(FALSE);
		}
#endif
	}

	return(TRUE);
}

/*
 * Create a new guild.
 */
int guild_create(int Ind, cptr name) {
	player_type *p_ptr = Players[Ind];
	int index = 0, i, j;
	object_type forge, *o_ptr = &forge;
	char temp[160];
	struct account acc;
	int *id_list, ids;
	bool success;

	strcpy(temp, name);
	if (!guild_name_legal(Ind, temp)) return(FALSE);

	/* zonk */
	if ((p_ptr->mode & MODE_PVP)) {
		msg_print(Ind, "\377yPvP characters may not create a guild.");
		return(FALSE);
	}
	if ((p_ptr->mode & MODE_SOLO)) {
		msg_print(Ind, "\377ySoloist characters may not create a guild.");
		return(FALSE);
	}

	/* anti-cheeze: People could get an extra house on each character.
	   So we allow only one guild master per player account to at least
	   reduce the nonsense to 1 extra house per Account.. */
	success = GetAccount(&acc, p_ptr->accountname, NULL, FALSE, NULL, NULL);
	/* paranoia */
	if (!success) {
		/* uhh.. */
		msg_print(Ind, "Sorry, guild creation has failed.");
		return(FALSE);
	}
	ids = player_id_list(&id_list, acc.id);
	for (i = 0; i < ids; i++) {
		if ((j = lookup_player_guild(id_list[i])) && /* one of his characters is in a guild.. */
		    guilds[j].master == id_list[i]) { /* ..and he is actually the master of that guild? */
			msg_print(Ind, "\377yOnly one character per account is allowed to be a guild master.");
			return(FALSE);
		}
	}
	if (ids) C_KILL(id_list, ids, int);

	/* Make sure this guy isn't in some other guild already */
	if (p_ptr->guild != 0) {
		msg_print(Ind, "\377yYou already belong to a guild!");
		return(FALSE);
	}
	if (p_ptr->lev < 30) {
		msg_print(Ind, "\377yYou are not high enough level to start a guild.");
		return(FALSE);
	}
	/* This could probably be improved. */
	if (p_ptr->au < GUILD_PRICE) {
		if (GUILD_PRICE >= 1000000)
			msg_format(Ind, "\377yYou need %d,000,000 gold pieces to start a guild.", GUILD_PRICE / 1000000);
		else if (GUILD_PRICE >= 1000)
			msg_format(Ind, "\377yYou need %d,000 gold pieces to start a guild.", GUILD_PRICE / 1000);
		else
			msg_format(Ind, "\377yYou need %d gold pieces to start a guild.", GUILD_PRICE);
		return(FALSE);
	}

	/* Find the "best" party index */
	for (i = 1; i < MAX_GUILDS; i++) {
		if (guilds[i].members == 0) {
			index = i;
			break;
		}
	}
	/* Make sure we found an empty slot */
	if (index == 0) {
		/* Error */
		msg_print(Ind, "\377yThere aren't enough guild slots!");
		return(FALSE);
	}


	/* Set guild identity */
	guilds[index].dna = (u32b)rand_int(0xFFFF) << 16;
	guilds[index].dna += rand_int(0xFFFF);

	/* Set guild name */
	strcpy(guilds[index].name, temp);

	/* Set guildmaster */
	guilds[index].master = p_ptr->id;
	guilds[index].cmode = p_ptr->mode;

	/* Init guild hall, flags & level */
	guilds[index].h_idx = 0;
	guilds[index].flags = 0;
	guilds[index].minlev = 0;

	/* Add the owner as a member */
	p_ptr->guild = index;
	p_ptr->guild_dna = guilds[index].dna;
	clockin(Ind, 3);
	guilds[index].members = 1;

	/* Set guild mode */
	if ((p_ptr->mode & MODE_EVERLASTING))
		guilds[index].flags |= GFLG_EVERLASTING;

	Send_guild(Ind, FALSE, FALSE);
	Send_guild_config(index);

	/* broadcast the news */
	snprintf(temp, 160, "\374\377yA new guild '\377%c%s\377y' has been created.", COLOUR_CHAT_GUILD, guilds[index].name);
	msg_broadcast(0, temp);
	//msg_print(Ind, "\374\377Gou can adjust guild options with the '/guild_cfg' command.");
	s_printf("GUILD_CREATE: (by %s) '%s'\n", p_ptr->name, guilds[index].name);
	l_printf("%s \\{yA new guild '%s' has been created\n", showdate(), guilds[index].name);

	p_ptr->au -= GUILD_PRICE;
	p_ptr->redraw |= PR_GOLD;

	/* make the guild key */
	invcopy(o_ptr, lookup_kind(TV_KEY, SV_GUILD_KEY));
	o_ptr->number = 1;
	o_ptr->pval = index;
	o_ptr->level = 1;
	o_ptr->owner = p_ptr->id;
	o_ptr->ident |= ID_NO_HIDDEN;
	o_ptr->mode = p_ptr->mode;
	o_ptr->iron_trade = p_ptr->iron_trade;
	//o_ptr->iron_turn = turn;
	o_ptr->iron_turn = 2147483647; //=_='
	object_known(o_ptr);
	object_aware(Ind, o_ptr);
	(void)inven_carry(Ind, o_ptr);

	/* Give the guildmaster some scrolls for a hall */
#if 0	//scrolls are broken
	invcopy(o_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_HOUSE));
	o_ptr->number = 6;
	o_ptr->level = p_ptr->lev;
	o_ptr->owner = p_ptr->id;
	o_ptr->mode = p_ptr->mode;
	o_ptr->discount = 50;
	object_known(o_ptr);
	object_aware(Ind, o_ptr);
	(void)inven_carry(Ind, o_ptr);
#endif

	return(TRUE);
}

/*
 * New party check function - to be timed
 *
 */
void party_check(int Ind) {
	int i, id;

	for (i = 1; i < MAX_PARTIES; i++) {
		if (parties[i].members != 0) {
			if (!(id = lookup_player_id(parties[i].owner))) {
				msg_format(Ind, "Lost party %s (%s)", parties[i].name, parties[i].owner);
				del_party(i);
			} else {
				if ((lookup_player_party(id) != i)) {
					msg_format(Ind, "Disowned party %s (%s)", parties[i].name, parties[i].owner);
					del_party(i);
				}
			}
		}
	}
}

/*
 * as with party checker, scan ALL player entries
 * if they are not linked to an existing account,
 * delete them.
 */
void account_check(int Ind) { /* Temporary Ind */
	hash_entry *ptr;
	int i, del;
	struct account acc;
	//player_type *p_ptr = Players[Ind];

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr) {
			/* Check this name */
			if (!GetAccountID(&acc, ptr->account, FALSE)) {
				s_printf("Lost player: %s\n", ptr->name);
				msg_format(Ind, "Lost player: %s", ptr->name);
				del = ptr->id;
				delete_player_id(del);
			}

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}
}

/*
 * Create a new party, owned by "Ind", and called
 * "name".
 */
int party_create(int Ind, cptr name) {
	player_type *p_ptr = Players[Ind];
	int index = 0, i, oldest = turn;
	char temp[160];

	strcpy(temp, name);
	if (!party_name_legal(Ind, temp)) return(FALSE);

	/* zonk */
	if ((p_ptr->mode & MODE_PVP)) {
		msg_print(Ind, "\377yPvP characters may not create a party.");
		return(FALSE);
	}
	if ((p_ptr->mode & MODE_SOLO)) {
		msg_print(Ind, "\377ySoloist characters may not create a party.");
		return(FALSE);
	}

	/* If he's party owner, it's name change */
	if (p_ptr->party) {
		if (parties[p_ptr->party].mode & PA_IRONTEAM) {
			msg_print(Ind, "\377yYour party is an Iron Team. Choose '(2) Create or rename an iron team' instead.");
			return(FALSE);
		}

		if (!streq(parties[p_ptr->party].owner, p_ptr->name)) {
			msg_print(Ind, "\377yYou must be the party owner to rename the party.");
			return(FALSE);
		}

		strcpy(parties[p_ptr->party].name, temp);

		/* Tell the party about its new name */
		party_msg_format(p_ptr->party, "\377%cYour party is now called '%s'.", COLOUR_CHAT_GUILD, temp);

		Send_party(Ind, FALSE, FALSE);
		return(TRUE);
	}

	/* Make sure this guy isn't in some other party already */
	if (p_ptr->party != 0) {
		msg_print(Ind, "\377yYou already belong to a party!");
		return(FALSE);
	}

#ifdef IDDC_IRON_COOP
	if (in_irondeepdive(&p_ptr->wpos)) {
		msg_print(Ind, "\377yCharacters must be outside the Ironman Deep Dive Challenge to form a party.");
		return(FALSE);
	}
 #ifdef IDDC_IRON_TEAM_ONLY
	/* we might just be passing by and not intending to enter IDDC, but we probably do intend to enter */
	if (on_irondeepdive(&p_ptr->wpos)) {
		msg_print(Ind, "\377yYou can only form 'Iron Team' parties for the Ironman Deep Dive Challenge.");
		return(FALSE);
	}
 #endif
#else
 #ifdef IDDC_IRON_TEAM_ONLY
	/* we might just be passing by and not intending to enter IDDC, but we probably do intend to enter */
	if (at_irondeepdive(&p_ptr->wpos)) {
		msg_print(Ind, "\377yYou can only form 'Iron Team' parties for the Ironman Deep Dive Challenge.");
		return(FALSE);
	}
 #endif
#endif

	/* Find the "best" party index */
	for (i = 1; i < MAX_PARTIES; i++) {
		/* Check deletion time of disbanded parties */
		if (parties[i].members == 0 && parties[i].created < oldest) {
			/* Track oldest */
			oldest = parties[i].created;
			index = i;
		}
	}

	/* Make sure we found an empty slot */
	if (index == 0 || oldest == turn) {
		/* Error */
		msg_print(Ind, "\377yThere aren't enough party slots!");
		return(FALSE);
	}

	/* Set party name */
	strcpy(parties[index].name, temp);

	/* Set owner name */
	strcpy(parties[index].owner, p_ptr->name);

	/* Set mode to normal */
	parties[index].mode = PA_NORMAL;
	parties[index].cmode = p_ptr->mode;
	/* For IDDC (if normal parties are actually allowed) */
	parties[index].iron_trade = (u32b)rand_int(0xFFFF) << 16;
	parties[index].iron_trade += rand_int(0xFFFF);

	/* Add the owner as a member */
	p_ptr->party = index;
	clockin(Ind, 2);
	parties[index].members = 1;
	p_ptr->iron_trade = parties[index].iron_trade;
	p_ptr->iron_turn = turn;

	/* Set the "creation time" */
	parties[index].created = turn;

	/* Resend party info */
	Send_party(Ind, FALSE, FALSE);

	/* Success */
	return(TRUE);
}

int party_create_ironteam(int Ind, cptr name) {
	player_type *p_ptr = Players[Ind];
	int index = 0, i, oldest = turn;
	char temp[160];

	strcpy(temp, name);
	if (!party_name_legal(Ind, temp)) return(FALSE);

	if ((p_ptr->mode & MODE_SOLO)) {
		msg_print(Ind, "\377ySoloist characters may not create a party.");
		return(FALSE);
	}

	/* If he's party owner, it's name change -- check that he is already in a party though! */
	if (p_ptr->party) {
		if (!(parties[p_ptr->party].mode & PA_IRONTEAM)) {
			msg_print(Ind, "\377yYour party is not an Iron Team. Choose '(1) Create or rename a party' instead.");
			return(FALSE);
		}

		if (!streq(parties[p_ptr->party].owner, p_ptr->name)) {
			msg_print(Ind, "\377yYou must be the iron team's owner to rename the team.");
			return(FALSE);
		}

		strcpy(parties[p_ptr->party].name, temp);

		/* Tell the party about its new name */
		party_msg_format(p_ptr->party, "\377%cYour iron team is now called '%s'.", COLOUR_CHAT_GUILD, temp);

		Send_party(Ind, FALSE, FALSE);
		return(TRUE);
	}

	/* Only newly created characters can create an iron team */
	if (p_ptr->max_exp > 0 || p_ptr->max_plv > 1) {
		msg_print(Ind, "\377yOnly newly created characters without experience can create an iron team.");
		return(FALSE);
	}

	/* If he's party owner, it's name change */
	if (streq(parties[p_ptr->party].owner, p_ptr->name)) {
		if (!(parties[p_ptr->party].mode & PA_IRONTEAM)) {
			msg_print(Ind, "\377yYour party isn't an Iron Team. Choose '1) Create a party' instead.");
			return(FALSE);
		}

		strcpy(parties[p_ptr->party].name, temp);

		/* Tell the party about its new name */
		party_msg_format(p_ptr->party, "\377%cYour iron team is now called '%s'.", COLOUR_CHAT_GUILD, temp);

		Send_party(Ind, FALSE, FALSE);
		return(TRUE);
	}

	/* Make sure this guy isn't in some other party already */
	if (p_ptr->party != 0) {
		msg_print(Ind, "\377yYou already belong to a party!");
		return(FALSE);
	}

#ifdef IDDC_IRON_COOP
	if (in_irondeepdive(&p_ptr->wpos)) {
		msg_print(Ind, "\377yCharacters must be outside the Ironman Deep Dive Challenge to form a party.");
		return(FALSE);
	}
#endif

	/* Find the "best" party index */
	for (i = 1; i < MAX_PARTIES; i++) {
		/* Check deletion time of disbanded parties */
		if (parties[i].members == 0 && parties[i].created < oldest) {
			/* Track oldest */
			oldest = parties[i].created;
			index = i;
		}
	}

	/* Make sure we found an empty slot */
	if (index == 0 || oldest == turn) {
		/* Error */
		msg_print(Ind, "\377yThere aren't enough party slots!");
		return(FALSE);
	}

	/* Set party name */
	strcpy(parties[index].name, temp);

	/* Set owner name */
	strcpy(parties[index].owner, p_ptr->name);

	/* Set mode to iron team */
	parties[index].mode = PA_IRONTEAM;
	parties[index].cmode = p_ptr->mode;
	parties[index].iron_trade = (u32b)rand_int(0xFFFF) << 16;
	parties[index].iron_trade += rand_int(0xFFFF);

	/* Initialize max exp */
	parties[index].experience = 0;

	/* Add the owner as a member */
	p_ptr->party = index;
	clockin(Ind, 2);
	parties[index].members = 1;
	p_ptr->iron_trade = parties[index].iron_trade;
	p_ptr->iron_turn = turn;

	/* Set the "creation time" */
	parties[index].created = turn;

	/* Resend party info */
	Send_party(Ind, FALSE, FALSE);

	/* Success */
	return(TRUE);
}

/*
 * Add player to a guild
 */
/* Allow the guild master or an added to use any other character on their
   account too, as long as they are guild members, to add other players? */
#define GUILD_ELIGIBLE_ACCOUNT_CAN_ADD
int guild_add(int adder, cptr name) {
	player_type *p_ptr;
	player_type *q_ptr = Players[adder];
	int guild_id = q_ptr->guild, Ind = 0, i;
	bool success;
#ifdef GUILD_ELIGIBLE_ACCOUNT_CAN_ADD
	int *id_list, ids;
	struct account acc;
	bool far_success = FALSE;
#endif

	if (!guild_id) {
		msg_print(adder, "\377yYou are not in a guild");
		return(FALSE);
	}

	Ind = name_lookup_loose(adder, name, FALSE, TRUE, FALSE);
	if (Ind <= 0) return(FALSE);
	p_ptr = Players[Ind];

	/* Leaderless guilds do not allow addition of new members */
	if (!lookup_player_name(guilds[guild_id].master)) {
		msg_print(adder, "\377yNo new members can be added while the guild is leaderless.");
		return(FALSE);
	}

	/* Everlasting and other chars cannot be in the same guild */
	if (compat_pmode(adder, Ind, TRUE)) {
		msg_format(adder, "\377yYou cannot add %s characters to this guild.", compat_pmode(adder, Ind, TRUE));
		return(FALSE);
	}

	if (p_ptr->mode & MODE_SOLO) {
		msg_print(Ind, "\377yThat player is a soloist. Those cannot join guilds.");
		return(FALSE);
	}

	/* Make sure this added person is neutral */
	if (p_ptr->guild != 0) {
		/* Message */
		if (p_ptr->guild != guild_id) msg_print(adder, "\377yThat player is already in a guild.");
		else msg_print(adder, "\377yThat player is already in your guild.");

		/* Abort */
		return(FALSE);
	}

	if (p_ptr->lev < guilds[guild_id].minlev) {
		msg_format(adder, "\377yThat player does not meet the minimum level requirements, %d, of the guild.", guilds[guild_id].minlev);
		return(FALSE);
	}

#ifdef GUILD_ELIGIBLE_ACCOUNT_CAN_ADD
	/* check if he has a character in there already, to be eligible to self-add */
	success = GetAccount(&acc, q_ptr->accountname, NULL, FALSE, NULL, NULL);
	/* paranoia */
	if (!success) {
		/* uhh.. */
		msg_print(Ind, "Sorry, adding has failed.");
		return(FALSE);
	}
	ids = player_id_list(&id_list, acc.id);
	for (i = 0; i < ids; i++) {
		if (lookup_player_guild(id_list[i]) != guild_id) continue;

		if (id_list[i] != guilds[guild_id].master &&
		    (!(guilds[guild_id].flags & GFLG_ALLOW_ADDERS) ||
		    !(lookup_player_guildflags(id_list[i]) & PGF_ADDER)))
			continue;

		/* Log - security */
		s_printf("GUILD_ADD_FAR: %s has been added to %s by %s.\n", p_ptr->name, guilds[guild_id].name, q_ptr->name);
		far_success = TRUE;
		break;
	}
	if (ids) C_KILL(id_list, ids, int);
	/* failure? */
	if (!far_success)
#endif

	/* Make sure this isn't an impostor */
	if (!((guilds[guild_id].flags & GFLG_ALLOW_ADDERS) && (q_ptr->guild_flags & PGF_ADDER))
	    && guilds[guild_id].master != q_ptr->id
	    && !is_admin(q_ptr)) {
		msg_print(adder, "\377yYou cannot add new members.");
		return(FALSE);
	}

	/* Ignoring a player will prevent getting added to a party by him */
	if (check_ignore(Ind, adder)) return(FALSE);

	/* Log - security */
	if (!far_success) s_printf("GUILD_ADD: %s has been added to %s by %s.\n", p_ptr->name, guilds[guild_id].name, q_ptr->name);

	/* Tell the guild about its new member */
	guild_msg_format(guild_id, "\374\377y%s has been added to %s by %s.", p_ptr->name, guilds[guild_id].name, q_ptr->name);

	/* One more player in this guild */
	guilds[guild_id].members++;

	/* Tell him about it */
	msg_format(Ind, "\374\377yYou've been added to '%s' by %s.", guilds[guild_id].name, q_ptr->name);

	/* Set his guild number */
	p_ptr->guild = guild_id;
	p_ptr->guild_dna = guilds[guild_id].dna;
	clockin(Ind, 3);

	/* Resend info */
	Send_guild(Ind, FALSE, FALSE);

	/* Display the guild note to him */
	for (i = 0; i < MAX_GUILDNOTES; i++) {
		if (!strcmp(guild_note_target[i], guilds[p_ptr->guild].name)) {
			if (strcmp(guild_note[i], ""))
				msg_format(Ind, "\374\377bGuild Note: \377%c%s", COLOUR_CHAT_GUILD, p_ptr->censor_swearing ? guild_note[i] : guild_note_u[i]);
			break;
		}
	}

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Success */
	return(TRUE);
}
int guild_add_self(int Ind, cptr guild) {
	player_type *p_ptr = Players[Ind];
	int guild_id = guild_lookup(guild), i, *id_list, ids;
	struct account acc;
	bool member = FALSE, success;

	/* no guild name specified? */
	if (!guild[0]) return(FALSE);

	if (p_ptr->mode & MODE_SOLO) {
		msg_print(Ind, "\377yAs a soloist you cannot join guilds.");
		return(FALSE);
	}

	if (guild_id == -1) {
		msg_print(Ind, "That guild does not exist.");
		return(FALSE);
	}

	if (p_ptr->lev < guilds[guild_id].minlev) {
		msg_format(Ind, "\377yYou do not meet the minimum level requirements, %d, of the guild.", guilds[guild_id].minlev);
		return(FALSE);
	}

	/* check if he has a character in there already, to be eligible to self-add */
	success = GetAccount(&acc, p_ptr->accountname, NULL, FALSE, NULL, NULL);
	/* paranoia */
	if (!success) {
		/* uhh.. */
		msg_print(Ind, "Sorry, self-adding has failed.");
		return(FALSE);
	}
	ids = player_id_list(&id_list, acc.id);
	for (i = 0; i < ids; i++) {
		if (lookup_player_guild(id_list[i]) == guild_id) {
			member = TRUE;

			/* Everlasting and other chars cannot be in the same guild */
			if (compat_mode(p_ptr->mode, lookup_player_mode(id_list[i]))) {
				msg_format(Ind, "\377yYou cannot join %s guilds.", compat_mode(p_ptr->mode, lookup_player_mode(id_list[i])));
				if (ids) C_KILL(id_list, ids, int);
				return(FALSE);
			}

			/* player is guild master? -> ok */
			if (id_list[i] == guilds[guild_id].master) {
				/* Log - security */
				s_printf("GUILD_ADD_SELF: (master) %s has been added to %s.\n", p_ptr->name, guilds[guild_id].name);

				/* success */
				break;
			}
			/* Make sure this isn't an impostor */
			if (!(guilds[guild_id].flags & GFLG_ALLOW_ADDERS)) continue;
			if (!(lookup_player_guildflags(id_list[i]) & PGF_ADDER)) continue;

			/* Log - security */
			s_printf("GUILD_ADD_SELF: (adder) %s has been added to %s.\n", p_ptr->name, guilds[guild_id].name);

			/* success */
			break;
		}
	}
	if (ids) C_KILL(id_list, ids, int);
	/* failure? */
	if (i == ids) {
		if (!member) msg_print(Ind, "You do not have any character that is member of that guild.");
		else msg_print(Ind, "You have no character in that guild that is allowed to add others.");
		return(FALSE);
	}

	/* Tell the guild about its new member */
	guild_msg_format(guild_id, "\374\377y%s has been added to %s.", p_ptr->name, guilds[guild_id].name);

	/* One more player in this guild */
	guilds[guild_id].members++;

	/* Tell him about it */
	msg_format(Ind, "\374\377yYou've been added to '%s'.", guilds[guild_id].name);

	/* Set his guild number */
	p_ptr->guild = guild_id;
	p_ptr->guild_dna = guilds[guild_id].dna;
	clockin(Ind, 3);

	/* Resend info */
	Send_guild(Ind, FALSE, FALSE);

	/* Display the guild note to him */
	for (i = 0; i < MAX_GUILDNOTES; i++) {
		if (!strcmp(guild_note_target[i], guilds[p_ptr->guild].name)) {
			if (strcmp(guild_note[i], ""))
				msg_format(Ind, "\374\377bGuild Note: \377%c%s", COLOUR_CHAT_GUILD, p_ptr->censor_swearing ? guild_note[i] : guild_note_u[i]);
			break;
		}
	}

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Success */
	return(TRUE);
}

int guild_auto_add(int Ind, int guild_id, char *message) {
	player_type *p_ptr = Players[Ind];
	int i;

	/* paranoia */
	if (!guild_id) return(FALSE);
	if (p_ptr->guild) return(FALSE);

	if (p_ptr->mode & MODE_SOLO) return(FALSE);

	if (!(guilds[guild_id].flags & GFLG_AUTO_READD)) return(FALSE);

	/* currently not eligible */
	if (p_ptr->mode & MODE_PVP) return(FALSE);

	/* Everlasting and other chars cannot be in the same guild */
	if (guilds[guild_id].flags & GFLG_EVERLASTING) {
		if (!(p_ptr->mode & MODE_EVERLASTING)) return(FALSE);
	} else
		if ((p_ptr->mode & MODE_EVERLASTING)) return(FALSE);

	/* Log - security */
	s_printf("GUILD_ADD_AUTO: %s has been added to %s.\n", p_ptr->name, guilds[guild_id].name);

	/* Tell the guild about its new member */
	//sprintf(message, "\374\377y%s has been auto-added to %s.", p_ptr->name, guilds[guild_id].name);
	//guild_msg_format(guild_id, "\374\377y%s has been auto-added to %s.", p_ptr->name, guilds[guild_id].name);
	guild_msg_format(guild_id, "\374\377y%s has been added to %s.", p_ptr->name, guilds[guild_id].name);

	/* One more player in this guild */
	guilds[guild_id].members++;

	/* Tell him about it */
	msg_format(Ind, "\374\377yYou've been added to '%s'.", guilds[guild_id].name);

	/* Set his guild number */
	p_ptr->guild = guild_id;
	p_ptr->guild_dna = guilds[guild_id].dna;
	clockin(Ind, 3);

	/* Resend info */
	Send_guild(Ind, FALSE, FALSE);

	/* Display the guild note to him */
	for (i = 0; i < MAX_GUILDNOTES; i++) {
		if (!strcmp(guild_note_target[i], guilds[p_ptr->guild].name)) {
			if (strcmp(guild_note[i], ""))
				msg_format(Ind, "\374\377bGuild Note: \377%c%s", COLOUR_CHAT_GUILD, p_ptr->censor_swearing ? guild_note[i] : guild_note_u[i]);
			break;
		}
	}

	/* Success */
	return(TRUE);
}

/*
 * Add a player to a party.
 */
int party_add(int adder, cptr name) {
	player_type *p_ptr;
	player_type *q_ptr = Players[adder];
	int party_id = q_ptr->party, Ind = 0, i;

	if (q_ptr->party == 0) {
		msg_print(adder, "\377yYou don't belong to a party.");
		return(FALSE);
	}

	Ind = name_lookup_loose(adder, name, FALSE, TRUE, FALSE);
	if (Ind <= 0) return(FALSE);

	if (adder == Ind) {
		msg_print(adder, "\377yYou cannot add yourself, you are already in the party.");
		return(FALSE);
	}

	/* Set pointer */
	p_ptr = Players[Ind];

#if 0 // It's really a prob that the owner can't add his own chars..so if0
	/* Make sure this isn't an impostor */
	if (!streq(parties[party_id].owner, q_ptr->name)) {
		/* Message */
		msg_print(adder, "\377yYou must be the owner to add someone.");

		/* Abort */
		return(FALSE);
	}
#endif

	/* Additional restriction for no-trading-mode-like characters */
#ifdef IRON_TEAM_LEVEL9
	if ((parties[party_id].mode & PA_IRONTEAM) && p_ptr->max_plv > 8) {
		msg_print(adder, "\377yIron Team owners can no longer add further players when they hit level 9.");
		return(FALSE);
	}
#endif
	if (parties[party_id].mode & PA_IRONTEAM_CLOSED) {
		msg_print(adder, "\377yThis Iron Team has been closed, not allowing adding any further members.");
		return(FALSE);
	}

	if (p_ptr->mode & MODE_SOLO) {
		msg_print(Ind, "\377yThat player is a soloist. Those cannot join parties.");
		return(FALSE);
	}

	/* Make sure this added person is neutral */
	if (p_ptr->party != 0) {
		/* Message */
		if (p_ptr->party != party_id) msg_print(adder, "\377yThat player is already in a party.");
		else msg_print(adder, "\377yThat player is already in your party.");

		/* Abort */
		return(FALSE);
	}

#ifdef IDDC_IRON_COOP
	if (in_irondeepdive(&p_ptr->wpos) || in_irondeepdive(&q_ptr->wpos)) {
		msg_print(adder, "\377yCharacters must be outside the Ironman Deep Dive Challenge to form a party.");
		return(FALSE);
	}
#endif

#ifdef IDDC_RESTRICTED_PARTYING
	/* Prevent someone inside IDDC adding someone outside or vice versa. */
	if ((in_irondeepdive(&p_ptr->wpos) || in_irondeepdive(&q_ptr->wpos)) &&
	    !(in_irondeepdive(&p_ptr->wpos) && in_irondeepdive(&q_ptr->wpos))) {
		msg_print(adder, "\377yCharacters outside of the IDDC cannot team up with characters inside.");
		return(FALSE);
	}
	/* Disallow adding someone to an IDDC party who already has another character in that party. */
	if (in_irondeepdive(&p_ptr->wpos)) {
		int *id_list, ids;
		struct account acc;
		bool success;

		int slot, p = q_ptr->party, max_depth = 0;
		hash_entry *ptr;
		char max_depth_name[NAME_LEN];


		/* check if he has a character in there already */
		success = GetAccount(&acc, p_ptr->accountname, NULL, FALSE, NULL, NULL);
		/* paranoia */
		if (!success) {
			/* uhh.. */
			msg_print(Ind, "Sorry, adding has failed.");
			return(FALSE);
		}
		success = FALSE;
		ids = player_id_list(&id_list, acc.id);
		for (i = 0; i < ids; i++) {
			if (lookup_player_party(id_list[i]) == party_id) {
				/* success */
				success = TRUE;
				break;
			}
		}
		if (ids) C_KILL(id_list, ids, int);

		/* success = fail! */
		if (success) {
			msg_print(adder, "\377yThat player already has another character in that same IDDC party.");
			return(FALSE);
		}

		/* We can only add someone who is at least on the same floor as the currently deepest party member, for anti-cheeze of item farming */
		for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
			ptr = hash_table[slot];
			while (ptr) {
				if (ptr->party == p && ABS(ptr->wpos.wz) > ABS(max_depth)) {
					max_depth = ptr->wpos.wz;
					strcpy(max_depth_name, ptr->name);
				}
				ptr = ptr->next;
			}
		}
		if (ABS(p_ptr->wpos.wz) < ABS(max_depth)) {
			msg_format(adder, "\377yPlayers to be added must at least match the depth of the party member currently farthest into the IDDC, which is %s at %dft!", max_depth_name, max_depth * 50);
			return(FALSE);
		}
 #endif
	}

	if (
#ifdef ALLOW_NR_CROSS_PARTIES
	    /* Note: Currently unnecessarily restricted: Actually q_ptr check is not needed, also not enabled for party_add_self().
	       Note: Above note might be wrong! total_winner check helps that we don't allow MODE_PVP to join! */
	    (!q_ptr->total_winner || !p_ptr->total_winner ||
	    !at_netherrealm(&q_ptr->wpos) || !at_netherrealm(&p_ptr->wpos)) &&
#endif
#ifdef IRONDEEPDIVE_ALLOW_INCOMPAT
 #ifndef IDDC_IRON_COOP
	    !in_irondeepdive(&p_ptr->wpos) &&
 #else
	    !on_irondeepdive(&p_ptr->wpos) &&
 #endif
#endif
#ifdef MODULE_ALLOW_INCOMPAT
	    !in_module(&p_ptr->wpos) &&
#endif
	/* Everlasting and other chars cannot be in the same party */
	    (compat_mode(parties[party_id].cmode, p_ptr->mode))) {
		msg_format(adder, "\377yYou cannot form a party with %s characters.", compat_mode(parties[party_id].cmode, p_ptr->mode));
		return(FALSE);
	}

	/* Only newly created characters can join an iron team */
	if ((parties[party_id].mode & PA_IRONTEAM) && (p_ptr->max_exp > 0 || p_ptr->max_plv > 1)) {
		msg_print(adder, "\377yOnly newly created characters without experience can join an iron team.");
		return(FALSE);
	}

	/* Ignoring a player will prevent getting added to a party by him */
	if (check_ignore(Ind, adder)) return(FALSE);

	/* Log - security */
	s_printf("PARTY_ADD: %s has been added to %s by %s.\n", p_ptr->name, parties[party_id].name, q_ptr->name);

	/* Tell the party about its new member */
	party_msg_format(party_id, "\374\377y%s has been added to party %s by %s.", p_ptr->name, parties[party_id].name, q_ptr->name);

	/* One more player in this party */
	parties[party_id].members++;

	/* Tell him about it */
	if (parties[party_id].mode & PA_IRONTEAM)
		msg_format(Ind, "\374\377yYou've been added to iron team '%s' by %s.", parties[party_id].name, q_ptr->name);
	else
		msg_format(Ind, "\374\377yYou've been added to party '%s' by %s.", parties[party_id].name, q_ptr->name);

	/* Set his party number */
	p_ptr->party = party_id;
	clockin(Ind, 2);
	p_ptr->iron_trade = parties[party_id].iron_trade;
	p_ptr->iron_turn = turn;

	/* Resend info */
	Send_party(Ind, FALSE, FALSE);

	/* Display the party note to him */
	for (i = 0; i < MAX_PARTYNOTES; i++) {
		if (!strcmp(party_note_target[i], parties[p_ptr->party].name)) {
			if (strcmp(party_note[i], ""))
				msg_format(Ind, "\374\377bParty Note: \377%c%s", COLOUR_CHAT_PARTY, p_ptr->censor_swearing ? party_note[i] : party_note_u[i]);
			break;
		}
	}

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Success */
	return(TRUE);
}
int party_add_self(int Ind, cptr party) {
	player_type *p_ptr = Players[Ind];
	int party_id = party_lookup(party), i, *id_list, ids;
#ifdef IRON_TEAM_LEVEL9
	int level_other = 0;
#endif
	struct account acc;
	bool success;
#if defined(IRONDEEPDIVE_ALLOW_INCOMPAT) || defined(MODULE_ALLOW_INCOMPAT)
	struct worldpos wpos_other = { -1, -1, -1};
#endif

	if (p_ptr->mode & MODE_SOLO) {
		msg_print(Ind, "\377yAs a soloist you cannot join parties.");
		return(FALSE);
	}

#ifdef IDDC_RESTRICTED_PARTYING
	if (in_irondeepdive(&p_ptr->wpos)) {
		msg_print(Ind, "You cannot add yourself to a party when inside the IDDC.");
		return(FALSE);
	}
#endif

	if (party_id == -1) {
		msg_print(Ind, "That party does not exist.");
		return(FALSE);
	}

	/* check if he has a character in there already, to be eligible to self-add */
	success = GetAccount(&acc, p_ptr->accountname, NULL, FALSE, NULL, NULL);
	/* paranoia */
	if (!success) {
		/* uhh.. */
		msg_print(Ind, "Sorry, self-adding has failed.");
		return(FALSE);
	}
	success = FALSE;
	ids = player_id_list(&id_list, acc.id);
	for (i = 0; i < ids; i++) {
		if (lookup_player_party(id_list[i]) == party_id) {
#if defined(IRONDEEPDIVE_ALLOW_INCOMPAT) || defined(MODULE_ALLOW_INCOMPAT)
			wpos_other = lookup_player_wpos(id_list[i]);
#endif
#ifdef IRON_TEAM_LEVEL9
			level_other = lookup_player_maxplv(id_list[i]);
#endif
			/* success */
			success = TRUE;
			break;
		}
	}
	if (ids) C_KILL(id_list, ids, int);

#ifdef IDDC_IRON_COOP
	if (in_irondeepdive(&p_ptr->wpos) || in_irondeepdive(&wpos_other)) {
		msg_print(Ind, "\377yCharacters must be outside the Ironman Deep Dive Challenge to form a party.");
		return(FALSE);
	}
#endif

	/* Everlasting and other chars cannot be in the same party */
	if (
#if 0 /* hm - anyway, if enabled, ensure that pvp-mode doesn't join! */
#ifdef ALLOW_NR_CROSS_PARTIES
	    (!p_ptr->total_winner || !at_netherrealm(&p_ptr->wpos)) &&
#endif
#endif
#ifdef IRONDEEPDIVE_ALLOW_INCOMPAT
 #ifndef IDDC_IRON_COOP
	    !(in_irondeepdive(&p_ptr->wpos) && in_irondeepdive(&wpos_other)) &&
 #else
	    !(on_irondeepdive(&p_ptr->wpos) && on_irondeepdive(&wpos_other)) &&
 #endif
#endif
#ifdef IRONDEEPDIVE_ALLOW_INCOMPAT
			!(in_module(&p_ptr->wpos) && in_module(&wpos_other)) &&
#endif
	    compat_mode(p_ptr->mode, parties[party_id].cmode)) {
		msg_format(Ind, "\377yYou cannot join %s parties.", compat_mode(p_ptr->mode, parties[party_id].cmode));
		return(FALSE);
	}

	/* Only newly created characters can join an iron team */
	if ((parties[party_id].mode & PA_IRONTEAM) && (p_ptr->max_exp > 0 || p_ptr->max_plv > 1)) {
		msg_print(Ind, "\377yOnly newly created characters without experience can join an iron team.");
		return(FALSE);
	}

	/* failure? */
	if (!success) {
		msg_print(Ind, "You do not have any character that is member of that party.");
		return(FALSE);
	}

	/* Additional restriction for no-trading-mode-like characters */
#ifdef IRON_TEAM_LEVEL9
	if ((parties[party_id].mode & PA_IRONTEAM) && level_other >= 9) {
		msg_print(Ind, "\377yIron Team owners can no longer add further players when they hit level 9.");
		return(FALSE);
	}
#endif
	if (parties[party_id].mode & PA_IRONTEAM_CLOSED) {
		msg_print(Ind, "\377yThis Iron Team has been closed, not allowing adding any further members.");
		return(FALSE);
	}

	/* Log - security */
	s_printf("PARTY_ADD_SELF: %s has been added to %s.\n", p_ptr->name, parties[party_id].name);

	/* Tell the party about its new member */
	party_msg_format(party_id, "\374\377y%s has been added to party %s.", p_ptr->name, parties[party_id].name);

	/* One more player in this party */
	parties[party_id].members++;

	/* Tell him about it */
	if (parties[party_id].mode & PA_IRONTEAM)
		msg_format(Ind, "\374\377yYou've been added to iron team '%s'.", parties[party_id].name);
	else
		msg_format(Ind, "\374\377yYou've been added to party '%s'.", parties[party_id].name);

	/* Set his party number */
	p_ptr->party = party_id;
	clockin(Ind, 2);
	p_ptr->iron_trade = parties[party_id].iron_trade;
	p_ptr->iron_turn = turn;

	/* Resend info */
	Send_party(Ind, FALSE, FALSE);

	/* Display the party note to him */
	for (i = 0; i < MAX_PARTYNOTES; i++) {
		if (!strcmp(party_note_target[i], parties[p_ptr->party].name)) {
			if (strcmp(party_note[i], ""))
				msg_format(Ind, "\374\377bParty Note: \377%c%s", COLOUR_CHAT_PARTY, p_ptr->censor_swearing ? party_note[i] : party_note_u[i]);
			break;
		}
	}

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Success */
	return(TRUE);
}

static void erase_guild_key(int id) {
	int i, this_o_idx, next_o_idx;
	monster_type *m_ptr;
	object_type *o_ptr, *q_ptr;
	char m_name[MNAME_LEN];

#if 0 /* account-based */
	int j;
	FILE *fp;
	char buf[1024];
	cptr cname;
	struct account acc;
#else /* just hash-table (character) based */
	int slot;
	hash_entry *ptr;
	player_type *p_ptr;
#endif


	/* objects on the floor/in monster inventories */
	for (i = 0; i < o_max; i++) {
		o_ptr = &o_list[i];
		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;
		/* skip non-guild keys */
		if (o_ptr->tval != TV_KEY || o_ptr->sval != SV_GUILD_KEY) continue;
		/* Skip wrong guild keys */
		if (o_ptr->pval != id) continue;

		/* in monster inventory */
		if (o_ptr->held_m_idx) {
			m_ptr = &m_list[o_ptr->held_m_idx];
			/* 1st object held is the key? */
			if (m_ptr->hold_o_idx == i) {
				m_ptr->hold_o_idx = o_ptr->next_o_idx;
				monster_desc(0, m_name, o_ptr->held_m_idx, 0);
				s_printf("GUILD_KEY_ERASE: monster inventory (%d, '%s', #1)\n", o_ptr->held_m_idx, m_name);
				delete_object_idx(i, TRUE, FALSE);
				return;
			} else {
				i = 1;
				q_ptr = &o_list[m_ptr->hold_o_idx];//compiler warning
				for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx) {
					if (this_o_idx == i) {
						q_ptr->next_o_idx = o_list[this_o_idx].next_o_idx;
						monster_desc(0, m_name, o_ptr->held_m_idx, 0);
						s_printf("GUILD_KEY_ERASE: monster inventory (%d, '%s', #%d)\n", o_ptr->held_m_idx, m_name, i);
						delete_object_idx(this_o_idx, TRUE, FALSE);
						return;
					}
					q_ptr = &o_list[this_o_idx];
					next_o_idx = q_ptr->next_o_idx;
					i++;
				}
			}
		}

		s_printf("GUILD_KEY_ERASE: floor\n");
		delete_object_idx(i, TRUE, FALSE);
		return;
	}

	/* Players online */
	for (this_o_idx = 1; this_o_idx <= NumPlayers; this_o_idx++) {
		p_ptr = Players[this_o_idx];
		/* scan his inventory */
		for (i = 0; i < INVEN_TOTAL; i++) {
			o_ptr = &p_ptr->inventory[i];
			if (!o_ptr->k_idx) continue;

			if (o_ptr->tval == TV_KEY && o_ptr->sval == SV_GUILD_KEY && o_ptr->pval == id) {
				s_printf("GUILD_KEY_ERASE: player '%s'\n", p_ptr->name);
				inven_item_increase(this_o_idx, i, -1);
				inven_item_describe(this_o_idx, i);
				inven_item_optimize(this_o_idx, i);
				return;
			}
		}
	}

#if 0 /* account-based */
	/* objects in player inventories */
	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb+");
	if (!fp) {
		s_printf("GUILD_KEY_ERASE: failed to open tomenet.acc\n");
		return;
	}
	while (fread(&acc, sizeof(struct account), 1, fp)) {
		int *id_list, chars;
		chars = player_id_list(&id_list, acc.id);
		for (i = 0; i < chars; i++) {
			cname = lookup_player_name(id_list[i]);
			...//not implemented
		}
		if (chars) C_KILL(id_list, chars, int);
	}
#else /* just hash-table (character) based */
	/* hack */
	NumPlayers++;
	MAKE(Players[NumPlayers], player_type);
	p_ptr = Players[NumPlayers];
	p_ptr->inventory = C_NEW(INVEN_TOTAL, object_type);
	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		ptr = hash_table[slot];
		while (ptr) {
			/* clear his data (especially inventory) */
			o_ptr = p_ptr->inventory;
			WIPE(p_ptr, player_type);
			p_ptr->inventory = o_ptr;
			p_ptr->Ind = NumPlayers;
			C_WIPE(p_ptr->inventory, INVEN_TOTAL, object_type);
			/* set his supposed name */
			strcpy(p_ptr->name, ptr->name);
			/* generate savefile name */
			process_player_name(NumPlayers, TRUE);
			/* try to load him! */
			if (!load_player(NumPlayers)) {
				/* bad fail */
				s_printf("GUILD_KEY_ERASE: load_player '%s' failed\n", p_ptr->name);
				/* unhack */
				C_FREE(p_ptr->inventory, INVEN_TOTAL, object_type);
				KILL(p_ptr, player_type);
				NumPlayers--;
				return;
			}
			/* scan his inventory */
			for (i = 0; i < INVEN_TOTAL; i++) {
				o_ptr = &p_ptr->inventory[i];
				if (!o_ptr->k_idx) continue;

				if (o_ptr->tval == TV_KEY && o_ptr->sval == SV_GUILD_KEY && o_ptr->pval == id) {
					s_printf("GUILD_KEY_ERASE: savegame '%s'\n", p_ptr->name);
					o_ptr->tval = o_ptr->sval = o_ptr->k_idx = 0;
					/* write savegame back */
					save_player(NumPlayers);
					/* unhack */
					C_FREE(p_ptr->inventory, INVEN_TOTAL, object_type);
					KILL(p_ptr, player_type);
					NumPlayers--;
					return;
				}
			}
			/* advance to next character */
			ptr = ptr->next;
		}
	}
	/* unhack */
	C_FREE(p_ptr->inventory, INVEN_TOTAL, object_type);
	KILL(p_ptr, player_type);
	NumPlayers--;
#endif

	/* hm, failed to locate the guild key. Maybe someone actually lost it. */
	s_printf("GUILD_KEY_ERASE: not found\n");
}

/*
 * Remove a guild. What a sad day.
 *
 * In style of del_party.
 */
void del_guild(int id) {
	char temp[160];
	int i;

	/* log! (to track auto-disbanding of leaderless guilds) */
	s_printf("DEL_GUILD: '%s' (%d)\n", guilds[id].name, id);
	snprintf(temp, 160, "\374\377yThe guild '\377%c%s\377y' no longer exists.", COLOUR_CHAT_GUILD, guilds[id].name);

	/* erase guild hall */
#if 0 /* not needed, since there can only be one guild hall */
	kill_houses(id, OT_GUILD);
#else
	if ((i = guilds[id].h_idx)) {
		struct dna_type *dna;

		i--; // (guild house index 0 means 'no house')
		dna = houses[i].dna;

		e_printf("(%s) %s (%d, %d) BACKUP-GUILD\n", showtime(), guilds[id].name, guilds[id].master, guilds[id].members); /* log to erasure.log file for compact overview */

		/* save real estate.. */
		if (!backup_one_estate(NULL, 0, 0, i, guilds[id].name))
			s_printf("(Estate backup: Guild hall failed!)\n");
		/* ..and rename estate file to indicate it's just a backup! */
		else ef_rename(guilds[id].name);

		dna->owner = 0L;
		dna->creator = 0L;
		dna->a_flags = ACF_NONE;
		kill_house_contents(i);

		/* and in case it was suspended due to leaderlessness,
		   so the next guild buying this house won't get a surprise.. */
		houses[i].flags &= ~HF_GUILD_SUS;
		fill_house(&houses[i], FILL_GUILD_SUS_UNDO, NULL);

		guilds[id].h_idx = 0;//obsolete?
	}
#endif

	/* delete pending guild notes */
	for (i = 0; i < MAX_GUILDNOTES; i++) {
		if (!strcmp(guild_note_target[i], guilds[id].name)) {
			strcpy(guild_note_target[i], "");
			strcpy(guild_note[i], "");
			break;
		}
	}

	/* erase guild bbs */
	for (i = 0; i < BBS_LINES; i++) strcpy(gbbs_line[id][i], "");

	/* Clear the basic info */
	guilds[id].members = 0; /* it should be zero anyway */
	strcpy(guilds[id].name, "");
	for (i = 0; i < 5; i++) guilds[id].adder[i][0] = '\0'; /* they should be cleared anyway */
	guilds[id].flags = GFLG_NONE;
	guilds[id].minlev = 0;

	/* Find and destroy the guild key! */
	erase_guild_key(id);

	/* Tell everyone */
	msg_broadcast(0, temp);
}
void guild_timeout(int id) {
	int i;
	player_type *p_ptr;

	for (i = 1; i <= NumPlayers; i++) {
		p_ptr = Players[i];
		if (p_ptr->guild && p_ptr->guild != id) continue;

		p_ptr->guild = 0;
		clockin(i, 3);

		if (p_ptr->conn == NOT_CONNECTED) continue;

		/* Re-check house permissions, to display doors in correct colour */
		if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

		msg_print(i, "\377oYour guild has been deleted from being leaderless for too long.");
		Send_guild(i, TRUE, FALSE);
	}

	s_printf("GUILD_TIMEOUT: '%s' (%d)\n", guilds[id].name, id);
	guilds[id].members = 0;
	del_guild(id);
}

/*
 * Delete a party. Was in party remove.
 *
 * Design improvement
 */
void del_party(int id) {
	int i, j;
	bool sent = FALSE;
	player_type *p_ptr;

	s_printf("DEL_PARTY: '%s' (%d)\n", parties[id].name, id);

	/* Remove the party altogether */
	kill_houses(id, OT_PARTY);

	/* delete pending party notes */
	for (i = 0; i < MAX_PARTYNOTES; i++) {
		if (!strcmp(party_note_target[i], parties[id].name)) {
			strcpy(party_note_target[i], "");
			strcpy(party_note[i], "");
			break;
		}
	}

	/* erase party bbs */
	for (i = 0; i < BBS_LINES; i++) strcpy(pbbs_line[id][i], "");

	/* Set the number of people in this party to zero */
	parties[id].members = 0;

	/* Remove everyone else */
	for (i = 1; i <= NumPlayers; i++) {
		/* Check if they are in here */
		if (player_in_party(id, i)) {
			if (!sent) { /* no need to spam this more than once */
				Send_party(i, FALSE, TRUE);
				sent = TRUE;
			}

			p_ptr = Players[i];

			/* Set his party number back to "neutral" */
			p_ptr->party = 0;
			clockin(i, 2);
			p_ptr->iron_trade = 0;
			for (j = 0; j < INVEN_TOTAL; j++)
				p_ptr->inventory[j].iron_trade = 0;
			/* and for client-side visual iddc-tradable marker, redraw: */
			if (in_irondeepdive(&p_ptr->wpos)) p_ptr->window |= PW_INVEN | PW_EQUIP;

			/* Re-check house permissions, to display doors in correct colour */
			if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

			if (parties[id].mode & PA_IRONTEAM)
				msg_print(i, "\374\377yYour iron team has been disbanded.");
			else
				msg_print(i, "\374\377yYour party has been disbanded.");
		}
	}

	/* Set the creation time to "disbanded time" */
	parties[id].created = turn;

	/* Empty the name */
	strcpy(parties[id].name, "");
}

/*
 * Remove player from a guild
 */
int guild_remove(int remover, cptr name) {
	player_type *p_ptr;
	player_type *q_ptr = Players[remover];
	int guild_id = q_ptr->guild, Ind = 0;

	if (!guild_id) {
		if (!is_admin(q_ptr)) {
			msg_print(remover, "\377yYou are not in a guild");
			return(FALSE);
		}
	}

	/* Make sure this is the owner */
	if (guilds[guild_id].master != q_ptr->id && !is_admin(q_ptr)) {
		/* Message */
		msg_print(remover, "\377yYou must be the owner to delete someone.");

		/* Abort */
		return(FALSE);
	}

	Ind = name_lookup_loose(remover, name, FALSE, TRUE, FALSE);
	if (Ind <= 0) return(FALSE);

	if (Ind == remover) {	/* remove oneself from guild - leave */
		guild_leave(remover, TRUE);
		return(TRUE);
	}

	p_ptr = Players[Ind];

	if (!guild_id && is_admin(q_ptr)) guild_id = p_ptr->guild;

	/* Make sure they were in the guild to begin with */
	if (guild_id != p_ptr->guild) {
		/* Message */
		msg_print(remover, "\377yYou can only delete guild members.");

		/* Abort */
		return(FALSE);
	}

	/* Keep the guild, just lose a member */
	else {
		/* Lose a member */
		guilds[guild_id].members--;

		/* Messages */
		msg_print(Ind, "\374\377yYou have been removed from the guild.");
		if (!is_admin(p_ptr)) guild_msg_format(guild_id, "\374\377y%s has been removed from the guild.", p_ptr->name);

		/* Last member deleted? */
		if (guilds[guild_id].members == 0) {
			Send_guild(Ind, TRUE, TRUE);
			p_ptr->guild = 0;
			clockin(Ind, 3);
			s_printf("Guild '%s' - last member removed -> deleting.\n", guilds[guild_id].name);
			del_guild(guild_id);

			/* Re-check house permissions, to display doors in correct colour */
			if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;
		} else {
			Send_guild(Ind, TRUE, FALSE);
			p_ptr->guild = 0;
			clockin(Ind, 3);

			/* Re-check house permissions, to display doors in correct colour */
			if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;
		}
	}

	return(TRUE);
}

/*
 * Remove a person from a party.
 *
 * Removing the party owner destroys the party. - Not always anymore:
 * On RPG server, removing the owner will just promote someone else to owner,
 * provided (s)he's logged on. Better for RPG diving partys - C. Blue
 */
int party_remove(int remover, cptr name) {
	player_type *p_ptr;
	player_type *q_ptr = Players[remover];
	int party_id = q_ptr->party, Ind = 0;
	int i, j;
	bool admin_unown = FALSE;

	/* Make sure this is the owner */
	if (!streq(parties[party_id].owner, q_ptr->name)) {
		if (!is_admin(q_ptr)) {
			msg_print(remover, "\377yYou must be the owner to delete someone.");
			/* Abort */
			return(FALSE);
		}
		/* Admin removes the party owner? */
		if (streq(parties[party_id].owner, name)) admin_unown = TRUE;
	}

	Ind = name_lookup_loose(remover, name, FALSE, TRUE, FALSE);
	if (Ind <= 0) return(FALSE);
	p_ptr = Players[Ind];

	/* Admin can delete players from their parties without being member of their party. */
	if ((!party_id || !streq(parties[party_id].owner, q_ptr->name)) && is_admin(q_ptr))
		party_id = p_ptr->party;

	/* Make sure they were in the party to begin with */
	if (!player_in_party(party_id, Ind)) {
		msg_print(remover, "\377yYou can only delete party members.");
		/* Abort */
		return(FALSE);
	}

	/* See if this is the owner we're deleting */
	if (remover == Ind || admin_unown) {
#ifndef RPG_SERVER
		int where = 0;

		if (admin_unown || (where = (in_irondeepdive(&p_ptr->wpos) ? 1 : 0) + (in_hallsofmandos(&p_ptr->wpos) ? 2 : 0) + (in_sector00(&p_ptr->wpos) ? 3 : 0) + (in_module(&p_ptr->wpos) ? 4 : 0))) {
#endif
			player_type *n_ptr;

			/* Keep the party, just lose a member but auto-transfer ownership on the next player who is around */
			for (i = 1; i <= NumPlayers; i++) {
				if (i == Ind) continue;
				n_ptr = Players[i];
				if (is_admin(n_ptr)) continue;
				if (n_ptr->party != q_ptr->party) continue;
#ifndef RPG_SERVER
				/* Player must be in the same dungeon to take over ownership */
				if (!admin_unown) switch (where) {
				case 1:
					if (!in_irondeepdive(&n_ptr->wpos)) continue;
					break;
				case 2:
					if (in_hallsofmandos(&n_ptr->wpos)) continue;
					break;
				case 3:
					if (in_sector00(&n_ptr->wpos)) continue;
					break;
				case 4:
					if (in_module(&n_ptr->wpos)) continue;
					break;
				}
#endif

				/* Found someone, transfer leadership to this player */
				strcpy(parties[party_id].owner, Players[i]->name);
				msg_print(i, "\374\377yYou are now the party owner!");

				/* Tell everybody else in the party about leadership transfer */
				for (j = 1; j <= NumPlayers; j++)
					if (Players[j]->party == Players[i]->party && j != i)
						msg_format(j, "\374\377y%s is now the party owner.", Players[i]->name);
				break;
			}
			/* no other player online who is in the same party and could overtake leadership? Then erase party! */
			if (i > NumPlayers) {
				s_printf("Party owner '%s' removed '%s', no replacement -> deleting party.\n", q_ptr->name, parties[party_id].name);
				del_party(party_id);
				return(TRUE);
			}
#ifndef RPG_SERVER
		} else {
			s_printf("Party owner '%s' removed '%s' -> deleting party.\n", q_ptr->name, parties[party_id].name);
			del_party(party_id);
			return(TRUE);
		}
#endif
	}

	/* Lose a member */
	parties[party_id].members--;

	/* Resend info */
	Send_party(Ind, TRUE, FALSE);

	/* Set his party number back to "neutral" */
	p_ptr->party = 0;
	clockin(Ind, 2);
	p_ptr->iron_trade = 0;
	for (i = 0; i < INVEN_TOTAL; i++)
		p_ptr->inventory[i].iron_trade = 0;
	/* and for client-side visual iddc-tradable marker, redraw: */
	if (in_irondeepdive(&p_ptr->wpos)) p_ptr->window |= PW_INVEN | PW_EQUIP;

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Messages */
	if (parties[party_id].mode & PA_IRONTEAM) {
		msg_print(Ind, "\374\377yYou have been removed from your iron team.");
		if (!is_admin(p_ptr)) party_msg_format(party_id, "\374\377y%s has been removed from the iron team.", p_ptr->name);
	} else {
		msg_print(Ind, "\374\377yYou have been removed from your party.");
		if (!is_admin(p_ptr)) party_msg_format(party_id, "\374\377y%s has been removed from the party.", p_ptr->name);
	}

	return(TRUE);
}
void party_close(int Ind) {
	player_type *p_ptr = Players[Ind];
	int party_id = p_ptr->party;

	/* Catch impossibilities (shouldn't occur) */
	if (!party_id) return; //not in a party
	if (!(parties[party_id].mode & PA_IRONTEAM) || (parties[party_id].mode & PA_IRONTEAM_CLOSED)) return; //not an iron team or already closed

	/* Make sure this is the owner */
	if (!streq(parties[party_id].owner, p_ptr->name) && !is_admin(p_ptr)) {
		msg_print(Ind, "\377yYou must be the owner to close an iron team.");
		return;
	}

	msg_print(Ind, "\377DYour iron team is now closed off to anyone who is not already a member!");
	s_printf("IRON_TEAM_CLOSED: %s (%d/%d) closed '%s' (%d members)\n", p_ptr->name, p_ptr->lev, p_ptr->max_plv, parties[party_id].name, parties[party_id].members);
	parties[party_id].mode |= PA_IRONTEAM_CLOSED;
	/* Update info for client */
	Send_party(Ind, FALSE, FALSE);
}

void guild_leave(int Ind, bool voluntarily) {
	player_type *p_ptr = Players[Ind];
	int guild_id = p_ptr->guild, i;

	/* Make sure he belongs to a guild */
	if (!guild_id) {
		msg_print(Ind, "\377yYou don't belong to a guild.");
		return;
	}

#ifdef GUILD_ADDERS_LIST
	if ((p_ptr->guild_flags & PGF_ADDER))
		for (i = 0; i < 5; i++) if (streq(guilds[p_ptr->guild].adder[i], p_ptr->name)) {
			guilds[p_ptr->guild].adder[i][0] = '\0';
			break;
		}
#endif

	/* Lose a member */
	guilds[guild_id].members--;

	/* Inform people */
	if (voluntarily) {
		msg_print(Ind, "\374\377yYou have left your guild.");
		p_ptr->guild = 0; //hack: prevent duplicate message
		if (!is_admin(p_ptr)) guild_msg_format(guild_id, "\374\377y%s has left the guild.", p_ptr->name);
		p_ptr->guild = guild_id; //unhack
	} else {
		msg_print(Ind, "\374\377yYou have been removed from your guild.");
		p_ptr->guild = 0; //hack: prevent duplicate message
		if (!is_admin(p_ptr)) guild_msg_format(guild_id, "\374\377y%s has been removed from the guild.", p_ptr->name);
		p_ptr->guild = guild_id; //unhack
	}

	/* If he's the guildmaster, set master to zero */
	if (p_ptr->id == guilds[guild_id].master) {
		guild_msg(guild_id, "\374\377yThe guild is now leaderless!");
		guilds[guild_id].master = 0;

		/* set guild hall to 'suspended' */
		if ((i = guilds[guild_id].h_idx)) {
			houses[i - 1].flags |= HF_GUILD_SUS;
			fill_house(&houses[i - 1], FILL_GUILD_SUS, NULL);
		}
	}

	/* Resend info */
	Send_guild(Ind, TRUE, FALSE);

	/* Set him back to "neutral" */
	p_ptr->guild = 0;
	clockin(Ind, 3);

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Last member deleted? */
	if (guilds[guild_id].members == 0) {
		s_printf("Guild '%s' - last member left -> deleting.\n", guilds[guild_id].name);
		del_guild(guild_id);
	}
}

/*
 * A player wants to leave a party.
 */
void party_leave(int Ind, bool voluntarily) {
	player_type *p_ptr = Players[Ind];
	int party_id = p_ptr->party, i;

	/* Make sure he belongs to a party */
	if (!party_id) {
		msg_print(Ind, "\377yYou don't belong to a party.");
		return;
	}

	/* If he's the owner, use the other function */
	if (streq(p_ptr->name, parties[party_id].owner)) {
		/* Call party_remove */
		party_remove(Ind, p_ptr->name);
		return;
	}

	/* Lose a member */
	parties[party_id].members--;

	/* Resend info */
	Send_party(Ind, TRUE, FALSE);

	/* Set him back to "neutral" */
	p_ptr->party = 0;
	clockin(Ind, 2);
	p_ptr->iron_trade = 0;
	for (i = 0; i < INVEN_TOTAL; i++)
		p_ptr->inventory[i].iron_trade = 0;
	/* and for client-side visual iddc-tradable marker, redraw: */
	if (in_irondeepdive(&p_ptr->wpos)) p_ptr->window |= PW_INVEN | PW_EQUIP;

	/* Re-check house permissions, to display doors in correct colour */
	if (!p_ptr->wpos.wz) p_ptr->redraw |= PR_MAP;

	/* Inform people */
	if (voluntarily) {
		if (parties[party_id].mode & PA_IRONTEAM) {
			msg_print(Ind, "\374\377yYou have left your iron team.");
			if (!is_admin(p_ptr)) party_msg_format(party_id, "\374\377y%s has left the iron team.", p_ptr->name);
		} else {
			msg_print(Ind, "\374\377yYou have left your party.");
			if (!is_admin(p_ptr)) party_msg_format(party_id, "\374\377y%s has left the party.", p_ptr->name);
		}
	} else {
		if (parties[party_id].mode & PA_IRONTEAM) {
			msg_print(Ind, "\374\377yYou have been removed from your iron team.");
			if (!is_admin(p_ptr)) party_msg_format(party_id, "\374\377y%s has been removed from the iron team.", p_ptr->name);
		} else {
			msg_print(Ind, "\374\377yYou have been removed from your party.");
			if (!is_admin(p_ptr)) party_msg_format(party_id, "\374\377y%s has been removed from the party.", p_ptr->name);
		}
	}
}

bool guild_rename(int Ind, char *new_name) {
	player_type *p_ptr = Players[Ind];
	int i, gid = p_ptr->guild;

	if (!gid) {
		msg_print(Ind, "\377yYou are not in a guild");
		return(FALSE);
	}

	/* Leaderless guilds do not allow addition of new members */
	if (p_ptr->id != guilds[gid].master) {
		msg_print(Ind, "\377yOnly the guild master can rename a guild.");
		return(FALSE);
	}

	if (!strcmp(guilds[gid].name, new_name)) {
		msg_print(Ind, "\377yNew guild name must be different from its current name.");
		return(FALSE);
	}

	if (!guild_name_legal(Ind, new_name)) return(FALSE);

	/* This could probably be improved. */
	if (p_ptr->au < GUILD_PRICE) {
		if (GUILD_PRICE >= 1000000 && ((int)(GUILD_PRICE / 1000000)) * 1000000 == GUILD_PRICE)
			msg_format(Ind, "\377yYou need %d,000,000 gold pieces to rename a guild.", GUILD_PRICE / 1000000);
		else if (GUILD_PRICE >= 1000 && ((int)(GUILD_PRICE / 1000)) * 1000 == GUILD_PRICE)
			msg_format(Ind, "\377yYou need %d,000 gold pieces to rename a guild.", GUILD_PRICE / 1000);
		else
			msg_format(Ind, "\377yYou need %d gold pieces to rename a guild.", GUILD_PRICE);
		return(FALSE);
	}


	p_ptr->au -= GUILD_PRICE;
	p_ptr->redraw |= PR_GOLD;
#ifdef USE_SOUND_2010
	sound(Ind, "drop_gold", NULL, SFX_TYPE_MISC, FALSE);
	sound(Ind, "store_paperwork", NULL, SFX_TYPE_MISC, FALSE);
#endif

	for (i = 0; i < MAX_GUILDNOTES; i++)
		if (!strcmp(guild_note_target[i], guilds[gid].name))
			strcpy(guild_note_target[i], new_name);

	msg_broadcast_format(0, "\374\377yThe guild '\377%c%s\377y' changed name to '\377%c%s\377y'.",
	    COLOUR_CHAT_GUILD, guilds[gid].name, COLOUR_CHAT_GUILD, new_name);

	l_printf("%s \\{yThe guild '%s' changed name to '%s'\n",
	    showdate(), guilds[gid].name, new_name);

	strcpy(guilds[gid].name, new_name);
	return(TRUE);
}

/*
 * Send a message to everyone in a party.
 */
void guild_msg(int guild_id, cptr msg) {
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;

		/* Check this guy */
		if (guild_id == Players[i]->guild)
			msg_print(i, msg);
	}
}
void guild_msg_ignoring(int sender, int guild_id, cptr msg) {
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, sender)) continue;
		if ((Players[sender]->mutedchat == 3 || Players[i]->mutedchat == 3) && i != sender) continue;

		/* Check this guy */
		if (guild_id == Players[i]->guild)
			msg_print(i, msg);
	}
}
void guild_msg_ignoring2(int sender, int guild_id, cptr msg, cptr msg_u) {
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, sender)) continue;
		if ((Players[sender]->mutedchat == 3 || Players[i]->mutedchat == 3) && i != sender) continue;

		/* Check this guy */
		if (guild_id == Players[i]->guild)
			msg_print(i, Players[i]->censor_swearing ? msg : msg_u);
	}
}
/*
 * Send a formatted message to a guild.
 */
void guild_msg_format(int guild_id, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	guild_msg(guild_id, buf);
}
void guild_msg_format_ignoring(int sender, int guild_id, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	guild_msg_ignoring(sender, guild_id, buf);
}

/*
 * Send a message to everyone in a party.
 */
void party_msg(int party_id, cptr msg) {
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED)continue;

		/* Check this guy */
		if (player_in_party(party_id, i))
			msg_print(i, msg);
	}
}

/*
 * Send a formatted message to a party.
 */
void party_msg_format(int party_id, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	party_msg(party_id, buf);
}

/*
 * Send a message to everyone in a party, considering ignorance.
 */
void party_msg_ignoring(int sender, int party_id, cptr msg) {
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, sender)) continue;
		if ((Players[sender]->mutedchat == 3 || Players[i]->mutedchat == 3) && i != sender) continue;

		/* Check this guy */
		if (player_in_party(party_id, i)) msg_print(i, msg);
	}
}
void party_msg_ignoring2(int sender, int party_id, cptr msg, cptr msg_u) {
	int i;

	/* Check for this guy */
	for (i = 1; i <= NumPlayers; i++) {
		if (Players[i]->conn == NOT_CONNECTED) continue;
		if (check_ignore(i, sender)) continue;
		if ((Players[sender]->mutedchat == 3 || Players[i]->mutedchat == 3) && i != sender) continue;

		/* Check this guy */
		if (player_in_party(party_id, i))
			msg_print(i, Players[i]->censor_swearing ? msg : msg_u);
	}
}

/*
 * Send a formatted message to a party.
 */
void party_msg_format_ignoring(int sender, int party_id, cptr fmt, ...) {
	va_list vp;
	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	party_msg_ignoring(sender, party_id, buf);
}
/*
 * Split some experience among party members.
 *
 * This should ONLY be used while killing monsters.  The amount
 * should be the monster base experience times the monster level.
 *
 * This algorithm may need some work....  However, it does have some nifty
 * features, such as:
 *
 * 1) A party with just one member functions identically as before.
 *
 * 2) A party with two equally-levelled members functions such that each
 * member gets half as much experience as he would have by killing the monster
 * by himself.
 *
 * 3) Higher-leveled members of a party get higher percentages of the
 * experience.
 */

 /* The XP distribution was too unfair for low level characters,
    it made partying a real pain. I am changing it so that if the players
    have a difference in level of less than 5 than there is no difference
    in XP distribution.

    I am also changing it so it divides by each players level, AFTER
    it has been given to them.

    UPDATE: it appears that it may be giving too much XP to the low lvl chars,
    but I have been too lazy to change it... however, this doesnt appear to be being
    abused much, and the new system is regardless much nicer than the old one.

    -APD-
    */

/* This helper function checks if two players are allowed to share experience,
   based on a) level difference and b) royal status.
   Note a quirk here:
   Winners and non-winners don't share exp, but winners and once-winners do. */
static bool players_in_level(int Ind, int Ind2) {
	player_type *p_ptr = Players[Ind], *p2_ptr = Players[Ind2];

	/* Check for max allowed level difference.
	   (p_ptr->lev would be more logical but harsh) */
	if (p_ptr->total_winner && p2_ptr->total_winner) {
#ifdef KING_PARTY_FREE_THRESHOLD
		if (KING_PARTY_FREE_THRESHOLD && p_ptr->max_lev >= KING_PARTY_FREE_THRESHOLD && p2_ptr->max_lev >= KING_PARTY_FREE_THRESHOLD) return(TRUE);
#endif
		if (ABS(p_ptr->max_lev - p2_ptr->max_lev) > MAX_KING_PARTY_LEVEL_DIFF) return(FALSE);
		return(TRUE);
	}
	if (ABS(p_ptr->max_lev - p2_ptr->max_lev) > MAX_PARTY_LEVEL_DIFF) return(FALSE);

	/* Winners and non-winners don't share experience!
	   This is actually important because winners get special features
	   such as super heavy armour and royal stances which are aimed at
	   nether realm, and are not meant to influence pre-king gameplay. */
	if ((p_ptr->total_winner && !(p2_ptr->total_winner || p2_ptr->once_winner)) ||
	    (p2_ptr->total_winner && !(p_ptr->total_winner || p_ptr->once_winner)))
		return(FALSE);

	return(TRUE);
}

/* Note: 'amount' is actually multiplied by 100 for extra decimal digits if we're very low level (Training Tower exp'ing).
   The same is done to 'base_amount' so it's *100 too. */
#define PERFORM_IRON_TEAM_CHECKS
void party_gain_exp(int Ind, int party_id, s64b amount, s64b base_amount, int henc, int henc_top, int *apply_exp_Ind) {
	player_type *p_ptr = Players[Ind], *q_ptr;
	int PInd[NumPlayers], pi = 0, Ind2; /* local working copy of player indices, for efficiency hopefully */
	int i, eff_henc, hlev = 0, htop = 0, aeI_idx = -1;
#ifdef ANTI_MAXPLV_EXPLOIT
 #ifdef ANTI_MAXPLV_EXPLOIT_SOFTEXP
	int diff;
 #endif
#endif
	struct worldpos *wpos = &p_ptr->wpos;
	s64b new_exp, new_exp_frac, average_lev = 0, num_members = 0, new_amount, new_boosted_amount;
	s64b modified_level;
	int dlev = getlevel(wpos);
	bool not_in_iddc = !in_irondeepdive(wpos);
#ifdef PERFORM_IRON_TEAM_CHECKS
	bool iron = ((parties[party_id].mode & PA_IRONTEAM) != 0);
	int iron_team_members_here = 0;//temporarily here
#endif

#ifdef ALLOW_NR_CROSS_PARTIES
	/* quick and dirty anti-cheeze (for if NR surface already allows partying up) */
	if (on_netherrealm(wpos)) return;
#endif
#ifdef IDDC_IRON_COOP
	/* quick and dirty anti-cheeze (for if IDDC surface already allows partying up) */
	if (on_irondeepdive(wpos)) return;
#endif

	/* Calculate the average level and iron team presence */
	for (i = 1; i <= NumPlayers; i++) {
		q_ptr = Players[i];

		if (q_ptr->conn == NOT_CONNECTED) continue;

		/* Check for his existence in the party */
		if (!(player_in_party(party_id, i) && (inarea(&q_ptr->wpos, wpos)))) continue;

		/* Create map for efficiency */
		PInd[pi] = i;
		pi++;

#ifdef PERFORM_IRON_TEAM_CHECKS
		/* will be moved to gain_exp() if decrease of party.experience is implemented, nasty though.
		until then, iron teams will just have to be re-formed as normal parties if a member falls
		behind too much in terms of exp and hence blocks the whole team from gaining exp. */

		/* Iron Teams only get exp if the whole team is on the same floor! */
		if (iron &&
		    /* note: this line means that iron teams must not add
		       admins, or the members won't gain exp anymore */
		    !is_admin(q_ptr))
			iron_team_members_here++;
#endif

		/* For exp-splitting: Increase the "divisor" */
		average_lev += q_ptr->lev;
		num_members++;

		/* Floor depth vs player level for efficient exp'ing always uses the highest-level team mate. */
		if (q_ptr->lev > hlev) hlev = q_ptr->lev;

		/* For keeping track of max-plv-exploits in case henc_strictness is actually disabled. */
		if ((q_ptr->max_lev + q_ptr->max_plv) / 2 > htop) htop = (q_ptr->max_lev + q_ptr->max_plv) / 2;
	}

#ifdef PERFORM_IRON_TEAM_CHECKS
	/* Iron team calc: nobody in an iron team can gain exp if not all members are present and in the same place! */
	if (iron && iron_team_members_here != parties[party_id].members) return;
#endif

	/* Exp share calc: */
	average_lev /= num_members;

	/* Replace top max_plv value taken from any party member on the floor
	   simply by the henc_top value, if we use the 'henc' feature: */
	if (cfg.henc_strictness) htop = henc_top;
	/* paranoia (m_ptr->henc shouldn't actually get set if
	   henc_strictness is disabled, but better safe than sorry: */
	else henc = 0;

	/* Now, distribute the experience */
	for (i = 0; i < pi; i++) {
		Ind2 = PInd[i];
		q_ptr = Players[Ind2];

		/* players who exceed the restrictions to share exp with us don't get any,
		   but still take share toll by increasing the divisor in the loop above. */
		if (q_ptr->ghost || !players_in_level(Ind, Ind2)) continue;

		/* Calculate this guy's experience gain */
		new_amount = amount;

		/* HEnc-checks -
		   in case henc_strictness is actually disabled, we abuse 'henc' here
		   by calculating with max_plv values instead, using them as pseudo-henc values. */
		if (!q_ptr->total_winner) {
			eff_henc = henc; //(0 if henc_strictness is disabled)

#ifdef ANTI_MAXPLV_EXPLOIT
			/* Don't allow cheap support from super-high level characters whose max_plv
			   is considerably higher than their normal level, resulting in them being
			   way more powerful than usual (because they have more skill points allocated). */

			/* Two cases:
			   1) We're not the killer and the killer has too high max_plv/supp_top (= henc_top),
			   2) We're the killer but we have too high supp_top
			      (and optionally the monster maybe also has too high henc_top from the killer,
			      but this can't be implemented easily because the henc_top might be ourself! oops!).
			   However, the problem is that we might actually be a 10(40) char and someone else too;
			   now we cannot distinguish anymore, and we should definitely get full exp when this
			   team of two kills a monster, right?
			   So there is probably no easy way except for disabling ANTI_MAXPLV_EXPLOIT completely :/. */

			/* ..instead of the above stuff, here a new, simpler attempt:
			   If there's anyone whose max_plv is > ours and exceeds the allowed level diff,
			   henc penalty (AMES-LEV) or exp penalty (AMES-EXP) applies to us. */
 #ifdef ANTI_MAXPLV_EXPLOIT_SOFTLEV
			 /* avg lev compariso, weighing a bit less than max_lev comparison */
			if (htop - (q_ptr->max_lev + q_ptr->max_plv) / 2 > ((MAX_PARTY_LEVEL_DIFF + 1) * 3) / 2) {
				if (!p_ptr->warning_partyexp) {
					msg_print(Ind, "\377yYou didn't gain experience points for a monster kill right now because the");
					msg_print(Ind, "\377y player level difference within your party is too big! (See ~ g c par).");
					s_printf("warning_partyexp: %s\n", p_ptr->name);
					p_ptr->warning_partyexp = 1;
				}
				continue; /* zonk, bam */
			}
 #else
  #ifdef ANTI_MAXPLV_EXPLOIT_SOFTEXP
			diff = htop - (q_ptr->max_lev + q_ptr->max_plv) / 2 - (MAX_PARTY_LEVEL_DIFF + 1);
			if (diff > 0) new_amount = (new_amount * 4) / (4 + diff);
  #else
			 /* avg lev comparison in the same way as max_lev comparison */
			if (htop - (q_ptr->max_lev + q_ptr->max_plv) / 2 > MAX_PARTY_LEVEL_DIFF + 1) {
				if (!p_ptr->warning_partyexp) {
					msg_print(Ind, "\377yYou didn't gain experience points for a monster kill right now because the");
					msg_print(Ind, "\377y player level difference within your party is too big! (See ~ g c par).");
					s_printf("warning_partyexp: %s\n", p_ptr->name);
					p_ptr->warning_partyexp = 1;
				}
				continue; /* zonk, bam */
			}
  #endif
 #endif
#endif
			/* Don't allow cheap support from super-high level characters */
			if (eff_henc - q_ptr->max_lev > MAX_PARTY_LEVEL_DIFF + 1) continue; /* zonk */
		}

		/* dungeon floor specific reduction if player dives too shallow */
		if (not_in_iddc && (!in_hallsofmandos(wpos) || q_ptr->lev >= 50)) new_amount = det_exp_level(new_amount, hlev, dlev);

		/* Never get too much exp off a monster
		   due to high level difference,
		   make exception for low exp boosts like "holy jackal" */
		if ((new_amount > base_amount * 4 * q_ptr->lev) && (new_amount > 200 * q_ptr->lev))
			new_amount = base_amount * 4 * q_ptr->lev; /* new_amount gets divided by q_ptr->lev later - mikaelh */

		/* Especially high party xp boost in IDDC, or people might mostly prefer to solo to 2k */
		if (!not_in_iddc)
			new_boosted_amount = (new_amount * (IDDC_PARTY_XP_BOOST + 1)) / (num_members + IDDC_PARTY_XP_BOOST);
		/* Some bonus is applied to encourage partying - Jir - */
		else
			new_boosted_amount = (new_amount * (PARTY_XP_BOOST + 1)) / (num_members + PARTY_XP_BOOST);

		/* get less or more exp depending on if we're above or below the party average */
		if (q_ptr->lev < average_lev) { // below average
			if ((average_lev - q_ptr->lev) > 2)
				modified_level = q_ptr->lev + 2;
			else	modified_level = average_lev;
		} else {
			if ((q_ptr->lev - average_lev) > 2)
				modified_level = q_ptr->lev - 2;
			else	modified_level = average_lev;
		}
		new_amount = (new_boosted_amount * modified_level) / average_lev;

		/* Calculate integer and fractional exp gain numbers */
		new_exp = new_amount / q_ptr->lev / 100;
		new_exp_frac = ((new_amount - new_exp * q_ptr->lev * 100)
		    * 100L) / q_ptr->lev + q_ptr->exp_frac;

		/* Visual hack so we don't show '0 XP' when it's actually just a fraction of an XP point... -_- */
		q_ptr->gain_exp_frac = (new_exp == 0 && new_amount != 0 && new_exp_frac - q_ptr->exp_frac != 0);

		/* Keep track of experience */
		if (new_exp_frac >= 10000L) {
			new_exp++;
			q_ptr->exp_frac = new_exp_frac - 10000L;
		} else {
			q_ptr->exp_frac = new_exp_frac;
			q_ptr->redraw |= PR_EXP; //EXP_BAR_FINESCALE
		}

		/* Gain experience */
		if (new_exp) {
			apply_exp_Ind[++aeI_idx] = Ind2;
			gain_exp_onhold(Ind2, new_exp); //for adding the amount to the monster death message
		} else if (!q_ptr->warning_fracexp && base_amount) {
			msg_print(Ind2, "\374\377ySome monsters give less than 1 experience point, but you still gain a bit!");
			s_printf("warning_fracexp: %s\n", q_ptr->name);
			q_ptr->warning_fracexp = 1;
		}
	}
}

/*
 * Add a player to another player's list of hostilities.
 */
bool add_hostility(int Ind, cptr name, bool initiator, bool admin_forced) {
	player_type *p_ptr = Players[Ind], *q_ptr;
	hostile_type *h_ptr;
	int i;
	bool bb = FALSE;

#if 0 /* too risky? (exploitable) */
	i = name_lookup_loose(Ind, name, TRUE, TRUE, FALSE);
#else
	i = name_lookup(Ind, name, TRUE, TRUE, FALSE);
#endif

	if (!i) {
s_printf("ADD_HOSTILITY: not found.\n");
		return(FALSE);
	}

	/* Check for sillyness */
	if (i == Ind) {
		/* Message */
		msg_print(Ind, "\377yYou cannot be hostile toward yourself.");
		return(FALSE);
	}

	/* If it's a blood bond, players may fight in safe zones and with party members np */
	if (i > 0) bb = check_blood_bond(Ind, i);
#ifdef NO_PK
	if ((i < 0 || !bb) && !admin_forced) return(FALSE);
#endif

	/* log any attempts */
	if (initiator) {
		/* paranoia? shouldn't i always be > 0 here? */
		if (i > 0) s_printf("HOSTILITY: %s attempts to declare war on %s.\n", p_ptr->name, Players[i]->name);
		else s_printf("HOSTILITY: %s attempts to declare war (%d).\n", p_ptr->name, i);
	}
	/* prevent log spam from stormbringer */
	else if (!istown(&p_ptr->wpos)) {
		/* paranoia? shouldn't i always be > 0 here? */
		if (i > 0) s_printf("HOSTILITY: %s attempts to declare war in return on %s.\n", p_ptr->name, Players[i]->name);
		else s_printf("HOSTILITY: %s attempts to declare war in return (%d).\n", p_ptr->name, i);
	}

#ifndef KURZEL_PK
	if (cfg.use_pk_rules == PK_RULES_DECLARE) {
		if (!(p_ptr->pkill & PKILL_KILLER) && (p_ptr->pvpexception != 1)) {
			msg_print(Ind, "\377yYou may not be hostile to other players.");
			return(FALSE);
		}
	}
#endif

	if (cfg.use_pk_rules == PK_RULES_NEVER && (p_ptr->pvpexception != 1)) {
		msg_print(Ind, "\377yYou may not be hostile to other players.");
		return(FALSE);
	}

	/* Non-validated players may not pkill */
	if (p_ptr->inval) {
		msg_print(Ind, "Your account needs to be validated in order to fight other players.");
		return(FALSE);
	}

#if 1
	if (!bb && initiator && !istown(&p_ptr->wpos)) {
		msg_print(Ind, "\377yYou need to be in town to declare war.");
		return(FALSE);
	}
#endif

	if (p_ptr->pvpexception == 2) return(FALSE);
	if (p_ptr->pvpexception == 3) {
		p_ptr->chp = -3;
		strcpy(p_ptr->died_from, "adrenaline poisoning");
		p_ptr->died_from_ridx = 0;
		p_ptr->deathblow = 0;
		p_ptr->energy = -666;
		//p_ptr->death = TRUE;
		player_death(Ind);
		return(FALSE);
	}

	if (i > 0) {
		q_ptr = Players[i];

		/* Make sure players aren't in the same party */
		if (!bb && p_ptr->party && player_in_party(p_ptr->party, i)) {
			msg_format(Ind, "\377y%^s is in your party!", q_ptr->name);
			return(FALSE);
		}

		/* Ensure we don't add the same player twice */
		for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next) {
			/* Check this ID */
			if (h_ptr->id == q_ptr->id) {
				msg_format(Ind, "\377yYou are already hostile toward %s.", q_ptr->name);
				return(FALSE);
			}
		}

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = q_ptr->id;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->hostile;
		p_ptr->hostile = h_ptr;

		/* Message */
		if (bb) {
			msg_format(Ind, "\377yYou are now hostile toward %s.", q_ptr->name);
			msg_format(i, "\377y* Player %s declared war on you! *", p_ptr->name);
		} else {
			msg_format(Ind, "\374\377RYou are now hostile toward %s.", q_ptr->name);
			msg_format(i, "\374\377R* Player %s declared war on you! *", p_ptr->name);
		}

		/* Success */
		return(TRUE);
	} else {
		/* Tweak - inverse i once more */
		i = 0 - i;

		/* Ensure we don't add the same party twice */
		for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next) {
			/* Check this ID */
			if (h_ptr->id == 0 - i) {
				msg_format(Ind, "\377yYou are already hostile toward party '%s'.", parties[i].name);
				return(FALSE);
			}
		}

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = 0 - i;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->hostile;
		p_ptr->hostile = h_ptr;

#if 0
		/* prevent anti-tele amulet instant recall into barrow-downs and crap */
		if (p_ptr->word_recall) //&& istown(&p_ptr->wpos)
			set_recall_timer(Ind, 0);
#endif

		/* Message */
		msg_format(Ind, "\377RYou are now hostile toward party '%s'.", parties[i].name);
		msg_broadcast_format(Ind, "\374\377R* %s declares war on party '%s'. *", p_ptr->name, parties[i].name);

		/* Success */
		return(TRUE);
	}
}

/*
 * Remove an entry from a player's list of hostilities
 */
bool remove_hostility(int Ind, cptr name, bool admin_forced) {
	player_type *p_ptr = Players[Ind];
	hostile_type *h_ptr, *i_ptr;
	cptr p, q = NULL;
	int i = name_lookup_loose(Ind, name, TRUE, TRUE, FALSE);

	if (!i) return(FALSE);

	/* Check for another silliness */
	if (i == Ind) {
		/* Message */
		msg_print(Ind, "\377GYou are not hostile toward yourself.");

		return(FALSE);
	}

#ifdef NO_PK
	/* If it's a blood bond, players may fight in safe zones and with party members np */
	if (i < 0 && !admin_forced) return(FALSE);
	else if (!check_blood_bond(Ind, i) && !admin_forced) return(FALSE);
#endif

	/* Forge name */
	if (i > 0) q = Players[i]->name;

	/* Initialize lock-step */
	i_ptr = NULL;

	/* Search entries */
	for (h_ptr = p_ptr->hostile; h_ptr; i_ptr = h_ptr, h_ptr = h_ptr->next) {
		/* Lookup name of this entry */
		if (h_ptr->id > 0) {
			/* Efficiency */
			if (i < 0) continue;

			/* Look up name */
			p = lookup_player_name(h_ptr->id);

			/* Check player name */
			//if (p && (streq(p, q) || streq(p, name)))
			if (p && streq(p, q)) {
				/* Delete this entry */
				if (i_ptr) {
					/* Skip over */
					i_ptr->next = h_ptr->next;
				} else {
					/* Adjust beginning of list */
					p_ptr->hostile = h_ptr->next;
				}

				/* Message */
				msg_format(Ind, "\374\377GNo longer hostile toward %s.", p);
				msg_format(i, "\374\377G%s is no longer hostile toward you.", p_ptr->name);

				/* Delete node */
				KILL(h_ptr, hostile_type);

				/* Success */
				return(TRUE);
			}
		} else {
			/* Efficiency */
			if (i >= 0) continue;

			/* Assume this is a party */
			//if (streq(parties[0 - h_ptr->id].name, q))
			if (i == h_ptr->id) {
				/* Delete this entry */
				if (i_ptr) {
					/* Skip over */
					i_ptr->next = h_ptr->next;
				} else {
					/* Adjust beginning of list */
					p_ptr->hostile = h_ptr->next;
				}

				/* Message */
				msg_format(Ind, "\374\377GNo longer hostile toward party '%s'.", parties[0 - i].name);
				msg_broadcast_format(Ind, "\374\377G%s is no longer hostile toward party '%s'.", p_ptr->name, parties[0 - i].name);

				/* Delete node */
				KILL(h_ptr, hostile_type);

				/* Success */
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

/*
 * Check if one player is hostile toward the other.
 * Returns FALSE if player and target are identical (ie the same player).
 */
bool check_hostile(int attacker, int target) {
	player_type *p_ptr = Players[attacker], *q_ptr = Players[target];
	hostile_type *h_ptr;

	/* Never be hostile to oneself */
	if (attacker == target) return(FALSE);

	/* Highlander Tournament is a fight to death */
	if ((p_ptr->global_event_temp & PEVF_AUTOPVP_00) &&
	    (q_ptr->global_event_temp & PEVF_AUTOPVP_00)) {
		return(TRUE);
	}

	/* towns are safe-zones for ALL players - except for blood bond */
	if ((istown(&q_ptr->wpos) || istown(&p_ptr->wpos)) &&
	    (!check_blood_bond(attacker, target) ||
	     p_ptr->afk || q_ptr->afk))
		return(FALSE);
	/* outside of towns, PvP-mode means auto-hostility! */
	else if (((q_ptr->mode & MODE_PVP) && (p_ptr->mode & MODE_PVP))
#ifdef KURZEL_PK
	    || (((q_ptr->pkill & PKILL_SET) && (p_ptr->pkill & PKILL_SET)) && ((!p_ptr->party && !q_ptr->party) || !(q_ptr->party == p_ptr->party)))
#endif
	    )
		return(TRUE);

	/* Scan list */
	for (h_ptr = p_ptr->hostile; h_ptr; h_ptr = h_ptr->next) {
		/* Check ID */
		if (h_ptr->id > 0) {
			/* Identical ID's yield hostility */
			if (h_ptr->id == q_ptr->id)
				return(TRUE);
		} else {
			/* Check if target belongs to hostile party */
			if (q_ptr->party == 0 - h_ptr->id)
				return(TRUE);
		}
	}

	/* Not hostile */
	return(FALSE);
}


/*
 * Add/remove a player to/from another player's list of ignorance.
 * These functions should be common with hostilityes in the future. -Jir-
 */
bool add_ignore(int Ind, cptr name) {
	player_type *p_ptr = Players[Ind], *q_ptr;
	hostile_type *h_ptr, *i_ptr;
	int i;
	cptr p, q = NULL;
#ifdef TOMENET_WORLDS
	int snum = 0;
	char search[80], *pname;
#endif

	/* Check for silliness */
	if (!name) {
		msg_print(Ind, "Usage: /ignore foobar");

		return(FALSE);
	}

#ifdef TOMENET_WORLDS
	if ((pname = strchr(name, '@'))) {
		struct remote_ignore *curr, *prev = NULL;
		struct rplist *w_player;

		strncpy(search, name, pname - name);
		search[pname - name] = '\0';
		snum = atoi(pname + 1);
		w_player = world_find_player(search, snum);
		if (!w_player) {
			msg_format(Ind, "Could not find %s in the world", search);
			return(FALSE);
		}
		curr = p_ptr->w_ignore;
		while (curr) {
			if (curr->serverid == w_player->server && curr->id == w_player->id) break;
			prev = curr;
			curr = curr->next;
		}
		if (!curr) {
			msg_format(Ind, "Ignoring %s across the world", w_player->name);
			curr = NEW(struct remote_ignore);
			curr->serverid = w_player->server;
			curr->id = w_player->id;
			curr->next = NULL;
			if (prev)
				prev->next = curr;
			else
				p_ptr->w_ignore = curr;
		}
		else {
			msg_format(Ind, "Hearing %s from across the world", search);
			if (!prev)
				p_ptr->w_ignore = curr->next;
			else
				prev->next = curr->next;
			FREE(curr, struct remote_ignore);
		}
		return(TRUE);
	}
#endif

	i = name_lookup_loose(Ind, name, TRUE, TRUE, FALSE);
	if (!i) return(FALSE);

	/* Check for another silliness */
	if (i == Ind) {
		/* Message */
		msg_print(Ind, "You cannot ignore yourself.");

		return(FALSE);
	}

	/* Forge name */
	if (i > 0) {
		q = Players[i]->name;
	}

	/* Initialize lock-step */
	i_ptr = NULL;

	/* Toggle ignorance if already on the list */
	for (h_ptr = p_ptr->ignore; h_ptr; i_ptr = h_ptr, h_ptr = h_ptr->next) {
		/* Lookup name of this entry */
		if (h_ptr->id > 0) {
			/* Efficiency */
			if (i < 0) continue;

			/* Look up name */
			p = lookup_player_name(h_ptr->id);

			/* Check player name */
			if (p && streq(p, q)) {
				/* Delete this entry */
				if (i_ptr) {
					/* Skip over */
					i_ptr->next = h_ptr->next;
				} else {
					/* Adjust beginning of list */
					p_ptr->ignore = h_ptr->next;
				}

				/* Message */
				msg_format(Ind, "Now listening to %s again.", p);

				/* Delete node */
				KILL(h_ptr, hostile_type);

				/* Success */
				return(TRUE);
			}
		} else {
			/* Efficiency */
			if (i > 0) continue;

			/* Assume this is a party */
//			if (streq(parties[0 - h_ptr->id].name, q))
			if (i == h_ptr->id) {
				/* Delete this entry */
				if (i_ptr) {
					/* Skip over */
					i_ptr->next = h_ptr->next;
				} else {
					/* Adjust beginning of list */
					p_ptr->ignore = h_ptr->next;
				}

				/* Message */
				msg_format(Ind, "Now listening to party '%s' again.", parties[0 - i].name);

				/* Delete node */
				KILL(h_ptr, hostile_type);

				/* Success */
				return(TRUE);
			}
		}
	}

	if (i > 0) {
		q_ptr = Players[i];

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = q_ptr->id;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->ignore;
		p_ptr->ignore = h_ptr;

		/* Message */
		msg_format(Ind, "You aren't hearing %s any more.", q_ptr->name);

		/* Success */
		return(TRUE);
	} else {
		/* Tweak - inverse i once more */
		i = 0 - i;

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = 0 - i;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->ignore;
		p_ptr->ignore = h_ptr;

		/* Message */
		msg_format(Ind, "You aren't hearing party '%s' any more.", parties[i].name);

		/* Success */
		return(TRUE);
	}
}

/*
 * Check if one player is ignoring the other
 */
bool check_ignore(int attacker, int target) {
	player_type *p_ptr = Players[attacker];
	hostile_type *h_ptr;

	/* Scan list */
	for (h_ptr = p_ptr->ignore; h_ptr; h_ptr = h_ptr->next) {
		/* Check ID */
		if (h_ptr->id > 0) {
			/* Identical ID's yield hostility */
			if (h_ptr->id == Players[target]->id)
				return(TRUE);
		} else {
			/* Check if target belongs to hostile party */
			if (Players[target]->party == 0 - h_ptr->id)
				return(TRUE);
		}
	}

	/* Not hostile */
	return(FALSE);
}

/*
 * The following is a simple hash table, which is used for mapping a player's
 * ID number to his name.  Only players that are still alive are stored in
 * the table, thus the mapping from a 32-bit integer is very sparse.  Also,
 * duplicate ID numbers are prohibitied.
 *
 * The hash function is going to be h(x) = x % n, where n is the length of
 * the table.  For efficiency reasons, n will be a power of 2, thus the
 * hash function can be a bitwise "and" and get the relevant bits off the end.
 *
 * No "probing" is done; if any two ID's map to the same hash slot, they will
 * be chained in a linked list.  This will most likely be a very rare thing,
 * however.
 */


/*
 * Return the slot in which an ID should be stored.
 */
static int hash_slot(int id) {
	/* Be very efficient */
	return(id & (NUM_HASH_ENTRIES - 1));
}


/*
 * Lookup a player record ID.  Will return NULL on failure.
 */
hash_entry *lookup_player(int id) {
	int slot;
	hash_entry *ptr;

	/* Get the slot */
	slot = hash_slot(id);

	/* Acquire the pointer to the first element in the chain */
	ptr = hash_table[slot];

	/* Search the chain, looking for the correct ID */
	while (ptr) {
		/* Check this entry */
		if (ptr->id == id)
			return(ptr);

		/* Next entry in chain */
		ptr = ptr->next;
	}

	/* Not found */
	return(NULL);
}


/*
 * Get the player's level. (In the distant past, this stored the player's highest level.)
 */
char lookup_player_level(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->level);

	/* Not found */
	return(-1);
}
/* Return a player's custom sort order of a character in his account overview screen */
char lookup_player_order(s32b id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->order);

	/* Not found */
	return(-1);
}
void set_player_order(s32b id, byte order) {
	hash_entry *ptr = lookup_player(id);

	ptr->order = order;
}
/*
 * Get the player's highest level.
 */
char lookup_player_maxplv(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->max_plv);

	/* Not found */
	return(-1);
}

char lookup_player_admin(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->admin ? 1 : 0);

	/* Not found */
	return(-1);
}

u16b lookup_player_type(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->race | (ptr->class << 8));

	/* Not found */
	//return(-1L); -- unsigned!
	return(0);
}

/*
 * Get the player's current party.
 */
s32b lookup_player_party(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->party);

	/* Not found */
	return(-1L);
}

s32b lookup_player_guild(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->guild);

	/* Not found */
	return(-1L);
}

u32b lookup_player_guildflags(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->guild_flags);

	/* Not found */
	//return(-1L); -- unsigned!
	return(0U);
}

/*
 * Get the timestamp for the last time player was on.
 */
time_t lookup_player_laston(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->laston);

	/* Not found */
	return(-1L);
}

/*
 * Lookup a player name by ID.  Will return NULL if the name doesn't exist.
 */
cptr lookup_player_name(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->name);

	/* Not found */
	return(NULL);
}

/*
 * Get the player's character mode (needed for account overview screen only).
 */
u32b lookup_player_mode(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->mode);

	/* Not found */
	return(255);
}

/*
 * Get the player's current location in the world.
 */
struct worldpos lookup_player_wpos(int id) {
	hash_entry *ptr;
	struct worldpos wpos = {-1, -1, 0};

	if ((ptr = lookup_player(id)))
		return(ptr->wpos);

	/* Not found */
	return(wpos);
}

#ifdef AUCTION_SYSTEM
/*
 * Get the amount of gold the player has.
 */
s32b lookup_player_au(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->au);

	/* Not found */
	return(-1L);
}

/*
 * Get the player's bank balance.
 */
s32b lookup_player_balance(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->balance);

	/* Not found */
	return(-1L);
}
#endif

/*
 * Lookup a player's ID by name.  Return 0 if not found.
 */
int lookup_player_id(cptr name) {
	hash_entry *ptr;
	int i;

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr) {
			/* Check this name */
			if (!strcmp(ptr->name, name)) return(ptr->id);
			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Not found */
	return(0);
}
/* Case-insensitive lookup_player_id() */
int lookup_case_player_id(cptr name) {
	hash_entry *ptr;
	int i;

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr) {
			/* Check this name */
			if (!strcasecmp(ptr->name, name)) return(ptr->id);
			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Not found */
	return(0);
}

bool fix_player_case(char *name) {
	hash_entry *ptr;
	int i;

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr) {
			/* Check this name */
			if (!strcasecmp(ptr->name, name)) {
				/* Overwrite it with currently used capitalization */
				if (strcmp(ptr->name, name)) {
					strcpy(name, ptr->name);
					return(TRUE);
				}
				return(FALSE);
			}

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Not found */
	return(FALSE);
}

/* Messed up version of lookup_player_id for house ownership changes
   that involve messed up characters (in terms of their ID, for example
   if they were copied / restored instead of cleanly created..) mea culpa! */
int lookup_player_id_messy(cptr name) {
	hash_entry *ptr;
	int i, tmp_id = 0;

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr) {
			/* Check this name
			   The ' && (ptr->id > 0)' part is only needed for the hack below this while loop - C. Blue */
			if (!strcmp(ptr->name, name) && (ptr->id > 0)) {
				tmp_id = ptr->id;
				break;
			}

			/* Next entry in chain */
			ptr = ptr->next;
		}
		if (tmp_id) break;
	}

	/* In case of messed up IDs (due to restoring erased chars, etc)
	   perform a double check on all players who are currently on: - C. Blue
	   (on a clean server *cough* this shouldn't be needed I think) */
	for (i = 1; i <= NumPlayers; i++)
		if (!strcmp(Players[i]->name, name)) {
			if (tmp_id && (tmp_id != Players[i]->id)) /* mess-up detected */
				s_printf("$ID-WARNING$ Player %s has hash id %d but character id %d.\n", Players[i]->name, tmp_id, Players[i]->id);
			/* have character-id override the hash id.. sigh */
			if (Players[i]->id > 0) return(Players[i]->id);
			/* the > 0 check is paranoia.. */
			break;
		}

	if (tmp_id) return(tmp_id);

	/* Not found */
	return(0);
}

u32b lookup_player_account(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id)))
		return(ptr->account);

	/* Not found */
	return(0L);
}

char lookup_player_winner(int id) {
	hash_entry *ptr;

	if ((ptr = lookup_player(id))) return(ptr->winner);

	/* Not found */
	return(-1);
}

void stat_player(char *name, bool on) {
	int id;
	int slot;
	hash_entry *ptr;

	id = lookup_player_id(name);
	if (id) {
		slot = hash_slot(id);
		ptr = hash_table[slot];
		while (ptr) {
			if (ptr->id == id) ptr->laston = on ? 0L : time(&ptr->laston);
			ptr = ptr->next;
		}
	}
}

/* Timestamp an existing player */
void clockin(int Ind, int type) {
	int slot;
	hash_entry *ptr;
	player_type *p_ptr = Players[Ind];

	slot = hash_slot(p_ptr->id);
	ptr = hash_table[slot];

	while (ptr) {
		if (ptr->id == p_ptr->id) {
			switch (type) {
			case 0:
				if (ptr->laston) ptr->laston = time(&ptr->laston);
				break;
			case 1:
				/*was: if (parm > ptr->level) in the past, now just uses current level, max_plv stores max level now. - C. Blue */
				ptr->level = p_ptr->lev;
				break;
			case 2:
				ptr->party = p_ptr->party;
				break;
			case 3:
				ptr->guild = p_ptr->guild;
				ptr->guild_flags = p_ptr->guild_flags;
				break;
			case 4:
				ptr->xorder = p_ptr->xorder_id;
				break;
#ifdef AUCTION_SYSTEM
			case 5:
				ptr->au = p_ptr->au;
				ptr->balance = p_ptr->balance;
				break;
#endif
			case 6:
				ptr->admin = p_ptr->admin_dm ? 1 : (p_ptr->admin_wiz ? 2 : 0);
				break;
			case 7:
				ptr->wpos = p_ptr->wpos;
				break;
			case 8:
				ptr->houses = p_ptr->houses_owned;
				break;
			case 9:
				ptr->winner = (p_ptr->total_winner ? 1 : 0) + (p_ptr->once_winner ? 2 : 0) + (p_ptr->iron_winner ? 4 : 0) + (p_ptr->iron_winner_ded ? 8 : 0);
				break;
#if defined(ENABLE_HELLKNIGHT) || defined(ENABLE_CPRIEST) /* not needed for 'normal' classes that don't change, not even Death Knight */
			case 10:
				ptr->class = p_ptr->pclass;
				break;
#endif
			case 11:
				ptr->max_plv = p_ptr->max_plv;
				break;
			}
			break;
		}
		ptr = ptr->next;
	}
}
void clockin_id(s32b id, int type, int parm, u32b parm2) {
	int slot;
	hash_entry *ptr;

	slot = hash_slot(id);
	ptr = hash_table[slot];

	while (ptr) {
		if (ptr->id == id) {
			switch (type) {
			case 0:
				if (ptr->laston) ptr->laston = time(&ptr->laston);
				break;
			case 1:
				/*was: if (parm > ptr->level) in the past, now just uses current level, max_plv stores max level now. - C. Blue */
				ptr->level = parm;
				break;
			case 2:
				ptr->party = parm;
				break;
			case 3:
				ptr->guild = parm;
				ptr->guild_flags = parm2;
				break;
			case 4:
				ptr->xorder = parm;
				break;
			case 6:
				ptr->admin = parm;
				break;
			case 8:
				ptr->houses = parm;
				break;
			case 9:
				ptr->winner = parm;
				break;
#if defined(ENABLE_HELLKNIGHT) || defined(ENABLE_CPRIEST) /* not needed for 'normal' classes that don't change, not even Death Knight */
			case 10:
				ptr->class = parm;
				break;
#endif
			case 11:
				ptr->max_plv = parm;
				break;
			}
			break;
		}
		ptr = ptr->next;
	}
}

/* dish out a valid new player ID */
int newid() {
	int id;
	int slot;
	hash_entry *ptr;

/* there should be no need to do player_id > MAX_ID check
   as it should cycle just fine */

	for (id = player_id; id <= MAX_ID; id++) {
		slot = hash_slot(id);
		ptr = hash_table[slot];

		while (ptr) {
			if (ptr->id == id) break;
			ptr = ptr->next;
		}
		if (ptr) continue;	/* its on a valid one */
		player_id = id + 1;	/* new cycle counter */
		return(id);
	}
	for (id = 1; id < player_id; id++) {
		slot = hash_slot(id);
		ptr = hash_table[slot];

		while (ptr) {
			if (ptr->id == id) break;
			ptr = ptr->next;
		}
		if (ptr) continue;	/* its on a valid one */
		player_id = id + 1;	/* new cycle counter */
		return(id);
	}
	s_printf("Warning: newid() returns 0!\n");
	return(0);	/* no user IDs available - not likely */
}

void sf_delete(const char *name) {
	int i, k = 0;
	char temp[128],fname[MAX_PATH_LENGTH];

	/* Extract "useful" letters */
	for (i = 0; name[i]; i++) {
		char c = name[i];

		/* Accept some letters */
		if (isalphanum(c)) temp[k++] = c;

		/* Convert space, dot, and underscore to underscore */
		else if (strchr(SF_BAD_CHARS, c)) temp[k++] = '_';
	}
	temp[k] = '\0';

	path_build(fname, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, temp);
	unlink(fname);
}
/* Back up a player save file instead of just deleting it */
void sf_rename(const char *name, bool keep_copy) {
	int i, k = 0;
	char temp[MAX_PATH_LENGTH], fname[MAX_PATH_LENGTH], fname_new[MAX_PATH_LENGTH];
	struct stat filestat;

	/* Extract "useful" letters */
	for (i = 0; name[i]; i++) {
		char c = name[i];

		/* Accept some letters */
		if (isalphanum(c)) temp[k++] = c;

		/* Convert space, dot, and underscore to underscore */
		else if (strchr(SF_BAD_CHARS, c)) temp[k++] = '_';
	}
	temp[k] = '\0';

	path_build(fname, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, temp);
	path_build(fname_new, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "__bak__");

	//TODO: Check if file already exists, if yes, add incrementing number to the file name

	strcat(fname_new, temp); //note: might theoretically exceed MAX_PATH_LENGTH, but practically not really :-p

	/* File already exists? Then add incrementing number to the file name */
	i = 2;
	strcpy(temp, fname_new);
	while (stat(fname_new, &filestat) != -1) {
		strcpy(fname_new, temp);
		strcat(fname_new, format("_#%d", i++));
	}
	if (i > 2) s_printf("sf_rename: Added incremental backup savefile number %d\n", i - 1);

	if (keep_copy) {
		FILE *fp1 = NULL;
		FILE *fp2 = NULL;
		int fd1 = -1;
		struct stat stbuf;
		off_t file_size = 0;
		char *buf = NULL;

		/* Open the input file */
		fd1 = open(fname, O_RDONLY);
		if (fd1 == -1) {
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
		}

		fp1 = fdopen(fd1, "rb");
		if (!fp1) {
			close(fd1);
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
			return;
		}

		/* Use fstat to get the size of the file */
		if (fstat(fd1, &stbuf) == -1) {
			fclose(fp1);
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
			return;
		}

		file_size = stbuf.st_size;

		buf = mem_alloc(file_size);
		if (!buf) {
			fclose(fp1);
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
			return;
		}

		/* Read the whole input file */
		if (fread(buf, 1, file_size, fp1) < file_size) {
			mem_free(buf);
			fclose(fp1);
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
			return;
		}

		/* Open the output file */
		fp2 = fopen(fname_new, "wb");
		if (!fp2) {
			mem_free(buf);
			fclose(fp1);
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
			return;
		}

		/* Write the remaining first line of input to output */
		if (fwrite(buf, 1, file_size, fp2) < file_size) {
			mem_free(buf);
			fclose(fp1);
			fclose(fp2);
			/* paranoia - fall back to renaming */
			rename(fname, fname_new);
			return;
		}

		/* Close the files and free memory */
		mem_free(buf);
		fclose(fp1);
		fclose(fp2);
	} else rename(fname, fname_new);
}
/* Rename an estate savefile (without changing the owner's name, just a simple file rn) */
void ef_rename(const char *name) {
	int i, k = 0;
	char temp[MAX_PATH_LENGTH], fname[MAX_PATH_LENGTH], fname_new[MAX_PATH_LENGTH];
	char epath[MAX_PATH_LENGTH];
	struct stat filestat;

	/* Extract "useful" letters */
	for (i = 0; name[i]; i++) {
		char c = name[i];

		/* Accept some letters */
		if (isalphanum(c)) temp[k++] = c;

		/* Convert space, dot, and underscore to underscore */
		else if (strchr(SF_BAD_CHARS, c)) temp[k++] = '_';
	}
	temp[k] = '\0';

	path_build(epath, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "estate");
	path_build(fname, MAX_PATH_LENGTH, epath, temp);
	path_build(fname_new, MAX_PATH_LENGTH, epath, "__bak__");

	strcat(fname_new, temp); //note: might theoretically exceed MAX_PATH_LENGTH, but practically not really :-p

	/* File already exists? Then add incrementing number to the file name */
	i = 2;
	strcpy(temp, fname_new);
	while (stat(fname_new, &filestat) != -1) {
		strcpy(fname_new, temp);
		strcat(fname_new, format("_#%d", i++));
	}
	if (i > 2) s_printf("ef_rename: Added incremental backup savefile number %d (%s->%s)\n", i - 1, fname, fname_new);

	rename(fname, fname_new);
}

/* For marking accounts active */
static unsigned char *account_active = NULL;
#if defined(EMAIL_NOTIFICATIONS) && defined(EMAIL_NOTIFICATION_EXPIRY_CHAR)
/* For marking accounts that 1+ char is going to expire */
static unsigned char *account_expiry_char = NULL;
#endif
/*
 *  Called once every 24 hours. Deletes unused IDs.
 */
void scan_characters() {
	int slot, amt = 0, total = 0;
	hash_entry *ptr, *pptr = NULL;
	time_t now;

#if 0 /* Low-performance version */
	struct account acc;
#endif

#ifdef PLAYERS_NEVER_EXPIRE
	s_printf("(scan_characters() disabled due to PLAYERS_NEVER_EXPIRE (DEF).)\n");
	return;
#else
	if (cfg.players_never_expire) {
		s_printf("(scan_characters() disabled due to PLAYERS_NEVER_EXPIRE (cfg).)\n");
		return;
	}
#endif

	/* Allocate an array for marking accounts as active */
	C_MAKE(account_active, MAX_ACCOUNTS / 8, unsigned char);
#if defined(EMAIL_NOTIFICATIONS) && defined(EMAIL_NOTIFICATION_EXPIRY_CHAR)
	/* Marking accounts that one or more chars are about to expire */
	C_MAKE(account_expiry_char, MAX_ACCOUNTS / 8, unsigned char);
#endif

	now = time(&now);

	s_printf("Starting player inactivity check..\n");
	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		pptr = NULL;
		ptr = hash_table[slot];
		while (ptr) {
			total++;
			if ((ptr->laston && (now - ptr->laston > 3600 * 24 * CHARACTER_EXPIRY_DAYS))
			    && !(cfg.admins_never_expire && ptr->admin)) {
				if (ptr->level >= 50 && ptr->admin == 0) l_printf("%s \\{D%s, level %d, was erased by timeout\n", showdate(), ptr->name, ptr->level);
				erase_player_hash(slot, &pptr, &ptr);
				amt++;
				continue;
			} else {
#if defined(EMAIL_NOTIFICATIONS) && defined(EMAIL_NOTIFICATION_EXPIRY_CHAR)
				if ((ptr->laston && (now - ptr->laston >= 3600 * 24 * (CHARACTER_EXPIRY_DAYS - EMAIL_NOTIFICATION_EXPIRY_CHAR)))
				    && !(cfg.admins_never_expire && ptr->admin)) //ACC_EMN_CX
					account_expiry_char[ptr->account / 8] |= 1U << (ptr->account % 8);
#endif
#if 0 /* Low-performance version */
				/* if a character didn't timeout, timestamp his
				   account here, to help the account-expiry routines,
				   keeping the account 'active' - see scan_accounts() - C. Blue */
				/* avoid tagging multi-char accounts again for each char - once is sufficient */
				if (GetAccountID(&acc, ptr->account, TRUE)) {
					if (acc.acc_laston != now) {
						acc.acc_laston = now;
						WriteAccount(&acc, FALSE);
					}
				}
#else
				/* If a character didn't timeout, mark his
				   account as active here */
				account_active[ptr->account / 8] |= 1U << (ptr->account % 8);
#endif
			}

			/* proceed to next entry of this hash slot */
			pptr = ptr;
			ptr = ptr->next;
		}
	}

	s_printf("  %d players expired\n", amt);
	s_printf("  %d players old total, %d new total\n", total, total - amt);
	s_printf("Finished player inactivity check.\n");
}
/*
 *  Called once every 24 hours. Deletes unused Account IDs.
 *  It's called straight after scan_characters, usually.
 *  Unused means that there aren't any characters on it,
 *  and it's not been used to log in with for a certain amount of time. - C. Blue
 */
void scan_accounts() {
	int total = 0, nondel = 0, active = 0, expired = 0, fixed = 0;
	bool modified, valid;
	FILE *fp;
	char buf[1024];
	struct account acc;
	time_t now;

#ifdef PLAYERS_NEVER_EXPIRE
	s_printf("(scan_accounts() disabled due to PLAYERS_NEVER_EXPIRE.)\n");
	return;
#else
	if (cfg.players_never_expire) {
		s_printf("(scan_accounts() disabled due to PLAYERS_NEVER_EXPIRE (cfg).)\n");
		return;
	}
#endif

	now = time(NULL);

	if (!account_active) {
		s_printf("scan_characters() must be called before scan_accounts() is called!\n");
		return;
	}

	s_printf("Starting account inactivity check..\n");

	path_build(buf, 1024, ANGBAND_DIR_SAVE, "tomenet.acc");
	fp = fopen(buf, "rb+");
	if (!fp) return;

	while (fread(&acc, sizeof(struct account), 1, fp)) {
		modified = FALSE;
		valid = TRUE; //assume valid

		/* Count all accounts in the file */
		total++;

		/* Skip accounts that are already marked as 'Deleted' */
		if (acc.flags & ACC_DELD) continue;

		/* Count non-deleted accounts */
		nondel++;

		//if (!acc.acc_laston) continue; /* not using this 'hack' for staticing here */

		/* Check for bad account id first of all! (These caused a crash on Arcade server actually) */
		if (acc.id >= MAX_ACCOUNTS) {
			/* Log it */
			s_printf("  Bad account ID: %d\n", acc.id);

			/* Fix the id */
			acc.id = MAX_ACCOUNTS - 1;

			/* Mark as deleted */
			acc.flags |= ACC_DELD;

			modified = TRUE;
			valid = FALSE; //it's deleted!
		}

		/* Admin accounts always count as active */
		else if (acc.flags & ACC_ADMIN) {
			active++;
			continue;

		}
		/* fix old accounts that don't have a timestamp yet */
		else if (!acc.acc_laston) {
			acc.acc_laston = acc.acc_laston_real = now; /* set new timestamp */
			fixed++;
			modified = TRUE;
		}

		/* Was the account marked as active? */
		else if (account_active[acc.id / 8] & (1U << (acc.id % 8))) {
			acc.acc_laston = now;

			/* Count active accounts */
			active++;

			modified = TRUE;
		}

		/* test for expiry -> delete */
		else if (now - acc.acc_laston > 3600 * 24 * ACCOUNT_EXPIRY_DAYS) {
#if 1 /* temporarily disable for testing purpose? */
			acc.flags |= ACC_DELD;

			/* Count expired accounts */
			expired++;

			s_printf("  Account '%s' expired.\n", acc.name);
			modified = TRUE;
#else
			s_printf("  (TESTING) Account '%s' expired.\n", acc.name);
#endif
			valid = FALSE; //it's expired!
		}

#ifdef EMAIL_NOTIFICATIONS
		if (valid) {
 #ifdef EMAIL_NOTIFICATION_EXPIRY_ACC
			if (!(acc.flags & ACC_EMN_AX) && acc.email[0] && now - acc.acc_laston >= 3600 * 24 * (ACCOUNT_EXPIRY_DAYS - EMAIL_NOTIFICATION_EXPIRY_ACC)) {
				int res;

				acc.flags |= ACC_EMN_AX;
				modified = TRUE;

				res = system(format("echo 'Your account will expire within approximately %d days if you don't log in.' > _testmail.txt", EMAIL_NOTIFICATION_EXPIRY_ACC));
				res += system(format("sh ./email.sh \"%s\" \"_testmail.txt\" &", acc.email));
				remove("_testmail.txt");
				if (res) s_printf("ACC_EMN_AX: System shell error (account '%s').\n", acc.name);
				else s_printf("ACC_EMN_AX: Dispatched to '%s', account '%s'.\n", acc.email, acc.name);
			} else if ((acc.flags & ACC_EMN_AX) && !(now - acc.acc_laston >= 3600 * 24 * (ACCOUNT_EXPIRY_DAYS - EMAIL_NOTIFICATION_EXPIRY_ACC))) {
				/* Account is no longer near expiry -> clear flag again */
				acc.flags &= ~ACC_EMN_AX;
				modified = TRUE;
			}
 #endif
 #ifdef EMAIL_NOTIFICATION_EXPIRY_CHAR
			if (!(acc.flags & ACC_EMN_CX) && acc.email[0] && (account_expiry_char[acc.id / 8] & (1U << (acc.id % 8)))) {
				int res;

				acc.flags |= ACC_EMN_CX;
				modified = TRUE;

				res = system(format("echo 'At least one of your characters will expire within approximately %d days if you don't log in.' > _testmail.txt", EMAIL_NOTIFICATION_EXPIRY_CHAR));
				res += system(format("sh ./email.sh \"%s\" \"_testmail.txt\" &", acc.email));
					remove("_testmail.txt");
				if (res) s_printf("ACC_EMN_CX: System shell error (account '%s').\n", acc.name);
				else s_printf("ACC_EMN_CX: Dispatched to '%s', account '%s'.\n", acc.email, acc.name);
			} else if ((acc.flags & ACC_EMN_CX) && !(account_expiry_char[acc.id / 8] & (1U << (acc.id % 8)))) {
				/* No character is near expiry anymore -> clear flag again */
				acc.flags &= ~ACC_EMN_CX;
				modified = TRUE;
			}
 #endif
		}
#endif

		/* Mark 'Deleted' (ie invalid) accounts and timestamp valid accounts */
		//if (modified) WriteAccount(&acc, FALSE);
		if (modified) {
			fseek(fp, -((signed int)(sizeof(struct account))), SEEK_CUR);
			if (fwrite(&acc, sizeof(struct account), 1, fp) < 1) {
				s_printf("Writing to account file failed: %s\n", feof(fp) ? "EOF" : strerror(ferror(fp)));
			}

			/* HACK - Prevent reading past the end of the file on Windows - mikaelh */
			fseek(fp, 0, SEEK_CUR);
		}
	}

	if (fixed) s_printf("  %d accounts have been fixed.\n", fixed);
	s_printf("  %d accounts in total, %d non-deleted, %d active.\n", total, nondel, active);
	s_printf("  %d accounts have expired.\n", expired);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));
	fclose(fp);
	s_printf("Finished account inactivity check.\n");

	C_KILL(account_active, MAX_ACCOUNTS / 8, unsigned char);
#if defined(EMAIL_NOTIFICATIONS) && defined(EMAIL_NOTIFICATION_EXPIRY_CHAR)
	C_KILL(account_expiry_char, MAX_ACCOUNTS / 8, unsigned char);
#endif
}

/* Rename a player's char savegame as well as the name inside.
   Not sure if this function is 100% ok to use, regarding treatment of hash table. */
void rename_character(char *pnames) {
	int slot, pos, i, Ind;
	hash_entry *ptr;
	char pname[40], nname[40];
	player_type *p_ptr;

	Ind = ++NumPlayers;

	/* Allocate memory for him */
	MAKE(Players[Ind], player_type);
	C_MAKE(Players[Ind]->inventory, INVEN_TOTAL, object_type);
	p_ptr = Players[Ind];

	/* extract old+new name from 'pnames' */
	if (!(strchr(pnames, ':'))) return;
	pos = strchr(pnames, ':') - pnames;
	strncpy(pname, pnames, pos);
	pname[pos] = 0;
	strcpy(nname, pnames + pos + 1);
	s_printf("Renaming player: %s to %s\n", pname, nname);

	/* scan hash entries */
	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		ptr = hash_table[slot];
		while (ptr) {
			if (strcmp(ptr->name, pname)) {
				ptr = ptr->next;
				continue;
			}

			/* load him */
			strcpy(p_ptr->name, pname);
			process_player_name(Ind, TRUE);
			if (!load_player(Ind)) {
				/* bad fail */
				s_printf("rename_character: load_player '%s' failed\n", pname);
				C_KILL(Players[Ind]->inventory, INVEN_TOTAL, object_type);
				KILL(Players[Ind], player_type);
				NumPlayers--;
				return;
			}

			/* change his name */
			strcpy(p_ptr->name, nname);
			if (!process_player_name(Ind, TRUE)) {
				/* done (failure) */
				s_printf("rename_character: bad new name '%s'\n", nname);
				C_KILL(Players[Ind]->inventory, INVEN_TOTAL, object_type);
				KILL(Players[Ind], player_type);
				NumPlayers--;
				return;
			}
			/* change his name in hash table */
			strcpy((char *) ptr->name, p_ptr->name);
			/* save him */
			save_player(Ind);
			/* delete old savefile) */
			sf_delete(pname);

			/* change his name as party owner */
			for (i = 1; i < MAX_PARTIES; i++) { /* was i = 0 but real parties start from i = 1 - mikaelh */
				if (streq(parties[i].owner, ptr->name)) {
					s_printf("Renaming owner of party: %s\n",parties[i].name);
					strcpy(parties[i].owner, nname);
					break;
				}
			}

			/* (guilds only use numerical ID, not names) */

			/* done (success) */
			s_printf("rename_character: success '%s' -> '%s'\n", pname, nname);
			C_KILL(Players[Ind]->inventory, INVEN_TOTAL, object_type);
			KILL(Players[Ind], player_type);
			NumPlayers--;

			/* update live player */
			for (i = 1; i <= NumPlayers; i++) {
				p_ptr = Players[i];
				if (strcmp(p_ptr->name, pname)) continue;

				strcpy(p_ptr->name, nname);
				if (!process_player_name(i, TRUE)) {
					/* done (failure) -- paranoia, shouldn't happen (caught above already!) */
					s_printf("rename_character: *LIVE* bad new name '%s'\n", nname);
					Destroy_connection(i, "Name changed. Please log in again.");
					/* delete old savefile) */
					sf_delete(pname);
					return;
				}
				Send_char_info(i, p_ptr->prace, p_ptr->pclass, p_ptr->ptrait, p_ptr->male, p_ptr->mode, p_ptr->lives - 1, p_ptr->name);
				return;
			}
			return;
		}
	}

	/* free temporary player */
	s_printf("rename_character: name not found\n");
	C_KILL(Players[Ind]->inventory, INVEN_TOTAL, object_type);
	KILL(Players[Ind], player_type);
	NumPlayers--;
}

/* Erase a player by hash - C. Blue
   NOTE: This is NOT used for deaths, but ONLY for /erasechar command or for inactivity timeout. */
void erase_player_hash(int slot, hash_entry **p_pptr, hash_entry **p_ptr) {
	int i, j;
	hash_entry *dptr, *pptr = *p_pptr, *ptr = *p_ptr;
	cptr acc;
	object_type *o_ptr;
	bool backup = FALSE, accok = FALSE;

	acc = lookup_accountname(ptr->id);
	if (!acc) acc = "(no account)";
	else accok = TRUE;

	s_printf("Removing player: %s (%d(%d), %s)\n", ptr->name, ptr->level, ptr->max_plv, acc);

#ifdef SAFETY_BACKUP_PLAYER
	/* Not sure if hash table level is already updated to live player level, so double check here: */
	j = ptr->max_plv;
	for (i = 1; i <= NumPlayers; i++) {
		if (strcmp(Players[i]->name, ptr->name)) continue;
		if (Players[i]->max_plv > j) {
			s_printf("(max_plv %d > hash max_plv %d)\n", Players[i]->max_plv, j);
			j = Players[i]->max_plv;
		}
		break;
	}

	/* Strange bug, workaround.
	   Wild susicion: Bug might stem from savegame already having been restored but the player didn't log in/clockin()
	   the max_plv on it anew, then it got timeouted again, so now the max_plv in the database is still 1 aka default. */
	if (j < ptr->level) {
		s_printf("(BUG: hash max_plv %d < hash level %d)\n", j, ptr->level);
		j = ptr->level;
	}

	if (j < SAFETY_BACKUP_PLAYER) {
		e_printf("(%s) %s (%d, %s)\n", showtime(), ptr->name, ptr->level, acc); /* log to erasure.log file for compact overview */
		s_printf("(Skipping safety backup (max_plv %d < %d))\n", j, SAFETY_BACKUP_PLAYER);
	} else {
		e_printf("(%s) %s (%d, %s) BACKUP\n", showtime(), ptr->name, ptr->level, acc); /* log to erasure.log file for compact overview */
		s_printf("(Creating safety backup (max_plv %d >= %d)\n", j, SAFETY_BACKUP_PLAYER);

		/* rename savefile to backup (side note: unlink() will fail to delete it then later) */
		sf_rename(ptr->name, FALSE);
		backup = TRUE;

		/* save all real estate.. */
		if (!backup_char_estate(0, ptr->id, ptr->name))
			s_printf("(Estate backup: At least one house failed!)\n");
		/* ..and rename estate file to indicate it's just a backup! */
		ef_rename(ptr->name);
	}
#else
	e_printf("(%s) %s (%d, %s)\n", showtime(), ptr->name, ptr->level, acc); /* log to erasure.log file for compact overview */
#endif

	for (i = 1; i < MAX_PARTIES; i++) { /* was i = 0 but real parties start from i = 1 - mikaelh */
		if (streq(parties[i].owner, ptr->name)) {
			s_printf("Disbanding party: %s\n",parties[i].name);
			del_party(i);
			/* remove pending notes to his party -C. Blue */
			for (j = 0; j < MAX_PARTYNOTES; j++) {
				if (!strcmp(party_note_target[j], parties[i].name)) {
					strcpy(party_note_target[j], "");
					strcpy(party_note[j], "");
				}
			}
			break;
		}
	}

	if (accok) {
		//ACC_HOUSE_LIMIT
		i = acc_get_houses(acc);
		i -= ptr->houses;
		acc_set_houses(acc, i);
	}
	kill_houses(ptr->id, OT_PLAYER);

	rem_xorder(ptr->xorder);
	/* Added this one.. should work well? */
	kill_objs(ptr->id);
#ifdef ENABLE_MERCHANT_MAIL
	merchant_mail_death(ptr->name);
#endif

#ifdef AUCTION_SYSTEM
	auction_player_death(ptr->id);
#endif

	/* Wipe Artifacts (s)he had  -C. Blue */
	for (i = 0; i < o_max; i++) {
		o_ptr = &o_list[i];
		if (true_artifact_p(o_ptr) && (o_ptr->owner == ptr->id))
			delete_object_idx(i, TRUE, TRUE);
	}

	if (!backup) sf_delete(ptr->name); /* a sad day ;( */
	if (!pptr) hash_table[slot] = ptr->next;
	else pptr->next = ptr->next;
	/* Free the memory in the player name */
	free((char *)(ptr->name));
	free((char *)(ptr->accountname));

	dptr = ptr;	/* safe storage */
	ptr = ptr->next;	/* advance */

	/* Free the memory for this struct */
	KILL(dptr, hash_entry);

	/* write back the modified pointers */
	*p_pptr = pptr;
	*p_ptr = ptr;
}

/*
 *  Erase a player by charfile-name - C. Blue
 */
void erase_player_name(char *pname) {
	int slot;
	hash_entry *ptr, *pptr = NULL;

	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		pptr = NULL;
		ptr = hash_table[slot];
		while (ptr) {
			if (!strcmp(ptr->name, pname)) {
				erase_player_hash(slot, &pptr, &ptr);
				continue;
			}
			pptr = ptr;
			ptr = ptr->next;
		}
	}
}

/*
 * List of players about to expire - mikaelh
 */
void checkexpiry(int Ind, int days) {
	int slot, expire;
	hash_entry *ptr;
	time_t now;
	now = time(NULL);
	msg_format(Ind, "\377oPlayers that are about to expire in %d days:", days);
	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		ptr = hash_table[slot];
		while (ptr) {
			expire = CHARACTER_EXPIRY_DAYS * 86400 - now + ptr->laston;
			if (ptr->laston && expire < days * 86400
			    && !(cfg.admins_never_expire && ptr->admin)) {
				if (expire < 86400)
					msg_format(Ind, "\377rPlayer %s (accid %d) will expire in less than a day!", ptr->name, ptr->account);
				else if (expire < 7 * 86400)
					msg_format(Ind, "\377yPlayer %s (accid %d) will expire in %d days!", ptr->name, ptr->account, (expire / 86400));
				else
					msg_format(Ind, "\377gPlayer %s (accid %d) will expire in %d days.", ptr->name, ptr->account, (expire / 86400));
			}
			ptr = ptr->next;
		}
	}
}

/*
 * Warn the player if other characters on his/her account will expire soon
 *  - mikaelh
 * Now also abused to check solo-reking timer. - C. Blue
 */
void account_checkexpiry(int Ind) {
	player_type *p_ptr = Players[Ind];
	int i, expire;
	hash_entry *ptr;
	time_t now;
	struct account acc;
	int *id_list, ids;
	bool success;

#ifdef PLAYERS_NEVER_EXPIRE
	return;
#else
	if (cfg.players_never_expire) return;
#endif

	now = time(NULL);

	success = GetAccount(&acc, p_ptr->accountname, NULL, FALSE, NULL, NULL);
	/* paranoia */
	if (!success) {
		/* uhh.. */
		msg_print(Ind, "Sorry, character expiry check has failed.");
		return;
	}

	ids = player_id_list(&id_list, acc.id);
	for (i = 0; i < ids; i++) {
		ptr = lookup_player(id_list[i]);
		if (p_ptr->id != ptr->id && ptr->laston) {
			expire = CHARACTER_EXPIRY_DAYS * 86400 - now + ptr->laston;
			if (expire < 86400) {
				if (cfg.admins_never_expire && ptr->admin)
					msg_format(Ind, "\374\377y(Your character %s would be removed \377rvery soon\377y if it wasn't an admin.)", ptr->name, expire / 86400);
				else
					msg_format(Ind, "\374\377yYour character %s will be removed \377rvery soon\377y!", ptr->name, expire / 86400);
			} else if (expire < 60 * 86400) {
				if (cfg.admins_never_expire && ptr->admin)
					msg_format(Ind, "\374\377y(Your character %s would be removed in %d days if it wasn't an admin.)", ptr->name, expire / 86400);
				else
					msg_format(Ind, "\374\377yYour character %s will be removed in %d days.", ptr->name, expire / 86400);
			}
		}
	}
	if (ids) C_KILL(id_list, ids, int);
}

/*
 * Add a name to the hash table.
 */
void add_player_name(cptr name, int id, u32b account, byte race, byte class, u32b mode, byte level, byte max_plv, u16b party, byte guild, u32b guild_flags, u16b xorder, time_t laston, byte admin, struct worldpos wpos, char houses, byte winner, byte order) {
	int slot;
	hash_entry *ptr;

	/* Set the entry's id */

	/* Get the destination slot */
	slot = hash_slot(id);

	/* Create a new hash entry struct */
	MAKE(ptr, hash_entry);

	/* Make a copy of the player name in the entry */
	ptr->name = strdup(name);
	ptr->accountname = strdup(lookup_accountname2(account));
	ptr->laston = laston;
	ptr->id = id;
	ptr->account = account;
	ptr->level = level;
	ptr->max_plv = max_plv;
	ptr->party = party;
	ptr->guild = guild;
	ptr->guild_flags = guild_flags;
	ptr->xorder = xorder;
	ptr->race = race;
	ptr->class = class;
	ptr->mode = mode;
	ptr->admin = admin;
	ptr->wpos.wx = wpos.wx;
	ptr->wpos.wy = wpos.wy;
	ptr->wpos.wz = wpos.wz;
	ptr->houses = houses;
	ptr->winner = winner;
	ptr->order = order; /* 0 = existing chars that don't have an order set yet */

	/* Add the rest of the chain to this entry */
	ptr->next = hash_table[slot];

	/* Put this entry in the table */
	hash_table[slot] = ptr;
}

/*
 * Verify a player's data against the hash table. - C. Blue
 */
void verify_player(cptr name, int id, u32b account, byte race, byte class, u32b mode, byte level, u16b party, byte guild, u32b guild_flags, u16b quest, time_t laston, byte admin, struct worldpos wpos, char houses, byte winner, byte order) {
	hash_entry *ptr = lookup_player(id);

	/* For savegame conversion 4.2.0 -> 4.2.2: */
	if (ptr->mode != mode) {
		s_printf("hash_entry: fixing mode of %s.\n", ptr->name);
		ptr->mode = mode;
	}
	/* For savegame conversion 4.3.4 -> 4.3.5: */
	if (ptr->class != class) {
		s_printf("hash_entry: fixing class of %s.\n", ptr->name);
		ptr->class = class;
	}
	if (ptr->guild != guild) {
		s_printf("hash_entry: fixing guild of %s.\n", ptr->name);
		ptr->guild = guild;
	}
	if (ptr->guild_flags != guild_flags) {
		s_printf("hash_entry: fixing guild_flags of %s.\n", ptr->name);
		ptr->guild_flags = guild_flags;
	}
	if (ptr->admin != admin) {
		s_printf("hash_entry: fixing admin state of %s.\n", ptr->name);
		ptr->admin = admin;
	}
	/* added in 4.5.7.1 */
	if (ptr->wpos.wx != wpos.wx || ptr->wpos.wy != wpos.wy || ptr->wpos.wz != wpos.wz) {
		s_printf("hash_entry: fixing wpos (%d,%d,%d) of %s (->%d,%d,%d).\n", ptr->wpos.wx, ptr->wpos.wy, ptr->wpos.wz, ptr->name, wpos.wx, wpos.wy, wpos.wz);
		ptr->wpos.wx = wpos.wx;
		ptr->wpos.wy = wpos.wy;
		ptr->wpos.wz = wpos.wz;
	}
	/* --actually not needed-- */
	if (!ptr->accountname) {
		s_printf("hash_entry: fixing accountname of %s.\n", ptr->name);
		ptr->accountname = strdup(lookup_accountname2(account));
	}
	if (ptr->houses != houses) {
		s_printf("hash_entry: fixing houses of %s.\n", ptr->name);
		ptr->houses = houses;
	}
	if (ptr->winner != winner) {
		s_printf("hash_entry: fixing winner of %s.\n", ptr->name);
		ptr->winner = winner;
	}
	if (order != 100 && ptr->order != order) { /* hack: order == 100 means do not change it */
		s_printf("hash_entry: fixing order of %s.\n", ptr->name);
		ptr->order = order;
	}
}

/*
 * Delete an entry from the table, by ID.
 */
void delete_player_id(int id) {
	int slot;
	hash_entry *ptr, *old_ptr;

	/* Get the destination slot */
	slot = hash_slot(id);

	/* Acquire the pointer to the entry chain */
	ptr = hash_table[slot];

	/* Keep a pointer one step behind this one */
	old_ptr = NULL;

	s_printf("delete_player_id(%d) -> slot %d...", id, slot);

	/* Attempt to find the ID to delete */
	while (ptr) {
		/* Check this one */
		if (ptr->id == id) {
			/* Delete this one from the table */
			if (old_ptr == NULL)
				hash_table[slot] = ptr->next;
			else old_ptr->next = ptr->next;

			/* Free the memory in the player name */
			free((char *)(ptr->name));
			free((char *)(ptr->accountname));

			/* Free the memory for this struct */
			KILL(ptr, hash_entry);

			/* Done */
			s_printf("done.\n");
			return;
		}

		/* Remember this entry */
		old_ptr = ptr;

		/* Advance to next entry in the chain */
		ptr = ptr->next;
	}

	/* Not found */
	s_printf("not found!\n");
	return;
}

/*
 * Delete a player by name.
 *
 * This is useful for fault tolerance, as it is possible to have
 * two entries for one player name, if the server crashes hideously
 * or the machine has a power outage or something.
 */
void delete_player_name(cptr name) {
	int id;

	/* Delete every occurence of this name */
	while ((id = lookup_player_id(name))) {
		/* Delete this one */
		delete_player_id(id);
	}
}

/*
 * Return a list of the player ID's stored in the table.
 *
 * If 'account' is zero, return the list of all player ID's on the server.
 */
int player_id_list(int **list, u32b account) {
	int i, j, len = 0, k = 0, tmp;
	hash_entry *ptr;
	int *llist;
	int max_cpa = MAX_CHARS_PER_ACCOUNT;
#ifdef ALLOW_DED_IDDC_MODE
	max_cpa += MAX_DED_IDDC_CHARS;
#endif
#ifdef ALLOW_DED_PVP_MODE
	max_cpa += MAX_DED_PVP_CHARS;
#endif

	/* Count up the number of valid entries */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire this chain */
		ptr = hash_table[i];

		/* Check this chain */
		while (ptr) {
			/* One more entry */
			if (!account || ptr->account == account)
				len++;

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}
	if (!len) return(0);

	/* Allocate memory for the list */
	C_MAKE((*list), len, int);

	/* Direct pointer to the list */
	llist = *list;

	/* Look again, this time storing ID's */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire this chain */
		ptr = hash_table[i];

		/* Check this chain */
		while (ptr) {
			/* Store this ID */
			if (!account || ptr->account == account) {
				llist[k++] = ptr->id;

				/* Insertion sort for account specific lists */
				if (account) {
					j = k - 1;
					while (j > 0 && llist[j - 1] > llist[j]) {
						tmp = llist[j - 1];
						llist[j - 1] = llist[j];
						llist[j] = tmp;
						j--;
					}
				}
			}

			/* Next entry in chain */
			ptr = ptr->next;
		}
	}

	/* Limit number of characters per account - C. Blue */
	if (account && len > max_cpa) len = max_cpa;

	return(len);
}

/* Change account that a player belong to.
   'Ind' is only for giving feedback msg. May be 0 to disable. */
void player_change_account(int Ind, int id, u32b new_account) {
	int i;
	hash_entry *ptr;
	cptr accname;

	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		ptr = hash_table[i];
		while (ptr) {
			if (ptr->id == id) {
				accname = lookup_accountname2(new_account);
				if (!accname[0]) {
					s_printf("player_change_account: '%s' from '%s' unchanged due to non-existant account #%d.\n", ptr->name, ptr->accountname, new_account);
					if (Ind) msg_format(Ind, "player_change_account: '%s' from '%s' unchanged due to non-existant account #%d.", ptr->name, ptr->accountname, new_account);
				} else {
					s_printf("player_change_account: '%s' moved from '%s' to '%s'.\n", ptr->name, ptr->accountname, accname);
					if (Ind) msg_format(Ind, "player_change_account: '%s' moved from '%s' to '%s'.", ptr->name, ptr->accountname, accname);
				}
				ptr->accountname = strdup(accname);
				ptr->account = new_account;
				return;
			}
			ptr = ptr->next;
		}
	}
}

/*
 * Set/reset 'pk' mode, which allows a player to kill the others
 *
 * These functions should be common with hostilityes in the future. -Jir-
 */
void set_pkill(int Ind, int delay) {
	player_type *p_ptr = Players[Ind];
	//bool admin = is_admin(p_ptr);

	if (cfg.use_pk_rules != PK_RULES_DECLARE) {
		msg_print(Ind, "\377o/pkill is not available on this server. Be pacifist.");
		p_ptr->tim_pkill = 0;
		p_ptr->pkill = 0;
		return;
	}
#ifdef KURZEL_PK
	else {
		if (is_admin(p_ptr)) {
			p_ptr->pkill ^= PKILL_SET; //Flag TOGGLE
			if (p_ptr->pkill & PKILL_SET) msg_print(Ind, "\377RYou may now kill (and be killed by) other PK enabled players.");
			else msg_print(Ind, "\377RPK Mode toggled OFF. (Admin ONLY)");
		} else {
			if (!(p_ptr->pkill & PKILL_SET)) {
				p_ptr->pkill |= PKILL_SET; //Flag ON
				msg_print(Ind, "\377RYou may now kill (and be killed by) other PK enabled players.");
			} else {
				msg_print(Ind, "\377RThere is no going back! (Party members are pacifist, however.)");
			}
		}
		return;
	}
#endif

//	p_ptr->tim_pkill= admin ? 10 : 200;	/* so many turns */
	p_ptr->tim_pkill= delay;
	p_ptr->pkill ^= PKILL_SET; /* Toggle value */
	if (p_ptr->pkill & PKILL_SET) {
		msg_print(Ind, "\377rYou wish to kill other players");
		p_ptr->pkill |= PKILL_KILLABLE;
	} else {
		hostile_type *t_host;
		msg_print(Ind, "\377gYou do not wish to kill other players");
		p_ptr->pkill &= ~PKILL_KILLER;
		/* Remove all hostilities */
		while (p_ptr->hostile) {
			t_host = p_ptr->hostile;
			p_ptr->hostile = t_host->next;
			KILL(t_host, hostile_type);
		}
	}
}

#if 0	// under construction
/*
 * Set/reset 'pilot' mode, which allows a player to follow another player
 * for comfort in party diving.
 *
 * These functions should be common with hostilityes in the future. -Jir-
 */
bool pilot_set(int Ind, cptr name) {
	player_type *p_ptr = Players[Ind], *q_ptr;
	hostile_type *h_ptr, *i_ptr;
	int i;
	cptr p,q;

	/* Check for silliness */
	if (!name) {
		msg_print(Ind, "Usage: /pilot foobar");
		return(FALSE);
	}

	i = name_lookup_loose(Ind, name, TRUE, TRUE, FALSE);

	if (!i) return(FALSE);

	/* Check for another silliness */
	if (i == Ind) {
		/* Message */
		msg_print(Ind, "You cannot follow yourself.");

		return(FALSE);
	}

	/* Forge name */
	if (i > 0) {
		q = Players[i]->name;
	}


	if (i > 0) {
		q_ptr = Players[i];

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = q_ptr->id;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->ignore;
		p_ptr->ignore = h_ptr;

		/* Message */
		msg_format(Ind, "You aren't hearing %s any more.", q_ptr->name);

		/* Success */
		return(TRUE);
	} else {
		/* Tweak - inverse i once more */
		i = 0 - i;

		/* Create a new hostility node */
		MAKE(h_ptr, hostile_type);

		/* Set ID in node */
		h_ptr->id = 0 - i;

		/* Put this node at the beginning of the list */
		h_ptr->next = p_ptr->ignore;
		p_ptr->ignore = h_ptr;

		/* Message */
		msg_format(Ind, "You aren't hearing party '%s' any more.", parties[i].name);

		/* Success */
		return(TRUE);
	}
}
#endif	// 0

void strip_true_arts_from_hashed_players() {
	int slot, i, j = 0;
	hash_entry *ptr;
	object_type *o_ptr;

	s_printf("Starting player true artifact stripping\n");
	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		j++;
//		if (j > 5) break;/*only check for 5 players right now */

		ptr = hash_table[slot];
		if (!ptr) continue;
		s_printf("Stripping player: %s\n", ptr->name);
		/* Wipe Artifacts (s)he had  -C. Blue */
		for (i = 0; i < o_max; i++) {
			o_ptr = &o_list[i];
			if (true_artifact_p(o_ptr) &&
			    (o_ptr->owner == ptr->id) && // <- why?
			    !winner_artifact_p(o_ptr))
#if 1 /* set 0 to not change cur_num */
				delete_object_idx(i, TRUE, TRUE);
#else
				excise_object_idx(o_idx);
				WIPE(o_ptr, object_type);
#endif
		}
	}
	s_printf("Finished true artifact stripping for %d players.\n", j);
}

void account_change_password(int Ind, char *old_pass, char *new_pass) {
	player_type *p_ptr = Players[Ind];
	struct account acc;

	/* Read from disk */
	if (!GetAccount(&acc, p_ptr->accountname, old_pass, FALSE, NULL, NULL)) {
		msg_print(Ind, "\377RWrong password!");
		return;
	}

	if (strlen(new_pass) < PASSWORD_MIN_LEN) {
		msg_format(Ind, "\377RPassword length must be at least %d.", PASSWORD_MIN_LEN);
		return;
	}

	if (strlen(new_pass) > ACCFILE_PASSWD_LEN) {
		msg_format(Ind, "\377RPassword length must be at most %d.", ACCFILE_PASSWD_LEN);
		return;
	}

	/* Change password */
	strcpy(acc.pass, t_crypt(new_pass, acc.name));

	/* Write account to disk */
	bool success = WriteAccount(&acc, FALSE);

	/* Prevent the password from leaking */
	memset(acc.pass, 0, sizeof(acc.pass));

	/* Check for failure */
	if (!success) {
		msg_print(Ind, "\377RFailed to write to account file!");
		return;
	}

	/* Success */
	s_printf("Changed password for account %s.\n", acc.name);
	msg_print(Ind, "\377GPassword successfully changed.");
}

void backup_acclists(void) {
	FILE *fp;
	char buf[MAX_PATH_LENGTH], buf2[MAX_PATH_LENGTH];
	hash_entry *ptr;
	int del, i;
	struct account acc;

	s_printf("Backing up all accounts...\n");
	/* create folder lib/save/estate if not existing */
	path_build(buf2, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "estate");
	path_build(buf, MAX_PATH_LENGTH, buf2, "_accounts_");
	if ((fp = fopen(buf, "wb")) == NULL) {
		s_printf("  error: cannot open file '%s'.\nfailed.\n", buf);
		return;
	}
	/* begin with a version tag */
	fprintf(fp, "%s\n", "v1");

	/* Search in each array slot */
	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire pointer to this chain */
		ptr = hash_table[i];

		/* Check all entries in this chain */
		while (ptr) {
			/* Check this name */
			if (GetAccountID(&acc, ptr->account, FALSE)) {
				/* back him up */
				fprintf(fp, "%d\n", (int)strlen(ptr->name));
				(void)fwrite(ptr->name, sizeof(char), strlen(ptr->name), fp);
#if 0
				(void)fprintf(fp, "%lu%d%u%c%hu%c%hd%c%c%c",
				    ptr->laston, ptr->id, ptr->account,
				    ptr->level, ptr->party, ptr->guild,
				    ptr->xorder, ptr->race, ptr->class, ptr->mode);
 #ifdef AUCTION_SYSTEM
				fprintf(fp, "%d%d", ptr->au, ptr->balance);
 #endif
#else
				(void)fwrite(ptr, sizeof(hash_entry), 1, fp);
#endif
			} else {
				s_printf("Lost player: %s\n", ptr->name);
#if 0 /* del might not always be initialized! */
				del = ptr->id;
			}

			/* Next entry in chain */
			ptr = ptr->next;
			delete_player_id(del);
#else /* isn't it supposed to be this way instead?: */
				del = ptr->id;
				delete_player_id(del);
			}

			/* Next entry in chain */
			ptr = ptr->next;
#endif
		}
	}

	s_printf("done.\n");
	fclose(fp);
}

void restore_acclists(void) {
	FILE *fp;
	char buf[MAX_PATH_LENGTH], buf2[MAX_PATH_LENGTH], tmp[MAX_CHARS];
	char name_forge[MAX_CHARS];
	hash_entry forge, *ptr = &forge;
	forge.name = name_forge;
	int name_len, r;

	s_printf("Restoring accounts...\n");

	path_build(buf2, MAX_PATH_LENGTH, ANGBAND_DIR_SAVE, "estate");
	path_build(buf, MAX_PATH_LENGTH, buf2, "_accounts_");
	if ((fp = fopen(buf, "rb")) == NULL) {
		s_printf("  error: cannot open file '%s'.\nfailed.\n", buf);
		return;
	}
	/* begin with a version tag */
	r = fscanf(fp, "%s\n", tmp);
	if (r == EOF || r == 0) {
		s_printf("  error: failed to read version tag.\n");
		fclose(fp);
		return;
	}

	while (!feof(fp)) {
		r = fscanf(fp, "%d\n", &name_len);
		if (r == EOF || r == 0) break;
		r = fread(name_forge, sizeof(char), name_len, fp);
		if (r == 0) break;
		name_forge[name_len] = '\0';
#if 0
		r = fscanf(fp, "%lu%d%u%c%hu%c%hd%c%c%c",
		    &ptr->laston, &ptr->id, &ptr->account,
		    &ptr->level, &ptr->party, &ptr->guild,
		    &ptr->xorder, &ptr->race, &ptr->class, &ptr->mode);
		if (r == EOF || r == 0) break;
 #ifdef AUCTION_SYSTEM
		r = fscanf(fp, "%d%d", &ptr->au, &ptr->balance);
		if (r == EOF || r == 0) break;
 #endif
#else
		r = fread(ptr, sizeof(hash_entry), 1, fp);
		if (r == 0) break;
		ptr->name = NULL;//just to be clean
#endif

		//s_printf(" '%s', id %d, acc %d, lev %d, race %d, class %d, mode %d.\n", ptr->name, ptr->id, ptr->account, ptr->level, ptr->race, ptr->class, ptr->mode);

		if (!lookup_player_name(ptr->id)) { /* paranoia: if the 'server' file was just deleted then there can be no names */
			time_t ttime;
			//s_printf("  adding: '%s' (id %d, acc %d)\n", ptr->name, ptr->id, ptr->account);
			/* Add backed-up entry again */
			add_player_name(name_forge, ptr->id, ptr->account, ptr->race, ptr->class, ptr->mode, ptr->level, ptr->max_plv, 0, 0, 0, 0, time(&ttime), ptr->admin, ptr->wpos, ptr->houses, ptr->winner, 0);
		} else s_printf("  already exists: '%s' (id %d, acc %d)\n", name_forge, ptr->id, ptr->account);
	}

	s_printf("done.\n");
	fclose(fp);
}

/* search for any members of a guild, and set guild's mode to that first member found's mode */
void fix_lost_guild_mode(int g_id) {
	int slot;
	hash_entry *ptr;

	for (slot = 0; slot < NUM_HASH_ENTRIES; slot++) {
		ptr = hash_table[slot];
		while (ptr) {
			if (ptr->guild && ptr->guild == g_id) {
				guilds[g_id].cmode = ptr->mode;
				s_printf("Guild '%s' (%d): Mode has been fixed to %d.\n", guilds[g_id].name, g_id, guilds[g_id].cmode);
				return;
			}
			ptr = ptr->next;
		}

	}
	/* paranoia: something went really wrong, such as savefile rollback or divine intervention */
	if (slot == NUM_HASH_ENTRIES) {
		guilds[g_id].members = 0;
		s_printf("Guild '%s' (%d): Has been erased!\n", guilds[g_id].name, g_id);
	}
}

//ACC_HOUSE_LIMIT:
char acc_sum_houses(struct account *acc, bool quiet) {
	int i;
	char j = 0;
	int *id_list, ids;
	hash_entry *ptr;

	ids = player_id_list(&id_list, acc->id);
	for (i = 0; i < ids; i++) {
		if (!(ptr = lookup_player(id_list[i]))) {
			s_printf("ACC_SUM_HOUSES_ERROR: cannot lookup_player id %d.\n", id_list[i]);
			continue;
		}
		if (!quiet) s_printf("ACC_SUM_HOUSES: character %s adds %d houses.\n", ptr->name, ptr->houses);
		j += ptr->houses;
	}

	if (ids) C_KILL(id_list, ids, int);
	return(j);
}

/* Initialize character ordering for the whole account database,
   for custom character order in account overview screen after login:
   All so far 'unsorted' characters have order 0. Due to the divide&conquer nature of ang_sort (quicksort),
   this means that the result will be partitioned and mixed, so the original ('natural') order is not preserved.
   This is no biggie, but for extra player comfort, let's imprint the original order here in a first-time conversion:
   (Ind is just for printing a result message, can be 0.) */
void init_character_ordering(int Ind) {
	int i, j, processed = 0, imprinted = 0, imprinted_accounts = 0;
	int ids, *id_list;
	hash_entry *ptr, *ptr2;

	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire this chain */
		ptr = hash_table[i];
		/* Check this chain */
		while (ptr) {
			processed++;
			/* A character was found without order -> imprint the whole account it belongs to */
			if (!ptr->order) {
				imprinted_accounts++;
				/* Treat all characters of this account */
				ids = player_id_list(&id_list, ptr->account);
				for (j = 0; j < ids; j++) {
					//if (lookup_player_order(id_list[j])) continue; /* Paranoia: Skip any that might already have been ordered */
					/* Imprint all character on this account, even if some were already imprinted, to keep the sequence flawless */
					imprinted++;
					ptr2 = lookup_player(id_list[j]);
					ptr2->order = j + 1; /* Natural order ;) */
				}
				if (ids) C_KILL(id_list, ids, int);
			}
			/* Next entry in chain */
			ptr = ptr->next;
		}
	}
	s_printf("INIT_CHARACTER_ORDERING: Processed %d; imprinted %d; imprinted accounts %d.\n", processed, imprinted, imprinted_accounts);
	if (Ind) msg_format(Ind, "Processed chars %d; imprinted chars %d; imprinted accounts %d.", processed, imprinted, imprinted_accounts);
}
/* Like init_character_ordering(), but only for one specified account.
   (Ind is just for printing a result message, can be 0.) */
void init_account_order(int Ind, s32b acc_id) {
	int i, j, processed = 0, imprinted = 0;
	int ids, *id_list;
	hash_entry *ptr, *ptr2;
	char acc_name[ACCNAME_LEN];
	bool found = FALSE;

	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire this chain */
		ptr = hash_table[i];
		/* Check this chain */
		while (ptr) {
			if (ptr->account != acc_id) {
				ptr = ptr->next;
				continue;
			}
			if (!found) {
				strcpy(acc_name, ptr->accountname);
				found = TRUE;
			}
			/* Check this character */
			processed++;
			/* A character was found without order -> imprint the whole account it belongs to */
			if (!ptr->order) {
				/* Treat all characters of this account */
				ids = player_id_list(&id_list, ptr->account);
				for (j = 0; j < ids; j++) {
					//if (lookup_player_order(id_list[j])) continue; /* Paranoia: Skip any that might already have been ordered */
					/* Imprint all character on this account, even if some were already imprinted, to keep the sequence flawless */
					imprinted++;
					ptr2 = lookup_player(id_list[j]);
					ptr2->order = j + 1; /* Natural order ;) */
				}
				if (ids) C_KILL(id_list, ids, int);
				i = NUM_HASH_ENTRIES;
				break;
			}
			/* Next entry in chain */
			ptr = ptr->next;
		}
	}
	s_printf("INIT_ACCOUNT_ORDER: '%s', processed %d; imprinted %d.\n", acc_name, processed, imprinted);
	if (Ind) msg_format(Ind, "Account '%s': Processed chars %d; imprinted chars %d.", acc_name, processed, imprinted);
}
/* Resets all character ordering to zero aka unordered.
   While this is the 'natural' order, note that in the Character Overview they will still show up in
   a different order than the one they were created in, due to the divide&conquer nature of ang_sort
   that is run over them.
   (Ind is just for printing a result message, can be 0.) */
void zero_character_ordering(int Ind) {
	int i, processed = 0, imprinted = 0;
	hash_entry *ptr;

	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire this chain */
		ptr = hash_table[i];
		/* Check this chain */
		while (ptr) {
			processed++;
			if (ptr->order) {
				/* A character was found without order -> imprint the whole account it belongs to */
				imprinted++;
				ptr->order = 0;
			}
			/* Next entry in chain */
			ptr = ptr->next;
		}
	}
	s_printf("INIT_CHARACTER_ORDERING: Processed %d; imprinted %d.\n", processed, imprinted);
	if (Ind) msg_format(Ind, "Processed chars %d; imprinted chars %d.", processed, imprinted);
}
/* Displays order weights for all characters of an account.
   (Ind is just for printing a result message, can be 0.) */
void show_account_order(int Ind, s32b acc_id) {
	int i, j, processed = 0;
	int ids, *id_list;
	hash_entry *ptr, *ptr2;
	char acc_name[ACCNAME_LEN];

	for (i = 0; i < NUM_HASH_ENTRIES; i++) {
		/* Acquire this chain */
		ptr = hash_table[i];
		/* Check this chain */
		while (ptr) {
			if (ptr->account != acc_id) {
				ptr = ptr->next;
				continue;
			}
			strcpy(acc_name, ptr->accountname);
			s_printf("SHOW_ACCOUNT_ORDER: '%s':\n", acc_name);
			if (Ind) msg_format(Ind, "Account '%s':", acc_name);
			/* Check all characters of this account */
			ids = player_id_list(&id_list, ptr->account);
			for (j = 0; j < ids; j++) {
				processed++;
				ptr2 = lookup_player(id_list[j]);
				s_printf("  '%s':%d\n", lookup_player_name(id_list[j]), ptr2->order);
				if (Ind) msg_format(Ind, "  '%s':%d\n", lookup_player_name(id_list[j]), ptr2->order);
			}
			if (ids) C_KILL(id_list, ids, int);
			i = NUM_HASH_ENTRIES;
			break;
		}
	}
	s_printf("SHOW_ACCOUNT_ORDER: '%s', processed %d.\n", acc_name, processed);
	if (Ind) msg_format(Ind, "Account '%s': Processed chars %d.", acc_name, processed);
}
