#include <stdint.h>

/* account flags */
#define ACC_TRIAL	0x00000001U	/* Account is awaiting validation */
#define ACC_ADMIN	0x00000002U	/* Account members are admins */
#define ACC_MULTI	0x00000004U	/* Simultaneous play */
#define ACC_NOSCORE	0x00000008U	/* No scoring allowed */
#define ACC_RESTRICTED	0x00000010U	/* is restricted (ie after cheezing) */
#define ACC_VRESTRICTED	0x00000020U	/* is restricted (ie after cheating) */
#define ACC_PRIVILEGED	0x00000040U	/* has privileged powers (ie for running quests) */
#define ACC_VPRIVILEGED	0x00000080U	/* has privileged powers (ie for running quests) */
#define ACC_PVP		0x00000100U	/* may kill other players */
#define ACC_NOPVP	0x00000200U	/* is not able to kill other players */
#define ACC_ANOPVP	0x00000400U	/* cannot kill other players; gets punished on trying */
#define ACC_GREETED	0x00000800U	/* */
#define ACC_QUIET	0x00001000U	/* may not chat or emote in public */
#define ACC_VQUIET	0x00002000U	/* may not chat or emote, be it public or private */
#define ACC_BANNED	0x00004000U	/* account is temporarily suspended */
#define ACC_DELD	0x00008000U	/* Delete account/members */
#define ACC_WARN_REST	0x80000000U	/* Received a one-time warning about resting */

#define u32b uint32_t
#define s32b int32_t

struct account{
	u32b id;	/* account id */
	u32b flags;	/* account flags */
	char name[30];	/* login - [ACCFILE_NAME_LEN] */
	char name_normalised[30];  /* login name, but in a simplified form, used for preventing creation of too similar account names - [ACCFILE_PASSWD_LEN] */
	char pass[20];	/* some crypts are not 13 - [ACCFILE_PASSWD_LEN] */
#ifdef ACC32
	int acc_laston, acc_laston_real;
#else
	time_t acc_laston, acc_laston_real;      /* last time this account logged on (for expiry check) */
#endif
	s32b cheeze;	/* value in gold of cheezed goods or money */
	s32b cheeze_self;	/* value in gold of cheezed goods or money to own characters */
	char deed_event;	/* receive a deed for a global event participation? */
	char deed_achievement;	/* receive a deed for a (currently PvP) achievement? */
	s32b guild_id;	/* auto-rejoin its guild after a char perma-died */
	u32b guild_dna;	/* auto-rejoin its guild after a char perma-died */

	char houses;	/* for account-wide house limit (installed after increasing the # of generic character slots above 8) */
	unsigned char runtime;	/* increments on each server (re)start simply, to track server instance */

	char hostname[20];	/* last used hostname -- [HOSTNAME_LEN] */
	char addr[80];		/* last used IP address -- [MAX_CHARS] */
	char reply_name[20];	/* p_ptr->reply_name [NAME_LEN] */
	char email[80];		/* idea: if player wants to, he can submit an email for notification if a character or the account is about to expire */

	/* for future use */
	unsigned char unused1;
	unsigned char unused2;
	unsigned char unused3;
};
