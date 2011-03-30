/*
 * tcl_cruncher : A TCL pseudo compiler and/or syntax checker
 * Copyright 1994 by Laurent Demailly (dl@hplyot.obspm.fr)
 *
 * This program transform nice Tcl/TclX/Itcl sources into an ugly but
 * faster standard output : it removes all unneeded white spaces, ; ,
 * all comments, etc... the result is unreadable, but can be twice as
 * fast & as small than original.
 * It can also be used to check syntax of your programs.
 * It does all this very fast.
 *
 * Usage: tcl_cruncher [-t] [-1] [-c] [-s] [-d more-definitions-file] filenames
 *                     or '-' for standard output.
 *
 * using -t shows the all the commands of the script, in a 'calling tree' form
 *          on stderr (use it like in tcl_cruncher -tcs  ... |& more for best
 *          result) it can help to detect problems. Experimental feature.
 *
 * using -1 keeps first comments (ie #! /usr/local/bin/wish -f and (c)...)
 *
 * using -s disable brace in comments & switch warnings and 
 *          processed line count output
 *
 * using -c performs check only (no stdoutput)
 *
 * using -d option provides for adding new recognized commands without
 *   recompiling, ie you have your own command 'cmd' that have 1 argument
 *   which is a script, like the itcl 'destructor' command, just put in a
 *   text file the line : "cmd destructor" and cmd will be analyzed !
 *
 * Example: "tcl_cruncher *.tcl > lib.tclc" 'compiles' everything into lib.tclc
 *          "tcl_cruncher -cs *.tcl" makes a good syntax checker
 *
 * to use it from inside tcl, re-define for instance :
 *   proc source file {uplevel #0 [exec tcl_cruncher -s $file]}
 * and all your sourced files will then be auto-magically speeded up
 *
 * Warnings:
 *        + switch ?o? v {p1 {s1} p2 {s2}} construct is not optimized
 *          you should do it by hand for full speed, or use the
 *          switch ?o? v p1 {s1} p2 {s2} ... construct...
 *        + "if ... elseif ..." can be misinterpreted in tree mode. (-t)
 *
 * It compiles with gcc -ansi -pedantic -Wall -O without any problem
 * (tested on hpux : add -D_HPUX_SOURCE and linux) if you don't have
 * an ansi compiler, grab gcc, or ansi2kr, or remove prototypes & const...
 *
 * See also Makefile, README & tcl_cruncher man page.
 *
 * $Id: tcl_cruncher.c,v 1.1 1999/05/06 15:12:46 decoster Exp $
 *
 * Free software - 'Artistic' license (see file LICENSE), summary :
 *
 * You can use, copy modify & distribute freely as long as credit to
 * original author is maintained, source is provided & changes you make
 * to the original are clearly stated.
 *
 * This software is provided "as is" without express or implied warranty.
 *
 * latest version is always on ftp hplyot.obspm.fr:/tcl/tcl_cruncher*
 * please send bugs/patches/suggestions/love letters... to dl@hplyot.obspm.fr
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef __linux__
#include <getopt.h>
#endif

static const char *filename;  /* current file name worked upon */
static FILE *fic;             /* current input stream */


static void work_on(void);
static void build_tree(void);

static char buf[1024];

typedef void *ptr;
static ptr *current_tree;
static ptr *root_tree=NULL;
typedef int (*tp_fctR)(int argc);

typedef struct rec {
  const char *cmdname;
  tp_fctR    fct;
} ts_rec,*tps_rec;

/*
 * functions that check 'syntax' for tcl commands,
 * they tell the caller which arguments are script to
 * be processed recursively
 * (note ?options? can makes life harder, if they use {} ...
 */
static int pRec1(int argc) {return (argc==1);}
static int pRec2p(int argc) {return (argc>=2);}
static int pRecAll(int argc) {return 1;}
/*
static int pRec3p(int argc) {return (argc>=3);}
*/
static int pRec2(int argc) {return (argc==2);}
static int pRec3(int argc) {return (argc==3);}
static int pRec4(int argc) {return (argc==4);}
static int pRec4p(int argc) {return (argc>=4);}
static int pRecSwitch(int argc);
     
