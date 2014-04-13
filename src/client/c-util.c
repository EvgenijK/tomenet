/* $Id$ */
/* File: c-util.c */
/* Client-side utility stuff */

#include "angband.h"

#include <sys/time.h>

#define ENABLE_SUBWINDOW_MENU /* allow =f menu function for setting fonts/visibility of term windows */
#ifdef ENABLE_SUBWINDOW_MENU
 #include <dirent.h>
#endif

#define MACRO_USE_CMD	0x01
#define MACRO_USE_STD	0x02
#define MACRO_USE_HYB	0x04

#define NR_OPTIONS_SHOWN	8 /*was 32 when there were 32 window_flag_desc[]*/

/* Have the Macro Wizard generate target code in
   the form *tXXX- instead of XXX*t? - C. Blue */
#define MACRO_WIZARD_SMART_TARGET

/* This should be extended onto all top-line clearing/message prompting during macro execution,
   to avoid erasing %:bla lines coming from the macro, by overwriting them with useless prompts: */
//#define DONT_CLEAR_TOPLINE_IF_AVOIDABLE -- turned into an option instead: c_cfg.keep_topline



#ifdef SET_UID
# ifndef HAS_USLEEP
/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
int usleep(huge microSeconds) {
	struct timeval		Timer;
	int			nfds = 0;

#ifdef FD_SET
	fd_set		*no_fds = NULL;
#else
	int			*no_fds = NULL;
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
		if (errno != EINTR) return -1;
	}

	/* Success */
	return 0;
}
# endif /* HAS_USLEEP */
#endif /* SET_UID */

#ifdef WIN32
int usleep(long microSeconds)
{
	Sleep(microSeconds / 1000); /* meassured in milliseconds not microseconds*/
	return(0);
}
#endif /* WIN32 */



static int MACRO_WAIT = 96;

static void ascii_to_text(char *buf, cptr str);

static bool after_macro = FALSE;
bool parse_macro = FALSE; /* are we inside the process of executing a macro */
int macro_missing_item = 0;
static bool parse_under = FALSE;
static bool parse_slash = FALSE;
static bool strip_chars = FALSE;

static bool flush_later = FALSE;

static byte macro__use[256];

static bool was_chat_buffer = FALSE;
static bool was_all_buffer = FALSE;

static char octify(uint i)
{
	return (hexsym[i%8]);
}

static char hexify(uint i)
{
	return (hexsym[i%16]);
}

void move_cursor(int row, int col)
{
	Term_gotoxy(col, row);
}

void flush(void)
{
	flush_later = TRUE;
}

void flush_now(void)
{
	/* Clear various flags */
	flush_later = FALSE;

	/* Cancel "macro" info */
//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
	restore_prompt();
//#endif
	parse_macro = after_macro = FALSE;

	/* Cancel "sequence" info */
	parse_under = parse_slash = FALSE;

	/* Cancel "strip" mode */
	strip_chars = FALSE;

	/* Forgot old keypresses */
	Term_flush();
}

/*
 * Check for possibly pending macros
 */
static int macro_maybe(cptr buf, int n)
{
	int i;

	/* Scan the macros */
	for (i = n; i < macro__num; i++)
	{
		/* Skip inactive macros */
		if (macro__hyb[i] && (shopping || inkey_msg)) continue;
		if (macro__cmd[i] && (shopping || !inkey_flag || inkey_msg)) continue;

		/* Check for "prefix" */
		if (prefix(macro__pat[i], buf))
		{
			/* Ignore complete macros */
			if (!streq(macro__pat[i], buf)) return (i);
		}
	}

	/* No matches */
	return (-1);
}


/*
 * Find the longest completed macro
 */
static int macro_ready(cptr buf)
{
	int i, t, n = -1, s = -1;

	/* Scan the macros */
	for (i = 0; i < macro__num; i++)
	{
		/* Skip inactive macros */
		if (macro__cmd[i] && (shopping || inkey_msg || !inkey_flag)) continue;
		if (macro__hyb[i] && (shopping || inkey_msg)) continue;

		/* Check for "prefix" */
		if (!prefix(buf, macro__pat[i])) continue;

		/* Check the length of this entry */
		t = strlen(macro__pat[i]);

		/* Find the "longest" entry */
		if ((n >= 0) && (s > t)) continue;

		/* Track the entry */
		n = i;
		s = t;
	}

	/* Return the result */
	return (n);
}


/*
 * Hack -- add a macro definition (or redefinition).
 *
 * If "cmd_flag" is set then this macro is only active when
 * the user is being asked for a command (see below).
 */
void macro_add(cptr pat, cptr act, bool cmd_flag, bool hyb_flag)
{
        int n;

        /* Paranoia -- require data */
        if (!pat || !act) return;


        /* Look for a re-usable slot */
        for (n = 0; n < macro__num; n++)
        {
                /* Notice macro redefinition */
                if (streq(macro__pat[n], pat))
                {
                        /* Free the old macro action */
                        string_free(macro__act[n]);

                        /* Save the macro action */
                        macro__act[n] = string_make(act);

                        /* Save the "cmd_flag" */
                        macro__cmd[n] = cmd_flag;

                        /* Save the hybrid flag */
                        macro__hyb[n] = hyb_flag;

			/* Update the "trigger" char - mikaelh */
			if (hyb_flag) macro__use[(byte)(pat[0])] |= MACRO_USE_HYB;
			else if (cmd_flag) macro__use[(byte)(pat[0])] |= MACRO_USE_CMD;
			else macro__use[(byte)(pat[0])] |= MACRO_USE_STD;

                        /* All done */
                        return;
                }
        }


        /* Save the pattern */
        macro__pat[macro__num] = string_make(pat);

        /* Save the macro action */
        macro__act[macro__num] = string_make(act);

        /* Save the "cmd_flag" */
        macro__cmd[macro__num] = cmd_flag;

        /* Save the "hybrid flag" */
        macro__hyb[macro__num] = hyb_flag;

        /* One more macro */
        macro__num++;


        /* Hack -- Note the "trigger" char */
        if (hyb_flag) macro__use[(byte)(pat[0])] |= MACRO_USE_HYB;
        else if (cmd_flag) macro__use[(byte)(pat[0])] |= MACRO_USE_CMD;
	macro__use[(byte)(pat[0])] |= MACRO_USE_STD;

}

/*
 * Try to delete a macro - mikaelh
 */
bool macro_del(cptr pat)
{
	int i, num = -1;

	/* Find the macro */
	for (i = 0; i < macro__num; i++)
	{
		if (streq(macro__pat[i], pat))
		{
			num = i;
			break;
		}
	}

	if (num == -1) return FALSE;

	/* Free it */
	string_free(macro__pat[num]);
	macro__pat[num] = NULL;
	string_free(macro__act[num]);
	macro__act[num] = NULL;

	/* Remove it from every array */
	for (i = num + 1; i < macro__num; i++)
	{
		macro__pat[i - 1] = macro__pat[i];
		macro__act[i - 1] = macro__act[i];
		macro__cmd[i - 1] = macro__cmd[i];
		macro__hyb[i - 1] = macro__hyb[i];
	}

#if 0 /* this can actually disable multiple keys if their macro patterns begin with the same byte - mikaelh */
	macro__use[(byte)(pat[0])] = 0;
#endif

	macro__num--;

	return TRUE;
}

/* Returns the difference between two timevals in milliseconds */
static int diff_ms(struct timeval *begin, struct timeval *end) {
	int diff;

	diff = (end->tv_sec - begin->tv_sec) * 1000;
	diff += (end->tv_usec - begin->tv_usec) / 1000;

	return diff;
}

static void sync_sleep(int milliseconds)
{
	static char animation[4] = { '-', '\\', '|', '/' };
	int result, net_fd;
	struct timeval begin, now;
	int time_spent;
	char ch;

#ifdef WINDOWS
	/* Use the multimedia timer function */
	DWORD systime_ms = timeGetTime();
	begin.tv_sec = systime_ms / 1000;
	begin.tv_usec = (systime_ms % 1000) * 1000;
#else
	gettimeofday(&begin, NULL);
#endif
	net_fd = Net_fd();

	/* HACK - Create a new key queue so we can receive fresh key presses */
	Term->keys_old = Term->keys;

	MAKE(Term->keys, key_queue);
	C_MAKE(Term->keys->queue, Term->key_size_orig, char);

	Term->keys->size = Term->key_size_orig;

	while (TRUE) {
		/* Check for fresh key presses */
		while (Term_inkey(&ch, FALSE, TRUE) == 0) {
			if (ch == ESCAPE) {
				/* Forget key presses */
				Term->keys->head = 0;
				Term->keys->tail = 0;
				Term->keys->length = 0;

				if (Term->keys_old) {
					/* Destroy the old queue */
					C_KILL(Term->keys_old->queue, Term->keys_old->size, char);
					KILL(Term->keys_old, key_queue);
				}

				/* Erase the spinner */
				Term_erase(Term->wid - 1, 0, 1);

				/* Abort */
				return;
			} else {
				if (Term->keys_old) {
					/* Add it to the old queue */
					Term_keypress_aux(Term->keys_old, ch);
				}
			}
		}

#ifdef WINDOWS
		/* Use the multimedia timer function */
		DWORD systime_ms = timeGetTime();
		now.tv_sec = systime_ms / 1000;
		now.tv_usec = (systime_ms % 1000) * 1000;
#else
		gettimeofday(&now, NULL);
#endif

		/* Check if we have waited long enough */
		time_spent = diff_ms(&begin, &now);
		if (time_spent >= milliseconds) {
			if (Term->keys_old) {
				/* Destroy the temporary key queue */
				C_KILL(Term->keys->queue, Term->keys->size, char);
				KILL(Term->keys, key_queue);

				/* Restore the old queue */
				Term->keys = Term->keys_old;
				Term->keys_old = NULL;
			}

			/* Erase the spinner */
			Term_erase(Term->wid - 1, 0, 1);
			return;
		}

		/* Do a little animation in the upper right corner */
		Term_putch(Term->wid - 1, 0, TERM_WHITE, animation[time_spent / 100 % 4]);

		/* Flush output - maintain flickering/multi-hued characters */
		do_flicker();

		/* Update our timer and if neccecary send a keepalive packet
		 */
		update_ticks();
		if(!c_quit) {
			do_keepalive();
			do_ping();
		}

		/* Flush the network output buffer */
		Net_flush();

		/* Wait for .001 sec, or until there is net input */
		SetTimeout(0, 1000);

		/* Update the screen */
		Term_fresh();

		if(c_quit) {
			usleep(1000);
			continue;
		}

		/* Parse net input if we got any */
		if (SocketReadable(net_fd))
		{
			if ((result = Net_input()) == -1)
			{
				quit("Net_input failed.");
			}
		}

		/* Redraw windows if necessary */
		if (p_ptr->window)
		{
			window_stuff();
		}
	}
}

/*
 * Helper function called only from "inkey()"
 *
 * This function does most of the "macro" processing.
 *
 * We use the "Term_key_push()" function to handle "failed" macros,
 * as well as "extra" keys read in while choosing a macro, and the
 * actual action for the macro.
 *
 * Embedded macros are illegal, although "clever" use of special
 * control chars may bypass this restriction.  Be very careful.
 *
 * The user only gets 500 (1+2+...+29+30) milliseconds for the macro.
 *
 * Note the annoying special processing to "correctly" handle the
 * special "control-backslash" codes following a "control-underscore"
 * macro sequence.  See "main-x11.c" and "main-xaw.c" for details.
 *
 *
 * Note that we NEVER wait for a keypress without also checking the network
 * for incoming packets.  This stops annoying "timeouts" and also lets
 * the screen get redrawn if need be while we are waiting for a key. --KLJ--
 */
static char inkey_aux(void)
{
	int	k = 0, n, p = 0, w = 0;
	char	ch = 0;
	cptr	pat, act;
	char	buf[1024];
	char	buf_atoi[3];
	bool	inkey_max_line_set;
	int net_fd;

	inkey_max_line_set = inkey_max_line;

	/* Acquire and save maximum file descriptor */
	net_fd = Net_fd();

	/* If no network yet, just wait for a keypress */
	if (net_fd == -1) {
		/* Wait for a keypress */
                (void)(Term_inkey(&ch, TRUE, TRUE));
	} else {
		/* Wait for keypress, while also checking for net input */
		do {
			int result;

			/* Flush output - maintain flickering/multi-hued characters */
			do_flicker();

			/* Animate things: Minimap '@' denoting our own position */
			if (minimap_posx != -1 && screen_icky) {
				if ((ticks % 6) < 3)
					Term_draw(minimap_posx, minimap_posy, TERM_WHITE, '@');
				else
					Term_draw(minimap_posx, minimap_posy, minimap_attr, minimap_char);
			}

			/* Look for a keypress */
			(void)(Term_inkey(&ch, FALSE, TRUE));

			/* If we got a key, break */
			if (ch) break;

			/* If we received a 'max_line' value from the net,
			   break if inkey_max_line flag was set */
			if (inkey_max_line != inkey_max_line_set) {
				/* hack: */
				ch = 1;
				break;
			}

			/* Update our timer and if neccecary send a keepalive packet
			 */
			update_ticks();
			if(!c_quit) {
				do_keepalive();
				do_ping();
			}

			/* Flush the network output buffer */
			Net_flush();

			/* Wait for .001 sec, or until there is net input */
//			SetTimeout(0, 1000);

			/* Wait according to fps - mikaelh */
			SetTimeout(0, next_frame());

			/* Update the screen */
			Term_fresh();

			if(c_quit) {
				usleep(1000);
				continue;
			}

			/* Parse net input if we got any */
			if (SocketReadable(net_fd)) {
				if ((result = Net_input()) == -1) quit("Net_input failed.");
			}

			/* Hack - Leave a store */
			if (shopping && leave_store) return ESCAPE;

			/* Are we supposed to abort a 'special input request'?
			   (Ie outside game situation has changed, eg left store.) */
			if (request_pending && request_abort) {
				request_abort = FALSE;
				return ESCAPE;
			}

			/* Redraw windows if necessary */
			if (p_ptr->window) window_stuff();
		} while (!ch);
	}


	/* End of internal macro */
	if (ch == 29) {
//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
		restore_prompt();
//#endif
		parse_macro = FALSE;
	}


	/* Do not check "ascii 28" */
	if (ch == 28) return (ch);


	/* Do not check "ascii 29" */
	if (ch == 29) return (ch);

	if (parse_macro && (ch == MACRO_WAIT)) {
		buf_atoi[0] = '0';
		buf_atoi[1] = '0';
		buf_atoi[2] = '\0';
		(void)(Term_inkey(&ch, FALSE, TRUE));
		if (ch) buf_atoi[0] = ch;
		(void)(Term_inkey(&ch, FALSE, TRUE));
		if (ch) buf_atoi[1] = ch;
		w = atoi(buf_atoi);
		sync_sleep(w * 100L); /* w 1/10th seconds */
		ch = 0;
		w = 0;
	}

	/* Do not check macro actions */
	if (parse_macro) {
		/* Check for problems with missing items the macro was supposed to use - C. Blue */
		if (macro_missing_item == 1) {
			/* Check for discarding item called by name */
			if (ch == '@') {
				ch = 0;
				/* Prepare to ignore the (partial) item name sequence */
				macro_missing_item = 2;
			}
			/* Check for discarding item called by numeric inscription */
			else if (ch >= '0' && ch <= '9') {
				ch = 0;
				/* Case closed */
				macro_missing_item = 0;
			}
			/* Expected item choice is missing IN the macro - bad macro! Case closed though. */
			else macro_missing_item = 0;
		/* Still within call-by-name sequence? */
		} else if (macro_missing_item == 2) {
			if (ch == '\r') {
				/* Sequence finally ends here */
				macro_missing_item = 0;
			} else ch = 0;
		/* Check for problems with missing inscription-called items */
		} else if (macro_missing_item == 3) {
			if (ch >= '0' && ch <= '9') ch = 0;
			/* Case closed in any way */
			macro_missing_item = 0;
		}

		/* return next macro character */
		return (ch);
	}

	/* Do not check "control-underscore" sequences */
	if (parse_under) return (ch);

	/* Do not check "control-backslash" sequences */
	if (parse_slash) return (ch);


	/* Efficiency -- Ignore impossible macros */
	if (!macro__use[(byte)(ch)]) return (ch);

	/* Efficiency -- Ignore inactive macros */
	if (((shopping || !inkey_flag || inkey_msg) && (macro__use[(byte)(ch)] == MACRO_USE_CMD)) || inkey_interact_macros) return (ch);
	if (((shopping || inkey_msg) && (macro__use[(byte)(ch)] == MACRO_USE_HYB)) || inkey_interact_macros) return (ch);


	/* Save the first key, advance */
	buf[p++] = ch;
	buf[p] = '\0';

	/* Wait for a macro, or a timeout */
	while (TRUE)
	{
		/* Check for possible macros */
		k = macro_maybe(buf, k);

		/* Nothing matches */
		if (k < 0) break;

		/* Check for (and remove) a pending key */
		if (0 == Term_inkey(&ch, FALSE, TRUE))
		{
			/* Append the key */
			buf[p++] = ch;
			buf[p] = '\0';

			/* Restart wait */
			w = 0;
		}

		/* No key ready */
		else
		{
			if (multi_key_macros)
			{
				/* Increase "wait" */
				w += 10;

				/* Excessive delay */
				if (w >= 100) break;

				/* Delay */
				Term_xtra(TERM_XTRA_DELAY, w);
			}
			else
			{
				/* No waiting */
				break;
			}
		}
	}

	/* Check for a successful macro */
	k = macro_ready(buf);

	/* No macro available */
	if (k < 0)
	{
		/* Push all the keys back on the queue */
		while (p > 0)
		{
			/* Push the key, notice over-flow */
			if (Term_key_push(buf[--p])) return (0);
		}

		/* Wait for (and remove) a pending key */
		(void)Term_inkey(&ch, TRUE, TRUE);

		/* Return the key */
		return (ch);
	}


	/* Access the macro pattern */
	pat = macro__pat[k];

	/* Get the length of the pattern */
	n = strlen(pat);


	/* Push the "extra" keys back on the queue */
	while (p > n)
	{
		if (Term_key_push(buf[--p])) return (0);
	}

	/* We are now inside a macro */
	parse_macro = TRUE;
	/* Assume that the macro has no problems with possibly missing items so far */
	macro_missing_item = 0;

	/* Push the "macro complete" key */
	if (Term_key_push(29)) return (0);


	/* Access the macro action */
	act = macro__act[k];

	/* Get the length of the action */
	n = strlen(act);

#if 0
	/* Push the macro "action" onto the key queue */
	while (n > 0)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(act[--n])) return (0);
	}
#else
	/* Push all at once */
	Term_key_push_buf(act, n);
#endif

	/* Force "inkey()" to call us again */
	return (0);
}



/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * Normally, this function will process "macros", but if "inkey_base" is
 * TRUE, then we will bypass all "macro" processing.  This allows direct
 * usage of the "Term_inkey()" function.
 *
 * Normally, this function will do something, but if "inkey_xtra" is TRUE,
 * then something else will happen.
 *
 * Normally, this function will wait until a "real" key is ready, but if
 * "inkey_scan" is TRUE, then we will return zero if no keys are ready.
 *
 * Normally, this function will show the cursor, and will process all normal
 * macros, but if "inkey_flag" is TRUE, then we will only show the cursor if
 * "hilite_player" is TRUE (--possibly deprecated info here-- after
 * hilite_player was always broken, a new implementation is tested in 2013),
 * and also, we will only process "command" macros.
 *
 * Note that the "flush()" function does not actually flush the input queue,
 * but waits until "inkey()" is called to perform the "flush".
 *
 * Refresh the screen if waiting for a keypress and no key is ready.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.  This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of a few "special" control-keys, which
 * are reserved to simplify the use of various "main-xxx.c" files,
 * or used by the "macro" code above.
 *
 * Ascii 27 is "control left bracket" -- normal "Escape" key
 * Ascii 28 is "control backslash" -- special macro delimiter
 * Ascii 29 is "control right bracket" -- end of macro action
 * Ascii 30 is "control caret" -- indicates "keypad" key
 * Ascii 31 is "control underscore" -- begin macro-trigger
 *
 * Hack -- Make sure to allow calls to "inkey()" even if "term_screen"
 * is not the active Term, this allows the various "main-xxx.c" files
 * to only handle input when "term_screen" is "active".
 *
 * Note the nasty code used to process the "inkey_base" flag, which allows
 * various "macro triggers" to be entered as normal key-sequences, with the
 * appropriate timing constraints, but without actually matching against any
 * macro sequences.  Most of the nastiness is to handle "ascii 28" (see below).
 *
 * The "ascii 28" code is a complete hack, used to allow "default actions"
 * to be associated with a given keypress, and used only by the X11 module,
 * it may or may not actually work.  The theory is that a keypress can send
 * a special sequence, consisting of a "macro trigger" plus a "default action",
 * with the "default action" surrounded by "ascii 28" symbols.  Then, when that
 * key is pressed, if the trigger matches any macro, the correct action will be
 * executed, and the "strip default action" code will remove the "default action"
 * from the keypress queue, while if it does not match, the trigger itself will
 * be stripped, and then the "ascii 28" symbols will be stripped as well, leaving
 * the "default action" keys in the "key queue".  Again, this may not work.
 */
char inkey(void)
{
	int v;
	char kk, ch;
	bool done = FALSE;
	term *old = Term;
	int w = 0;
	int skipping = FALSE;


        /* Hack -- handle delayed "flush()" */
        if (flush_later) {
                /* Done */
                flush_later = FALSE;

                /* Cancel "macro" info */
//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
		restore_prompt();
//#endif
                parse_macro = after_macro = FALSE;

                /* Cancel "sequence" info */
                parse_under = parse_slash = FALSE;

                /* Cancel "strip" mode */
                strip_chars = FALSE;

                /* Forget old keypresses */
                Term_flush();
        }


	/* Access cursor state */
	(void)Term_get_cursor(&v);

	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag)) {
		/* Show the cursor */
		(void)Term_set_cursor(1);
	}


	/* Hack -- Activate the screen */
	Term_activate(term_screen);


	/* Get a (non-zero) keypress */
	for (ch = 0; !ch; ) {
		/* Flush output - maintain flickering/multi-hued characters */
		do_flicker(); //unnecessary since it's done in inkey_aux() anyway? */

		/* Nothing ready, not waiting, and not doing "inkey_base" */
		if (!inkey_base && inkey_scan && (0 != Term_inkey(&ch, FALSE, FALSE))) break;

		/* Hack -- flush the output once no key is ready */
		if (!done && (0 != Term_inkey(&ch, FALSE, FALSE))) {
			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate the screen */
			Term_activate(term_screen);

			/* Only once */
			done = TRUE;
		}

		/* Hack */
		if (inkey_base) {
			char xh;

#if 0 /* don't block.. */
			/* Check for keypress, optional wait */
			(void)Term_inkey(&xh, !inkey_scan, TRUE);
#else /* ..but keep processing net input in the background so we don't timeout. - C. Blue */
			int net_fd = Net_fd();
			if (inkey_scan) (void)Term_inkey(&xh, FALSE, TRUE); /* don't wait */
			else if (net_fd == -1) (void)Term_inkey(&xh, TRUE, TRUE); /* wait and block, no network yet anyway */
			else do { /* do wait, but don't block; keep processing */
				/* Note that we assume here that we're ONLY called from get_macro_trigger()! */

				/* Flush output - maintain flickering/multi-hued characters */
				do_flicker();

				/* Look for a keypress */
				(void)(Term_inkey(&xh, FALSE, TRUE));

				/* If we got a key, break */
				if (xh) break;

 #if 0 /* probably should be disabled - assumption: we're ONLY called for get_macro_trigger() */
				/* If we received a 'max_line' value from the net,
				   break if inkey_max_line flag was set */
				if (inkey_max_line != inkey_max_line_set) {
					/* hack: */
					ch = 1;
					break;
				}
 #endif

				/* Update our timer and if neccecary send a keepalive packet */
				update_ticks();
				if (!c_quit) {
					do_keepalive();
					do_ping();
				}

				/* Flush the network output buffer */
				Net_flush();

				/* Wait according to fps - mikaelh */
				SetTimeout(0, next_frame());

				/* Update the screen */
				Term_fresh();

				if (c_quit) {
					usleep(1000);
					continue;
				}

				/* Parse net input if we got any */
				if (SocketReadable(net_fd)) {
					if (Net_input() == -1) quit("Net_input failed.");
				}

 #if 0 /* probably not needed - assumption: we're ONLY called for get_macro_trigger() */
				/* Hack - Leave a store */
				if (shopping && leave_store) {
					return ESCAPE;
				}
 #endif

				/* Redraw windows if necessary */
				if (p_ptr->window) window_stuff();
			} while (1);
#endif

			/* Key ready */
			if (xh) {
				/* Reset delay */
				w = 0;

				/* Mega-Hack */
				if (xh == 28) {
					/* Toggle "skipping" */
					skipping = !skipping;
				}

				/* Use normal keys */
				else if (!skipping) {
					/* Use it */
					ch = xh;
				}
			}

			/* No key ready */
			else {
				if (multi_key_macros) {
					/* Increase "wait" */
					w += 10;

					/* Excessive delay */
					if (w >= 100) break;

					/* Delay */
					Term_xtra(TERM_XTRA_DELAY, w);
				} else {
					/* No waiting */
					break;
				}
			}

			/* Continue */
			continue;
		}

		/* Get a key (see above) */
		kk = ch = inkey_aux();


		/* Finished a "control-underscore" sequence */
		if (parse_under && (ch <= 32)) {
			/* Found the edge */
			parse_under = FALSE;

			/* Stop stripping */
			strip_chars = FALSE;

			/* Strip this key */
			ch = 0;
		}


		/* Finished a "control-backslash" sequence */
		if (parse_slash && (ch == 28)) {
			/* Found the edge */
			parse_slash = FALSE;

			/* Stop stripping */
			strip_chars = FALSE;

			/* Strip this key */
			ch = 0;
		}


		/* Handle some special keys */
		switch (ch) {
			/* Hack -- convert back-quote into escape */
			case '`':

			/* Convert to "Escape" */
			ch = ESCAPE;

			/* Done */
			break;

			/* Hack -- strip "control-right-bracket" end-of-macro-action */
			case 29:

			/* Strip this key */
			ch = 0;

			/* Done */
			break;

			/* Hack -- strip "control-caret" special-keypad-indicator */
			case 30:

			/* Strip this key */
			ch = 0;

			/* Done */
			break;

			/* Hack -- strip "control-underscore" special-macro-triggers */
			case 31:

			/* Strip this key */
			ch = 0;

			/* Inside a "underscore" sequence */
			parse_under = TRUE;

			/* Strip chars (always) */
			strip_chars = TRUE;

			/* Done */
			break;

			/* Hack -- strip "control-backslash" special-fallback-strings */
			case 28:

			/* Strip this key */
			ch = 0;

			/* Inside a "control-backslash" sequence */
			parse_slash = TRUE;

			/* Strip chars (sometimes) */
			strip_chars = after_macro;

			/* Done */
			break;
		}


		/* Hack -- Set "after_macro" code */
		after_macro = ((kk == 29) ? TRUE : FALSE);


		/* Hack -- strip chars */
		if (strip_chars) ch = 0;
	}


	/* Hack -- restore the term */
	Term_activate(old);

	/* Restore the cursor */
	Term_set_cursor(v);

	/* Cancel the various "global parameters" */
	inkey_base = inkey_scan = inkey_flag = inkey_max_line = FALSE;

	/* Return the keypress */
	return (ch);
}

