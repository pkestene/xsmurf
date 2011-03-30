
#include "tclInt.h"
#include "tclPort.h"

/*
 *-----------------------------------------------------------------
 *
 * Tcl_ReEval --
 *
 *	Execute a command in the Tcl language.
 *	Derived from Tcl_Eval(), to suit obTcl's needs.
 *	Something like this should exist in Tcl really. 
 *	Stupid name..
 *
 * Results:
 *	The return value is one of the return codes defined in tcl.hd
 *	(such as TCL_OK), and interp->result contains a string value
 *	to supplement the return code.  The value of interp->result
 *	will persist only until the next call to Tcl_Eval:  copy it or
 *	lose it! *TermPtr is filled in with the character just after
 *	the last one that was part of the command (usually a NULL
 *	character or a closing bracket).
 *
 * Side effects:
 *	Almost certainly;  depends on the command.
 *
 *-----------------------------------------------------------------
 */

int
Tcl_ReEval(clientData, interp, argc, argv)
    ClientData clientData;	/* iclass name for this object */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    int result;				/* Return value. */
    register Interp *iPtr = (Interp *) interp;
    Tcl_HashEntry *hPtr;
    Command *cmdPtr;
    register Trace *tracePtr;
    int oldCount = iPtr->cmdCount;	/* Used to tell whether any commands
					 * at all were executed. */

    /*
     * Initialize the result to an empty string and clear out any
     * error information.  This makes sure that we return an empty
     * result if there are no commands in the command string.
     */

    Tcl_FreeResult((Tcl_Interp *) iPtr);
    iPtr->result = iPtr->resultSpace;
    iPtr->resultSpace[0] = 0;
    result = TCL_OK;

    /*
     * Check depth of nested calls to Tcl_Eval:  if this gets too large,
     * it's probably because of an infinite loop somewhere.
     */

    iPtr->numLevels++;
    if (iPtr->numLevels > iPtr->maxNestingDepth) {
	iPtr->numLevels--;
	iPtr->result =  "too many nested calls to Tcl_Eval (infinite loop?)";
	return TCL_ERROR;
    }

/*----------------------------------------------------------------------*/

	/*
	 * Find the procedure to execute this command.  If there isn't
	 * one, then see if there is a command "unknown".  If so,
	 * invoke it instead, passing it the words of the original
	 * command as arguments.
	 */

	hPtr = Tcl_FindHashEntry(&iPtr->commandTable, argv[0]);
	if (hPtr == NULL) {
	    int i;

	    hPtr = Tcl_FindHashEntry(&iPtr->commandTable, "unknown");
	    if (hPtr == NULL) {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "invalid command name \"",
			argv[0], "\"", (char *) NULL);
		result = TCL_ERROR;
		goto done;
	    }
	    for (i = argc; i >= 0; i--) {
		argv[i+1] = argv[i];
	    }
	    argv[0] = "unknown";
	    argc++;
	}
	cmdPtr = (Command *) Tcl_GetHashValue(hPtr);


	/*
	 * At long last, invoke the command procedure.  Reset the
	 * result to its default empty value first (it could have
	 * gotten changed by earlier commands in the same command
	 * string).
	 */

	iPtr->cmdCount++;
	Tcl_FreeResult(iPtr);
	iPtr->result = iPtr->resultSpace;
	iPtr->resultSpace[0] = 0;
	result = (*cmdPtr->proc)(cmdPtr->clientData, interp, argc, argv);
	if (Tcl_AsyncReady()) {
	    result = Tcl_AsyncInvoke(interp, result);
	}

/*----------------------------------------------------------------------*/

    iPtr->numLevels--;
done:
    return result;
}


/*
 * ObTcl_dispatch mimics the varframe changes obtained from calling
 * a normal Tcl-proc, before calling the object command.  This is
 * so objects commands will behave as close as possible to
 * object procs.
 * BUGS:
 *	1. argv[1] is modified in the current frame, so it won't be
 *	   the name of the object, as it should be.
 *	2. Clumsy implementation..  Better Tcl-support for these kind
 *	   of things would be nice.
 */
