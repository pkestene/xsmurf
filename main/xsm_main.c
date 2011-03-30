/* 
 * xsm_main.c --
 *
 *	This file contains the main program for xsmurf application.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1996 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * $Id: xsm_main.c,v 1.13 1999/05/06 13:19:46 decoster Exp $
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif

//#include "../config.h"

#ifdef LOG_MESSAGES
#include "mainLogMsg.h"

static char *log_file_name = "~/.smurfLog";
static int is_log_file_name_set = 0;
#endif

#ifdef FFTW_THREADS
#include "../image/fftw3_inc.h"
#endif


/*
 * Declarations for various library procedures and variables (don't want
 * to include tkInt.h or tkPort.h here, because people might copy this
 * file out of the Tk source directory to make their own modified versions).
 * Note: don't declare "exit" here even though a declaration is really
 * needed, because it will conflict with a declaration elsewhere on
 * some systems.
 */

extern int		isatty _ANSI_ARGS_((int fd));
extern int		read _ANSI_ARGS_((int fd, char *buf, size_t size));
extern char *		strrchr _ANSI_ARGS_((CONST char *string, int c));

/*
 * Global variables used by the main program:
 */

static Tcl_Interp *interp;	/* Interpreter for this application. */
static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static Tcl_DString line;	/* Used to read the next line from the
                                 * terminal input. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */

/*
 * Forward declarations for procedures defined later in this file.
 */

#ifdef USE_READLINE
static void             Tk_DoNothing _ANSI_ARGS_((ClientData clientData,
						  int mask));
static int              Tk_rl_DoOneEvent ();
#endif
static void		Prompt _ANSI_ARGS_((Tcl_Interp *interp, int partial));
static void		StdinProc _ANSI_ARGS_((ClientData clientData,
			    int mask));

static void myTk_MainEx(int , char **, Tcl_AppInitProc *, Tcl_Interp *);



/*
 * Bidouillage pour contourner l'initialisation de la colormap....
 */
int noColorMapFlag;

/*
 * Bidouillage, encore....
 */
int noDisplayFlag;


