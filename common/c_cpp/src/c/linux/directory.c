/* $Id$
 *
 * OpenMAMA: The open middleware agnostic messaging API
 * Copyright (C) 2011 NYSE Technologies, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include<sys/types.h>
#include<sys/stat.h>
#include<err.h>
#include<fts.h>
#include<fnmatch.h>
#include<stddef.h>
#include<string.h>
#include<wombat/directory.h>
#include<wombat/port.h>

int enumerateDirectory (const char*  path,
                        const char*  pattern,
                        fileNameCb   cb,
                        void*        closure) 
{
    FTS*       ftsp  = NULL;
    FTSENT *p, *chp  = NULL;
    char*       sep  = NULL;

    char* const  pathlist[2] = {(char*)path, NULL};
 
    int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
    int rval = 0;
    
    if ((ftsp = fts_open(pathlist, fts_options, NULL)) == NULL) 
    {
        /*Path not found*/
        return -1;
    }
    
    chp = fts_children(ftsp, 0);
    if (chp == NULL) 
    {
        /*No files to traverse*/
        return -1;               
    }
    
    while ((p = fts_read(ftsp)) != NULL)
    {
        if(p->fts_info == FTS_F) 
        {
            sep = strrchr(p->fts_path, '/');
            if (!sep) continue;
           
            /* path includes the filename - we only want the path*/ 
            *sep = '\0';
            
            if (!pattern ||
                0 == fnmatch(pattern, p->fts_name, 0))
            {
                cb (p->fts_path, p->fts_name, closure);
            }
        }
    }
    fts_close(ftsp);
    return 0;
}