static int hack_dir = 0;

/*
 * Convert a "Rogue" keypress into an "Angband" keypress
 * Pass extra information as needed via "hack_dir"
 *
 * Note that many "Rogue" keypresses encode a direction.
 */
static char roguelike_commands(char command)
{
        /* Process the command */
        switch (command)
        {
                /* Movement (rogue keys) */
                case 'b': hack_dir = 1; return (';');
                case 'j': hack_dir = 2; return (';');
                case 'n': hack_dir = 3; return (';');
                case 'h': hack_dir = 4; return (';');
                case 'l': hack_dir = 6; return (';');
                case 'y': hack_dir = 7; return (';');
                case 'k': hack_dir = 8; return (';');
                case 'u': hack_dir = 9; return (';');

                /* Running (shift + rogue keys) */
                case 'B': hack_dir = 1; return ('.');
                case 'J': hack_dir = 2; return ('.');
                case 'N': hack_dir = 3; return ('.');
                case 'H': hack_dir = 4; return ('.');
                case 'L': hack_dir = 6; return ('.');
                case 'Y': hack_dir = 7; return ('.');
                case 'K': hack_dir = 8; return ('.');
                case 'U': hack_dir = 9; return ('.');

                /* Tunnelling (control + rogue keys) */
                case KTRL('B'): hack_dir = 1; return ('+');
                case KTRL('J'): hack_dir = 2; return ('+');
                case KTRL('N'): hack_dir = 3; return ('+');
                case KTRL('H'): hack_dir = 4; return ('+');
                case KTRL('L'): hack_dir = 6; return ('+');
                case KTRL('Y'): hack_dir = 7; return ('+');
                case KTRL('K'): hack_dir = 8; return ('+');
                case KTRL('U'): hack_dir = 9; return ('+');

                /* Hack -- White-space */
                case KTRL('M'): return ('\r');

                /* Allow use of the "destroy" command */
                case KTRL('D'): return ('k');

                /* Hack -- Commit suicide */
//(display fps) case KTRL('C'): return ('Q');
                /* Locate player on map */
                case 'W': return ('L');
                /* Browse a book (Peruse) */
                case 'P': return ('b');
                /* Steal */
                case 'S': return ('j');
                /* Toggle search mode */
                case '#': return ('S');
                /* Use a staff (Zap) */
                case 'Z': return ('u');
                /* Take off equipment */
                case 'T': return ('t');
                /* Fire an item */
                case 't': return ('f');
                /* Bash a door (Force) */
                case 'f': return ('B');
                /* Look around (examine) */
                case 'x': return ('l');
                /* Aim a wand (Zap) */
                case 'z': return ('a');
                /* Zap a rod (Activate) */
                case 'a': return ('z');
		/* Party mode */
		case 'O': return ('P');

		/* Secondary 'wear/wield' */
		case KTRL('W'): return ('W');
		/* Swap item */
		case KTRL('A'): return ('x');
		/* House commands */
		case KTRL('E'): return ('h');

                /* Run */
                case ',': return ('.');
                /* Stay still (fake direction) */
                case '.': hack_dir = 5; return (',');
                /* Stay still (fake direction) */
                case '5': hack_dir = 5; return (',');

                /* Standard walking */
                case '1': hack_dir = 1; return (';');
                case '2': hack_dir = 2; return (';');
                case '3': hack_dir = 3; return (';');
                case '4': hack_dir = 4; return (';');
                case '6': hack_dir = 6; return (';');
                case '7': hack_dir = 7; return (';');
                case '8': hack_dir = 8; return (';');
                case '9': hack_dir = 9; return (';');
        }

        /* Default */
        return (command);
}



/*
 * Convert an "Original" keypress into an "Angband" keypress
 * Pass direction information back via "hack_dir".
 *
 * Note that "Original" and "Angband" are very similar.
 */
static char original_commands(char command)
{
	/* Process the command */
	switch(command)
	{
		/* Hack -- White space */
		case KTRL('J'): return ('\r');
		case KTRL('M'): return ('\r');

		/* Tunnel */
		case 'T': return ('+');

		/* Run */
		case '.': return ('.');

		/* Stay still (fake direction) */
		case ',': hack_dir = 5; return (',');

		/* Stay still (fake direction) */
		case '5': hack_dir = 5; return (',');

		/* Standard walking */
		case '1': hack_dir = 1; return (';');
		case '2': hack_dir = 2; return (';');
		case '3': hack_dir = 3; return (';');
		case '4': hack_dir = 4; return (';');
		case '6': hack_dir = 6; return (';');
		case '7': hack_dir = 7; return (';');
		case '8': hack_dir = 8; return (';');
		case '9': hack_dir = 9; return (';');

		/* Hack -- Commit suicide */
		case KTRL('K'): return ('Q');
//(display fps)	case KTRL('C'): return ('Q');
	}

	/* Default */
	return (command);
}



/*
 * React to new value of "rogue_like_commands".
 *
 * Initialize the "keymap" arrays based on the current value of
 * "rogue_like_commands".  Note that all "undefined" keypresses
 * by default map to themselves with no direction.  This allows
 * "standard" commands to use the same keys in both keysets.
 *
 * To reset the keymap, simply set "rogue_like_commands" to -1,
 * call this function, restore its value, call this function.
 *
 * The keymap arrays map keys to "command_cmd" and "command_dir".
 *
 * It is illegal for keymap_cmds[N] to be zero, except for
 * keymaps_cmds[0], which is unused.
 *
 * You can map a key to "tab" to make it "non-functional".
 */
void keymap_init(void)
{
	int i, k;

	/* Initialize every entry */
	for (i = 0; i < 128; i++)
	{
		/* Default to "no direction" */
		hack_dir = 0;

		/* Attempt to translate */
		if (c_cfg.rogue_like_commands)
		{
			k = roguelike_commands(i);
		}
		else
		{
			k = original_commands(i);
		}

		/* Save the keypress */
		keymap_cmds[i] = k;

		/* Save the direction */
		keymap_dirs[i] = hack_dir;
	}
}


/*
 * Flush the screen, make a noise
 */
void bell(void)
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Make a bell noise (if allowed) */
	if (c_cfg.ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
}

/* Generate a page sfx (beep) */
int page(void) {
#ifdef USE_SOUND_2010
#ifdef SOUND_SDL
	/* Try to beep via page sfx of the SDL audio system first */
	if (c_cfg.audio_paging && sound_page()) return 1;
#endif
#endif

	/* Fall back on system-specific default beeps */
	//Term_fresh();
	Term_xtra(TERM_XTRA_NOISE, 0);
	//flush();

	return 1;
}
/* Generate a warning sfx (beep) or if it's missing then a page sfx */
int warning_page(void) {
#ifdef USE_SOUND_2010
#ifdef SOUND_SDL
	/* Try to beep via page sfx of the SDL audio system first */
	if (sound_warning()) return 1;
	if (c_cfg.audio_paging && sound_page()) return 1;
#endif
#endif

	/* Fall back on system-specific default beeps */
	//Term_fresh();
	Term_xtra(TERM_XTRA_NOISE, 0);
	//flush();

	return 1;
}


/*
 * Display a string on the screen using an attribute, and clear
 * to the end of the line.
 */
void c_prt(byte attr, cptr str, int row, int col)
{
	/* Hack -- fake monochrome */
	/* if (!c_cfg.use_color) attr = TERM_WHITE; */

	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}

/*
 * As above, but in "white"
 */
void prt(cptr str, int row, int col)
{
	/* Spawn */
	c_prt(TERM_WHITE, str, row, col);
}


/*
 * Print the last n characters of str onto the screen.
 */
static void c_ptr_last(int x, int y, int n, byte attr, char *str)
{
	int len = strlen(str);

	if (n < len) {
		Term_putstr(x, y, n, attr, str + (len - n));
	} else {
		Term_putstr(x, y, len, attr, str);
	}
}


/*
 * Get some input at the cursor location.
 * Assume the buffer is initialized to a default string.
 * Note that this string is often "empty" (see below).
 * The default buffer is displayed in yellow until cleared.
 * Pressing RETURN right away accepts the default entry.
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 */

/* APD -- added private so passwords will not be displayed. */
/*
 * Jir -- added history.
 * TODO: cursor editing
 */
bool askfor_aux(char *buf, int len, char mode) {
	int y, x;
	int i = 0;
	int k = 0;

	/* Terminal width */
	int wid = 80;

	/* Visible length on the screen */
	int vis_len = len;

	bool done = FALSE;

	/* Hack -- if short, don't use history */
	bool nohist = (mode & ASKFOR_PRIVATE) || len < 20;
	byte cur_hist;

	if (mode & ASKFOR_CHATTING) {
		cur_hist = hist_chat_end;
	} else {
		cur_hist = hist_end;
	}

	/* Handle wrapping */
	if (cur_hist >= MSG_HISTORY_MAX) cur_hist = 0;

	/* Locate the cursor */
	Term_locate(&x, &y);

	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Paranoia -- check len */
	if (len < 1) len = 1;

	/* Paranoia -- check column */
	if ((x < 0) || (x >= wid - 1)) x = 0;

	/* Restrict the visible length */
	if (x + vis_len > wid - 1) vis_len = wid - 1 - x;

	/* Paranoia -- Clip the default entry */
	buf[len] = '\0';

	/* Display the default answer */
	Term_erase(x, y, len);
	if (mode & ASKFOR_PRIVATE) c_ptr_last(x, y, vis_len, TERM_YELLOW, buf[0] ? "(default)" : "");
	else c_ptr_last(x, y, vis_len, TERM_YELLOW, buf);

	if (mode & ASKFOR_CHATTING) {
		strncpy(message_history_chat[hist_chat_end], buf, sizeof(*message_history_chat) - 1);
		message_history_chat[hist_chat_end][sizeof(*message_history_chat) - 1] = '\0';
	}
	else if (!nohist) {
		strncpy(message_history[hist_end], buf, sizeof(*message_history) - 1);
		message_history[hist_end][sizeof(*message_history) - 1] = '\0';
	}

	/* Process input */
	while (!done) {
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		i = inkey();

		/* Analyze the key */
		switch (i) {
			case ESCAPE:
			case KTRL('X'):
			k = 0;
			done = TRUE;
			break;

			case '\n':
			case '\r':
			k = strlen(buf);
			done = TRUE;
			break;

			case 0x7F:
			case '\010':
			if (k > 0) k--;
			break;

			case KTRL('I'): /* TAB */
			if (mode & ASKFOR_CHATTING) {
				/* Change chatting mode - mikaelh */
				chat_mode++;
				if (chat_mode > CHAT_MODE_GUILD)
					chat_mode = CHAT_MODE_NORMAL;

				/* HACK - Change the prompt */
				switch (chat_mode) {
					case CHAT_MODE_PARTY:
						c_prt(C_COLOUR_CHAT_PARTY, "Party: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Party: ");
						break;
					case CHAT_MODE_LEVEL:
						c_prt(C_COLOUR_CHAT_LEVEL, "Level: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Level: ");
						break;
					case CHAT_MODE_GUILD:
						c_prt(C_COLOUR_CHAT_GUILD, "Guild: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Guild: ");
						break;
					default:
						prt("Message: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Message: ");
						break;
				}

				Term_locate(&x, &y);
			}
			break;

			case KTRL('U'): /* reverse KTRL('I') */
			if (mode & ASKFOR_CHATTING) {
				/* Reverse change chatting mode */
				chat_mode--;
				if (chat_mode < CHAT_MODE_NORMAL)
					chat_mode = CHAT_MODE_GUILD;

				/* HACK - Change the prompt */
				switch (chat_mode) {
					case CHAT_MODE_PARTY:
						c_prt(C_COLOUR_CHAT_PARTY, "Party: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Party: ");
						break;
					case CHAT_MODE_LEVEL:
						c_prt(C_COLOUR_CHAT_LEVEL, "Level: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Level: ");
						break;
					case CHAT_MODE_GUILD:
						c_prt(C_COLOUR_CHAT_GUILD, "Guild: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Guild: ");
						break;
					default:
						prt("Message: ", 0, 0);

						/* Recalculate visible length */
						vis_len = wid - 1 - sizeof("Message: ");
						break;
				}

				Term_locate(&x, &y);
			}
			break;

			case KTRL('A'): /* All ('message') */
			if (mode & ASKFOR_CHATTING) {
				chat_mode = CHAT_MODE_NORMAL;
				prt("Message: ", 0, 0);

				/* Recalculate visible length */
				vis_len = wid - 1 - sizeof("Message: ");

				Term_locate(&x, &y);
			}
			break;

			case KTRL('T'): /* Party */
			if (mode & ASKFOR_CHATTING) {
				chat_mode = CHAT_MODE_PARTY;
				c_prt(C_COLOUR_CHAT_PARTY, "Party: ", 0, 0);

				/* Recalculate visible length */
				vis_len = wid - 1 - sizeof("Party: ");

				Term_locate(&x, &y);
			}
			break;

			case KTRL('L'): /* Level */
			if (mode & ASKFOR_CHATTING) {
				chat_mode = CHAT_MODE_LEVEL;
				c_prt(C_COLOUR_CHAT_LEVEL, "Level: ", 0, 0);

				/* Recalculate visible length */
				vis_len = wid - 1 - sizeof("Level: ");

				Term_locate(&x, &y);
			}
			break;

			case KTRL('G'): /* Guild */
			if (mode & ASKFOR_CHATTING) {
				chat_mode = CHAT_MODE_GUILD;
				c_prt(C_COLOUR_CHAT_GUILD, "Guild: ", 0, 0);

				/* Recalculate visible length */
				vis_len = wid - 1 - sizeof("Guild: ");

				Term_locate(&x, &y);
			}
			break;

#if 0 /* was: erase line. Can just hit ESC though, so if 0'ed. */
			case KTRL('U'):
			k = 0;
			break;
#endif

			case KTRL('N'):
			if (nohist) break;
			cur_hist++;

			if (mode & ASKFOR_CHATTING) {
				if ((!hist_chat_looped && hist_chat_end < cur_hist) ||
					(hist_chat_looped && cur_hist >= MSG_HISTORY_MAX)) {
					cur_hist = 0;
				}

				strncpy(buf, message_history_chat[cur_hist], len);
				buf[len] = '\0';
			} else {
				if ((!hist_looped && hist_end < cur_hist) ||
					(hist_looped && cur_hist >= MSG_HISTORY_MAX)) {
					cur_hist = 0;
				}

				strncpy(buf, message_history[cur_hist], len);
				buf[len] = '\0';
			}

			k = strlen(buf);
			break;

			case KTRL('P'):
			if (nohist) break;
			if (mode & ASKFOR_CHATTING) {
				if (cur_hist) cur_hist--;
				else {
					cur_hist = hist_chat_looped ? MSG_HISTORY_MAX - 1 : hist_chat_end;
				}

				strncpy(buf, message_history_chat[cur_hist], len);
				buf[len] = '\0';
			} else {
				if (cur_hist) cur_hist--;
				else {
					cur_hist = hist_looped ? MSG_HISTORY_MAX - 1 : hist_end;
				}

				strncpy(buf, message_history[cur_hist], len);
				buf[len] = '\0';
			}

			k = strlen(buf);
			break;

			case KTRL('W'): {
				bool tail = FALSE;
				for (--k; k >= 0; k--)
				{
					if ((buf[k] == ' ' || buf[k] == ':' || buf[k] == ',') && tail)
					{
						/* leave the last separator */
						k++;
						break;
					}
					tail = TRUE;
				}
				if (k < 0) k = 0;
				break;
			}

			default:
				/* inkey_letter_all hack for c_get_quantity() */
				//if (inkey_letter_all && !k && ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z'))) { i = 'a';
				if (inkey_letter_all && !k && (i == 'a' || i == 'A' || i == ' ')) { /* allow spacebar too */
					buf[k++] = 'a';
					done = TRUE;
					break;
				}

				if ((k < len) && (isprint(i))) buf[k++] = i;
				else bell();
			break;
		}

		/* Terminate */
		buf[k] = '\0';

		/* Update the entry */
		if (!(mode & ASKFOR_PRIVATE)) {
			Term_erase(x, y, vis_len);
			c_ptr_last(x, y, vis_len, TERM_WHITE, buf);
		} else {
			Term_erase(x + k, y, len - k);
			if (k) Term_putch(x + k - 1, y, TERM_WHITE, 'x');
		}
	}

	/* c_get_quantity() hack is in any case inactive again */
	inkey_letter_all = FALSE;

	/* The top line is OK now */
	topline_icky = FALSE;
	if(!c_quit)
		Flush_queue();

	/* Aborted */
	if (i == ESCAPE) return (FALSE);

	/* Success */
	if (nohist) return (TRUE);

	/* Add this to the history */
	if (mode & ASKFOR_CHATTING) {
		strncpy(message_history_chat[hist_chat_end], buf, sizeof(*message_history_chat) - 1);
		message_history_chat[hist_chat_end][sizeof(*message_history_chat) - 1] = '\0';
		hist_chat_end++;
		if (hist_chat_end >= MSG_HISTORY_MAX) {
			hist_chat_end = 0;
			hist_chat_looped = TRUE;
		}
	} else {
		strncpy(message_history[hist_end], buf, sizeof(*message_history) - 1);
		message_history[hist_end][sizeof(*message_history) - 1] = '\0';
		hist_end++;
		if (hist_end >= MSG_HISTORY_MAX) {
			hist_end = 0;
			hist_looped = TRUE;
		}
	}

	/* Handle the additional chat modes */
#if 0
	/* Slash commands and self-chat are exceptions */
	if ((mode & ASKFOR_CHATTING) && chat_mode != CHAT_MODE_NORMAL && buf[0] != '/' && !(buf[0] == '%' && buf[1] == ':'))
#else
	/* Slash commands and aimed chat (with *: prefix) are exceptions */
	if ((mode & ASKFOR_CHATTING) && chat_mode != CHAT_MODE_NORMAL && buf[0] != '/' &&
	    (buf[1] != ':' || (buf[0] != '%' && buf[0] != '!' && buf[0] != '$' && buf[0] != '#' && buf[0] != '+')))
#endif
	{
		for (i = k; i >= 0; i--)
			buf[i + 2] = buf[i];

		if (chat_mode == CHAT_MODE_PARTY)
			buf[0] = '!';
		else if (chat_mode == CHAT_MODE_LEVEL)
			buf[0] = '#';
		else if (chat_mode == CHAT_MODE_GUILD)
			buf[0] = '$';
		else
			buf[0] = '\0';
		buf[1] = ':';
		k += 2;
		buf[k] = '\0';
	}

	/* Success */
	return (TRUE);
}


/*
 * Get a string from the user
 *
 * The "prompt" should take the form "Prompt: "
 *
 * Note that the initial contents of the string is used as
 * the default response, so be sure to "clear" it if needed.
 *
 * We clear the input, and return FALSE, on "ESCAPE".
 */
bool get_string(cptr prompt, char *buf, int len)
{
	bool res;
	char askfor_mode = 0x00;

	/* suppress hybrid macros */
	bool inkey_msg_old = inkey_msg;
	inkey_msg = TRUE;

	/* Display prompt */
	Term_erase(0, 0, 255);
	Term_putstr(0, 0, -1, TERM_WHITE, (char *)prompt);

	if (streq(prompt, "Message: ")) {
		askfor_mode |= ASKFOR_CHATTING;

		/* HACK - Change the prompt according to current chat mode */
		switch (chat_mode) {
			case CHAT_MODE_PARTY:
				c_prt(C_COLOUR_CHAT_PARTY, "Party: ", 0, 0);
				break;
			case CHAT_MODE_LEVEL:
				c_prt(C_COLOUR_CHAT_LEVEL, "Level: ", 0, 0);
				break;
			case CHAT_MODE_GUILD:
				c_prt(C_COLOUR_CHAT_GUILD, "Guild: ", 0, 0);
				break;
		}
	}

	/* Ask the user for a string */
	res = askfor_aux(buf, len, askfor_mode);

	/* Clear prompt */
	if (askfor_mode & ASKFOR_CHATTING) clear_topline_forced();
	else if (res) clear_topline(); /* exited via confirming */
	else clear_topline_forced(); /* exited via ESC */

	/* restore responsiveness to hybrid macros */
	inkey_msg = inkey_msg_old;

	/* Result */
	return (res);
}


/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command)
{
	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Display a prompt */
	prompt_topline(prompt);

	/* Get a key */
	*command = inkey();

	/* Clear the prompt */
	clear_topline();

	/* Fix the top line */
	topline_icky = FALSE;

	/* Flush any events */
	Flush_queue();

	/* Handle "cancel" */
	if (*command == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Request a command from the user.
 *
 * Sets command_cmd, command_dir, command_rep, command_arg.
 *
 * Note that "caret" ("^") is treated special, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that this command is used both in the dungeon and in
 * stores, and must be careful to work in both situations.
 */
void request_command() {
	char cmd;


	/* Flush the input */
	/* flush(); */

	/* Get a keypress in "command" mode */

	/* Activate "command mode" */
	inkey_flag = TRUE;

	/* Activate "scan mode" */
	inkey_scan = TRUE;

	/* Get a command */
	cmd = inkey();

	/* Return if no key was pressed */
	if (!cmd) return;

	msg_flag = FALSE;
	clear_topline();

//#ifndef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (!c_cfg.keep_topline)
	/* Clear top line */
	clear_topline();
//#endif

	/* Bypass "keymap" */
	if (cmd == '\\') {
//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
		/* Clear top line */
		clear_topline();
//#endif

		/* Get a char to use without casting */
		(void)(get_com("Command: ", &cmd));

		/* Hack -- allow "control chars" to be entered */
		if (cmd == '^') {
			/* Get a char to "cast" into a control char */
			(void)(get_com("Command: Control: ", &cmd));

			/* Convert */
			cmd = KTRL(cmd);
		}

//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
		/* Clear top line */
		clear_topline();
//#endif

		/* Use the key directly */
		command_cmd = cmd;
	} else {
		/* Hack -- allow "control chars" to be entered */
		if (cmd == '^') {
//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
			/* Clear top line */
			clear_topline();
//#endif

			/* Get a char to "cast" into a control char */
			(void)(get_com("Control: ", &cmd));

			/* Convert */
			cmd = KTRL(cmd);

//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (c_cfg.keep_topline)
			/* Clear top line */
			clear_topline();
//#endif
		}

		/* Access the array info */
		command_cmd = keymap_cmds[cmd & 0x7F];
		command_dir = keymap_dirs[cmd & 0x7F];
	}

	/* Paranoia */
	if (!command_cmd) command_cmd = ESCAPE;

//#ifndef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
if (!c_cfg.keep_topline)
	/* Hack -- erase the message line. */
	clear_topline();
//#endif
}

bool get_dir(int *dp)
{
	int	dir = 0;
	char	command;
	cptr	p;

	p = "Direction ('*' choose target, '-' acquired target, '+' acquired/manual)?";
	get_com(p, &command);

#if 0 /* maaaybe in future: client-side monster check \
	 for best performance (might be exploitable though). \
	 Meanwhile see below. */
	if (command == '+') {
		if (!target_okay()) {
			p = "Direction ('*' to choose a target)?";
			get_com(p, &command);
		} else command = '-';
	}
#endif

	/* Handle target request */
	if (command == '*') {
		if (cmd_target()) dir = 5;
	}
	/* Handle previously acquired target, discard cast if not available */
	else if (command == '-') dir = 10;
	else if (command == '+') dir = 11;
	/* Normal direction, including 5 for 'acquired target or self' */
	else dir = keymap_dirs[command & 0x7F];

	*dp = dir;

	if (!dir) return (FALSE);
	return (TRUE);
}


/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, (char*)str);
}


/*
 * As above, but in "white"
 */
void put_str(cptr str, int row, int col)
{
	/* Spawn */
	Term_putstr(col, row, -1, TERM_WHITE, (char*)str);
}

/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check2(cptr prompt, bool default_yes) {
	int i;
	char buf[80];

	/* Hack -- Build a "useful" prompt */
	if (default_yes) strnfmt(buf, 78, "%.70s [Y/n]", prompt);
	else strnfmt(buf, 78, "%.70s [y/N]", prompt);

	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Prompt for it */
	prompt_topline(buf);

#if 0
	/* Get an acceptable answer */
	while (TRUE) {
		i = inkey();
		if (c_cfg.quick_messages) break;
		if (i == ESCAPE) break;
		if (strchr("YyNn", i)) break;
		bell();
	}
#else
	/* c_cfg.quick_messages now always on */
	i = inkey();
#endif

	/* Erase the prompt */
	clear_topline();

	/* The top line is OK again */
	topline_icky = FALSE;

	/* Flush any events that came in while we were icky */
	if (!c_quit)
		Flush_queue();

	/* More normal */
	if (default_yes) {
		if (i == 'n' || i == 'N') return FALSE;
		return TRUE;
	}

	if ((i == 'Y') || (i == 'y')) return (TRUE);
	return (FALSE);
}

