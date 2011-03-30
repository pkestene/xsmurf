#include "wt2d_int.h"

#define INFINITE 10000000

#define YES 0
#define NO  1

/*
 * -Protect- 
 * 
 * Ensemble de fonctions permettant de marquer les liens up<->down auquels
 * on n'a pas le droit de toucher lors du lissage d'une echelle. Ces liens
 * sont le premier et le dernier de chaque chaine.
 */

typedef struct Protect {
  Extremum * extremum;
  struct Protect * next;
}Protect;

Protect * protect;

/*----------------------------------------------------------------------
  _initProtect_
  
  Libere le chainage 'protect', afin de pouvoir une nouvelle liste
  d'extrema a proteger.
  --------------------------------------------------------------------*/
void
_initProtect_()
{
  Protect *pro = protect, *tmp = protect;

  while(pro) {
    tmp = pro->next;
    free(pro);
    pro = tmp;
  }
}

/*----------------------------------------------------------------------
  _addProtectExt_
  
  Ajoute l'extremum passe en parametre a la liste d'extrema a proteger.
  --------------------------------------------------------------------*/
void
_addProtectExt_(Extremum *ext)
{
  Protect *pro, *tmp;

  pro = (Protect *) malloc (sizeof(Protect));
  if(!pro)
    exit(0);

  pro->extremum = ext;
  pro->next = NULL;
  tmp = protect;
  if(tmp){
    while(tmp->next)
      tmp = tmp->next;
    tmp->next = pro;
  }
  else
    protect = pro;
}

/*----------------------------------------------------------------------
  _isProtected_
  
  Renvoie 'YES' si l'extremum passe en parametre est protege, et 'NO'
  sinon.
  --------------------------------------------------------------------*/
int
_isProtected_(Extremum *ext)
{
  Protect *tmp = protect;

  while(tmp){
    if(tmp->extremum == ext)
      return YES;
    tmp = tmp->next;
  }
  return NO;
}

/*----------------------------------------------------------------------
  _protect_
  
  Parcours les chaines d'une extImage et protege le premier et le dernier
  extremum de chaque chaine.
  --------------------------------------------------------------------*/
void
_protect_(ExtImage * extImage)
{
  Extremum *ext, * ext_to_protect = NULL;
  int      i;

  for(i=0;i<extImage->chainNb;i++){

    /* on protege en haut */
    ext_to_protect = NULL;
    ext = extImage->chain[i];
    if (ext->up) {
      _addProtectExt_(ext);
      _addProtectExt_(ext->up);
    }
    else {
      do{
	ext = ext->next;
      }while(ext && (ext != extImage->chain[i]) && (!ext->up));
      if (ext->up) {
	_addProtectExt_(ext);
	_addProtectExt_(ext->up);
      }
    }
    while((ext = ext->next) && (ext != extImage->chain[i])){
      if(ext->up)
	ext_to_protect = ext;
    }
    if(ext_to_protect) {
      _addProtectExt_(ext_to_protect);
      _addProtectExt_(ext_to_protect->up);
    }

    /* on protege en bas */
    ext_to_protect = NULL;
    ext = extImage->chain[i];
    if (ext->down) {
      _addProtectExt_(ext);
      _addProtectExt_(ext->down);
    }
    else {
      do{
	ext = ext->next;
      }while(ext && (ext != extImage->chain[i]) && (!ext->down));
      if (ext->down) {
	_addProtectExt_(ext);
	_addProtectExt_(ext->down);
      }
    }
    while((ext = ext->next) && (ext != extImage->chain[i])){
      if(ext->down)
	ext_to_protect = ext;
    }
    if(ext_to_protect) {
      _addProtectExt_(ext_to_protect);
      _addProtectExt_(ext_to_protect->down);
    }
  }
}

/*----------------------------------------------------------------------
  _isNear_
  
  Regarde si les 2 arguments passes en parametre sont proches a 2 Pi pres
  et suivant la precision passee en parametre.
  --------------------------------------------------------------------*/
int
_isNear_(real arg1,
	 real arg2, 
	 real precision)
{
  if ((fabs(arg1-arg2) < precision) ||
      (fabs(fabs(arg1-arg2)-2*M_PI) < precision))
    return YES;
  return NO;
}

/*----------------------------------------------------------------------
  _lissage_
  
  Parcours de toutes les chaines de l'extimage passee en parametre avec
  elimination des extrema successif dont l'argument est proche.
  --------------------------------------------------------------------*/
void
_lissage_(ExtImage * downExtImage)
{
  Extremum * beginExt, * tmpExt, * ext;
  int      i;
  real    arg;

  for(i=0;i<downExtImage->chainNb;i++){
    beginExt  = downExtImage->chain[i];
    tmpExt   = downExtImage->chain[i];
    arg = beginExt->arg;
    while((tmpExt = tmpExt->next) && (tmpExt != downExtImage->chain[i])){
      if (_isNear_(tmpExt->arg, arg, 0.1) == YES){
	if((tmpExt->prev != beginExt) &&
	   (tmpExt->prev != downExtImage->chain[i]) &&
	   !(tmpExt->prev->down &&
	     (_isProtected_(tmpExt->prev->down) == YES)) &&
	   !(_isProtected_(tmpExt->prev) == YES)){
	  if(tmpExt->prev->down &&
	     tmpExt->prev->down->prev &&
	     tmpExt->prev->down->next) {
	    tmpExt->prev->down->up = NULL;
	    tmpExt->prev->down = NULL;
	  }
	  if(!(tmpExt->prev->up)){
	    if(tmpExt->prev->down)
	      if(tmpExt->prev->down->prev &&
		 tmpExt->prev->down->next)
		{
		  tmpExt->prev->prev->next = tmpExt;
		  tmpExt->prev->next = NULL;
		  ext = tmpExt->prev->prev;
		  tmpExt->prev->prev = NULL;
		  tmpExt->prev = ext;
		}
	      else;
	    else {
		  tmpExt->prev->prev->next = tmpExt;
		  tmpExt->prev->next = NULL;
		  ext = tmpExt->prev->prev;
		  tmpExt->prev->prev = NULL;
		  tmpExt->prev = ext;
		}
	  }
	}
      }
      else {
	beginExt = tmpExt->prev;
	arg = beginExt->arg;
      }
    }
  }
}
