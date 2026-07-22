// project: dicm2deck
// file: deck_sqlite.c
// created by jacquesfauquex on 2026-04-24.

#include "deck_sqlite3.h"
#include "datatypes.h"
#include "../thirdparty/sqlite3/sqlite3.h"

static sqlite3      *db;
static char         *dberr = 0;
static int           dbrc  = 0;//return code

//statements
static sqlite3_stmt *eblake3stmt;
static sqlite3_stmt *einsertstmt;
static int currentEpk;

static sqlite3_stmt *sblake3stmt;
static sqlite3_stmt *sinsertstmt;
static int currentSpk;

static sqlite3_stmt *iblake3stmt;
static sqlite3_stmt *iinsertstmt;
static int currentIpk;

static sqlite3_stmt *iframepksstmt;
static sqlite3_stmt *finsertstmt;

static sqlite3_stmt *tinsertstmt;

void sqlite3open() {
   //file deck.db within current dir
   //also prepare statements
   struct stat deckdbfileinfo;
   if (stat("deck.db", &deckdbfileinfo)==-1)
   {
      dbrc = sqlite3_open_v2("deck.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
      if (dbrc == SQLITE_OK)
      {
         //TODO create default db from a dedicated file in the  project
      }
   }
   else dbrc = sqlite3_open_v2("deck.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

   if (dbrc != SQLITE_OK)
   {
      E( "Cannot open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }

#pragma mark E sqlite stmt(s)
   //blake3 stmt
   char eblake3[]="SELECT pk, eblake3 FROM E WHERE euid=?;";
   dbrc = sqlite3_prepare_v2(db, eblake3, sizeof(eblake3), &eblake3stmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot initialize eblake3stmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
   char einsert[] = "INSERT INTO E(procid,edate,euid,edckv,eblake3,pname,pide,pidr,pbirth,psex,eid,ean,eal,eau,eat,img,cda,req,ref,pay,edesc,ecode) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
   dbrc=sqlite3_prepare(db, einsert, -1, &einsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare einsertStmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }


#pragma mark S sqlite stmt(s)
   //blake3 stmt
   char sblake3[]="SELECT pk, sblake3 FROM S WHERE suid=?;";
   dbrc = sqlite3_prepare_v2(db, sblake3, sizeof(sblake3), &sblake3stmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot initialize Sblake3stmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
   char sinsert[] = "INSERT INTO S(Efk,sdate,stime,suid,sdckv,sblake3,sxml,spdf,snumber,sclass,smod,sdesc,sicon,sframes) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
   dbrc=sqlite3_prepare(db, sinsert, -1, &sinsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare sinsertStmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }



#pragma mark I+P sqlite stmt(s)
   //blake3 stmt
   char iblake3[]="SELECT pk, iblake3 FROM I WHERE iuid=?;";
   dbrc = sqlite3_prepare_v2(db, iblake3, sizeof(iblake3), &iblake3stmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot initialize iblake3stmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
   char iinsert[] = "INSERT INTO I(Sfk,iuid,idckv,iblake3,inumber,ianumber,iclass,itype,syntaxidx,icomment,iframes,pdckv,spp,photocode,rows,cols,alloc,stored,high,pixrep,planar,DICMrelpath) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
   dbrc=sqlite3_prepare(db, iinsert, -1, &iinsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare iinsertStmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }



#pragma mark F sqlite stmt(s)
   //existing one
   /*
    Ifk
    pk
    fnumber
    fdckv BLOB
    DICMidx
    DICMlen
    syntaxidx
    compressed
    fast
    high
    original
    */
   char iframepks[]="SELECT pk, fnumber FROM F WHERE Ifk=?;";
   dbrc = sqlite3_prepare_v2(db, iframepks, sizeof(iframepks), &iframepksstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot initialize iframepksstmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
   char finsertchars[] = "INSERT INTO F(Ifk,fnumber,DICMidx,DICMlen,syntaxidx,compressed,fast,high,original) VALUES(?,?,?,?,?,?,?,?,')";
   dbrc=sqlite3_prepare(db, finsertchars, -1, &finsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare finsertstmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }

   char fcinsertchars[] = "INSERT INTO F(Ifk,fnumber,fdckv,nativeurl,syntaxidx,compressed) VALUES(?,?,?,?,?,?)";
   dbrc=sqlite3_prepare(db, finsertchars, -1, &finsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare finsertstmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
   char fcfhoinsertchars[] = "INSERT INTO F(Ifk,fnumber,fdckv,nativeurl,syntaxidx,compressed,fast,high,original) VALUES(?,?,?,?,?,?,?,?,?)";
   dbrc=sqlite3_prepare(db, finsertchars, -1, &finsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare finsertstmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }



#pragma mark T sqlite stmt(s)
   //existing one
   /*
    Ifk
    pk
    blob BLOB
    */
   /*
   char iframepks[]="SELECT pk, fnumber FROM F WHERE Ifk=?;";
   dbrc = sqlite3_prepare_v2(db, iframepks, sizeof(iframepks), &iframepksstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot initialize iframepksstmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
    */
   /*
   char tinsertchars[] = "INSERT INTO T(Ifk,blob) VALUES(?,?)";
   dbrc=sqlite3_prepare(db, tinsertchars, -1, &tinsertstmt, 0);
   if (dbrc != SQLITE_OK)
   {
      E( "Cannot prepare tinsertstmt: %s\n", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
      exit(1);
   }
*/
}

void xx()
{
#pragma mark pid dir
   sprintf(relativepath, "%d", getpid());
   relativepathlength=intdecsize(getpid());
   relativepath[relativepathlength++]='/';
   relativepath[relativepathlength]=0x00;
   struct stat relativepathfileinfo1;
   if ((stat(relativepath, &relativepathfileinfo1)==-1) && (mkdir(relativepath, 0777)==-1)) return false;
   //iuidb64
   if (iuidb64length==0) return false;
   memcpy(relativepath+relativepathlength, iuidb64, iuidb64length);
   relativepath[relativepathlength+iuidb64length]=0;


#pragma mark E 0000000000000000
   if (Eidx>0)
   {
      currentEpk=0;
      blake3_hasher_reset(&hasher);
      blake3_hasher_update(&hasher, Ebuf, Eidx);
      blake3_hasher_finalize(&hasher, hashbytes, BLAKE3_OUT_LEN);

      //does euidb64[euidb64length] exist in sqlite3 and what is its blake3 ?
      //find currentEpk
      sqlite3_bind_text(eblake3stmt, 1, euidb64,euidb64length, NULL);
      stepreturnstatus = sqlite3_step(eblake3stmt);
      if (stepreturnstatus != SQLITE_ROW)
      {
         if (!einsert()) return false;
         //get pk
         sqlite3_reset(eblake3stmt);
         sqlite3_bind_text(eblake3stmt, 1, euidb64,euidb64length, NULL);
         stepreturnstatus = sqlite3_step(eblake3stmt);
         currentEpk=sqlite3_column_int(eblake3stmt, 0);
         D("sqlite commit E pk: %d",currentEpk);
      }
      else //euidb64 exists in E
      {
         //more than one ROW?
         notRegistered=true;
         while (notRegistered)
         {
            registeredhashbytes=(u8 *)(sqlite3_column_blob(eblake3stmt, 1));
            //equals?
            if (memcmp(hashbytes, registeredhashbytes, BLAKE3_OUT_LEN)==0)
            {
               notRegistered=false;
               currentEpk=sqlite3_column_int(eblake3stmt, 0);
               continue;
            }
            //next answer?
            stepreturnstatus = sqlite3_step(eblake3stmt);
            notRegistered=(stepreturnstatus == SQLITE_ROW);
         }
         if (notRegistered)
         {
            if (!einsert()) return false;
            //get pk
            sqlite3_reset(eblake3stmt);
            sqlite3_bind_text(eblake3stmt, 1, euidb64,euidb64length, NULL);
            stepreturnstatus = sqlite3_step(eblake3stmt);
            currentEpk=sqlite3_column_int(eblake3stmt, 0);
            D("sqlite commit E pk: %d",currentEpk);
         }
      }
      sqlite3_reset(eblake3stmt);

      Eidx=0;
   }

#pragma mark S : 1 s SS SS 0r RR RR CC CC
   if (Sidx>0)
   {
      //blake3 computed in the same conditions without prefix variability
      blake3_hasher_reset(&hasher);
      blake3_hasher_update(&hasher, Sbuf, Sidx);
      blake3_hasher_finalize(&hasher, hashbytes, BLAKE3_OUT_LEN);

//does suidb64[suidb64length] exist in sqlite3 and what is its blake3 ?
//find currentSpk
      currentSpk=0;
      sversion=0;
      sqlite3_bind_text(sblake3stmt, 1, suidb64,suidb64length, NULL);
      stepreturnstatus = sqlite3_step(sblake3stmt);
      if (stepreturnstatus != SQLITE_ROW)
      {
         if (!sinsert(0x10|sversion|u16swap(snumber)*0x100|rversion*0x1000000|u16swap(rnumber)*0x100000000|u16swap(classidx)*0x100000000000000)) return false;
      }
      else //suidb64 exists in S
      {
         //more than one ROW?
         bool notRegistered=true;
         while (notRegistered)
         {
            registeredhashbytes=(u8 *)(sqlite3_column_blob(sblake3stmt, 1));
            //equals?
            if (memcmp(hashbytes, registeredhashbytes, BLAKE3_OUT_LEN)==0)
            {
               notRegistered=false;
               currentSpk=sqlite3_column_int(sblake3stmt, 0);
               continue;
            }
            sversion++;
            //next answer?
            stepreturnstatus = sqlite3_step(sblake3stmt);
            notRegistered=(stepreturnstatus == SQLITE_ROW);
         }
         if (notRegistered)
         {
            if (!sinsert(0x10|sversion|u16swap(snumber)*0x100|rversion*0x1000000|u16swap(rnumber)*0x100000000|u16swap(classidx)*0x100000000000000)) return false;
         }
      }
      //reset (not snumber y sversion que se usan para I N C)
      sqlite3_reset(sblake3stmt);
      Sidx=0;
   }

#pragma mark IP : 2/3 s SS SS iu II II CC CC
   //u iversion (relates the frames of instances into a same volume)
   //versions of the same instance with diferent quality have diferent i
   if (Iidx>0 || Pidx>0)
   {
      //blake3
      blake3_hasher_reset(&hasher);
      blake3_hasher_update(&hasher, Ibuf, Iidx);
      blake3_hasher_finalize(&hasher, hashbytes, BLAKE3_OUT_LEN);

      //does iuidb64[iuidb64length] exist in sqlite3 and what is its blake3 ?
      //find currentIpk
      currentIpk=0;
      iversion=0;
      concat=0;
      sqlite3_bind_text(iblake3stmt, 1, iuidb64,iuidb64length, NULL);
      stepreturnstatus = sqlite3_step(iblake3stmt);
      if (stepreturnstatus != SQLITE_ROW)
      {
         if (!iinsert(0x20|sversion|u16swap(snumber)*0x100|iversion*0x100000|concat*0x1000000|u16swap(inumber)*0x100000000|u16swap(classidx)*0x100000000000000)) return false;
      }
      else //iuidb64 exists in I
      {
         //more than one ROW?
         bool notRegistered=true;
         while (notRegistered)
         {
            registeredhashbytes=(u8 *)(sqlite3_column_blob(iblake3stmt, 1));
                  //equals?
            if (memcmp(hashbytes, registeredhashbytes, BLAKE3_OUT_LEN)==0)
            {
               notRegistered=false;
               currentIpk=sqlite3_column_int(iblake3stmt, 0);
               continue;
            }

            //next answer?
            stepreturnstatus = sqlite3_step(iblake3stmt);
            notRegistered=(stepreturnstatus == SQLITE_ROW);
         }
         if (notRegistered)
         {
            if (!iinsert(0x20|sversion|u16swap(snumber)*0x100|iversion*0x100000|concat*0x1000000|u16swap(inumber)*0x100000000|u16swap(classidx)*0x100000000000000)) return false;
         }
      }
      sqlite3_reset(iblake3stmt);
      Iidx=0;
      Pidx=0;
   }
   return true;
}


