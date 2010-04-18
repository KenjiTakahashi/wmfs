/*
*      tag.c
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "wmfs.h"

/* Set the tag
 * \param tag The tag number
*/
void
tag_set(int tag)
{
     Client *c;
     Bool al = False;
     int i;

     screen_get_sel();

     prevseltag[selscreen] = seltag[selscreen];

     if(conf.tag_round)
     {
          if(tag <= 0)
               seltag[selscreen] = conf.ntag[selscreen];
          else if(tag  > conf.ntag[selscreen])
               seltag[selscreen] = 1;
          else
               seltag[selscreen] = tag;
     }
     else
     {
          if(!tag || tag == seltag[selscreen]
             || tag > conf.ntag[selscreen])
               return;

          seltag[selscreen] = tag;
     }

     ewmh_update_current_tag_prop();

     /* Arrange infobar position */
     if(tags[selscreen][prevseltag[selscreen]].barpos != tags[selscreen][seltag[selscreen]].barpos)
          infobar_set_position(tags[selscreen][seltag[selscreen]].barpos);

     /* Check if a layout update is needed with additional tags */
     if(tags[selscreen][seltag[selscreen]].tagad)
          al = True;

     for(i = 1; i < conf.ntag[selscreen] + 1; ++i)
          if(tags[selscreen][i].tagad & TagFlag(seltag[selscreen]))
               al = True;

     arrange(selscreen, al);

     if(tags[selscreen][tag].request_update)
     {
          tags[selscreen][seltag[selscreen]].layout.func(selscreen);
          tags[selscreen][tag].request_update = False;
     }

     /* To focus the first client in the new tag */
     for(c = clients; c; c = c->next)
          if(c->tag == seltag[selscreen] && c->screen == selscreen)
               break;

     client_focus((c) ? c : NULL);

     return;
}

/* Transfert a client to a tag
 * \param c Client pointer
 * \param tag Tag
*/
void
tag_transfert(Client *c, int tag)
{
     screen_get_sel();

     CHECK(c);

     if(!tag)
          tag = 1;

     if(tag > conf.ntag[selscreen])
          return;

     c->tag = tag;
     c->screen = selscreen;

     arrange(c->screen, True);

     if(c == sel && c->tag != tag)
          client_focus(NULL);

     client_update_attributes(c);

     tags[c->screen][tag].request_update = True;

     return;
}

/** Uicb Set a tag
 * \param cmd Tag number or '+' / '-', uicb_t type
*/
void
uicb_tag(uicb_t cmd)
{
     int tmp = atoi(cmd);

     if(cmd[0] == '+' || cmd[0] == '-')
          tag_set(seltag[selscreen] + tmp);
     else
          tag_set(tmp);

     return;
}

/** Set the next tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_next(uicb_t cmd)
{
     screen_get_sel();

     tag_set(seltag[selscreen] + 1);

     return;
}

/** Set the previous tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_prev(uicb_t cmd)
{
     screen_get_sel();

     tag_set(seltag[selscreen] - 1);

     return;
}

/** Transfert the selected client to
 *  the wanted tag
 * \param cmd Wanted tag, uicb_t type
*/
void
uicb_tagtransfert(uicb_t cmd)
{
     CHECK(sel);

     tag_transfert(sel, atoi(cmd));

     return;
}

/** Set the previous selected tag
  * \param cmd uicb_t type unused
  */
void
uicb_tag_prev_sel(uicb_t cmd)
{
     screen_get_sel();

     tag_set(prevseltag[selscreen]);

     return;
}

/** Transfert the selected client to the next tag
 * \param cmd uicb_t type unused
*/
void
uicb_tagtransfert_next(uicb_t cmd)
{
     CHECK(sel);
     int tag = seltag[selscreen] + 1;

     if(tag > conf.ntag[selscreen])
     {
         if(!conf.tag_round)
             return;
         tag = 1;
     }
     tag_transfert(sel, tag);

     return;
}

/** Transfert the selected client to the prev tag
 * \param cmd uicb_t type unused
*/
void
uicb_tagtransfert_prev(uicb_t cmd)
{
     CHECK(sel);
     int tag = seltag[selscreen] - 1;

     if(tag <= 0)
     {
         if(!conf.tag_round)
              return;
         tag = conf.ntag[selscreen];
     }
     tag_transfert(sel, tag);

     return;
}

/** Go to the current urgent tag
  *\param cmd uicb_t type unused
  */
void
uicb_tag_urgent(uicb_t cmd)
{
     Client *c;
     Bool b = False;

     /* Check if there is a urgent client */
     for(c = clients; c; c = c->next)
          if(c->flags & UrgentFlag)
          {
               b = True;
               break;
          }

     if(!b)
          return;

    screen_set_sel(c->screen);
    tag_set(c->tag);
    client_focus(c);

    return;
}