ts_rec syntax_tab[]= 
{
/* basic tcl */
  {"proc", pRec3},     /* proc    has arg no3 which is a script */
  {"while", pRec2},    /* while   has arg no2 which is a script*/
  {"if", pRec2p},      /* if      has arg no2 and after which are scripts */
  {"foreach", pRec3},  /* foreach has arg no3 which is a script*/
  {"for", pRec4},      /* for     has arg no4 which is a script*/
  {"switch",pRecSwitch},/* switch construct with switch ?o? v {p1 {s1} p2 {s2}}
                           is not supported :/ */
/* tclX */
  {"loop",pRec4p},     /* loop    has arg no4 and after ('cause of opt) */
/* itcl */
  {"itcl_class",pRec2},/* itcl_class : arg 2 */
  {"method",pRec3},    /* method     : arg 3 like proc */
  {"constructor",pRec2},/*constructor: arg 2 */
  {"destructor",pRec1},/* destructor : arg 1 */
/* might be usefull for -d */
  {"_recParseAll_",pRecAll}, /* reparse everything */
  {NULL,NULL}
};

void add_to_tree(const unsigned char *name,tp_fctR fct);

static int silent=0;  /* output swicth warnings and line count ? */
static int nostdout=0;/* output compiled stream ? */
static int keepfcmt=0;/* keep first comments ? */
static int showtree=0;/* show tree of commands ? */
static int spf=0;/* special kludge for "[cmd] args" pretty tree print */
/*
 *
 *  Main : parse arguments and call 'work_on' for each file or stdin
 *         also add alias/new commands if -d switch is provided.
 */

int main(argc,argv)
     int argc;
     char **argv;
{
  int c,errflg=0;
  char *dfilename=NULL;

  extern char *optarg;
  extern int optind;
  while ((c = getopt(argc, argv, "1hcstd:")) != EOF)
    switch (c) {
    case 'h':
      errflg++;
      break;
    case '?':
      errflg++;
      break;
    case 'c':
      nostdout++;
      break;
    case '1':
      keepfcmt++;
      break;
    case 't':
      showtree++;
      break;
    case 'd':
      if (dfilename) {
        errflg++;
      } else {
        dfilename=optarg;
      }
      break;
    case 's':
      silent++;
      break;
    }
  if (errflg || optind>=argc) {
    fprintf(stderr, "usage: tcl_cruncher [-cs1t] [-d definition-file] file names or '-' for stdin\n   -c : checks only\n   -s : no warnings\n   -1 : keep first comments\n   -t : show script tree\n   -d : definition file with lines 'newcmd1 alias1'...\n          v1.11, 9/1994 by Laurent Demailly (dl@hplyot.obspm.fr)\n");
    exit (2);
  }
  build_tree();
  if (dfilename) 
    { /* process file : "new_command working_like_this_one\n..." */
      if (access(dfilename,R_OK)) {
        sprintf(buf,"Can't read %s",dfilename);
        perror(buf);
        exit(7);
      } else {
	char newcmd[80];
	char alias[80];
	fic=fopen(dfilename,"r");
	while (fic && fscanf(fic,"%79s %79s",newcmd,alias)==2) 
	  {
	    ts_rec *s;
	    int    found;
	    s=syntax_tab;
	    found=0;
	    /* find alias in syntax_tab :*/
	    while (!found && (s->cmdname!=NULL))
	      { 
		if (!strcmp(s->cmdname,alias)) {
		    /* found : add newcmd, with same parsing hook : */
		    add_to_tree((unsigned char *)newcmd,s->fct);
		    found=1;
		    break;
		  }
		s++;
	      }
            if (!found) {
	      fprintf(stderr,"alias for %s : %s not found !\n",newcmd,alias);
	    }
	  }
	fclose(fic);
      }
    }
  for ( ; optind < argc; optind++) {
    filename=argv[optind];
    if (!strcmp(filename,"-")) {
      filename="<stdin>";
      fic=stdin;
      work_on();
      return(0);
    } else {
      if (access(filename,R_OK)) {
        sprintf(buf,"Can't read %s",filename);
        perror(buf);
        exit(3);
      } else {
	fic=fopen(filename,"r");
	work_on();
	fclose(fic);
      }
    } 
  }
  return(0);
}
     