byte get_3way(cptr prompt, bool default_no) {
	int i, res = -1;
	char buf[80];

	/* Hack -- Build a "useful" prompt */
	if (default_no) strnfmt(buf, 78, "%.70s [y/a/N]", prompt);
	else strnfmt(buf, 78, "%.70s [y/a/n]", prompt);

	/* The top line is "icky" */
	topline_icky = TRUE;

	/* Prompt for it */
	prompt_topline(buf);

	while (TRUE) {
		/* c_cfg.quick_messages now always on */
		i = inkey();

		if (i == 'Y' || i == 'y') res = 1;
		else if (i == 'A' || i == 'a') res = 2;
		else if (i == 'N' || i == 'n' ||
		    (default_no && (i == '\r' || i == '\n' || i == '\e'))) /* not all keys, just ESC and RETURN */
			res = 0;
		if (res != -1) break;
	}

	/* Flush any events that came in while we were icky */
	if (!c_quit) Flush_queue();

	/* Erase the prompt */
	clear_topline();

	/* The top line is OK again */
	topline_icky = FALSE;

	return res;
}


/*
 * Recall the "text" of a saved message
 */
cptr message_str(s32b age)
{
        s32b x;
        s32b o;
        cptr s;

        /* Forgotten messages have no text */
        if ((age < 0) || (age >= message_num())) return ("");

        /* Acquire the "logical" index */
        x = (message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

        /* Get the "offset" for the message */
        o = message__ptr[x];

        /* Access the message text */
        s = &message__buf[o];

        /* Return the message text */
        return (s);
}
cptr message_str_chat(s32b age)
{
        s32b x;
        s32b o;
        cptr s;

        /* Forgotten messages have no text */
        if ((age < 0) || (age >= message_num_chat())) return ("");

        /* Acquire the "logical" index */
        x = (message__next_chat + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

        /* Get the "offset" for the message */
        o = message__ptr_chat[x];

        /* Access the message text */
        s = &message__buf_chat[o];

        /* Return the message text */
        return (s);
}
cptr message_str_msgnochat(s32b age)
{
        s32b x;
        s32b o;
        cptr s;

        /* Forgotten messages have no text */
        if ((age < 0) || (age >= message_num_msgnochat())) return ("");

        /* Acquire the "logical" index */
        x = (message__next_msgnochat + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

        /* Get the "offset" for the message */
        o = message__ptr_msgnochat[x];

        /* Access the message text */
        s = &message__buf_msgnochat[o];

        /* Return the message text */
        return (s);
}


/*
 * How many messages are "available"?
 */
s32b message_num(void)
{
        int last, next, n;

        /* Extract the indexes */
        last = message__last;
        next = message__next;

        /* Handle "wrap" */
        if (next < last) next += MESSAGE_MAX;

        /* Extract the space */
        n = (next - last);

        /* Return the result */
        return (n);
}
s32b message_num_chat(void)
{
        int last, next, n;

        /* Extract the indexes */
        last = message__last_chat;
        next = message__next_chat;

        /* Handle "wrap" */
        if (next < last) next += MESSAGE_MAX;

        /* Extract the space */
        n = (next - last);

        /* Return the result */
        return (n);
}
s32b message_num_msgnochat(void)
{
        int last, next, n;

        /* Extract the indexes */
        last = message__last_msgnochat;
        next = message__next_msgnochat;

        /* Handle "wrap" */
        if (next < last) next += MESSAGE_MAX;

        /* Extract the space */
        n = (next - last);

        /* Return the result */
        return (n);
}


/*
 * Add a new message, with great efficiency
 */
void c_message_add(cptr str)
{
        int i, k, x, n;


        /*** Step 1 -- Analyze the message ***/

        /* Hack -- Ignore "non-messages" */
        if (!str) return;

        /* Message length */
        n = strlen(str);

        /* Important Hack -- Ignore "long" messages */
        if (n >= MESSAGE_BUF / 4) return;


        /*** Step 2 -- Attempt to optimize ***/

        /* Limit number of messages to check */
        k = message_num() / 4;

        /* Limit number of messages to check */
        if (k > MESSAGE_MAX / 32) k = MESSAGE_MAX / 32;

        /* Check the last few messages (if any to count) */
        for (i = message__next; k; k--)
        {
                u32b q;

                cptr old;

                /* Back up and wrap if needed */
                if (i-- == 0) i = MESSAGE_MAX - 1;

                /* Stop before oldest message */
                if (i == message__last) break;

                /* Extract "distance" from "head" */
                q = (message__head + MESSAGE_BUF - message__ptr[i]) % MESSAGE_BUF;

                /* Do not optimize over large distance */
                if (q > MESSAGE_BUF / 2) continue;

                /* Access the old string */
                old = &message__buf[message__ptr[i]];

                /* Compare */
                if (!streq(old, str)) continue;

                /* Get the next message index, advance */
                x = message__next++;

                /* Handle wrap */
                if (message__next == MESSAGE_MAX) message__next = 0;

                /* Kill last message if needed */
                if (message__next == message__last) message__last++;

                /* Handle wrap */
                if (message__last == MESSAGE_MAX) message__last = 0;

                /* Assign the starting address */
                message__ptr[x] = message__ptr[i];

		/* Redraw - assume that all chat messages start with '[' */
#if 0
		if (str[0] == '[')
	                p_ptr->window |= (PW_MESSAGE | PW_CHAT);
                else
		        p_ptr->window |= (PW_MESSAGE | PW_MSGNOCHAT);
#endif
		p_ptr->window |= (PW_MESSAGE);

                /* Success */
                return;
        }


        /*** Step 3 -- Ensure space before end of buffer ***/

        /* Kill messages and Wrap if needed */
        if (message__head + n + 1 >= MESSAGE_BUF)
        {
                /* Kill all "dead" messages */
                for (i = message__last; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next) break;

                        /* Kill "dead" messages */
                        if (message__ptr[i] >= message__head)
                        {
                                /* Track oldest message */
                                message__last = i + 1;
                        }
                }

                /* Wrap "tail" if needed */
                if (message__tail >= message__head) message__tail = 0;

                /* Start over */
                message__head = 0;
        }


        /*** Step 4 -- Ensure space before next message ***/

        /* Kill messages if needed */
        if (message__head + n + 1 > message__tail)
        {
                /* Grab new "tail" */
                message__tail = message__head + n + 1;

                /* Advance tail while possible past first "nul" */
                while (message__buf[message__tail-1]) message__tail++;

                /* Kill all "dead" messages */
                for (i = message__last; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next) break;

                        /* Kill "dead" messages */
                        if ((message__ptr[i] >= message__head) &&
                            (message__ptr[i] < message__tail))
                        {
                                /* Track oldest message */
                                message__last = i + 1;
                        }
                }
        }


        /*** Step 5 -- Grab a new message index ***/

        /* Get the next message index, advance */
        x = message__next++;

        /* Handle wrap */
        if (message__next == MESSAGE_MAX) message__next = 0;

        /* Kill last message if needed */
        if (message__next == message__last) message__last++;

        /* Handle wrap */
        if (message__last == MESSAGE_MAX) message__last = 0;



        /*** Step 6 -- Insert the message text ***/

        /* Assign the starting address */
        message__ptr[x] = message__head;

        /* Append the new part of the message */
        for (i = 0; i < n; i++)
        {
                /* Copy the message */
                message__buf[message__head + i] = str[i];
        }

        /* Terminate */
        message__buf[message__head + i] = '\0';

        /* Advance the "head" pointer */
        message__head += n + 1;

#if 0
//deprecated?:
	/* Window stuff - assume that all chat messages start with '[' */
        if (str[0] == '[')
	        p_ptr->window |= (PW_MESSAGE | PW_CHAT);
        else
	        p_ptr->window |= (PW_MESSAGE | PW_MSGNOCHAT);
//      	p_ptr->window |= PW_MESSAGE;
#else
	p_ptr->window |= PW_MESSAGE;
#endif
}
void c_message_add_chat(cptr str)
{
        int i, k, x, n;


        /*** Step 1 -- Analyze the message ***/

        /* Hack -- Ignore "non-messages" */
        if (!str) return;

        /* Message length */
        n = strlen(str);

        /* Important Hack -- Ignore "long" messages */
        if (n >= MESSAGE_BUF / 4) return;


        /*** Step 2 -- Attempt to optimize ***/

        /* Limit number of messages to check */
        k = message_num_chat() / 4;

        /* Limit number of messages to check */
        if (k > MESSAGE_MAX / 32) k = MESSAGE_MAX / 32;

        /* Check the last few messages (if any to count) */
        for (i = message__next_chat; k; k--)
        {
                u32b q;

                cptr old;

                /* Back up and wrap if needed */
                if (i-- == 0) i = MESSAGE_MAX - 1;

                /* Stop before oldest message */
                if (i == message__last_chat) break;

                /* Extract "distance" from "head" */
                q = (message__head_chat + MESSAGE_BUF - message__ptr_chat[i]) % MESSAGE_BUF;

                /* Do not optimize over large distance */
                if (q > MESSAGE_BUF / 2) continue;

                /* Access the old string */
                old = &message__buf_chat[message__ptr_chat[i]];

                /* Compare */
                if (!streq(old, str)) continue;

                /* Get the next message index, advance */
                x = message__next_chat++;

                /* Handle wrap */
                if (message__next_chat == MESSAGE_MAX) message__next_chat = 0;

                /* Kill last message if needed */
                if (message__next_chat == message__last_chat) message__last_chat++;

                /* Handle wrap */
                if (message__last_chat == MESSAGE_MAX) message__last_chat = 0;

                /* Assign the starting address */
                message__ptr_chat[x] = message__ptr_chat[i];

		/* Redraw - assume that all chat messages start with '[' */
                p_ptr->window |= (PW_CHAT);

                /* Success */
                return;
        }


        /*** Step 3 -- Ensure space before end of buffer ***/

        /* Kill messages and Wrap if needed */
        if (message__head_chat + n + 1 >= MESSAGE_BUF)
        {
                /* Kill all "dead" messages */
                for (i = message__last_chat; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next_chat) break;

                        /* Kill "dead" messages */
                        if (message__ptr_chat[i] >= message__head_chat)
                        {
                                /* Track oldest message */
                                message__last_chat = i + 1;
                        }
                }

                /* Wrap "tail" if needed */
                if (message__tail_chat >= message__head_chat) message__tail_chat = 0;

                /* Start over */
                message__head_chat = 0;
        }


        /*** Step 4 -- Ensure space before next message ***/

        /* Kill messages if needed */
        if (message__head_chat + n + 1 > message__tail_chat)
        {
                /* Grab new "tail" */
                message__tail_chat = message__head_chat + n + 1;

                /* Advance tail while possible past first "nul" */
                while (message__buf_chat[message__tail_chat-1]) message__tail_chat++;

                /* Kill all "dead" messages */
                for (i = message__last_chat; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next_chat) break;

                        /* Kill "dead" messages */
                        if ((message__ptr_chat[i] >= message__head_chat) &&
                            (message__ptr_chat[i] < message__tail_chat))
                        {
                                /* Track oldest message */
                                message__last_chat = i + 1;
                        }
                }
        }


        /*** Step 5 -- Grab a new message index ***/

        /* Get the next message index, advance */
        x = message__next_chat++;

        /* Handle wrap */
        if (message__next_chat == MESSAGE_MAX) message__next_chat = 0;

        /* Kill last message if needed */
        if (message__next_chat == message__last_chat) message__last_chat++;

        /* Handle wrap */
        if (message__last_chat == MESSAGE_MAX) message__last_chat = 0;



        /*** Step 6 -- Insert the message text ***/

        /* Assign the starting address */
        message__ptr_chat[x] = message__head_chat;

        /* Append the new part of the message */
        for (i = 0; i < n; i++)
        {
                /* Copy the message */
                message__buf_chat[message__head_chat + i] = str[i];
        }

        /* Terminate */
        message__buf_chat[message__head_chat + i] = '\0';

        /* Advance the "head" pointer */
        message__head_chat += n + 1;

	/* Window stuff - assume that all chat messages start with '[' */
        p_ptr->window |= (PW_CHAT);

}
void c_message_add_msgnochat(cptr str)
{
        int i, k, x, n;


        /*** Step 1 -- Analyze the message ***/

        /* Hack -- Ignore "non-messages" */
        if (!str) return;

        /* Message length */
        n = strlen(str);

        /* Important Hack -- Ignore "long" messages */
        if (n >= MESSAGE_BUF / 4) return;


        /*** Step 2 -- Attempt to optimize ***/

        /* Limit number of messages to check */
        k = message_num_msgnochat() / 4;

        /* Limit number of messages to check */
        if (k > MESSAGE_MAX / 32) k = MESSAGE_MAX / 32;

        /* Check the last few _msgnochatmessages (if any to count) */
        for (i = message__next; k; k--)
        {
                u32b q;

                cptr old;

                /* Back up and wrap if needed */
                if (i-- == 0) i = MESSAGE_MAX - 1;

                /* Stop before oldest message */
                if (i == message__last_msgnochat) break;

                /* Extract "distance" from "head" */
                q = (message__head_msgnochat + MESSAGE_BUF - message__ptr_msgnochat[i]) % MESSAGE_BUF;

                /* Do not optimize over large distance */
                if (q > MESSAGE_BUF / 2) continue;

                /* Access the old string */
                old = &message__buf_msgnochat[message__ptr_msgnochat[i]];

                /* Compare */
                if (!streq(old, str)) continue;

                /* Get the next message index, advance */
                x = message__next_msgnochat++;

                /* Handle wrap */
                if (message__next_msgnochat == MESSAGE_MAX) message__next_msgnochat = 0;

                /* Kill last message if needed */
                if (message__next_msgnochat == message__last_msgnochat) message__last_msgnochat++;

                /* Handle wrap */
                if (message__last_msgnochat == MESSAGE_MAX) message__last_msgnochat = 0;

                /* Assign the starting address */
                message__ptr_msgnochat[x] = message__ptr_msgnochat[i];

		/* Redraw - assume that all chat messages start with '[' */
                p_ptr->window |= (PW_MSGNOCHAT);

                /* Success */
                return;
        }


        /*** Step 3 -- Ensure space before end of buffer ***/

        /* Kill messages and Wrap if needed */
        if (message__head_msgnochat + n + 1 >= MESSAGE_BUF)
        {
                /* Kill all "dead" messages */
                for (i = message__last_msgnochat; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next_msgnochat) break;

                        /* Kill "dead" messages */
                        if (message__ptr_msgnochat[i] >= message__head_msgnochat)
                        {
                                /* Track oldest message */
                                message__last_msgnochat = i + 1;
                        }
                }

                /* Wrap "tail" if needed */
                if (message__tail_msgnochat >= message__head_msgnochat) message__tail_msgnochat = 0;

                /* Start over */
                message__head_msgnochat = 0;
        }


        /*** Step 4 -- Ensure space before next message ***/

        /* Kill messages if needed */
        if (message__head_msgnochat + n + 1 > message__tail_msgnochat)
        {
                /* Grab new "tail" */
                message__tail_msgnochat = message__head_msgnochat + n + 1;

                /* Advance tail while possible past first "nul" */
                while (message__buf_msgnochat[message__tail_msgnochat-1]) message__tail_msgnochat++;

                /* Kill all "dead" messages */
                for (i = message__last_msgnochat; TRUE; i++)
                {
                        /* Wrap if needed */
                        if (i == MESSAGE_MAX) i = 0;

                        /* Stop before the new message */
                        if (i == message__next_msgnochat) break;

                        /* Kill "dead" messages */
                        if ((message__ptr_msgnochat[i] >= message__head_msgnochat) &&
                            (message__ptr_msgnochat[i] < message__tail_msgnochat))
                        {
                                /* Track oldest message */
                                message__last_msgnochat = i + 1;
                        }
                }
        }


        /*** Step 5 -- Grab a new message index ***/

        /* Get the next message index, advance */
        x = message__next_msgnochat++;

        /* Handle wrap */
        if (message__next_msgnochat == MESSAGE_MAX) message__next_msgnochat = 0;

        /* Kill last message if needed */
        if (message__next_msgnochat == message__last_msgnochat) message__last_msgnochat++;

        /* Handle wrap */
        if (message__last_msgnochat == MESSAGE_MAX) message__last_msgnochat = 0;



        /*** Step 6 -- Insert the message text ***/

        /* Assign the starting address */
        message__ptr_msgnochat[x] = message__head_msgnochat;

        /* Append the new part of the message */
        for (i = 0; i < n; i++)
        {
                /* Copy the message */
                message__buf_msgnochat[message__head_msgnochat + i] = str[i];
        }

        /* Terminate */
        message__buf_msgnochat[message__head_msgnochat + i] = '\0';

        /* Advance the "head" pointer */
        message__head_msgnochat += n + 1;

	/* Window stuff - assume that all chat messages start with '[' */
        p_ptr->window |= (PW_MSGNOCHAT);
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
 * The global "msg_flag" variable can be cleared to tell us to
 * "erase" any "pending" messages still on the screen.
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
void c_msg_print(cptr msg) {
	int n, x, y;
	char buf[1024];
	char *t;

	/* using clear_topline() here prevents top-line clearing via c_msg_print(NULL) */
	if (!topline_icky) clear_topline();

	/* No message */
	if (!msg) return;

	/* Copy it */
	strcpy(buf, msg);
	/* mark beginning of text to output */
	t = buf;

	/* Remember cursor position */
	Term_locate(&x, &y);

	/* Message length */
	n = strlen(msg);

	/* Paranoia */
	if (n > 1000) return;

	/* Memorize the message */
	if (msg[0] == '\375') {
		was_chat_buffer = TRUE;
		was_all_buffer = FALSE;
	} else if (msg[0] == '\374') {
		was_all_buffer = TRUE;
		was_chat_buffer = FALSE;
	} else {
		was_chat_buffer = FALSE;
		was_all_buffer = FALSE;
	}

	/* strip (from here on) effectless control codes */
	if (*t == '\374') t++;
	if (*t == '\375') t++;

	/* add the message to generic, chat-only and no-chat buffers
	   accordingly, KEEPING '\376' control code (v>4.4.2.4) for main buffer */
	c_message_add(t);

	/* strip remaining control codes before displaying on screen */
	if (*t == '\376') t++;

	if (was_chat_buffer || was_all_buffer)
		c_message_add_chat(t);
	if (!was_chat_buffer)
		c_message_add_msgnochat(t);

	/* Small length limit */
	if (n > 80) n = 80;

	/* Display the tail of the message */
	if (!topline_icky) Term_putstr(0, 0, n, TERM_WHITE, t);

	/* Restore cursor */
	Term_gotoxy(x, y);

	/* Remember the message */
	msg_flag = TRUE;
}

/*
 * Display a formatted message, using "vstrnfmt()" and "c_msg_print()".
 */
void c_msg_format(cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	c_msg_print(buf);
}


/*
 * Request a "quantity" from the user
 *
 * Hack -- allow "command_arg" to specify a quantity
 */
#define QUANTITY_WIDTH 10
s32b c_get_quantity(cptr prompt, s32b max) {
	s32b amt;
	char tmp[80];
	char buf[80];

	char bi1[80], bi2[6 + 1];
	int n = 0, i = 0, j = 0;
	s32b i1 = 0, i2 = 0, mul = 1;

	/* Build a prompt if needed */
	if (!prompt) {
		/* Build a prompt */
		inkey_letter_all = TRUE;
		sprintf(tmp, "Quantity (1-%d, 'a' for all): ", max);

		/* Use that prompt */
		prompt = tmp;
	}


	/* Default to one */
	amt = 1;

	/* Build the default */
	sprintf(buf, "%d", amt);

	/* Ask for a quantity */
	if (!get_string(prompt, buf, QUANTITY_WIDTH)) return (0);


#if 1
	/* new method slightly extended: allow leading 'k' or 'm' too */
	if ((buf[0] == 'k' || buf[0] == 'K' || buf[0] == 'm' || buf[0] == 'M') && buf[1]) {
		/* add leading '0' to revert it to the usual format */
		for (i = QUANTITY_WIDTH + 1; i >= 1; i--) buf[i] = buf[i - 1];
		buf[0] = '0';
	}
	/* new method for inputting amounts of gold:  1m35 = 1,350,000  - C. Blue */
	while (buf[n] >= '0' && buf[n] <= '9') bi1[i++] = buf[n++];
	bi1[i] = '\0';
	i1 = atoi(bi1);
	if ((buf[n] == 'k' || buf[n] == 'K') && n > 0) mul = 1000;
	else if (buf[n] == 'm' || buf[n] == 'M') mul = 1000000;
	if (mul > 1) {
		n++;
		i = 0;
		while(buf[n] >= '0' && buf[n] <= '9' && i < 6) bi2[i++] = buf[n++];
		bi2[i] = '\0';
//Send_msg(format("%s-%s", bi1, bi2));

		i = 0;
		while (i < 6) {
			if (bi2[i] == '\0') {
				j = i;
				while (j < 6) bi2[j++] = '0';
				break;
			}
			i++;
		}
//Send_msg(format("%s-%d", bi2, mul));

		if (mul == 1000) bi2[3] = '\0';
		else if (mul == 1000000) bi2[6] = '\0';

		i2 = atoi(bi2);
		amt = i1 * mul + i2;
	} else amt = i1;

#else

	/* Extract a number */
	amt = atoi(buf);

	/* Analyse abreviation like '15k' */
	if (strchr(buf, 'k'))
		amt = amt * 1000;
	else if (strchr(buf, 'm') || strchr(buf, 'M')
		amt = amt * 1000000;
#endif


	/* 'a' means "all" - C. Blue */
	//if (isalpha(buf[0])) {
	if (buf[0] == 'a') {
		if (max >= 0) amt = max;
		/* hack for dropping gold (max is -1) */
		else amt = 1000000000;
	}

	/* Enforce the maximum, if maximum is defined */
	if ((max >= 0) && (amt > max)) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

	/* Return the result */
	return (amt);
}

void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}

void prt_num(cptr header, int num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];
	put_str(header, row, col);
	put_str("   ", row, col + len);
	(void)sprintf(out_val, "%6d", num);
	c_put_str(color, out_val, row, col + len + 3);
}

void prt_lnum(cptr header, s32b num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];
	put_str(header, row, col);
	(void)sprintf(out_val, "%9d", (int)num);
	c_put_str(color, out_val, row, col + len);
}