int
ObTcl_dispatch(clientData, interp, argc, argv)
    ClientData clientData;	/* iclass name for this object */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    int i;
    char *self; char buf[200];
    register Interp *iPtr = (Interp *) interp;
    int result;
    CallFrame frame;

    Tcl_InitHashTable(&frame.varTable, TCL_STRING_KEYS);
    if (iPtr->varFramePtr != NULL) {
	frame.level = iPtr->varFramePtr->level + 1;
    } else {
	frame.level = 1;
    }
    frame.argc = argc;
    frame.argv = argv;
    frame.callerPtr = iPtr->framePtr;
    frame.callerVarPtr = iPtr->varFramePtr;
    iPtr->framePtr = &frame;
    iPtr->varFramePtr = &frame;
    iPtr->returnCode = TCL_OK;

    if ( argc <  2 ) {
	char s[200];
	sprintf(s, "no value given for parameter \"cmd\" to \"%s\"", argv[0]);
	Tcl_AddErrorInfo(interp, s);
	return TCL_ERROR;
    }

    /*
     * Check depth of nested calls to Tcl_Eval:  if this gets too large,
     * it's probably because of an infinite loop somewhere.
     */

    iPtr->numLevels++;
    if (iPtr->numLevels > iPtr->maxNestingDepth) {
	iPtr->numLevels--;
	iPtr->result =  "too many nested calls to Tcl_Eval (infinite loop?)";
	return TCL_ERROR;
    }
    Tcl_SetVar(interp, "self", argv[0], 0);
    Tcl_SetVar(interp, "iclass", (char *)clientData, 0);
    strcpy(buf, clientData);
    strcat(buf, "::"); strcat(buf, argv[1]);

    argv[1] = buf;
    result = Tcl_ReEval(clientData, interp, argc-1, &argv[1]);

    iPtr->numLevels--;

    iPtr->framePtr = frame.callerPtr;
    iPtr->varFramePtr = frame.callerVarPtr;
    if (iPtr->flags & ERR_IN_PROGRESS) {
	TclDeleteVars(iPtr, &frame.varTable);
	iPtr->flags |= ERR_IN_PROGRESS;
    } else {
	TclDeleteVars(iPtr, &frame.varTable);
    }

	return result;
}

int
ObTcl_otProc(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
	char *p;

	if ( argc !=  3 ) {
		Tcl_AddErrorInfo(interp,
		    "\n  otProc: I require 2 arguments");
		fprintf(stderr, "argc != 3\n");
		return TCL_ERROR;
	}
	p = (char *) ckalloc((unsigned) strlen(argv[1])+1);
	strcpy(p, argv[1]);
	Tcl_CreateCommand(interp, argv[2], ObTcl_dispatch, (ClientData)p,
		(Tcl_CmdDeleteProc *)NULL);

	return TCL_OK;
}

int
ObTcl_otGetSelf(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
	Tcl_UpVar(interp, "1", "self", "self", 0);
	Tcl_UpVar(interp, "1", "iclass", "iclass", 0);
	Tcl_UpVar(interp, "1", "Umethod", "method", 0);

	return TCL_OK;
}

/*
 * The var format defs are to move to a common
 * definition-place for tcl and C code (ASAP).
 * Some other format defs may need the same treatment.
 */
#define PRE_INSTVAR	"_oIV_%s:%s:"
#define PRE_CLASSVAR	"_oDCV_%s:"
#define PRE_ICLASSVAR	"_oICV_%s:"

typedef struct {
	char *prefix;
	char *v1;
	char *v2;
} obtcl_vardata_s;

static obtcl_vardata_s obtcl_vardata[] = {
	{ PRE_INSTVAR,	"class", "self" },
	{ PRE_CLASSVAR,	"class", NULL },
	{ PRE_ICLASSVAR,"iclass", NULL },
};

int
ObTcl_Var(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
	register obtcl_vardata_s *vd = (obtcl_vardata_s *)clientData;

	char v1[100], v2[100], tmp[200], *p;
	int i;

	tmp[0] = '\0';
	p = Tcl_GetVar(interp, vd->v1, 0);
	if (!p) {
	    sprintf(tmp,
		"%s: No `%s' variable defined (%s used outside of method?)",
		argv[0], vd->v1, argv[0]);
	    Tcl_AddErrorInfo(interp, tmp);
	    return TCL_ERROR;
	}
	strcpy(v1, p);

	if (vd->v2) {
		p = Tcl_GetVar(interp, vd->v2, 0);
		if (!p) {
		    sprintf(tmp,
		    	"%s: No `%s' variable defined (%s used outside of method?)",
			argv[0], vd->v2, argv[0]);
		    Tcl_AddErrorInfo(interp, tmp);
		    return TCL_ERROR;
		}
		strcpy(v2, p);
	}

	sprintf(tmp, vd->prefix, v1, v2);
	p = &tmp[strlen(tmp)];

	for (i=1; i < argc; i++) {
		strcpy(p, argv[i]);
		Tcl_UpVar(interp, "#0", tmp, argv[i], 0);
	}
	return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * ObTcl_Init --
 *
 *	This procedure is invoked to initialized the obTcl commands.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Creates the new command and adds an entry into a global
 *	Tcl associative array.
 *
 *--------------------------------------------------------------
 */

int
Obtcl_Init(interp)
    Tcl_Interp *interp;
{
    int code;

    /*
     * Need to arrange independent version control etc. for
     * the binary extension. ASAP :-)
     */
    Tcl_SetVar(interp, "obtcl_version", OBTCL_VERSION,
	TCL_GLOBAL_ONLY);

    Tcl_CreateCommand(interp, "otProc", ObTcl_otProc, (ClientData)0,
	(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand(interp, "otGetSelf", ObTcl_otGetSelf, (ClientData)0,
	(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand(interp, "instvar", ObTcl_Var, (ClientData)&obtcl_vardata[0],
	(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand(interp, "classvar", ObTcl_Var, (ClientData)&obtcl_vardata[1],
	(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand(interp, "iclassvar", ObTcl_Var, (ClientData)&obtcl_vardata[2],
	(Tcl_CmdDeleteProc *)NULL);

    code = Tcl_PkgProvide(interp, "obtcl", OBTCL_VERSION);

    return code;
}