void *CallocMe(int nelem,int size) {
  void *ptr=calloc(nelem,size);
  if (!ptr) {perror("tcl_cruncher"); exit(6);}
  return ptr;
}
	     
  
void add_to_tree(name,fct) 
     const unsigned char *name;
     tp_fctR fct;
{
  ptr *tree=root_tree;
/*  fprintf(stderr,"adding %s : %x\n",name,fct); */
  while (*name) 
    {
      if (tree[*name]==NULL) {
	tree[*name]=CallocMe(256,sizeof(ptr));  
      }
      tree=tree[*name];
      name++;
    }
  tree[0]=(ptr *)fct;
} 

static void build_tree() {
  ts_rec *s=syntax_tab;
  
  root_tree=CallocMe(256,sizeof(ptr));
  while (s->cmdname!=NULL) 
    {
      add_to_tree((unsigned char *)s->cmdname,s->fct);
      s++;
    }
}
  
typedef int (*tp_fct)(int node);


typedef enum {
  BFRCMD, /* 0 */
  INCMD,  /* 1 */
  INCMT,  /* 2 */
  BFRWRD, /* 3 */
  INWRD,  /* 4 */
  INDQT,  /* 5 */
  INBRACE,/* 6 */
  AFTERBRACE,/* 7 */
  AFTERQUOTE,/* 8 */
  AFTERCMT,/* 9 */
  NBR_STATES
} e_st;


static int pBfrCmd(int c);
static int pInCmd(int c);
static int pInCmt(int c);
static int pBfrWrd(int c);
static int pInDQt(int c);
static int pInBrace(int c);
static int pAfterBrace(int c);
static int pAfterQuote(int c);


static tp_fct tabfs[NBR_STATES]={
  pBfrCmd,
  pInCmd,
  pInCmt,
  pBfrWrd,
  pInCmd,
  pInDQt,
  pInBrace,
  pAfterBrace,
  pAfterQuote,
  pBfrCmd
};

/*
 * state variables used :
 */
static int line;	/* Current Line number (first=1) */
static int pos;		/* Current character position in line (first=1) */
static e_st state;	/* Current state */
static int backslash;	/* Previous char is a backslash ? */
static int brace;	/* number of nested braces */
static int end;		/* what car marks then end of current analysis */
static int level;	/* current recursive level */
static int argc;	/* indice of argument in current cmd */
static int first;	/* are we before the first cmd (then no \n)*/
static int dollar;	/* prev is a $ (1) or proces an ${xxx} construct (2) */
static tp_fctR recurse; /* function called to determine if arg have to be reparsed */

static void error(msg) 
    char *msg;
{
  fflush(stdout);
  fprintf(stderr,"\n%s:%d: Error: %s (p %d)\n",filename,line,msg,pos);
  exit(5);
}

static previouswarn=-1; /* previous warning line no (for 'flood' control)*/

static void warning(msg) 
    char *msg;
{
  /* try to limit warning flood, specially for comments */
  if (!silent && !(state==INCMT && previouswarn>=(line-1))) 
    {
      fflush(stdout);
      if (showtree) fputc('\n',stderr);
      fprintf(stderr,"%s:%d: Warning: %s (p %d)\n",filename,line,msg,pos);
    }
  previouswarn=line;
}

void analyze(int);
void outputc(int);

/* I don't like that AFTERCMT thing, but comment management in real Tcl */
/* is really weird... so these complications are needed... */

void check_end_cmt() {
  if (brace) warning("unmatched { in previous comment block");
}

static int pBfrCmd(c)
  int c;
{
  if (backslash) {if (state==AFTERCMT) check_end_cmt(); state=INCMD;outputc('\\');outputc(c);return 1;}
  if (c==end) {if (state==AFTERCMT) check_end_cmt();outputc(c);return 0;}
  switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case ';':
      return 1;
    case '#':
      if (state!=AFTERCMT) {brace=0;}
      state=INCMT;
      return pInCmt(c);
    case '"':
      if (state==AFTERCMT) check_end_cmt();
      if (!first) outputc('\n');
      outputc(c);
      state=INDQT;
      break;
    case '{':
      if (state==AFTERCMT) check_end_cmt();
      if (!first) outputc('\n');
      outputc(c);
      state=INBRACE;
      brace=1;
      break;
    case '[':
      if (state==AFTERCMT) check_end_cmt();
      if (!first) {
	outputc('\n');
	if (showtree) fputc('\n',stderr);
      }  
      spf=1;
      outputc(c);
      analyze(']');
      state=INCMD;
      break;
    default:
      if (state==AFTERCMT) check_end_cmt();
      if (!first) outputc('\n');
      state=INCMD;
      outputc(c);
  }
  first=0;
  return 1;
}