/*
 *----------------------------------------------------------------------
 *
 * Tk_MainEx --
 *
 *	Main program for Wish and most other Tk-based applications.
 *
 * Results:
 *	None. This procedure never returns (it exits the process when
 *	it's done.
 *
 * Side effects:
 *	This procedure initializes the Tk world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

void
myTk_MainEx(int argc, char *argv[], Tcl_AppInitProc *appInitProc, Tcl_Interp *the_interp)
     /*int argc;*/     			/* Number of arguments. */
     /*char **argv;*/			/* Array of argument strings. */
     /*Tcl_AppInitProc *appInitProc;*/	/* Application-specific initialization
					 * procedure to call after most
					 * initialization but before starting
					 * to execute commands. */
{
    char *args, *fileName;
    char buf[20];
    int code;
    size_t length;
    Tcl_Channel inChannel, outChannel, errChannel;

#ifdef USE_READLINE
    int count, gotPartial = 0;
    char *cmd, *input;
#endif

    Tcl_FindExecutable(argv[0]);
    interp = the_interp; /*Tcl_CreateInterp();*/
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

#ifdef LOG_MESSAGES
    if (!is_log_file_name_set) {
      Tcl_DString temp;
      char *fullName;
      fullName = Tcl_TranslateFileName(interp, log_file_name, &temp);
      log_file_name = (char *) malloc (strlen(fullName)*sizeof(char));
      strcpy (log_file_name, fullName);
      SetLogFileName(log_file_name);
      is_log_file_name_set = 1;
      Tcl_DStringFree(&temp);
    }
#endif

    /*
     * Bidouillage pour contourner l'initialisation de la colormap....
     */

    noColorMapFlag = 0;
    if (argc > 1) {
	length = strlen(argv[1]);
	if ((length >= 2) && (strncmp(argv[1], "-nocolormap", length) == 0)) {
	  noColorMapFlag = 1;
	  argc--;
	  argv++;
	}
    }


    /*
     * Bidouillage, encore....
     */

    noDisplayFlag = 0;
    if (argc > 1) {
	length = strlen(argv[1]);
	if ((length >= 2) && (strncmp(argv[1], "-nodisplay", length) == 0)) {
	  noDisplayFlag = 1;
	  noColorMapFlag = 1;
	  argc--;
	  argv++;
	}
    }
    //noDisplayFlag = 1;
    //noColorMapFlag = 1;



    /*
     * Parse command-line arguments.  A leading "-file" argument is
     * ignored (a historical relic from the distant past).  If the
     * next argument doesn't start with a "-" then strip it off and
     * use it as the name of a script file to process.
     */

    fileName = NULL;
    if (argc > 1) {
	length = strlen(argv[1]);
	if ((length >= 2) && (strncmp(argv[1], "-file", length) == 0)) {
	    argc--;
	    argv++;
	}
    }
    if ((argc > 1) && (argv[1][0] != '-')) {
	fileName = argv[1];
	argc--;
	argv++;
    }

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".
     */

    args = Tcl_Merge(argc-1, (CONST char **) argv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv[0],
	    TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    /*
     * For now, under Windows, we assume we are not running as a console mode
     * app, so we need to use the GUI console.  In order to enable this, we
     * always claim to be running on a tty.  This probably isn't the right
     * way to do it.
     */

#ifdef __WIN32__
    tty = 1;
#else
    if (!noDisplayFlag) {
      tty = 0;
    } else {
      tty = isatty(0);
    }
#endif
    Tcl_SetVar(interp, "tcl_interactive",
	    ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     * This is where Tcl_AppInit defined in smurf_init.c
     * is executed !!!
     */

    if ((*appInitProc)(interp) != TCL_OK) {
	errChannel = Tcl_GetStdChannel(TCL_STDERR);
	if (errChannel) {
            Tcl_Write(errChannel,
		    "application-specific initialization failed: ", -1);
            Tcl_Write(errChannel, interp->result, -1);
            Tcl_Write(errChannel, "\n", 1);
        }
    }

    /*
     * Evaluate the .rc file, if one has been specified.
     */

    Tcl_SourceRCFile(interp);
    Tcl_EvalFile(interp, SM_SRC_DIR_STR"/main/int_startup.tcl");

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
	code = Tcl_EvalFile(interp, fileName);
	if (code != TCL_OK) {
	    goto error;
	}
	tty = 0;
    } else {

	/*
	 * Establish a channel handler for stdin.
	 */

      if (noDisplayFlag) {
	inChannel = Tcl_GetStdChannel(TCL_STDIN);

	if (inChannel) {
#ifdef USE_READLINE
	  Tcl_CreateChannelHandler(inChannel, TCL_READABLE, Tk_DoNothing,
				   (ClientData) inChannel);
#else   
	  Tcl_CreateChannelHandler(inChannel, TCL_READABLE, StdinProc,
				   (ClientData) inChannel);
#endif
	}
#ifndef USE_READLINE
	  if (tty) 
	    Prompt(interp, 0);
#endif
      }
    }
/*
	inChannel = Tcl_GetStdChannel(TCL_STDIN);
	if (inChannel) {
	    Tcl_CreateChannelHandler(inChannel, TCL_READABLE, StdinProc,
		    (ClientData) inChannel);
	}
	if (tty) {
	    Prompt(interp, 0);
	}
*/

    if (noDisplayFlag) {
      outChannel = Tcl_GetStdChannel(TCL_STDOUT);
      if (outChannel) {
	Tcl_Flush(outChannel);
      }
    }
    Tcl_DStringInit(&command);
    Tcl_DStringInit(&line);
    Tcl_ResetResult(interp);

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

#ifdef USE_READLINE
    if (noDisplayFlag) {
      if (tty){
	Tk_DoOneEvent (TK_ALL_EVENTS | TK_DONT_WAIT);
	rl_event_hook = Tk_rl_DoOneEvent;
	while (1){
	prompt:
	  input = readline (gotPartial ? "|" : "]");
	  if (input){
	    count = strlen (input);
	    add_history (input);
	    cmd = Tcl_DStringAppend (&command, input, count);
	    if (!Tcl_CommandComplete (cmd)){
	      gotPartial = 1;
	      goto prompt;
	    }
	    gotPartial = 0;
	    code = Tcl_RecordAndEval (interp, cmd, 0);
	    Tcl_DStringFree (&command);
	    if (*interp->result != 0) {
	      if ((code != TCL_OK) || (tty)){
		printf ("%s\n", interp->result);
	      }
	    }
	  } else {
	    if (!gotPartial){
	      if (tty){
		Tcl_Eval (interp, "exit");
		exit (1);
	      } else {
		Tcl_DeleteChannelHandler (inChannel, Tk_DoNothing,
					  (ClientData) inChannel);
	      }
	      return;
	    } 
	  }
	}
      }
    }
#endif

    Tk_MainLoop();
    Tcl_DeleteInterp(interp);
    Tcl_Exit(0);

error:
    /*
     * The following statement guarantees that the errorInfo
     * variable is set properly.
     */

    Tcl_AddErrorInfo(interp, "");
    errChannel = Tcl_GetStdChannel(TCL_STDERR);
    if (errChannel) {
        Tcl_Write(errChannel, Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY),
		-1);
        Tcl_Write(errChannel, "\n", 1);
    }
    Tcl_DeleteInterp(interp);
    Tcl_Exit(1);
}


