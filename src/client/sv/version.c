#include <sys/time.h>
#ifdef WIN32
#include <winsock.h>
#undef GF_INERTIA
#endif
#define CLIENT
#include "../../common/angband.h"
/* Local baseline predicate keeps SV independent of unrelated common helpers. */
bool is_newer_than(version_type *version, int major, int minor, int patch, int extra, int branch, int build) {
//#ifdef ATMOSPHERIC_INTRO /* only defined client-side, so commented out is simpler for now */
	/* hack for animating colours before we even have network contact to server */
	if (!version->major) return(TRUE);
//#endif

	if (version->major < major)
		return(FALSE); /* very old */
	else if (version->major > major)
		return(TRUE); /* very new */
	else if (version->minor < minor)
		return(FALSE); /* pretty old */
	else if (version->minor > minor)
		return(TRUE); /* pretty new */
	else if (version->patch < patch)
		return(FALSE); /* somewhat old */
	else if (version->patch > patch)
		return(TRUE); /* somewhat new */
	else if (version->extra < extra)
		return(FALSE); /* a little older */
	else if (version->extra > extra)
		return(TRUE); /* a little newer */
#if 0 /* for current version checks this isn't helpful, as we cannot handle different branches atm */
	/* Check that the branch is an exact match */
	else if (version->branch == branch)
	{
		/* Now check the build */
		if (version->build < build)
			return(FALSE);
		else if (version->build > build)
			return(TRUE);
	}
#else /* so instead, just make it straightforward < > comparison as for all the other fields */
	else if (version->branch < branch)
		return(FALSE); /* somewhat old */
	else if (version->branch > branch)
		return(TRUE); /* somewhat new */
	else if (version->build < build)
		return(FALSE); /* a little older */
	else if (version->build > build)
		return(TRUE); /* a little newer */
#endif

	/* Default */
	return(FALSE);
}
