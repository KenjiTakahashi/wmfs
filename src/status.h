/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef STATUS_H
#define STATUS_H

#include "wmfs.h"

struct status_ctx status_new_ctx(struct barwin *b, struct theme *t);
void status_free_ctx(struct status_ctx *ctx);
void status_flush_list(struct status_ctx *ctx);
void status_flush_mousebind(struct status_ctx *ctx);
void status_copy_mousebind(struct status_ctx *ctx);
void status_parse(struct status_ctx *ctx);
void status_render(struct status_ctx *ctx);
void status_render_with_offset(struct status_ctx *ctx, int offset);
void status_manage(struct status_ctx *ctx);
void status_manage_with_offset(struct status_ctx *ctx, int offset);
void status_flush_surface(void);
void uicb_status(Uicb cmd);
void uicb_status_surface(Uicb cmd);

#endif /* STATUS_H */
