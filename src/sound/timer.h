/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#ifndef _TIMER_H_
#define _TIMER_H_


typedef struct timer_struct {
    double time;		// when
    unsigned int odo_debut;
    unsigned int nb_cycle;
    int param;
    unsigned int del_it;
    void (*func) (int param);
    struct timer_struct *next;
} timer_struct;

extern double neogeo_timer_count;

timer_struct *insert_timer(double duration, int param, void (*func) (int));
void del_timer(timer_struct * ts);
void my_timer(void);
double timer_get_time(void);
void free_all_timer(void);

#endif