int main(int argc,char *argv[])
     /*int argc;*/			/* Number of command-line arguments. */
     /*char **argv;*/		/* Values of command-line arguments. */
{
  /* !!!!! WARNING !!!!! */
  /* Tk_Main is now redefined by
     #define Tk_Main(argc, argv, proc) \
    Tk_MainEx(argc, argv, proc, Tcl_CreateInterp())
  */

#ifdef FFTW_THREADS
  {
    int status=my_fftw_init_threads();
    if (status == 0) {
      printf("got an error when doing fftw_init_threads (returned %d)!\n",status);
      exit(EXIT_FAILURE);
    }
  }
#endif

  myTk_MainEx(argc, argv, Tcl_AppInit, Tcl_CreateInterp());
  return 0;			/* Needed only to prevent compiler warning. */
}


/*
 *----------------------------------------------------------------------
 *
 * StdinProc --
 *
 *	This procedure is invoked by the event dispatcher whenever
 *	standard input becomes readable.  It grabs the next line of
 *	input characters, adds them to a command being assembled, and
 *	executes the command if it's complete.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Could be almost arbitrary, depending on the command that's
 *	typed.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
static void
StdinProc(ClientData clientData,int mask)
     /*ClientData clientData;*/		/* Not used. */
     /*int mask;*/			/* Not used. */
{
    static int gotPartial = 0;
    char *cmd;
    int code, count;
    Tcl_Channel chan = (Tcl_Channel) clientData;

    count = Tcl_Gets(chan, &line);

    if (count < 0) {
	if (!gotPartial) {
	    if (tty) {
		Tcl_Exit(0);
	    } else {
		Tcl_DeleteChannelHandler(chan, StdinProc, (ClientData) chan);
	    }
	    return;
	} else {
	    count = 0;
	}
    }

    (void) Tcl_DStringAppend(&command, Tcl_DStringValue(&line), -1);
    cmd = Tcl_DStringAppend(&command, "\n", -1);
    Tcl_DStringFree(&line);
    
    if (!Tcl_CommandComplete(cmd)) {
        gotPartial = 1;
        goto prompt;
    }
    gotPartial = 0;

    /*
     * Disable the stdin channel handler while evaluating the command;
     * otherwise if the command re-enters the event loop we might
     * process commands from stdin before the current command is
     * finished.  Among other things, this will trash the text of the
     * command being evaluated.
     */

    Tcl_CreateChannelHandler(chan, 0, StdinProc, (ClientData) chan);
    code = Tcl_RecordAndEval(interp, cmd, TCL_EVAL_GLOBAL);
    Tcl_CreateChannelHandler(chan, TCL_READABLE, StdinProc,
	    (ClientData) chan);
    Tcl_DStringFree(&command);
    if (*interp->result != 0) {
	if ((code != TCL_OK) || (tty)) {
	    /*
	     * The statement below used to call "printf", but that resulted
	     * in core dumps under Solaris 2.3 if the result was very long.
             *
             * NOTE: This probably will not work under Windows either.
	     */

	    puts(interp->result);
	}
    }

    /*
     * Output a prompt.
     */

    prompt:
    if (tty) {
	Prompt(interp, gotPartial);
    }
    Tcl_ResetResult(interp);
}

