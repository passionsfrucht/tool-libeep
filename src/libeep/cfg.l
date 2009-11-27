%option yylineno
%option noyywrap
%{

/*
  ------------------------------------------------------------------------
  EEP - ERP Evaluation Package
  
  generalized configuration lexer

  Max-Planck-Institute of Cognitive Neuroscience Leipzig
  R. Nowagk 1996

  (Id: cfg.l,v 1.4 1999/02/26 13:49:22 nowagk Exp nowagk) 


  ------------------------------------------------------------------------
  $Id: cfg.l 1762 2008-03-14 13:15:30Z rsmies $
  ------------------------------------------------------------------------
*/

#include <eep/cfg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

char RCS_cfg_h[] = RCS_CFG_H;
char RCS_cfg_l[] = "$RCSfile: cfg.l,v $ $Revision: 1762 $";

void cfg_sect(cfg_t *cfg, char *s, int lineno)
{
  int i, j;
  char *buf = (char *) malloc(strlen(s) + 1);
  int c = cfg->sectionc;
  
  cfg->sectionv = (cfgsection_t *) 
    realloc(cfg->sectionv, (c + 1) * sizeof(cfgsection_t));
  
  /* remove unwanted but matched chars */ 
  for (i = 0, j = 0; i < strlen(s); i++) {
    if (!isspace(s[i]) && s[i] != '[' && s[i] != ']') {
      buf[j] = s[i]; j++;
    }
    
    if (j > 0 && isspace(s[i]) && !isspace(buf[j - 1])) {
      buf[j] = ' '; j++;
    }
  }
  if (buf[j - 1] == ' ') j--;
  buf[j] = '\0';
  
  cfg->sectionv[c].line = lineno;
  cfg->sectionv[c].itemc = 0;
  cfg->sectionv[c].itemv = NULL;
  cfg->sectionv[c].section = (char *) malloc(strlen(buf) + 1);
  strcpy(cfg->sectionv[c].section, buf);
  free(buf);

  (cfg->sectionc)++;
}

void cfg_item_open(cfg_t *cfg, char *s, int lineno)
{
  cfgsection_t *sect = &(cfg->sectionv[cfg->sectionc - 1]);
  int c = sect->itemc;
  int n = strlen(s);

  if (c)
    sect->itemv = (cfgitem_t *) realloc(sect->itemv, (c + 1) * sizeof(cfgitem_t));
  else
    sect->itemv = (cfgitem_t *) malloc(sizeof(cfgitem_t));
  sect->itemv[c].line = lineno;
  sect->itemv[c].item = NULL;
  
  while (n > cfg->total) {
    cfg->total += 80;
    cfg->buf = (char *) realloc(cfg->buf, cfg->total);
  }
  strcpy(cfg->buf, s);
  cfg->used = n + 1;
}

void cfg_item_append(cfg_t *cfg, char *s)
{
  int n = strlen(s);
  while (cfg->used + n > cfg->total) {
    cfg->total += 80;
    cfg->buf = (char *) realloc(cfg->buf, cfg->total);
  }
  strcat(cfg->buf, s);
  cfg->used += n;
}

void cfg_item_close(cfg_t *cfg)
{
  cfgsection_t *sect = &cfg->sectionv[cfg->sectionc - 1];
  int c = sect->itemc;
  int pos;
  
  /* remove heading/trailing whitespaces from item */
  pos = strlen(cfg->buf) - 1;
  while (isspace(cfg->buf[pos])) {cfg->buf[pos] = '\0'; pos--;}
  pos = 0;
  while (isspace(cfg->buf[pos])) pos++;
  
  /* register it */
  sect->itemv[c].item = malloc(strlen(cfg->buf + pos) + 1);
  strcpy(sect->itemv[c].item, cfg->buf + pos);
  (cfg->sectionv[cfg->sectionc - 1].itemc)++;
}

#define YY_DECL int cfg_lex(cfg_t *cfg)
/* ^~'` */
%}

%x SECTION
%x ITEM
%x LITERAL
%x COMMENT

%%