/** Add an additional tag to the current tag
  *\param sc Screen
  *\param tag Tag where apply additional tag
  *\param adtag Additional tag to apply in tag
 */
void
tag_additional(int sc, int tag, int adtag)
{
     if(tag < 0 || tag > conf.ntag[sc]
       || adtag < 1 || adtag > conf.ntag[sc] || adtag == seltag[sc])
          return;

     tags[sc][tag].tagad ^= TagFlag(adtag);
     tags[sc][adtag].request_update = True;
     arrange(sc, True);

     return;
}

/** Add an additional tag to the current tag
  *\param cmd uicb_t
 */
void
uicb_tag_toggle_additional(uicb_t cmd)
{
     screen_get_sel();

     tag_additional(selscreen, seltag[selscreen], atoi(cmd));

     return;
}

/** Swap 2 tags
  *\param s  Screen
  *\param t1 Tag 1
  *\param t2 Tag 2
*/
void
tag_swap(int s, int t1, int t2)
{
     Client *c;
     Tag t;

     if(t1 > conf.ntag[s] || t1 < 1
               || t2 > conf.ntag[s] || t2 < 1 || t1 == t2)
          return;

     t = tags[s][t1];
     tags[s][t1] = tags[s][t2];
     tags[s][t2] = t;

     for(c = clients; c; c = c->next)
     {
          if(c->screen == s && c->tag == t1)
               c->tag = t2;
          else if(c->screen == s && c->tag == t2)
               c->tag = t1;
     }

     infobar_update_taglist(s);
     tag_set(t2);

     return;
}

/** Swap current tag with a specified tag
  *\param cmd uicb_t type
*/
void
uicb_tag_swap(uicb_t cmd)
{
     screen_get_sel();

     tag_swap(selscreen, seltag[selscreen], atoi(cmd));

     return;
}

/** Swap current tag with next tag
  *\param cmd uicb_t type
*/
void
uicb_tag_swap_next(uicb_t cmd)
{
     screen_get_sel();

     tag_swap(selscreen, seltag[selscreen], seltag[selscreen] + 1);

     return;
}

/** Swap current tag with previous tag
  *\param cmd uicb_t type
*/
void
uicb_tag_swap_previous(uicb_t cmd)
{
     screen_get_sel();

     tag_swap(selscreen, seltag[selscreen], seltag[selscreen] - 1);

     return;
}

/** Adding a tag
  *\param s Screen number
  *\param name New tag name
*/
void
tag_new(int s, char *name)
{
     Tag t = { NULL, NULL, 0, 0, 0.65, 1, False, False, False, False, IB_Top,
          layout_name_to_struct(conf.layout, "tile_right", conf.nlayout, layout_list), 0, NULL, 0 };

     if(conf.ntag[s] + 1 > MAXTAG)
     {
          warnx("Too many tag: Can't create new tag");

          return;
     }

     ++conf.ntag[s];

     tags[s][conf.ntag[s]] = t;

     tags[s][conf.ntag[s]].name = _strdup(((strlen(name)) ? name : "new tag"));

     infobar_update_taglist(s);
     infobar_draw(s);

     return;
}

/** Adding a tag
  *\param cmd uicb_t type
*/
void
uicb_tag_new(uicb_t cmd)
{
     screen_get_sel();

     tag_new(selscreen, (char*)cmd);

     return;
}

/** Delete a tag
  *\param s Screen number
  *\param tag Tag number
*/
void
tag_delete(int s, int tag)
{
     Tag t = { 0 };
     Client *c;
     int i;

     if(tag < 0 || tag > conf.ntag[s] || conf.ntag[s] == 1)
          return;

     for(c = clients; c; c = c->next)
          if(c->screen == s && c->tag == tag)
          {
               warnx("Client(s) present in this tag, can't delete it");

               return;
          }

     --conf.ntag[s];

     tags[s][tag] = t;
     infobar[s].tags[tag] = NULL;

     for(i = tag; i < conf.ntag[s] + 1; ++i)
          tags[s][i] = tags[s][i + 1];

     infobar[s].need_update = True;
     infobar_update_taglist(s);
     infobar_draw(s);

     tag_set(tag);

     return;
}

/** Delete a tag
  *\param cmd uicb_t type
*/
void
uicb_tag_del(uicb_t cmd)
{
     screen_get_sel();

     tag_delete(selscreen, ((strlen((char*)cmd)) ? atoi(cmd) : seltag[selscreen]));

     return;
}