static void ascii_to_text(char *buf, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		byte i = (byte)(*str++);

		if (i == ESCAPE)
		{
			*s++ = '\\';
			*s++ = 'e';
		}
		else if (i == MACRO_WAIT)
		{
			*s++ = '\\';
			*s++ = 'w';
		}
		else if (i == ' ')
		{
			*s++ = '\\';
			*s++ = 's';
		}
		else if (i == '\b')
		{
			*s++ = '\\';
			*s++ = 'b';
		}
		else if (i == '\t')
		{
			*s++ = '\\';
			*s++ = 't';
		}
		else if (i == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (i == '\r')
		{
			*s++ = '\\';
			*s++ = 'r';
		}
		else if (i == '^')
		{
			*s++ = '\\';
			*s++ = '^';
		}
		else if (i == '\\')
		{
			*s++ = '\\';
			*s++ = '\\';
		}
		else if (i < 32)
		{
			*s++ = '^';
			*s++ = i + 64;
		}
		else if (i < 127)
		{
			*s++ = i;
		}
		else if (i < 64)
		{
			*s++ = '\\';
			*s++ = '0';
			*s++ = octify(i / 8);
			*s++ = octify(i % 8);
		}
		else
		{
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify(i / 16);
			*s++ = hexify(i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}

static errr macro_dump(cptr fname)
{
	int i, j, n;

	FILE *fff;

	char buf[1024];
	char buf2[4096];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* Check if the file already exists */
	fff = my_fopen(buf, "r");

	if (fff) {
		fclose(fff);

		/* Attempt to rename */
		strcpy(buf2, buf);
		strncat(buf2, ".bak", 1023);
		rename(buf, buf2);
	}

	/* Write to the file */
	fff = my_fopen(buf, "w");

	/* Failure */
	if (!fff) return (-1);


	/* Skip space */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic macro dump\n\n");

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		fprintf(fff, "# Macro '%d'\n\n", i);

#if 0
		/* Extract the action */
		ascii_to_text(buf, macro__act[i]);

		/* Dump the macro */
		fprintf(fff, "A:%s\n", buf);
#else
		/* Support really long macros - mikaelh */
		fprintf(fff, "A:");

		for (j = 0, n = strlen(macro__act[i]); j < n; j += 1023)
		{
			/* Take a piece of the action */
			strncpy(buf, &macro__act[i][j], 1023);
			buf[1023] = '\0';

			/* Convert it */
			ascii_to_text(buf2, buf);

			/* Dump it */
			fprintf(fff, "%s", buf2);
		}

		fprintf(fff, "\n");
#endif

		/* Extract the action */
		ascii_to_text(buf, macro__pat[i]);

		/* Dump command macro */
		if (macro__hyb[i]) fprintf(fff, "H:%s\n", buf);
		else if (macro__cmd[i]) fprintf(fff, "C:%s\n", buf);
		/* Dump normal macros */
		else fprintf(fff, "P:%s\n", buf);

		/* End the macro */
		fprintf(fff, "\n\n");
	}

	/* Finish dumping */
	fprintf(fff, "\n\n\n\n");

	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}

static void get_macro_trigger(char *buf)
{
	int i, n = 0;

	char tmp[1024];

	/* Flush */
	flush();

	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	i = inkey();

	/* Read the pattern */
	while (i)
	{
		/* Save the key */
		buf[n++] = i;

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		i = inkey();
	}

	/* Terminate */
	buf[n] = '\0';

	/* Flush */
	flush();


	/* Convert the trigger */
	ascii_to_text(tmp, buf);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);
}

void interact_macros(void)
{
	int i, j = 0, l;

	char tmp[160], buf[1024], buf2[1024], *bptr, *b2ptr, chain_macro_buf[1024];

	char fff[1024], t_key[10], choice;
	bool m_ctrl, m_alt, m_shift, t_hyb, t_com;

	bool were_recording = FALSE;


	/* Save screen */
	Term_save();

	/* No macros should work within the macro menu itself */
	inkey_interact_macros = TRUE;

	/* Did we just finish recording a macro by entering this menu? */
	if (recording_macro) {
		/* stop recording */
		were_recording = TRUE;
		recording_macro = FALSE;
		/* cut off the last key which was '%' to bring us back here */
		recorded_macro[strlen(recorded_macro) - 1] = '\0';
		/* use the recorded macro */
		strcpy(macro__buf, recorded_macro);
	}

	/* Process requests until done */
	while (1) {
		/* Clear screen */
		Term_clear();

		/* Describe */
		Term_putstr(0, 0, -1, TERM_L_GREEN, "Interact with Macros");


		/* Selections */
		l = 2;
		Term_putstr(5, l++, -1, TERM_L_BLUE, "(\377yz\377B) Invoke macro wizard         *** Recommended ***");
		Term_putstr(5, l++, -1, TERM_WHITE, "(\377ys\377w) Save macros to a pref file");
		Term_putstr(5, l++, -1, TERM_WHITE, "(\377yl\377w) Load macros from a pref file");
		l++;
		Term_putstr(5, l++, -1, TERM_WHITE, "(\377yd\377w) Delete a macro from a key   (restores a key's normal behaviour)");
		Term_putstr(5, l++, -1, TERM_WHITE, "(\377yt\377w) Test a key for an existing macro");
		Term_putstr(5, l++, -1, TERM_WHITE, "(\377yi\377w) List all currently defined macros");
		l++;
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377ua\377s) Enter a new macro action manually. Afterwards..");
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377uh\377s) ..create a hybrid macro     (usually preferable over command/normal)");
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377uc\377s) ..create a command macro    (eg for using / and * key)");
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377un\377s) ..create a normal macro     (persists everywhere, even in chat)");
//		Term_putstr(5, l++, -1, TERM_SLATE, "(\377u4\377s) Create a identity macro  (erases a macro)");
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377ue\377s) Create an empty macro       (completely disables a key)");
//		Term_putstr(5, l++, -1, TERM_SLATE, "(\377uq\377s/\377yQ\377w) Enter and create a 'quick & dirty' macro / set preferences"),
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377uq\377s) Enter and create a 'quick & dirty' macro"),
//		Term_putstr(5, l++, -1, TERM_SLATE, "(\377r\377w/\377yR\377w) Record a macro / set preferences");
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377ur\377s) Record a macro");
		Term_putstr(5, l++, -1, TERM_SLATE, "(\377uW\377s) Wipe all macros and (\377uw\377s) reload default macro files (use together!)");
		l++;

		/* Describe that action */
		Term_putstr(0, l + 3, -1, TERM_L_GREEN, "Current action (if any) shown below:");

		/* Analyze the current action */
		ascii_to_text(buf, macro__buf);

		/* Display the current action */
		Term_putstr(0, l + 4, -1, TERM_WHITE, buf);

		/* Prompt */
		Term_putstr(0, l, -1, TERM_L_GREEN, "Command: ");

		/* Get a key */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Allow to chat, to tell exact macros to other people easily */
		else if (i == ':') cmd_message();

		/* Take a screenshot */
		else if (i == KTRL('T')) xhtml_screenshot("screenshot????");

		/* Load a pref file */
		else if (i == 'l') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Load a user pref file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "File: ");

			/* Default filename */
//			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);
			/* Use the character name by default - mikaelh */
			sprintf(tmp, "%s.prf", cname);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);

			/* Pref files may change settings, so reload the keymap - mikaelh */
			keymap_init();
		}

		/* Save a 'macro' file */
		else if (i == 's') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Save a macro file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "File: ");

			/* Default filename */
//			sprintf(tmp, "user-%s.prf", ANGBAND_SYS);
			/* Use the character name by default - mikaelh */
			sprintf(tmp, "%s.prf", cname);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Dump the macros */
			(void)macro_dump(tmp);
		}

		/* Enter a new action */
		else if (i == 'a') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Enter a new action");

			/* Go to the correct location */
			Term_gotoxy(0, l + 4);

			/* Get an encoded action */
			if (!askfor_aux(buf, 159, 0)) continue;

			/* Extract an action */
			text_to_ascii(macro__buf, buf);
		}

		/* Create a command macro */
		else if (i == 'c') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Create a command macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Some keys aren't allowed to prevent the user 
			   from locking himself out accidentally */
			if (!strcmp(buf, "\e") || !strcmp(buf, "%")) {
				c_msg_print("\377yKeys <ESC> and '%' aren't allowed to carry a macro.");
			} else {
				/* Link the macro */
				macro_add(buf, macro__buf, TRUE, FALSE);
				/* Message */
				c_msg_print("Created a new command macro.");
			}
		}

		/* Create a hybrid macro */
		else if (i == 'h') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Create a hybrid macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Some keys aren't allowed to prevent the user 
			   from locking himself out accidentally */
			if (!strcmp(buf, "\e") || !strcmp(buf, "%")) {
				c_msg_print("\377yKeys <ESC> and '%' aren't allowed to carry a macro.");
			} else {
				/* Link the macro */
				macro_add(buf, macro__buf, FALSE, TRUE);
				/* Message */
				c_msg_print("Created a new hybrid macro.");
			}
		}

		/* Create a normal macro */
		else if (i == 'n') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Create a normal macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Some keys aren't allowed to prevent the user 
			   from locking himself out accidentally */
			if (!strcmp(buf, "\e") || !strcmp(buf, "%")) {
				c_msg_print("\377yKeys <ESC> and '%' aren't allowed to carry a macro.");
			} else {
				/* Link the macro */
				macro_add(buf, macro__buf, FALSE, FALSE);
				/* Message */
				c_msg_print("Created a new normal macro.");
			}
		}

#if 0
		/* Create an identity macro */
		else if (i == 'd') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Create an identity macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Link the macro */
			macro_add(buf, buf, FALSE, FALSE);

			/* Message */
			c_msg_print("Created a new identity macro.");
		}
#else
		/* Delete a macro */
		else if (i == 'd') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Delete a macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Delete the macro */
			if (macro_del(buf))
				c_msg_print("The macro was deleted.");
			else
				c_msg_print("No macro was found.");
		}