\; {
  BEGIN(COMMENT);
}
\[[ \t]* {
  BEGIN(SECTION);
}
[ ,\r\n\t]+
[^,;"\n\[\]]+ {
  if (!cfg->sectionc)
    return yylineno;
  cfg_item_open(cfg, yytext, yylineno);
  BEGIN(ITEM);
}

<COMMENT>.*\n {
  BEGIN(INITIAL);
}

<SECTION>{
  [a-zA-Z_0-9][ \ta-zA-Z_0-9]+ {
    cfg_sect(cfg, yytext, yylineno);
  }
  [\]][ \t\n\r]* {
    BEGIN(INITIAL);
  }
  . {
    return yylineno;
  }
}

<ITEM>{
  [,\n]+ {
    cfg_item_close(cfg);
    BEGIN(INITIAL);
  }
  \" {
    cfg_item_append(cfg, yytext);
    BEGIN(LITERAL);
  }
  \; { 
    cfg_item_close(cfg);
    BEGIN(COMMENT);
  }
  <<EOF>> {
    cfg_item_close(cfg);
  }
  [^,;"\n\[\]]+ {
    cfg_item_append(cfg, yytext);
  }
}

<LITERAL>{
  [^"\n] {
    cfg_item_append(cfg, yytext);
  }
  [<<EOF>>\n]  {
    return yylineno;
  }
  \" {
    cfg_item_append(cfg, yytext);
    BEGIN(ITEM);
  }
}

%%

int cfg_init(cfg_t *cfg, FILE *f)
{
  int r;
  /* yyin = f; */
  struct stat statbuf;
  
  
  if (    fstat(fileno(f), &statbuf)
       || S_ISDIR(statbuf.st_mode & S_IFMT) )
    return 1;
  
  yyrestart(f);
  cfg->sectionc = 0;
  cfg->sectionv = NULL;
  cfg->buf = NULL;
  cfg->total = 0;
  cfg->used = 0;
  
  r = cfg_lex(cfg);
  if (cfg->buf) free(cfg->buf);
  
  return r;
}

int cfg_init_buf(cfg_t *cfg, char *buf)
{
  int r;
  YY_BUFFER_STATE b;
  
  b = yy_scan_string(buf);
  cfg->sectionc = 0;
  cfg->sectionv = NULL;
  cfg->buf = NULL;
  cfg->total = 0;
  cfg->used = 0;
  
  r = cfg_lex(cfg);
  if (cfg->buf) free(cfg->buf);
  yy_delete_buffer(b);
  return r;
}  

void cfg_free(cfg_t *cfg)
{
  int s, i;
  
  
  for (s = 0; s < cfg->sectionc; s++) {
    for ( i = 0; i < cfg->sectionv[s].itemc; i++) {
      free(cfg->sectionv[s].itemv[i].item);
    }
    free(cfg->sectionv[s].itemv);
  }
  if (s) free(cfg->sectionv);
}

short get_cfg_sectionc (cfg_t *cfg)
{
  return cfg->sectionc;
}

char *get_cfg_section  (cfg_t *cfg, short section, short *line)
{
  if (section < cfg->sectionc) {
    *line = cfg->sectionv[section].line;
    return cfg->sectionv[section].section;
  }
  else {
    *line = 0;
    return NULL;
  }
}

short get_cfg_itemc(cfg_t *cfg, const char *section)
{
  short s;
  
  for (s = 0; s < cfg->sectionc; s++) {
    if (!strcasecmp(section, cfg->sectionv[s].section))
      return cfg->sectionv[s].itemc;
  }
  
  return 0;
}

char *get_cfg_item(cfg_t *cfg, const char *section, short item, short *line)
{
  short s;
  
  for (s = 0; s < cfg->sectionc; s++) {
    if (!strcasecmp(section, cfg->sectionv[s].section)) {
      *line = cfg->sectionv[s].itemv[item].line;
      return cfg->sectionv[s].itemv[item].item;
    }
  }
  
  return NULL;
}

void cfg_write(cfg_t *cfg, FILE *f)
{
  short s, i;
  
  for (s = 0; s < cfg->sectionc; s++) {
    fprintf(f, "sect#%02d[%s]\n", s, cfg->sectionv[s].section);
    for (i = 0; i < cfg->sectionv[s].itemc; i++) {
      fprintf(f, "item#%02d>%s<\n", i, cfg->sectionv[s].itemv[i].item);
    }
  }
}
