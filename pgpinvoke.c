/*
 * Copyright (C) 1997-1998 Thomas Roessler <roessler@guug.de>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "mutt.h"
#include "pgp.h"


/*******************************************************************
 * 
 * PGP V2 Invocation stuff
 * 
 *******************************************************************/

pid_t pgp_v2_invoke_decode(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd,
			   const char *fname, int need_passphrase)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd), "%scat %s%s | "
	   "%s +language=%s +pubring=%s +secring=%s +verbose=0 +batchmode -f",
	   need_passphrase ? "PGPPASSFD=0; export PGPPASSFD; " : "",
	   need_passphrase ? "- " : "",
	   fname,
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring));
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


pid_t pgp_v2_invoke_verify(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd,
			   const char *signedstuff, const char *sigfile)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd), 
	   "%s +language=%s +pubring=%s +secring=%s +batchmode +verbose=0 %s %s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), sigfile, signedstuff);

  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


pid_t pgp_v2_invoke_decrypt(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd),
	   "PGPPASSFD=0; export PGPPASSFD; cat - %s | %s +language=%s +pubring=%s +secring=%s "
	   "+verbose=0 +batchmode -f",
	   fname, NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring));

  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			    pgpinfd, pgpoutfd, pgperrfd);
}




pid_t pgp_v2_invoke_sign(struct pgp_vinfo *pgp,
			 FILE **pgpin, FILE **pgpout, FILE **pgperr,
			 int pgpinfd, int pgpoutfd, int pgperrfd, 
			 const char *fname)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd),
	   "PGPPASSFD=0; export PGPPASSFD; cat - %s | %s "
	   "+language=%s +pubring=%s +secring=%s +verbose=0 +batchmode -abfst %s %s",
	   fname, NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), 
	   PgpSignAs ? "-u" : "",
	   PgpSignAs ? PgpSignAs : "");

  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}



pid_t pgp_v2_invoke_encrypt(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname, const char *uids, int sign)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd),
	   "%scat %s%s | %s +language=%s +pubring=%s +secring=%s +verbose=0 %s +batchmode -aeft%s %s%s %s",
	   sign ? "PGPPASSFD=0; export PGPPASSFD; " : "",
	   sign ? "- " : "",
	   fname,
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), 
	   option(OPTPGPENCRYPTSELF) ? "+encrypttoself" : "",
	   sign ? "s" : "",
	   sign && PgpSignAs ? "-u " : "",
	   sign && PgpSignAs ? PgpSignAs : "",
	   uids);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr, 
			       pgpinfd, pgpoutfd, pgperrfd);
}

void pgp_v2_invoke_import(struct pgp_vinfo *pgp, const char *fname)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd), "%s +language=%s +pubring=%s +secring=%s -ka %s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), fname);
  mutt_system(cmd);
}

pid_t pgp_v2_invoke_export(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd, const char *id)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd), "%s -kxaf +language=%s +pubring=%s +secring=%s 0x%8s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), id);

  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


pid_t pgp_v2_invoke_verify_key(struct pgp_vinfo *pgp,
			       FILE **pgpin, FILE **pgpout, FILE **pgperr,
			       int pgpinfd, int pgpoutfd, int pgperrfd, const char *id)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd), "%s +language=%s +pubring=%s +secring=%s +batchmode -kcc 0x%8s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), id);
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


/*******************************************************************
 * 
 * PGP V3 Invocation stuff
 * 
 *******************************************************************/


pid_t pgp_v3_invoke_decode(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd,
			   const char *fname, int need_passphrase)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd), "%scat %s%s | "
	   "%sv +language=%s +pubring=%s +secring=%s +verbose=0 +batchmode -f "
	   "--OutputInformationFD=2",
	   need_passphrase ? "PGPPASSFD=0; export PGPPASSFD; " : "",
	   need_passphrase ? "- " : "",
	   fname,
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring));
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


pid_t pgp_v3_invoke_verify(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd,
			   const char *signedstuff, const char *sigfile)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd),
	   "%sv +language=%s +pubring=%s +secring=%s --OutputInformationFD=1 +batchmode +verbose=0 %s %s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), sigfile, signedstuff);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}