#endif

		/* Create an empty macro */
		else if (i == 'e') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Create an empty macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Some keys aren't allowed to prevent the user 
			   from locking himself out accidentally */
			if (!strcmp(buf, "\e") || !strcmp(buf, "%")) {
				c_msg_print("\377yKeys <ESC> and '%' aren't allowed to carry a macro.");
			} else {
				/* Link the macro */
				macro_add(buf, "", FALSE, FALSE);
				/* Message */
				c_msg_print("Created a new empty macro.");
			}
		}

		/* Query a macro */
		else if (i == 't') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Test key for a macro");

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Re-using 'i' here shouldn't matter anymore */
			for (i = 0; i < macro__num; i++)
			{
				if (streq(macro__pat[i], buf))
				{
					strncpy(macro__buf, macro__act[i], 159);
					macro__buf[159] = '\0';

					/* Message */
					if (macro__hyb[i]) c_msg_print("A hybrid macro was found.");
					else if (macro__cmd[i]) c_msg_print("A command macro was found.");
					else {
						if (!macro__act[i][0]) c_msg_print("An empty macro was found.");
						else c_msg_print("A normal macro was found.");
					}

					/* Update windows */
					window_stuff();

					break;
				}
			}

			if (i == macro__num) {
				/* Message */
				c_msg_print("No macro was found.");
			}
		}

		/* List all macros */
		else if (i == 'i') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: List all macros");

			/* Re-using 'i' here shouldn't matter anymore */
			for (i = 0; i < macro__num; i++) {
				if (i % 20 == 0) {
					/* Clear screen */
					Term_clear();

					/* Describe */
					Term_putstr(0, 1, -1, TERM_L_UMBER, format("  *** Current Macro List (Entry %d-%d) ***", i + 1, i + 20 > macro__num ? macro__num : i + 20));
					Term_putstr(0, 22, -1, TERM_L_UMBER, "  [Press any key to continue, 'p' for previous, ESC to exit]");
				}

				/* Get trigger in parsable format */
				ascii_to_text(buf, macro__pat[i]);

				/* Get macro in parsable format */
				strncpy(macro__buf, macro__act[i], 159);
				macro__buf[159] = '\0';
				ascii_to_text(buf2, macro__buf);

				/* Make the trigger human-readable */
				m_ctrl = m_alt = m_shift = FALSE;
				if (strlen(buf) == 1) {
					/* just a simple key */
					sprintf(t_key, "%c        ", buf[0]);
				} else if (strlen(buf) == 2) {
					/* '^%c' : ctrl + a simple key */
					m_ctrl = TRUE; /* not testing for the '^' actually, we assume it must be there.. */
					sprintf(t_key, "%c        ", buf[1]);
				} else {
					/* special key, possibly with shift and/or ctrl */
					bptr = buf + 2;
					if (*bptr == 'N') { m_ctrl = TRUE; bptr++; }
					if (*bptr == 'S') { m_shift = TRUE; bptr++; }
					if (*bptr == 'O') { m_alt = TRUE; bptr++; }
					bptr++;
					sprintf(t_key, "%-9.9s", bptr);
					/* ensure termination (in paranoid case of overflow) */
					t_key[9] = '\0';
				}

				/* determine trigger type */
				t_hyb = t_com = FALSE;
				if (macro__hyb[i]) t_hyb = TRUE;
				else if (macro__cmd[i]) t_com = TRUE;

				/* build a whole line */
				sprintf(fff, "%s %s %s %s [%s]  %-49.49s", m_shift ? "SHF" : "   ", m_ctrl ? "CTL" : "   ", m_alt ? "ALT" : "   ",
				    t_key, t_hyb ? "HY" : t_com ? "C " : "  ", buf2);

				Term_putstr(0, i % 20 + 2, -1, TERM_WHITE, fff);

				/* Wait for keypress before displaying more */
				if ((i % 20 == 19) || (i == macro__num - 1)) switch (inkey()) {
					case ESCAPE:
						i = -2; /* hack to leave for loop */
						break;
					case 'p':
					case '\010': /* backspace */
						if (i >= 39) {
							/* show previous 20 entries */
							i -= 20 + (i % 20) + 1;
						} else { /* wrap around */
							i = macro__num - (macro__num % 20) - 1;
							if (i == macro__num - 1) i -= 20;
						}
						break;
					case KTRL('T'):
						/* Take a screenshot */
						xhtml_screenshot("screenshot????");
						/* keep current list */
						i -= (i % 20) + 1;
						break;
					default:
						/* show next 20 entries */
						if (i == macro__num - 1) i = -1; /* restart list at 1st macro again */
				}
				if (i == -2) break;
			}

			if (i == 0) {
				/* Message */
				c_msg_print("No macro was found.");
			}
		}

		/* Enter a 'quick & dirty' macro */
		else if (i == 'q') {
			bool call_by_name = FALSE, mimic_transform = FALSE, mimic_transform_by_name = FALSE;

			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Enter a new 'quick & dirty' macro");

			/* Go to the correct location */
			Term_gotoxy(0, l + 4);

			/* Get an encoded action */
			if (!askfor_aux(buf, 159, 0)) continue;

			/* Fix it up quick and dirty: Ability code short-cutting */
			buf2[0] = '\\'; //note: should in theory be ')e\',
			buf2[1] = 'e'; //      but doesn't work due to prompt behaviour 
			buf2[2] = ')'; //      (\e will then get ignored)
			bptr = buf;
			b2ptr = buf2 + 3;
			while (*bptr) {
				if (call_by_name) {
					if (*bptr == '\\') {
						call_by_name = FALSE;
						*b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;
					} else {
						*b2ptr++ = *bptr++;
					}
				} else {
					/* service: mimic-transformation automatically adds the required '\r' at the end */
					switch (*bptr) {
					case 'P': case 'I': case 'F': case 'S': case 'T': case 'M': case 'R': case 's':
						if (mimic_transform) {
							if (!mimic_transform_by_name) {
								*b2ptr++ = '\\'; *b2ptr++ = 'r';
							}
							mimic_transform = mimic_transform_by_name = FALSE;
						}
						break;
					default:
						if (mimic_transform && ((*bptr) < '0' || (*bptr) > '9')) {
							*b2ptr++ = '\\'; *b2ptr++ = 'r';
							mimic_transform = FALSE;
						}
					}

					switch (*bptr) {
					case 'P': /* use innate mimic power */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '3'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
					case 'I': /* use innate mimic power: transform into specific */
						mimic_transform = TRUE;
						if (*(bptr + 1) == '@') mimic_transform_by_name = TRUE;
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '3'; *b2ptr++ = '\\'; *b2ptr++ = 'r'; *b2ptr++ = 'c';
						bptr++;	break;
					case 'F': /* employ fighting technique */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '5'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
					case 'S': /* employ shooting technique */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '6'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
					case 'T': /* set a trap */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '1'; *b2ptr++ = '0'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
					case 'M': /* cast a spell */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '1'; *b2ptr++ = '1'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
					case 'R': /* draw a rune */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '1'; *b2ptr++ = '2'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
					case 's': /* change stance */
						*b2ptr++ = 'm'; *b2ptr++ = '@'; *b2ptr++ = '1'; *b2ptr++ = '3'; *b2ptr++ = '\\'; *b2ptr++ = 'r';
						bptr++;	break;
#if 0 /* disabled, to allow @q */
					case '*': /* set a target */
						*b2ptr++ = '*'; *b2ptr++ = 't';
						bptr++;	break;
#endif
					case '@': /* start 'call-by-name' mode, reading the spell/item name next */
						call_by_name = TRUE;
						*b2ptr++ = '@';
						bptr++;	break;
					default:
						*b2ptr++ = *bptr++;
					}
				}
			}

			/* service: mimic-transformation automatically adds the required '\r' at the end */
			if (mimic_transform) {
				if (!mimic_transform_by_name) {
					*b2ptr++ = '\\'; *b2ptr++ = 'r';
				}
			}

			/* terminate anyway */
			*b2ptr = '\0';

			/* Display the current action */
        		Term_putstr(0, l + 4, -1, TERM_WHITE, buf2);

			/* Extract an action */
			text_to_ascii(macro__buf, buf2);

			/* Prompt */
			Term_putstr(0, l + 1, -1, TERM_WHITE, "Trigger: ");

			/* Get a macro trigger */
			get_macro_trigger(buf);

			/* Some keys aren't allowed to prevent the user 
			   from locking himself out accidentally */
			if (!strcmp(buf, "\e") || !strcmp(buf, "%")) {
				c_msg_print("\377yKeys <ESC> and '%' aren't allowed to carry a macro.");
			} else {
				/* Automatically choose usually best fitting macro type,
				   depending on chosen trigger key! */
				//[normal macros: F-keys (only keys that aren't used for any text input)]
				//command macros: / * a..w (all keys that are used in important standard prompts)
				//hybrid macros: all others, maybe even also normal-macro-keys
				if (!strcmp(buf, "/") || !strcmp(buf, "*") || /* windows */
				    (strlen(buf) == 10 && (buf[8] == '/' || buf[8] == '*')) || /* x11 - to do: fix this crap */
				    (*buf >= 'a' && *buf <= 'w')) {
					/* make it a command macro */
					/* Link the macro */
					macro_add(buf, macro__buf, TRUE, FALSE);
					/* Message */
					c_msg_print("Created a new command macro.");
				} else {
					/* make it a hybrid macro */
					/* Link the macro */
					macro_add(buf, macro__buf, FALSE, TRUE);
					/* Message */
					c_msg_print("Created a new hybrid macro.");
				}
			}
		}

		/* Configure 'quick & dirty' macro functionality */
		else if (i == 'Q') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Configure 'quick & dirty' macro functionality");

			/* TODO:
			   config auto-prefix '\e)' */
		}

		/* Start recording a macro */
		else if (i == 'r') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Record a macro");

			/* Clear screen */
			Term_clear();

			/* Describe */
			Term_putstr(25, 4, -1, TERM_L_RED, "*** Recording a macro ***");
			Term_putstr(8, 8, -1, TERM_WHITE, "As soon as you press any key, the macro recorder will begin to");
			Term_putstr(15, 9, -1, TERM_WHITE, "record all further key presses you will perform.");
			Term_putstr(5, 11, -1, TERM_WHITE, "These can afterwards be saved to a macro, that you can bind to a key.");
			Term_putstr(5, 13, -1, TERM_WHITE, "To stop the recording process, just press '%' key to enter the macro");
			Term_putstr(5, 14, -1, TERM_WHITE, "menu again. You'll then be able to create a normal, command or hybrid");
			Term_putstr(4, 15, -1, TERM_WHITE, "macro from the whole recorded action by choosing the usual menu points");
			Term_putstr(16, 16, -1, TERM_WHITE, "for the different macro types: h), c) or n).");
			Term_putstr(19, 20, -1, TERM_L_RED, ">>>Press any key to start recording<<<");

			/* Wait for confirming keypress to finally start recording */
			inkey();

			/* Reload screen */
			Term_load();

			/* Flush the queue */
			Flush_queue();

			/* leave macro menu */
			inkey_interact_macros = FALSE;
			/* hack: Display recording status */
			Send_redraw(0);

			/* enter recording mode */
			strcpy(recorded_macro, "");
			recording_macro = TRUE;

			/* leave the macro menu */
			return;
		}

		/* Configure macro recording functionality */
		else if (i == 'R') {
			/* Prompt */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Configure macro recording functionality");

			/* TODO: implement */
		}

		else  if (i == 'W') {
			/* Wipe all macros */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Wipe all macros");

			for (i = 0; i < macro__num; i++) {
				string_free(macro__pat[i]);
				macro__pat[i] = NULL;
				string_free(macro__act[i]);
				macro__act[i] = NULL;
				macro__cmd[i] = FALSE;
				macro__hyb[i] = FALSE;
			}
			macro__num = 0;
			for (i = 0; i < 256; i++) macro__use[i] = 0;
		}

		else if (i == 'w') {
			/* Reload default macro files */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Reload default macro files");

			initialize_main_pref_files();
			initialize_player_pref_files();
		}

		else if (i == 'z') {
			int target_dir = '5';
#define mw_quaff 'a'
#define mw_read 'b'
#define mw_fire 'c'
#define mw_throw 'C'
#define mw_schoolnt 'd'
#define mw_mimicnt 'D'
#define mw_schoolt 'e'
#define mw_mimict 'E'
#define mw_mimicidx 'f'
#define mw_poly 'g'
#define mw_prfimm 'G'
#define mw_rune 'h'
#define mw_fight 'i'
#define mw_shoot 'j'
#define mw_trap 'k'
#define mw_device 'l'
#define mw_abilitynt 'm'
#define mw_abilityt 'M'
#define mw_custom 'n'
#define mw_load 'o'

#define mw_LAST 'o'

			/* Invoke wizard to create a macro step-by-step as easy as possible  */
			Term_putstr(0, l, -1, TERM_L_GREEN, "Command: Invoke macro wizard");

			/* initialise macro-chaining */
			chain_macro_buf[0] = 0;
Chain_Macro:

			/* Clear screen */
			Term_clear();

			/* Describe */
			Term_putstr(29, 0, -1, TERM_L_UMBER, "*** Macro Wizard ***");
			//Term_putstr(25, 22, -1, TERM_L_UMBER, "[Press ESC to exit anytime]");
			Term_putstr(1, 6, -1, TERM_L_DARK, "Don't forget to save your macros with 's' when you are back in the macro menu!");
			Term_putstr(19, 7, -1, TERM_L_UMBER, "----------------------------------------");

			/* Initialize wizard state: First state */
			i = choice = 0;

			while (i != -1) {
				Term_putstr(12, 2, -1, i == 0 ? TERM_L_GREEN : TERM_SLATE, "Step 1:  Choose an action for the macro to perform.");
				Term_putstr(12, 3, -1, i == 1 ? TERM_L_GREEN : TERM_SLATE, "Step 2:  If required, choose item, spell, and targetting method.");
				Term_putstr(12, 4, -1, i == 2 ? TERM_L_GREEN : TERM_SLATE, "Step 3:  Choose the key you want to bind the macro to.");

				clear_from(8);

				switch (i) {
				case 0:
					Term_putstr( 5, 9, -1, TERM_GREEN, "Which of the following actions should the macro perform?");
					Term_putstr(10, 10, -1, TERM_L_GREEN, "a/b) Drink a potion/read a scroll");
					Term_putstr(10, 11, -1, TERM_L_GREEN, "c)/C) Fire ranged weapon/throw an item");
					Term_putstr(10, 12, -1, TERM_L_GREEN, "d)/D) Cast school/mimic spell without a target (or target manually)");
					Term_putstr(10, 13, -1, TERM_L_GREEN, "e)/E) Cast school/mimic spell with target");
					Term_putstr(10, 14, -1, TERM_L_GREEN, "f) Cast a mimic spell by number (with and without target)");
					Term_putstr(10, 15, -1, TERM_L_GREEN, "g/G) Polymorph into monster/set preferred immunity (mimicry users)");
					Term_putstr(10, 16, -1, TERM_L_GREEN, "h) Draw runes to cast a runespell");
					Term_putstr(10, 17, -1, TERM_L_GREEN, "i) Use a fighting technique (most melee classes)");
					Term_putstr(10, 18, -1, TERM_L_GREEN, "j) Use a shooting technique (archers and rangers)");
					Term_putstr(10, 19, -1, TERM_L_GREEN, "k) Set up a monster trap");
					Term_putstr(10, 20, -1, TERM_L_GREEN, "l) Use a magic device");
					Term_putstr(10, 21, -1, TERM_L_GREEN, "m)/M) Use a basic ability ('m') without/with target");
					Term_putstr(10, 22, -1, TERM_L_GREEN, "n) Enter a custom action (same as pressing 'a' in macro screen)");
					Term_putstr(10, 23, -1, TERM_L_GREEN, "o) Load a macro file");

					while (TRUE) {
						switch (choice = inkey()) {
						case ESCAPE:
						case 'p':
						case '\010': /* backspace */
							i = -1; /* leave */
							break;
						case KTRL('T'):
							/* Take a screenshot */
							xhtml_screenshot("screenshot????");
							continue;
						default:
							/* invalid action -> exit wizard */
							if ((choice < 'a' || choice > mw_LAST) && choice != 'C' && choice != 'D' && choice != 'E' && choice != 'M' && choice != 'G') {
//								i = -1;
								continue;
							}
						}
						break;
					}
					/* exit? */
					if (i == -1) continue;

					/* advance to next step */
					i++;
					break;
				case 1:
					switch (choice) {
					case mw_quaff:
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter a distinctive part of the potion's name or inscription.");
						//Term_putstr(5, 11, -1, TERM_GREEN, "and pay attention to upper-case and lower-case letters!");
						Term_putstr(5, 11, -1, TERM_GREEN, "For example, enter:     \377GCritical Wounds");
						Term_putstr(5, 12, -1, TERM_GREEN, "if you want to quaff a 'Potion of Cure Critical Wounds'.");
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Enter partial potion name or inscription:");
						break;
					case mw_read:
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter a distinctive part of the scroll's name or inscription.");
						//Term_putstr(5, 11, -1, TERM_GREEN, "and pay attention to upper-case and lower-case letters!");
						Term_putstr(5, 11, -1, TERM_GREEN, "For example, enter:     \377GPhase Door");
						Term_putstr(5, 12, -1, TERM_GREEN, "if you want to read a 'Scroll of Phase Door'.");
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Enter partial scroll name or inscription:");
						break;
					case mw_schoolnt:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact spell name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GPhase Door");
						Term_putstr(10, 12, -1, TERM_GREEN, "You must have learned a spell before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact spell name:");
						break;
					case mw_mimicnt:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact spell name.");//and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GBlink");
						Term_putstr(10, 12, -1, TERM_GREEN, "You must have learned a spell before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact spell name:");
						break;
					case mw_schoolt:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact spell name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GManathrust");
						Term_putstr(10, 12, -1, TERM_GREEN, "You must have learned a spell before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact spell name:");
						break;
					case mw_mimict:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact spell name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GMagic Missile");
						Term_putstr(10, 12, -1, TERM_GREEN, "You must have learned a spell before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact spell name:");
						break;
					case mw_mimicidx:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter a spell number, starting from 1, which is");
						Term_putstr(10, 11, -1, TERM_GREEN, "the first spell after the 3 basic powers and immunity");
						Term_putstr(10, 12, -1, TERM_GREEN, "preference setting, which always occupy spell slots a)-d).");
						Term_putstr(10, 13, -1, TERM_GREEN, "So \377G1\377g = spell e), \377G2\377g = f), \377G3\377g = g), \377G4\377g = h) etc.");
						Term_putstr(10, 14, -1, TERM_GREEN, "You must have learned a spell before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter spell index number:");
						break;
					case mw_fight:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact technique name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GSprint");
						Term_putstr(10, 12, -1, TERM_GREEN, "You must have learned a technique before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact technique name:");
						break;
					case mw_shoot:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact technique name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GFlare missile");
						Term_putstr(10, 12, -1, TERM_GREEN, "You must have learned a technique before you can use it!");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact technique name:");
						break;
					case mw_poly:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact monster name OR its code. (You can find");
						Term_putstr(10, 11, -1, TERM_GREEN, "codes you have already learned by pressing  \377s~ 2  \377gin the game");
						Term_putstr(10, 12, -1, TERM_GREEN, "or by pressing  \377s:  \377gto chat and then typing the command:  \377s/mon");
						Term_putstr(10, 13, -1, TERM_GREEN, "The first number on the left, in parentheses, is what you need.)");
						Term_putstr(10, 14, -1, TERM_GREEN, "For example, enter  \377GFruit bat\377g  or just  \377G37  \377gto transform into one.");
						Term_putstr(10, 15, -1, TERM_GREEN, "To return to your normal form, use  \377GPlayer\377g  or its code  \377G0\377g  .");
//						Term_putstr(10, 15, -1, TERM_GREEN, "You must have learned a form before you can use it!");
						Term_putstr(1, 17, -1, TERM_L_GREEN, "Enter exact monster name/code or leave blank:");
						break;
					case mw_prfimm:
						Term_putstr(5, 10, -1, TERM_GREEN, "Please choose an immunity preference:");
						Term_putstr(5, 11, -1, TERM_GREEN, "\377Ga\377g) Lightning  \377Gb\377g) Frost  \377Gc\377g) Acid  \377Gd\377g) Fire  \377Ge\377g) Poison  \377Gf\377g) Water");
						Term_putstr(5, 12, -1, TERM_GREEN, "\377G*\377g) None (pick one randomly on polymorphing)");
						Term_putstr(5, 13, -1, TERM_GREEN, "\377G?\377g) Just check (displays your current immunity preference)");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Pick one (a-f,*,?):");

						while (TRUE) {
							switch (choice = inkey()) {
							case ESCAPE:
							case 'p':
							case '\010': /* backspace */
								i = -1; /* leave */
								break;
							case KTRL('T'):
								/* Take a screenshot */
								xhtml_screenshot("screenshot????");
								continue;
							default:
								/* invalid action -> exit wizard */
								if ((choice < 'a' || choice > 'f') && choice != '?' && choice != '*') {
//									i = -1;
									continue;
								}
							}
							break;
						}
						/* exit? */
						if (i == -1) continue;

						/* build macro part */
						switch (choice) {
						case 'a': strcpy(buf2, "\\e)m@3\rd@Lightning\r"); break;
						case 'b': strcpy(buf2, "\\e)m@3\rd@Frost\r"); break;
						case 'c': strcpy(buf2, "\\e)m@3\rd@Acid\r"); break;
						case 'd': strcpy(buf2, "\\e)m@3\rd@Fire\r"); break;
						case 'e': strcpy(buf2, "\\e)m@3\rd@Poison\r"); break;
						case 'f': strcpy(buf2, "\\e)m@3\rd@Water\r"); break;
						case '*': strcpy(buf2, "\\e)m@3\rd@None\r"); break;
						case '?': strcpy(buf2, "\\e)m@3\rd@Check\r"); break;
						}

						choice = mw_prfimm; /* hack - remember */
						break;
					case mw_rune: //Hardcoded, so must be maintained. - Kurzel
						strcpy(buf2, "");
						strcpy(buf, "");
						u16b e_flags = 0;
						u16b m_flags = 0;

						/* ---------- Draw runes ---------- */

						Term_putstr(10, 10, -1, TERM_GREEN, "Please choose the component runes of the spell.");
						Term_putstr(10, 11, -1, TERM_GREEN, "ie. Select the spell runes to combine:");
						
						Term_putstr(4, 13, -1, TERM_GREEN, "(Select the \377Usame \377grune twice or press \377sENTER \377gto cast a single rune spell!)");
						
						i = 0;
						for (j = 0; j < 2; j++) {
							if (i == -1 || i == -2) continue; //invalid action -OR- drawing less than the maximum number of runes

							clear_from(14);
//							Term_putstr(10, 14, -1, TERM_GREEN, format("Selected runes: \377s%s", buf2)); //Redundant unless RCRAFT_MAX_ELEMENTS > 2
							for (i = 0; i < RCRAFT_MAX_ELEMENTS; i++) {
								/* Show duplicate rune(s) in umber (or press ENTER; allow both!) */
								if (e_flags & r_elements[i].flag) {
									Term_putstr(10, 15 + i % 6, -1, TERM_L_UMBER, format("%c) \377%c%s", 'a' + i, 'U', r_elements[i].name));
								}
								else {
									Term_putstr(10, 15 + i % 6, -1, TERM_L_GREEN, format("%c) \377%c%s", 'a' + i, 'G', r_elements[i].name));
								}
							}

							switch (choice = inkey()) {
							case '\r':
								if (j > 0) i = -2; //Are we on the second iteration? (Don't exit with 0 runes!)
								else j--;
								break;
							case ESCAPE:
							case '\010': /* backspace */
								i = -1; /* leave */
								continue;
							case KTRL('T'):
								/* Take a screenshot */
								xhtml_screenshot("screenshot????");
								j--;
								continue;
							default: /* invalid action -> exit wizard */
								if (choice < 'a' || choice > 'a' + RCRAFT_MAX_ELEMENTS - 1) {
									j--;
//									i = -1; //Just ignore it. (ESC to exit)
									continue;
								}

								/* add this rune.. */
								e_flags |= r_elements[choice - 'a'].flag;
								/* Enable below for old 'rune list' display - Redundant at the moment, but cool~ +_+ */
//								//strcat(buf2, r_elements[choice - 'a'].name);
//								//strcat(buf2, " ");
							}

							/* build macro part: 'a'..'p' or '\r' */
							strcat(buf, format("%c", choice));
						}
						if (i == -1) continue; //invalid action -> exit

						/* ---------- Select imperative ---------- */

						clear_from(10);
						Term_putstr(10, 10, -1, TERM_GREEN, "Please choose the imperative of the spell.");
						Term_putstr(10, 11, -1, TERM_GREEN, "ie. Select the spell modifier:");

						/* Print the header */
						Term_putstr(10, 13, -1, TERM_L_GREEN, "   Imperative Level Cost Fail Damage Radius Duration Energy");

						/* Analyze parameters */
						byte element[RCRAFT_MAX_ELEMENTS];
						byte elements = flags_to_elements(element, e_flags);
						byte skill = rspell_skill(element, elements);

						/* Fill the list */
						int color;
						char tmpbuf[80];
						for (i = 0; i < RCRAFT_MAX_IMPERATIVES; i++) {

							/* Get the line color */
							if (r_imperatives[i].level < skill) color = 'G';
							else color = 'D';

							/* Fill a line */
							sprintf(tmpbuf, "\377%c%c) %-10s %5d %3d%% %s%d%% %5d%% %5s%d %7d%% %5d%%",
							    color, 'a' + i, r_imperatives[i].name, r_imperatives[i].level, r_imperatives[i].cost * 10,
							    ABS(r_imperatives[i].fail) >= 10 ? (r_imperatives[i].fail >= 0 ? "+" : "") : (r_imperatives[i].fail >= 0 ? " +" : " "),
							    r_imperatives[i].fail, r_imperatives[i].damage * 10, r_imperatives[i].radius >= 0 ? "+" : "-", ABS(r_imperatives[i].radius),
							    r_imperatives[i].duration * 10, r_imperatives[i].energy * 10);

							/* Print the line */
							Term_putstr(10, 14 + i, -1, TERM_L_GREEN, tmpbuf);
						}

						while (TRUE) {
							switch (choice = inkey()) {
							case ESCAPE:
							case 'p':
							case '\010': /* backspace */
								i = -1; /* leave */
								break;
							case KTRL('T'):
								/* Take a screenshot */
								xhtml_screenshot("screenshot????");
								continue;
							default:
								/* invalid action -> exit wizard */
								if (choice < 'a' || choice > ('a'+RCRAFT_MAX_TYPES-1)) {
//									i = -1; //Just ignore it. (ESC to exit)
									continue;
								}
							}
							break;
						}
						/* exit? */
						if (i == -1) continue;

						/* Store the imperative */
						m_flags |= r_imperatives[choice - 'a'].flag;

						/* build macro part: 'a'..'h' */
						strcat(buf, format("%c", choice));

						/* ---------- Select form ---------- */

						clear_from(10);
						Term_putstr(10, 10, -1, TERM_GREEN, "Please choose the form of the spell.");
						Term_putstr(10, 11, -1, TERM_GREEN, "ie. Select the spell type:");

						/* Print the header */
						Term_putstr(10, 13, -1, TERM_L_GREEN, "   Form    Level Cost Fail Info");

						/* Check for a penalty due to status */
						u16b penalty = 0;
						if (p_ptr->blind) penalty += 10;
						//if (p_ptr->confused) penalty += 10; //#ifdef ENABLE_CONFUSED_CASTING
						if (p_ptr->stun > 50) penalty += 25;
						else if (p_ptr->stun) penalty += 15;

						/* Analyze more parameters */
						byte projection = flags_to_projection(e_flags);
						byte imperative = flags_to_imperative(m_flags);
						byte level, cost, fail;
						s16b diff, sdiff;

						/* Print the list */
						//int color;
						//char tmpbuf[80];
						for (i = 0; i < RCRAFT_MAX_TYPES; i++) {

							/* Analyze list parameters */
							level = rspell_level(imperative, i);
							diff = rspell_diff(skill, level);
							cost = rspell_cost(imperative, i, skill);
							fail = rspell_fail(imperative, i, diff, penalty);
							u32b dx, dy;
							u16b damage = rspell_damage(&dx, &dy, imperative, i, skill, projection);
							u16b dice = damroll(dx, dy);
							byte radius = rspell_radius(imperative, i, skill, projection);
							byte duration = rspell_duration(imperative, i, skill, projection, dice);

							/* Extra parameters */
							sdiff = skill - level + 1; //For a real 'level difference' display.

							/* Get the line color */
							if (diff > 0) {
								color = 'G';
								if (penalty) color = 'y';
								if (p_ptr->msp < cost) color = 'o';
								if (p_ptr->anti_magic && r_types[i].flag != T_ENCH) color = 'r'; //#define ENABLE_SHELL_ENCHANT
							}
							else color = 'D';

							/* Fill a line */
							switch (r_types[i].flag) {
							case T_BOLT:
								sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, dx, dy);
								break;
							case T_BEAM:
								if (r_imperatives[imperative].flag == I_ENHA) {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d dur %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, duration);
								} else {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, dx, dy);
								}
								break;
							case T_CLOU:
								if (r_imperatives[imperative].flag != I_ENHA) {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d rad %d dur %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, radius, duration);
								} else { //Storm Hack
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d rad %d dur %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage*3/2, radius, duration*2);
								}
								break;
							case T_BALL:
								sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, radius);
								break;
							case T_SIGN:
								switch (projection) {
								case SV_R_LITE:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% call light dam %d rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, radius);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% starlite dam %dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, dx, dy);
									}
									break;
								case SV_R_DARK:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% call dark dam %d rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, radius);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% invis dur %dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, dx, dy);
									}
									break;
								case SV_R_NEXU:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% tele %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, radius);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% teleport", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_NETH:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% obliteration", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% genocide", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_CHAO:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% polymorph self", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% chaos brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								case SV_R_MANA:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% remove curses", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% remove *curses*", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_CONF:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% monster confusion", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% identify", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_INER:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% anchor dur %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% tele-forward", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_ELEC:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% res electricity dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% lightning brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								case SV_R_FIRE:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% res fire dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% fire brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								case SV_R_WATE:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% neutralize poison", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% regen pow %d dur %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, duration);
									}
									break;
								case SV_R_GRAV:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% tele-to rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, radius);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% tele-forward", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_COLD:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% res cold dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% frost brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								case SV_R_ACID:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% res acid dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% acid brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								case SV_R_POIS:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% res poison dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% poison brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								case SV_R_TIME:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% +%d speed dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% recharging pow %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage);
									}
									break;
								case SV_R_SOUN:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% disarm rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, radius);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% disarming", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_SHAR:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% earthquake rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, radius);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dig", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_DISE:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% unmagic", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% cancellation", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									}
									break;
								case SV_R_FORC:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% +%d AC dur %dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% reflect dur %dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, dx, dy);
									}
									break;
								case SV_R_PLAS:
									if (r_imperatives[imperative].flag != I_ENHA) {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% resistance dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									} else {
										sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% power brand dur %d+%dd%d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, duration, dx, dy);
									}
									break;
								default:
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%%", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
									break;
								}
								break;
							case T_RUNE:
								if (r_imperatives[imperative].flag != I_ENHA) {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d rad %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, radius);
								} else {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% warding", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
								}
								break;
							case T_ENCH:
								if (r_imperatives[imperative].flag != I_ENHA) {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% rune resist", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
								} else {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% rune misc", color, 'a' + i, r_types[i].name, sdiff, cost, fail);
								}
								break;
							case T_WAVE:
								if (r_imperatives[imperative].flag != I_ENHA) {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d dur %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage, duration);
								} else {
									sprintf(tmpbuf, "\377%c%c) %-7s %5d %4d %3d%% dam %d", color, 'a' + i, r_types[i].name, sdiff, cost, fail, damage);
								}
								break;
							}
							Term_putstr(10, 14 + i, -1, TERM_L_GREEN, tmpbuf);
						}

						while (TRUE) {
							switch (choice = inkey()) {
							case ESCAPE:
							case 'p':
							case '\010': /* backspace */
								i = -1; /* leave */
								break;
							case KTRL('T'):
								/* Take a screenshot */
								xhtml_screenshot("screenshot????");
								continue;
							default:
								/* invalid action -> exit wizard */
								if (choice < 'a' || choice > ('a'+RCRAFT_MAX_IMPERATIVES-1)) {
//									i = -1; //Just ignore it. (ESC to exit)
									continue;
								}
							}
							break;
						}
						/* exit? */
						if (i == -1) continue;

						/* Store the type */
						m_flags |= r_types[choice - 'a'].flag;

						/* build macro part: 'a'..'h' */
						strcat(buf, format("%c", choice));

						/* ---------- Determine if a direction is needed (hard-coded) ---------- */

						/* Request the Direction -- Hardcoded - Kurzel */
						if (((m_flags & T_BOLT) == T_BOLT)
						 || ((m_flags & T_BEAM) == T_BEAM)
						 || (((m_flags & T_CLOU) == T_CLOU) && !((m_flags & I_ENHA) == I_ENHA))
						 || ((m_flags & T_BALL) == T_BALL)
						 || (((m_flags & T_SIGN) == T_SIGN) && (
						 ((projection == SV_R_INER) && ((m_flags & I_ENHA) == I_ENHA)) ||
						 ((projection == SV_R_GRAV) && ((m_flags & I_ENHA) == I_ENHA)) ||
						 ((projection == SV_R_SOUN) && ((m_flags & I_ENHA) == I_ENHA)) ||
						 ((projection == SV_R_SHAR) && ((m_flags & I_ENHA) == I_ENHA))
						 ))) 
							strcat(buf, "*t");

						/* hack before we exit: remember menu choice 'runespell' */
						choice = mw_rune;
						i = 1;
						break;
					case mw_trap:
						/* ---------- Enter trap kit name ---------- */
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter a distinctive part of the trap kit name or inscription.");
						//Term_putstr(5, 11, -1, TERM_GREEN, "and pay attention to upper-case and lower-case letters!");
						Term_putstr(5, 11, -1, TERM_GREEN, "For example, enter:     \377GCatapult");
						Term_putstr(5, 12, -1, TERM_GREEN, "if you want to use a 'Catapult Trap Kit'.");
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Enter partial trap kit name or inscription:");

						/* Get an item name */
						Term_gotoxy(50, 16);
						strcpy(buf, "");
						if (!askfor_aux(buf, 159, 0)) {
							i = -1;
							continue;
						}
						strcat(buf, "\\r");

						/* ---------- Enter ammo/load name ---------- */
						clear_from(10);
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter a distinctive part of the item name or inscription you");
						Term_putstr(5, 11, -1, TERM_GREEN, "want to load the trap kit with.");//, and pay attention to upper-case");
						//Term_putstr(5, 12, -1, TERM_GREEN, "and lower-case letters!");
						Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GPebbl     \377gif you want");
						Term_putstr(5, 13, -1, TERM_GREEN, "to load a catapult trap kit with 'Rounded Pebbles'.");
						Term_putstr(5, 14, -1, TERM_GREEN, "If you want to choose ammo manually, just press the \377GRETURN\377g key.");
						Term_putstr(5, 17, -1, TERM_L_GREEN, "Enter partial ammo/load name or inscription:");

						/* Get an item name */
						Term_gotoxy(50, 17);
						strcpy(buf2, "");
						if (!askfor_aux(buf2, 159, 0)) {
							i = -1;
							continue;
						}
						/* Choose ammo manually? Terminate partial macro here. */
						if (streq(buf2, "")) break;

						strcat(buf2, "\\r");
						strcat(buf, "@");
						strcat(buf, buf2);
						break;
					case mw_device:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please choose the type of magic device you want to use:");
						Term_putstr(15, 12, -1, TERM_L_GREEN, "a) a wand");
						Term_putstr(15, 13, -1, TERM_L_GREEN, "b) a staff");
						Term_putstr(15, 14, -1, TERM_L_GREEN, "c) a rod that doesn't require a target");
						Term_putstr(15, 15, -1, TERM_L_GREEN, "d) a rod that requires a target");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "e) an activatable item that doesn't require a target");
						Term_putstr(15, 17, -1, TERM_L_GREEN, "f) an activatable item that requires a target");

						while (TRUE) {
							switch (choice = inkey()) {
							case ESCAPE:
							case 'p':
							case '\010': /* backspace */
								i = -1; /* leave */
								break;
							case KTRL('T'):
								/* Take a screenshot */
								xhtml_screenshot("screenshot????");
								continue;
							default:
								/* invalid action -> exit wizard */
								if (choice < 'a' || choice > 'f') {
//									i = -1;
									continue;
								}
							}
							break;
						}
						/* exit? */
						if (i == -1) continue;

						/* build macro part */
						j = 0; /* hack: != 1 means 'undirectional' device */
						if (c_cfg.rogue_like_commands) {
							switch (choice) {
							case 'a': strcpy(buf2, "\\e)z@"); j = 1; break;
							case 'b': strcpy(buf2, "\\e)Z@"); break;
							case 'c': strcpy(buf2, "\\e)a@"); break;
							case 'd': strcpy(buf2, "\\e)a@"); j = 1; break;
							case 'e': strcpy(buf2, "\\e)A@"); break;
							case 'f': strcpy(buf2, "\\e)A@"); j = 1; break;
							}
						} else {
							switch (choice) {
							case 'a': strcpy(buf2, "\\e)a@"); j = 1; break;
							case 'b': strcpy(buf2, "\\e)u@"); break;
							case 'c': strcpy(buf2, "\\e)z@"); break;
							case 'd': strcpy(buf2, "\\e)z@"); j = 1; break;
							case 'e': strcpy(buf2, "\\e)A@"); break;
							case 'f': strcpy(buf2, "\\e)A@"); j = 1; break;
							}
						}

						/* ---------- Enter device name ---------- */

						clear_from(10);
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter a distinctive part of the magic device's name or");
						Term_putstr(5, 11, -1, TERM_GREEN, "inscription.");// and pay attention to upper-case and lower-case letters!");
						switch (choice) {
						case 'a': Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GMagic Mis");
							Term_putstr(5, 13, -1, TERM_GREEN, "if you want to use a 'Wand of Magic Missiles'.");
							break;
						case 'b': Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GTelep");
							Term_putstr(5, 13, -1, TERM_GREEN, "if you want to use a 'Staff of Teleportation'.");
							break;
						case 'c': Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GTrap Loc");
							Term_putstr(5, 13, -1, TERM_GREEN, "if you want to use a 'Rod of Trap Location'.");
							break;
						case 'd': Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GLightn");
							Term_putstr(5, 13, -1, TERM_GREEN, "if you want to use a 'Rod of Lightning Bolts'.");
							break;
						case 'e': Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GFrostweav");
							Term_putstr(5, 13, -1, TERM_GREEN, "if you want to use a 'Cloak of Frostweaving'.");
							break;
						case 'f': Term_putstr(5, 12, -1, TERM_GREEN, "For example, enter:     \377GSerpen");
							Term_putstr(5, 13, -1, TERM_GREEN, "if you want to use an 'Amulet of the Serpents'.");
							break;
						}
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Enter partial device name or inscription:");

						/* hack before we exit: remember menu choice 'magic device' */
						choice = mw_device;

						break;

					case mw_abilitynt:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact ability name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GSwitch between main-hand and dual-hand");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact ability name:");
						break;
					case mw_abilityt:
						Term_putstr(10, 10, -1, TERM_GREEN, "Please enter the exact ability name.");// and pay attention");
						//Term_putstr(10, 11, -1, TERM_GREEN, "to upper-case and lower-case letters and spaces!");
						Term_putstr(10, 11, -1, TERM_GREEN, "For example, enter:     \377GBreathe element");
						Term_putstr(15, 16, -1, TERM_L_GREEN, "Enter exact ability name:");
						break;

					case mw_throw:
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter a distinctive part of the item's name or inscription.");
						//Term_putstr(5, 11, -1, TERM_GREEN, "and pay attention to upper-case and lower-case letters!");
						Term_putstr(5, 11, -1, TERM_GREEN, "For example, enter:     \377G{bad}");
						Term_putstr(5, 12, -1, TERM_GREEN, "if you want to throw any item that is inscribed '{bad}'.");
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Enter partial potion name or inscription:");
						break;

					case mw_custom:
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter the custom macro action string.");
						Term_putstr(5, 11, -1, TERM_GREEN, "(You have to specify everything manually here, and won't get");
						Term_putstr(5, 12, -1, TERM_GREEN, "prompted about a targetting method or anything else either.)");
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Enter a new action:");
						break;

					case mw_load:
						Term_putstr(5, 10, -1, TERM_GREEN, "Please enter the macro file name.");
						Term_putstr(5, 11, -1, TERM_GREEN, "If you are on Linux or OSX it is case-sensitive! On Windows it is not.");
						Term_putstr(5, 12, -1, TERM_GREEN, format("For example, enter:     \377G%s.prf", nick));
						Term_putstr(5, 16, -1, TERM_L_GREEN, "Exact file name:");
						break;
					}

					/* --------------- specify item/parm if required --------------- */

					/* might input a really long line? */
					if (choice == mw_custom) {
						Term_gotoxy(5, 17);

						/* Get an item/spell name */
						strcpy(buf, "");
						if (!askfor_aux(buf, 159, 0)) {
							i = -1;
							continue;
						}
					}
					/* mw_mimicidx is special: it requires a number (1..n) */
					else if (choice == mw_mimicidx) {
						while (TRUE) {
							/* Get power slot */
							Term_gotoxy(47, 16);
							strcpy(buf, "");
							if (!askfor_aux(buf, 159, 0)) {
								i = -1;
								break;
							}
							/* not a number/invalid? retry (we have slots d)..z)) */
							if (atoi(buf) < 1 || atoi(buf) > 26 - 3) continue;

							/* ok (1..23) - translate into spell slot */
							strcpy(buf, format("%c", 'd' + atoi(buf)));
							break;
						}
						if (i == -1) continue;
					}
					/* no need for inputting an item/spell to use with the macro? */
					else if (choice != mw_fire && choice != mw_rune && choice != mw_trap && choice != mw_prfimm) {
						if (choice == mw_load) Term_gotoxy(23, 16);
						else if (choice == mw_poly) Term_gotoxy(47, 17);
						else Term_gotoxy(47, 16);

						/* Get an item/spell name */
						strcpy(buf, "");
						if (!askfor_aux(buf, 159, 0)) {
							i = -1;
							continue;
						}

						/* allow 'empty' polymorph-into macro that prompts for form */
						if (choice != mw_poly || buf[0]) strcat(buf, "\\r");
					}

					/* --------------- complete the macro by glueing premade part and default part together --------------- */

					/* generate the full macro action; magic device/preferred immunity macros are already pre-made */
					if (choice != mw_device && choice != mw_prfimm && choice != mw_custom) {
						buf2[0] = '\\'; //note: should in theory be ')e\',
						buf2[1] = 'e'; //      but doesn't work due to prompt behaviour
						buf2[2] = ')'; //      (\e will then get ignored)
					}

					switch (choice) {
					case mw_quaff:
						buf2[3] = 'q';
						buf2[4] = '@';
						strcpy(buf2 + 5, buf);
						break;
					case mw_read:
						buf2[3] = 'r';
						buf2[4] = '@';
						strcpy(buf2 + 5, buf);
						break;
					case mw_fire:
						if (c_cfg.rogue_like_commands) buf2[3] = 't';
						else buf2[3] = 'f';
						buf2[4] = '*';
						buf2[5] = 't';
						buf2[6] = 0;
						break;
					case mw_schoolnt:
					case mw_schoolt:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '1';
						buf2[6] = '1';
						buf2[7] = '\\';
						buf2[8] = 'r';
						buf2[9] = '@';
						strcpy(buf2 + 10, buf);
						if (choice == mw_schoolt) {
							strcpy(buf, "*t");
							strcat(buf2, buf);
						}
						break;
					case mw_mimicnt:
					case mw_mimict:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '3';
						buf2[6] = '\\';
						buf2[7] = 'r';
						buf2[8] = '@';
						strcpy(buf2 + 9, buf);
						if (choice == mw_mimict) {
							strcpy(buf, "*t");
							strcat(buf2, buf);
						}
						break;
					case mw_mimicidx:
						buf2[3] = '*';
						buf2[4] = 't';
						buf2[5] = 'm';
						buf2[6] = '@';
						buf2[7] = '3';
						buf2[8] = '\\';
						buf2[9] = 'r';
						strcpy(buf2 + 10, buf);
						/* note: targetting method '-' is the only option here,
						   for safety reasons (if spell doesn't take a target!) */
						strcat(buf2, "-");
						break;
					case mw_fight:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '5';
						buf2[6] = '\\';
						buf2[7] = 'r';
						buf2[8] = '@';
						strcpy(buf2 + 9, buf);
						break;
					case mw_shoot:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '6';
						buf2[6] = '\\';
						buf2[7] = 'r';
						buf2[8] = '@';
						strcpy(buf2 + 9, buf);
						break;
					case mw_poly:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '3';
						buf2[6] = '\\';
						buf2[7] = 'r';
						buf2[8] = 'c';
						strcpy(buf2 + 9, buf);
						break;
					case mw_rune:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '1';
						buf2[6] = '2';
						buf2[7] = '\\';
						buf2[8] = 'r';
						strcpy(buf2 + 9, buf);
						break;
					case mw_trap:
						buf2[3] = 'm';
						buf2[4] = '@';
						buf2[5] = '1';
						buf2[6] = '0';
						buf2[7] = '\\';
						buf2[8] = 'r';
						buf2[9] = '@';
						strcpy(buf2 + 10, buf);
						break;
					case mw_device:
						/* hack: magiv device uses direction? */
						if (j == 1) strcat(buf, "*t");

						strcat(buf2, buf);
						break;
					case mw_abilitynt:
					case mw_abilityt:
						buf2[3] = 'm';
						buf2[4] = '@';
						strcpy(buf2 + 5, buf);
						if (choice == mw_abilityt) {
							strcpy(buf, "*t");
							strcat(buf2, buf);
						}
						break;
					case mw_throw:
						buf2[3] = 'v';
						buf2[4] = '@';
						strcpy(buf2 + 5, buf);
						l = strlen(buf2);
						buf2[l] = '*';
						buf2[l + 1] = 't';
						buf2[l + 2] = 0;
						break;

					case mw_custom:
						strcpy(buf2, buf);
						break;

					case mw_load:
						buf2[3] = '%';
						buf2[4] = 'l';
						strcpy(buf2 + 5, buf);
						l = strlen(buf2);
						buf2[l] = '\\';
						buf2[l + 1] = 'e';
						buf2[l + 2] = 0;
						break;
					}

					/* Convert the targetting method from XXX*t to *tXXX- ? */
