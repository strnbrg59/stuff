/*
**   _______              __
**  / ___/ /  ___  __ _  / /  ___
** / /__/ _ \/ _ \/  ' \/ _ \/ _ \
** \___/_//_/\___/_/_/_/_.__/\___/ 
**
** This software is copyright (C) by the Lawrence Berkeley
** National Laboratory.  Permission is granted to reproduce
** this software for non-commercial purposes provided that
** this notice is left intact.
** 
** It is acknowledged that the U.S. Government has rights to
** this software under Contract DE-AC03-765F00098 between
** the U.S. Department of Energy and the University of
** California.
**
** This software is provided as a professional and academic
** contribution for joint exchange.  Thus it is experimental,
** is provided ``as is'', with no warranties of any kind
** whatsoever, no support, no promise of updates, or printed
** documentation.  By using this software, you acknowledge
** that the Lawrence Berkeley National Laboratory and
** Regents of the University of California shall have no
** liability with respect to the infringement of other
** copyrights by any part of this software.
**
*/
// Author: Ted Sternberg
#ifndef INCLUDED_CONSTS_H
#define INCLUDED_CONSTS_H

#include <string>
using std::string;

/** Mostly string constants.  Several cases:
 *    1. Constants that are needed for both hdf5 formats (old, and new-eb)
 *       and are spelled the same in both: static data in parent Consts class.
 *    2. Constants needed in both formats but are spelled differently (e.g.
 *       "dx" and "DX": instance data in parent class, initialized in subclass
 *       ctors.  Accessed as member data of polymorphic Consts pointer that's
 *       member data of class ChomboHDF5 and gets passed as param to some global
 *       functions.
 *    3. Constants needed in only one of the formats: static in subclass (and
 *       therefore inaccessible from polymorphic Consts pointer).
*/
struct Consts
{
    //
    // Names of attributes in the Chombo HDF5 format
    //

    // Static -- initialized in Consts.cpp.
    static char const * const BoxCorners;
    static char const * const Chombo_global;
    static char const * const SpaceDim;
    static char const * const component_;
    static char const * const comps;
    static char const * const data_attributes;
    static char const * const data_centering;
    static char const * const data_datatype0;
    static char const * const intvecti;
    static char const * const intvectj;
    static char const * const intvectk;
    static char const * const iteration;
    static char const * const name;
    static char const * const num_components;
    static char const * const num_particles;
    static char const * const objectType;
    static char const * const origin;
    static char const * const particles;
    static char const * const realvectx;
    static char const * const realvecty;
    static char const * const realvectz;
    static char const * const ref_ratio;
    static char const * const space_dim;
    static char const * const testReal;
    static char const * const time;
    static char const * const values;


    // Nonstatic -- initialized in subclass constructors.
    char const * anisotropic;
    char const * boxes;
    char const * dx;
    char const * dt;
    char const * filetype;
    char const * filetype_name;
    char const * ghost;
    char const * outputGhost;
    char const * output_ghost; // Invented for ascii format.
    char const * level_; // initialized in subclass
    char const * num_levels; // initialized in subclass
    char const * prob_domain; // initialized in subclass


    // Other constants (unrelated to any HDF5 format).
    static char const * const precision;
    static char const * const component_names;
    static char const * const data;
    static char const * const unlikely_name;
    static char const * const position_x;
    static char const * const position_y;
    static char const * const position_z;
    static char const * const chombovis_home;
    static char const * const ascii2hdf5;
};


/** For things that appear in the new EB HDF5 format but are spelled differently
 */
struct OldConsts : public Consts
{
    OldConsts()
    {
        anisotropic = "anisotropic";
        boxes = "boxes";
        dx = "dx";
        dt = "dt";
        filetype = "filetype";
        filetype_name = "VanillaAMRFileType";
        ghost = "ghost";
        level_ = "level_";
        num_levels = "num_levels";
        output_ghost = "output_ghost";
        outputGhost = "outputGhost";
        prob_domain = "prob_domain";
    }
};


/** For string constants new to the EB HDF5 format, or not but spelled
 *  differently than in the old format.
*/
struct EBConsts : public Consts
{
    EBConsts()
    {
        anisotropic = "AspectRatio";
        boxes = "Boxes";
        dx = "DX";
        dt = "DT";
        filetype = "Filetype";
        filetype_name = "Chombo EB File";
        level_ = "Level";
        num_levels = "NumLevels";
        // output_ghost : not needed
        outputGhost = "Ghost";
        prob_domain = "ProblemDomain";
    }

    static char const * const CenteredComponents;
    static char const * const centering_prefixes[8];
    static char const * const short_centering_prefixes[8];
};

#endif