pid_t pgp_v3_invoke_encrypt(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname, const char *uids, int sign)
{
  char cmd[HUGE_STRING];
  char tmpcmd[HUGE_STRING];
  char *cp;
  char *keylist;

  snprintf(cmd, sizeof(cmd),
	   "%scat %s%s | %se +language=%s +pubring=%s +secring=%s +verbose=0 %s +batchmode +nobatchinvalidkeys=off -aft%s %s%s",
	   sign ? "PGPPASSFD=0; export PGPPASSFD; " : "",
	   sign ? "- " : "",
	   fname,
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), 
	   option(OPTPGPENCRYPTSELF) ? "+encrypttoself" : "",
	   sign ? "s" : "",
	   sign && PgpSignAs ? "-u " : "",
	   sign && PgpSignAs ? PgpSignAs : "");
  
  keylist = safe_strdup(uids);
  
  for(cp = strtok(keylist, " "); cp ; cp = strtok(NULL, " "))
  {
    snprintf(tmpcmd, sizeof(tmpcmd), "%s -r %s", 
	     cmd, cp);
    strcpy(cmd, tmpcmd);
  }
  safe_free((void **) &keylist);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr, 
			       pgpinfd, pgpoutfd, pgperrfd);
}

pid_t pgp_v3_invoke_decrypt(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd),
	   "PGPPASSFD=0; export PGPPASSFD; cat - %s | %sv +language=%s +pubring=%s +secring=%s "
	   "+verbose=0 +batchmode -f --OutputInformationFD=2",
	   fname, NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring));
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


pid_t pgp_v3_invoke_sign(struct pgp_vinfo *pgp,
			 FILE **pgpin, FILE **pgpout, FILE **pgperr,
			 int pgpinfd, int pgpoutfd, int pgperrfd, 
			 const char *fname)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd),
	   "PGPPASSFD=0; export PGPPASSFD; cat - %s | %ss "
	   "+language=%s +pubring=%s +secring=%s +verbose=0 -abft %s %s",
	   fname, NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring),
	   PgpSignAs ? "-u" : "",
	   PgpSignAs ? PgpSignAs : "");

  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}
void pgp_v3_invoke_import(struct pgp_vinfo *pgp, const char *fname)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd), "%sk +language=%s +pubring=%s +secring=%s -a --OutputInformationFD=1 %s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), fname);
  mutt_system(cmd);
}


pid_t pgp_v3_invoke_export(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd, const char *id)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd), "%sk -xa +language=%s +pubring=%s +secring=%s --OutputInformationFD=1 0x%8s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), id);

  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}

pid_t pgp_v3_invoke_verify_key(struct pgp_vinfo *pgp,
			       FILE **pgpin, FILE **pgpout, FILE **pgperr,
			       int pgpinfd, int pgpoutfd, int pgperrfd, const char *id)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd), "%sk +language=%s +pubring=%s +secring=%s +batchmode -c --OutputInformationFD=1 0x%8s",
	   NONULL (*pgp->binary), NONULL (*pgp->language), NONULL (*pgp->pubring), NONULL (*pgp->secring), id);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}


/*******************************************************************
 * 
 * GNU Privacy Guard invocation stuff
 * 
 * Credits go to Werner Koch for sending me the code on which this 
 * is based.
 * 
 *******************************************************************/

pid_t pgp_gpg_invoke_decode(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname, int need_passphrase)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd),
	   "%s%s --no-verbose  -v --batch --status-fd 2 -o - "
	   "--decrypt %s",
	   NONULL(*pgp->binary), need_passphrase? " --passphrase-fd 0":"",
	   fname);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}

pid_t pgp_gpg_invoke_verify(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *signedstuff, const char *sigfile)
{  
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd),
	   "%s --no-verbose --batch --status-fd 2 -o - "
	   "--verify %s %s",
	   NONULL(*pgp->binary), sigfile, signedstuff);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}