static int pInCmt(int c) {
  if (keepfcmt && first && level==1) {
      outputc(c);
    }
  if (!backslash) {
    switch (c) {
      case '{' :
        brace++;
        break;
      case '}' :
        if (brace) brace--; else {
          if (end=='}') {warning("using closing } from comment");outputc(c);return 0;}
          else warning("non backslashed unmatched } in comment");  
        }
        break;
      case '\n' :
        state=AFTERCMT;
        break;
      default :
        break;
    }
  }
  return 1;
} 

static int pInDQt(int c) {
  if (backslash) {outputc('\\');outputc(c);return 1;}
  if (c=='[') {outputc(c);analyze(']');return 1;}
  if (c=='"') state=AFTERQUOTE;
  outputc(c);
  return 1;
}

static int pInBrace(int c) {
  if (backslash) {outputc('\\');outputc(c);return 1;}
  if (c=='{') brace++; else 
  if (c=='}' && (!(--brace))) {
    if (dollar==2) {state=INWRD; dollar=0;} else state=AFTERBRACE;
  }
  outputc(c);
  return 1;
}

static void next_arg(void);

static void next_arg() 
{
  outputc(' ');
  argc++;
}

static void next_cmd(void);

static void next_cmd()
{
/*  outputc('\n'); */
  argc=0;
  current_tree=root_tree;
}  


static int pBfrWrd(int c) {
  if (backslash) {next_arg();state=INWRD;outputc('\\');outputc(c);return 1;}
  if (c==end) {outputc(c);return 0;}
  switch (c) {
    case ' ':
    case '\t':
      break;
    case '\n':
    case ';':
      next_cmd();
      state=BFRCMD;
      break;
    case '"':
      next_arg();
      outputc(c);
      state=INDQT;
      break;
    case '{':
      next_arg();
      outputc(c);
      if (recurse && ( (*recurse)(argc) ) ) {
	analyze('}');
	state=AFTERBRACE;
      } else {
	state=INBRACE;
	brace=1;
      }
      break;
    case '[':
      next_arg();
      outputc(c);
      analyze(']');
      current_tree=NULL;
      state=INWRD;
      break;
    default:
      next_arg();
      outputc(c);
      state=INWRD;
  }
  return 1;
} 

static int pAfterBrace(c)
  int c;
{
  if (backslash) error("extra characters after close-brace");
  if (c==end) {outputc(c);return 0;}
  switch (c) {
    case ' ':
    case '\t':
      state=BFRWRD;
      break;
    case '\n':
    case ';':
      next_cmd();
      state=BFRCMD;
      break ;
    default:
      error("extra characters after close-brace");
  }
  return 1;
}

static int pAfterQuote(c)
  int c;
{
  if (backslash) error("extra characters after close-quote");
  if (c==end) {outputc(c);return 0;}
  switch (c) {
    case ' ':
    case '\t':
      state=BFRWRD;
      break;
    case '\n':
    case ';':
      next_cmd();
      state=BFRCMD;
      break ;
    default:
      error("extra characters after close-quote");
  }
  return 1;
}

static int pInCmd(c)
  int c;
{
  if (backslash) {outputc('\\');outputc(c);return 1;}
  if (c==end) {state=BFRCMD;outputc(c);return 0;}
  switch (c) {
    case ' ':
    case '\t':
      if (state==INCMD) 
	{
	  if (current_tree) {current_tree=current_tree[0];}
	  recurse=(tp_fctR)(current_tree);
	}
      state=BFRWRD;
      break;
    case '\n':
    case ';':
      next_cmd();
      state=BFRCMD;
      break ;
    case '{': /* { in middle of a word : */
      outputc(c);
      if (dollar) { /* allow set toto xx${var}yy construct */
	state=INBRACE;
	brace=1;
	dollar=2;
      } else if (end=='}') {
       /* most probably an error...('cept at top level where it works) */
        error("{ inside a word and inside a construct : should be backslashed");
      }
      break;
    case '[':
      outputc(c);
      analyze(']');
      break;
    default:
      outputc(c);
  }
  return 1;
}

int get_nextc(void);

int get_nextc() {
  return fgetc(fic);
}