#ifdef MACRO_WIZARD_SMART_TARGET
					/* ask about replacing '*t' vs '-' (4.4.6) vs '+' (4.4.6b) */
					if (strstr(buf2, "*t") && choice != mw_mimicidx
					    && choice != mw_load /* (paranoia) */
					    && choice != mw_custom
					    ) {
						clear_from(8);
						Term_putstr(10, 8, -1, TERM_GREEN, "Please choose the targetting method:");

						//Term_putstr(10, 11, -1, TERM_GREEN, "(\377UHINT: \377gAlso inscribe your ammo '!=' for auto-pickup!)");
						Term_putstr(10, 10, -1, TERM_L_GREEN, "a) Target closest monster if such exists,");
						Term_putstr(10, 11, -1, TERM_L_GREEN, "   otherwise cancel action. (\377URecommended in most cases!\377G)");
						Term_putstr(10, 12, -1, TERM_L_GREEN, "b) Target closest monster if such exists,");
						Term_putstr(10, 13, -1, TERM_L_GREEN, "   otherwise prompt for direction.");
						Term_putstr(10, 14, -1, TERM_L_GREEN, "c) Target closest monster if such exists,");
						Term_putstr(10, 15, -1, TERM_L_GREEN, "   otherwise target own grid.");
						Term_putstr(10, 16, -1, TERM_L_GREEN, "d) Fire into a fixed direction.");
						Term_putstr(10, 17, -1, TERM_L_GREEN, "e) Target own grid (ie yourself).");

						Term_putstr(10, 19, -1, TERM_L_GREEN, "f) Target most wounded friendly player,");
						Term_putstr(10, 20, -1, TERM_L_GREEN, "   cancel action if no player is nearby. (\377UEg for 'Cure Wounds'.\377G)");
						Term_putstr(10, 21, -1, TERM_L_GREEN, "g) Target most wounded friendly player,");
						Term_putstr(10, 22, -1, TERM_L_GREEN, "   target own grid instead if no player is nearby.");

						while (TRUE) {
							switch (choice = inkey()) {
							case ESCAPE:
							case 'p':
							case '\010': /* backspace */
								i = -1; /* leave */
								break;
							case KTRL('T'):
								/* Take a screenshot */
								xhtml_screenshot("screenshot????");
								continue;
							default:
								/* invalid action -> exit wizard */
								if (choice < 'a' || choice > 'g') {
//									i = -1;
									continue;
								}
							}
							break;
						}
						/* exit? */
						if (i == -1) continue;

						/* Get a specific fixed direction */
						if (choice == 'd') {
							clear_from(8);
							Term_putstr(10, 10, -1, TERM_GREEN, "Please pick the specific, fixed direction:");

							Term_putstr(30, 13, -1, TERM_L_GREEN, " 7  8  9");
							Term_putstr(30, 14, -1, TERM_GREEN, "  \\ | / ");
							Term_putstr(30, 15, -1, TERM_L_GREEN, "4 \377g-\377G 5 \377g-\377G 6");
							Term_putstr(30, 16, -1, TERM_GREEN, "  / | \\ ");
							Term_putstr(30, 17, -1, TERM_L_GREEN, " 1  2  3");

							Term_putstr(15, 20, -1, TERM_L_GREEN, "Your choice? (1 to 9) ");

							while (TRUE) {
								switch (target_dir = inkey()) {
								case ESCAPE:
								case 'p':
								case '\010': /* backspace */
									i = -1; /* leave */
									break;
								case KTRL('T'):
									/* Take a screenshot */
									xhtml_screenshot("screenshot????");
									continue;
								default:
									/* invalid action -> exit wizard */
									if (target_dir < '1' || target_dir > '9') {
//										i = -1;
										continue;
									}
								}
								break;
							}
							/* exit? */
							if (i == -1) continue;
						}

						if (choice != 'c') {
							/* choose initial targetting mechanics */
							if (choice == 'd') strcpy(buf, "\\e)");
							else if (choice == 'e') strcpy(buf, "\\e)*q");
							else if (choice == 'f') strcpy(buf, "\\e)(");
							else strcpy(buf, "\\e)*t");

							/* We assume that '*t' is always the last part in the macro
							   and that '\e)' is always the first part */
							strncat(buf, buf2 + 3, strlen(buf2) - 5);

							/* add new direction feature */
							if (choice == 'b') strcat(buf, "+");
							else if (choice == 'd') strcat(buf, format("%c", target_dir));
							else if (choice == 'e' || choice == 'g') strcat(buf, "5");
							else strcat(buf, "-");

							/* replace old macro by this one */
							strcpy(buf2, buf);
						}
					}
#endif

					/* Extract an action */
					/* Omit repeated '\e)' -- can break chain macros (todo: fix?) */
					if (chain_macro_buf[0] && choice != mw_custom) text_to_ascii(macro__buf, buf2 + 3);
					else text_to_ascii(macro__buf, buf2);
					/* Handle chained macros */
					strcat(chain_macro_buf, macro__buf);
					strcpy(macro__buf, chain_macro_buf);

					/* advance to next step */
					i++;
					break;
				case 2:
					Term_putstr(10, 9, -1, TERM_GREEN, "In this final step, press the key you would like");
					Term_putstr(10, 10, -1, TERM_GREEN, "to bind the macro to. (ESC and % key cannot be used.)");
					Term_putstr(10, 11, -1, TERM_GREEN, "You should use keys that have no other purpose!");
					Term_putstr(10, 12, -1, TERM_GREEN, "Good examples are the F-keys, F1 to F12 and unused number pad keys.");
					//Term_putstr(10, 14, -1, TERM_GREEN, "The keys ESC and '%' are NOT allowed to be used.");
					Term_putstr(10, 14, -1, TERM_GREEN, "Most keys can be combined with \377USHIFT\377g, \377UALT\377g or \377UCTRL\377g modifiers!");
					Term_putstr(10, 16, -1, TERM_GREEN, "If you want to \377Uchain another macro\377g, press '\377U%\377g' key.");
					Term_putstr(10, 17, -1, TERM_GREEN, "By doing this you can combine multiple macros into one hotkey.");
					Term_putstr(5, 19, -1, TERM_L_GREEN, "Press the key to bind the macro to, or '%' for chaining: ");

					while (TRUE) {
						/* Get a macro trigger */
						Term_putstr(67, 19, -1, TERM_WHITE, "  ");//45
						Term_gotoxy(67, 19);
						get_macro_trigger(buf);

						/* choose proper macro type, and bind it to key */
						if (!strcmp(buf, format("%c", KTRL('T')))) {
							/* Take a screenshot */
							xhtml_screenshot("screenshot????");
							continue;
						} else if (!strcmp(buf, "\e")) {//chaining: || !strcmp(buf, "%")) {
							c_msg_print("\377yKeys <ESC> and '%' aren't allowed to carry a macro.");
							if (!strcmp(buf, "\e")) {
								i = -1; /* leave */
								break;
							}
							continue;
						} else if (!strcmp(buf, "%")) {
							/* max length check, rough estimate */
							if (strlen(chain_macro_buf) + strlen(macro__buf) >= 1024 - 50) {
								c_msg_print("\377oYou cannot chain any further commands to this macro.");
								continue;
							}

							/* chain another macro */
							goto Chain_Macro;
						}
						break;
					}
					/* exit? */
					if (i == -1) continue;

					/* Automatically choose usually best fitting macro type,
					   depending on chosen trigger key! */
					//[normal macros: F-keys (only keys that aren't used for any text input)]
					//command macros: / * a..w (all keys that are used in important standard prompts)
					//hybrid macros: all others, maybe even also normal-macro-keys
					if (!strcmp(buf, "/") || !strcmp(buf, "*") || /* windows */
					    (strlen(buf) == 10 && (buf[8] == '/' || buf[8] == '*')) || /* x11 - to do: fix this crap */
					    (*buf >= 'a' && *buf <= 'w')) {
						/* make it a command macro */
						/* Link the macro */
						macro_add(buf, macro__buf, TRUE, FALSE);
						/* Message */
						c_msg_print("Created a new command macro.");
					} else {
						/* make it a hybrid macro */
						/* Link the macro */
						macro_add(buf, macro__buf, FALSE, TRUE);
						/* Message */
						c_msg_print("Created a new hybrid macro.");
					}

					/* this was the final step, we're done */
					i = -1;
					break;
				}
			}
		}

		/* Oops */
		else {
			/* Oops */
			bell();
		}
	}

	/* Reload screen */
	Term_load();

	/* Flush the queue */
	Flush_queue();

	inkey_interact_macros = FALSE;

	/* in case we entered this menu from recording a macro,
	   we might have to update the '*recording*' status line */
	if (were_recording) Send_redraw(0);
}


void auto_inscriptions(void) {
	int i, cur_line = 0;
	bool redraw = TRUE, quit = FALSE;

	char tmp[160], buf[1024], *buf_ptr;
	char match_buf[80], tag_buf[40];

	char fff[1024];

	/* Save screen */
	Term_save();

	/* Prevent hybrid macros from triggering in here */
	inkey_msg = TRUE;

	/* Process requests until done */
	while (1) {
		if (redraw) {
			/* Clear screen */
			Term_clear();

			/* Describe */
			Term_putstr(20,  0, -1, TERM_L_UMBER, "*** Current Auto-Inscriptions List ***");
			Term_putstr(14, 21, -1, TERM_L_UMBER, "[Press 'n' for next, 'p' for previous, ESC to exit]");
			Term_putstr(12, 22, -1, TERM_L_UMBER, "(l/s) Load/save auto-inscriptions from/to an '.ins' file");
			Term_putstr(4, 23, -1, TERM_L_UMBER, "(e/d/c) Edit current ('?' wildcard, '!' forces)/delete current/CLEAR ALL");

			for (i = 0; i < MAX_AUTO_INSCRIPTIONS; i++) {
				/* build a whole line */
				strcpy(match_buf, "\377s<\377w");
				strcat(match_buf, auto_inscription_match[i]);
				strcat(match_buf, "\377s>");
				strcpy(tag_buf, "\377y");
				strcat(tag_buf, auto_inscription_tag[i]);
				sprintf(fff, "%2d) %-46s      %s", i + 1, match_buf, tag_buf);

				Term_putstr(5, i + 1, -1, TERM_WHITE, fff);
			}
		}
		redraw = TRUE;

		/* display editing 'cursor' */
		Term_putstr(1, cur_line + 1, -1, TERM_ORANGE, ">>>");

		/* make cursor invisible */
		Term_set_cursor(0);
		inkey_flag = TRUE;

		/* Wait for keypress */
		switch (inkey()) {
		case KTRL('T'):
			/* Take a screenshot */
			xhtml_screenshot("screenshot????");
			redraw = FALSE;
			break;
		case ESCAPE:
			quit = TRUE; /* hack to leave loop */
			break;
		case 'n':
		case '2':
			Term_putstr(1, cur_line + 1, -1, TERM_ORANGE, "   ");
			cur_line++;
			if (cur_line >= 20) cur_line = 0;
			redraw = FALSE;
			break;
		case 'p':
		case '8':
			Term_putstr(1, cur_line + 1, -1, TERM_ORANGE, "   ");
			cur_line--;
			if (cur_line < 0) cur_line = 19;
			redraw = FALSE;
			break;
		case 'l':
			/* Prompt */
			clear_from(21);
			Term_putstr(0, 22, -1, TERM_L_GREEN, "*** Load an .ins file ***");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 23, -1, TERM_WHITE, "File: ");

			sprintf(tmp, "%s.ins", cname);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Process the given filename */
			load_auto_inscriptions(tmp);
			for (i = 0; i <= INVEN_TOTAL; i++) apply_auto_inscriptions(i, FALSE);
			break;
		case 's':
			/* Prompt */
			clear_from(21);
			Term_putstr(0, 22, -1, TERM_L_GREEN, "*** Save an .ins file ***");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 23, -1, TERM_WHITE, "File: ");

			sprintf(tmp, "%s.ins", cname);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Dump the macros */
			save_auto_inscriptions(tmp);
			break;
		case 'e':
		case '6':
		case '\n':
		case '\r':
			/* Clear previous matching string */
			Term_putstr(9, cur_line + 1, -1, TERM_L_GREEN, "                                         ");
			/* Go to the correct location */
			Term_gotoxy(10, cur_line + 1);
			strcpy(buf, auto_inscription_match[cur_line]);
			/* Get a new matching string */
			if (!askfor_aux(buf, 40, 0)) continue;
			/* hack: remove leading/trailing wild cards since they are obsolete.
			   Especially trailing ones currently make it not work. */
			buf_ptr = buf;
			while (*buf_ptr == '?') buf_ptr++;
			while (*(buf_ptr + strlen(buf_ptr) - 1) == '?') *(buf_ptr + strlen(buf_ptr) - 1) = '\0';
			Term_putstr(9, cur_line + 1, -1, TERM_L_GREEN, "                                         ");
			Term_putstr(9, cur_line + 1, -1, TERM_WHITE, buf_ptr);
			/* ok */
			strcpy(auto_inscription_match[cur_line], buf_ptr);

			/* Clear previous tag string */
			Term_putstr(55, cur_line + 1, -1, TERM_L_GREEN, "                    ");
			/* Go to the correct location */
			Term_gotoxy(55, cur_line + 1);
			strcpy(buf, auto_inscription_tag[cur_line]);
			/* Get a new tag string */
			if (!askfor_aux(buf, 20, 0)) {
				/* in case match was changed, we may also need to reapply */
				for (i = 0; i <= INVEN_TOTAL; i++) apply_auto_inscriptions(i, FALSE);
				continue;
			}
			strcpy(auto_inscription_tag[cur_line], buf);
			for (i = 0; i <= INVEN_TOTAL; i++) apply_auto_inscriptions(i, FALSE);

			/* comfort hack - fake advancing ;) */
			Term_putstr(1, cur_line + 1, -1, TERM_ORANGE, "   ");
			cur_line++;
			if (cur_line >= 20) cur_line = 0;
			break;
		case 'd':
		case '\b':
			strcpy(auto_inscription_match[cur_line], "");
			strcpy(auto_inscription_tag[cur_line], "");
#if 0
			/* also auto-advance by 1 line, for convenience */
			cur_line++;
			if (cur_line >= 20) cur_line = 0;
#endif
			break;
		case 'c':
			for (i = 0; i < MAX_AUTO_INSCRIPTIONS; i++) {
				strcpy(auto_inscription_match[i], "");
				strcpy(auto_inscription_tag[i], "");
			}
			/* comfort hack - jump to first line */
			Term_putstr(1, cur_line + 1, -1, TERM_ORANGE, "   ");
			cur_line = 0;
			break;
		default:
			/* Oops */
			bell();
		}

		/* Leave */
		if (quit) break;

	}

	/* Reload screen */
	Term_load();

	/* Flush the queue */
	Flush_queue();

	/* Re-enable hybrid macros */
	inkey_msg = FALSE;
}


/*
 * Interact with some options
 */
static void do_cmd_options_aux(int page, cptr info) {
	char	ch;
	int	i, k = 0, n = 0;
	int	opt[24];
	char	buf[256];


	/* Lookup the options */
	for (i = 0; i < 24; i++) opt[i] = 0;

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++) {
		/* Notice options on this "page" */
		if (option_info[i].o_page == page &&
		    option_info[i].o_enabled) opt[n++] = i;
	}


	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE) {
		/* Prompt XXX XXX XXX */
		sprintf(buf, "%s (\377yUp/Down\377w moves, \377yy\377w/\377yn\377w sets, \377yt\377w toggles, \377yESC\377w accepts)", info);
		//prompt_topline(buf);
		Term_putstr(0, 0, -1, TERM_WHITE, buf);

		/* Display the options */
		for (i = 0; i < n; i++) {
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Color disabled options */
			if (!option_info[opt[i]].o_enabled)
				a = (a == TERM_L_BLUE) ? TERM_SLATE : TERM_L_DARK;

			/* Display the option text */
			sprintf(buf, "%-49s: %s  (%s)",
			        option_info[opt[i]].o_desc,
			        (*option_info[opt[i]].o_var ? "yes" : "no "),
			        option_info[opt[i]].o_text);
			c_prt(a, buf, i + 2, 0);
		}

		/* Hilite current option */
		move_cursor(k + 2, 50);

		/* Get a key */
		ch = inkey();

		/* Analyze */
		switch (ch) {
			case ESCAPE:
			case KTRL('X'):
				return;

			case KTRL('T'):
				/* Take a screenshot */
				xhtml_screenshot("screenshot????");
				break;

			case ':':
				/* specialty: allow chatting from within here */
				cmd_message();
				break;

			case '-':
			case '8':
			case 'k':
				k = (n + k - 1) % n;
				break;

			case ' ':
			case '\n':
			case '\r':
			case '2':
			case 'j':
				k = (k + 1) % n;
				break;

			case 'y':
			case 'Y':
			case '6':
			case 'l':
				(*option_info[opt[k]].o_var) = TRUE;
				Client_setup.options[opt[k]] = TRUE;
				check_immediate_options(opt[k], TRUE, TRUE);
				k = (k + 1) % n;
				break;

			case 'n':
			case 'N':
			case '4':
			case 'h':
				(*option_info[opt[k]].o_var) = FALSE;
				Client_setup.options[opt[k]] = FALSE;
				check_immediate_options(opt[k], FALSE, TRUE);
				k = (k + 1) % n;
				break;

			case 't':
			case 'T':
			case '5':
			case 's':
			{
				bool tmp = 1 - (*option_info[opt[k]].o_var);
				(*option_info[opt[k]].o_var) = tmp;
				Client_setup.options[opt[k]] = tmp;
				check_immediate_options(opt[k], tmp, TRUE);
				k = (k + 1) % n;
				break;
			}
			default:
				bell();
				break;
		}
	}
}


void display_account_information(void) {
	if (acc_opt_screen) {
		if (acc_got_info) {
			if (acc_flags & ACC_TRIAL) {
				c_prt(TERM_YELLOW, "Your account hasn't been validated.", 3, 2);
			} else {
				c_prt(TERM_L_GREEN, "Your account is valid.", 3, 2);
			}
		} else {
			c_prt(TERM_SLATE, "Retrieving data...", 3, 2);
		}
	}
}


/*
 * Account options
 */
static void do_cmd_options_acc(void) {
	char ch;
	bool change_pass = FALSE;
	bool go = TRUE;
	char old_pass[16], new_pass[16], con_pass[16];
	char tmp[16];

	acc_opt_screen = TRUE;
	acc_got_info = FALSE;

	/* Get the account info */
	Send_account_info();

	/* Clear screen */
	Term_clear();

	while (go) {
		/* Clear everything except the topline */
		clear_from(1);

		if (change_pass) {
			prt("Change password", 1, 0);
			prt("Maximum length is 15 characters", 3, 2);
			c_prt(TERM_L_WHITE, "Old password:", 6, 2);
			c_prt(TERM_L_WHITE, "New password:", 8, 2);
			c_prt(TERM_L_WHITE, "Confirm:", 10, 2);
			c_prt(TERM_L_WHITE, "(by typing the new password once more)", 11, 3);

			/* Ask for old password */
			move_cursor(6, 16);
			tmp[0] = '\0';
			if (!askfor_aux(tmp, 15, ASKFOR_PRIVATE)) {
				change_pass = FALSE;
				continue;
			}
			if (!tmp[0]) {
				c_prt(TERM_YELLOW, "Password must not be empty", 8, 2);
				change_pass = FALSE;
				continue;
			}
			my_memfrob(tmp, strlen(tmp));
			strncpy(old_pass, tmp, sizeof(old_pass));
			old_pass[sizeof(old_pass) - 1] = '\0';

			while (1) {
				/* Ask for new password */
				move_cursor(8, 16);
				tmp[0] = '\0'; 
				if (!askfor_aux(tmp, 15, ASKFOR_PRIVATE)) {
					change_pass = FALSE;
					break;
				}
				my_memfrob(tmp, strlen(tmp));
				strncpy(new_pass, tmp, sizeof(new_pass));
				new_pass[sizeof(new_pass) - 1] = '\0';

				/* Ask for the confirmation */
				move_cursor(10, 16);
				tmp[0] = '\0'; 
				if (!askfor_aux(tmp, 15, ASKFOR_PRIVATE)) {
					change_pass = FALSE;
					break;
				}
				my_memfrob(tmp, strlen(tmp));
				strncpy(con_pass, tmp, sizeof(con_pass));
				con_pass[sizeof(con_pass) - 1] = '\0';

				/* Compare */
				if (strcmp(new_pass, con_pass)) {
					c_prt(TERM_YELLOW, "Passwords don't match!", 8, 2);
					Term_erase(16, 8, 255);
					Term_erase(16, 10, 255);
				} else break;
			}

			if (!change_pass) continue;

			/* Send the request */
			if (Send_change_password(old_pass, new_pass) == 1) {
				c_msg_print("\377gPassword change has been submitted.");
			} else {
				c_msg_print("\377yFailed to send password change request.");
			}

			/* Wipe the passwords from memory */
			memset(old_pass, 0, sizeof(old_pass));
			memset(new_pass, 0, sizeof(new_pass));
			memset(con_pass, 0, sizeof(con_pass));

			/* Return to the account options screen */
			change_pass = FALSE;
			continue;
		} else {
			prt("Account information", 1, 0);
			display_account_information();
			Term_putstr(2, 19, -1, TERM_WHITE, "(\377yC\377w) Change account password");
		}

		ch = inkey();

		switch (ch) {
			case ESCAPE:
				go = FALSE;
				break;
			case 'C':
				change_pass = TRUE;
				break;
		}
	}

	acc_opt_screen = FALSE;
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void) {
	int i, j, d, vertikal_offset = 2;

	int y = 0;
	int x = 1;

	char ch;

	bool go = TRUE;

	u32b old_flag[8];


	/* Memorize old flags */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		/* Acquire current flags */
		old_flag[j] = window_flag[j];
	}


	/* Clear screen */
	Term_clear();

	/* Interact */
	while (go) {
		/* Prompt XXX XXX XXX */
		Term_putstr(0, 0, -1, TERM_WHITE, "Window flags (<\377ydir\377w>, \377yt\377w (take), \377yy\377w (set), \377yn\377w (clear), \377yENTER\377w (toggle), \377yESC\377w) ");

		/* Display the windows */
		for (j = 1; j < ANGBAND_TERM_MAX; j++) {
			byte a = TERM_WHITE;
			cptr s = ang_term_name[j];

			/* Use color */
			if (c_cfg.use_color && (j == x)) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, vertikal_offset + j % 2, -1, a, (char*)s);
		}

		/* Display the options */
		for (i = 0; i < NR_OPTIONS_SHOWN; i++) {
			byte a = TERM_WHITE;

			cptr str = window_flag_desc[i];

			/* Use color */
			if (c_cfg.use_color && (i == y)) a = TERM_L_BLUE;

			/* Unused option */
			if (!str) str = "\377D(Unused option)\377w";

			/* Flag name */
			Term_putstr(0, i + vertikal_offset + 2, -1, a, (char*)str);

			/* Display the windows */
			for (j = 1; j < ANGBAND_TERM_MAX; j++)
			{
				byte a = TERM_SLATE;
				char c = '.';

				/* Use color */
				if (c_cfg.use_color && (i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (window_flag[j] & (1L << i)) {
					a = TERM_L_GREEN;
					c = 'X';
				}

				/* Flag value */
				Term_putch(35 + j * 5, i + vertikal_offset + 2, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + vertikal_offset + 2);

		/* Get key */
		ch = inkey();

		/* Analyze */
		switch (ch) {
			case ESCAPE:
				go = FALSE;
				break;

			case KTRL('T'):
				/* Take a screenshot */
				xhtml_screenshot("screenshot????");
				break;

			/* specialty: allow chatting from within here */
			case ':':
				cmd_message();
				break;

			case 'T':
			case 't':
				/* Clear windows */
				for (j = 1; j < ANGBAND_TERM_MAX; j++)
					window_flag[j] &= ~(1L << y);

				/* Clear flags */
				for (i = 1; i < NR_OPTIONS_SHOWN; i++)
					window_flag[x] &= ~(1L << i);

				/* Fall through */

			case 'y':
			case 'Y':
				/* Ignore screen */
				if (x == 0) break;

				/* Set flag */
				window_flag[x] |= (1L << y);

				/* Update windows */
				p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER | PW_MSGNOCHAT | PW_MESSAGE | PW_CHAT | PW_MINIMAP);//PW_LAGOMETER is called automatically, no need.
				window_stuff();
				break;

			case 'n':
			case 'N':
				/* Clear flag */
				window_flag[x] &= ~(1L << y);

				/* Update windows */
				p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER | PW_MSGNOCHAT | PW_MESSAGE | PW_CHAT | PW_MINIMAP);//PW_LAGOMETER is called automatically, no need.
				window_stuff();
				break;

			case '\r':
				/* Toggle flag */
				window_flag[x] ^= (1L << y);

				/* Update windows */
				p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER | PW_MSGNOCHAT | PW_MESSAGE | PW_CHAT | PW_MINIMAP);//PW_LAGOMETER is called automatically, no need.
				window_stuff();
				break;

			default:
				d = keymap_dirs[ch & 0x7F];

				x = (x + ddx[d] + 6) % 7 + 1;
				y = (y + ddy[d] + NR_OPTIONS_SHOWN) % NR_OPTIONS_SHOWN;

				if (!d) bell();
		}
	}

	/* Notice changes */
	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		term *old = Term;

		/* Dead window */
		if (!ang_term[j]) continue;

		/* Ignore non-changes */
		if (window_flag[j] == old_flag[j]) continue;

		/* Activate */
		Term_activate(ang_term[j]);

		/* Erase */
		Term_clear();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}

	/* Update windows */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER | PW_MSGNOCHAT | PW_MESSAGE | PW_CHAT | PW_MINIMAP);//PW_LAGOMETER is called automatically, no need.
	window_stuff();
}