pid_t pgp_gpg_invoke_decrypt(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd),
	   "%s --passphrase-fd 0 --no-verbose -v --batch --status-fd 2 -o - "
	   "--decrypt %s",
	   NONULL(*pgp->binary), fname );
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}

static char *gpg_digalg(void)
{
  static char digalg[STRING];
  if(PgpSignMicalg && !strncasecmp(PgpSignMicalg, "pgp-", 4))
    strfcpy(digalg, PgpSignMicalg + 4, sizeof(digalg));
  else
  {
   /* We use md5 here as the default value as it's the good
    * old default value for PGP and will be used in the
    * message's headers.
    */

    strcpy(digalg, "md5");
  }
  return digalg;
}
  
pid_t pgp_gpg_invoke_sign(struct pgp_vinfo *pgp,
			 FILE **pgpin, FILE **pgpout, FILE **pgperr,
			 int pgpinfd, int pgpoutfd, int pgperrfd, 
			 const char *fname)
{
  char cmd[HUGE_STRING];
  
  snprintf(cmd, sizeof(cmd),
	   "%s --no-verbose -vv --batch --status-fd 2 -o - "
	   "--passphrase-fd 0 --digest-algo %s "
	   "--detach-sign --textmode --armor %s%s %s",
	   NONULL(*pgp->binary),
	   gpg_digalg(),
	   *PgpSignAs? "-u " : "",
	   *PgpSignAs? PgpSignAs : "", fname );
  
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}



pid_t pgp_gpg_invoke_encrypt(struct pgp_vinfo *pgp,
			    FILE **pgpin, FILE **pgpout, FILE **pgperr,
			    int pgpinfd, int pgpoutfd, int pgperrfd,
			    const char *fname, const char *uids, int sign)
{
  char cmd[HUGE_STRING];
  char tmpcmd[HUGE_STRING];
  char *cp;
  char *keylist;
  
  snprintf(cmd, sizeof(cmd),
	   "%s%s --no-verbose --batch --status-fd 2 -o - "
	   "--digest-algo %s "
	   "--encrypt%s --textmode --armor %s%s",
	   NONULL(*pgp->binary),
	   sign? " --passphrase-fd 0":"",
	   gpg_digalg(),
	   sign? " --sign":"",
	   *PgpSignAs? "-u " : "",
	   *PgpSignAs? PgpSignAs : "" );
  
  keylist = safe_strdup(uids);

  for(cp = strtok(keylist, " "); cp ; cp = strtok(NULL, " "))
  {
    snprintf(tmpcmd, sizeof(tmpcmd), "%s -r %s",
	     cmd, cp);
    strcpy(cmd, tmpcmd);
  }
  safe_free((void **) &keylist);
  snprintf(tmpcmd, sizeof(tmpcmd), "%s %s", cmd, fname);
  strcpy(cmd, tmpcmd);
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr, 
			       pgpinfd, pgpoutfd, pgperrfd);
}

void pgp_gpg_invoke_import(struct pgp_vinfo *pgp, const char *fname)
{
  mutt_error("pgp_gpg_invoke_import() has not yet been implemented.");
}

pid_t pgp_gpg_invoke_export(struct pgp_vinfo *pgp,
			   FILE **pgpin, FILE **pgpout, FILE **pgperr,
			   int pgpinfd, int pgpoutfd, int pgperrfd, const char *id)
{
  mutt_error("pgp_gpg_invoke_export() has not yet been implemented.");
  return -1;
}


pid_t pgp_gpg_invoke_verify_key(struct pgp_vinfo *pgp,
			       FILE **pgpin, FILE **pgpout, FILE **pgperr,
			       int pgpinfd, int pgpoutfd, int pgperrfd, const char *id)
{
  char cmd[HUGE_STRING];

  snprintf(cmd, sizeof(cmd),
	   "%sm --no-verbose --batch --fingerprint --check-sigs %s%s",
	   NONULL(*pgp->binary), (strlen(id)==8 || strlen(id)==16)? "0x":"", id );
  
  return mutt_create_filter_fd(cmd, pgpin, pgpout, pgperr,
			       pgpinfd, pgpoutfd, pgperrfd);
}