/*
 *----------------------------------------------------------------------
 *
 * Prompt --
 *
 *	Issue a prompt on standard output, or invoke a script
 *	to issue the prompt.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A prompt gets output, and a Tcl script may be evaluated
 *	in interp.
 *
 *----------------------------------------------------------------------
 */

static void
Prompt(Tcl_Interp *interp,int partial)
     /*Tcl_Interp *interp;*/   		/* Interpreter to use for prompting. */
     /*int partial;*/			/* Non-zero means there already
					 * exists a partial command, so use
					 * the secondary prompt. */
{
    CONST char *promptCmd;
    int code;
    Tcl_Channel outChannel, errChannel;

    errChannel = Tcl_GetChannel(interp, "stderr", NULL);

    promptCmd = Tcl_GetVar(interp, partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
    
    if (promptCmd == NULL) {
defaultPrompt:
      if (!partial) {

	/*
	 * We must check that outChannel is a real channel - it
	 * is possible that someone has transferred stdout out of
	 * this interpreter with "interp transfer".
	 */
	
	outChannel = Tcl_GetChannel(interp, "stdout", NULL);
	if (outChannel != (Tcl_Channel) NULL) {
	  Tcl_Write(outChannel, "% ", 2);
	}
      }
    } else {
      code = Tcl_Eval(interp, promptCmd);
      if (code != TCL_OK) {
	Tcl_AddErrorInfo(interp,
			 "\n    (script that generates prompt)");
	/*
	 * We must check that errChannel is a real channel - it
	 * is possible that someone has transferred stderr out of
	 * this interpreter with "interp transfer".
	 */
	
	errChannel = Tcl_GetChannel(interp, "stderr", NULL);
	if (errChannel != (Tcl_Channel) NULL) {
	  Tcl_Write(errChannel, interp->result, -1);
	  Tcl_Write(errChannel, "\n", 1);
	}
	goto defaultPrompt;
      }
    }
    outChannel = Tcl_GetChannel(interp, "stdout", NULL);
    if (outChannel != (Tcl_Channel) NULL) {
      Tcl_Flush(outChannel);
    }
}



#ifdef USE_READLINE

/*
 *----------------------------------------------------------------------
 *
 * Tk_rl_DoOneEvent:
 *
 *	This routine is called from readline in the idle time.
 *      Calls Tk's Tk_DoOneEvent.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A prompt gets output, and a Tcl script may be evaluated
 *	in interp.
 *
 *----------------------------------------------------------------------
 */
static int
Tk_rl_DoOneEvent ()
{
  Tk_DoOneEvent (0);

  return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * Tk_DoNothing
 *
 *	This functions is called when the standard input becomes readable.
 *      After this, Tk_DoOneEvent will return and control gets back to
 *      readline, it read the data and calls Tk_rl_DoOneEvent again.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
static void
Tk_DoNothing ( ClientData clientData,int mask)
     /*ClientData clientData;*/	        /* Not used. */
     /*int mask;*/			/* Not used. */
{
}

/*
 * xalloc, xrealloc --
 *
 *      GNU's readline requieres externally defined xalloc and
 *      xrealloc that handle errors.
 */
/* char * */
/* xmalloc (int bytes) */
/*      /\*int bytes;*\/ */
/* { */
/*     return (char *) malloc (bytes); */
/* } */

/* char * */
/* xrealloc (char *pointer, int bytes) */
/*      /\*char *pointer; */
/*        int bytes;*\/ */
/* { */
/*   if (!pointer) { */
/*     return (char *) malloc (bytes); */
/*   } else { */
/*     return (char *) realloc (pointer, bytes); */
/*   } */
/* } */

#endif