#ifdef ENABLE_SUBWINDOW_MENU
#if defined(WINDOWS) || defined(USE_X11)
#define MAX_FONTS 50
static int font_name_cmp(const void *a, const void *b) {
#if 0 /* simple way */
	return strcmp((const char*)a, (const char*)b);
#else /* sort in single-digit numbers before double-digit ones */
	char at[256], bt[256];
	at[0] = '0';
	bt[0] = '0';
	if (atoi((char*)a) < 10) strcpy(at + 1, (char *)a);
	else strcpy(at, (char *)a);
	if (atoi((char*)b) < 10) strcpy(bt + 1, (char *)b);
	else strcpy(bt, (char *)b);
	return strcmp(at, bt);
#endif
}
static void do_cmd_options_fonts(void) {
	int j, d, vertikal_offset = 3;
	int y = 0;
	char ch;
	bool go = TRUE;

	char font_name[MAX_FONTS][256], path[1024];
	int fonts = 0;
	char tmp_name[256];

#ifdef WINDOWS /* Windows uses the .FON files */
	DIR *dir;
	struct dirent *ent;

	/* read all locally available fonts */
	memset(font_name, 0, sizeof(char) * (MAX_FONTS * 256));

	path_build(path, 1024, ANGBAND_DIR_XTRA, "font");
	if (!(dir = opendir(path))) {
		c_msg_format("Couldn't open fonts directory (%s).", path);
		return;
	}

	while ((ent = readdir(dir))) {
		strcpy(tmp_name, ent->d_name);
		j = -1;
		while(tmp_name[++j]) tmp_name[j] = tolower(tmp_name[j]);
		if (strstr(tmp_name, ".fon")) {
			strcpy(font_name[fonts], ent->d_name);
			fonts++;
		}
	}
	closedir(dir);
#endif

#ifdef USE_X11 /* Linux/OSX use at least the basic system fonts (/usr/share/fonts/misc) */
	/* we boldly assume that these exist by default! */
	strcpy(font_name[0], "5x8");
	strcpy(font_name[1], "6x9");
	strcpy(font_name[2], "8x13");
	strcpy(font_name[3], "9x15");
	strcpy(font_name[4], "10x20");
	strcpy(font_name[5], "12x24");
	fonts = 6;
	//todo: test for more available, good fonts
	/*
	    lucidasanstypewriter-8/10/12/18
	    -misc-fixed-medium-r-normal--15-140-75-75-c-90-iso8859-1
	    -sony-fixed-medium-r-normal--16-*-*-*-*-*-jisx0201.1976-0
	    -misc-fixed-medium-r-normal--20-200-75-75-c-100-iso8859-1
	    -sony-fixed-medium-r-normal--24-170-100-100-c-120-jisx0201.1976-0
	*/
#endif

	if (!fonts) {
		c_msg_format("No .fon font files found in directory (%s).", path);
		return;
	}

	qsort(font_name, fonts, sizeof(char[256]), font_name_cmp);
//	for (j = 0; j < fonts; j++) c_msg_format("'%s'", font_name[j]);

#ifdef WINDOWS /* windows client currently saves full paths (todo: just change to filename only) */
	for (j = 0; j < fonts; j++) {
		strcpy(tmp_name, font_name[j]);
		//path_build(font_name[j], 1024, path, font_name[j]);
		strcpy(font_name[j], ".\\");
		strcat(font_name[j], path);
		strcat(font_name[j], "\\");
		strcat(font_name[j], tmp_name);
	}
#endif


	/* Clear screen */
	Term_clear();

	/* Interact */
	while (go) {
		/* Prompt XXX XXX XXX */
		Term_putstr(0, 0, -1, TERM_WHITE, "  (<\377ydir\377w>, \377yv\377w (visibility), \377y-\377w/\377y+\377w (smaller/bigger), \377yENTER\377w (enter font name), \377yESC\377w)");

		/* Display the windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++) {
			byte a = TERM_WHITE;
			cptr s = ang_term_name[j];
			char buf[256];

			/* Use color */
			if (c_cfg.use_color && (j == y)) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(1, vertikal_offset + j, -1, a, (char*)s);

			/* Display the font of this window */
			if (c_cfg.use_color && !term_get_visibility(j)) a = TERM_L_DARK;
			strcpy(buf, get_font_name(j));
			buf[59] = 0;
			while(strlen(buf) < 59) strcat(buf, " ");
			Term_putstr(20, vertikal_offset + j, -1, a, buf);
		}

		/* Place Cursor */
		//Term_gotoxy(20, vertikal_offset + y);
		/* hack: hide cursor */
		Term->scr->cx = Term->wid;
		Term->scr->cu = 1;

		/* Get key */
		ch = inkey();

		/* Analyze */
		switch (ch) {
		case ESCAPE:
			go = FALSE;
			break;

		case KTRL('T'):
			/* Take a screenshot */
			xhtml_screenshot("screenshot????");
			break;
		case ':':
			/* specialty: allow chatting from within here */
			cmd_message();
			break;

		case 'v':
			if (y == 0) break; /* main window cannot be invisible */
			term_toggle_visibility(y);
			Term_putstr(0, 15, -1, TERM_YELLOW, "-- Changes to window visibilities require a restart of the client --");
			break;

		case '+':
			/* find out which of the fonts in lib/xtra/fonts we're currently using */
			for (j = 0; j < fonts - 1; j++) {
				if (!strcmp(font_name[j], get_font_name(y))) {
					/* advance to next font file in lib/xtra/font */
					set_font_name(y, font_name[j + 1]);
					break;
				}
			}
			break;

		case '-':
			/* find out which of the fonts in lib/xtra/fonts we're currently using */
			for (j = 1; j < fonts; j++) {
				if (!strcmp(font_name[j], get_font_name(y))) {
					/* retreat to previous font file in lib/xtra/font */
					set_font_name(y, font_name[j - 1]);
					break;
				}
			}
			break;

		case '\r':
			Term_putstr(0, 20, -1, TERM_L_GREEN, "Enter a font name:");
			Term_gotoxy(0, 21);
			strcpy(tmp_name, "");
			if (!askfor_aux(tmp_name, 159, 0)) {
				clear_from(20);
				break;
			}
			clear_from(20);
			if (!tmp_name[0]) break;
			set_font_name(y, tmp_name);
			break;

		default:
			d = keymap_dirs[ch & 0x7F];
			y = (y + ddy[d] + NR_OPTIONS_SHOWN) % NR_OPTIONS_SHOWN;
			if (!d) bell();
		}
	}
}
#endif
#endif

errr options_dump(cptr fname)
{
	int i, j;

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* Check if the file already exists */
	fff = my_fopen(buf, "r");

	if (fff) {
		char buf2[1024];
		fclose(fff);

		/* Attempt to rename */
		strcpy(buf2, buf);
		strncat(buf2, ".bak", 1023);
		rename(buf, buf2);
	}

	/* Write to the file */
	fff = my_fopen(buf, "w");

	/* Failure */
	if (!fff) return (-1);


	/* Skip space */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic option dump\n\n");

	/* Dump them */
	for (i = 0; i < OPT_MAX; i++)
	{
		/* Require a real option */
		if (!option_info[i].o_desc) continue;

		/* Comment */
		fprintf(fff, "# Option '%s'\n", option_info[i].o_desc);

		/* Dump the option */
		if (*option_info[i].o_var)
		{
			fprintf(fff, "Y:%s\n", option_info[i].o_text);
		}
		else
		{
			fprintf(fff, "X:%s\n", option_info[i].o_text);
		}

		/* End the option */
		fprintf(fff, "\n");
	}

	fprintf(fff, "\n");

	/* Dump window flags */
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		/* Require a real window */
		if (!ang_term_name[i]) continue;

		/* Check each flag */
		for (j = 0; j < NR_OPTIONS_SHOWN; j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Dump the flag */
			if (window_flag[i] & (1L << j))
			{
				/* Comment */
				fprintf(fff, "# Window '%s', Flag '%s'\n",
						ang_term_name[i], window_flag_desc[j]);

				fprintf(fff, "W:%d:%d\n", i, j);

				/* Skip a line */
				fprintf(fff, "\n");
			}
		}
	}

	/* Finish dumping */
	fprintf(fff, "\n\n\n\n");

	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}


/* Attempt to find sound+music pack 7z files in the client's root folder
   and to install them properly. - C. Blue
   Notes: Uses the GUI version of 7z, 7zG. Reason is that a password prompt
          might be required. That's why non-X11 (ie command-line clients)
          are currently not supported. */

#ifdef WINDOWS
 #include <winreg.h>	/* remote control of installed 7-zip via registry approach */
 #include <process.h>	/* use spawn() instead of normal system() (WINE bug/Win inconsistency even maybe) */
 #define MAX_KEY_LENGTH 255
 #define MAX_VALUE_NAME 16383
#endif

static void do_cmd_options_install_audio_packs(void) {
	FILE *fff;
	char path[1024], c, ch, out_val[1024];

#ifdef WINDOWS /* use windows registry to locate 7-zip */
	HKEY hTestKey;
	char path_7z[1024], path_7z_quoted[1024];
	LPBYTE path_7z_p = (LPBYTE)path_7z;
	int path_7z_size = 1023;
	LPDWORD path_7z_size_p = (LPDWORD)&path_7z_size;
	unsigned long path_7z_type = REG_SZ;
#endif
	int r;

	bool sound_pack = TRUE, music_pack = TRUE;
	bool sound_already = (audio_sfx > 3), music_already = (audio_music > 0);

	/* Clear screen */
	Term_clear();
	Term_putstr(0, 0, -1, TERM_WHITE, "Trying to install sound pack and music pack from 7z files...");
	Term_fresh();
	if (quiet_mode) {
		Term_putstr(0, 1, -1, TERM_RED, "Client is running in 'quiet mode'. Cannot install audio packs!");
		Term_putstr(0, 2, -1, TERM_RED, "(Restart TomeNET client with 'Sound=1' and without '-q'.)");
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	}

	/* test for availability of unarchiver */
#ifdef WINDOWS
	/* check registry for 7zip (note that for example WinRAR could cause problems with 7z files) */
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\7-Zip\\"), 0, KEY_READ, &hTestKey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hTestKey, "Path", NULL, &path_7z_type, path_7z_p, path_7z_size_p) == ERROR_SUCCESS) {
			path_7z[path_7z_size] = '\0';
		} else {
			// odd case
			RegCloseKey(hTestKey);
			Term_putstr(0, 1, -1, TERM_RED, "7-zip not properly installed. Please reinstall it. (www.7-zip.org)");
			Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
			inkey();
			return;
		}
	} else {
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\7-Zip\\"), 0, KEY_READ | 0x0200, &hTestKey) == ERROR_SUCCESS) {//KEY_WOW64_32KEY (0x0200)
			if (RegQueryValueEx(hTestKey, "Path", NULL, &path_7z_type, path_7z_p, path_7z_size_p) == ERROR_SUCCESS) {
				path_7z[path_7z_size] = '\0';
			} else {
				// odd case
				RegCloseKey(hTestKey);
				Term_putstr(0, 1, -1, TERM_RED, "7-zip not properly installed. Please reinstall it. (www.7-zip.org)");
				Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
				inkey();
				return;
			}
		} else {
			/* This case should work on 64-bit Windows (w/ 32-bit client) */
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\7-Zip\\"), 0, KEY_READ | 0x0100, &hTestKey) == ERROR_SUCCESS) {//KEY_WOW64_64KEY (0x0100)
				if (RegQueryValueEx(hTestKey, "Path", NULL, &path_7z_type, path_7z_p, path_7z_size_p) == ERROR_SUCCESS) {
					path_7z[path_7z_size] = '\0';
				} else {
					// odd case
					RegCloseKey(hTestKey);
					Term_putstr(0, 1, -1, TERM_RED, "7-zip not properly installed. Please reinstall it. (www.7-zip.org)");
					Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
					inkey();
					return;
				}
			} else {
				Term_putstr(0, 1, -1, TERM_RED, "7-zip not found. Please install it first: www.7-zip.org");
				Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
				inkey();
				return;
			}
		}
	}
	RegCloseKey(hTestKey);
	/* enclose full path in quotes, to handle possible spaces */
	strcpy(path_7z_quoted, "\"");
	strcat(path_7z_quoted, path_7z);
	strcat(path_7z_quoted, "\\7zG.exe\"");
	strcat(path_7z, "\\7zG.exe");

	/* do the same tests once more as for posix clients */
 	fff = fopen("tmp", "w");
 	fprintf(fff, "mh");
 	fclose(fff);

	_spawnl(_P_WAIT, path_7z, path_7z_quoted, "a", "tmp.7z", "tmp", NULL); /* supposed to work on WINE, yet crashes if not exit(0)ing next oO */
	remove("tmp");

	if (!(fff = fopen("tmp.7z", "r"))) { /* paranoia? */
		Term_putstr(0, 1, -1, TERM_RED, "7-zip wasn't installed properly. Please reinstall it. (www.7-zip.org)");
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	} else if (fgetc(fff) == EOF) { /* normal */
		Term_putstr(0, 1, -1, TERM_RED, "7-zip wasn't installed properly. Please reinstall it. (www.7-zip.org)");
		fclose(fff);
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	}
	Term_putstr(0, 1, -1, TERM_WHITE, "Unarchiver 7-Zip (7zG.exe) found.");
#else /* assume posix */
if (!strcmp(ANGBAND_SYS, "x11")) {
 #if 0	/* command-line 7z */
	r = system("7z > tmp.7z");
 #else	/* GUI 7z (for password prompts) */
 	fff = fopen("tmp", "w");
 	fclose(fff);
	r = system("7zG a tmp.7z tmp");
	remove("tmp");
 #endif
        if (!(fff = fopen("tmp.7z", "r"))) { /* paranoia? */
		Term_putstr(0, 1, -1, TERM_RED, "7-zip GUI not found ('7zG'). Install it first. (Package name is 'p7zip'.)");
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	} else if (fgetc(fff) == EOF) { /* normal */
		Term_putstr(0, 1, -1, TERM_RED, "7-zip GUI not found ('7zG'). Install it first. (Package name is 'p7zip'.)");
		fclose(fff);
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	}
	Term_putstr(0, 1, -1, TERM_WHITE, "Unarchiver (7zG) found.");
} else { /* gcu */
	/* assume posix; ncurses commandline */
	r = system("7z > tmp.7z");
	if (!(fff = fopen("tmp.7z", "r"))) { /* paranoia? */
		Term_putstr(0, 1, -1, TERM_RED, "7-zip not found ('7z'). Install it first. (Package name is 'p7zip'.)");
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	} else if (fgetc(fff) == EOF) { /* normal */
		Term_putstr(0, 1, -1, TERM_RED, "7-zip not found ('7z'). Install it first. (Package name is 'p7zip'.)");
		fclose(fff);
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	}
	Term_putstr(0, 1, -1, TERM_WHITE, "Unarchiver (7z) found.");
}
#endif
	Term_fresh();
	fclose(fff);
	remove("tmp.7z");

	/* test for existance of sound pack file */
	if (!(fff = fopen("TomeNET-soundpack.7z", "r"))) sound_pack = FALSE;
	else fclose(fff);

	/* test for existance of music pack file */
	if (!(fff = fopen("TomeNET-musicpack.7z", "r"))) music_pack = FALSE;
	else fclose(fff);

	if (!sound_pack && !music_pack) {
		Term_putstr(0, 3, -1, TERM_ORANGE, "Neither file 'TomeNET-soundpack.7z' nor 'TomeNET-musicpack.7z' were");
		Term_putstr(0, 4, -1, TERM_ORANGE, "found in your TomeNET folder. Aborting audio pack installation.");
		Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
		inkey();
		return;
	}

	/* verify if we'd be overwriting stuff */
	if (!sound_pack) {
		Term_putstr(0, 3, -1, TERM_SLATE, "(File 'TomeNET-soundpack.7z' not found, skipping.)");
	}
	if (sound_pack && sound_already) {
		Term_putstr(0, 3, -1, TERM_YELLOW, "It seems a sound pack is already installed.");
		Term_putstr(0, 4, -1, TERM_YELLOW, "Do you want to extract 'TomeNET-soundpack.7z' and overwrite it? (y/n)");
		while (TRUE) {
			c = inkey();
			if (c == 'n' || c == 'N') {
				sound_pack = FALSE;
				break;
			}
			if (c == 'y' || c == 'Y') break;
		}
	}

	/* install sound pack */
	if (sound_pack) {
		Term_putstr(0, 3, -1, TERM_WHITE, "Installing sound pack...                   ");
		Term_putstr(0, 4, -1, TERM_WHITE, "                                                                            ");
		Term_fresh();
		Term_flush();
#if defined(WINDOWS)
		_spawnl(_P_WAIT, path_7z, path_7z_quoted, "x", "TomeNET-soundpack.7z", NULL);
		path_build(path, 1024, ANGBAND_DIR_XTRA, "sound");
		sprintf(out_val, "xcopy /I /E /Q /Y /H sound %s", path);
		r = system(out_val);
		r = system("rmdir /S /Q sound");
#else /* assume posix */
if (!strcmp(ANGBAND_SYS, "x11")) {
		r = system("7zG x TomeNET-soundpack.7z");
		path_build(path, 1024, ANGBAND_DIR_XTRA, "sound");
		//r = system(format("mv sound %s", path));
		mkdir(path, 0777); /* in case someone deleted his whole sound folder */
		sprintf(out_val, "cp --recursive -f sound/* %s/", path);
		r = system(out_val);
		r = system("rm -rf sound");
} else { /* gcu */
		r = system("7z x TomeNET-soundpack.7z");
		path_build(path, 1024, ANGBAND_DIR_XTRA, "sound");
		//r = system(format("mv sound %s", path));
		mkdir(path, 0777); /* in case someone deleted his whole sound folder */
		sprintf(out_val, "cp --recursive -f sound/* %s/", path);
		r = system(out_val);
		r = system("rm -rf sound");
}
#endif
		Term_putstr(0, 3, -1, TERM_L_GREEN, "Sound pack has been installed.             ");
		Term_putstr(0, 4, -1, TERM_L_GREEN, "YOU NEED TO RESTART TomeNET FOR THIS TO TAKE EFFECT.                        ");
	}

	/* verify if we'd be overwriting stuff */
	if (!music_pack) {
		Term_putstr(0, 6, -1, TERM_SLATE, "(File 'TomeNET-musicpack.7z' not found, skipping.)");
	}
	if (music_pack && music_already) {
		Term_putstr(0, 6, -1, TERM_YELLOW, "It seems a music pack is already installed.");
		Term_putstr(0, 7, -1, TERM_YELLOW, "Do you want to extract 'TomeNET-musicpack.7z' and overwrite it? (y/n)");
		while (TRUE) {
			c = inkey();
			if (c == 'n' || c == 'N') {
				music_pack = FALSE;
				break;
			}
			if (c == 'y' || c == 'Y') break;
		}
	}

	/* install music pack */
	if (music_pack) {
		Term_putstr(0, 6, -1, TERM_WHITE, "Installing music pack...                   ");
		Term_putstr(0, 7, -1, TERM_WHITE, "                                                                            ");
		Term_fresh();
		Term_flush();
#if defined(WINDOWS)
		_spawnl(_P_WAIT, path_7z, path_7z_quoted, "x", "TomeNET-musicpack.7z", NULL);
		path_build(path, 1024, ANGBAND_DIR_XTRA, "music");
		sprintf(out_val, "xcopy /I /E /Q /Y /H music %s", path);
		r = system(out_val);
		r = system("rmdir /S /Q music");
#else /* assume posix */
if (!strcmp(ANGBAND_SYS, "x11")) {
		r = system("7zG x TomeNET-musicpack.7z");
		path_build(path, 1024, ANGBAND_DIR_XTRA, "music");
		//r = system(format("mv music %s", path));
		mkdir(path, 0777); /* in case someone deleted his whole music folder */
		sprintf(out_val, "cp --recursive -f music/* %s/", path);
		r = system(out_val);
		r = system("rm -rf music");
} else { /* gcu */
		r = system("7z x TomeNET-musicpack.7z");
		path_build(path, 1024, ANGBAND_DIR_XTRA, "music");
		//r = system(format("mv music %s", path));
		mkdir(path, 0777); /* in case someone deleted his whole music folder */
		sprintf(out_val, "cp --recursive -f music/* %s/", path);
		r = system(out_val);
		r = system("rm -rf music");
}
#endif
		Term_putstr(0, 6, -1, TERM_L_GREEN, "Music pack has been installed.             ");
		Term_putstr(0, 7, -1, TERM_L_GREEN, "YOU NEED TO RESTART TomeNET FOR THIS TO TAKE EFFECT.                        ");
	}

	r = r;//slay silly compiler warning

	Term_putstr(0, 9, -1, TERM_WHITE, "Press any key to return to options menu...");
	Term_fresh();
	/* inkey() will react to client timeout after long extraction time, terminating
	   near-instantly with hardly a chance to see the green 'installed' messages. */
	Term_inkey(&ch, TRUE, TRUE);
	//inkey();
}

