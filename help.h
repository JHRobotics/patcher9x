/******************************************************************************
 * Copyright (c) 2022 Jaroslav Hensl                                          *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person                *
 * obtaining a copy of this software and associated documentation             *
 * files (the "Software"), to deal in the Software without                    *
 * restriction, including without limitation the rights to use,               *
 * copy, modify, merge, publish, distribute, sublicense, and/or sell          *
 * copies of the Software, and to permit persons to whom the                  *
 * Software is furnished to do so, subject to the following                   *
 * conditions:                                                                *
 *                                                                            *
 * The above copyright notice and this permission notice shall be             *
 * included in all copies or substantial portions of the Software.            *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,            *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES            *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                   *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT                *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,               *
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING               *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR              *
 * OTHER DEALINGS IN THE SOFTWARE.                                            *
 *                                                                            *
 ******************************************************************************/
 
/*
 *  options:
 *   HELP_NAME  - name of string variable 
 *   HELP_SHORT - shot help version
 *   HELP_EXTRA - enable extra text on net
 *   HELP_PROGNAME  - program name or "%s"
 *
 */

#ifndef HELP_NAME
#define HELP_NAME help
#endif

#ifndef HELP_PROGNAME
#define HELP_PROGSTR "%s"
#else
#define HELP_PROGSTR HELP_PROGNAME
#endif

#define HELP_NAME_ARGS(_def) _def##_args

static const char HELP_NAME[] = "Patch for Windows 9x for run on newest CPUs - AMD ZEN 2+, Intel Tiger Lake+\n"
	"Version: " PATCHER9X_VERSION_STR "\n\n"
	"Usage:\n" HELP_PROGSTR " [options] [path]\n"
	"  or\n"
	HELP_PROGSTR " [options] <batch function> <batch arguments>\n"
	"path: path to installed windows directory or directory with windows installation\n"
	"options:\n"
#ifndef HELP_SHORT /* shorten help a bit to fit 80x25 screen */
	"\t-h,/?: print this help\n"
	"\t-v: print program version\n"
#else
  "\t-hh: print longer help\n"
#endif
	"\t-auto,-y: use default actions (if path given) and don't bother user\n"
	"\t-no-backup: don't backup files before modify\n"
#ifndef HELP_SHORT
	"\t-force-w3: when patching VMM32.VXD, leave it as W3 file\n"
	"\t-force-w4: when patching VMM32.VXD, always compress to W4 file\n"
	"\t-force-tlb: always try to apply TLB patch set\n"
	"\t-force-cpupatch: always try to apply CPU SPEED patch set\n"
	"\t-force-cpupatch-ndis: always try to apply CPU SPEED patch for NDIS.VXD\n"
	"\t-no-tlb: do not apply TLB patches\n"
	"\t-no-cpupatch: do not apply CPU SPEED patches\n"
	"\t-no-cpupatch-ndis: do not apply CPU SPEED patches for NDIS.VXD\n"
	"\t-millennium: ignored, Me patch is included in TLB set\n"
#endif
	"\n"
#ifdef HELP_EXTRA
	"When running without arguments, program operate in interactive mode (ask user)\n"
#endif
;

#ifndef HELP_LONG_REP_ARG
# define HELP_LONG_REP_ARG(_arg) (_arg),(_arg)
#endif

#ifndef HELP_SHORT_REP_ARG
# define HELP_SHORT_REP_ARG(_arg) (_arg),(_arg)
#endif

#undef HELP_NAME_ARGS
#undef HELP_PROGSTR
#undef HELP_NAME