void outputc(c)
  int c;
{
  if (!nostdout) fputc(c,stdout);
/*  fprintf(stderr,"state=%d (%d), argc=%d, ct=%x , c='%c'\n",state,state==INCMD,argc,current_tree,c); */
  if (state==INCMD) 
    {
      if (showtree) 
	{
	  if (current_tree==root_tree && (!first || level>1)) {
	    if (end==']') {
	      if (first) {
		if (spf) {fputc('[',stderr); spf=0;}
		else fputs(" [",stderr);
	      }
	      else fputs("; ",stderr);
	    } else fprintf(stderr,"\n%*s",3*(level-1),"");
	  }
	  fputc(c,stderr);
	}
      if (current_tree) 
	{
	  if (c<=0 || c>255) {error("illegal caracter in source file");}
	  current_tree=current_tree[c];
	}
    }
}

/*
 *
 *  work_on : call analyze to do the real job on each stream
 *
 */
static void work_on()
{
  if (!fic) {
    fprintf(stderr,"Null file for %s !\n",filename);
    exit(4);
  }
  line=1;
  pos=0;
  state=BFRCMD;
  brace=backslash=0;
  level=0;
  end=EOF;
  previouswarn=-1;
  analyze(EOF);
  switch (state) 
    {
      case INDQT:
        error("missing \" !");
      case INBRACE:
        error("missing } !");
      default:
	break;
    }      
  /* the \n is **needed** */
  outputc('\n');
  if (showtree) fputc('\n',stderr);
  if (!silent) fprintf(stderr,"successfully processed %s: %d lines\n",filename,line); 
}

/* real job : */
void analyze(nend)
  int  nend;
{  
/* save previous state : */
  e_st ostate=state;
  int oend=end,obrace=brace,oargc=argc;
  int pline=line,ppos=pos;
  tp_fctR orecurse=recurse;

  int c,r=1,bn=0;
  argc=0;
  first=1;
  current_tree=root_tree;
  recurse=NULL;
  
  state=BFRCMD;brace=0;end=nend;
  level++;
  if (level>1000) error("too many nested call to analyze");
/*  fprintf(stderr,"level=%d,c='%c'\n",level,nend);*/
  while (r && ( (c=get_nextc())!=EOF ) ) {
#ifdef DEBUG
    fprintf(stderr,"l=%d,c=%d,level=%d,state=%d,dollar=%d,c='%c',bn=%d,bs=%d\n",line,pos,level,state,dollar,c,bn,backslash);
#endif
    if (c=='\n') {
      line++;
      pos=0;
      if (backslash) {bn=1;backslash=0;continue;}
    } else pos++;
    if (bn) {
      if (strchr(" \t\n",c)) {continue;} else {bn=0;(tabfs[state])(' ');}
    }
    if (!backslash && c=='\\') {backslash=1; continue;}
    r=(tabfs[state])(c);
    /* set dollar if previous is a dollar, or maintain, if state is in {} */
    if (dollar!=2) {dollar= ( !backslash && c=='$' );}
    backslash=0;
  }
  if (r && level>1) 
    {
      switch (state) 
	{
	case INDQT:
	  sprintf(buf,"missing \" (and %c [opened line %d, c%d]) !",
		  nend,pline,ppos);
	  error(buf);
	case INBRACE:
	  if (c=='}') {
	    sprintf(buf,"missing } [opened line %d, c%d] !",
		    pline,ppos);
	    error(buf);
          } else {
	    sprintf(buf,"missing } (and %c [opened line %d, c%d]) !",
		    nend,pline,ppos);
	    error(buf);
	  }
	default:
	  sprintf(buf,"missing %c [opened line %d, c%d] !",
		  nend,pline,ppos);
	  error(buf);
	}      
    }
/*  fprintf(stderr,"leaving level=%d,ostate=%d,oargc=%d\n",level,ostate,oargc);*/
  if (--level) 
    {
      if (showtree && end==']') fputc(']',stderr);
      /* restore previous state : */
      state=ostate;brace=obrace;end=oend;
      argc=oargc;
      recurse=orecurse;
      first=0;
    }
}


static int pRecSwitch(int argc) 
{
/* special case for switch, which sux 'cause of damn double parsing 
   ( switch var { str1 {script1} str2 {script2} } ) needed 
   dunno how to handle yet... just warn */
  if ( argc==2 /* || argc==3 */ )
    {
      warning("probable unsupported switch construct");
    }
  return (argc>2);
}