/*
 * Set or unset various options.
 *
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 */
void do_cmd_options(void) {
	int k;
	char tmp[1024];

	bool dummy = c_cfg.exp_need;

	/* Save the screen */
	Term_save();


	/* Interact */
	while (1) {
		/* Clear screen */
		Term_clear();

		/* Why are we here */
		c_prt(TERM_L_GREEN, "TomeNET options", 2, 0);

		/* Give some choices */
		Term_putstr(3, 4, -1, TERM_WHITE, "(\377y1\377w) User Interface Options 1");
		Term_putstr(3, 5, -1, TERM_WHITE, "(\377y2\377w) User Interface Options 2");
		Term_putstr(3, 6, -1, TERM_WHITE, "(\377y3\377w) Audio Options");
		Term_putstr(3, 7, -1, TERM_WHITE, "(\377y4\377w) Game-Play Options 1");
		Term_putstr(3, 8, -1, TERM_WHITE, "(\377y5\377w) Game-Play Options 2");
		Term_putstr(3, 9, -1, TERM_WHITE, "(\377yw\377w) Window Flags");
		Term_putstr(3,10, -1, TERM_WHITE, "(\377ys\377w) Save Options & Flags");
		Term_putstr(3,11, -1, TERM_WHITE, "(\377yl\377w) Load Options & Flags");

#if defined(WINDOWS) || defined(USE_X11)
 #ifdef ENABLE_SUBWINDOW_MENU
		Term_putstr(3,13, -1, TERM_WHITE, "(\377yf\377w) Window Fonts and Visibility");
 #endif
		/* CHANGE_FONTS_X11 */
		Term_putstr(3,14, -1, TERM_WHITE, "(\377yc\377w) Cycle all font sizes at once (tap multiple times)");
		Term_putstr(3,15, -1, TERM_SLATE, "(Fonts and window visibility are saved automatically on quitting via CTRL+X)");
#endif

		Term_putstr(3,18, -1, TERM_WHITE, "(\377UA\377w) Account Options");
		Term_putstr(3,19, -1, TERM_WHITE, "(\377UI\377w) Install sound/music pack from 7z-file you placed in your TomeNET folder");

		/* Prompt */
		c_prt(TERM_L_GREEN, "Command: ", 21, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE || k == KTRL('X')) break;

		/* Take a screenshot */
		if (k == KTRL('T')) xhtml_screenshot("screenshot????");
		/* specialty: allow chatting from within here */
		else if (k == ':') cmd_message();

		else if (k == '1') {
			do_cmd_options_aux(1, "User Interface Options 1");
		} else if (k == '2') {
			do_cmd_options_aux(4, "User Interface Options 2");
		} else if (k == '3') {
			do_cmd_options_aux(5, "Audio Options");
		} else if (k == '4') {
			do_cmd_options_aux(2, "Game-Play Options 1");
		} else if (k == '5') {
			do_cmd_options_aux(3, "Game-Play Options 2");
		}

		/* Save a 'option' file */
		else if (k == 's') {
			/* Prompt */
			Term_putstr(0, 20, -1, TERM_L_GREEN, "Command: Save an options file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 21, -1, TERM_YELLOW, "File: ");

			/* Default filename */
			//sprintf(tmp, "global.opt");
			sprintf(tmp, "%s.opt", cname);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Dump the macros */
			(void)options_dump(tmp);
		}
		/* Load a pref file */
		else if (k == 'l') {
			/* Prompt */
			Term_putstr(0, 20, -1, TERM_L_GREEN, "Command: Load an options file");

			/* Get a filename, handle ESCAPE */
			Term_putstr(0, 21, -1, TERM_YELLOW, "File: ");

			/* Default filename */
			//sprintf(tmp, "global.opt");
			sprintf(tmp, "%s.opt", cname);

			/* Ask for a file */
			if (!askfor_aux(tmp, 70, 0)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);
		}

		/* Account options */
		else if (k == 'A') do_cmd_options_acc();

		/* Window flags */
		else if (k == 'w') {
			/* Spawn */
			do_cmd_options_win();
		}

#if defined(WINDOWS) || defined(USE_X11)
 #ifdef ENABLE_SUBWINDOW_MENU
		/* Change fonts separately and manually */
		else if (k == 'f') do_cmd_options_fonts();
 #endif
		/* Cycle all fonts */
		else if (k == 'c') change_font(-1);
#endif

		else if (k == 'I') do_cmd_options_install_audio_packs();

		/* Unknown option */
		else {
			/* Oops */
			bell();
		}
	}

	/* Restore the screen */
	Term_load();

	Flush_queue();

	/* Verify the keymap */
	keymap_init();

	/* for exp_need option changes: */
	if (dummy != c_cfg.exp_need)
		prt_level(p_ptr->lev, p_ptr->max_lev, p_ptr->max_plv, p_ptr->max_exp, p_ptr->exp, exp_adv);

	/* Resend options to server */
	Send_options();
}


/*
 * Centers a string within a 31 character string		-JWT-
 */
static void center_string(char *buf, cptr str)
{
	int i, j;

	/* Total length */
	i = strlen(str);

	/* Necessary border */
	j = 15 - i / 2;

	/* Mega-Hack */
	(void)sprintf(buf, "%*s%s%*s", j, "", str, 31 - i - j, "");
}


/*
 * Display a "tomb-stone"
 */
/* ToME parts. */
static void print_tomb(cptr reason)
{
	bool done = FALSE;

	/* Print the text-tombstone */
	if (!done)
	{
		char	tmp[160];

		char	buf[1024];

		FILE        *fp;

		time_t	ct = time((time_t)0);


		/* Clear screen */
		Term_clear();

		/* Build the filename */
		path_build(buf, 1024, ANGBAND_DIR_TEXT, ct % 2 ? "dead.txt" : "dead2.txt");

		/* Open the News file */
		fp = my_fopen(buf, "r");

		/* Dump */
		if (fp)
		{
			int i = 0;

			/* Dump the file to the screen */
			while (0 == my_fgets(fp, buf, 1024))
			{
				/* Display and advance */
				Term_putstr(0, i++, -1, TERM_WHITE, buf);
			}

			/* Close */
			my_fclose(fp);
		}

#if 0	/* make the server send those info! */
		/* King or Queen */
		if (total_winner || (p_ptr->lev > PY_MAX_LEVEL))
		{
			p = "Magnificent";
		}

		/* Normal */
		else
		{
			p =  cp_ptr->titles[(p_ptr->lev-1)/5] + c_text;
		}
#endif	/* 0 */

		center_string(buf, cname);
		put_str(buf, 6, 11);

		center_string(buf, "the");
		put_str(buf, 7, 11);

        	center_string(buf, race_info[race].title);
		put_str(buf, 8, 11);

        	center_string(buf, class_info[class].title);
		put_str(buf, 9, 11);

		(void)sprintf(tmp, "Level: %d", (int)p_ptr->lev);
		center_string(buf, tmp);
		put_str(buf, 11, 11);

		(void)sprintf(tmp, "Exp: %d", p_ptr->exp);
		center_string(buf, tmp);
		put_str(buf, 12, 11);

		/* XXX usually 0 */
		(void)sprintf(tmp, "AU: %d", p_ptr->au);
		center_string(buf, tmp);
		put_str(buf, 13, 11);

		if (c_cfg.depth_in_feet)
			(void)sprintf(tmp, "Died on %dft of [%2d, %2d]", p_ptr->wpos.wz * 50, p_ptr->wpos.wy, p_ptr->wpos.wx);
		else
			(void)sprintf(tmp, "Died on Level %d of [%2d, %2d]", p_ptr->wpos.wz, p_ptr->wpos.wy, p_ptr->wpos.wx);
		center_string(buf, tmp);
		put_str(buf, 14, 11);

#if 0
		(void)sprintf(tmp, "Killed on Level %d", dun_level);
		center_string(buf, tmp);
		put_str(buf, 14, 11);


		if (strlen(died_from) > 24)
		{
			strncpy(dummy, died_from, 24);
			dummy[24] = '\0';
			(void)sprintf(tmp, "by %s.", dummy);
		}
		else
			(void)sprintf(tmp, "by %s.", died_from);

		center_string(buf, tmp);
		put_str(buf, 15, 11);
#endif	/* 0 */


		(void)sprintf(tmp, "%-.24s", ctime(&ct));
		center_string(buf, tmp);
		put_str(buf, 17, 11);

		put_str(reason, 21, 10);
	}
}

/*
 * Display some character info	- Jir -
 * For now, only when losing the character.
 */
void c_close_game(cptr reason)
{
	int k;
	char tmp[MAX_CHARS];

	/* Let the player view the last scene */
	c_msg_format("%s ...Press '0' key to proceed", reason);

	while (inkey() != '0');

	/* You are dead */
	print_tomb(reason);

	put_str("ESC to quit, 'f' to dump the record or any other key to proceed", 23, 0);

	/* TODO: bandle them in one loop instead of 2 */
	while (1)
	{
		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE || k == KTRL('X') || k == 'q' || k == 'Q') return;

		else if (k == KTRL('T'))
		{
			/* Take a screenshot */
			xhtml_screenshot("screenshot????");
		}

		/* Dump */
		else if ((k == 'f') || (k == 'F'))
		{
			strnfmt(tmp, MAX_CHARS - 1, "%s.txt", cname);
			if (get_string("Filename(you can post it to http://angband.oook.cz/): ", tmp, MAX_CHARS - 1))
			{
				if (tmp[0] && (tmp[0] != ' '))
				{
					file_character(tmp, FALSE);
					break;
				}
			}
			continue;
		}
		/* Safeguard */
		else if (k == '0') continue;

		else if (k) break;
	}

	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Why are we here */
		prt("Your tracks", 2, 0);

		/* Give some choices */
		prt("(1) Character", 4, 5);
		prt("(2) Inventory", 5, 5);
		prt("(3) Equipments", 6, 5);
		prt("(4) Messages", 7, 5);
		prt("(5) Chat messages", 8, 5);

		/* What a pity no score list here :-/ */

		/* Prompt */
		prt("Command: ", 17, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE || k == KTRL('X')) break;

		/* Take a screenshot */
		if (k == KTRL('T'))
		{
			xhtml_screenshot("screenshot????");
		}

		/* Character screen */
		else if (k == '1' || k == 'C')
		{
			cmd_character();
		}

		/* Inventory */
		else if (k == '2' || k == 'i')
		{
			cmd_inven();
		}

		/* Equipments */
		else if (k == '3' || k == 'e')
		{
			/* Process the running options */
			cmd_equip();
		}

		/* Message history */
		else if (k == '4' || k == KTRL('P'))
		{
			do_cmd_messages();
		}

		/* Chat history */
		else if (k == '5' || k == KTRL('O'))
		{
			do_cmd_messages_chatonly();
		}

#if 0
		/* Skill browsing ... is not available for now */
		else if (k == '6' || k == KTRL('G'))
		{
			do_cmd_skill();
		}
#endif	/* 0 */

		/* Unknown option */
		else
		{
			/* Oops */
			bell();
		}
	}
}



/*
 * Since only GNU libc has memfrob, we use our own.
 */
void my_memfrob(void *s, int n)
{
        int i;
        char *str;

        str = (char*) s;

        for (i = 0; i < n; i++)
        {
                /* XOR every byte with 42 */
                str[i] ^= 42;
        }
}



/*
 * Check if the server version fills the requirements.
 * Copied from the server code.
 *
 * Branch has to be an exact match.
 */
bool is_newer_than(version_type *version, int major, int minor, int patch, int extra, int branch, int build)
{
	if (version->major < major)
		return FALSE; /* very old */
	else if (version->major > major)
		return TRUE; /* very new */
	else if (version->minor < minor)
		return FALSE; /* pretty old */
	else if (version->minor > minor)
		return TRUE; /* pretty new */
	else if (version->patch < patch)
		return FALSE; /* somewhat old */
	else if (version->patch > patch)
		return TRUE; /* somewhat new */
	else if (version->extra < extra)
		return FALSE; /* a little older */
	else if (version->extra > extra)
		return TRUE; /* a little newer */
	/* Check that the branch is an exact match */
	else if (version->branch == branch)
	{
		/* Now check the build */
		if (version->build < build)
			return FALSE;
		else if (version->build > build)
			return TRUE;
	}

	/* Default */
	return FALSE;
}

bool is_same_as(version_type *version, int major, int minor, int patch, int extra, int branch, int build)
{
	if (version->major == major
	    && version->minor == minor
	    && version->patch == patch
	    && version->extra == extra
	    && version->branch == branch
	    && version->build == build)
		return TRUE;

	return FALSE;
}

/* dummy */
void msg_format(int Ind, cptr fmt, ...) {
	return;
}

#ifdef USE_SOUND_2010
bool sound(int val, int type, int vol, s32b player_id) {
	if (!use_sound) return TRUE;

	/* play a sound */
	if (sound_hook) return sound_hook(val, type, vol, player_id);
	else return FALSE;
}

void sound_weather(int val) {
	if (!use_sound) return;

	/* play a sound */
	if (sound_weather_hook) sound_weather_hook(val);
}

void sound_weather_vol(int val, int vol) {
	if (!use_sound) return;

	/* play a sound */
	if (sound_weather_hook_vol) sound_weather_hook_vol(val, vol);
}

bool music(int val) {
	if (!use_sound) return TRUE;

	/* play a sound */
	if (music_hook) return music_hook(val);
	else return FALSE;
}

void sound_ambient(int val) {
	if (!use_sound) return;

	/* play a sound */
	if (sound_ambient_hook) sound_ambient_hook(val);
}

void set_mixing(void) {
	if (!use_sound) return;

	/* Set the mixer levels */
	if (mixing_hook) mixing_hook();
}

void interact_audio(void) {
	int i, j, item_x[8] = {2, 12, 22, 32, 42, 52, 62, 72};
	static int cur_item = 0;
	int y_label = 20, y_toggle = 12, y_slider = 18;
	bool redraw = TRUE, quit = FALSE;

	/* Save screen */
	Term_save();

	/* Prevent hybrid macros from triggering in here */
	inkey_msg = TRUE;

	/* Process requests until done */
	while (1) {
		if (redraw) {
			/* Clear screen */
			Term_clear();

			/* Describe */
			Term_putstr(30,  0, -1, TERM_L_UMBER, "*** Audio Mixer ***");
			Term_putstr(3, 1, -1, TERM_L_UMBER, "Press arrow keys to navigate/modify, RETURN/SPACE to toggle, ESC to leave.");
//			Term_putstr(6, 2, -1, TERM_L_UMBER, "Shortcuts: 'a': master, 'w': weather, 's': sound, 'c' or 'm': music.");
//			Term_putstr(7, 3, -1, TERM_L_UMBER, "Jump to volume slider: SHIFT + according shortcut key given above.");
			Term_putstr(6, 2, -1, TERM_L_UMBER, "Shortcuts: 'a','w','s','c'/'m'. Shift + shortcut to jump to a slider.");

			if (quiet_mode) Term_putstr(12, 4, -1, TERM_L_RED,                              "  Client is running in 'quiet mode': Audio is disabled.  ");
			else if (audio_sfx > 3 && audio_music > 0) Term_putstr(12, 4, -1, TERM_L_GREEN, "     Sound pack and music pack have been detected.      ");
			else if (audio_sfx > 3 && audio_music == 0) Term_putstr(12, 4, -1, TERM_YELLOW, "Sound pack detected. No music pack seems to be installed.");
			else if (audio_sfx <= 3 && audio_music > 0) Term_putstr(12, 4, -1, TERM_YELLOW, "Music pack detected. No sound pack seems to be installed.");
			else Term_putstr(12, 4, -1, TERM_L_RED,                                         "Neither sound pack nor music pack seems to be installed. ");

			Term_putstr(3, y_label + 2, -1, TERM_SLATE, "Outside of this mixer you can toggle audio and music by CTRL+N and CTRL+C.");

			/* draw mixer */
			Term_putstr(item_x[0], y_toggle, -1, TERM_WHITE, format(" [%s]", cfg_audio_master ? "\377GX\377w" : " "));
			Term_putstr(item_x[0], y_toggle + 3, -1, TERM_SLATE, "CTRL+N");
			Term_putstr(item_x[1], y_toggle, -1, TERM_WHITE, format(" [%s]", cfg_audio_music ? "\377GX\377w" : " "));
			Term_putstr(item_x[1], y_toggle + 3, -1, TERM_SLATE, "CTRL+C");
			Term_putstr(item_x[2], y_toggle, -1, TERM_WHITE, format(" [%s]", cfg_audio_sound ? "\377GX\377w" : " "));
			Term_putstr(item_x[3], y_toggle, -1, TERM_WHITE, format(" [%s]", cfg_audio_weather ? "\377GX\377w" : " "));

			for (i = 4; i <= 7; i++) {
				Term_putstr(item_x[i - 4], y_toggle + 1, -1, TERM_SLATE, "on/off");
				Term_putstr(item_x[i], y_slider - 12, -1, TERM_WHITE, "  ^");
				Term_putstr(item_x[i], y_slider, -1, TERM_WHITE, "  V");
				for (j = y_slider - 1; j >= y_slider - 11; j--)
					Term_putstr(item_x[i], j, -1, TERM_SLATE, "  |");
			}

			Term_putstr(item_x[4], y_slider - cfg_audio_master_volume / 10 - 1, -1, TERM_L_GREEN, "  =");
			Term_putstr(item_x[5], y_slider - cfg_audio_music_volume / 10 - 1, -1, TERM_L_GREEN, "  =");
			Term_putstr(item_x[6], y_slider - cfg_audio_sound_volume / 10 - 1, -1, TERM_L_GREEN, "  =");
			Term_putstr(item_x[7], y_slider - cfg_audio_weather_volume / 10 - 1, -1, TERM_L_GREEN, "  =");
		}
		redraw = TRUE;

		/* display editing 'cursor' */
//		Term_putstr(item_x[cur_item], y_label + 2, -1, TERM_ORANGE, "  ^");
//		Term_putstr(item_x[cur_item], y_label + 3, -1, TERM_ORANGE, "  |");
		Term_putstr(item_x[0], y_label, -1, cur_item == 0 ? TERM_ORANGE : TERM_WHITE, "Master");
		Term_putstr(item_x[1], y_label, -1, cur_item == 1 ? TERM_ORANGE : TERM_WHITE, "Music");
		Term_putstr(item_x[2], y_label, -1, cur_item == 2 ? TERM_ORANGE : TERM_WHITE, "Sound");
		Term_putstr(item_x[3], y_label, -1, cur_item == 3 ? TERM_ORANGE : TERM_WHITE, "Weather");
		Term_putstr(item_x[4], y_label, -1, cur_item == 4 ? TERM_ORANGE : TERM_WHITE, "Master");
		Term_putstr(item_x[5], y_label, -1, cur_item == 5 ? TERM_ORANGE : TERM_WHITE, "Music");
		Term_putstr(item_x[6], y_label, -1, cur_item == 6 ? TERM_ORANGE : TERM_WHITE, "Sound");
		Term_putstr(item_x[7], y_label, -1, cur_item == 7 ? TERM_ORANGE : TERM_WHITE, "Weather");


		/* make cursor invisible */
		Term_set_cursor(0);
		inkey_flag = TRUE;

		/* Wait for keypress */
		switch (inkey()) {
		case KTRL('T'):
			/* Take a screenshot */
			xhtml_screenshot("screenshot????");
			redraw = FALSE;
			break;
		case KTRL('U'):
		case ESCAPE:
			quit = TRUE; /* hack to leave loop */
			break;
		case 'n':
		case '6':
//			Term_putstr(item_x[cur_item], y_label + 2, -1, TERM_ORANGE, "   ");
//			Term_putstr(item_x[cur_item], y_label + 3, -1, TERM_ORANGE, "   ");
			cur_item++;
			if (cur_item > 7) cur_item = 0;
			redraw = FALSE;
			break;
		case 'p':
		case '4':
//			Term_putstr(item_x[cur_item], y_label + 2, -1, TERM_ORANGE, "   ");
//			Term_putstr(item_x[cur_item], y_label + 3, -1, TERM_ORANGE, "   ");
			cur_item--;
			if (cur_item < 0) cur_item = 7;
			redraw = FALSE;
			break;
		case '8':
		case '+':
			switch (cur_item) {
			case 0: cfg_audio_master = !cfg_audio_master; break;
			case 1: cfg_audio_music = !cfg_audio_music; break;
			case 2: cfg_audio_sound = !cfg_audio_sound; break;
			case 3: cfg_audio_weather = !cfg_audio_weather; break;
			case 4: if (cfg_audio_master_volume <= 90) cfg_audio_master_volume += 10; else cfg_audio_master_volume = 100; break;
			case 5: if (cfg_audio_music_volume <= 90) cfg_audio_music_volume += 10; else cfg_audio_music_volume = 100; break;
			case 6: if (cfg_audio_sound_volume <= 90) cfg_audio_sound_volume += 10; else cfg_audio_sound_volume = 100; break;
			case 7: if (cfg_audio_weather_volume <= 90) cfg_audio_weather_volume += 10; else cfg_audio_weather_volume = 100; break;
			}
			set_mixing();
			break;
		case '2':
		case '-':
			switch (cur_item) {
			case 0: cfg_audio_master = !cfg_audio_master; break;
			case 1: cfg_audio_music = !cfg_audio_music; break;
			case 2: cfg_audio_sound = !cfg_audio_sound; break;
			case 3: cfg_audio_weather = !cfg_audio_weather; break;
			case 4: if (cfg_audio_master_volume >= 10) cfg_audio_master_volume -= 10; else cfg_audio_master_volume = 0; break;
			case 5: if (cfg_audio_music_volume >= 10) cfg_audio_music_volume -= 10; else cfg_audio_music_volume = 0; break;
			case 6: if (cfg_audio_sound_volume >= 10) cfg_audio_sound_volume -= 10; else cfg_audio_sound_volume = 0; break;
			case 7: if (cfg_audio_weather_volume >= 10) cfg_audio_weather_volume -= 10; else cfg_audio_weather_volume = 0; break;
			}
			set_mixing();
			break;
		case KTRL('N'):
		case 'a':
			cfg_audio_master = !cfg_audio_master;
			set_mixing();
			break;
		case KTRL('C'):
		case 'c':
		case 'm':
			cfg_audio_music = !cfg_audio_music;
			set_mixing();
			break;
		case 's':
			cfg_audio_sound = !cfg_audio_sound;
			set_mixing();
			break;
		case 'w':
			cfg_audio_weather = !cfg_audio_weather;
			set_mixing();
			break;
		case 'A':
			cur_item = 4;
			break;
		case 'C':
		case 'M':
			cur_item = 5;
			break;
		case 'S':
			cur_item = 6;
			break;
		case 'W':
			cur_item = 7;
			break;
		case '\n':
		case '\r':
		case ' ':
			switch (cur_item) {
			case 0: cfg_audio_master = !cfg_audio_master; break;
			case 1: cfg_audio_music = !cfg_audio_music; break;
			case 2: cfg_audio_sound = !cfg_audio_sound; break;
			case 3: cfg_audio_weather = !cfg_audio_weather; break;
			case 4: if (cfg_audio_master_volume <= 90) cfg_audio_master_volume += 10; else cfg_audio_master_volume = 0; break;
			case 5: if (cfg_audio_music_volume <= 90) cfg_audio_music_volume += 10; else cfg_audio_music_volume = 0; break;
			case 6: if (cfg_audio_sound_volume <= 90) cfg_audio_sound_volume += 10; else cfg_audio_sound_volume = 0; break;
			case 7: if (cfg_audio_weather_volume <= 90) cfg_audio_weather_volume += 10; else cfg_audio_weather_volume = 0; break;
			}
			set_mixing();
			break;
		default:
			/* Oops */
			bell();
		}

		/* Leave */
		if (quit) break;
	}

	/* Reload screen */
	Term_load();

	/* Flush the queue */
	Flush_queue();

	/* Re-enable hybrid macros */
	inkey_msg = FALSE;
}
void toggle_music(void) {
	cfg_audio_music = !cfg_audio_music;
	set_mixing();
}
void toggle_audio(void) {
	cfg_audio_master = !cfg_audio_master;
	set_mixing();
}
#endif

/* For pasting monster lore into chat, also usable for item-pasting. - C. Blue
   Important feature: Replaces first ':' by '::' if sending to normal chat. */
void Send_paste_msg(char *msg) {
	/* replace first : by :: in case we're going for normal chat mode */
	char buf[MSG_LEN], *c;

	if (chat_mode == CHAT_MODE_PARTY)
		Send_msg(format("!:%s", msg));
	else if (chat_mode == CHAT_MODE_LEVEL)
		Send_msg(format("#:%s", msg));
	else if (chat_mode == CHAT_MODE_GUILD)
		Send_msg(format("$:%s", msg));
	else {
		strcpy(buf, msg);
		if ((c = strchr(msg, ':')) &&
		    *(c + 1) != ':') {
			buf[c - msg + 1] = ':';
			strcpy(&buf[c - msg + 2], c + 1);
		}
		Send_msg(format("%s", buf));
	}
}

/* Check if certain options have an immediate effect when toggled,
   for switching BIG_MAP feature on/off live. - C. Blue */
#define PANEL_X	(SCREEN_PAD_LEFT)
#define PANEL_Y	(SCREEN_PAD_TOP)
void check_immediate_options(int i, bool yes, bool playing) {
#ifdef USE_GCU
	/* BIG_MAP is currently not supported in GCU client */
	if (!strcmp(ANGBAND_SYS, "gcu")) {
		if (option_info[i].o_var == &c_cfg.big_map)
			c_cfg.big_map = FALSE;
			Client_setup.options[i] = FALSE;
			screen_hgt = SCREEN_HGT;
	}
#endif

	if (option_info[i].o_var == &c_cfg.big_map
            && is_newer_than(&server_version, 4, 4, 9, 1, 0, 1) /* redundant */
            && (sflags1 & SFLG1_BIG_MAP)) {
		if (!yes && screen_hgt != SCREEN_HGT) {
	        	screen_hgt = SCREEN_HGT;
    		        resize_main_window(CL_WINDOW_WID, CL_WINDOW_HGT);
	                /* too early, connection not ready yet? (otherwise done in Input_loop()) */
    		        if (playing) {
				if (screen_icky) Term_switch(0);
				Term_clear(); /* get rid of map tiles where now status bars go instead */
				if (screen_icky) Term_switch(0);
				Send_screen_dimensions();
    		    	}
		}
		if (yes && screen_hgt <= SCREEN_HGT) {
                        screen_hgt = MAX_SCREEN_HGT;
                        resize_main_window(CL_WINDOW_WID, CL_WINDOW_HGT);
	                /* too early, connection not ready yet? (otherwise done in Input_loop()) */
    		        if (playing) {
				if (screen_icky) Term_switch(0);
				Term_clear(); /* paranoia ;) */
				if (screen_icky) Term_switch(0);
				Send_screen_dimensions();
    		    	}
		}
	}

	/* Terminate all weather visuals and sounds via 'no_weather' option? */
	if (!noweather_mode && option_info[i].o_var == &c_cfg.no_weather && c_cfg.no_weather) {
		int i;

		/* restore tiles on display that were overwritten by weather */
		if (screen_icky) Term_switch(0);
		for (i = 0; i < weather_elements; i++) {
			/* only for elements within visible panel screen area */
			if (weather_element_x[i] >= weather_panel_x &&
			    weather_element_x[i] < weather_panel_x + screen_wid &&
			    weather_element_y[i] >= weather_panel_y &&
			    weather_element_y[i] < weather_panel_y + screen_hgt) {
				/* restore original grid content */
				Term_draw(PANEL_X + weather_element_x[i] - weather_panel_x,
				    PANEL_Y + weather_element_y[i] - weather_panel_y,
				    panel_map_a[weather_element_x[i] - weather_panel_x][weather_element_y[i] - weather_panel_y],
				    panel_map_c[weather_element_x[i] - weather_panel_x][weather_element_y[i] - weather_panel_y]);
			}
		}
		if (screen_icky) Term_switch(0);
		/* terminate weather */
		weather_elements = 0;
		weather_type = 0;

#ifdef USE_SOUND_2010
		if (use_sound) sound_weather(-2); //stop
#endif
	}

	/* Hide the cursor again after disabling self-highlighting */
	if (option_info[i].o_var == &c_cfg.hilite_player && !c_cfg.hilite_player) {
		if (screen_icky) Term_switch(0); //should always be icky since we're in = menu..
		Term_set_cursor(0);
		if (screen_icky) Term_switch(0);
	}
}

/* Helper functions for DONT_CLEAR_TOPLINE_IF_AVOIDABLE - C. Blue */
void prompt_topline(cptr prompt) {
#if 0
 #ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
	/* store prompt in case macro fails at an item prompt etc */
	strcpy(last_prompt, prompt);
	last_prompt_macro = parse_macro;
	if (!parse_macro)
 #endif
	prt(prompt, 0, 0);
#else
 if (c_cfg.keep_topline) {
	/* store prompt in case macro fails at an item prompt etc */
	strcpy(last_prompt, prompt);
	last_prompt_macro = parse_macro;
	if (!parse_macro)
		prt(prompt, 0, 0);
 } else
	prt(prompt, 0, 0);
#endif
}
void clear_topline(void) {
#if 0
 #ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
	/* clear stored prompt */
	last_prompt[0] = 0;
	if (!parse_macro || !last_prompt_macro)
 #endif
	prt("", 0, 0);
	//last_prompt_macro = parse_macro; not needed (it's just done in next prompt_topline call that follows)
#else
 if (c_cfg.keep_topline) {
	/* clear stored prompt */
	last_prompt[0] = 0;
	if (!parse_macro || !last_prompt_macro)
		prt("", 0, 0);
 } else
	prt("", 0, 0);
#endif
}
void clear_topline_forced(void) {
	last_prompt[0] = 0;
	prt("", 0, 0);
}
//#ifdef DONT_CLEAR_TOPLINE_IF_AVOIDABLE
/* If a macro failed while waiting for input at topline prompt,
   then restore the prompt, so the user can manually respond. */
void restore_prompt(void) {
	if (last_prompt[0] && parse_macro)
		prt(last_prompt, 0, 0);
	/* clear for next time (paranoia) */
	last_prompt[0] = 0;
}
//#endif
